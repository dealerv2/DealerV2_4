/* This is file dealglobals.c -- defines global vars and allocates storage for them  */
/* Uses types, tags, and symbolic constants defined in other header files */
/* 2022-02-27 -- Mods for Francois Dellacherie enhanced shapes */
/* 2022-10-18 Mods for Has_card and future user_eval functionlity */
/* 2023/01/07 -- Merged in changes from V4 to fix predeal; dealcards_subs.c and globals, etc. */
/* 2023/10/27 --  Deleted references to lcrep, and the -u cmd line switch. only ucrep is ever used. */
/* 2023/11/03 -- Deleted some variables that have been moved to their own .h file in anticipation of library modules */

#ifndef DEALGLOBALS_C
#define DEALGLOBALS_C
#ifndef _GNU_SOURCE
  #define _GNU_SOURCE
#endif
#include "std_hdrs.h"
#include "dealtypes.h"          /* some type defs and struct tags needed below. Will include dealdefs.h also */
#include "dealer_paths.h"       /* #defines for dealer directories and program names, expressed as strings */

int jgmDPRT = 0 ;				/* cant ifdef it bec some modules may compile with JGMDBG off and some on. */
int DealMode = DEF_MODE ;  /* 2023-12-10 Re-Org for Bias Deal */

/* Dealer Variables not related to Flex or Yacc */
int nprod = 0 ;
int ngen  = 40;
int deal_err = 0 ;    /* -ve if we want to reject the deal. So far only bias deal returns -1 if can't satisfy*/

char *crlf = "\n";				/* For windows this would be \r\n but this program no longer supports windows */
char card_id[13]    = {'2','3','4','5','6','7','8','9','T','J','Q','K','A'};
char rank_id[]      =  "23456789TJQKA";
char strain_id[6]   = {'C','D','H','S','N', 'L' };  /* the L is for longest fit used by DOP calculations */
char seat_id[4]     = {'N','E','S','W'};
char *player_name[] = {"North", "East", "South", "West" };
char *suit_name[]   = {"Club", "Diamond", "Heart", "Spade"};
char suit_id[5][3]   = {"C:", "D:", "H:", "S:", "N:"};  /* never used */
char side_seat[2][2]= { {'N','S'} , {'E','W'} };

char   ucrep[] = "23456789TJQKA"; /*deleted lcrep and associated -u switch since it was never used. */
struct handstat    hs[4] ;
struct sidestat_st ss[2] ; 
const int beg_pos[4] = {0 ,13,26,39} ;  /* the slots in the deck where the suits or hands begin */
const int end_pos[4] = {13,26,39,52};   /* the slots in the deck where the suits or hands end +1 -- for use in calls to get_rnd_slot among other things */

	/* Constants for Evaluation and Scoring */
const int imp_tbl[24] = { 10,   40,   80,  120,  160,  210,  260,  310,  360,  410,  490,  590,
                   740,  890, 1090, 1190, 1490, 1740, 1990, 2240, 2490, 2990, 3490, 3990  };
 /* Cmd line may set options Will override values from input file if set */
struct options_st options = {0};        /* C99 supposed to set according to type?*/
struct options_st *p_opts = &options ;
struct param_st parm = {0};
int    csv_firsthand = COMPASS_NORTH ;
char   csv_trixbuff[64] = {0} ; // room for (20 compass-strain-combos) * (2digits + comma) and a bit extra
size_t csv_trixbuff_len = 0 ;

/* original cmd line switches -- many also appear in a yyparse action clause. --   would be nicer to put these all in a struct */

int  maxgenerate = 0 ;          /* -g: */  /* flex action clause Must be zero for Flex to process the input file value*/
int  progressmeter = 0;         /* -m */   /* this is a toggle option */
int  Opener = COMPASS_WEST;     /* -O: */  /* flex action clause  0=north (or east) 1=east(or north) 2=south(or west) 3=west(or south) */
char opc_opener = 'W' ;                   /* define one so that extern will be happy. Dont want to extern a struct memb*/
int  maxproduce = 0     ;       /* -p: */  /* flex action clause Init value MUST be zero for Flex to process the input file value*/
int  quiet = 0 ;                /* -q */   /* option for pbn printout */
long seed  = 0 ;                /* -s: */ /* seed can now be set in Input File; -s also used to set LIB file start point */
long seed_provided = -1  ;      
int verbose = 1;                /* -v */   /* turn off end of run stats. Default is to print them */
int swapping = 0 ;              /* -x [0|2|3] Zero turns off swapping, 2 swap E/W, 3 swap all combos of (E,S,W) */
int swapindex = 0;				  /*            Used by swapping routine to decide on a new deal */
int errflg = 0;

