/* File Karpin_calc.c */
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
extern int Pav_round(float val, int body ) ;
extern int Pav_body_val(HANDSTAT_k *p_hs );
extern float_t calc_alt_hcp( HANDSTAT_k *p_hs, int suit, int mtag ) ;
extern float_t shortHon_adj( HANDSTAT_k *p_hs, int suit, int mtag ) ;
extern struct misfit_st misfit_chk(HANDSTAT_k *phs[], int s ) ; 
extern void SaveUserVals(struct UserEvals_st UEv , USER_VALUES_k *p_ures ) ;
extern void set_dbg_names(int m, char *dbg_func) ;
extern FIT_PTS_k Do_Df_Fn_pts( UE_SIDESTAT_k *p_ss,
                               int (*calc_dfval)(UE_SIDESTAT_k *p_ss, int h),
                               int (*calc_fnval)(UE_SIDESTAT_k *p_ss, int h)           ) ;

   /*  -------- Do_Df_Fn Assumptions and Explanation ----------- 
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
 
 /* Forward Function Definitions */
extern void zero_globals( int side ) ; 
int Fn_ptsKARPIN( UE_SIDESTAT_k *p_ss, int dc ) ;
int DfitKARPIN(   UE_SIDESTAT_k *p_ss, int du ) ;



// main or userfunc has a) set all the mm_ptrs etc. b) used prolog to zero globals c) filled sidestat global with trump suit choice.

/*  Karpin per Pavlicek web site Counts for Long suits, and shortness once a fit is found */
int karpin_calc( UE_SIDESTAT_k *p_ss ) {    /* Tag Number: 6 */
   int karpin_pts[2] ;
   int h, s ;

   HANDSTAT_k *p_hs, *phs[2];
   zero_globals(p_ss->side) ;
   
   phs[0] = p_ss->phs[0];
   phs[1] = p_ss->phs[1];
   p_hs   = p_ss->phs[0] ;
   set_dbg_names(5, "karpin_calc" ) ;
 
   JGMDPRT(7 , "++++++++++ KARPIN_calc for side=%d compass[0]=%c, compass[1]=%c, phs[0]=%p, phs[1]=%p, hcp[0]=%d, hcp[1]=%d, fhcp=[%g,%g]\n",
               p_ss->side, compass[0],compass[1],(void *)phs[0], (void *)phs[1], phs[0]->hs_totalpoints, phs[1]->hs_totalpoints, fhcp[0],fhcp[1] ) ;
   for (h = 0 ; h < 2 ; h++) {         /* for each hand */
      p_hs = phs[h] ; /* phs array set by prolog to point to hs[north] and hs[south] OR to hs[east] and hs[west] */
      hcp[h] = p_hs->hs_totalpoints ; fhcp[h] = hcp[h];

      for (s = CLUBS ; s<= SPADES ; s++ ) { /* I think I should total the adjustments, and THEN round them. use fhcp_adj[2] */
         fhcp_adj[h] += shortHon_adj(p_hs, s, KARPIN ) ; /* -1 for K, Q, J, QJ, Qx, Jx. */
         JGMDPRT(8,"KARPIN hcp_adj, Hand=%d, suit=%d, hcp[s]=%d, Tot_fhcp_adj=%g SuitLen=%d\n",
                     h, s, p_hs->hs_points[s], fhcp_adj[h], p_hs->hs_length[s] );

         /* simpler to do inline than call Lpts_std */
         if (p_hs->hs_length[s] > 4 ) {
            lpts[h] += (p_hs->hs_length[s] - 4 ) ; // +1 for 5 card suit, +2 for six, +3 for 7 etc.
            JGMDPRT(8,"L_ptsKarpin, Hand=%d, suit=%d, len=%d, Tot_Lpts=%d\n", h, s, p_hs->hs_length[s], lpts[h] );
          }
      } /* end for s = CLUBS to SPADES*/
      hcp_adj[h] = roundf( fhcp_adj[h] ) ;
      karpin_pts[h] = lpts[h] + hcp_adj[h] + hcp[h] ; /* starting points for hand h */
      UEv.nt_pts_seat[h] = karpin_pts[h] ;
   } /* end for each hand */
   UEv.nt_pts_side = UEv.nt_pts_seat[0] + UEv.nt_pts_seat[1] ;
   JGMDPRT(7,"Karpin NT pts done. pts[0]=%d, pts[1]=%d, UEv_Side_pts=%d\n", karpin_pts[0],karpin_pts[1], UEv.nt_pts_side );

   /* Done both hands -- Now calc Dfit and Fn for the side  */
   TFpts = Do_Df_Fn_pts(p_ss, DfitKARPIN, Fn_ptsKARPIN) ; /* Set the globals dfit_pts[],Fn_pts[], and struct TFpts */

   karpin_pts[0] += dfit_pts[0] + Fn_pts[0] ;
   karpin_pts[1] += dfit_pts[1] + Fn_pts[1] ;
   UEv.hldf_pts_seat[0] = karpin_pts[0] ;
   UEv.hldf_pts_seat[1] = karpin_pts[1] ;
   UEv.hldf_pts_side = karpin_pts[0] + karpin_pts[1] ;
   UEv.hldf_suit   = p_ss->t_suit;     /* So we know which dds tricks to count if we are playing in a suit */
   UEv.hldf_fitlen = p_ss->t_fitlen ;
   UEv.misc_count = 0 ;
/* now some debugging fields Standard set even tho some don't apply to this metric */

      /* The factors that apply to both NT and Suit */
      UEv.misc_pts[UEv.misc_count++] = hcp_adj[0];
      UEv.misc_pts[UEv.misc_count++] = lpts[0];
      /* Factors that apply to suit contracts only */
      UEv.misc_pts[UEv.misc_count++] = dfit_pts[0];   /* Dummy support pts typically 3-2-1 with 3, 5-3-1 with 4+ */
      UEv.misc_pts[UEv.misc_count++] = Fn_pts[0];     /* Declarer side suit and extra trump length */
      /* ditto Hand 1 */
      UEv.misc_pts[UEv.misc_count++] = hcp_adj[1];
      UEv.misc_pts[UEv.misc_count++] = lpts[1];
      UEv.misc_pts[UEv.misc_count++] = dfit_pts[1];
      UEv.misc_pts[UEv.misc_count++] = Fn_pts[1];

  /* now put the results into the user result area at p_uservals */
  SaveUserVals( UEv , p_uservals ) ;
     JGMDPRT(7, "Karpin fit calcs Done; Tsuit=%d, Decl=%c, HLDFpts[%d : %d], Fn_pts[%d:%d], Dfit_pts=[%d:%d]\n",
         p_ss->t_suit,"NSEW"[p_ss->decl_seat], karpin_pts[0],karpin_pts[1], Fn_pts[0],Fn_pts[1],dfit_pts[0],dfit_pts[1] );
  return ( 6 + UEv.misc_count ) ;
} /* end karpin_calc */

