/* File analyze_side.c */
/*  Date   Version  Author  Description
 * 2024-08-15 0.9    JGM    Created values pertaining to the side for all metrics; most relevant is BestFit details. 
 * 
 */
#include "../include/std_hdrs_all.h"
#include "../include/UserEval_types.h"
#include "../include/UserEval_externs.h"
#include "../include/dbgprt_macros.h"
#include "../include/mmap_template.h"
#define ISMAJOR(s)  ( (  (s) >= HEARTS  ) ? 1 : 0 )
#define ISMINOR(s)  ( (  (s) <= DIAMONDS) ? 1 : 0 )
#define WEAKER(x,y) ( (  (x) < (y) ) ? (x) : (y)  )

	/* External/Util Function Declarations */
extern int didxsort4( int v[4], int x[4] ) ;
extern void show_sorted_slen(UE_SIDESTAT_k *p_ss) ;
extern void show_sorted_fits(UE_SIDESTAT_k *p_ss) ;
extern void show_UEsidestat( UE_SIDESTAT_k *p_ss ) ;
extern int Pav_body_val( HANDSTAT_k  *p_hs ) ;

/* Forward Declarations */

int SetDeclarer(   UE_SIDESTAT_k *p_ss ) ;
int BestFit(       UE_SIDESTAT_k *p_ss ) ;
int choose_7fit(   UE_SIDESTAT_k *p_ss ) ;
int choose_fit(    UE_SIDESTAT_k *p_ss ) ;
void save_fit_vals(UE_SIDESTAT_k *p_ss, int suit ) ;
int picksuit( int x, int y, HANDSTAT_k *phs[] ) ;
int rank_sorted_ids(int v[] , int idx[] ) ;
FIT_PTS_k Do_Df_Fn_pts( UE_SIDESTAT_k *p_ss,
                        int (*calc_dfval)( UE_SIDESTAT_k *p_ss, int h),
                        int (*calc_fnval)( UE_SIDESTAT_k *p_ss, int h) ) ;

