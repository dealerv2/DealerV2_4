/* File bissell_calc.c */
/* Date        Version  Author   Description
 * 2024/08/12  1.0      JGM      Extracted from metrics_calcs.c and factors.c; Using the UE_SIDESTAT functionality
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
extern float_t shortHon_adj( HANDSTAT_k *p_hs, int suit, int mtag ) ;
extern void SaveUserVals(struct UserEvals_st UEv , USER_VALUES_k *p_ures ) ;
extern void set_dbg_names(int m, char *dbg_func) ;
extern int Pav_round(float val, int body ) ;
 /* Forward Function Definitions */

// main or userfunc has a) set all the mm_ptrs etc. b) used prolog to zero globals c) filled sidestat global with trump suit choice.

/* Bissell hand evaluation per the Bridge Encyclopedia and Pavlicek WebSite. Unique method from 1936 Nice way of Evaluating Honor cards
 * Bissell never mentions any sort of support points or Dfit or shortness pts of any kind.
 * Honors (incl T) are worth: (3 pts Minus the number of higher honors missing).
 * So JTx is worth zero(missing A,K,Q), but QJT is worth 1+1+1 = 3;(Missing only A and K)
 * 4 card suits are worth +1; 5 card or longer suits are worth 3*(L - 4)
 * Deductions for short honors are a bit from the iNet and a bit from JGM. Try to reflect that AJ is worth more than Ax e.g.
 */
