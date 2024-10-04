/* File Kaplan_calc.c */
/* Date        Version  Author   Description
 * 2024/08/12  1.0      JGM      Extracted from metrics_calcs.c and factors.c; Using the UE_SIDESTAT functionality
 *
 */

#include "../include/std_hdrs_all.h"
#include "../include/UserEval_types.h"
#include "../include/UserEval_externs.h"
#include "../include/dbgprt_macros.h"
#include "../include/mmap_template.h"

/* External functions -- metrics_util_subs.c , short_honors_subs.c */
extern void zero_globals( int side ) ; 
extern int Pav_round(float val, int body ) ;
extern float_t shortHon_adj (HANDSTAT_k *phs, int suit, int tag ) ;
extern void SaveUserVals(struct UserEvals_st UEv , USER_VALUES_k *p_ures ) ;
extern void set_dbg_names(int m, char *dbg_func) ;
/* forward func defs */
int Hf_ptsKAPLAN( UE_SIDESTAT_k *p_ss, int h) ;
int DfitKAPLAN(   UE_SIDESTAT_k *p_ss, int h) ;

/* Kaplan per the Book Winning Contract Bridge Complete. 1960. Counts for Long suits and shortness once a fit is found */

int kaplan_calc (UE_SIDESTAT_k *p_ss ) {    /* Tag Number: 5 */
   int kaplan_pts[2] = {0} ;
   /* code uses global temporary vars .. see globals.c */
   int h ;
   int s ;
   zero_globals( p_ss->side ) ; 
   HANDSTAT_k *p_hs, *phs[2];
   phs[0] = p_ss->phs[0];
   phs[1] = p_ss->phs[1];
   p_hs   = p_ss->phs[0] ;
   set_dbg_names(4,"kaplan_calc");
   JGMDPRT( 7 , "++++++++++ Kaplan_calc for p_ss->side= %d; compass[0]=%c, compass[1]=%c, phs[0]=%p, phs[1]=%p, hcp[0]=%d, hcp[1]=%d, fhcp=[%g,%g]\n",
               p_ss->side, compass[0],compass[1],(void *)phs[0], (void *)phs[1], phs[0]->hs_totalpoints, phs[1]->hs_totalpoints, fhcp[0],fhcp[1] ) ;
   for (h = 0 ; h < 2 ; h++) {         /* for each hand */
      p_hs = phs[h] ;

      for (s = CLUBS ; s<= SPADES ; s++ ) {
         fhcp_suit[h][s] =  p_hs->hs_points[s]  /* TBD?? why not just use p_hs->hs_totalpoints ?? like Larsson ?? */;
         fhcp_adj_suit[h][s] = shortHon_adj(p_hs, s, KAPLAN ) ; /* Stiff Q, J = 0; (unless pard bids the suit) QJ, Qx=2, Jx=1 */
         /* simpler to do inline than call function */
         if (p_hs->hs_length[s] > 4 ) {
            lpts_suit[h][s] = (p_hs->hs_length[s] - 4 ) ; // +1 for each card over 4 in each suit.
         }
         lpts[h] += lpts_suit[h][s] ;
         fhcp_adj[h] += fhcp_adj_suit[h][s] ;
         fhcp[h] += fhcp_suit[h][s] ;
         JGMDPRT(8,"L_ptsKAPLAN, Hand=%d, suit=%d, len=%d, lpts_suit=%d,RunTot_Lpts=%d, RunTotHCP=%g, RunTotAdj=%g\n ",
                      h, s, p_hs->hs_length[s], lpts_suit[h][s],lpts[h],fhcp[h], fhcp_adj[h]);
      } /* end for s = CLUBS to SPADES*/
      hcp_adj[h] =  Pav_round( fhcp_adj[h], p_ss->pav_body[h] ) ;  // roundf( fhcp_adj[h] ) ;
      hcp[h] = Pav_round( fhcp[h], p_ss->pav_body[h] ) ;
      hf_pts[h] = Hf_ptsKAPLAN(p_ss, h ) ; /* give any Stiff Q or J in pards 4+ suit full value instead of zero */
      kaplan_pts[h] = hcp[h] + hcp_adj[h] + lpts[h] + hf_pts[h]; /* NT pts for hand h */
      JGMDPRT(7,"KAPLAN hand=%d,kaplan_pts_net=%d, hcp=%d, hcp_adj=%d, Lpts=%d, Hf_pts=%d\n",
                  h, kaplan_pts[h],hcp[h],hcp_adj[h],lpts[h], hf_pts[h] );
      UEv.nt_pts_seat[h] = kaplan_pts[h] ;
   } /* end for each hand */

   UEv.nt_pts_side = UEv.nt_pts_seat[0] + UEv.nt_pts_seat[1] ;
   JGMDPRT(7,"KAPLAN NT pts done. pts[0]=%d, pts[1]=%d, UEv_Side_pts=%d\n", kaplan_pts[0],kaplan_pts[1], UEv.nt_pts_side );

   /* Done both hands -- Now check for a trump fit */
     if (p_ss->t_fitlen >= 8 ) {  /* Kaplan only gives Dfit/Fn for 8+ fits */
      dfit_pts[0] = DfitKAPLAN(p_ss, 0 ) ;
      kaplan_pts[0] += dfit_pts[0] ;
      dfit_pts[1] = DfitKAPLAN(p_ss, 1 ) ;
      kaplan_pts[1] += dfit_pts[1] ;
   }

   UEv.hldf_pts_seat[0] = kaplan_pts[0] ;
   UEv.hldf_pts_seat[1] = kaplan_pts[1] ;
   UEv.hldf_pts_side = kaplan_pts[0] + kaplan_pts[1] ;
   UEv.hldf_suit   = p_ss->t_suit;     /* So we know which dds tricks to count if we are playing in a suit */
   UEv.hldf_fitlen = p_ss->t_fitlen ;
   UEv.misc_count = 0 ;
      /* The factors that apply to both NT and Suit */
      UEv.misc_pts[UEv.misc_count++] = hcp_adj[0];
      UEv.misc_pts[UEv.misc_count++] = lpts[0];
      UEv.misc_pts[UEv.misc_count++] = hf_pts[0];
      /* Factors that apply to suit contracts only */
      UEv.misc_pts[UEv.misc_count++] = dfit_pts[0];   /* Dummy support pts typically 3-2-1 with 3, 5-3-1 with 4+ */

       /* ditto hand 1 */
      UEv.misc_pts[UEv.misc_count++] = hcp_adj[1];
      UEv.misc_pts[UEv.misc_count++] = lpts[1];
      UEv.misc_pts[UEv.misc_count++] = hf_pts[1];
      UEv.misc_pts[UEv.misc_count++] = dfit_pts[1];

  /* now put the results into the user result area at p_uservals */
  SaveUserVals( UEv , p_uservals ) ;
     JGMDPRT(7, "Kaplan fit calcs Done; Tsuit=%d, HLDFpts[%d : %d], Dfit_pts=[%d:%d]\n",
         p_ss->t_suit, kaplan_pts[0],kaplan_pts[1],dfit_pts[0],dfit_pts[1] );
  return ( 6 + UEv.misc_count ) ;
} /* end kaplan_calc */