/* prolog called before this one; prolog preserves the phs[] and side values in UEsidstat[] */
int analyze_side( UE_SIDESTAT_k *p_UEss , int side) { /* fill a struct with the details of the side's fits */
   int sorted_sids[2][4] = {{0,1,2,3} , {0,1,2,3} };
   int fitlen[4] = {0,0,0,0} ;
   int fitids[4] = {0,1,2,3} ;
   int t_suit ; 
   HANDSTAT_k  *phs[2] , *p_hs; /* local pointers with same names as the global ones */
   JGMDPRT(7,"analyze_side begins with p_UEss=%p and side=%d\n",(void *)p_UEss,side);
  
   phs[0] = p_UEss->phs[0] ; /* p_UEss->phs[], set by main::prolog() via main::calc_ptrs() along with other pointers */
   phs[1] = p_UEss->phs[1] ;
   p_hs   = phs[0] ;
   JGMDPRT(7,"analyze side inits handstat ptrs to: phs[0]=%p, phs[1]=%p, p_hs=%p\n",(void*)phs[0],(void*)phs[1],(void*)p_hs);
  
	/* sort the suit lengths (long to short) for each hand; keep the suit_ids in sync 
	 * so if hearts is the longest suit with 6 then slen_sorted[0]=6, and sids_sorted[0] = 2 (aka hearts) 
	 */
    for (int h=0 ; h<2 ; h++ ) {   /* sort suit lengths long to short for both hands */ 
      p_hs = phs[h];
      for( int j=0; j<4; j++) {p_UEss->sorted_slen[h][j] = p_hs->hs_length[j] ; } /* copy short ints to ints */
      memcpy(   p_UEss->sorted_sids[h], sorted_sids[h],  sizeof(sorted_sids[h]) ) ;
      didxsort4(p_UEss->sorted_slen[h], p_UEss->sorted_sids[h] );   /* sorted_slen[h[0]] is longest length, [3] is shortest */
      JGMDPRT(7, "After didxsort4 Sorted_slen[h] done. %d,%d,%d,%d sorted sids=[%d,%d,%d,%d]\n",
       p_UEss->sorted_slen[h][0], p_UEss->sorted_slen[h][1], p_UEss->sorted_slen[h][2], p_UEss->sorted_slen[h][3],
       p_UEss->sorted_sids[h][0], p_UEss->sorted_sids[h][1], p_UEss->sorted_sids[h][2], p_UEss->sorted_sids[h][3] );
     

      /* we might have 4=4=4=1; or N=N=x=y; we want the id in slot 0 to be highest ranking suit */
      rank_sorted_ids(p_UEss->sorted_slen[h], p_UEss->sorted_sids[h] ) ; /* Call ranking in case there are any ties anywhere */

      p_UEss->pav_body[h] = Pav_body_val( p_hs) ; /* used by several metrics so calc it once */
        JGMDPRT(7, "analyze_side=%d hand=%d, pav_body=%d Longest SuitLen=%d in suit=%d\n",
               p_UEss->side, h, p_UEss->pav_body[h], p_UEss->sorted_slen[h][0], p_UEss->sorted_sids[h][0] );
   } /* end foreach hand */
   DBGDO(8, show_sorted_slen( p_UEss ) );  /* shows ranked sorts for both hands */
    
   for (int s=CLUBS; s<=SPADES; s++ ) {
      fitlen[s] = phs[0]->hs_length[s] + phs[1]->hs_length[s] ;
      JGMDPRT(8,"Fitlen[%d]=%d, ",s,fitlen[s] );
   }
   JGMDPRT(8,"\n") ; /* to end the JGMDPRT in the loop */
	didxsort4(fitlen, fitids) ; 						/* sort the fits, longest to shortest */
	memcpy(p_UEss->fitlen, fitlen, sizeof(fitlen) ) ;
	memcpy(p_UEss->fitids, fitids, sizeof(fitids) ) ;
	rank_sorted_ids(p_UEss->fitlen, p_UEss->fitids  )  ; /* Call ranking in case there are any ties anywhere */
   
   DBGDO(8, show_sorted_fits( p_UEss ) );
   /* from the sorted fits, pick the best one Major>minor, more symmetrical, or finally weakest */
   t_suit = BestFit( p_UEss ) ; /* if returns -1 there is no 8+ card fit; there might be a 5:2 fit which needs extra checking if wanted */
                               /* BestFit calls save_fit_vals to fill the UEsidestat fields */
   JGMDPRT(7,"In Analyze_side. BestFit() returns t_suit=%d\n",t_suit); /* t_suit might be 5:2 fit or might be -1 if no fit that good */
   int decl_h = SetDeclarer(p_UEss) ;     /* if trump_suit < 0 Decl set as for NT contract */

   p_UEss->decl_h =  decl_h;
   p_UEss->dummy_h= !decl_h;   /* 1 -> 0 and 0 -> 1  */
   p_UEss->decl_seat = seat[ decl_h];  /* 0 or 2 if side NS, 1 or 3 if side EW (seat[0] and [1] set by prolog to COMPASS_NORTH etc.) */
   p_UEss->dummy_seat= seat[!decl_h];
   JGMDPRT(7,"Analyze_side. Done. Showing UEsidestat now \n");
   DBGDO(7, show_UEsidestat(p_UEss) ) ; 
   return p_UEss->t_suit ;                  /* might as well return something useful, even if ignored */
}
/* end analyze_side */

/* If we are sorting suit lengths, we call the rank_sorted_ids routine afterwards to make sure that in the case of ties
 * the suit_id in the lowest slot of the ties (the 'longest suit') is the highest ranking suit.  Worst case we have 4=4=4=1; might have N=N=x-y
 * Same after we have sorted the fitlens, we rank ties in suit rank order. Simplifies choosing Maj > Min etc. only need to consider top two.
 * We 'suit_rank' all ties not just the one(s) that matter; helps with debugging and may help in future.
 */
