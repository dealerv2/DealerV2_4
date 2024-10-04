/* File KnR_calc.c */
/* Date        Version  Author   Description
 * 2024/08/12  1.0      JGM      Extracted from metrics_calcs.c and factors.c; Using the UE_SIDESTAT functionality
 *
 */

/* The Four C's KnR algorithm, including the extra points for Fits and Support, and the deductions for misfits
 * These latter ones are specified in the article but have never been coded before
 * Probably because most programs do not evaluate two hands together but only separately then add them up.
 * But of course no bridge player actually works that way.
 * Plain vanilla CCCC is implemented in Dealer so is not needed here. (see the file deal_knr.c in the dealer src dir )
 */
/*
* The Original CCCC code has been submitted to dealer original version by Danil Suits <DSuits@silknet.com>, Mar 10, 1999.
* 2023-01-21 JGM Major rewrite for the DealerServer daemon called by the Dealer usereval function
*/

#define _GNU_SOURCE 1
#include <assert.h>
#include "../include/std_hdrs.h"
#include "../include/dealdefs.h"
#include "../include/dealtypes.h"
#include "../include/UserEval_externs.h"
#include "../include/UserEval_types.h"
#include "../include/dbgprt_macros.h"
#include "../include/KnR_calc.h"

extern void set_dbg_names(int m, char *dbg_func) ;

extern HANDSTAT_k hs[4] ;

extern int jgmDebug ;
extern struct misfit_st misfit_chk(HANDSTAT_k *phs[], int s ) ;

struct KnR_points_st KnR_pts_all (HANDSTAT_k *phs) {
  struct KnR_points_st Pts = { 0 };
  int knr_hand_tot = 0 ;  /* Total for the hand */
  int knr_suit_tot = 0;      /* total for the suit */
  int pakq = 0 ;     /* A=3, K=2, Q=1 before adjustments for the suit*/
  int pjt  = 0 ;     /* lower honors; J and Ten for the suit*/
  int ShapePoints = 0;  /* for the suit */
  int QualityPoints = 0; /* for the suit */

  int HigherHonors = 0;  /* Counts the honors as we go thru the suit; eventually includes even the Jack */
  int Dbltons = 0 ;        /* For the hand */
  int Shorts = 0 ; /* JGM 2023. Count voids and stiffs in the hand */

