/* File Larsson_calc.c */
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
extern float_t calc_alt_hcp( HANDSTAT_k *p_hs, int suit, int mtag ) ;
extern float_t shortHon_adj( HANDSTAT_k *p_hs, int suit, int mtag ) ;
extern void SaveUserVals(struct UserEvals_st UEv , USER_VALUES_k *p_ures ) ;
extern void set_dbg_names(int m, char *dbg_func) ;
extern FIT_PTS_k Do_Df_Fn_pts( UE_SIDESTAT_k *p_ss,
                               int (*calc_dfval)(UE_SIDESTAT_k *p_ss, int h),
                               int (*calc_fnval)(UE_SIDESTAT_k *p_ss, int h)           ) ;
 
 /* Forward Function Definitions */
 int DfitLAR(    UE_SIDESTAT_k *p_ss, int du) ;
 int LAR_Fn_pts( UE_SIDESTAT_k *p_ss, int h ) { return 0 ; }
 int SynLAR(HANDSTAT_k *p_hs ) ;
// main or userfunc has a) set all the mm_ptrs etc. b) used prolog to zero globals c) filled sidestat global with trump suit choice.

/*  Larsson per his book, Good, Better, Best. It is quite a good method.
 *  syn* pts, body-pts, Lpts=(Len-5) max of 3,
 * -1 for short honors. Do not deduct for misfit;
 * 5-3-1 with 4 trumps
 */