/* JGM Added the following cmd line options .. Also the Opener O flag above */
int jgmDebug = 0;       /* -D 1 .. 9; level of verbosity in fprintf(stderr, ) statements. 0 = No Debug (unless defined JGMDEBUG) */
int srvDebug = 0;       /* -D has a decimal digit 0..9 e.g. -D 1.6 (dealer=1, server=6), or -D .9 (dealer = 0, server = 9) */
int dds_mode = 1;       /* -M 1 use Board Mode fastest for 1-5 results; 2 Use Table Mode, fastest for 5-20 results*/
int TblModeThreads = 8; /*    Number of threads to use when user asks fto indicate if any title is wanted. A zero title is valid toor -M2 unless overridden by -R */
/* If we need to force Table Mode on DDS for e.g. Par Calcs, or just via -M switch, then also force extra threads. */

char title[MAXTITLE+1]= ""; /* -T title. Usually in quotes which are removed by getopt flex action clause */
int title_len = -1 ;		  /* >0 valid title;  <0 no title specified =0 suppress zrdhdr record(s) even if title in dli file*/

char *input_file = '\0';
FILE *fexp;      /* -X file for exporting to; Normally NOT left as stdout except for testing */
FILE *fcsv;      /* -C file for csvreport. Open in append mode unless user puts w:filename */
FILE *fzrd;      /* -Z [Nw:]filename Save produced deals for future use. Put N:filename if No DDS tricks wanted; default is tricks in all 20 possible contracts */
FILE *fzrdlib;   /* -L zrd Library file. Default is ../dat/rpLib.zrd */
FILE *flog;      /* -l [Nw:]filename. Save produced deals in Deal52 fmt.Put N:filename if No DDS tricks wanted; default is tricks in all 20 possible contracts */ 


char zrdlib_default[64] = ZRD_LIB; /* /usr/local/games/DealerV2_4/dat/rpLib.zrd or /home/myuser/DealerV2_4/dat/rpLib.zrd */
int    zrdlib_mode= 0 ;              /* 0= Not using RP Lib file; 1= Using RP Lib file; affects swapping, predeal, seed, */
int    zrdlib_blksz = ZRD_BLOCKSIZE ; /* will be adjusted based on DB file size */
int    zrdlib_recs  = ZRD_MAX_LIBRECS; /* will be calculated at run time */
int    zrdlib_recnum = 0 ;
int    zrd_max_seed = ZRD_MAX_SEED ;
int    zrd_cnt      = 0 ;
int    zrdlib_pass_num = 0 ;

/* -U Path name for the UserEval binary. Default is 'DealerServer' in the current directory. Can be set by -U cmd line parm
 * -U ./DealerServer or ../Prod/DealerServer or ../lib/DealerServer or ../Debug/DealSrvdbg or ../lib/DealerSrvdbg
 * Instead: /usr/local/games/DealerV2/UserEval/DealerServer  or /home/MyUser/MyDir/MySubDir/MyUserPgm
 */
 char server_pgm[64]   = SERVER_PGM ; /* In the current directory. or user sets path name via -U switch */

/* char server_dir[SERVER_PATH_SIZE+1]  = "/usr/local/games/DealerV2_4/bin";  */           
/* char server_path[SERVER_PATH_SIZE+1] = "/usr/local/games/DealerV2_4/UserEval/DealerServer"; */

char server_dir[SERVER_PATH_SIZE+1]  = UEV_DIR ;     //   /usr/local/games/DealerV2_4/UserEval
char server_path[SERVER_PATH_SIZE+1] = SERVER_PGM;   //   /usr/local/games/DealerV2_4/bin/DealerServer
pid_t userserver_pid = 0 ;