int bissell_calc( UE_SIDESTAT_k *p_ss ) {   /* Tag Number: 1 */

   int h, s, Length ;
   HANDSTAT_k *p_hs, *phs[2];
   zero_globals( p_ss->side ) ;
   phs[0] = p_ss->phs[0];
   phs[1] = p_ss->phs[1];
   p_hs   = p_ss->phs[0] ;

   int biss_pts[2] ;
   float_t biss_totpts[2], biss_shape_ded[2]; ;
   float_t biss_honpts[2][4],biss_adj[2][4],biss_lenpts[2][4],biss_suitpts[2][4]; /* need to track gross and net by suit */
   float_t honor_pts = 0 ;
   int missing_honors = 0 ;
   int len4_cnt = 0 ;
   int void_cnt = 0 ;
   set_dbg_names(1, "bissel_calc");
   JGMDPRT(7 , "++++++++++ BISSELL calc for p_ss->side= %d; compass[0]=%c, compass[1]=%c, phs[0]=%p, phs[1]=%p, hcp[0]=%d, hcp[1]=%d\n",
               p_ss->side, compass[0],compass[1],(void *)phs[0], (void *)phs[1], phs[0]->hs_totalpoints, phs[1]->hs_totalpoints ) ;

   for (h = 0 ; h < 2 ; h++) {         /* for each hand */
      p_hs = phs[h] ; /* phs array set by prolog to point to hs[north] and hs[south] OR to hs[east] and hs[west] */
      int seat = p_hs->hs_seat;
      len4_cnt = 0 ;
      void_cnt = 0 ;
      biss_totpts[h] = 0.0 ;
      biss_shape_ded[h] = 0 ; 
      for (s = SUIT_CLUB; s <= SUIT_SPADE; ++s) {
         biss_honpts[h][s]=0;
         biss_adj[h][s]=0;
         biss_lenpts[h][s]=0;
         biss_suitpts[h][s]=0; /* need to track gross and net by suit */
         honor_pts = 0 ;
         missing_honors = 0 ;

         Length = p_hs->hs_length[s];
         if ( Length == 4 ) len4_cnt++ ;
         if ( Length == 0 ) void_cnt++ ;
         JGMDPRT(9,"BISS for seat=%d, suit=%d, length=%d \n", seat, s, Length) ;

         int HasAce   = p_hs->Has_card[s][ACE] ;
         int HasKing  = p_hs->Has_card[s][KING];
         int HasQueen = p_hs->Has_card[s][QUEEN];
         int HasJack  = p_hs->Has_card[s][JACK];
         int HasTen   = p_hs->Has_card[s][TEN];

         JGMDPRT(9,"A=%d,K=%d,Q=%d,J=%d,T=%d,\n",HasAce,HasKing,HasQueen,HasJack,HasTen ) ;

    /* Honors Points. Each honor (incl T) is worth 3 - number of missing higher honors. */
         if (HasAce)   {  honor_pts += 3; }
         else          { missing_honors++ ; }
         JGMDPRT(9,"Has_Ace=%c honor_pts=%g, missing_honors=%d \n","NY"[HasAce],honor_pts,missing_honors ) ;
         if (HasKing) {  honor_pts += (3 > missing_honors) ? (3 - missing_honors) : 0 ; }
         else { missing_honors++ ; }
         JGMDPRT(9,"Has_King=%c honor_pts=%g, missing_honors=%d \n","NY"[HasKing],honor_pts,missing_honors ) ;
         if (HasQueen) {  honor_pts += (3 > missing_honors) ? (3 - missing_honors) : 0 ; }
         else { missing_honors++ ; }
         JGMDPRT(9,"Has_Queen=%c honor_pts=%g, missing_honors=%d \n","NY"[HasQueen],honor_pts,missing_honors ) ;
         if (HasJack) {  honor_pts += (3 > missing_honors) ? (3 - missing_honors) : 0 ; }
         else { missing_honors++ ; }
         JGMDPRT(9,"Has_Jack=%c honor_pts=%g, missing_honors=%d \n","NY"[HasJack],honor_pts,missing_honors ) ;
         if (HasTen ) { honor_pts += (3 > missing_honors) ? (3 - missing_honors) : 0 ; }
         else { missing_honors++ ; }
         JGMDPRT(9,"Has_Ten=%c honor_pts=%g, missing_honors=%d \n","NY"[HasTen],honor_pts,missing_honors ) ;
         biss_honpts[h][s] = honor_pts;
         biss_adj[h][s]    = shortHon_adj(p_hs, s, BISSEL) ;
         if ( (biss_honpts[h][s] + biss_adj[h][s] ) < 0.0 ) {
            biss_adj[h][s] = 0.0 - biss_honpts[h][s] ; /* cant have neg pts in suit*/
         }
         biss_lenpts[h][s] = (Length <= 3 ) ? 0.0 :
                             (Length == 4 ) ? 1.0 : 3.0*(Length - 4) ; /* 3 pts for every card over 4 */
         biss_suitpts[h][s] = biss_honpts[h][s] + biss_adj[h][s] + biss_lenpts[h][s] ;
         if (biss_suitpts[h][s] > 3.0 * Length ) { biss_suitpts[h][s] = 3.0*Length ; }
         JGMDPRT(9, "BISS Suit=%c, SuitTot=%g, Lenpts=%g, HonPts=%g, AdjPts=%g\n",
               "CDHS"[s],biss_suitpts[h][s],biss_lenpts[h][s],biss_honpts[h][s],biss_adj[h][s] ) ;
         biss_totpts[h] += biss_suitpts[h][s] ;
      } /* end for suit */
      if ( p_hs->square_hand || (len4_cnt == 3)       /* 4333 or 4441 */
        || ( (len4_cnt == 2) && (void_cnt == 0) ) ) {
         biss_shape_ded[h] = -1.0; /* or 44xy not 4450  therefore 4432 */
         biss_totpts[h] += -1.0 ;
      }
      biss_pts[h] = Pav_round(biss_totpts[h], p_ss->pav_body[h]); /* starting points for hand h */
      UEv.nt_pts_seat[h] = biss_pts[h] ;
      JGMDPRT(8, "Biss Hand[%d] UEv.nt=%d, biss_tot=%g biss_shape_ded=%g\n",h, UEv.nt_pts_seat[h], biss_totpts[h],biss_shape_ded[h] );
   } /* end for each hand */
   /*SaveUserEvals always puts these next values into the first six slots. Bissel does not have any HLDF but keep the std. */
    UEv.nt_pts_side = biss_pts[0] + biss_pts[1] ;
    UEv.nt_pts_seat[0] = biss_pts[0] ;
    UEv.nt_pts_seat[1] = biss_pts[1] ;
    UEv.hldf_pts_side = biss_pts[0] + biss_pts[1] ;
    UEv.hldf_pts_seat[0] = biss_pts[0] ;
    UEv.hldf_pts_seat[1] = biss_pts[1] ;
    
    UEv.hldf_suit   = p_ss->t_suit;     /* So we know which dds tricks to count if we are playing in a suit */
    UEv.hldf_fitlen = p_ss->t_fitlen ;  /* Bissell does not use fits or anything from UEsidestat, but Dealer might want this. */

    UEv.misc_count = 0 ;

      /* Bissell pts breakdown; same in NT and in suit */
      for (h= 0 ; h < 2 ; h++ ) {
         for (s = SUIT_CLUB; s <= SUIT_SPADE; ++s) {
            UEv.misc_pts[UEv.misc_count++] = lround(biss_suitpts[h][s] ) ;
         }
      }
      UEv.misc_pts[UEv.misc_count++] = biss_shape_ded[0];
      UEv.misc_pts[UEv.misc_count++] = biss_shape_ded[1];

      SaveUserVals( UEv , p_uservals ) ;
      JGMDPRT(7, "Bissell calcs Done UEV MIsc-count=%d biss_pts[0]=%d, biss_pts[1]=%d\n", UEv.misc_count, biss_pts[0],biss_pts[1]);
  return ( 6 + UEv.misc_count ) ;
/* Deductions for stiffs A(3-0=3), K(2-1 = 1) Q(1-1 = 0) ; Since stiff J and stiff T = 0 no deduct for them
 * Deductions for HH     AK(6-1=5), AQ(5-1=4), AJ(4-.5=3.5), KQ(4-1 = 3), KJ(3-.5=2.5),QJ(2-.5=1.5),Qx(1-.5=.5)
*/
} /* end bissell_calc */

