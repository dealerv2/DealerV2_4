/* Roth Per Book: Modern Bridge Bidding Complete by Roth & Rubens -- 1968 */
#include "../include/std_hdrs_all.h"
#include "../include/UserEval_types.h"
#include "../include/UserEval_externs.h"
#include "../include/UserEval_protos.h"
#include "../include/dbgprt_macros.h"
/*
static int jgmDPRT = 3 ; 
extern int jgmDebug ; 
#undef JGMDPRT
#define JGMDPRT(l,fmt,...) do {if (jgmDPRT >= (l)) { fprintf(stderr, "%s:%d " fmt, __FILE__,__LINE__,## __VA_ARGS__) ; } } while(0)
#undef DBGDO
#define DBGDO(l,...) do { if(jgmDPRT >= (l) ) { (__VA_ARGS__) ; } } while (0)  // call an arbitrary subroutine ...
*/
#define ISMAJOR(s) ( (s) >= HEARTS  ) ? 1 : 0
#define ISMINOR(s) ( (s) <= DIAMONDS) ? 1 : 0
#define WEAKER(x,y) ( (x) < (y) ) ? (x) : (y)
#define LONGER(x,y) ( (x) > (y) ) ? (x) : (y) 
enum RothSuitType_ek { RS_NONE=0 , RS_WKMAJ, RS_GOOD, RS_RELBL, RS_SSS, RS_NA=-1 };
struct hsuit_st { /*properties of a suit considering one hand only */
	int s_type ;	 /* If SSS keep Dpts and 2*Lpts; no extra FN; if RSS keep Dpts (Lpts?) */
	int s_hcp   ;	 /* unadjusted; use for NT */
	int s_hcp_adj;  /* for Roth in suit contracts only; for most always */
	int s_len   ;
	int s_Dpts  ;   /* 3/2/1 */
	int s_Lpts  ;   /* For suit contracts only. vanilla no SSS adjustment yet */
	int s_LptsNT;   /* For NT contracts only GOOD or better suits count */
	int s_FN_pts;   /* +1 for each card >4 in a 'supported suit'. One suit only */
	int s_FN_ptsNT; /* NT FN pts can be one suit in each hand, but suit must be RS_GOOD */
	int s_Dfit_pts; /* use these in final total. For Decl Dfit_pts == Dpts. For Dummy Dfit_pts == upgraded Dpts in all suits */
} ;

struct hand_st {  /* typically the total of the suit attributes, plus a couple of extra things */
	int h_isdecl  ;     /* 1=Declarer; In Suit gets Dpts, Lpts, FN pts, HCP. Else Dfit Pts and HCP. No L, No FN */
	int h_type ;		 /* RS_NONE RS_WKMAJ RS_GOOD RS_RELBL RS_SSS */
	int h_bal   ;		  /* 1 = bal; ie 4333, or 4432, or 5332 per Roth Strict rules */
	int h_Aces  ;
	int h_hcp   ;
	int h_hcp_adj;
	int h_Aces_pt;
	int h_Dpts  ;
	int h_Lpts  ;
	int h_LptsNT;		/* might also put the NT FN Lpt for a 'supported' GOOD 5 card suit here */
	int h_FN_pts ;
	int h_FN_ptsNT;    
	int h_Dfit_pts;
	int h_ptsNT;
	int h_ptsSuit;	
	int h_fitsuit   ;  /* preferred trump suit; usually longest fit */
} ;


int isBalanced(HANDSTAT_k *phs ) ;
int dsort13 (char a[13] ) ;
void prolog ( int side ) ;
void cpy_to_trumpfit(TRUMP_FIT_k *ptrump , SIDE_FIT_k *psf ) ;

int rothSuitType(	HANDSTAT_k *phs, int  s ) ;
int DptsROTH(    	HANDSTAT_k *phs, int  s ) ;
int LptsROTH(    	HANDSTAT_k *phs, int  s, int stype) ;		
int DfitROTH(    	SIDE_FIT_k *psf ); /* new code side_stat has all info we need for this */
int Fn_pts_2ndfit(int t_suit, struct hsuit_st rsuit[2][4] ) ;
int FN_ptsROTH( 	SIDE_FIT_k *psf, struct hsuit_st rsuit[2][4] ) ;
int FNnt_ptsROTH( struct hand_st rhand[2], struct hsuit_st rsuit[2][4] ) ;

int tot_suit_pts(SIDE_FIT_k *sf, int pts[] );
int tot_NT_pts(  SIDE_FIT_k *sf, int pts[] ); 
int choose_2ndfit_suit( int fid[4], int hsl[3] ) ;

/* Make these file globals only with a static keyword */
static	struct hand_st  rhand[2]    = { {0}, {0} };
static	struct hsuit_st rsuit[2][4] = { {{0},{0},{0},{0}}, {{0},{0},{0},{0}} };

/* Move these next two to the Library? Util.c file ? */	
void show1D_arr( int *arr, int NC ) {
			for (int nc=0; nc< NC; nc++ ) {
			fprintf(stderr,"%d ", arr[nc] );
		}
		fprintf(stderr,"\n");
	}