/* OPC Related vars. Maybe they don't need to be globals */
char opc_cmd_buff[128] = OPC_PGM; // /usr/local/games/DealerV2_4/bin/dop
char opc_pgm_path[64]  = OPC_PGM;
int  opc_pgmlen = 35 ;
int  opc_dealnum = -1;   /* -1 will force the opc cache to be updated the first time opc() is called */
struct opc_Vals_st opcRes ;
/* Exporting hands for future import via -N, -E, -S, -W switches 
 * Ex: -N SAT8542,H984,DQ5,CT7  -S SKQ97,HJ3,DKJ972,CK2
 * The above is 49 chars for 26 cards. In theory you could predeal another 24 cards for 47 chars so a total of 96 chars.
 */
 char       export_buff[128] ; /* Max (never happen) predeal export is 96 chars. if doing all 4 hands which is not supported yet*/
 char       zrd_default[64] = "../dealLib.zrd"  ;  /* The default outfile name */

	/* Decks for Shuffling, dealing, predealing and bias dealing */
 DEAL52_k  *deallist = NULL;	/* ptr to malloc'ed array of deals to be printed at end of run */
 DEAL52_k  asc_pack;    		/* pack in order C2 up to SA used as a convenient source for Bias Deals*/
 DEAL52_k  curdeal;		
 DEAL52_k  fullpack;    		/* pack in order SA down to C2 */
 DEAL52_k  stacked_pack; 		/* deck with normal predeal cards in it */
 DEAL52_k  small_pack;       	/* 2023-01-05 cards left after normal predeal done */
int  small_size   = 52 ;  		/* number of cards left after predeal or bias deal done; used in Shuffle */
int  stacked_size = 0  ;		/* number of non-bias cards predealt set by yyparse and cmdline parms*/
int  full_size    = 52 ;		/* number of cards left in full pack after predealt cards removed set by yyparse*/

/* biasdeal never implemented in original dealer. JGM implemented in November 2023 It's complicated! */  
int bias_suits[4][4] = { {-1, -1, -1, -1}, {-1, -1, -1, -1},  /* these are -1 because we might want to bias with a void */
                       {-1, -1, -1, -1}, {-1, -1, -1, -1} };  /* MACRO TRUNCZ(x) returns zero if x<0, x otherwise */
int bias_deal_wanted = 0;  			/* set by yyparse along with bias_suits[h][s], ref'd by shuffle and deal code */

/* RANKMASK not used; would be used to create a DEAL64_k or DEAL32_k deal, 
 * using  zero as the rank of a deuce, compatible with rest of Dealer. 
 * Similar to DDS binary deal, but DDS gives deuce a rank of Two.
 */
// Masks to set rank bits 0..12       deuce     trey    four     five    six      7       8       9
unsigned int    RANKMASK[16] = {     0x0001,   0x0002, 0x0004, 0x0008, 0x0010, 0x0020, 0x0040, 0x0080,
// Masks to set rank bits contd           T        J       Q       K       A      bit13   bit14   bit15
                                      0x0100,   0x0200, 0x0400, 0x0800, 0x1000, 0x2000, 0x4000, 0x8000} ;

/* Various handshapes can be asked for. For every shape the user is
   interested in a number (shapeno) is generated. In every distribution that fits that
   shape the corresponding bit is set in the distrbitmaps 4-dimensional array.
   This makes looking up a shape a small constant cost.
*/
int ***distrbitmaps[14];	/* if we made these long ints then we could have 64 shape statements instead of 32 */

	/* Default Values for First Nodes in the Trees created by yyparse */
struct tree    defaulttree = {TRT_NUMBER, NIL, NIL, 1, 0};
struct tree   *decisiontree = &defaulttree;
struct action  defaultaction = {   /* next-ptr     type       expr1            expr2             int1   str1      */
                    (struct action *) 0,     ACT_PRINTALL, (struct tree *) 0, (struct tree *) 0 , 0 , (char *) 0
} ;
 // end defaultaction intializer list
struct action *actionlist = &defaultaction;
struct var    *vars = 0 ;

        /* Vars used by code in parser action statements init them for safety */
