/* File zar_calc.c */
/* Date        Version  Author   Description
 * 2024/08/12  1.0      JGM      Extracted from metrics_calcs.c and factors.c; Using the UE_SIDESTAT functionality
 *                               Returns the Zar/2 Pav_rounded results as well as the Raw results
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
extern int Pav_round(float val, int body ) ;
extern float_t shortHon_adj( HANDSTAT_k *p_hs, int suit, int mtag ) ;
extern void SaveUserVals(struct UserEvals_st UEv , USER_VALUES_k *p_ures ) ;
extern void set_dbg_names(int m, char *dbg_func) ;
int didxsort4( int v[4], int x[4] ) ;

 /* Forward Function Definitions */
int Fn_ptsZar( UE_SIDESTAT_k *p_ss,  int t_suit) ;
int DfitZAR(   UE_SIDESTAT_k *p_ss,  int h     ) ;
int Hf_ptsZar( HANDSTAT_k    *phs[]            ) ;
int SynZAR(    HANDSTAT_k    *p_hs             ) ;

/* Note that an Opening Zar hand is 26 Zar pts, and it takes 52 Zars to bid game.
 * Each extra level of bidding requires 6 Zars. The conclusion is (per Pavlicek) that each Zar pt is worth half a 'regular' point
 * and to compare Zar evaluation to others, just divide by two, then use Pav_round to get the final result
 */

// main or userfunc has a) set all the mm_ptrs etc. b) used prolog to zero globals c) filled sidestat global with trump suit choice.

/* This code makes several assumptions about edge cases that I could not get answered by ZP.
 * 1) Certain HH honor combos. AK? AQ? AJ? Downgraded or not? Assume AJ, not the other two.
 * 2) What HCP to use for the 2/3 suit adj. The raw or the Downgraded ones (assume raw; its simpler)
 * 3) Can Declarer get Hf points in Dummy's 4+ side suit? (assume yes) How long must the fit be? (assume 7+)
 * 4) In a 4-4 fit can Both hands get Hf pts (e.g. Kxxx vs Qxxx). Assume yes, to a max of 2 in the suit total.
 * 5) The Zar doc sometimes gives (2-d) pts per extra trump sometimes (3-d). Assume 3-d. (Pavlicek assumes 2-d)
 * 6) In the case of an 8 fit, Dummy counts Zar Ruffing with 3+ trump; Declarer with 6+ trump
 * 7) This code does not consider any Misfit points. Either Positive or Negative.
 * 8) The hand with the most length in trump is assumed to be Declarer. With Equal length, the stronger hand is Declarer. Else North/East.
 */