int karpb_calc( UE_SIDESTAT_k *p_ss ) {      /*Tag Number: 6 Karpin method with BWjgm points (4.25/3/1.75/0.75/0.25)*/
   int karpb_pts[2] = {0} ;
   float_t karpb_raw_pts[2] = { 0.0, 0.0 } ; 
   float_t karpb_hcp[2][4] = { {0.0}, {0.0} } ;
   int h, s ;
   HANDSTAT_k *p_hs, *phs[2];
   zero_globals( p_ss->side) ; 
   phs[0] = p_ss->phs[0];
   phs[1] = p_ss->phs[1];
   p_hs   = p_ss->phs[0] ;
   set_dbg_names(6, "karpb_calc" ) ;
   JGMDPRT(7 , "++++++++++ KARP_B_calc for side=%d compass[0]=%c, compass[1]=%c, phs[0]=%p, phs[1]=%p, hcp[0]=%d, hcp[1]=%d, fhcp=[%g,%g]\n",
               p_ss->side, compass[0],compass[1],(void *)phs[0], (void *)phs[1], phs[0]->hs_totalpoints, phs[1]->hs_totalpoints, fhcp[0],fhcp[1] ) ;

   for (h = 0 ; h < 2 ; h++) {         /* for each hand */
      p_hs = phs[h] ; 
      for (s = CLUBS ; s<= SPADES ; s++ ) { /* I think I should total the adjustments, and THEN round them. use fhcp_adj[2] */
         karpb_hcp[h][s] = calc_alt_hcp(p_hs, s, KARP_B) ; /* calc suit AltHCP see file honors_calc_subs */
         fhcp[h] += karpb_hcp[h][s] ;
         fhcp_adj[h] += shortHon_adj(p_hs, s, KARP_B ) ; /* -1 for K, Q, J, KQ, QJ, */
         
         JGMDPRT(7,"karpb_calc Using BWjgm scale: hcp_adj, Hidx=%d, suit=%d, hcp[s]=%g, Tot_Raw_fhcp[h]=%g, Tot_fhcp_adj=%g, SuitLen=%d\n",
                     h, s, karpb_hcp[h][s], fhcp[h], fhcp_adj[h], p_hs->hs_length[s] );

         /* simpler to do inline than call Lpts_std */
         if (p_hs->hs_length[s] > 4 ) {
            lpts[h] += (p_hs->hs_length[s] - 4 ) ; // +1 for 5 card suit, +2 for six, +3 for 7 etc.
            JGMDPRT(8,"L_pts_KARP_B, Hand=%d, suit=%d, len=%d, Tot_Lpts=%d\n", h, s, p_hs->hs_length[s], lpts[h] );
          }
      } /* end for s = CLUBS to SPADES*/
      hcp_adj[h] = roundf( fhcp_adj[h] ) ;  /* simple rounding for debugging */
      hcp[h]     = roundf( fhcp[h] ) ;

      karpb_pts[h] = Pav_round( (fhcp[h] + fhcp_adj[h]), p_ss->pav_body[h]) + lpts[h] ; /* NT pts for hand h HCP + Lpts + short HCP_adj */
      karpb_raw_pts[h] = fhcp[h] + fhcp_adj[h] + lpts[h];
      JGMDPRT(7,"Karp_B hand=%d,karpb_pts_net=%d, fhcp=%g, fhcp_adj=%g,   Lpts=%d,\n",
                  h, karpb_pts[h],fhcp[h],fhcp_adj[h], lpts[h] );
      UEv.nt_pts_seat[h] = karpb_pts[h] ;
      
   } /* end for each hand */
   UEv.nt_pts_side = UEv.nt_pts_seat[0] + UEv.nt_pts_seat[1] ;
   JGMDPRT(7,"Karp_B NT pts done. pts[0]=%d, pts[1]=%d, UEv_Side_pts=%d\n", karpb_pts[0],karpb_pts[1], UEv.nt_pts_side );

   /* Done both hands -- Now calc Dfit and Fn for the side  */
   TFpts = Do_Df_Fn_pts(p_ss, DfitKARPIN, Fn_ptsKARPIN) ; /* Set the globals dfit_pts[],Fn_pts[], and struct TFpts */
   karpb_pts[0] += TFpts.df_val[0] + TFpts.fn_val[0] ;
   karpb_pts[1] += TFpts.df_val[1] + TFpts.fn_val[1] ;
   UEv.hldf_pts_seat[0] = karpb_pts[0] ;
   UEv.hldf_pts_seat[1] = karpb_pts[1] ;
   UEv.hldf_pts_side = karpb_pts[0] + karpb_pts[1] ;
   UEv.hldf_suit   = p_ss->t_suit;     /* So we know which dds tricks to count if we are playing in a suit */
   UEv.hldf_fitlen = p_ss->t_fitlen ;
   UEv.misc_count = 0 ;

   
   /* Provide the RAW Karp_B points (with fractions) x100 for the Database. BF::Side,N,S then NT Side,N,S */
   int tx100 ;
   UEv.misc_count = 0 ;
   for (h=0 ; h < 2 ; h++ ) {  
      tx100 = (int) (karpb_raw_pts[h] * 100.0) ;
      UEv.misc_pts[1+h] = tx100 + (TFpts.df_val[h] + TFpts.fn_val[h])*100 ;  /* BF value  Must scale Fit pts also */
      UEv.misc_pts[4+h] = tx100 ;                                            /* NT value */
   }
   UEv.misc_pts[0] = UEv.misc_pts[1] + UEv.misc_pts[2] ;  /* 6, 7, 8 side, hand[0], hand[1] in BF */
   UEv.misc_pts[3] = UEv.misc_pts[4] + UEv.misc_pts[5] ;  /* 9,10,11 side, hand[0], hand[1] in NT */
   JGMDPRT(7,"Karp_B RAW BF pts Res[6, 7, 8] => %d = %d + %d \n",UEv.misc_pts[0], UEv.misc_pts[1], UEv.misc_pts[2] );
   JGMDPRT(7,"Karp_B RAW NT pts Res[9,10,11] => %d = %d + %d \n",UEv.misc_pts[3], UEv.misc_pts[4], UEv.misc_pts[5] );
   UEv.misc_count = 6 ;
   /* Debugging vars */

/* now some debugging fields Standard set even tho some don't apply to this metric */

      /* The factors that apply to both NT and Suit */
      UEv.misc_pts[UEv.misc_count++] = hcp_adj[0];
      UEv.misc_pts[UEv.misc_count++] = lpts[0];
      /* Factors that apply to suit contracts only */
      UEv.misc_pts[UEv.misc_count++] = dfit_pts[0];   /* Dummy support pts typically 3-2-1 with 3, 5-3-1 with 4+ */
      UEv.misc_pts[UEv.misc_count++] = Fn_pts[0];     /* Declarer side suit and extra trump length */
      /* Ditto for hand 1 */
      UEv.misc_pts[UEv.misc_count++] = hcp_adj[1];
      UEv.misc_pts[UEv.misc_count++] = lpts[1];
      UEv.misc_pts[UEv.misc_count++] = dfit_pts[1];
      UEv.misc_pts[UEv.misc_count++] = Fn_pts[1];
  /* now put the results into the user result area at p_uservals */
  SaveUserVals( UEv , p_uservals ) ;
     JGMDPRT(6, "KarpB fit calcs Done; Tsuit=%d, Decl=%c, HLDFpts[%d : %d], Fn_pts[%d:%d], Dfit_pts=[%d:%d]\n",
         p_ss->t_suit,"NSEW"[p_ss->decl_seat], karpb_pts[0],karpb_pts[1], Fn_pts[0],Fn_pts[1],dfit_pts[0],dfit_pts[1] );
  return ( 6 + UEv.misc_count ) ;
} /* end karpb_karp_calc */