int rank2sids (int idx, int sids[4] ) { /* called when len[idx] == len[idx+1] */
   int t, t0, t1 ;
   t0 = sids[idx] ; t1 = sids[idx+1] ; 
   if (sids[idx] < sids[idx+1] ) {
      t= sids[idx]; sids[idx]=sids[idx+1]; sids[idx+1] = t ;
      JGMDPRT(9,"Rank2sids[%d, %d] Swapping them to [%d, %d]\n", t0, t1, sids[idx], sids[idx+1] );
   }
   else { JGMDPRT(9,"Rank2sids[%d, %d] NO swap. [%d, %d]\n", t0, t1, sids[idx], sids[idx+1] );
      ;
   }
   return sids[idx] ;
}
int rank3sids(int idx, int sids[4] ) { /* called when len[idx] == len[idx+1] == len[idx+2] */
   JGMDPRT(9,"Ranking 3 sids from idx=%d \n",idx );
   rank2sids(idx,   sids );
   rank2sids(idx+1, sids );
   rank2sids(idx,   sids );
   JGMDPRT(9,"Result of 3 rank [%d, %d, %d ]\n",sids[idx] ,sids[idx+1] ,sids[idx+2] );
   return ( sids[idx] ) ;
}

int rank_sorted_ids(int val[], int idx[] ) { /* used mostly for fitlens but can be used for slens also */
   if(val[0] == val[1] && val[0] == val[2] ) { rank3sids(0, idx) ; return idx[0]; } /* swap and adjust the sid contents to be in desc suit order */
   else if (val[0] == val[1] && val[2] == val[3] ) {
      rank2sids(0, idx) ; rank2sids(2,idx) ;
      return idx[0];
   }
   /* val[1] is longest by itself but there may be ties in the other 3; */
   else if (val[1] == val[2] && val[1] == val[3] ) { rank3sids(1, idx) ; return idx[0]; }
   else if (val[1] == val[2] ) { rank2sids(1, idx) ; return idx[0] ; }
   else if (val[2] == val[3] ) { rank2sids(2, idx) ; return idx[0] ; }
   return idx[0];
}

void save_fit_vals(UE_SIDESTAT_k *p_ss,  int s ) {
   if (s < 0 ) {  /* there is no 8+ or 5:2 fit set some sane values */
          p_ss->t_suit = -1 ;
          p_ss->t_fitlen = 7;  /* this must be the case */
          p_ss->t_len[0] = 0 ; /* it is really 4:3 or 3:4 but not relevant if not 5:2 */
          p_ss->t_len[1] = 0 ;
          p_ss->t_rank = 0 ;
   }
   else {
       p_ss->t_suit = s ;
       p_ss->t_len[0] = p_ss->phs[0]->hs_length[s] ;
       p_ss->t_len[1] = p_ss->phs[1]->hs_length[s] ;
       p_ss->t_fitlen = p_ss->t_len[0] + p_ss->t_len[1] ;
       p_ss->t_rank = ISMAJOR(s) ;
   }
       JGMDPRT(4, "SaveFitVals t_suit=%d,t_rank=%d, t_fitlen=%d, t_lens=[%d : %d]\n",
       p_ss->t_suit,p_ss->t_rank,p_ss->t_fitlen,p_ss->t_len[0],p_ss->t_len[1] ) ;
       return ;
} /* end save fit vals */
  
