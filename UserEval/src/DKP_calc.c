/* File dkp_calc.c */
/* Date        Version  Author   Description
 * 2024/08/12  1.0      JGM      Extracted from metrics_calcs.c and factors.c; Using the UE_SIDESTAT functionality
 * 2025/04/12  1.1      JGM      Prevent any final results from being less than zero.
 *
 */
#define GNU_SOURCE
#include "../include/std_hdrs_all.h"
#include "../include/UserEval_types.h"
#include "../include/UserEval_externs.h"
#include "../include/dbgprt_macros.h"
#include "../include/mmap_template.h"

/* External functions -- metrics_util_subs.c , short_honors_subs.c */
extern void zero_globals( int side ) ; 
extern float_t calc_alt_hcp( HANDSTAT_k *p_hs, int suit, int mtag ) ;
extern float_t shortHon_adj( HANDSTAT_k *p_hs, int suit, int mtag ) ;
extern void SaveUserVals(struct UserEvals_st UEv , USER_VALUES_k *p_ures ) ;
extern FIT_PTS_k Do_Df_Fn_pts( UE_SIDESTAT_k *p_ss,
                               int (*calc_dfval)(UE_SIDESTAT_k *p_ss, int h),
                               int (*calc_fnval)(UE_SIDESTAT_k *p_ss, int h)           ) ;
extern void set_dbg_names(int m, char *dbg_func) ; 
 /* Forward Function Definitions */
 float_t AdjDKP( HANDSTAT_k    *p_hs, int suit ) ;
 int SynDKP(     HANDSTAT_k    *p_hs, int s ) ;
 int DfitDKP(    UE_SIDESTAT_k *p_ss, int du) ;
 int DKP_Fn_pts( UE_SIDESTAT_k *p_ss, int h ) { return 0 ; } /* DK does not do Fn Pts */

// main or userfunc has a) set all the mm_ptrs etc. b) used prolog to zero globals c) filled sidestat global with trump suit choice.

/* Danny Kleinman's "Little Jack Points" per his NoTrump Zone book.
 * He does not give any method for trump fit support points so go with a vanilla approach
 */