  int seat = phs->hs_seat;
  int suit;
  set_dbg_names(7, "KnR_Fit");
  JGMDPRT(7,"C4 START for HandStat Ptr=%p seat=%d\n", (void * ) phs, seat ) ;
  /* For each suit.... */
  for (suit = SUIT_CLUB; suit <= SUIT_SPADE; ++suit) {
    knr_suit_tot = 0;  pakq = 0 ;  pjt = 0 ;
    ShapePoints = 0 ; /* Shortness points in this suit */
    HigherHonors = 0; /* A, K, or Q in this suit */
    QualityPoints = 0 ; /* QualityPoints in this suit */

    int Length = phs->hs_length[suit];
    JGMDPRT(8,"C4 for seat=%d, suit=%d, length=%d \n", seat, suit, Length) ;

    int HasAce   = HAS_CARD2p(phs,suit, RK_ACE);
    int HasKing  = HAS_CARD2p(phs,suit, RK_KING);
    int HasQueen = HAS_CARD2p(phs,suit, RK_QUEEN);
    int HasJack  = HAS_CARD2p(phs,suit, RK_JACK);
    int HasTen   = HAS_CARD2p(phs,suit, RK_TEN);
    int HasNine  = HAS_CARD2p(phs,suit, RK_NINE);
    JGMDPRT(8,"A=%d,K=%d,Q=%d,J=%d,T=%d,Nine=%d\n",HasAce,HasKing,HasQueen,HasJack,HasTen,HasNine ) ;

    /* Honors Points. A=3, K=2, Q=1 ; J, T, 9 depend on other higher cards */
    if (HasAce) {
      pakq += 300;
      HigherHonors++;
    } /* end HasAce*/
    JGMDPRT(8,"Has_ACE=%d, pakq_321Pts=%d High_Honors=%d\n", HasAce, pakq,HigherHonors) ;

    if (HasKing) {
      pakq += 200;
      if (Length == 1) pakq -= 150;  /* Stiff K worth only 0.5 plus 2 pts for the stiff added later*/
      HigherHonors++;               // HigherHonors counts A and K at this point.
    } /* end HasKing */
    JGMDPRT(8,"Has_KING=%d, pakq_321Pts=%d High_Honors=%d\n", HasKing, pakq,HigherHonors) ;

    if (HasQueen) {  // Stiff Q=0; AQ or KQ, -> Q=0.5 ; QJ/QT/Qx ->  Q = 0.25
         /* Stiff Q gets zero 321 points */
        if (Length == 2 && HigherHonors == 0)  pakq +=  25 ;    // QJ, QT, Qx => Queen is worth 0.25
        if (Length == 2 && HigherHonors >  0)  pakq +=  50 ;    // AQ or KQ =>  Queen is worth 0.5
        if (Length >= 3 && HigherHonors == 0)  pakq +=  75 ;    // Qxx, QJx, QTx etc. Q is worth 0.75
        if (Length >= 3 && HigherHonors >  0)  pakq += 100 ;    // AQx(x), KQx(x), AKQ(x) Q is worth 1.0
        HigherHonors++;                                        // HigherHonors now includes the Q.
    } /* end HasQueen */
    JGMDPRT(8,"Has_Queen=%d, pakq_321Pts=%d High_Honors=%d\n", HasQueen, pakq, HigherHonors) ;
    Pts.knr_honor_pts += pakq ;
    knr_suit_tot += pakq ;

    if (HasJack) {
        if (HigherHonors == 2) pjt += 50;   // J with precisely two (but not 3) of A,K,Q
        if (HigherHonors == 1) pjt += 25;
        HigherHonors++;                                        // HigherHonors now includes the J.
    } /* end HasJack */
    JGMDPRT(8,"Has_Jack=%d, pjt_H_Pts=%d High_Honors=%d\n", HasJack, pjt,HigherHonors) ;

    if (HasTen ) { /* Count all Tens even in long suits for this part  */
        if ( HigherHonors == 2)             pjt += 25;
        if ((HigherHonors == 1) && HasNine) pjt += 25;
    } /* end HasTen */
    JGMDPRT(8,"Has_Ten=%d, pjt_H_Pts=%d High_Honors=%d Has_Nine=%d\n", HasTen, pjt,HigherHonors,HasNine) ;
    knr_suit_tot += pjt ;
    Pts.knr_honor_pts += pjt ;
    JGMDPRT(8,"High Card suit pts=%d from AKQ=%d, + JT=%d \n",knr_suit_tot , pakq, pjt ) ;
   /*
    * Calculate suit Quality for this suit
    */
    if (Length > 0 ) {
        QualityPoints = KnR_suit_quality (phs, suit);
    }
    else { QualityPoints = 0 ; }
    knr_suit_tot += QualityPoints;
    Pts.knr_qual_pts += QualityPoints ;

    JGMDPRT(8,"Qual=%d,  New suitEval=%d \n", QualityPoints ,knr_suit_tot ) ;

    switch (Length) {
       case 0 : ShapePoints = 300 ; Shorts++; break ;
       case 1 : ShapePoints = 200 ; Shorts++; break ;
       case 2 : ShapePoints = 100 ; Dbltons++ ; break ; /* Will subtract later if only 1 Dblton and no other shortness */
      default : ShapePoints = 0 ; break ;
    }
    knr_suit_tot += ShapePoints ;  /* the total points for this suit so far */
    Pts.knr_short_pts += ShapePoints ;
    knr_hand_tot += knr_suit_tot ;      /* total points for the hand so far */
    JGMDPRT(8,"This suit ShapePoints=%d,  New suitEval=%d NewHand Total=%d \n", ShapePoints ,knr_suit_tot,knr_hand_tot ) ;
  }      /* end for (suit;...) */

         /* All Dbltons were given +100; deduct the 1st one if there was no other shortness */
         /* This "no other shortness" is from an inference in the text which describes AJTxxx/KT9xx/xx/x as having
          * "3 points for distribution, the singleton and the doubleton ... "
          */
  if (Dbltons > 0 && Shorts == 0) { Pts.knr_short_pts -= 100  ; knr_hand_tot -= 100 ; }

