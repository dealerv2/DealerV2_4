/*  JGM -- File dealdds_subs.c 2022-feb-15
 *     Borrows some defs and types from Dealer .h files without importing everything as there are name conflicts
 */
#ifndef _GNU_SOURCE
  #define _GNU_SOURCE
#endif

#define CACHE_INV 0
#define CACHE_UPD 2
#define CACHE_OK  1
#define DDS_TABLE_MODE 2
#define DDS_BOARD_MODE 1
#define INV_TRIX -1
/* 
 * These next two not used in Dealerv2; may be useful in a library of generic bridge functions
 */
#define DDS_INP_DEALER = 4
#define DDS_INP_PBN    = 8

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "../include/dealdefs.h"
#include "../include/dds_dll.h"
#include "../include/dbgprt_macros.h"

#include "../include/dealdds_subs.h"
extern struct options_st options ;
extern int dds_dealnum;          /* keeps track of whether the dds cache needs refreshing */
extern int dbg_dds_lib_calls;    /* print dds counts at end of run for debugging and timing */
extern int dbg_dds_res_calls;
extern int dbg_parscore_calls;
extern int csv_firsthand;        /* not used was intending to allow user to print deal in arbitrary seq; but too confusing */
extern int zrdlib_mode ;
extern int TblModeThreads;

/*API we export to the DDS side */
typedef char     CARD52_k ; 
typedef CARD52_k DEAL52_k[52];
extern int dds_mode ; /* 1 = solveOneBoard, 2=CalcDDtable */
extern int ngen, dds_dealnum, jgmDebug ;
extern DEAL52_k  curdeal;

/* Global var for use by Dealer code -- structure with tricks, parscore, error message etc.*/
DDSRES_k dds_res_bin;

/*    Prototypes of Dealer code, calling DDS library using DDS structs */
#include "../include/Dealer_DDS_IF.h"

char Denom[]         = "NSHDC";  	// Different order yet again ... for the parResultsMaster structure
char seatNames[6][3] = { {"N"},{"E"},{"S"},{"W"},{"NS"},{"EW"} };
char dl52Vul[4][6]   = { {"none"}, {"ns"}, {"ew"}, {"both"} } ;
    
DDSRES_k true_CalcTable(DEAL52_k  dl, int vul, int dds_mode ) ; // ddTableResults uses [suits][hands] index order.
int      true_SolveBoard(DEAL52_k  curdeal, int compass, int strain ) ;
int      rplib_Par(DDSRES_k *DealerRes, int par_vuln) ;
int      ddsParCalcs( struct ddTableResults  *pRes_20,   DDSRES_k *pDealerRes ) ;
char    *dds_ParContract(char *buff,  int dl52_vul, struct parResultsMaster *sidesRes ) ; 


void ZeroCache( DDSRES_k *Results) {
    memset(Results, 0 ,  sizeof(DDSRES_k) ) ;
    memset(Results->tricks, INV_TRIX , sizeof(Results->tricks) ) ;
}

/* return 0 = Cache invalid, 1 = entry [h][s] needs update, 2 = Cache OK */
int CheckCache ( DDSRES_k *Results, int ngen, int ddsnum , int h, int s ) {
  #ifdef JGMDBG
     if ( jgmDebug >= 8 ) {
       fprintf(stderr, "CheckCache.66 ngen=%d,ddsnum=%d, h=%d, s=%d,Res.Tricks[h][s]=%d, INVTRIX=%d \n",
               ngen,ddsnum,h,s,Results->tricks[h][s], INV_TRIX );
      }
  #endif
    if( ngen != ddsnum ) return CACHE_INV ; // call dds library either SolveBoard or CalcDDtable depending on mode
    if ( Results->tricks[h][s] == INV_TRIX ) return CACHE_UPD ; // need to call DDS lib to update this entry at least
    assert(0<=Results->tricks[h][s] && 13 >= Results->tricks[h][s] );
    return CACHE_OK ;   // No need to call DDS lib.
}