int dkp_calc( UE_SIDESTAT_k *p_ss) {       /* Tag Number: 2 */
   int dkp_pts[2] = {0, 0};
   int syn_pts[2] = {0, 0};
   float_t dkp_hcp[2][4] = { {0.0}, {0.0} };
   int h, s ;
   float_t adj_suit;
   int syn_suit ;
   int temp = 0 ;

   zero_globals( p_ss->side ) ; 
   HANDSTAT_k *p_hs, *phs[2];
   phs[0] = p_ss->phs[0];
   phs[1] = p_ss->phs[1];
   p_hs   = p_ss->phs[0] ;
   set_dbg_names(2, "dkp_calc" ) ;
   JGMDPRT(7 , "++++++++++ DKP (m=2) calc for p_ss->side= %d; compass[0]=%c, compass[1]=%c, phs[0]=%p, phs[1]=%p, hcp[0]=%d, hcp[1]=%d, fhcp=[%g,%g]\n",
               p_ss->side, compass[0],compass[1],(void *)phs[0], (void *)phs[1], phs[0]->hs_totalpoints, phs[1]->hs_totalpoints, fhcp[0],fhcp[1] ) ;
   for (h = 0 ; h < 2 ; h++) {         /* for each hand */
      p_hs = phs[h] ; 
      for (s = CLUBS ; s<= SPADES ; s++ ) {
         dkp_hcp[h][s] = calc_alt_hcp(p_hs, s, DKP) ; /* calc DKP LJP points for this suit. */
         fhcp[h] += dkp_hcp[h][s] ;
         adj_suit = AdjDKP(p_hs, s ) ; /* -1 for any suit whose lowest card is higher than a Ten incl A, AK, AKQJ & unguarded Qx Jxx */
         fhcp_adj[h] += adj_suit;
         syn_suit = SynDKP(p_hs, s) ; /* add for K if with Ace, and Q or J if with higher honor and Tens with companions */
         syn_pts[h] += syn_suit;
         JGMDPRT(8,"DKP_Hidx=%d, suit=%c, dkp_hcp[s]=%g, adj[s]=%g,syn[s]=%d HandTot_Raw_fhcp[h]=%g, HandTot_fhcp_adj=%g, Tot_syn=%d, SuitLen=%d\n",
                     h, "CDHS"[s], dkp_hcp[h][s], adj_suit, syn_suit, fhcp[h], fhcp_adj[h], syn_pts[h], p_hs->hs_length[s] );

/* DK does not say if he adds pts for length or not. But he is OldFashioned so assume +1 for each card > 4 max of 3 Lpts*/
         if (p_hs->hs_length[s] > 4 ) {
            temp = (p_hs->hs_length[s] - 4 ) ; // +1 for five carder, +2 for 6, +3 for longer. etc.
            if (temp > 3 ) { lpts[h] += 3 ; }
            else           { lpts[h] += temp; }
            JGMDPRT(8,"L_ptsDKP, Hand=%d, suit=%d, len=%d, Tot_Lpts=%d\n", h, s, p_hs->hs_length[s], lpts[h] );
          }
      } /* end for s = CLUBS to SPADES*/
      hcp_adj[h] = fhcp_adj[h]; /* just for debugging in the UE misc values */
      if (p_hs->square_hand) { hand_adj[h] += -2 ; }
      dkp_pts[h] = (int)lround( (fhcp[h] + fhcp_adj[h]  + syn_pts[h] + hand_adj[h] )/3.0 ) ; /* hcp and synpts are div by 3 */
      dkp_pts[h] += lpts[h];      /* lpts are not div by 3 */
      if (dkp_pts[h] < 0 ) dkp_pts[h] = 0; /* JGM::2025-04-12 a hand can NEVER have negative points no matter the deductions */
      JGMDPRT(7,"-------DKP for hidx=%d DONE. dkp_pts=%d, lpts=%d, 4333_adj=%d, synpts=%d, fhcp=%g, fhcp_adj=%g \n",
               h, dkp_pts[h],lpts[h],hand_adj[h], syn_pts[h], fhcp[h], fhcp_adj[h] ) ;
      UEv.nt_pts_seat[h] = dkp_pts[h] ;
   } /* end for each hand */
   UEv.nt_pts_side = UEv.nt_pts_seat[0] + UEv.nt_pts_seat[1] ;
   JGMDPRT(7,"DKP NT pts done. pts[0]=%d, pts[1]=%d, UEv_Side_pts=%d\n", dkp_pts[0],dkp_pts[1], UEv.nt_pts_side );

   /* Done both hands -- Now calc Dfit and Fn for the side  
    *  Assume DK is 'Standard' 531 with 4 trump, 321 with 3 trump. A 4-4 fit then both hands count Dfit
    */
   TFpts = Do_Df_Fn_pts(p_ss, DfitDKP, DKP_Fn_pts) ; /* Set the globals dfit_pts[],Fn_pts[], and struct TFpts */

   dkp_pts[0] += TFpts.df_val[0] + TFpts.fn_val[0] ;  /* there are no DKP Fn pts currently */
   dkp_pts[1] += TFpts.df_val[1] + TFpts.fn_val[1] ;
   UEv.hldf_pts_seat[0] = dkp_pts[0] ;
   UEv.hldf_pts_seat[1] = dkp_pts[1] ;
   UEv.hldf_pts_side = dkp_pts[0] + dkp_pts[1] ;
   UEv.hldf_suit   = p_ss->t_suit;     /* So we know which dds tricks to count if we are playing in a suit */
   UEv.hldf_fitlen = p_ss->t_fitlen ;

   UEv.misc_count = 0 ;
      /* The factors that apply to both NT and Suit */
      UEv.misc_pts[UEv.misc_count++] = hcp_adj[0];
      UEv.misc_pts[UEv.misc_count++] = lpts[0];
      UEv.misc_pts[UEv.misc_count++] = syn_pts[0];
      UEv.misc_pts[UEv.misc_count++] = hand_adj[0];   /* 4333 deduction */
      /* Factors that apply to suit contracts only */
      UEv.misc_pts[UEv.misc_count++] = dfit_pts[0];   /* Dummy support pts typically 3-2-1 with 3, 5-3-1 with 4+ */
      UEv.misc_pts[UEv.misc_count++] = Fn_pts[0];     /* DKP Fn_ptsMONE */
      /* ditto Hand 1 */
      UEv.misc_pts[UEv.misc_count++] = hcp_adj[1];
      UEv.misc_pts[UEv.misc_count++] = lpts[1];
      UEv.misc_pts[UEv.misc_count++] = syn_pts[1];
      UEv.misc_pts[UEv.misc_count++] = hand_adj[1];
      UEv.misc_pts[UEv.misc_count++] = dfit_pts[1];
      UEv.misc_pts[UEv.misc_count++] = Fn_pts[1];

  /* now put the results into the user result area at p_uservals */
  SaveUserVals( UEv , p_uservals ) ;
     JGMDPRT(7, "DKP fit calcs Done; Tsuit=%d, Decl=%c, HLDFpts[%d : %d], Fn_pts[%d:%d], Dfit_pts=[%d:%d]\n",
         p_ss->t_suit,"NSEW"[p_ss->decl_seat], dkp_pts[0],dkp_pts[1], Fn_pts[0],Fn_pts[1],dfit_pts[0],dfit_pts[1] );
  return ( 6 + UEv.misc_count ) ;
} /* end dkp_calc */
/* DfitDKP is the same as DfitSTD; Any 8+ fit; 5/3/1 with 4+ suppt, 3/2/1 with 3+ suppt -- ONE short suit only */
int DfitDKP( UE_SIDESTAT_k *p_ss, int du) { /* du specifies the hand getting the Dfit Pts. */
   int ss_len, t_len, dfit ;
   int idx_short ;
   int DF_DKP[2][4] = { {3,2,1,0},{5,3,1,0} } ; /* Dfit pts with 3/4 trump for V/S/D/Longer any 8+ fit  */
   int v0, s1, db ; /* Debug vars */
   t_len = p_ss->t_len[du];
   ss_len = p_ss->sorted_slen[du][3] ; /* shortest suit is in slot 3 */
   JGMDPRT(7, "DKP Calc Dfit for du seat=%d:%c, du=%d, tlen=%d, and ss_len=%d\n",
               seat[du],compass[du], du, p_ss->t_len[du], p_ss->sorted_slen[du][3]  );
   /* No Dfit if no short suit, or fewer than 3 card support, or no 8+ fit */
   if ( (p_ss->t_len[du] <  3 ) ||  (p_ss->t_fitlen < 8) || 3 <= ss_len  ) { return 0 ; } 

   v0 = 0 ; s1 = 0 ; db = 0 ;
   if      ( 0 == ss_len ) { idx_short = 0 ; v0++ ;}
   else if ( 1 == ss_len ) { idx_short = 1 ; s1++ ;}
   else if ( 2 == ss_len ) { idx_short = 2 ; db++ ;}
   else                    { idx_short = 3 ; }
   if (3 == t_len  ) { dfit = DF_DKP[0][idx_short] ; }  /* 3 card support; */
   else              { dfit = DF_DKP[1][idx_short] ; }  /* 4+ support; */
   JGMDPRT(7, "DfitDKP Returning, Dummy=[%d], dfit=%d, from sidestat.ss_len=%d,t_len[du]=%d: V_S_D_cnt=%d,%d,%d\n",
                  du, dfit, ss_len, t_len,v0,s1,db ) ;
   return dfit ;
} /* end DfitDKP */