int zarbas_calc( UE_SIDESTAT_k *p_ss ) {   /* Tag Number: 12 Two Hands independent. No HF, no Fn, No Dfit in the basic Zar */
   int zar_pts[2]   = {0}; /* Hand value if played in a suit */
   int zar_ctls[2]  = {0};
   int zar_Dist[2]  = {0};
   int zar_syn_pts[2] = {0}; /* Point for all HCP being in 2 or 3 suits or for 25 Zars and 4 card spade suit */
   int half_zars ;
   zero_globals( p_ss->side ) ;    
   int h, s;
   HANDSTAT_k *p_hs, *phs[2];
   phs[0] = p_ss->phs[0];
   phs[1] = p_ss->phs[1];
   p_hs   = p_ss->phs[0] ;
   set_dbg_names(12, "zarbas_calc" ) ; 
   JGMDPRT(7 , "++++++++++ ZAR   calc for p_ss->side= %d; compass[0]=%c, compass[1]=%c, phs[0]=%p, phs[1]=%p, hcp[0]=%d, hcp[1]=%d\n",
               p_ss->side, compass[0],compass[1],(void *)phs[0], (void *)phs[1], phs[0]->hs_totalpoints, phs[1]->hs_totalpoints ) ;

   for (h = 0 ; h < 2 ; h++) {         /* for each hand */
      p_hs = phs[h] ; /* phs array set by prolog to point to hs[north] and hs[south] OR to hs[east] and hs[west] */
      zar_ctls[h] = p_hs->hs_totalcontrol ;
      for (s = CLUBS ; s<= SPADES ; s++ ) {
         fhcp_suit[h][s]  = p_hs->hs_points[s]  ;
         fhcp_adj_suit[h][s] = shortHon_adj(p_hs, s, ZARBAS ) ; /* Deduct in a std way in case we play the hand in NT */
         fhcp[h] += fhcp_suit[h][s] ;
         fhcp_adj[h] += fhcp_adj_suit[h][s] ;

         JGMDPRT(8,"zarbas_calc, Hand=%d, suit=%d, SuitLen=%d, fhcp[h][s]=%g, fhcp-adj[h][s]=%g\n",
                     h, s, p_hs->hs_length[s], fhcp_suit[h][s], fhcp_adj_suit[h][s] );
      } /* end CLUBS <= s <= SPADES */

      
      zar_Dist[h] = p_ss->sorted_slen[h][0]*2 + p_ss->sorted_slen[h][1] - p_ss->sorted_slen[h][3] ;
      hcp_adj[h] = roundf(fhcp_adj[h]) ;
      hcp[h] = roundf( fhcp[h] + fhcp_adj[h] ) ;
      zar_syn_pts[h] = SynZAR( p_hs ) ; /* No Answ from ZP re AKxxx/KQxx/Kx/Jx - 16 HCP corrected to 15. +1 for 3 suits? */

      zar_pts[h] = hcp[h] + zar_ctls[h] + zar_Dist[h] + zar_syn_pts[h];
      if (25 == zar_pts[h] && p_hs->hs_length[SPADES] >= 4 ) {zar_pts[h] += 1 ; zar_syn_pts[h] += 1 ; }
      JGMDPRT(7,"ZARBAS hand=%d,zarbas_pts_net=%d, net_hcp=%d, hcp_adj=%d, CtlPts=%d, Dist_pts=%d Syn_pts=%d\n",
                  h, zar_pts[h],hcp[h],hcp_adj[h],zar_ctls[h], zar_Dist[h], zar_syn_pts[h] );
   } /* end for each hand */
   UEv.nt_pts_seat[0] = zar_pts[0];
   UEv.nt_pts_seat[1] = zar_pts[1];
   UEv.nt_pts_side = UEv.nt_pts_seat[0] + UEv.nt_pts_seat[1] ;
   /* for basic Zar pts there is no extra calculations for trump fit, and so on. so NT pts are same as Suit Pts.
    * however the mainline UserEval code has already calculated a trump suit at this point and put it in UEsidestat structure
    */
    
   UEv.hldf_pts_seat[0] = zar_pts[0] ;
   UEv.hldf_pts_seat[1] = zar_pts[1] ;
   UEv.hldf_pts_side = zar_pts[0] + zar_pts[1] ;
   UEv.hldf_suit   = p_ss->t_suit;     /* So we know which dds tricks to count if we are playing in a suit */
   UEv.hldf_fitlen = p_ss->t_fitlen ;
   /* calculate the zar points on a more common scale BF: Side, N, S ::: NT: Side, N, S */
   UEv.misc_count = 0 ;
   for (h=0 ; h < 2 ; h++ ) {  /* but put the rounded zar/2 pts in both anyway for consistency and easy remembering*/
      half_zars = Pav_round(  0.5 * (float_t)UEv.hldf_pts_seat[h] ,p_ss->pav_body[h] ) ;
      JGMDPRT(8,"Scaling HLDFpts[%d]=%d with body=%d Result=%d \n",h, UEv.hldf_pts_seat[h], p_ss->pav_body[h], half_zars );
      UEv.misc_pts[1+h] = half_zars ;  
   }
   UEv.misc_pts[0] = UEv.misc_pts[1] + UEv.misc_pts[2] ;  /* 6,7,8 side, hand[0], hand[1] in BF */
   JGMDPRT(7,"ZarBas Rounded HLDF pts Res[6,7,8]=> %d = %d + %d\n",UEv.misc_pts[0], UEv.misc_pts[1], UEv.misc_pts[2] ); 
   for (h=0 ; h < 2 ; h++ ) {  
       half_zars = Pav_round(  0.5 *((float_t)UEv.nt_pts_seat[h]) ,p_ss->pav_body[h] ) ;
       JGMDPRT(8,"Scaling NTpts[%d]=%d with body=%d Result=%d \n",h, UEv.nt_pts_seat[h], p_ss->pav_body[h], half_zars );
       UEv.misc_pts[4+h] = half_zars ;  // results u.res[9,10,11] are the HALF ZARs for zarbas metric NT saved in UEv_misc[3,4,5]
   }
   UEv.misc_pts[3] = UEv.misc_pts[4] + UEv.misc_pts[5] ;  /* 10,11,12side, hand[0], hand[1] in NT */
   JGMDPRT(7,"ZarBas Rounded NT   pts Res[6,7] => %d = %d + %d \n",UEv.misc_pts[3], UEv.misc_pts[4], UEv.misc_pts[5] );
      UEv.misc_count = 6 ;
   /* Debugging vars */
   /* The factors that apply to both NT and Suit */
      UEv.misc_pts[UEv.misc_count++] = hcp_adj[0];     // Result[10]
      UEv.misc_pts[UEv.misc_count++] = zar_ctls[0];
      UEv.misc_pts[UEv.misc_count++] = zar_Dist[0];
      UEv.misc_pts[UEv.misc_count++] = zar_syn_pts[0];
      UEv.misc_pts[UEv.misc_count++] = p_ss->pav_body[0];  // to check rounding calcs
       /* ditto hand 1 */
      UEv.misc_pts[UEv.misc_count++] = hcp_adj[1];
      UEv.misc_pts[UEv.misc_count++] = zar_ctls[1];
      UEv.misc_pts[UEv.misc_count++] = zar_Dist[1];
      UEv.misc_pts[UEv.misc_count++] = zar_syn_pts[1]; 
      UEv.misc_pts[UEv.misc_count++] = p_ss->pav_body[1]; // Result[19]


  /* now put the results into the user result area at p_uservals */
  SaveUserVals( UEv , p_uservals ) ;
  JGMDPRT(7,"ZARBAS pts ALL done. pts[0]=%d, pts[1]=%d, UEv_NTSide_pts=%d MiscCount=%d\n",
                        zar_pts[0],zar_pts[1], UEv.nt_pts_side, UEv.misc_count );
  return ( 6 + UEv.misc_count ) ;
} /* end zarbas_calc */