void show2D_arr( int *arr, int NR, int NC ) {
	int nr, nc ; 
	for (nr=0 ; nr< NR ; nr++ ) {
		fprintf(stderr,"NR=%d: ",nr);
		for (nc=0; nc< NC; nc++ ) {
			fprintf(stderr,"%d ", *(arr + nr*NC + nc) );
		}
		fprintf(stderr,"\n");
	} 
   return;
}		

int roth_calc (int side) {     /* Tag Number: 14 */
   int roth_NTpts[2] = {0}; /* Hand value if played in NT */
   int roth_pts[2]   = {0}; /* Hand value if played in a suit */
   int h, s;
   HANDSTAT_k *p_hs;
   SIDE_FIT_k  side_stat ;
   memset(rhand , 0 , sizeof(rhand) ); 
   memset(rsuit , 0 , sizeof(rsuit) );
 //  if (jgmDebug > 1 ) jgmDPRT = jgmDebug ;
 //  fprintf(stderr, "%s:%d jgmDPRT=%d from jgmDebug=%d\n",__FILE__,__LINE__,jgmDPRT, jgmDebug );
   prolog( side ) ;  /* zero globals, set the two handstat pointers, the two seats, and the pointer to the usereval results area */
   JGMDPRT(7 , "++++++++++ roth_calc prolog done for side= %d; compass[0]=%c, compass[1]=%c, phs[0]=%p, phs[1]=%p, hcp[0]=%d, hcp[1]=%d\n",
               side, compass[0],compass[1],(void *)phs[0], (void *)phs[1], phs[0]->hs_totalpoints, phs[1]->hs_totalpoints ) ;
   for (h = 0 ; h < 2 ; h++) {         /* for each hand */
      p_hs = phs[h] ; /* phs array set by prolog to point to hs[north] and hs[south] OR to hs[east] and hs[west] */
      for (s = CLUBS ; s<= SPADES ; s++ ) {
			rsuit[h][s].s_len = p_hs->hs_length[s]  ;
			rsuit[h][s].s_type = rothSuitType(p_hs, s) ; /* Check if suit is special */
			rsuit[h][s].s_hcp = p_hs->hs_points[s]  ;
         
         /* Roth Does not Deduct for play in NT. Only in suit so the adjustments only apply to roth_pts not roth_NTpts*/
         rsuit[h][s].s_hcp_adj = (int) shortHon_adj(p_hs, s, ROTH  ) ;
         /* Dpts Count 3/2/1. incl all dbltons; -- if there is no fit, you lose Dpts AND the hcp adj pts */
         rsuit[h][s].s_Dpts = DptsROTH( p_hs, s ) ;
         rsuit[h][s].s_Lpts = LptsROTH( p_hs, s, rsuit[h][s].s_type ) ; /* Lpts for any 6+ Maj or GOOD/up 6+ minor */
         if (rsuit[h][s].s_type >= RS_GOOD ) { rsuit[h][s].s_LptsNT = rsuit[h][s].s_Lpts ; } 
         rhand[h].h_Aces += p_hs->Has_card[s][Ace_rk] ; /* dont rely on pt4 since may have been redefined... */
         rhand[h].h_hcp     += rsuit[h][s].s_hcp  ;
         rhand[h].h_hcp_adj +=rsuit[h][s].s_hcp_adj ;
         rhand[h].h_Dpts    += rsuit[h][s].s_Dpts ;
         // A hand can count Lpts in only ONE suit. (Exception? later). Choose the greater 
			if (rsuit[h][s].s_Lpts   > rhand[h].h_Lpts)   { rhand[h].h_Lpts   = rsuit[h][s].s_Lpts;   }
			if (rsuit[h][s].s_LptsNT > rhand[h].h_LptsNT) { rhand[h].h_LptsNT = rsuit[h][s].s_LptsNT; }
			if (rsuit[h][s].s_type > rhand[h].h_type )    { rhand[h].h_type = rsuit[h][s].s_type ;    }
			
         JGMDPRT(5,"Roth_Calc, Hand=%d, suit=%c, SuitLen=%d, hcp[h][s]=%d, hcp-adj[h][s]=%d, Dpts[h][s]=%d Lpts[h][s]=%d:%d\n",
                     h, "CDHS"[s], p_hs->hs_length[s], rsuit[h][s].s_hcp, rsuit[h][s].s_hcp_adj,
                     rsuit[h][s].s_Dpts, rsuit[h][s].s_Lpts,rsuit[h][s].s_LptsNT );
      } /* end CLUBS <= s <= SPADES */
      rhand[h].h_bal = isBalanced(p_hs) ;
            
      /* +1 for all 4 Aces. With No Aces -1 when deciding to open. After that keep all original HCP. No -1
       * So a 14 Roth pt hand with no Aces would eval to 13 and Not open. But if pard Opens hand counts as 14 again
       * Given that this program does not bid, no point to -1 for No Aces.
       */
       if (rhand[h].h_Aces == 4 ) {rhand[h].h_Aces_pt  = 1 ;} 
      /* We can't total the pts for each hand yet; since we don't know who is Decl and who is Dummy
       * only Decl counts D, L, and FN. Dummy counts Dfit. D and Dfit depend on their being a fit or a RS_RLBL suit
       */
      JGMDPRT(6,"roth_calc:: Hand=%d, RothPts_suit=%d, RothPtsNT=%d, RothHandType=%d, Bal=%c, Aces=%d\n",
                     h, rhand[h].h_ptsSuit, rhand[h].h_ptsNT, rhand[h].h_type,"NY"[rhand[h].h_bal],rhand[h].h_Aces );
      JGMDPRT(6,"roth_calc:: Hand=%d, hcp[h]=%d, hcp-adj[h]=%d, Dpts[h]=%d, Lpts[h]=%d,LptsNT[h]=%d\n",
                  h, rhand[h].h_hcp, rhand[h].h_hcp_adj, rhand[h].h_Dpts, rhand[h].h_Lpts, rhand[h].h_LptsNT );
	}
	/* end for each hand rhand[] structs filled */

	/* Now we consider the hands as pair:
	 * if there is a '5-2 or better fit in a 'good' suit add an FN pt for NT.
	 * Next evaluate the hands in a suit contract:
	 *    In a suit contract Declarer keeps Dpts, and Lpts, and gets extra FN pts in trump suit.
	 * 							 Dummy loses Lpts gets Dfit pts which REPLACE Dpts.
	 * 	if the hand has an RS_SSS then it becomes declarer and keeps Dpts and Lpts*2. No FN Pts. Dummy gets Dfit pts.
	 *    if the hand has an RS_RRS then it becomes declarer and keeps Dpts and Lpts. Gets FN Pts. Dummy gets Dfit pts.
	 *    if there is an 8+ fit then: Declarer Keeps Dpts & LenPts, Gets FNpts in trump suit. Dummy gets Dfit points
	 *    if there is NO 8+ fit then: All Dpts are set to zero; Both can keep their Lpts ;
	 */
	 Fill_side_fitstat( phs , &side_stat ) ; /* side effect: fill trump_details and find declarer */
	 cpy_to_trumpfit(&trump, &side_stat) ;  /* for backwards compatibility */

  JGMDPRT(6,"TrumpFitCheck ROTH fit_len=%d  -- ",trump.fit_len) ;
   if ( trump.fit_len >= 8) {   /* Fit-len might be 7 here which means a 5-2 fit but a supported suit */
      if(seat[0] == trump.dummy) {h_dummy = 0 ; h_decl = 1 ;  }
      else                       {h_dummy = 1 ; h_decl = 0 ;  }
      JGMDPRT(6," -- TrumpSuit=%d, Decl=%d, Dummy=%d \n", trump.tsuit, h_decl, h_dummy);
        /* if there is a trump fit, **Replace** Dummy Dpts with Dfit_pts; Keep all D; promote Stiffs and Voids and One Dblton. 
         *                          **Add** Fn Points to Declarer
         * 									** If 2 or fewer trump in Dummy, deduct all Dpts, even with a fit.
         */
      /* Promote Dpts with Dfit pts 5 Trump: 2/2/1 or 4T=1/1/1 or 3T=0/0/0 Each Void or Stiff. Only ONE Dblton */
      rhand[h_dummy].h_Dfit_pts = DfitROTH  (&side_stat)  ; /* Dpts +1 if 4 trump; Dpts +2 if 5+ trump */
      rhand[h_decl].h_FN_pts    = FN_ptsROTH(&side_stat, rsuit ); /* +1 for each trump >4 in longest trump hand */
      FNnt_ptsROTH(rhand,rsuit); /* put up to 1 FN_NTpt into each hand for a GOOD 5 card suit with 'support' */
      rhand[h_decl].h_Dfit_pts  = rhand[h_decl].h_Dpts;     /* Keep Decl Dpts, but simplifies logic to use Dfit_pts in the total at end */
      JGMDPRT(7,"8+Trump Fit. Dummy=%d, Dfit_Pts=%d, Decl=%d, FN_pts=%d, FN_ptsNT=%d, D_aka_Dfit=%d \n",
								h_dummy,rhand[h_dummy].h_Dfit_pts, h_decl, rhand[h_decl].h_FN_pts, rhand[h_decl].h_FN_ptsNT, rhand[h_decl].h_Dfit_pts);
   } /* End Trump fit>=8 roth_pts all done for this case */
   else {
		rhand[h_dummy].h_Dfit_pts = 0 ;
		rhand[h_decl].h_Dfit_pts  = 0 ; /* cant keep Dpts if No 8 fit unless a Reliable Suit*/
      FNnt_ptsROTH(rhand,rsuit); /* put up to 1 FN_NTpt into each hand for a GOOD 5 card suit with 'support' */
		JGMDPRT(7,"No Trump Fit. Dummy=%d, Dfit_Pts=%d, Decl=%d, FN_pts=%d, FN_ptsNT=%d, D_Dfit=%d \n",
				h_dummy,rhand[h_dummy].h_Dfit_pts, h_decl, rhand[h_decl].h_FN_pts, rhand[h_decl].h_FN_ptsNT, rhand[h_decl].h_Dfit_pts);
	}
   
	tot_suit_pts(&side_stat, roth_pts   ); /* set roth_pts array; uses global var rhand[]. ignore return value */
	tot_NT_pts(  &side_stat, roth_NTpts ); /* set roth_NTpts array; uses global var rhand[]. ignore return value */

	JGMDPRT(6,"ROTH Pts NT: %d=%d + %d   Suit: %d = %d + %d \n",
				roth_NTpts[0]+roth_NTpts[1],roth_NTpts[0],roth_NTpts[1], roth_pts[0]+roth_pts[1], roth_pts[0],roth_pts[1]);
	
	/* Save the 6 main results that is the NT pts for the side and each hand and the suit pts for the same.
	 * also put in some debugging fields that show the detailed fields used to get the totals
	 */
      UEv.nt_pts_seat[0]  = roth_NTpts[0] ;
      UEv.nt_pts_seat[1]  = roth_NTpts[1] ;
      UEv.nt_pts_side = UEv.nt_pts_seat[0] + UEv.nt_pts_seat[1] ;

      UEv.hldf_pts_seat[0] = roth_pts[0];
      UEv.hldf_pts_seat[1] = roth_pts[1];
      UEv.hldf_pts_side = UEv.hldf_pts_seat[0] + UEv.hldf_pts_seat[1] ;
      JGMDPRT(6,"ROTH Eval Done.  Deal=%d,  Decl=%d, End Result for Suit Play:Tot=%d,Hand0=%d,Hand1=%d\n",
                                  gen_num, h_decl, UEv.hldf_pts_side, roth_pts[0], roth_pts[1] );
      JGMDPRT(5,"ROTH Final NTpts Fn incl. Tot=%d,Hand0=%d,Hand1=%d\n",
                  UEv.nt_pts_side, UEv.nt_pts_seat[0], UEv.nt_pts_seat[1] ) ;
/* now some debugging fields */
   UEv.misc_count = 0 ;
      UEv.misc_pts[UEv.misc_count++] = rhand[0].h_type;
      UEv.misc_pts[UEv.misc_count++] = rhand[0].h_hcp_adj; 
      UEv.misc_pts[UEv.misc_count++] = rhand[0].h_Dpts;
      UEv.misc_pts[UEv.misc_count++] = rhand[0].h_Lpts;
      UEv.misc_pts[UEv.misc_count++] = rhand[0].h_LptsNT;
      UEv.misc_pts[UEv.misc_count++] = rhand[0].h_FN_pts;
      UEv.misc_pts[UEv.misc_count++] = rhand[0].h_FN_ptsNT;
      UEv.misc_pts[UEv.misc_count++] = rhand[0].h_Dfit_pts;  
       /* same again for hand 1 */
      UEv.misc_pts[UEv.misc_count++] = rhand[1].h_type;
      UEv.misc_pts[UEv.misc_count++] = rhand[1].h_hcp_adj; 
      UEv.misc_pts[UEv.misc_count++] = rhand[1].h_Dpts;
      UEv.misc_pts[UEv.misc_count++] = rhand[1].h_Lpts;
      UEv.misc_pts[UEv.misc_count++] = rhand[1].h_LptsNT;
      UEv.misc_pts[UEv.misc_count++] = rhand[1].h_FN_pts;
      UEv.misc_pts[UEv.misc_count++] = rhand[1].h_FN_ptsNT;
      UEv.misc_pts[UEv.misc_count++] = rhand[1].h_Dfit_pts; 

      JGMDPRT(6,"ROTH UEv misc_count=%d,[h0]=%d,%d,%d,%d,%d,%d,%d,%d\n", UEv.misc_count, UEv.misc_pts[0],UEv.misc_pts[1],
                  UEv.misc_pts[2], UEv.misc_pts[3],UEv.misc_pts[4],UEv.misc_pts[5],UEv.misc_pts[6],UEv.misc_pts[7]);
      JGMDPRT(6,"ROTH UEv misc_count=%d,[h1]=%d,%d,%d,%d,%d,%d,%d,%d\n", UEv.misc_count, UEv.misc_pts[8], UEv.misc_pts[9],
						UEv.misc_pts[10],UEv.misc_pts[11],UEv.misc_pts[12],UEv.misc_pts[13],UEv.misc_pts[14],UEv.misc_pts[15]);
   SaveUserVals( UEv , p_uservals ) ;
   return ( 6 + UEv.misc_count ) ;
} /* end roth_calc */