int SynDKP(HANDSTAT_k *p_hs, int s ) {     /* Honor cards with A (incl K) or K -> +1 syn pt per card */
   /*  e.g. +1=>[ AKx, KQx, KJx], +2:=>[KQJx, AKQx, AQJx], 0 =>[QJx]*/
    int synpts = 0 ;
    int ten_synpts = 0 ; 
    if (p_hs->Has_card[s][KING]  && p_hs->Has_card[s][ACE] )          { synpts += 1 ; } // King with the Ace
    if (p_hs->Has_card[s][QUEEN] && p_hs->hs_counts[idxTop2][s] > 0 ) { synpts += 1 ; } // Queen with Ace or King
    if (p_hs->Has_card[s][JACK]  && p_hs->hs_counts[idxTop2][s] > 0 ) { synpts += 1 ; } // Jack with Ace or King

    /* Tens with two 'companions' worth +2; with one 'companion' +1 (not T9 dblton) */
    if (p_hs->Has_card[s][TEN] ) {
       if ( (p_hs->hs_counts[idxTop4][s] >= 2 ) || (p_hs->hs_counts[idxTop4][s] == 1 && p_hs->Has_card[s][NINE]) ) {
          synpts += 2 ;
       }
       else if ( (p_hs->hs_counts[idxTop4][s] == 1)  || (p_hs->Has_card[s][NINE] && p_hs->hs_length[s] > 2 ) ) {
           synpts += 1 ;
       }
    } /* end Has_Ten */
    JGMDPRT(8,"SynDKP, SynPTS=%d, Ten_Synpts=%d, Suit=%c Len=%d, Top4=%d, HAS_Ten=%d, Has_Nine=%d \n",
         synpts, ten_synpts, "CDHS"[s],p_hs->hs_length[s],p_hs->hs_counts[idxTop4][s], p_hs->Has_card[s][TEN], p_hs->Has_card[s][NINE]) ;
    return (synpts+ten_synpts) ;
} /* end SynDKP */