int DfitKARPIN( UE_SIDESTAT_k *p_ss, int du ) { /*du will point to Dummy's hand -- the one getting the Dfit pts */
   int s_len, dfit ;
   int DF_KARPIN[2][4] = { {2,1,0,0},{3,2,1,0} } ; /* Dfit pts with 3/4 trump for V/S/D/Longer */
   int df_idx;  /* 0 for 3 trump, 1 for 4+ trump */
   HANDSTAT_k *p_hs = p_ss->phs[du] ;
   if (p_ss->t_len[du] < 3 ) { return 0 ; }
   if (p_ss->t_len[du] == 3) {df_idx = 0 ; }
   else { df_idx = 1 ; }
    
   JGMDPRT(7, "KARPIN Calc Dfit for du seat=%d:%c, du=%d, tlen=%d, and ss_len=%d df_idx=%d\n",
               p_ss->dummy_h,compass[du], du, p_ss->t_len[du], p_ss->sorted_slen[du][3], df_idx );

   /* count for all short suits; that seems to have been standard back then */
   dfit = 0 ;
   for (int s=CLUBS; s<=SPADES; s++ ) {
      if (s == p_ss->t_suit) continue ;
      s_len = p_hs->hs_length[s];
      if (s_len < 3 ) {
         dfit += DF_KARPIN[df_idx][s_len];
      }
   }
   JGMDPRT(7, "DfitKARPIN All Short Suits Return, Dummy=[%d], dfit=%d, from t_len[du]=%d: \n", du, dfit, p_ss->t_len[du] ) ;
   return dfit ;
} /* end DfitKARPIN */

   /* Karpin add for short suits in Declarer's hand once a trump fit is found */