/* Return Suit-type=0: normal, 1: wk_Major, 2: GoodSuit, 3:RothReliableSuit, 4:SelfSufficientSuit */
int rothSuitType (	HANDSTAT_k *p_hs , int s ) {
	int slen = p_hs->hs_length[s] ;
	if (5 >   slen ) {                                                      // Suits shorter than 5 cards never get Lpts, or FN pts
		JGMDPRT(7,"Suitlen[%d] < 5. Not special Returning RS_NONE=%d\n", s, RS_NONE);
		return RS_NONE ;
	} /* Not special */
   if (5 == slen && 2 <= p_hs->hs_counts[idxTop3][s] ) { return RS_GOOD ; } // a GOOD suit does not automatically get Lpts; Lpts code checks Len also
   if (5 == slen && 2 >  p_hs->hs_counts[idxTop3][s] ) { return RS_NONE ; } // all the non Good 5 card suits. 
	if (6 ==  slen ) {
		if(  (4 <= p_hs->hs_counts[idxTop5][s] ) ) { /*Lose at most one: KQJTxx min. AKJxxx? AQJxxx? => || (3<=p_hs->hs_counts[idxTop4][s])? */
			JGMDPRT(7,"Suit Len[%d] == 6. Top5 Cnt=%d Returning RS_SSS=%d\n", s, p_hs->hs_counts[idxTop5][s], RS_SSS);
			return RS_SSS ;  /* The max; a SelfSufficientSuit Hand keeps Dpts; Dbls all Lpts */
		}
		if( ((52 == p_hs->topcards[s][2]) && p_hs->Has_card[s][Nine_rk]) /* Bit mask for top 3 cards. KQT9xx */
		 || ((76  < p_hs->topcards[s][2]) && p_hs->Has_card[s][Nine_rk]) /* Bit mask for top 3 cards. AJT9xx or better JGM*/
		 || (  3 <= p_hs->hs_counts[idxTop4][s])                         /* KQJxxx or better JGM */
		 ) {
			JGMDPRT(7,"Suit Len[%d] == 6. Top4 Cnt=%d Topcards Weight=%d, Returning RS_RELBL=%d\n",
								s, p_hs->hs_counts[idxTop4][s],p_hs->topcards[s][2], RS_RELBL);
			return RS_RELBL ;  /* 2nd Best; Roth Reliable Suit Hand keeps Dpts Gets L pts */
		}
		if( 2 <= p_hs->hs_counts[idxTop3][s] ) {
			JGMDPRT(7,"Suit Len[%d] == 6. Top3 Cnt=%d Returning RS_GOOD=%d\n", s, p_hs->hs_counts[idxTop3][s], RS_GOOD);
			return RS_GOOD ;	/* 2+ of Top3 Good suit; Gets Lpts in NT and suit contracts */
		}
		if( ISMAJOR(s) ) 	{
			JGMDPRT(7,"Suit Len[%d] == 6. Top3 Cnt=%d IS MAJOR=1, Returning RS_WKMAJ=%d\n", s, p_hs->hs_counts[idxTop3][s], RS_WKMAJ);
			return RS_WKMAJ;	/* 6 crd M Gets Lpts in suit contract only */
		} 
		/* must be 6 card minor without 2 of top 3; it gets zero Lpts */
		JGMDPRT(7,"Suit Len[%d] == 6. Top3 Cnt=%d IS MAJOR=0, Returning RS_NONE=%d\n", s, p_hs->hs_counts[idxTop3][s], RS_NONE);
		return RS_NONE ;
	} /* end if 6 == slen */
	
	assert(7 <= slen ) ;
	if ( ( 3 <= p_hs->hs_counts[idxTop4][s] )  /* 3 of top 4 or AKTxxxx */
	 ||  (2 == p_hs->hs_counts[idxTop2][s] && 1 <= p_hs->hs_counts[idxTens][s] ) 	 
	 ) {
		JGMDPRT(7,"Suit Len[%d] >= 7. Top4 Cnt=%d Returning RS_SSS=%d\n", s, p_hs->hs_counts[idxTop4][s], RS_SSS);
		return RS_SSS ;
	} /* KQJxxxx (3 of top 4) or better or JGM added AKTxxxx */
	if( ( 2 <= p_hs->hs_counts[idxTop3][s] ) 							   /* KQxxxxx = Reliable Suit */
	 || (76 <= p_hs->topcards[s][2] )  										/* Bit mask for top 3 cards. AJTxxxx  or better JGM?? */
	 ) {
		 JGMDPRT(7,"Suit Len[%d]>= 7. Top3 Cnt=%d Topcards Weight=%d, Returning RS_RELBL=%d\n",
								s, p_hs->hs_counts[idxTop3][s],p_hs->topcards[s][2], RS_RELBL);
		 return RS_RELBL ;
	} /* KQxxxxx or better */
	if( 2 <= p_hs->hs_counts[idxTop3][s] ) {
		JGMDPRT(7,"Suit Len[%d] >= 7. Top3 Cnt=%d Returning RS_GOOD=%d\n", s, p_hs->hs_counts[idxTop3][s], RS_GOOD);
		return RS_GOOD ;
	}
	if( ISMAJOR(s) ) 	{
		JGMDPRT(7,"Suit Len[%d] >= 7. Top3 Cnt=%d IS MAJOR=1, Returning RS_WKMAJ=%d\n", s, p_hs->hs_counts[idxTop3][s], RS_WKMAJ);
		return RS_WKMAJ;
		
	}		/* This will only apply in a suit contract */
	/* must be 7+ minor without 2 of top 3; it gets zero Lpts */
	JGMDPRT(7,"Suit Len[%d] >= 7. Top3 Cnt=%d IS MAJOR=0, Returning RS_NONE=%d\n", s, p_hs->hs_counts[idxTop3][s], RS_NONE);
	return RS_NONE ;
} /* end rothSuitType */