float_t AdjDKP(HANDSTAT_k  *p_hs, int suit ) {
    /*     Note DKP adj are in effect 1/3 of what is calc here since final result is div by 3 */
    /*     Adj rules are simple: Any suit who's lowest card is higher than a Ten gets -1 adj
     *     This includes all stiff honors (stiff A incl) and all Dblton Honors (AK incl, down to QJ)
     *     Also any unguarded Q (QJ, QT, Qx) or Jack (JT(x) or Jx(x) ) gets -1 adj; but QJx is not adjusted
     */
      JGMDPRT(8," In AdjDKP, suit=%c, len=%d Top4=%d, QUEEN=%d, JACK=%d\n",
         "CDHS"[suit], p_hs->hs_length[suit], p_hs->hs_counts[idxTop4][suit] , p_hs->Has_card[suit][QUEEN], p_hs->Has_card[suit][JACK] ) ;
     /* Next statement covers Stiff A, K, Q, J, AK, AQ, AJ, KQ, KJ, QJ */
     if (p_hs->hs_counts[idxTop4][suit] == p_hs->hs_length[suit] && p_hs->hs_length[suit] > 0 ) { return (-1.0) ; } /* higher than T*/
     /* unguarded Queen. Note: KQ, AQ, QJ covered by previous statement. */
     if (p_hs->Has_card[suit][QUEEN] && p_hs->hs_length[suit] == 2 ) { return (-1.0) ; } /* QT, or Qx */
     /* test for unguarded Jack. i.e. No higher honor and length <= 3 */
     if (p_hs->Has_card[suit][JACK] && p_hs->hs_counts[idxTop4][suit] == 1 && p_hs->hs_length[suit] <= 3  ) { return (-1.0) ; }
     // fprintf(stderr, "!!!!!! AdjDKP reached NO adjustment \n");
     return (0.0) ; /* no adjustment */
} /* end adjDKP */