    /* Test for 4333 */
  if ( phs->square_hand == 1 )  { knr_hand_tot -= 50 ; Pts.knr_adj = -50 ; }
  JGMDPRT(7,"C4 END SEAT[%d] TotEval=%d, square=%d\n",seat,knr_hand_tot, phs->square_hand ) ;

  assert ((knr_hand_tot % 5) == 0); /* KnR pts go in steps of 0.05 always */
  /* The traditional CCCC points for this hand have been done. The Dfit and Fn pts will come later */
  Pts.knr_tot_pts = Pts.knr_honor_pts + Pts.knr_short_pts + Pts.knr_qual_pts + Pts.knr_adj ;
  Pts.knr_body_val = KnR_Body( phs ) ;
  Pts.knr_dfit = 0 ;
  Pts.knr_Fn_pts = 0 ;
  Pts.knr_misfit_adj = 0 ;
  Pts.knr_rounded_pts = KnR_Round( Pts.knr_body_val, Pts.knr_tot_pts ) ; /* Round what we have so far; update later */
  #ifdef JGMDBG
   if (jgmDebug >= 7) { show_knr_pts (7, Pts, "End Of KnR_pts_all() - No Dfit or Fn" ) ; }
  #endif
  return (Pts);
}

int KnR_suit_quality (HANDSTAT_k *phs, int suit) {
    /* called from KnR_pts_all above  */
  int qhcp = 0 ;  /* the value of the A,K,Q,J,T and adj before *len/10 */
  int Quality = 0; /* suit qual pts (hcp + adj) * len / 10 ; hcp: A=4,K=3,Q=2,J=1,T=0.5; adj for T with 9, missing QJ in 7+ etc.*/
  int HigherHonors = 0;
  int seat = phs->hs_seat;
  int Length = phs->hs_length[suit];
  JGMDPRT(8,"Quality :: suit=%d, Length=%d, seat=%d \n", suit, Length, seat);

  int HasAce   = HAS_CARD2p(phs,suit, RK_ACE);      /* JGM Mod; HAS_CARD2 now just accesses hs[seat].Has_card[s][r] */
  int HasKing  = HAS_CARD2p(phs,suit, RK_KING);
  int HasQueen = HAS_CARD2p(phs,suit, RK_QUEEN);
  int HasJack  = HAS_CARD2p(phs,suit, RK_JACK);
  int HasTen   = HAS_CARD2p(phs,suit, RK_TEN);
  int HasNine  = HAS_CARD2p(phs,suit, RK_NINE);
  int HasEight = HAS_CARD2p(phs,suit, RK_EIGHT);


  /*ACE*/
  if (HasAce) {
      qhcp += 400 ;
      HigherHonors++;
      JGMDPRT(8,"Quality ::Ace qhcp=%d HighHonors=%d\n", qhcp,HigherHonors) ;
   }

  /*KING*/
  if (HasKing) {
      qhcp += 300;
      HigherHonors++;
      JGMDPRT(8,"Quality ::King qhcp=%d HighHonors=%d\n", qhcp,HigherHonors) ;
   }

  /*QUEEN*/
  if (HasQueen) {
      qhcp += 200;
      HigherHonors++;
      JGMDPRT(8,"Quality ::Queen qhcp=%d HighHonors=%d\n", qhcp,HigherHonors) ;
   }

  /*JACK*/
  if (HasJack) {
      qhcp += 100;
      HigherHonors++;
      JGMDPRT(8,"Quality ::Jack qhcp=%d HighHonors=%d\n", qhcp,HigherHonors) ;
   }
   /* Ten */
     if (HasTen) {      /* All Tens count for at least 0.5; some tens count more see later */
      qhcp += 50;
      JGMDPRT(8,"Quality ::Ten qhcp=%d HighHonors=%d\n", qhcp,HigherHonors) ;
   }

/* This next bit adds Q and/or J equivalent in 7+ suits even if the honor is not present. ReDone to suit me JGM */
  if ( Length == 7 && (HasJack == 0 || HasQueen == 0 ) ) {qhcp += 100 ; } /* +1 if either honor is missing in 7 card suit */
  else if ( Length == 8 ) {                    /* +2 for missing Q; if Q present +1 for missing Jack */
      if      (HasQueen == 0 ) { qhcp += 200 ; } /* if missing Q, Jack is irrelevant */
      else if (HasJack  == 0 ) { qhcp += 100 ; } /* if HasQueen but not Jack, then +1 */
   }
   else if (Length >= 9 ) { /* a 9+ suit can get 2 extra for missing Q AND 1 extra for missing Jack */
      if ( HasQueen == 0 ) { qhcp += 200 ; }
      if ( HasJack  == 0 ) { qhcp += 100 ; }
   } /* end if-else-if testing for Q or J in 7+ suits */
     JGMDPRT(8,"qhcp :: with Q & J adjustment= %d suit len = %d \n",qhcp, Length ) ;

   /* Now check for presence of working Ten in suits less than 6 in len */
    if (Length <= 6 ) {
     if (HasTen) {
        if ( (HigherHonors > 1) || HasJack ) { qhcp += 50 ; }       // Ten with J or with 2+ is worth 1 else worth 0.5
     }
     if (HasNine && ( HigherHonors == 2 || HasEight || HasTen ) ) { qhcp += 50 ; }
      JGMDPRT(8,"qhcp :: with Ten & 9 adjustment= %d suit len = %d \n",qhcp, Length ) ;
   } /* end Length <= 6 */
   Quality = qhcp * Length/10 ;
   JGMDPRT(8,"Suit[%d:%c] Final Quality Pts=%d Length=%d, qhcp=%d\n", suit, "CDHS"[suit],  Quality, Length,qhcp) ;
  assert ((Quality % 5) == 0);  /* Q must be divisible by 5 */
  return Quality;
} /* end suit_quality() */