/* If suit is RS_SSS the Lpts will be doubled later as part of FN pts calcualtion */
int LptsROTH(HANDSTAT_k *p_hs , int s, int stype ) {
	int slen = p_hs->hs_length[s] ;
	if(  6 >   slen  ) { return 0 ; } /* No Lpts for Suits shorter than 6 */
	if( (6 ==  slen) && (stype >= RS_WKMAJ ) ) { return 1 ; } /* 1 pt for 6 card GOOD or Major suit*/
	if( (7 <=  slen) && (stype >= RS_WKMAJ ) ) { return 2 ; } /* 2 pts for 7 card GOOD or Major suit*/
	return 0 ;  /* No pts for long minors that are not GOOD */
} /* end LptsROTH */

	
/* In Calculating FN pts, the Book is NOT 100% clear on some corner cases
 * a) what if there is a 4=4 fit and a 5=3 fit and we are playing in the 4=4 ?
 *    Book implies that in that case, the 5 card side suit gets a +1 Length pt.
 * b) Book states that in NT a 5=2 fit gets +1 length pt. But it does not explicitely state that only 1 hand gets it.
 *    So if there is 2=5 fit and a 5=2 fit then would there be a total of TWO FN pts in NT? JGM assumes yes.
 */


int DptsROTH( HANDSTAT_k *p_hs, int s ) {
	int dpts ;
	switch (p_hs->hs_length[s] ) { /* count all dbltons; adj for short honors done separately */
		case 0 : dpts = 3 ; break ;
		case 1 : dpts = 2 ; break ;
		case 2 : dpts = 1 ; break ;
		default: dpts = 0 ; break ;
	}
	return dpts ;
} /* end DptsROTH */