int SetDDSmode(int mode) {       /* Default is mode 1; but Par, csv_trix, and the user -M2 will set to mode 2 */
    if ( mode == DDS_BOARD_MODE ) {
        dds_mode = DDS_BOARD_MODE ;
        SetResources(320 , 2 ) ;     // We probably can only use 1 thread in this mode, but give it 2 just in case.
        options.maxRamMB = 320 ;            // update global vars for consistency
        options.nThreads = 2 ;
        return 1 ;
    }
    if (mode == DDS_TABLE_MODE ) {
        dds_mode = DDS_TABLE_MODE ;
        if (options.nThreads < 2 ) {                 // The user has not specified nThreads via a -R switch so use TBLMODE default
            options.maxRamMB = TblModeThreads*160 ;       // update global vars for consistency
            options.nThreads = TblModeThreads ;
            SetResources(options.maxRamMB, TblModeThreads) ;
        }
        else {
            options.maxRamMB = options.nThreads*160;
            SetResources(options.maxRamMB, options.nThreads) ;  // Honor Users -R switch value.
        }
        return 1 ;
    }
    else  {
        fprintf(stderr, "SetDDSmode:: dds mode[%d] is INVALID!!. Setting to TableMode with %d Threads \n",
        mode, TblModeThreads );
        dds_mode = DDS_TABLE_MODE ;
        options.maxRamMB = TblModeThreads*160 ;       
        options.nThreads = TblModeThreads ;
        SetResources(options.maxRamMB, TblModeThreads) ;

        return 1 ;
    }
} /* end SetDDSmode */

int dds2dl_vuln( int dds_vuln ) {
	int dl_vuln;
	dl_vuln =  (dds_vuln == 0 ) ? 0 :  // none
	           (dds_vuln == 1 ) ? 3 :  // both
	           (dds_vuln == 2 ) ? 1 :  // NS
	           (dds_vuln == 3 ) ? 2 :  // EW
	            0 ;							// assume none if invalid entry
	return dl_vuln ;
}
int dl2dds_vuln(int dealer_vuln) {
   int dds_vuln = (dealer_vuln == 0 ) ? 0 :  // None
                  (dealer_vuln == 1 ) ? 2 :  // NS
                  (dealer_vuln == 2 ) ? 3 :  // EW
                  (dealer_vuln == 3 ) ? 1 :  // Both.
                    0 ;                      // assume none if invalid entry
   return dds_vuln ;
}
int dds2dl_strain(int si) {
   int dl_strain ;
   dl_strain = (si == 0 ) ?  3 :     // Spades
               (si == 1 ) ?  2 :     // Hearts
               (si == 2 ) ?  1 :     // Diamonds
               (si == 3 ) ?  0 :     // Clubs
                             4 ;     // si must equal 4 aka No Trump
   return dl_strain;
}
int dl2dds_tricks(DDSRES_k *DealerRes, struct ddTableResults *Res_20) {
               // fill the DDS Tricks Results array for use by Par calcs; need to do this when tricks comes from RPLIB
   int si, h;
   int dl_strain  ;
   for (si=0; si < DDS_STRAINS ; si++ ) {
       for (h=0 ; h < DDS_HANDS ; h++ ) {
          dl_strain = dds2dl_strain(si) ;
          Res_20->resTable[si][h] = DealerRes->tricks[h][dl_strain] ;
       } /* end for h < DDS_HANDS */
   } /* end for si < DDS_STRAINS */
   return 1 ;
} /* end dl2dds_tricks */