int Fn_ptsKARPIN( UE_SIDESTAT_k *p_ss, int dc ) {
   int  fn_pts ;
   int suit ;
   int idx_short, idx_trump ;
   int t_len ;
   int v0, s1, db ;     /*Debug Vars */
   int FN_Karpin[2][4] = { {2,1,0,0}, {3,2,1,0} } ; /* 5 Trumps V=2,S=1,D=0; 6+ trumps V=3, S=2, D=1 each ; longer = 0 */ 
   HANDSTAT_k *p_hs = p_ss->phs[dc] ;
   JGMDPRT(7, "KARPIN Calc FN_pts for dc seat=%d:%c, dc=%d, tlen=%d, and ss_len=%d\n",
               p_ss->decl_h,compass[dc], dc, p_ss->t_len[dc], p_ss->sorted_slen[dc][3] ); /* [3] is the shortest suit */

   t_len = p_ss->t_len[dc] ;
   if (t_len < 5 ) { return 0 ; }
   if (t_len == 5 ) { idx_trump = 0 ; }
   else {             idx_trump = 1 ; }
   
   fn_pts  = 0 ;
   v0 = 0 ; s1 = 0 ; db = 0 ;
   for (suit = CLUBS; suit<=SPADES; suit++ ) {
      if(suit == p_ss->t_suit) continue ;
      if      ( 0 == p_hs->hs_length[suit] ) { idx_short = 0 ; v0++ ;}
      else if ( 1 == p_hs->hs_length[suit] ) { idx_short = 1 ; s1++ ;}
      else if ( 2 == p_hs->hs_length[suit] ) { idx_short = 2 ; db++ ;} /* assume Karpin gives for each dblton in Decl Hand*/
      else if ( 3 <= p_hs->hs_length[suit] ) { idx_short = 3 ; }
      fn_pts += FN_Karpin[idx_trump][idx_short] ; 
   } /* end for each suit */
   JGMDPRT(7,"Fn_KARPIN Done. Decl=[%d:%c] TotFn_ptsAllSuits=%d from T_len=%d, void_cnt=%d, stiff_cnt=%d, dblton_cnt=%d \n",
                           dc, compass[dc], fn_pts, p_ss->t_len[dc], v0, s1, db );
   return fn_pts ;
} /* end Fn_ptsKARPIN */