int DfitROTH (SIDE_FIT_k *psf ) { /* Dfit Pts REPLACE Dummy's Dpts. If Dummy has <3 trumps his Dpts become zero */
	int inc = 0 ;
	int dbltons = 0 ; 
	int dfpts = 0 ;
	int suit ; 
	int tmp1, tmp2, du;
	if (psf->t_fitlen < 8 ) { return 0 ; }  /* Cant have Dfit pts if there is no 8+ fit */
	/* applies to Dummy only; 3trump 3/2/1, 4trump 4/3/2, 5+trump 5/4/2 */
	switch (psf->t_len[psf->dummy_h] ) {  /* how many trump does dummy have? */
		case 0: return 0 ; break;  /* Dummy loses any Dpts if there is no fit, or if fewer than 3 trump */
		case 1: return 0 ; break;
		case 2: return 0 ; break; /* 2 trump or less loses Dpts. */
		case 3: return rhand[psf->dummy_h].h_Dpts ; break; /* Dfit same as Dpts 3/2/1 */
		case 4: inc = 1 ; break ; 
		case 5: inc = 2 ; break ;
	}
	du = psf->dummy_h;
	dfpts = rhand[du].h_Dpts ; /* start with Dpts and promote accordingly */
	tmp1 = dfpts ; 
	for (int s = 0 ; s < 4 ; s++ ) {  /* scan the suits in sorted order ... */
		suit = psf->sorted_sids[du][s] ; 
		if (suit == psf->t_suit) continue;
		tmp2 = psf->sorted_slen[du][s] ;
		if      (2 == psf->sorted_slen[du][s] && (++dbltons < 2) ) { dfpts += 1 ; } /* promote 1st dblton only by 1 pt */
		else if (2 >  psf->sorted_slen[du][s] )                    { dfpts += inc ; } /* promote stiffs/voids by 1 with 4, 2 with 5+ */
		JGMDPRT(8, "Roth Dfit Loop dummy_h=%d, suitID=%d, slen=%d, dbltons=%d, start_dfit=%d, inc=%d, dfit=%d\n",
		                             du,      suit,      tmp2,    dbltons,     tmp1,        inc,    dfpts ) ; 
	} /* end for s */
	JGMDPRT(7,"Roth Dfit Pts dummy_h=%d  Dummy_trumps=%d, Start Dpts=%d, Promo Inc=%d, Final Dfit Pts=%d\n",
			du, psf->t_len[du], rhand[du].h_Dpts , inc, dfpts );
	return dfpts;
}
	 