/* return the DD tricks for compass strain; from cache if possible, else call TrueCalcTable to fill cache */
int dds_tricks(int compass, int strain ) {  //ngen, dds_dealnum, dds_res_bin, dds_mode are globals
    int CacheState ;
    dbg_dds_res_calls++;
   JGMDPRT(5,"IN DDS_TRICKS res_calls=%d ngen=%d dds_dealnum=%d,hand[%c],strain[%c], dds_mode=%d \n",
                           dbg_dds_res_calls, ngen, dds_dealnum, "neswSW"[compass],"cdhsN"[strain], dds_mode );
   CacheState = CheckCache(&dds_res_bin, ngen, dds_dealnum, compass, strain ) ;

   JGMDPRT(5, "DDS_TRICKS CheckCache returns[%d], ngen=%d,dds_dealnum=%d, trix=%d \n",
                        CacheState, ngen, dds_dealnum, dds_res_bin.tricks[compass][strain] );
    if    (CacheState == CACHE_INV ) { /* ngen is on a new deal */
        ZeroCache( &dds_res_bin);
        dds_res_bin.CacheStatus = CACHE_INV ;
    }
    else if (CacheState == CACHE_UPD ) {
        dds_res_bin.CacheStatus = CACHE_UPD ; /* The cache is partially OK, but need to call lib anyway for new h/s combo */
    }
    if ( CacheState == CACHE_UPD || CacheState == CACHE_INV ) { /* Decide what kind of lib call to make then make it. */
           dbg_dds_lib_calls++;

      JGMDPRT(5,"IN DDS_TRICKS CacheInvalid lib_calls=%d, mode=%d, dds_dealnum=[%d]  \n",
                           dbg_dds_lib_calls, dds_mode, dds_dealnum );

        if (dds_mode == DDS_TABLE_MODE ) {
            dds_res_bin = true_CalcTable (curdeal, options.par_vuln, DDS_TABLE_MODE) ;
        }
        else if (dds_mode == DDS_BOARD_MODE) {

            dds_res_bin.tricks[compass][strain] = true_SolveBoard(curdeal, compass, strain ) ;
            JGMDPRT(5,"SOLVEBOARD for compass=%c strain=%c, returns %d trix \n",
                           "neswSW"[compass], "cdhsN"[strain], dds_res_bin.tricks[compass][strain] );
        }
        else {
            fprintf(stderr, "Cant Happen in dds_tricks. dds_mode=[%d] is Invalid!! Continuing with TableMode\n", dds_mode );
            SetDDSmode(DDS_TABLE_MODE) ;
            dds_res_bin = true_CalcTable (curdeal, options.par_vuln, DDS_TABLE_MODE) ;
        }
    }  /* end if CacheState */
      JGMDPRT(6,"IN DDS_TRICKS res_calls=%d ngen=%d dds_dealnum=%d, trix[%d,%d]=%d \n",
            dbg_dds_res_calls, ngen, dds_dealnum, compass, strain, dds_res_bin.tricks[compass][strain] );

    // assert The cache entry for dds_res_bin.tricks[compass][strain] is valid

    /* Just a normal compass, north,east,south,west. */
    /* Then return the relevant result from the cache */

    JGMDPRT(6," ngen=%d  lib_calls=%d, res_calls=%d tricks=%d, par=%d\n",
                ngen, dbg_dds_lib_calls, dbg_dds_res_calls, dds_res_bin.tricks[compass][strain], dds_res_bin.parScore_NS );

   JGMDPRT(6,"DDS tricks RETURNS dds_res_bin[%d][%d] = %d ******* \n",
                        compass, strain, dds_res_bin.tricks[compass][strain] );
       return dds_res_bin.tricks[compass][strain];
} /* end dds_tricks */

/* Use ParResultsMaster to put Contract strings in buffer, typically in DDSRes struct member */
char *dds_ParContract(char *buff,  int dl52_vul, struct parResultsMaster *sidesRes ) { /*buff should point to area in dds_res_bin */
	int nch ;
	int buff_sz = sizeof(dds_res_bin.ParContracts[0]) ; 
	char Dbled  ;
	if(sidesRes->contracts[0].underTricks > 0 ) { Dbled = '*' ; }
   else { Dbled = ' ' ; }
   /* Example [ 4H* by NS] */
   nch = snprintf( buff, buff_sz, "%2d%c%c by %s %s", sidesRes->contracts[0].level,
                       Denom[sidesRes->contracts[0].denom], Dbled, seatNames[sidesRes->contracts[0].seats], dl52Vul[dl52_vul] );
   JGMDPRT(8,"Dl52Vuln[%d] = %s, undertricks=%d, ParContract[%s], nch=%d\n",
				dl52_vul, dl52Vul[dl52_vul], sidesRes[0].contracts[0].underTricks, buff, nch ) ;
	return (nch + buff) ;  /* points to NULL snprintf put in buffer */
} /* end dds_ParContract */
	
/* 2025-07-13 Refactor to put all the Par Score and Par Contract stuff in a separate routine we can also use in RPLIB mode */