int Hf_ptsKAPLAN ( UE_SIDESTAT_k *p_ss , int h) { /*give any Stiff Q or J in pards 4+ suit full value instead of zero */
   int Hf_pts = 0;
   int h_pard = 0 ;
   int sh_cnt = 0 ;
   if ( 0 == h ) { h_pard = 1 ; }
   for (int s = CLUBS; s <= SPADES ; s++ ) {
      if (p_ss->phs[h]->hs_length[s] == 1 && p_ss->phs[h]->Has_card[s][QUEEN] && p_ss->phs[h_pard]->hs_length[s] >= 4 ) {
         Hf_pts += 2 ;
         sh_cnt++ ;
      }
      if (p_ss->phs[h]->hs_length[s] == 1 && p_ss->phs[h]->Has_card[s][JACK] && p_ss->phs[h_pard]->hs_length[s] >= 4 ) {
         Hf_pts += 1 ;
         sh_cnt++ ;
      }
   }
   JGMDPRT(7,"Hf_ptsKAPLAN Hand=%d, Short Honors=%d, Returning %d Tot_Hf pts \n",h,sh_cnt, Hf_pts ) ;
   return Hf_pts ;
}
/* Kaplan Dfit: given an 8+ fit add for ALL short suits 3/2/1. Add same to Opener, ALL shortsuits, 3/2/1 */
/* call this routine only if there is an 8 fit ; assume that if we are in a 6-2 fit 6 card suit still gets Dfit */
int DfitKAPLAN( UE_SIDESTAT_k *p_ss, int h ) { /* routine called twice, once with h=0, once with h=1 */
   int  t_suit, t_len, s, s_len, dfit ;
   HANDSTAT_k *p_hs ;
   p_hs = p_ss->phs[h];
   int DF_KAPLAN[4] =  {3,2,1,0}  ; /* Dfit pts with 3/4 trump for V/S/D/Longer */
   t_suit = p_ss->t_suit;
   t_len = p_hs->hs_length[t_suit] ;
   if (t_len <= 2 ) return 0 ;
   dfit = 0 ;
   for (s=CLUBS; s<=SPADES; s++ ) {  /* count for EACH shortness */
      if (s == t_suit) continue ;
      s_len = p_hs->hs_length[s];
      if (s_len < 3 ) {
         dfit += DF_KAPLAN[s_len];
      }
   }
   JGMDPRT(7,"DfitKAPLAN for compass=%c, t_len=%d, DfitTotal=%d\n","NESW"[p_hs->hs_seat],t_len,dfit );
   return dfit ;
} /* end DfitKAPLAN */