void cpy_to_trumpfit(TRUMP_FIT_k *ptrump , SIDE_FIT_k *sf ) {
	ptrump->dummy = sf->dummy_seat; 
	ptrump->decl  = sf->decl_seat ;
	ptrump->tsuit = sf->t_suit ;
	ptrump->tlen[0] = sf->t_len[0];
	ptrump->tlen[1] = sf->t_len[1];
	ptrump->fit_len = sf->t_fitlen ;
	ptrump->ss_len[0] = sorted_slen[0][3] ; /* globals set by Fill_side_fitstat */
	ptrump->ss_len[1] = sorted_slen[1][3] ;
	return ; 
} /* end cpy_to_trumpfit */


int tot_suit_pts(SIDE_FIT_k *psf, int pts[] ) { /* results in pts[]; uses GLOBAL rhand[] */
	int dc, du ;
	dc = psf->decl_h ;
	du = psf->dummy_h;
	pts[dc] = rhand[dc].h_hcp + rhand[dc].h_hcp_adj + rhand[dc].h_Aces_pt;
	pts[du] = rhand[du].h_hcp + rhand[du].h_hcp_adj + rhand[du].h_Aces_pt;  // No Lpts for Dummy in a suit contract. Only one trump suit.
	/* Now add to the Basics, the Lpts, the FNpts, and the D or Dfit pts. */
	/* A hand with an RS_SSS or RS_RELBL suit will always be declarer in that suit. */
	if (RS_SSS == rhand[dc].h_type) { /* Decl: always keep D; no FN ; Dble the L */
		pts[dc] +=  rhand[dc].h_Dpts + rhand[dc].h_Lpts*2 ; 
	   pts[du] +=  rhand[du].h_Dfit_pts ; 
	}
	else if (RS_RELBL == rhand[dc].h_type) { /* Decl: always keep D; also FN and L*/
		pts[dc] +=  rhand[dc].h_Dpts + rhand[dc].h_Lpts + rhand[dc].h_FN_pts; 
	   pts[du] +=  rhand[du].h_Dfit_pts ;
	}  
	else if (psf->t_fitlen >= 8 ) {  /* we have a true fit; Decl gets Dpts ; Dummy gets  DFIT */
		pts[dc] += rhand[dc].h_Dpts + rhand[dc].h_Lpts + rhand[dc].h_FN_pts; 
		pts[du] += rhand[du].h_Dfit_pts ;
	}
	else { /* No fit but playing in a suit anyway. Nobody Gets D or Dfit; might get FN for 5=2 fit */
		pts[dc] += rhand[dc].h_Lpts + rhand[dc].h_FN_pts;
		/* Nothing extra for Dummy in this case */
	}
	JGMDPRT(6,"tot_Suit_pts Decl=%d :: type=%d, Tot[%d]=HCP[%d]+Adj[%d]+Aces_pts[%d]+D_pts[%d]+Lpts[%d]+FN_pts[%d]\n", 
				dc, rhand[dc].h_type, pts[dc],rhand[dc].h_hcp,rhand[dc].h_hcp_adj,rhand[du].h_Aces_pt, rhand[dc].h_Dpts,rhand[dc].h_Lpts,rhand[dc].h_FN_pts); 
	JGMDPRT(6,"tot_Suit_pts Dmmy=%d :: type=%d, Tot[%d]=HCP[%d]+Adj[%d]+Aces_pts[%d]+Dfit_pts[%d]\n", 		
				du, rhand[du].h_type,pts[du],rhand[du].h_hcp,rhand[du].h_hcp_adj,rhand[du].h_Aces_pt, rhand[du].h_Dfit_pts );
  rhand[dc].h_ptsSuit = pts[dc] ;
  rhand[du].h_ptsSuit = pts[du] ;
  rhand[dc].h_fitsuit = psf->t_suit ;
  rhand[du].h_fitsuit = psf->t_suit ;
  
  return  pts[dc]+pts[du];
} /* end tot_suit_pts */

 int tot_NT_pts(SIDE_FIT_k *sf,  int pts[] ) { /* Results in pts[]; uses global rhand[] *sf not needed here; keep for symmetry and future */
	pts[0] = rhand[0].h_hcp + rhand[0].h_LptsNT + rhand[0].h_Aces_pt ; /* one hand may have been given FN pts; it does not matter which*/
	pts[1] = rhand[1].h_hcp + rhand[1].h_LptsNT + rhand[1].h_Aces_pt ;
	if (RS_SSS == rhand[0].h_type) {pts[0] += rhand[0].h_LptsNT ; } /* DBL the Lpts */
	else                           {pts[0] += rhand[0].h_FN_ptsNT;} /* L + FN pts */
	if (RS_SSS == rhand[1].h_type) {pts[1] += rhand[1].h_LptsNT ; }
	else                           {pts[1] += rhand[1].h_FN_ptsNT;}
	JGMDPRT(6,"tot_NT_pts hand_0:: type=%d,Tot[%d]= HCP[%d]+LptsNT[%d]+FN_NTpts[%d]+Acespt[%d] hand_1:: type=%d,Tot[%d]= HCP[%d]+LptsNT[%d]+FN_NTpts[%d]+Acespt[%d]\n",
				rhand[0].h_type,pts[0],rhand[0].h_hcp, rhand[0].h_LptsNT,rhand[0].h_FN_ptsNT,rhand[0].h_Aces_pt,
				rhand[1].h_type,pts[1],rhand[1].h_hcp, rhand[1].h_LptsNT,rhand[1].h_FN_ptsNT,rhand[1].h_Aces_pt );
  rhand[0].h_ptsNT = pts[0] ;
  rhand[1].h_ptsNT = pts[1] ;
	return pts[0]+pts[1] ;
} /* end tot_NT_pts */