DDSRES_k true_CalcTable(DEAL52_k  dl, int par_vul, int dds_mode ) {  // Mode should always be DDS_TABLE_MODE here...
    int dds_rc = 1; /* OK*/
    int si, so, h, t;
    struct ddTableResults    Res_20;    /* 20 Results */
    struct ddTableDeal       dds_BIN_deal; /* Deal in DDS binary suitable for input to CalcDDtable */
    DDSRES_k DealerRes ;
    char line[128] ; 				// for DDS error messages
    int  dds_vuln   ;
    /* convert dealer coding par_vul argument  to DDS coding */
    dds_vuln =  (par_vul == 0 ) ? 0 :      // None
                (par_vul == 1 ) ? 2 :      // NS
                (par_vul == 2 ) ? 3 :      // EW
                (par_vul == 3 ) ? 1 :      // Both.
                0 ;                         // assume none if invalid entry

    // dbg_dds_lib_calls and dbg_dds_res_calls updated by dds_tricks before calling this function

   JGMDPRT(7,"IN trueCalcTable dbg_dds_lib_calls=%d, ngen=%d, dealnum=%d jgmDebug=%d, dds_mode=%d, dds_vuln=%d\n",
                                  dbg_dds_lib_calls, ngen, dds_dealnum,  jgmDebug,   dds_mode,     dds_vuln);
   dds_BIN_deal =  Deal52_to_DDSBIN (dl);  /* convert a deal in dl52 fmt to internal DDS fmt */
   dds_rc = CalcDDtable(dds_BIN_deal, &Res_20 );
   JGMDPRT(7, " DDS CalcDDtable returned with RetCode=%d \n", dds_rc);

        // check return code; print error msg if not OK  NO_FAULT is 1
        if (dds_rc != RETURN_NO_FAULT)  {
            ErrorMessage(dds_rc, line);
            fprintf(stderr, "Table Mode DDS error: %s\n", line);
            strncpy(DealerRes.ddsmsg, line, 40);
            DealerRes.errcode = -1 ;
        }
        DBGDO(7,showRawResults(&Res_20) );
        // successful DDS call. Fill the Dealer Results struct and return it
        DealerRes.errcode = RETURN_NO_FAULT ; // success
            // fill the tricks results array
        for (si=0; si < DDS_STRAINS ; si++ ) {
            for (h=0 ; h < DDS_HANDS ; h++ ) {
                t = Res_20.resTable[si][h]; // Tricks
                so =(si == 0 ) ?  3 :     // Spades
                    (si == 1 ) ?  2 :     // Hearts
                    (si == 2 ) ?  1 :     // Diamonds
                    (si == 3 ) ?  0 :     // Clubs
                                  4 ;     // si must equal 4 aka No Trump
                DealerRes.tricks[h][so] = t ;
            } /* end for h < DDS_HANDS */
        } /* end for si < DDS_STRAINS */

        dds_rc = ddsParCalcs( &Res_20,  &DealerRes ) ; /* Use Res20 to calc the Par scores and contracts and put into DealerRes */
        
        DealerRes.CacheStatus = CACHE_OK ;
        dds_dealnum = ngen ; /* mark the cache as up to date. */
        
        DBGDO(7, showReturns(&DealerRes) );

        JGMDPRT(7,"Done trueCalcTable: North Spades Tricks=%d, parScore_NS=%d \n ",  DealerRes.tricks[0][3], DealerRes.parScore_NS );

        return DealerRes ;  /* returns the whole structure */
} /* end true_CalcTable*/
/* end true_CalcTable */
void init_DDS_deal_st (struct deal_st *dl_ptr) { /* deal_st is what is passed to the DDS solveOneBoard routine */
    int h, s ;
    dl_ptr->first = 0 ;
    dl_ptr->trump = 0 ;
    for (h=0 ; h<3 ; h++ ) {
        dl_ptr->currentTrickSuit[h] = 0;
        dl_ptr->currentTrickRank[h] = 0;
    }
    for (h=0 ; h<4 ; h++ ) {
        for (s = 0; s < 4 ; s++ ) {
            dl_ptr->remainCards[h][s] = 0 ;
        }
    }
} /* end init_DDS_deal_st */