int lar_calc( UE_SIDESTAT_k *p_ss ) {    /* Tag Number: 8 */
   int larsson_pts[2] = {0} ;
   int nines = 0 ;
   int h, s ;
   int temp = 0 ;
   HANDSTAT_k *p_hs, *phs[2];
   zero_globals( p_ss->side ) ;
      
   phs[0] = p_ss->phs[0];
   phs[1] = p_ss->phs[1];
   p_hs   = p_ss->phs[0] ;
   set_dbg_names(8, "lar_calc" ) ;
  JGMDPRT(7 , "++++++++++ Larsson (m=8)_calc for p_ss->side= %d; compass[0]=%c, compass[1]=%c, phs[0]=%p, phs[1]=%p, hcp[0]=%d, hcp[1]=%d, fhcp=[%g,%g]\n",
               p_ss->side, compass[0],compass[1],(void *)phs[0], (void *)phs[1], phs[0]->hs_totalpoints, phs[1]->hs_totalpoints, fhcp[0],fhcp[1] ) ;

  for (h = 0 ; h < 2 ; h++) {         /* for each hand */
      p_hs = phs[h] ; 
      hcp[h] = p_hs->hs_totalpoints ; fhcp[h] = hcp[h];

      for (s = CLUBS ; s<= SPADES ; s++ ) { /* I think I should total the adjustments, and THEN round them. use fhcp_adj[2] */
         fhcp_adj[h] += shortHon_adj(p_hs, s, LAR ) ; /* -1 for K, Q, J, KQ, QJ, */
         JGMDPRT(8,"LAR hcp-adj, Hidx=%d, suit=%d, hcp[s]=%d, HandTot_Raw_hcp[h]=%g, HandTot_hcp_adj=%g, SuitLen=%d\n",
                              h, s, p_hs->hs_points[s],  fhcp[h],   fhcp_adj[h], p_hs->hs_length[s] );

         /* simpler to do inline than call function */
         if (p_hs->hs_length[s] > 5 ) {
            temp = (p_hs->hs_length[s] - 5 )  ; // +1 for six carder, +2 for 7, +3 for longer. etc.
            if (temp > 3 ) { lpts[h] += 3 ; }
            else           { lpts[h] += temp; }
            JGMDPRT(8,"L_ptsLAR, Hand=%d, suit=%d, len=%d, HandTot_Lpts=%d\n", h, s, p_hs->hs_length[s], lpts[h] );
          }
          if ((p_hs->Has_card[s][Nine_rk] ) ) { nines++ ; } /* count nines for later body_pts */
      } /* end for s = CLUBS to SPADES*/
      syn_pts[h] = SynLAR(p_hs) ;  /* count suits with 2 of top 4 incl A or K; +1 for 2 such, +2 for 3 or 4 such*/
      /* Body_pts +1 if the hand has two Tens or one Ten and two+ Nines */
      if ( (p_hs->hs_totalcounts[idxTens] >= 2) || (p_hs->hs_totalcounts[idxTens] == 1 && nines >= 2) ) {body_pts[h] = 1 ;}
      hcp_adj[h] = roundf( fhcp_adj[h] ) ;
      larsson_pts[h] = hcp[h] + hcp_adj[h] + body_pts[h] + syn_pts[h] + lpts[h] ; /* NT pts for hand h */
      JGMDPRT(7,"LARSSON hand=%d,lar_pts_net=%d, hcp=%d, hcp_adj=%d,  body=%d, syn=%d, Lpts=%d,\n",
                  h, larsson_pts[h],hcp[h],hcp_adj[h],body_pts[h],syn_pts[h],lpts[h] );
      UEv.nt_pts_seat[h] = larsson_pts[h] ;
   } /* end for each hand */
   UEv.nt_pts_side = UEv.nt_pts_seat[0] + UEv.nt_pts_seat[1] ;
   JGMDPRT(7,"LARSSON NT pts done. pts[0]=%d, pts[1]=%d, UEv_Side_pts=%d\n", larsson_pts[0],larsson_pts[1], UEv.nt_pts_side );

   /* Done both hands -- Now check for a trump fit Set the globals struct TFpts, dfit_pts[] and Fn_pts[] */
   TFpts = Do_Df_Fn_pts(p_ss, DfitLAR, LAR_Fn_pts) ; /* will also set the globals dfit_pts[],Fn_pts[] */

   larsson_pts[0] += TFpts.df_val[0] + TFpts.fn_val[0] ;  /* there are no LAR Fn pts currently */
   larsson_pts[1] += TFpts.df_val[1] + TFpts.fn_val[1] ;
   UEv.hldf_pts_seat[0] = larsson_pts[0] ;
   UEv.hldf_pts_seat[1] = larsson_pts[1] ;
   UEv.hldf_pts_side = larsson_pts[0] + larsson_pts[1] ;
   UEv.hldf_suit   = p_ss->t_suit;     /* So we know which dds tricks to count if we are playing in a suit */
   UEv.hldf_fitlen = p_ss->t_fitlen ;
   UEv.misc_count = 0 ;
/* now some debugging fields Standard set even tho some don't apply to this metric */

      /* The factors that apply to both NT and Suit */
      UEv.misc_pts[UEv.misc_count++] = hcp_adj[0];
      UEv.misc_pts[UEv.misc_count++] = lpts[0];
      UEv.misc_pts[UEv.misc_count++] = body_pts[0];
      UEv.misc_pts[UEv.misc_count++] = syn_pts[0];
      /* Factors that apply to suit contracts only */
      UEv.misc_pts[UEv.misc_count++] = dfit_pts[0];   /* Dummy support pts typically 3-2-1 with 3, 5-3-1 with 4+ */
       /* ditto hand 1 */
      UEv.misc_pts[UEv.misc_count++] = hcp_adj[1];
      UEv.misc_pts[UEv.misc_count++] = lpts[1];
      UEv.misc_pts[UEv.misc_count++] = body_pts[1];
      UEv.misc_pts[UEv.misc_count++] = syn_pts[1];
      UEv.misc_pts[UEv.misc_count++] = dfit_pts[1];
  /* now put the results into the user result area at p_uservals */
  SaveUserVals( UEv , p_uservals ) ;
     JGMDPRT(7, "Larsson fit calcs Done; Tsuit=%d, Decl=%c, HLDFpts[%d : %d], Dfit_pts=[%d:%d] Fn_pts[%d:%d],\n",
         p_ss->t_suit,"NSEW"[p_ss->decl_seat], larsson_pts[0],larsson_pts[1], 
                                          TFpts.df_val[0],TFpts.df_val[1], TFpts.fn_val[0],TFpts.fn_val[1]
         );
  return ( 6 + UEv.misc_count ) ;
} /* end larsson_calc */