int BestFit( UE_SIDESTAT_k *p_ss ) {
	/* Called by analyze_side. (*p_ss) already has sorted suit_lens, and sorted/ranked fitlens in it */
   /* check if there is an 8+ fit;
    * if yes choose suit of longest fit;
    * if two fits same len choose the Major;
    * if both fits are same rank, then choose the more symmetrical fit (i.e. 4=4 instead of 5=3, or 5=4 instead of 6=3
    * if both fits are the same rank, and same symmetry then choose the one with the FEWER total hcp.(Vondracek effect)
    *
    * if no 8 fit there will be at least two, maybe three 7-fits. Reject 4=3,6=1, or 7=0. Accept 5:2.
    * Choose the best 5:2 fit as Major > minor, or if same rank, weakest > strongest.
    */
   int s0 ;
   p_ss->t_suit = -1 ;  /* assume no acceptable fit */
   p_ss->t_fitlen = -1 ; /* must be an un-acceptable 7 card fit; prob 4-3 */
   s0 = p_ss->fitids[0] ; /* suit with longest fit or tied */
   JGMDPRT(8,"Checking Best Fit.  Fitlen[0]=[%d:%d] Fitlen[1]=[%d:%d]\n",p_ss->fitids[0],p_ss->fitlen[0], p_ss->fitids[1],p_ss->fitlen[1] );
   /* There will always be either two or three 7fits, or at leat one 8+ fit */
	if (p_ss->fitlen[0] > p_ss->fitlen[1] ) { /* there is only one longest fit -- it MUST be longer than 7 */
      save_fit_vals(p_ss, s0) ; 
      JGMDPRT(8,"BestFit. Only one fit suit. Suit=%c Fitlen=%d [%d : %d] rank=%c \n",
						"CDHS"[s0],p_ss->t_fitlen,p_ss->t_len[0],p_ss->t_len[1],"mM"[p_ss->t_rank] );
      return s0 ;
   }
   /* There is more than one longest fit. Might be two or three fits of 7 or more. 7 Are special cases */
   /* With two+ equal we cannot assume that the 'best fit' is in slot zero; Spades (slot 0) might be 5-3 while hearts (slot 1) are 4:4 */

	if (7 == p_ss->fitlen[0]) { /* are the longest fits 7 cards ? */  	 
		 s0 = choose_7fit( p_ss ) ;  /* might return -1 if no 5:2 fits */
       save_fit_vals(p_ss, s0 );
       JGMDPRT(8, "Choose 7 fit returns suit=%d -1 means 4:3 fit \n",s0 );
       return s0 ; 
	} /* end 7 card fit choice */
   
   /* There are two/three 8+ fits of same length */
   s0 = choose_fit( p_ss ) ;  /* Choose between them */
   save_fit_vals(p_ss, s0);
   JGMDPRT(8, "Choose 8+ fit returns suit=%d with length %d\n",s0, p_ss->t_fitlen );
   DBGDO(8,show_UEsidestat(p_ss) ) ; 
   return s0 ; 
} /* end BestFit */

/* choose between Major and minor, then between weaker and stronger */
int picksuit( int x, int y, HANDSTAT_k *phs[] ) {
   int hcp_x, hcp_y ;
   JGMDPRT(8,"Picking between suits x=%d, y=%d\n",x,y);
   if(ISMAJOR(x) && !ISMAJOR(y) ) { return x ; }
   if(!ISMAJOR(x) && ISMAJOR(y) ) { return y ; }
   /* x and y have same rank; pick the weaker one as the trump suit */
   hcp_x = phs[0]->hs_points[x] + phs[1]->hs_points[x] ;
   hcp_y = phs[0]->hs_points[y] + phs[1]->hs_points[y] ;
   if (hcp_x <= hcp_y ) { return x ; }
   return y ;
}
   

/* called when fitlen[0] == 7 and fitlen[1] == 7 could be 4:3 or 5:2 fit return the idx in fitids */
/* TBD If there are two 7 fits, one a 5:2 (or 6:1) minor fit,and one a 4:3 Major fit which one should we choose? *The Major for now */
int choose_7fit (UE_SIDESTAT_k *p_ss ) { /* edge case 5=5=2=1 vs 2=2=5=4 aka 3 7 card fits*/
		 int s0, s1, s2;
		 int diff0, diff1, diff2, idx ;
		  s0 = p_ss->fitids[0] ;
		  s1 = p_ss->fitids[1] ;
		  s2 = p_ss->fitids[2] ;
        diff0 = abs( p_ss->phs[0]->hs_length[s0] - p_ss->phs[1]->hs_length[s0] ) ; /* only accept diff==3, i.e. 5-2 fits */
        diff1 = abs( p_ss->phs[0]->hs_length[s1] - p_ss->phs[1]->hs_length[s1] ) ;
        diff2 = abs( p_ss->phs[0]->hs_length[s2] - p_ss->phs[1]->hs_length[s2] ) ;
        idx = ((diff0 == 3)<<2) | ( (diff1 == 3 ) << 1 ) | ((diff2 == 3) ) ;  /* code the results of testing for a 5:2 fit */
        JGMDPRT(8, "Choose 7 fit s0=%d,s1=%d,s2=%d IDX=%d from %d, %d, %d \n",s0,s1,s2,idx,diff0,diff1,diff2) ;
        switch (idx) {
           case 0 : { return -1 ; } /* no 5:2 fit */
           case 1 : { return s2 ; } /* only 5:2 fit is s2 */
           case 2 : { return s1 ; } /* only 5:2 fit is s1 */
           case 3 : { return picksuit(s1,s2, p_ss->phs); } /* choose major over minor, weaker suit over stronger suit */
           case 4 : { return s0 ; } /* only 5:2 fit is s0 */
           case 5 : { return picksuit(s0,s2, p_ss->phs); }
           case 6 : { return picksuit(s0,s1, p_ss->phs); }
           case 7 : { return picksuit(s0, picksuit(s1,s2,p_ss->phs), p_ss->phs) ; }
           default: { /* NOTREACHED */ return -1 ; }
        } /* end switch idx */
   /* NOTREACHED */
   return -1 ;      
}
/* end choose_7fit */