// true_SolveBoard called with Dealer encoding for strain where 0=clubs and 3=spades, 4=NT
int true_SolveBoard (DEAL52_k  curdeal, int h, int s ) {
struct futureTricks        fut;
struct deal_st              dl;
int rc, trix ;
int lho[4]={1,2,3,0} ; // The lho[compass] is the opening leader we pass to SolveOneBoard
int dds_strain[5]={3,2,1,0,4} ; /* translate from Dealer stain to DDS strain; Clubs=3, Spades=0 NT = 4*/
char line[120] ;
// dbg_dds_lib_calls and dbg_dds_res_calls updated by dds_tricks before calling this function
// memset( &dl, 0 , sizeof(dl) ); /* set the whole struct to zeroes */
  init_DDS_deal_st ( &dl ) ;

 #ifdef JGMDBG
    if (jgmDebug >= 7) {
         fprintf( stderr, "true_SolveBoard.295 dl struct set to zero.ngen=%d, trump=[%d], first=[%d], dds_mode=%d\n",
                     ngen, dl.trump, dl.first, dds_mode ) ;
    }
#endif
  rc =  Deal52_to_Holding(curdeal, dl.remainCards);
 #ifdef JGMDBG
    if (jgmDebug >= 8) {
        fprintf(stderr, "true_SolveBoard.302 Deal52_To_Holding Done. Calling dump_Deal\n");
        dump_Deal(dl)  ;
    }
#endif

//syntax dl.trump dl.first dl.remainCards[Compass][Suit] dl.currentTrickSuit[3] dl.currentTrickRank[3]=0,
 dl.first = lho[h];  /* solveOneboard results depend on who is on lead. CalcDDtable results dont */
 dl.trump = dds_strain[s] ;  /* 0=Clubs, 3=Spades, 4=NT */
 #ifdef JGMDBG
    if (jgmDebug >= 6) {
        fprintf(stderr, "true_SolveBoard.312 Calling Solveboard ,Dealer[trump=%d, Decl=%d] DDS[ trump=%d, First=%d] ,dbg_dds_lib_calls=%d\n",
              s,h, dl.trump, dl.first, dbg_dds_lib_calls );
    }
#endif
 rc = SolveBoard(dl, -1, 1, 0, &fut, 0 ) ;
 if (rc != RETURN_NO_FAULT) {
    ErrorMessage(rc, line);  // convert the numeric err msg to text in the char array line[]
    fprintf(stderr, "DDS error [%d] in true_SolveBoard\n",rc );
    assert(0);
 }
 dds_dealnum = ngen;
 dds_res_bin.CacheStatus = CACHE_OK ;
 trix = 13 - fut.score[0];  /* since we have calc the trix for our LHO our result is 13 - his */
 #ifdef JGMDBG
    if (jgmDebug >= 7) {
        fprintf(stderr, "true_SolveBoard.326 tricks=[%d , %d], ngen=[%d], dds_dealnum=[%d], dds_lib_calls=[%d], dds_res_calls=[%d]\n",
                                       fut.score[0], trix, ngen,dds_dealnum,dbg_dds_lib_calls, dbg_dds_res_calls ) ;
    }
 #endif
 return trix ;
} /* End true_SolveBoard */
/* End true_SolveBoard */

int dds_parscore(int compass, int vuln ) {  /* uses globals  Returns Par Score, not par Contract */
    int CacheState;
    int par_NS ;
    int vuln_par = 0;   /* nobody vul either coding */
    dbg_parscore_calls++;
    JGMDPRT(6,"IN DDS_PARSCORE par_calls=%d ngen=%d dds_dealnum=%d, dds_mode=%d, compass=%d, vuln=%d DefVuln=%d \n",
                           dbg_parscore_calls, ngen, dds_dealnum, dds_mode, compass, vuln,options.par_vuln );
    vuln_par = (vuln     >= 0 ) ? vuln :				          /* vul in input file takes priority */
               (options.par_vuln >= 0 ) ? options.par_vuln : /* use vul set on cmd line with -P switch if not set will be -1*/
               0 ; 											          /* default value */
 
    if ( 1 == zrdlib_mode ) {
       par_NS =  rplib_Par(&dds_res_bin , vuln_par) ; /* these globals will have been set by get_rp_deal and parsing stage */
       if(compass == COMPASS_NORTH || compass == COMPASS_SOUTH || compass == SIDE_NS ) {
          return par_NS ;
       }
       else return -(par_NS) ;
    } /* end rplib mode */


    if (dds_mode != DDS_TABLE_MODE) {
        fprintf(stderr, "In ParScore:: Using Parscore requires TableMode. Setting Mode and continuing..\n");
        SetDDSmode(DDS_TABLE_MODE) ;
        dds_dealnum = -1 ; //force a call to fill the Cache
    }
    CacheState = CheckCache(&dds_res_bin, ngen, dds_dealnum , 0, 0 ) ;
    if    (CacheState == CACHE_INV || CacheState == CACHE_UPD ) {
        dds_res_bin.CacheStatus = CACHE_INV ;
        dbg_dds_lib_calls++;
        dds_res_bin = true_CalcTable (curdeal, vuln_par, DDS_TABLE_MODE) ;
    }

   JGMDPRT(6,"Leaving DDS_PARSCORE dbg_dds_lib_calls=%d, par=%d\n", dbg_dds_lib_calls,dds_res_bin.parScore_NS  );

    /* at this point the global cache of 20 results + 4 par scores and 4 Par contracts  is valid */
     if(compass == COMPASS_NORTH || compass == COMPASS_SOUTH || compass == SIDE_NS ) {
         return dds_res_bin.parScores_NS[vuln_par] ;
     }
     else return -(dds_res_bin.parScores_NS[vuln_par]) ;
} /* end dds_parscore */