/* Body per Pavlicek. Affects how the KnR points are rounded to ints -- could be replaced by using the values in SIDESTAT */
int KnR_Body( HANDSTAT_k *phs ) {
   int Body   = 0 ;
   int suit ;
   for (suit = 0 ; suit < 4 ; suit++ ) {
      if ( HAS_CARD2p(phs,suit, RK_EIGHT) ) { Body += 1 ; } /* HAS_CARD2 Macro in this file uses variable phs */
      if ( HAS_CARD2p(phs,suit, RK_NINE ) ) { Body += 2 ; }
      if ( HAS_CARD2p(phs,suit, RK_TEN  ) ) { Body += 3 ; }
   }
   JGMDPRT(7,"Body=%d, seat=%d %c \n",Body,phs->hs_seat,"NESW"[phs->hs_seat]);
   return Body ;
}
int KnR_Round ( int Body, int KnR_pts ) { /* scale the KnR points into the usual 0 - 40 pt range. Round per Pavlicek */
   int fraction = KnR_pts % 100 ;
   int result   = KnR_pts / 100  ;
   if (fraction >= 60 ) { result += 1 ; } /* more than halfway round up Pav refers to this as 12/20 or higher */
   else if (fraction >= 45 && Body >= 12 ) { result += 1 ; } /* round halfway stuff(9/20, 10/20, 11/20) up if hand has good body */
   /* fraction must be between 0 and 40 here  Result will be the rounded down portion */
   JGMDPRT(7,"ROUNDING KnR_pts=%d, Body=%d, Rounded=%d \n",KnR_pts,Body,result );
   return result ;
}