int zaradv_calc( UE_SIDESTAT_k  *p_ss ) {   /* Tag Number: 13 */
   JGMDPRT(8,"------- ZarAdv calling ZarBas------\n");
   zarbas_calc( p_ss ) ;  /* fill the UEv struct with the basic stuff */
   
   /* the basic zar pts are in the UEv.nt_pts_side, and UEv.nt_pts_seat[h] */
   /* Now add in Fn_pts, Hf_pts, Dfit_pts */
   /* dont zero globals from zarbas? because we might need some of them? or zero them anyway? */
   HANDSTAT_k *phs[2];
   phs[0] = p_ss->phs[0];
   phs[1] = p_ss->phs[1];

   int h;
   int zHf_pts, zDfit_pts ;
   int half_zars;
   set_dbg_names(13, "zaradv_calc" ) ;

   JGMDPRT(7,"ZARADV ztrump_suit=%d, decl=%d, t_fitlen=%d\n",p_ss->t_suit, p_ss->decl_h ,p_ss->t_fitlen );
   Fn_pts[1] = Fn_ptsZar( p_ss, p_ss->t_suit ) ;   /* one or two points for a secondary fit Arbitrary choose hand 1*/
   zHf_pts  =  Hf_ptsZar( phs ) ;                  /* Sets the globals hf_pts[2] */
   zDfit_pts = DfitZAR(   p_ss, p_ss->dummy_h ) ;  /* Sets the globals dfit_pts[2] */
   JGMDPRT(7,"ZARADV zFn_pts=%d, zHf_pts=%d, zDfit_pts=%d\n",Fn_pts[1], zHf_pts, zDfit_pts );
   /* now add the extra pts to the UEv array -- Much of the global UEv array will have been set by zarbas() already */
   UEv.hldf_pts_seat[0] += hf_pts[0] + dfit_pts[0] ;
   UEv.hldf_pts_seat[1] += hf_pts[1] + dfit_pts[1]  + Fn_pts[1] ; /* arbitrarily give the Fn pts to Hand 1 */
   UEv.hldf_pts_side = UEv.hldf_pts_seat[0] + UEv.hldf_pts_seat[1],
   UEv.hldf_suit   = p_ss->t_suit;     /* So we know which dds tricks to count if we are playing in a suit */
   UEv.hldf_fitlen = p_ss->t_fitlen ;
   
   /* calculate the zar points on a more common scale   BF: Side, N, S ::: NT: Side, N, S 
    * Overwrite the ones that zarbas put there since our pts are different                */
   UEv.misc_count = 0 ;
   for (h=0 ; h < 2 ; h++ ) {  /* but put the rounded zar/2 pts in both anyway for consistency and easy remembering*/
      half_zars = Pav_round(  0.5 * (float_t)UEv.hldf_pts_seat[h] ,p_ss->pav_body[h] ) ;
      JGMDPRT(8,"Scaling HLDFpts[%d]=%d with body=%d Result=%d \n",h, UEv.hldf_pts_seat[h], p_ss->pav_body[h], half_zars );
      UEv.misc_pts[1+h] = half_zars ;  
   }
   UEv.misc_pts[0] = UEv.misc_pts[1] + UEv.misc_pts[2] ;  /* 6,7,8 side, hand[0], hand[1] in BF */
   JGMDPRT(7,"ZarBas Rounded HLDF pts Res[6,7,8] => %d = %d + %d \n",UEv.misc_pts[0], UEv.misc_pts[1], UEv.misc_pts[2] ); 
   for (h=0 ; h < 2 ; h++ ) {  
       half_zars = Pav_round(  0.5 *((float_t)UEv.nt_pts_seat[h]) ,p_ss->pav_body[h] ) ;
       JGMDPRT(8,"Scaling NTpts[%d]=%d with body=%d Result=%d \n",h, UEv.nt_pts_seat[h], p_ss->pav_body[h], half_zars );
       UEv.misc_pts[4+h] = half_zars ;  // results u.res[9,10,11] are the HALF ZARs for zarbas metric NT saved in UEv_misc[3,4,5]
   }
   UEv.misc_pts[3] = UEv.misc_pts[4] + UEv.misc_pts[5] ;  /* 10,11,12side, hand[0], hand[1] in NT */
   JGMDPRT(7,"ZarBas Rounded NT   pts Res[6,7] => %d = %d + %d \n",UEv.misc_pts[3], UEv.misc_pts[4], UEv.misc_pts[5] );
      UEv.misc_count = 6 ;
   /* Debugging vars */
     /* Debugging vars in addition to the ones already there by zarbas*/

   UEv.misc_pts[UEv.misc_count++] = Fn_pts[0];  // Result[20]
   UEv.misc_pts[UEv.misc_count++] = hf_pts[0];
   UEv.misc_pts[UEv.misc_count++] = dfit_pts[0];
   UEv.misc_pts[UEv.misc_count++] = Fn_pts[1];
   UEv.misc_pts[UEv.misc_count++] = hf_pts[1];
   UEv.misc_pts[UEv.misc_count++] = dfit_pts[1]; // result[25]

   UEv.hldf_suit   = p_ss->t_suit;     /* So we know which dds tricks to count if we are playing in a suit */
   UEv.hldf_fitlen = p_ss->t_fitlen ;  /* will be put into slots 126, 127 by SaveUserVals since there is room */
  /* now put the results into the user result area at p_uservals */
  SaveUserVals( UEv , p_uservals ) ;
  JGMDPRT(6,"ZARADV pts ALL done. pts[0]=%d, pts[1]=%d, UEv_Side_pts=%d\n", UEv.hldf_pts_seat[0],UEv.hldf_pts_seat[1], UEv.nt_pts_side );
  return ( 6 + UEv.misc_count ) ;
} /* end zaradv_calc */