/* csv_trix will return tricks for all 5 strains,  for any number of hands 1-4 parscores not relevant but calc'ed anyway*/
int csv_trix( char *buff, int h_mask ) {  //ngen, dds_dealnum, dds_res_bin, dds_mode are globals
    int CacheState ;
    int i, h, s, nch ;

    dbg_dds_res_calls++;
 #ifdef JGMDBG
    if (jgmDebug >= 5 ) {
       fprintf(stderr, "IN csv_trix res_calls=%d ngen=%d dds_dealnum=%d,h_mask=%d, dds_mode=%d \n",
                           dbg_dds_res_calls, ngen, dds_dealnum, h_mask, dds_mode ); }
 #endif
     if (dds_mode != DDS_TABLE_MODE) {  /* this will only happen once */
        fprintf(stderr, "In csv_trix:: Using csv_trix requires TableMode. Setting Mode and continuing..\n");
        SetDDSmode(DDS_TABLE_MODE) ;
        dds_dealnum = -1 ; //force a call to fill the Cache
    }
    CacheState = CheckCache(&dds_res_bin, ngen, dds_dealnum, COMPASS_SOUTH, SUIT_NT ) ; /* in mode 2 any combo will do */
 #ifdef JGMDBG
    if (jgmDebug >= 5 ) {
       fprintf(stderr, "csv_trix: CheckCache returns[%d], ngen=%d,dds_dealnum=%d, trix=%d \n",
                        CacheState, ngen, dds_dealnum, dds_res_bin.tricks[0][0] );
    }
 #endif

    if (CacheState != CACHE_OK ) { /* ngen is on a new deal */
        ZeroCache( &dds_res_bin);
        dds_res_bin.CacheStatus = CACHE_INV ;
        dbg_dds_lib_calls++;

         JGMDPRT(6,"IN csv_trix:: CacheInvalid lib_calls=%d, mode=%d, dds_dealnum=[%d]  \n",
                           dbg_dds_lib_calls, dds_mode, dds_dealnum );

        dds_res_bin = true_CalcTable (curdeal, options.par_vuln, DDS_TABLE_MODE) ;
     }  /* end if CacheState */

    JGMDPRT(6,"IN csv_trix:: true_CalcTable Done. res_calls=%d ngen=%d dds_dealnum=%d \n",
                           dbg_dds_res_calls, ngen, dds_dealnum );

    // assert The cache entry for dds_res_bin.tricks[compass][strain] is valid


      JGMDPRT(6,"Leaving csv_trix. ngen=%d  lib_calls=%d, res_calls=%d tricks[0][0]=%d, par=%d\n",
                           ngen, dbg_dds_lib_calls, dbg_dds_res_calls, dds_res_bin.tricks[0][0], dds_res_bin.parScore_NS);

      // now format the buffer giving trix in all 5 strains for each hand asked for.
      nch = 0 ;
      for(i = 0 ; i < 4 ; i++ ) {
         h = (i + csv_firsthand ) & 3 ;
         if ( !(h_mask & 1 << h) ) { continue ; }
         JGMDPRT(6, "hand=%d, ",h);
         for (s=0; s < 5 ; s++ ) { // for each strain clubs to NT
            nch += sprintf(buff+nch, "%d,", dds_res_bin.tricks[h][s] ) ;
            JGMDPRT(6, "s=%d, tricks=%d, nch=%d, ",s,dds_res_bin.tricks[h][s], nch );
         } /* end for s */
         JGMDPRT(6,"\n");  // begin next hand debug on new line
      } /* end for i  aka hand */
         buff[nch-1] = '\0' ; // replace last comma with null terminator
         return nch-1 ;       // return length of the trix buffer contents

} /* end csv_trix */