void show_knr_pts(int dbg_lvl, struct KnR_points_st Pts, char *descr ) {
   JGMDPRT(dbg_lvl, "<<<<<<<Showing KnR Points Struct>>>>>>>: Descr=[%s] \n", descr );
   JGMDPRT(dbg_lvl, "Honor=%d,Short=%d,Qual=%d,Sq=%d,Tot=%d,Dfit=%d,Fn=%d,Misfit=%d,Body=%d,Rounded=%d\n",
      Pts.knr_honor_pts,Pts.knr_short_pts,Pts.knr_qual_pts,Pts.knr_adj,Pts.knr_tot_pts,
      Pts.knr_dfit,Pts.knr_Fn_pts,Pts.knr_misfit_adj,Pts.knr_body_val,Pts.knr_rounded_pts ) ;
   return ;
}
int KnR_Trump_fit(UE_SIDESTAT_k *p_ss, struct KnR_points_st *Ph0, struct KnR_points_st *Ph1 ) {
   /*
    * Per the Text; add 50% or 100% to shortness in supporting hand, 25%,50%,100% to shortness in Declaring hand
    */
   struct KnR_points_st *pKnR_decl, *pKnR_dummy ;
   int h_decl = p_ss->decl_h  ;
   int h_dummy= p_ss->dummy_h ;
   int fit_len = p_ss->t_fitlen;
   
   
   if ( p_ss->t_fitlen < 8  ) { return -1 ; } /* -1 if no 8+ or 5-2 fit  else 7 or the fit length*/

JGMDPRT(7,"KnR_TRUMP FIT FOUND. p_ss->t_len=%d + %d = %d\n",p_ss->t_len[0], p_ss->t_len[1], p_ss->t_len[0]+p_ss->t_len[1] );
    /*
     * if there is a trump fit, calc Dfit and Fn Points.
     */

     
      if  (h_decl == 0) {  pKnR_decl = Ph0 ; pKnR_dummy = Ph1 ; }
      else {               pKnR_decl = Ph1 ; pKnR_dummy = Ph0 ; }

      JGMDPRT(7,"KnR:: h_dummy=%c, h_decl=%c Seat[0] = %d\n",compass[h_dummy], compass[h_decl], seat[0] );

      /* Dummy gets 50% boost if an 8 fit, 100% boost if a 9+ fit Decl gets 25%, 50%, 100%*/
      fit_len = p_ss->t_fitlen ;
      if       (fit_len == 8 ) {
         pKnR_dummy->knr_dfit = pKnR_dummy->knr_short_pts * 0.5 ;  /* Note this may break the assert that total pts div by 5 */
         pKnR_decl->knr_Fn_pts= pKnR_decl->knr_short_pts * 0.25 ;
         JGMDPRT(7,"8FIT: DummyDfit=%d, DeclFN=%d\n", pKnR_dummy->knr_dfit,pKnR_decl->knr_Fn_pts );
      }
      else if ( fit_len == 9 ) {
         pKnR_dummy->knr_dfit = pKnR_dummy->knr_short_pts  ;
         pKnR_decl->knr_Fn_pts= pKnR_decl->knr_short_pts * 0.5 ;
         JGMDPRT(7,"9FIT: DummyDfit=%d, DeclFN=%d\n", pKnR_dummy->knr_dfit,pKnR_decl->knr_Fn_pts );
      }
      else if ( fit_len > 9 )  {
         pKnR_dummy->knr_dfit = pKnR_dummy->knr_short_pts ;
         pKnR_decl->knr_Fn_pts= pKnR_decl->knr_short_pts ;
         JGMDPRT(7,"10FIT: DummyDfit=%d, DeclFN=%d\n", pKnR_dummy->knr_dfit,pKnR_decl->knr_Fn_pts );
      }
      else {
         fprintf(stderr, "***ABORT***  CANT Happen in KnR_Trump_Fit. fit_len = %d\n",fit_len );
         assert(0) ;
      }
      return fit_len;
} /* End KnR Trump Fit */