int lar_b_calc(UE_SIDESTAT_k *p_ss ) {    /* Tag Number:  9 -- this is LAR with BWjgm HCP.and possible JGM mods to Dfit calcs */
   int lar_b_pts[2] = {0} ;
   
   float_t lar_b_hcp[2][4] = { {0.0}, {0.0} } ;
   float_t lar_b_pts_raw[2] = {0.0,0.0};
   int nines = 0 ;
   int h, s ;
   int temp = 0 ;
   int pav_body;

   HANDSTAT_k *p_hs, *phs[2];
   zero_globals( p_ss->side ) ;
   
   phs[0] = p_ss->phs[0];
   phs[1] = p_ss->phs[1];
   p_hs   = p_ss->phs[0] ;
//fprintf(stderr, "****metrics_calcs:%d :: lar_b_calc Entered. side=%d, jgmDebug=%d \n",__LINE__,side,jgmDebug);
   set_dbg_names(9, "lar_b_calc" ) ;
   JGMDPRT( 7 , "++++++++++ lar_b_calc(m=9) for side=%d compass[0]=%c, compass[1]=%c, phs[0]=%p, phs[1]=%p, hcp[0]=%d, hcp[1]=%d, fhcp=[%g,%g]\n",
               p_ss->side, compass[0],compass[1],(void *)phs[0], (void *)phs[1], phs[0]->hs_totalpoints, phs[1]->hs_totalpoints, fhcp[0],fhcp[1] ) ;
  for (h = 0 ; h < 2 ; h++) {         /* for each hand */
      p_hs = phs[h] ; 
      for (s = CLUBS ; s<= SPADES ; s++ ) { /* I think I should total the adjustments, and THEN round them. use fhcp_adj[2] */
         lar_b_hcp[h][s] = calc_alt_hcp(p_hs, s, LAR_B) ; /* calc BWjgm points for this suit. */
         fhcp[h] += lar_b_hcp[h][s] ;
         fhcp_adj[h] += shortHon_adj(p_hs, s, LAR_B ) ; /* -1 for K, Q, J, KQ, QJ, */
         JGMDPRT(8,"Adjlar_b, Hidx=%d, suit=%d, hcp[s]=%g, Tot_Raw_fhcp[h]=%g, Tot_fhcp_adj=%g, SuitLen=%d\n",
                     h, s, (double)lar_b_hcp[h][s], (double)fhcp[h], (double)fhcp_adj[h], p_hs->hs_length[s] );

         /* Using Larsson for now. No requirement to have 3 hcp in the suit. Simpler to do inline than call function */
         if (p_hs->hs_length[s] > 5 ) {
            temp = (p_hs->hs_length[s] - 5 ) ; // +1 for six carder, +2 for 7, +3 for longer. etc.
            if (temp > 3 ) { lpts[h] += 3 ; }
            else           { lpts[h] += temp; }
            JGMDPRT(8,"L_ptslar_b, Hand=%d, suit=%d, len=%d, Tot_Lpts=%d\n", h, s, p_hs->hs_length[s], lpts[h] );
          }
          if ((p_hs->Has_card[s][Nine_rk] ) ) { nines++ ; } /* count nines for later body_pts JGM no do since count T already?*/
      } /* end for s = CLUBS to SPADES*/
      syn_pts[h] = SynLAR(p_hs) ;  /* count suits with 2 of top 4 incl A or K; +1 for 2 such, +2 for 3 or 4 such*/
      /* Body_pts +1 if the hand has two Tens or one Ten and two+ Nines */
      if ( (p_hs->hs_totalcounts[idxTens] >= 2) || (p_hs->hs_totalcounts[idxTens] == 1 && nines >= 2) ) {body_pts[h] = 1 ;}
      pav_body = p_ss->pav_body[h] ; ; 
      hcp_adj[h] = Pav_round( fhcp_adj[h], pav_body ) ;  /* for debug only. */
      hcp[h] = Pav_round( fhcp[h] , pav_body) ;          /* for debug only */
      lar_b_pts[h] = Pav_round( (fhcp[h] + fhcp_adj[h]), pav_body) + syn_pts[h] + lpts[h] ; /* NT pts for hand h omit + body_pts[h] */
      JGMDPRT(7,"lar_b hand=%d,lar_b_pts_net=%d, fhcp=%g, fhcp_adj=%g,  syn=%d, Lpts=%d, pav_body=%d\n",
                  h,           lar_b_pts[h],     fhcp[h], fhcp_adj[h],  syn_pts[h],lpts[h], pav_body );
      UEv.nt_pts_seat[h] = lar_b_pts[h] ;
      lar_b_pts_raw[h] = fhcp[h] + fhcp_adj[h] + syn_pts[h] + lpts[h] ; /* omit body_pts[h] */
   } /* end for each hand */
   UEv.nt_pts_side = UEv.nt_pts_seat[0] + UEv.nt_pts_seat[1] ;
   JGMDPRT(7,"lar_b NT pts done. pts[0]=%d, pts[1]=%d, UEv_Side_pts=%d\n", lar_b_pts[0],lar_b_pts[1], UEv.nt_pts_side );

   /* Done both hands -- Now check for a trump fit */
   TFpts = Do_Df_Fn_pts(p_ss, DfitLAR, LAR_Fn_pts) ; /* LAR_B same as LAR. will also set the globals dfit_pts[],Fn_pts[] */
    
   UEv.hldf_pts_seat[0] = lar_b_pts[0] + TFpts.df_val[0] + TFpts.fn_val[0] ;  /* there are no Fn pts currently */
   UEv.hldf_pts_seat[1] = lar_b_pts[1] + TFpts.df_val[1] + TFpts.fn_val[1] ; 
   UEv.hldf_pts_side    = UEv.hldf_pts_seat[0] + UEv.hldf_pts_seat[1] ;
   UEv.hldf_suit   = p_ss->t_suit;     /* So we know which dds tricks to count if we are playing in a suit */
   UEv.hldf_fitlen = p_ss->t_fitlen ;

   /* Return also the RAW points, with fractions x100 for inclusion in DBase */
   int tx100 ;
   UEv.misc_count = 0 ;
   for (h=0 ; h < 2 ; h++ ) {  
      tx100 = (int) (lar_b_pts_raw[h] * 100.0) ;
      UEv.misc_pts[1+h] = tx100 + (TFpts.df_val[h] + TFpts.fn_val[h])*100 ;  /* BF value Must scale these also */
      UEv.misc_pts[4+h] = tx100 ;                                      /* NT value */
   }
   UEv.misc_pts[0] = UEv.misc_pts[1] + UEv.misc_pts[2] ;  /* 6, 7, 8 side, hand[0], hand[1] in BF */
   UEv.misc_pts[3] = UEv.misc_pts[4] + UEv.misc_pts[5] ;  /* 9,10,11 side, hand[0], hand[1] in NT */
   JGMDPRT(7,"LAR_B RAW BF pts Res[6, 7, 8] => %d = %d + %d \n",UEv.misc_pts[0], UEv.misc_pts[1], UEv.misc_pts[2] );
   JGMDPRT(7,"LAR_B RAW NT pts Res[9,10,11] => %d = %d + %d \n",UEv.misc_pts[3], UEv.misc_pts[4], UEv.misc_pts[5] );
   UEv.misc_count = 6 ;
/* now some debugging fields Standard set even tho some don't apply to this metric */
      /* The factors that apply to both NT and Suit */
      UEv.misc_pts[UEv.misc_count++] = hcp_adj[0];
      UEv.misc_pts[UEv.misc_count++] = lpts[0];
      UEv.misc_pts[UEv.misc_count++] = body_pts[0];
      UEv.misc_pts[UEv.misc_count++] = syn_pts[0];
      /* Factors that apply to suit contracts only */
      UEv.misc_pts[UEv.misc_count++] = dfit_pts[0];   /* Dummy support pts typically 3-2-1 with 3, 5-3-1 with 4+ */

      /* ditto for hand 1 */
      UEv.misc_pts[UEv.misc_count++] = hcp_adj[1];
      UEv.misc_pts[UEv.misc_count++] = lpts[1];
      UEv.misc_pts[UEv.misc_count++] = body_pts[1];
      UEv.misc_pts[UEv.misc_count++] = syn_pts[1];
      UEv.misc_pts[UEv.misc_count++] = dfit_pts[1];
  /* now put the results into the user result area at p_uservals */
      SaveUserVals( UEv , p_uservals ) ;
      JGMDPRT(7, "Lar_b fit calcs Done; Tsuit=%d, Decl=%c, HLDFpts[%d : %d], Dfit_pts=[%d:%d] Fn_pts[%d:%d],\n",
         p_ss->t_suit,"NSEW"[p_ss->decl_seat], lar_b_pts[0],lar_b_pts[1], 
                                          TFpts.df_val[0],TFpts.df_val[1], TFpts.fn_val[0],TFpts.fn_val[1] 
         );
  return ( 6 + UEv.misc_count ) ;
} /* end lar_b_calc */