int will_print = 0;
int maxdealer = -1;           /* set to a value to force yyparse to fill it in if needed */
int maxvuln = -1;             /* Ditto. Both Strictly descriptive in printpbn routine. Seldom used */
int userserver_reqd =  0 ;    /* will set this if we see a usereval statement in the input file */
int predeal_compass = -1 ;    /* global variable for predeal communication */
int shapeno = 0;              /* Count number of shape statements. 0-31. Defines bit in bit mask */
int use_compass[NSEATS] = {0,0,0,0};  /* skip analysis if compass never used */
int use_side[2] = {0,0};     	/* opc and usereval use this. will also cause related use_compass'es to be set */
int dds_dealnum = -1 ;       	/* -1 no DDS needed; 0 DDS needed; >0 ngen number of last fetch */
int alt_tbl_idx = -1 ;        /* Global var set by yyparse()  to track which altcount is being modified */
int pointcount_index;         /* Global var set by yyparse()  to track which rank is being modified */

/* from deuce to Ace  -- (weight of a void is 128) allows stiffs and dblton honors to all have unique value.*/
/*                      2, 3, 4, 5, 6, 7, 8, 9, T, J, Q,   K,  A   */
int ltc_weights[13] = { 1, 1, 1, 1, 1, 1, 1, 1, 4, 8, 16, 32, 64 }; /* may be superceded by CardAttr_RO  below Void=128 */
int points[13]      = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,  2,  3,  4 }; /* Goren HCP values */
/* the pointcount array itself */

int tblPointcount [idxEnd][13] = {
    /* tables tens to c13 MUST be in this order to make sense to the user.
     * Put HCP at very end since it has its own routines to handle it.
     * Order of other tables does not matter; they are unlikely to be changed; Controls and LTC weights are pretty fixed
     */
 /* 2  3  4  5  6  7  8  9  T  J  Q  K  A */
 {  0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0}, /* tens = pt0 so idx must be 0 */
 {  0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0}, /* jacks */
 {  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0}, /* queens */
 {  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0}, /* kings */
 {  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1}, /* aces */
 {  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1}, /* top2 */
 {  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1}, /* top3 */
 {  0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1}, /* top4 */
 {  0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1}, /* top5 */
 {  0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 4, 6}, /* c13 = pt9 idx=9 */
 {  0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4}, /* HCP idx=(idxEnd - 1); At end since it has its own zero etc rtn*/
} ; /* End tblPointCount */

int CardAttr_RO [idxEndRO][13] = { /* Values Not changeable by user via altcount or pointcount cmd */
 /* 2  3  4  5  6  7  8  9  T   J   Q   K   A */
 {  0, 0, 0, 0, 0, 0, 0, 0, 0,  0,  0,  1,  2},  /* controls idxControls = 0 */
 {  1, 1, 1, 1, 1, 1, 1, 1, 4,  8, 16, 32, 64},  /* ltc weights. idxLTCwts.  Will ID WHICH of the top cards we have. */
 {  0, 0, 0, 0, 0, 0, 0, 0, 0,  2,  5,  9, 13},  /* Kleinman Pts idxKleinman Need to add 1 synergy pt to a suit with A or K and 1 other*/
 {  0, 0, 0, 0, 0, 0, 0, 0, 25,75,150,300,450},  /* BumWrap Points x 100 idxBumWrap */
} ; /* End CardAttr_RO */


 /* some debugging stuff */
int treelev = 0;            /* the level we are at in the decision tree */
int showtree  = 1; 			/* default to show it if in DBG mode */
int treedepth = 0; 			/* in case we want to debug the tree walking */
int num_trees = 0 ;         /* running count of  trees allocated so far */
struct treeptr_st tree_ptrs[100] ;  /* JGM?+ an array to store pointers for tracing and debugging. */

int dbg_dds_lib_calls = 0;
int dbg_dds_res_calls = 0;
int dbg_parscore_calls =0;
int dbg_tdd_calls = 0;
int dbg_dd_calls = 0;
int dbg_opc_calls = 0;
int dbg_opc_cmd_calls = 0;

int dbg_userserver_extcalls = 0 ;
int dbg_userserver_askquery = 0 ;


/* JGM Debugging and learning about Dealer use of Trees */
struct treeptr_st tree_ptrs[100] ;      /* JGM?+ an array to store pointers for tracing and debugging. */

#endif /* ifndef DEALGLOBALS_C */