/* add a pt if all the HCP are in 2 or 3 suits similar to Larsson */
int SynZAR( HANDSTAT_k *p_hs ) {
   int syn_cnt = 0 ;
   int syn_pts = 0 ;
   int s ;
   for (s=CLUBS; s<=SPADES; s++ ) {
      if (p_hs->hs_points[s] > 0 ) syn_cnt++ ;
   }
   if (11 <= p_hs->hs_totalpoints && p_hs->hs_totalpoints <= 14 && syn_cnt <= 2 ) {syn_pts = 1 ; }
   else if ( p_hs->hs_totalpoints >= 14 && syn_cnt <= 3 ) {syn_pts = 1 ; }
   JGMDPRT(8,"ZarSyn:: TotPts=%d, syn_cnt=%d, syn_pts=%d\n",p_hs->hs_totalpoints,syn_cnt,syn_pts );
   return syn_pts ;
} /* end SynZAR */

int Fn_ptsZar(UE_SIDESTAT_k *p_ss,  int t_suit) {
   /* add 1 pt for a second 8 fit, 2Pts for a second 9/up fit JGM version */
   /* Pav says +1 for second 9fit +2 for 2nd 10 fit; will that ever happen? */
   /* Zar says +3 HC points for a secondary 7+ fit; but that may be old info. JGM invented the following as a compromise */
   int MaxFit = 0 ;
   int fitpt = 0 ;
   int s, fitlen ;
   for (s=CLUBS; s<=SPADES; s++) {
      if( s == t_suit ) { continue ; }
      fitlen = phs[0]->hs_length[s] + phs[1]->hs_length[s] ;
      if (fitlen > MaxFit) {MaxFit = fitlen ; }
   }
   fitpt = (MaxFit > 8) ? 2 :
           (MaxFit ==8) ? 1 : 0 ;
   JGMDPRT(8,"%d Zar Fn Pts for 2nd fit of len=%d\n",fitpt, MaxFit);
   return fitpt ;
}
/* Hf Zar; Does not depend on trump suit or who is dummy; any suit with a 7+ fit, and both hands can have Hf Pts */
int Hf_ptsZar( HANDSTAT_k *phs[] ) {
   /* Either hand (but not both in same suit) can add +1 for each honor,incl Ten, (max of 2 pts per suit)
    * in a suit where partner has shown 4+ cards. Also Short Honor Deductions are Reversed if the short honor is facing a 4+ suit 
    * Max of two such suits per hand.
    * In this code I only add Hf pts to the shorter hand; e.g. QJTxxx facing AKxx only 2 Hf pts total, not 4.
    * In a 4-4 Fit I add Hf only to the hand with the FEWER Tops or HCP. AKJx facing QTxx only Q, T count = 2Hf pts.
    * a holding like Jx which starts out as being worth zero (1 - 1 dwngrade) now becomes worth +2 if pard shows 4+ there.
    * See page 84 ex2 in pdf where Jx facing AKxx becomes 1+1=2, instead of 1 - 1 = 0 (so Stiff J facing xxxx becomes 2pts!)
    * reverse the downgrade and add +1 for an honor in pards long suit(s)
    * Pavlicek website is much more conservative with the Hf points only Dummy in the trump suit.
    */
   int suit_idx[2][4] = { {0,1,2,3}, {0,1,2,3} } ;
   int top5[2] = {0, 0};   
   int s , h_Hf;
   float_t temp = 0;

   /* Track the Hf pts by seat so that we know if our hand has improved enough for an invite etc. */
   /* Use the globals hf_pts[2] and hf_pts_suit[2][4] */
   for (s = CLUBS; s <= SPADES ; s++ ) {
      if ( (4 > phs[0]->hs_length[s]) && (4 > phs[1]->hs_length[s]) )  { continue ; } /* bypass suits where Neither has 4+ */
      top5[0] = (phs[0]->hs_counts[idxTop5][s] < 2 ) ? phs[0]->hs_counts[idxTop5][s] : 2 ; /* count max of two honors per suit */
      top5[1] = (phs[1]->hs_counts[idxTop5][s] < 2 ) ? phs[1]->hs_counts[idxTop5][s] : 2 ;
      /* Assign Hf only to the hand with the shorter suit. */
      if      ( phs[0]->hs_length[s] > phs[1]->hs_length[s] ) {  h_Hf = 1 ; }
      else if ( phs[0]->hs_length[s] < phs[1]->hs_length[s] ) {  h_Hf = 0 ; }
      /* Both hands equal length.(e.g 4=4 fit) Assign Hf to hand with FEWER honors */
      else if ( phs[0]->hs_counts[idxTop5][s] > phs[1]->hs_counts[idxTop5][s] ) { h_Hf = 1 ;}
      else if ( phs[0]->hs_counts[idxTop5][s] < phs[1]->hs_counts[idxTop5][s] ) { h_Hf = 0 ;}
      /* Both hands equal Length, equal honors e.g. Axxx vs Kxxx - Give Hf to hand with Weaker suit */
      else if ( phs[0]->hs_points[s] > phs[1]->hs_points[s] ) { h_Hf = 1 ;}
      else    { h_Hf = 0 ;}
      JGMDPRT(7,"ZHf_Pts::Suit=%c, FitLen=%d, Top5[0]=%d, Top5[1]=%d, h_Hf=%d\n","CDHS"[s],(phs[0]->hs_length[s] + phs[1]->hs_length[s]),
                                     top5[0],top5[1],h_Hf) ;
      /* we have Honor(s) facing a 4+ suit This code even counts Stiff J facing xxxx since the PDF implies this is the case */
      if (phs[h_Hf]->hs_length[s] < 3 ) { temp = shortHon_adj(phs[h_Hf], s, ZARBAS) ;} /* we need to reverse the deduction we previously made if any */
      else { temp = 0.0 ; }  /* there was no  ded if the suit_len >= 3 */
      hf_pts_suit[h_Hf][s] = top5[h_Hf] - (int)temp ;  /* top5 <= 2 here; temp is negative */
      JGMDPRT(6,"ZHf_Pts:: %d Hf Points assigned to hand=%d in suit =%d reversing %g deduction\n",hf_pts_suit[h_Hf][s], h_Hf, s,temp);
   }
   didxsort4(hf_pts_suit[0], suit_idx[0]);            /* sort the HfPts for hand 0 in Desc order We need the sorted suit_ids also*/
   didxsort4(hf_pts_suit[1], suit_idx[1]);            /* sort the HfPts for hand 1 in Desc order */
   hf_pts[0] = hf_pts_suit[0][0] + hf_pts_suit[0][1] ;      /* keep only the two suits with the highest Hf */
   hf_pts[1] = hf_pts_suit[1][0] + hf_pts_suit[1][1]  ;     /* keep only the two suits with the highest Hf */
   JGMDPRT(9,"Zar Hf for hand[0]=%d, for hand[1]=%d\n", hf_pts[0], hf_pts[1] ) ;
   return (hf_pts[0] + hf_pts[1]) ;
} /* end Hf_ptsZar */