int rplib_Par(DDSRES_k *DealerRes, int vuln_par) {
    int  dds_rc = 1; /* OK*/
    char line[128];
    int  dds_vuln;

    struct ddTableResults    Res_20;    /* 20 Results */
    
   dds_vuln = dl2dds_vuln(vuln_par) ;
   dl2dds_tricks(DealerRes, &Res_20) ;           /* copy DealerRes.tricks to Res_20 in the right order . */
   dds_rc = ddsParCalcs( &Res_20, DealerRes ) ;  /* Fill the Par scores and contracts in DealerRes, given the 20 Tricks results */
            // check return code; print error msg if not OK
   
   if (dds_rc != RETURN_NO_FAULT)  {
       ErrorMessage(dds_rc, line);
       fprintf(stderr, "Table Mode DDS error: %s\n", line);
       strncpy(DealerRes->ddsmsg, line, 40);
        DealerRes->errcode = -1 ;
    }
    // successful DDS call. Fill the Dealer Results struct and return it
    DealerRes->errcode = RETURN_NO_FAULT ; // success
    DealerRes->CacheStatus = CACHE_OK ;
    
    dds_dealnum = ngen ; /* mark the cache as up to date. */
    return DealerRes->parScores_NS[vuln_par] ;
} /* end  rplib_Par */

 /* Starting from the 20 results in DDS format, calculate the Par Scores and Contracts for all Vulnerabilities; save in DealerRes */
int ddsParCalcs( struct ddTableResults  *pRes_20,   DDSRES_k *pDealerRes )  {
      struct parResultsMaster  sidesRes[2] ;
      int dds_vuln ;
      int  dl52_vuln ;
      int dds_rc = RETURN_NO_FAULT ;
      char *buff_ptr; 
      char line[128] ; 				// for DDS error messages 
      int t;
      
        /* Calculate the Par for all VUL. Does not take long, and simplifies caching */
        for (dds_vuln = 0 ; dds_vuln < 4 ; dds_vuln++) {
			   dl52_vuln = dds2dl_vuln(dds_vuln) ;
				dds_rc    = SidesParBin(pRes_20, sidesRes, dds_vuln);  /* fill sidesRes with Par results */
            // check return code; print error msg if not OK
				if (dds_rc != RETURN_NO_FAULT)  {
						ErrorMessage(dds_rc, line);
						fprintf(stderr, "Table Mode DDS error from SidesParBin: %s\n", line);
						strncpy(pDealerRes->ddsmsg, line, 40);
						pDealerRes->errcode = -1 ;
				}
				pDealerRes->parScores_NS[dl52_vuln] = sidesRes[0].score;  /* Set the NS Par Score */
				buff_ptr = dds_ParContract(pDealerRes->ParContracts[dl52_vuln], dl52_vuln, &sidesRes[0] ) ;  /* fill the contract strings */
				JGMDPRT(8,"Vuln = %d, Score=%d, , undertricks=%d, ParContract[%s]\n",dl52_vuln,
					pDealerRes->parScores_NS[dl52_vuln],sidesRes[0].contracts[0].underTricks, pDealerRes->ParContracts[dl52_vuln]) ; 
		  } /* end for dds_vuln */
          // set default par score for backwards compatibility
        t = (options.par_vuln >= 0 ) ? options.par_vuln : 0  ;/* use vul set on cmd line with -P switch if not set will be -1*/
       pDealerRes->parScore_NS = pDealerRes->parScores_NS[t] ; // set the default one backwards compat

		  JGMDPRT(7, " DefParScore(NoneVul)=%d, 4xParScores Calc=[%d,%d,%d,%d]\n",
						pDealerRes->parScore_NS,pDealerRes->parScores_NS[0],pDealerRes->parScores_NS[1],
						                        pDealerRes->parScores_NS[2],pDealerRes->parScores_NS[3] );
      return dds_rc ; 
}


/* One Time Setup of the DDS environment. Not used by Dealer at present; allows for dds_subs to be made into a lib module */
void setup_DDS_env(DDSRES_k *dds_res_bin, int mode, struct deal_st *deal_st_ptr ) {
	init_DDS_deal_st(deal_st_ptr) ; 
	ZeroCache(dds_res_bin) ; 
	SetDDSmode(mode);
	return ; 
}
	