/* called when fitlen[0] == fitlen[1] && >= 8
 * Even if there are three 8 card fits, we only need to consider the top two, since ties are in suit rank order and we pick Maj > Minor
 */
int choose_fit (UE_SIDESTAT_k *p_ss ) {  /* return the suit id of the 'best' 8+ fit suit */
	int s0, s1;
	int diff0, diff1; 
	int rank0, rank1 ;
	int hcp0, hcp1   ;

	s0 = p_ss->fitids[0] ;
   s1 = p_ss->fitids[1] ;		  
   rank0 = ISMAJOR(s0) ;
   rank1 = ISMAJOR(s1) ;
   JGMDPRT(8,"Two+ fits Len=%d Suit0[%c] Rank0=%c Suit1[%c] Rank1=%c\n", p_ss->fitlen[0],"CDHS"[s0],"mM"[rank0], "CDHS"[s1], "mM"[rank1] );
   if      ( rank0 > rank1 )	{ return s0; }/* Major over minor? choose it */
   else if ( rank1 > rank0 ) 	{ return s1; }
   // the two longest so far are same rank -- choose the more balanced;
   diff0 = abs( p_ss->phs[0]->hs_length[s0] - p_ss->phs[1]->hs_length[s0] ) ;
   diff1 = abs( p_ss->phs[0]->hs_length[s1] - p_ss->phs[1]->hs_length[s1] ) ;
   if (      diff0 < diff1 ) { return s0 ; } // check which fit is more balanced
   else if ( diff1 < diff0 ) { return s1 ; }
         // same length fit, same rank, same balance 3=5 & 5=3 or both 4=4 etc.
         // Vondracek effect choose fit with the least hcp.
   hcp0 = p_ss->phs[0]->hs_points[s0] + p_ss->phs[1]->hs_points[s0] ;
   hcp1 = p_ss->phs[0]->hs_points[s1] +p_ss-> phs[1]->hs_points[s1] ;
   JGMDPRT(8,"Two 8+ fits Same Rank, Same Shape. Choosing weaker hcp0=%d, hcp1=%d\n",hcp0,hcp1);
   if ( hcp0 <= hcp1) { return s0 ; } /* choose weaker suit.  Vondracek effect */
	return s1 ;
}
/* end choose_fit */

int SetDeclarer(UE_SIDESTAT_k *p_ss ) { 

   /* The stronger hand is always declarer if there is no trump suit. (Played in NT) */
   if(p_ss->t_suit > SPADES || p_ss->t_suit < 0 ) {  /* NT just the stronger hand is declarer */
      if (p_ss->phs[0]->hs_totalpoints >= p_ss->phs[1]->hs_totalpoints ) { return 0 ; }
      return 1 ;
   }

   assert( ( CLUBS <= p_ss->t_suit && p_ss->t_suit <= SPADES ) );
   /* The hand with the longer trumps is the Declarer -- per Pavlicek */
   int t_suit = p_ss->t_suit  ; 
   if (      p_ss->phs[0]->hs_length[t_suit] > p_ss->phs[1]->hs_length[t_suit] ) { return 0 ; }
   else if ( p_ss->phs[1]->hs_length[t_suit] > p_ss->phs[0]->hs_length[t_suit] ) { return 1 ; }
   /* The trump lengths are equal Check who is stronger. If Equal assume hand zero i.e. N or E */
   if (p_ss->phs[0]->hs_totalpoints >= p_ss->phs[1]->hs_totalpoints ) { return 0 ; }
   return 1 ;
} /* end SetDeclarer */