int choose_2ndfit_suit(int fid[4], int hsl[3] ) { /* find the hand/suit with the longest suit. to excl the t_suit start with fid[1] */
	int h2f, l2f, s2f, s, htmp, i ;
	l2f = 0 ; 
	for (i = 1 ; 1< 4; i++ ) {
		s = fid[i];
		htmp = (rsuit[0][s].s_len >= rsuit[1][s].s_len) ? 0 : 1 ;
		if (rsuit[htmp][s].s_len > l2f ) {
			h2f = htmp;
			l2f = rsuit[h2f][s].s_len;
			s2f = s ;
		}
	}
	hsl[0]=h2f; hsl[1] = s2f; hsl[2] = l2f ; 
	return s2f ;
} /* end choose 2nd fit */

	/* 2nd Fit is only relevant if the trump suit is a 4=4 fit and hence get no FnPts.
	 * If the trump suit is anything else, 5=4, 5=3, 6=3, 6=2 it gets at least 1 FnPt and there is no need to find a second fit.
	 * So this code will only be relevant if in addition to the 4=4 there is a 5=3 or 6=2 fit.
	 * Typically would happen if there was a 4:4 major fit which will be the trump suit, and a 5-3 or 3-5 fit in some other suit.
	 */

int Fn_pts_2ndfit (int t_suit, struct hsuit_st rsuit[2][4] ) {
	int s,lh, fn_pts, suit2 ;
	fn_pts = 0 ;
	suit2 = 0 ;  
	for (s = 0 ; s<4 ; s++ ) {  /* check each suit in turn; there could be 3 x 8 fits, e.g. 4=4, 3=5, 6=2, 0=2 */
		if (s == t_suit ) continue ;
		if ((rsuit[0][s].s_len + rsuit[1][s].s_len) >= 8 ) { /* found a secondary 8+ fit */
			lh = 1 ;
			if ( rsuit[0][s].s_len >= rsuit[1][s].s_len ) lh = 0 ; /* lh is the hand with the longest trumps */
			if( (rsuit[lh][s].s_len > 4 ) && 
 			    (rsuit[lh][s].s_type >= RS_GOOD || ISMAJOR(s) )    // either 5+ Major or 5+ GOOD_SUIT
 			  ){
				  if( (rsuit[lh][s].s_len - 4 ) > fn_pts ) { /* this suit get more fn_pts than previously */
						fn_pts = rsuit[lh][s].s_len - 4 ;    /* replace previous 2nd fit fn_pts with new ones. */
						suit2 = s ; 
				  }
			 } /* choose 2nd fit with the longest, long hand */
	   } /* end fit >= 8 */
	} /* end for s */
	rsuit[lh][suit2].s_FN_pts = fn_pts ; 
	return fn_pts;
}	/* end 2nd fit */