int knr_calc ( UE_SIDESTAT_k *p_ss ) {      /* Tag Number 7 */
   struct misfit_st KnR_misfit ; /* Misfits are on a per suit basis */
   struct KnR_points_st  KNR_pts[2] ;
   int h, trump_fit;
   HANDSTAT_k *p_hs, *phs[2];
   phs[0] = p_ss->phs[0];
   phs[1] = p_ss->phs[1];
   p_hs   = p_ss->phs[0] ;
   
   JGMDPRT( 7 , "++++++++++ KnR_calc for side= %d; compass[0]=%c, compass[1]=%c, phs[0]=%p, phs[1]=%p, hcp[0]=%d, hcp[1]=%d\n",
               p_ss->side, compass[0],compass[1],(void *)phs[0], (void *)phs[1], phs[0]->hs_totalpoints, phs[1]->hs_totalpoints ) ;
   for (h = 0 ; h < 2 ; h++) {         /* for each hand count up the KnR pts ignoring fit and misfit */
      p_hs = phs[h] ; /* phs array set by prolog to point to hs[north] and hs[south] OR to hs[east] and hs[west] */
      JGMDPRT(7,"++++++++++ Calling KnR_pts_all for hand_idx=%d, hsptr=%p\n",h, (void * )p_hs );
      KNR_pts[h] = KnR_pts_all (p_hs) ; /* Fill the Struct */
      JGMDPRT(7,"------------- KnR_pts_all for hand_idx=%d Done --------------\n",h ) ;

   }
      /* Now check if a trump fit
       * If trump fit calculate calc the Dfit pts and the Fn pts
       * mult the short pts  in each hand by 25%,50%,100% as needed
       * Return the len of the fit; a fit-len of 7 promises a 5-2 fit; a fit-len of 0 means no 8+fit and no 5-2 fit
       */
      trump_fit =  KnR_Trump_fit( p_ss, &KNR_pts[0], &KNR_pts[1] ) ; /* if 8+ fit, calc Dfit and Fn pts; return -1 if no 8+ fit. */
      JGMDPRT(7,"KnR_Trump_fit length = %d \n",trump_fit );
      /* If no 8 + trump fit, check for misfits; trump_fit might be 7 */
      if ( trump_fit < 8 ) {  /* only deduct misfit pts if there is no other fit. RP advice */
         for (int s = CLUBS; s<=SPADES; s++ ) {
            KnR_misfit =  misfit_chk(phs, s) ;
            int sh = KnR_misfit.short_hand ;
            int ss_len = KnR_misfit.sh_len ;
            if (KnR_misfit.mf_type >= 5 ) {   /* assume a deduction if shortness vs 5+ crd suit; not if vs 4 or less suit */
               /* A 5-2 fit with shortness would play in the suit. Deduct the shortpts in the misfit only, not all*/
               if (trump_fit == 7 ) {
                  if      (ss_len == 0 ) { KNR_pts[sh].knr_misfit_adj -= 300 ; }
                  else if (ss_len == 1 ) { KNR_pts[sh].knr_misfit_adj -= 200 ; }
                  else { fprintf(stderr, "CANT HAPPEN in KnR Misfit 7 Fit check ss_len = %d \n",ss_len ); };
                  JGMDPRT(7,"KnR Misfit type=%d, Trump Fit Len=%d, short_hand=%d, ss_len=%d\n",
                           KnR_misfit.mf_type,trump_fit, sh, ss_len );
               }
               else { /* Not even a 7-fit; the hand will be played in NT; deduct ALL shortness pts */
                     KNR_pts[0].knr_misfit_adj -= KNR_pts[0].knr_short_pts ;
                     KNR_pts[1].knr_misfit_adj -= KNR_pts[1].knr_short_pts ;
                     JGMDPRT(7,"KnR Misfit type=%d, Trump Fit Len=%d, short_hand=%d, ss_len=%d BREAKING OUT \n",
                           KnR_misfit.mf_type,trump_fit, sh, ss_len );
                     break ;  /* end for loop now; nothing else to adjust */
               }
            } /* end misfit type test */
         } /* end for loop; might be more than one misfit in either hand */
      } /* end trump_fit < 8 test all misfits, if any, done */
      /* Now do a final total of Dfit, FN_pts, and Misfit adjustments */
      int KnR_side_tot = 0 ;
      int KnR_final_pts ;
      int KnR_final_ptsx100[2];
      for (h = 0 ; h <2 ; h++ ) {
         KnR_final_pts = KNR_pts[h].knr_tot_pts + KNR_pts[h].knr_dfit + KNR_pts[h].knr_Fn_pts + KNR_pts[h].knr_misfit_adj;
         KNR_pts[h].knr_rounded_pts = KnR_Round( KNR_pts[h].knr_body_val, KnR_final_pts ) ;
         KnR_side_tot += KnR_final_pts ;
         KnR_final_ptsx100[h] = KnR_final_pts;
      }
      DBGDO(7,show_knr_pts(7,KNR_pts[0], "Final For Hand[0]") );
      DBGDO(7,show_knr_pts(7,KNR_pts[1], "Final For Hand[1]") );
      JGMDPRT(7,"KnR Metric DONE. Side Total=%d, Filling uservals area with results \n",KnR_side_tot);
      UEv.misc_count = 0 ;
     /* Put the KNR vallues in the same general slots as the others. Makes for easier scripting. */
      p_uservals->u.res[1] = KnR_Round( KNR_pts[0].knr_body_val, KNR_pts[0].knr_tot_pts );  /* N-NT pts rounded to 0-40 pt range */
      p_uservals->u.res[2] = KnR_Round( KNR_pts[1].knr_body_val, KNR_pts[1].knr_tot_pts );  /* S-NT pts */
      p_uservals->u.res[0] = p_uservals->u.res[1] +  p_uservals->u.res[2] ; /* the NT total in a 0-40 pt range*/
      p_uservals->u.res[3] = KNR_pts[0].knr_rounded_pts + KNR_pts[1].knr_rounded_pts ; /* The HLDF total rounded to 0-40 pt range*/
      p_uservals->u.res[4] = KNR_pts[0].knr_rounded_pts; /* North-hldf pts */
      p_uservals->u.res[5] = KNR_pts[1].knr_rounded_pts; /* South-hldf pts */
      p_uservals->u.res[6] = KnR_side_tot ;  /* The x100 version of the complete KnR Points */
      p_uservals->u.res[7] = KnR_final_ptsx100[0]; /* The x100 version for north */
      p_uservals->u.res[8] = KnR_final_ptsx100[1]; /* The x100 version for south */
      /* hand[0] details only missing adj for square hand*/
      p_uservals->u.res[9]  = KNR_pts[0].knr_honor_pts;
      p_uservals->u.res[10] = KNR_pts[0].knr_short_pts;
      p_uservals->u.res[11] = KNR_pts[0].knr_qual_pts;
      p_uservals->u.res[12] = KNR_pts[0].knr_tot_pts ;
      p_uservals->u.res[13] = KNR_pts[0].knr_dfit;
      p_uservals->u.res[14]= KNR_pts[0].knr_Fn_pts;
      p_uservals->u.res[15]= KNR_pts[0].knr_misfit_adj;
      p_uservals->u.res[16]= KNR_pts[0].knr_body_val ;
      /* hand[1] details only missing adj for square hand */
      p_uservals->u.res[17]= KNR_pts[1].knr_honor_pts;
      p_uservals->u.res[18]= KNR_pts[1].knr_short_pts;
      p_uservals->u.res[19]= KNR_pts[1].knr_qual_pts;
      p_uservals->u.res[20]= KNR_pts[1].knr_tot_pts ;
      p_uservals->u.res[21]= KNR_pts[1].knr_dfit;
      p_uservals->u.res[22]= KNR_pts[1].knr_Fn_pts;
      p_uservals->u.res[23]= KNR_pts[1].knr_misfit_adj;
      p_uservals->u.res[24]= KNR_pts[1].knr_body_val ;
         /* also put mainresults in UEv structure in usual slots for possible use by other functions such as set88 etc.*/
      UEv.hldf_pts_side    = p_uservals->u.res[3] ;
      UEv.hldf_pts_seat[0] = p_uservals->u.res[4] ;
      UEv.hldf_pts_seat[1] = p_uservals->u.res[5] ;
      UEv.nt_pts_side      = p_uservals->u.res[0] ;
      UEv.nt_pts_seat[0]   = p_uservals->u.res[1] ;
      UEv.nt_pts_seat[1]   = p_uservals->u.res[2] ;
      UEv.hldf_suit   = p_ss->t_suit;     /* So we know which dds tricks to count if we are playing in a suit */
      UEv.hldf_fitlen = p_ss->t_fitlen ;

      #ifdef JGMDBG
      if(jgmDebug >= 7 ) {
         fprintf(stderr, "%s:%d KnR_calc Final Results {jgmDebug=%d} Prod_num=%d\n",__FILE__,__LINE__ , jgmDebug,prod_num ) ;
         for (int i = 0 ; i < 24 ; i++ ) {
            fprintf(stderr, "[%d]=%d, ",i, p_uservals->u.res[i] );
            if( (i+1)%10 == 0 ) {fprintf(stderr, "\n"); }
         }
      }
      #endif
      return 1 ;
}