FIT_PTS_k Do_Df_Fn_pts( UE_SIDESTAT_k *p_ss,
                        int (*calc_dfval)( UE_SIDESTAT_k *p_ss, int h),
                        int (*calc_fnval)( UE_SIDESTAT_k *p_ss, int h) )
         {
   /* -------- Do_Df_Fn Assumptions and Explanation ----------- 
    * Check if there is an 8+ fit. If there is call the metric specific function to
    * a) calculate the Dummy support points, and
    * b) calculate any other points that Declarer might get in the case of a trump fit. (Called Fn_pts )
    * The Reference material specifying the metric does not usually address these two issues:
    * i) how to handle a 4-4 (or 5-5) fit.
    * ii) how many short suits is dummy allowed to count for ?
    * This code assumes that if the trumps are equal in both hands (4-4 or 5-5) then BOTH hands count dummy points and Nobody gets FN pts
    * How many short suits are counted, depends on how the Dfit metric is coded.
    * The Analyze_side function that fills the UE_SIDESTAT_k struct, chooses the trump suit, and the Decl and Dummy hands.
    * The details of the actual Dfit and Fn Calculations are left up to the individual metrics.
   */
  // ---- Do_Df_Fn_pts ----
   FIT_PTS_k TFpts = { {0,0}, {0,0} } ;
   int h_du = p_ss->dummy_h;
   int h_dc = p_ss->decl_h ;

   if ( p_ss->t_fitlen < 8 ) { return TFpts ; } /* if no 8-fit return zeroes This routine ignores 5-2 fits */

    /*
     * if there is a 8+ trump fit, calc Dfit and Fn Points.
     * What kind of Df and Fn will depend on which func ptrs are passed in
     */
   
      JGMDPRT(7,"DO_Df_Fn_pts: h_dummy=%c, h_decl=%c Seat[0] = %d t_suit=%d t_len[dc]=%d, t_len[du]=%d\n",
            compass[h_du],compass[h_dc],seat[0], p_ss->t_suit, p_ss->t_len[h_dc], p_ss->t_len[h_du] );
      if (p_ss->t_len[h_du] < p_ss->t_len[h_dc]) {  
         TFpts.df_val[h_du]   = (*calc_dfval) ( p_ss, h_du ) ;  /* call Dfit routine; returns single value */
         TFpts.fn_val[h_dc]   = (*calc_fnval) ( p_ss, h_dc ) ;  /* call Fn function; returns single value */
         JGMDPRT(7, ": Dfit for Dummy[%c]=%d, FN for Decl[%c]=%d\n",
            compass[h_du],TFpts.df_val[h_du],compass[h_dc],TFpts.fn_val[h_dc] );
      }
      else if (p_ss->t_len[h_du] == p_ss->t_len[h_dc]) {  /* probably a 4=4 fit; assign Dfit to both hands, and no Fn  */
         TFpts.df_val[h_du]   = (*calc_dfval) (p_ss, h_du ) ;
         /* now call dfit again for the other hand */

         TFpts.df_val[h_dc]   = (*calc_dfval) (p_ss, h_dc )  ;
         JGMDPRT(7, ": 4=4 Dfit for dummy[%c]=%d, AND Dfit for Decl[%c]=%d\n",
            compass[h_du],TFpts.df_val[h_du],compass[h_dc],TFpts.df_val[h_dc] );
      }
      else {
         fprintf (stderr, "***ABORT*** Cant happen in Do_Df_Fn_pts, Dummy_tlen[%d]=%d, Decl_tlen[%d]=%d\n",
                           h_du, p_ss->t_len[h_du], h_dc, p_ss->t_len[h_dc] );
         assert(0) ;
      }
      /* set the globals as a convenience */
      dfit_pts[0] = TFpts.df_val[0]; dfit_pts[1] = TFpts.df_val[1];
      Fn_pts[0]   = TFpts.fn_val[0]; Fn_pts[1]   = TFpts.fn_val[1];
      return TFpts ;
}
/* end Do_Df_Fn_pts */   