/* Dfit = ( ExtraTrumps ) * ( 3 - shortestSuitLength) :: Both hands can do this */
int DfitZAR( UE_SIDESTAT_k *p_ss, int h)  { /* h unused place holder */
   /* Use the global dfit_pts[2] */
   int du, dc;
   int excess[2] ;
   dc = p_ss->decl_h ;
   du = p_ss->dummy_h;
   excess[dc] = p_ss->t_len[dc] - 5 ; if (excess[dc] < 0 ) excess[dc] = 0 ;
   excess[du] = p_ss->t_len[du] - 3 ; if (excess[du] < 0 ) excess[du] = 0 ;
   if(p_ss->t_fitlen > 8 ) {  /* a 9+ fit will always have at least one hand with 'excess' trump */
      dfit_pts[du] = excess[du] * (3 - p_ss->sorted_slen[du][3] ) ; /* 3 - len of shortest suit */
      dfit_pts[dc] = excess[dc] * (3 - p_ss->sorted_slen[dc][3] ) ;
   }
   else if ( p_ss->t_fitlen == 8 ) { /* Even if Dummy has the minimum trump holding can still get ruffing value */
      /* A 6-2 fit Declarer does not get any Dfit because Partner would not raise. */
      if ( (p_ss->t_len[du] >= 3) && ((p_ss->t_len[du] - p_ss->sorted_slen[dc][3] ) >= 2 ) ) {
         dfit_pts[du] = (p_ss->t_len[du] - p_ss->sorted_slen[dc][3] ) ; /* [3] is the shortest length */
      }
   } /* end 8 fit */
   JGMDPRT(9,"Dfitzar:: fitlen=%d, decl=%d, Du=%d, XS_dc=%d, XS_du=%d, SSlenDC=%d, SSlenDU=%d, DfitDC=%d, DfitDU=%d\n",
            p_ss->t_fitlen,dc,du,excess[dc],excess[du],p_ss->sorted_slen[dc][3],p_ss->sorted_slen[du][3],dfit_pts[dc],dfit_pts[du] );
   return (dfit_pts[0] + dfit_pts[1] ) ;
} /* end DfitZAR */