/* Larsson Synergy. His own invention? */
/* if there are two suits (len = 3+) with two+ of top4 honors (one of which must be A or K), then +1; if 3 such suits, +2 */
int SynLAR(HANDSTAT_k *p_hs ) {
   int syn_cnt = 0 ;
   int syn_pts = 0 ;
   int suit ;
   for (suit = CLUBS; suit <=SPADES ; suit++ ) {
      if ( p_hs->hs_counts[idxTop2][suit] >= 1 && p_hs->hs_counts[idxTop4][suit] >= 2 && p_hs->hs_length[suit] >= 3 ) {
         syn_cnt++ ;
      }
   }
   syn_pts = (syn_cnt >  2 ) ? 2 :
             (syn_cnt == 2 ) ? 1 : 0 ;
   JGMDPRT(8,"SynLAR, syn_cnt=%d, syn_pts=%d\n", syn_cnt, syn_pts) ;
   return syn_pts ;
}
/* end SynLAR */

int DfitLAR( UE_SIDESTAT_k *p_ss, int du) { /* called with 'short' hand 0/1/2; for 4=4 fit called once for each hand */ 
   int ss_len, t_len, dfit ;
   int idx_short ;
   int DF_LAR[2][4] = { {3,2,1,0},{5,3,1,0} } ; /* Dfit pts with 3/4 trump for V/S/D/Longer -- Only if 9+ fit */
   int v0, s1, db ; /* Debug vars */
   t_len = p_ss->t_len[du];
   ss_len = p_ss->sorted_slen[du][3] ; /* shortest suit is in slot 3 */
   JGMDPRT(7, "LAR Calc Dfit for du seat=%d:%c, du=%d, tlen=%d, and sorted_slen[3]=%d, ss_len=%d\n",
               seat[du],compass[du], du, p_ss->t_len[du], p_ss->sorted_slen[du][3] , ss_len  );
   v0 = 0 ; s1 = 0 ; db = 0 ;
   /* Larsson only gives for "9 fit"  he says then assign 5-3-1; but I am not sure about 6-3? 5-3-1 with only 3 trump is a lot.
    * So I compromise a 9 fit with 4 trump in dummy gets 5-3-1; a 9+ fit with only 3 trump in dummy 3-2-1
    */
   if ( (p_ss->t_len[du] <= 2 ) ||  (p_ss->t_fitlen < 9) ) { return 0 ; }
   //  if ( (p_ss->t_len[du] < 4 ) ||  (p_ss->t_fitlen < 9) ) { return 0 ;} /* this is the other choice for larsson */
   /* We have a 9 card fit. count for 1 shortest suit only */
   if      ( 0 == ss_len ) { idx_short = 0 ; v0++ ;}
   else if ( 1 == ss_len ) { idx_short = 1 ; s1++ ;}
   else if ( 2 == ss_len ) { idx_short = 2 ; db++ ;}
   else                    { idx_short = 3 ; }
   if (3 == t_len  ) { dfit = DF_LAR[0][idx_short] ; }  /* 3 card support; */
   else              { dfit = DF_LAR[1][idx_short] ; }  /* 4+ support; */
   JGMDPRT(7, "DfitLAR Returning, Dummy=[%d], dfit=%d, from p_ss->ss_len=%d,t_len[du]=%d: V_S_D:%d,%d,%d\n",
                  du, dfit, ss_len, t_len,v0,s1,db ) ;
   return dfit ;
} /* end DfitLAR */