/* NT FN pts awarded to any 5 card GOOD suit that has been 'supported'.
 * Note: If the suit is GOOD and it is longer than 5 it has already Got Lpts so can't add any more.
 * JGM assumes that if there is a 5:2 and a 2:5 fit then each hand will get an FN pt in NT. (But not in a suit contract)
 * This complicates the code, but is not contra-indicated by the book and two 'supported' 5crd GOOD suits, may each be a source of tricks.
 */
 
int FNnt_ptsROTH( struct hand_st rhand[2], struct hsuit_st rsuit[2][4] ) { 
	int h,s,oh;

	int fn_ptsNT[2] = {0,0}; /* max fn pts for hand[i] found so far */

   for (h = 0 ; h <2 ; h++ ) {
      oh = (h == 0 ) ? 1 : 0 ; 
      if (rhand[h].h_type < RS_GOOD ) continue ; /* if the hand does not have a GOOD suit, no FNpts_NT */
      if (rhand[h].h_LptsNT > 0 )     continue ; /* if the hand already has LptsNT then can't give any more */
      
      for (s = 0 ; s < 4 ; s++ ) {
         /* if suit is not GOOD then no FN; if a GOOD suit and > 5 already has Lpts; if < 5 does not get any */
			if ( (rsuit[h][s].s_len == 5) && (rsuit[h][s].s_type >= RS_GOOD) && (rsuit[oh][s].s_len >= 2 ) ) { 
			   // assert  must be a RS_GOOD (or better) 5 card suit facing 2+ support; give it 1 FN_ptsNT
            rhand[h].h_FN_ptsNT = 1 ;
            fn_ptsNT[h] = 1 ; 
            break ;  /* quit now, can only get one NT Lpt per hand */
         }
		} /* end for s */
   } /* end for h */
   return (fn_ptsNT[0] + fn_ptsNT[1] ) ;
}

// Add extra Len pts in the trump suit only, and only in Declarer's hand. For Minor suits must be a RS_GOOD suit.
int FN_ptsROTH( SIDE_FIT_k *psf, struct hsuit_st rsuit[2][4] ) { 
	int h,s, slen;
   if (psf->t_fitlen  < 8 ) { return 0 ; } /* No FN pts if there is not an 8+ fit */
   s = psf->t_suit;
   h = psf->decl_h ;

	slen = psf->t_len[h];  /* trump len in Decl hand */
	if ( slen < 5 ) { return 0 ; }  /* No FN pts for 4=4 fits */
   if ( rsuit[h][s].s_type < RS_WKMAJ ) { return 0 ; } /* Must be at least a 6 card Major or a GOOD Suit */
	rsuit[h][s].s_FN_pts = slen - 4 ; /* +1 for every card over 4 in a suit that qualifies. This is in addition to any L pts */
	return (rsuit[h][s].s_FN_pts);
} /* end FN_ptsRoth */

		
		
		
		

		

