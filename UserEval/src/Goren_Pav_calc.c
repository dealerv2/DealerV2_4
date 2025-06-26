/* File goren_pav_calc.c */
/* Date        Version  Author   Description
 * 2024/08/12  1.0      JGM      Extracted from metrics_calcs.c and factors.c; Using the UE_SIDESTAT functionality
 * 2024/09/11  1.1      JGM      PAV Metric bug; wrong HCP scale?
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
extern int ss_index(int weight) ; 
extern struct misfit_st misfit_chk(HANDSTAT_k *phs[], int s ) ; 
extern void SaveUserVals(struct UserEvals_st UEv , USER_VALUES_k *p_ures ) ;
extern void set_dbg_names(int m, char *dbg_func) ;
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
 int DptsGOREN(   HANDSTAT_k    *p_hs, int s ) ;
 int DfitGOREN(   UE_SIDESTAT_k *p_ss, int du) ;
 int Fn_ptsGOREN( UE_SIDESTAT_k *p_ss, int dc) ;
 int Hf_ptsGOREN( HANDSTAT_k    *p_hs, int Hf_hand) ;

 int DptsPAV(   HANDSTAT_k    *p_hs, int s ) ;
 int LptsPAV(   HANDSTAT_k    *phs[] )  ;
 int DfitPAV(   UE_SIDESTAT_k *p_ss, int du) ;
 int Fn_ptsPAV( UE_SIDESTAT_k *p_ss, int dc) ;

 static int Dpts_GorPav[][22] = {
   //               A K Q J T x  AK     Ax  KQ    Kx  QJ  Qx JT  Tx,xx
   {  /* Goren   */ 2,2,2,2,2,2, 1,1,1,1,1, 1,1,1,1,  1,1,1, 1,1, 1,1 }, // Stiff K = 3 - 2 + 3 = 4
   {  /* Pavlicek*/ 2,2,2,2,2,2, 1,1,1,1,1, 1,1,1,1,  1,1,1, 1,1, 1,1 },
 /* These Dpts are demoted to zero if no suppt*/
} ;
// main or userfunc has a) set all the mm_ptrs etc. b) used prolog to zero globals c) filled sidestat global with trump suit choice.

/* Goren Calc. Shortness based Method, -- Dpts, Dfit_pts per Pavlicek, FnPts and HfPts per iNet; Fits must be >=8 ; no 5:2 or 4:3 */
/* Pav claims to be close to Goren, and Pav does Dfit only for dummy even in a 4:4 fit, so assume Goren does also */
int goren_calc( UE_SIDESTAT_k *p_ss ) {     /* Tag number 3 */
   int gor_NTpts[2]     = {0};
   int gor_pts[2]       = {0};
   int h, s , body_cnt, slen;
   HANDSTAT_k *phs_dummy ;
   HANDSTAT_k *p_hs, *phs[2];
   zero_globals( p_ss->side ) ; 
   phs[0] = p_ss->phs[0];
   phs[1] = p_ss->phs[1];
   p_hs   = p_ss->phs[0] ;
   set_dbg_names(3, "goren_calc");
   JGMDPRT(7 , "++++++++++ GOREN calc for p_ss->side= %d; compass[0]=%c, compass[1]=%c, phs[0]=%p, phs[1]=%p, hcp[0]=%d, hcp[1]=%d\n",
               p_ss->side, compass[0],compass[1],(void *)phs[0], (void *)phs[1], phs[0]->hs_totalpoints, phs[1]->hs_totalpoints ) ;
   for (h = 0 ; h < 2 ; h++) {         /* for each hand */
      p_hs = phs[h] ; 
      for (s = CLUBS ; s<= SPADES ; s++ ) {
         slen = p_hs->hs_length[s] ;
         if (0 == slen ) {
            fhcp_suit[h][s]  = 0.0 ;
            fhcp_adj_suit[h][s] = 0.0 ;
            dpts_suit[h][s] = 3 ;
         }
         else if ( slen <= 2 ) {
            fhcp_suit[h][s]  = p_hs->hs_points[s]  ;
            fhcp_adj_suit[h][s] = shortHon_adj(p_hs, s, GOREN ) ; /* Deduct in a std way in case we play the hand in NT */
            dpts_suit[h][s] = DptsGOREN( p_hs, s ) ;
         }
         else { /* no shortness pts; straight HCP */
            fhcp_suit[h][s]  = p_hs->hs_points[s] ;
            fhcp_adj_suit[h][s] = 0.0 ;
            dpts_suit[h][s] = 0 ;
         }
         /*  end hcp and dpts calcs */
         fhcp[h] += fhcp_suit[h][s] ;
         fhcp_adj[h] += fhcp_adj_suit[h][s] ;
         dpts[h] += dpts_suit[h][s] ;
         JGMDPRT(8,"goren_calc, Hand=%d, suit=%d, SuitLen=%d, fhcp[h][s]=%g, fhcp-adj[h][s]=%g, Dpts[h][s]=%d \n",
                     h, s, p_hs->hs_length[s], fhcp_suit[h][s], fhcp_adj_suit[h][s], dpts_suit[h][s] );
      } /* end CLUBS <= s <= SPADES */
      /* Check for Aces or lack of */
      body_cnt = p_hs->hs_totalcounts[idxAces] ;
      if (body_cnt == 0 && gor_NTpts[h] > 10 ) {body_pts[h] = -1 ; } /* Only deduct for lack of Aces if considering Inv or Open, i.e. pts > 10 JGM*/
      else if (body_cnt == 4 )                 {body_pts[h] =  1 ; } 
      else                                     {body_pts[h] =  0 ; }
      JGMDPRT(7,"goren_calc, Hand=%d, fhcp[h]=%g, fhcp-adj[h]=%g, Dpts[h]=%d, body_pts[h]=%d, Aces_cnt=%d\n",
                     h, fhcp[h], fhcp_adj[h], dpts[h],body_pts[h], body_cnt );
      /* In NT Do not count Dpts; so we just need the HCP + ShortHonor_ADJ + body */

      if (p_hs->square_hand == 1 ) { hand_adj[h] = -1 ; } /* Square hand Ded applies to NT and to dummy in suit contract */
      gor_NTpts[h] = lround(fhcp[h] + fhcp_adj[h]) + body_pts[h] + hand_adj[h]  ; /* NT points for hand h -- dont count Dpts for NT */
      if ( gor_NTpts[h] < 0 ) gor_NTpts[h] = 0; /* might go minus with 0 hcp, no aces, and/or a square hand */
      JGMDPRT(7,"GOR NT Result: Hand=%d, NT_pts=%d, fhcp=%g, fhcp_adj=%g, body_pts=%d, body_cnt=%d, Sq=%d \n",
                     h,  gor_NTpts[h],fhcp[h],fhcp_adj[h], body_pts[h],  body_cnt , hand_adj[h]);
      UEv.nt_pts_seat[h] = gor_NTpts[h] ;

      /* get Prelim value for hand in suit contract; will be affected by any later discovered misfits; also by Dfit_pts */
      /* in a suit contract we add the Dpts to the NT pts. The short Hon Ded are chosen so this comes out right */
      gor_pts[h] = lround(fhcp[h] + fhcp_adj[h]) + body_pts[h] + dpts[h]; /* No Square Hand Deduction yet; Only Dummy in a suit contract */
      JGMDPRT(7,"GOR Suit Prelim Val[%d]=%d :: fhcp=%g fhcp_adj=%g dpts=%d body_pts=%d, body_cnt=%d\n",
                           h, gor_pts[h], fhcp[h], fhcp_adj[h], dpts[h], body_pts[h],  body_cnt);
   }
   /*---------------------- end for each hand --------------------------- */

   /* Begin Evaluating the side as a whole; look for fit, assign Dfit, Fn, Hf, Syn, etc */
   UEv.nt_pts_side = UEv.nt_pts_seat[0] + UEv.nt_pts_seat[1] ;

   JGMDPRT(7,"GOR NT pts done.UEv_Side_pts=%d, UEv[0]=%d, UEv[1]=%d\n",  UEv.nt_pts_side, UEv.nt_pts_seat[0], UEv.nt_pts_seat[1] );
   JGMDPRT(7,"Prelim gor_pts=[%d,%d], Dpts=[%d,%d] \n", gor_pts[0],gor_pts[1],dpts[0],dpts[1] );

   /* Cant use the TFpts = Do_Df_Fn call because Goren does not count Dfit for both hands if it is 4:4 */

   if ( p_ss->t_fitlen >= 8) {   /* t_fitlen might ==7 here which means a 5-2 fit But Goren ignores fits < 8 */

        /* if there is a trump fit, Add Dfit and Fn Points to the Honors and Dpts already included  */
        /* Assume that Declarer and Dummy are defined as per PAV. Decl=Longest trumps or if equal the hand with the most HCP */
        /* If it's a 4-4 fit (equal len trumps), only Dummy gets Dfit, Declarer gets Fn (side suit points) */
        /* The Dfit and Fn stuff is from a teacher's post on BridgeWinners */
        /* Dummy does not count for Length, keeps its Dpts (incl +1 __each__ Dblton) and adds for EACH stiff/void with 4+ trump
         */
      h_dummy = p_ss->dummy_h;
      h_decl  = p_ss->decl_h ;
      phs_dummy = phs[h_dummy] ;

      dfit_pts[h_dummy] = DfitGOREN  (p_ss, h_dummy ) ; /* 4+T:2/1/0 These are in addition to Dpts awarded ealier */
      Fn_pts[h_decl]    = Fn_ptsGOREN(p_ss, h_decl  ) ; /* 5th Trump +1, 6th & up +2 each.) */
      hand_adj[h_decl] = 0 ; /* hand_adj set ealier for both hands. But in a suit contract only Dummy gets the Ded if any */
      JGMDPRT(7, "GOR: dfit for dummy[%d]=%d, FN for Decl[%d]=%d\n",h_dummy,dfit_pts[h_dummy],h_decl,Fn_pts[h_decl]);

      /* Goren also counts Hf Pts for honors <= 3 hcp in the trump suit. */
      gor_pts[h_decl]  += Fn_pts[h_decl];
      hf_pts[h_dummy] = Hf_ptsGOREN( phs_dummy , p_ss->t_suit ) ;
      gor_pts[h_dummy] += ( dfit_pts[h_dummy] + hand_adj[h_dummy] + hf_pts[h_dummy] );


      JGMDPRT(7,"GOR TrumpFit Deal=%d, in suit=%d FitLen:%d Decl=[%d], Fn=%d, Hldf=%d Dummy=[%d], Dfit=%d Hf=%d, SQ=%d, Hldf=%d\n",
            gen_num,p_ss->t_suit,p_ss->fitlen[0],h_decl,Fn_pts[h_decl],gor_pts[h_decl],
            h_dummy,dfit_pts[h_dummy],hf_pts[h_dummy], hand_adj[h_dummy],gor_pts[h_dummy] );
   } /* End Trump fit gor_pts all done for this case */

    /* Since there is no fit, Check for misfits which cannot be ignored. */
   else {
      /* If there is shortness vs pards long (4+) suit, and there is no trump fit, do not count for the shortness
       * We subtract ALL shortness pts, not just those in the misfit suit. Different from PAV
       * This may be wrong if we end up in a suit contract anyway in a 5-2 or 4-3 fit.
       */
      for (s=CLUBS; s<=SPADES; s++ ) {
         misfit[s] = misfit_chk(phs , s) ; /* return results in a struct */
         if (misfit[s].mf_type > 3) {  /* Misfit shortness vs 4+ suit; Subtract ALL dpts not just the Dpts in the suit. */
            gor_pts[0] -= dpts[0]; gor_pts[1] -= dpts[1] ; /* Dpts were added earlier */
               JGMDPRT(7,"GOR MISFIT Deal=%d, in suit=%d Type:%d sh_len=%d, gor_pts[0]=%d, gor_pts[1]=%d\n",
                  gen_num,s,misfit[s].mf_type,misfit[s].sh_len,gor_pts[0], gor_pts[1] );
            break ; /* exit suit loop as soon as one misfit found */
         }
      } /* end for each suit */
   } /* end else check for misfit */
   JGMDPRT(7,"GOREN Side Fit/Misfit Eval Done  Deal=%d,  End Result for Suit Play: gor_pts[0]=%d, gor_pts[1]=%d\n",
                     gen_num, gor_pts[0], gor_pts[1] );
   /* gor_pts[] now done; [Dpts + (hcp+hcp_adj)] + body + Dfit + Fn + hand_adj */
   UEv.hldf_pts_seat[0] = ( gor_pts[0] > 0 ) ? gor_pts[0] : 0 ;
   UEv.hldf_pts_seat[1] = ( gor_pts[1] > 0 ) ? gor_pts[1] : 0 ;
   UEv.hldf_pts_side = UEv.hldf_pts_seat[0] + UEv.hldf_pts_seat[1] ;
   UEv.hldf_suit   = p_ss->t_suit;     /* So we know which dds tricks to count if we are playing in a suit */
   UEv.hldf_fitlen = p_ss->t_fitlen ;

   JGMDPRT(7,"GOREN HLDF pts done.UEv_HLDFSide_pts=%d, UEv_HLDF[0]=%d, UEv_HLDF[1]=%d\n",
            UEv.hldf_pts_side, UEv.hldf_pts_seat[0], UEv.hldf_pts_seat[1]);
/* now some debugging fields */
   UEv.misc_count = 0 ;
      /* The factors that apply to both NT and Suit */
      UEv.misc_pts[UEv.misc_count++] = lround(fhcp_adj[0]);
      UEv.misc_pts[UEv.misc_count++] = body_pts[0];      /* +1 for 4 Aces, -1 for No Aces 0 otherwise */
      /* Factors that apply to suit contracts only */
      UEv.misc_pts[UEv.misc_count++] = dpts[0];       /* distribution aka shortness pts before a fit is found */
      UEv.misc_pts[UEv.misc_count++] = dfit_pts[0];   /* Dummy support pts typically 3-2-1 with 3, 5-3-1 with 4+ */
      UEv.misc_pts[UEv.misc_count++] = Fn_pts[0];     /* Declarer side suit and extra trump length */
      UEv.misc_pts[UEv.misc_count++] = hf_pts[0];
      UEv.misc_pts[UEv.misc_count++] = hand_adj[0];  /* Square Hand Deduction */
      /* Ditto for hand 1 */
      UEv.misc_pts[UEv.misc_count++] = roundf(fhcp_adj[1]);
      UEv.misc_pts[UEv.misc_count++] = body_pts[1];
      UEv.misc_pts[UEv.misc_count++] = dpts[1];
      UEv.misc_pts[UEv.misc_count++] = dfit_pts[1];
      UEv.misc_pts[UEv.misc_count++] = Fn_pts[1];
      UEv.misc_pts[UEv.misc_count++] = hf_pts[1];
      UEv.misc_pts[UEv.misc_count++] = hand_adj[1];
      SaveUserVals( UEv , p_uservals ) ;
      JGMDPRT(7,"GOR UEv misc_count=%d,[h0]=%d,%d,%d,%d,%d,%d,%d\n", UEv.misc_count, UEv.misc_pts[0],UEv.misc_pts[1],
                  UEv.misc_pts[2],UEv.misc_pts[3],UEv.misc_pts[4], UEv.misc_pts[5],UEv.misc_pts[6]);
      JGMDPRT(7,"GOR UEv misc_count=%d,[h1]=%d,%d,%d,%d,%d,%d,%d\n", UEv.misc_count, UEv.misc_pts[7],UEv.misc_pts[8],
                  UEv.misc_pts[9], UEv.misc_pts[10],UEv.misc_pts[11],UEv.misc_pts[12],UEv.misc_pts[13] );
  return ( 6 + UEv.misc_count ) ;
} /* end gor_calc */

/* Pavlicek Evaluation  per his web site. Count for shortness not length.*/
/* Some Short honors (K,Q,J,KQ,KJ,QK,Qx,Jx) count either the honor HCP (stiff K) or the shortness points (Dpts) but not both.
 * Other Short honors (A, AK, AQ, AJ Ax, Kx ) count both the HCP and the shortness pts. A=6, AK=8 etc.
 * +1 body point for any combination of 4 Aces and 4 Tens ( +2 if all 8 )
 * If a fit is found, Declarer can get length pts for longer trumps and/or 4+ side suit. Len pts depends on fit type. 4-4 or not
 * Dummy can get extra Dfit (suppt) points for a stiff or void with 4+ Trumps, but not for a doubleton.
 */

int pav_calc ( UE_SIDESTAT_k *p_ss ) {      /* Tag Number: 10 */
   int pav_NTpts[2] = {0}; /* Hand value if played in NT */
   int pav_pts[2]   = {0}; /* Hand value if played in a suit */
   int h, s , body_cnt;
   int h_NTdummy = 1 ;              /* will be set by LptsPAV function */
   int mf_sh ;              /* misfit long_hand and short_hand. temp var */
   int slen ;
   HANDSTAT_k *phs_dummy;
   HANDSTAT_k *p_hs, *phs[2];
   zero_globals( p_ss->side ) ;
   phs[0] = p_ss->phs[0];
   phs[1] = p_ss->phs[1];
   p_hs   = p_ss->phs[0] ;
   set_dbg_names(10, "pav_calc");
  
   JGMDPRT(7 , "++++++++++ PAV calc for p_ss->side= %d; compass[0]=%c, compass[1]=%c, phs[0]=%p, phs[1]=%p, hcp[0]=%d, hcp[1]=%d\n",
               p_ss->side, compass[0],compass[1],(void *)phs[0], (void *)phs[1], phs[0]->hs_totalpoints, phs[1]->hs_totalpoints ) ;
   for (h = 0 ; h < 2 ; h++) {         /* for each hand */

      p_hs = phs[h] ; 
      for (s = CLUBS ; s<= SPADES ; s++ ) {
         fhcp_suit[h][s]  = p_hs->hs_points[s]  ;
         fhcp_adj_suit[h][s] = shortHon_adj(p_hs, s, PAV ) ; /* Deduct in a std way in case we play the hand in NT */
         dpts_suit[h][s] = DptsPAV( p_hs, s ) ; /* strict 3/2/1 for _EACH_ shortness incl first dblton */
         fhcp[h] += fhcp_suit[h][s] ;
         fhcp_adj[h] += fhcp_adj_suit[h][s] ;
         dpts[h] += dpts_suit[h][s] ;
         slen = p_hs->hs_length[s];
         JGMDPRT(8,"pav_calc, Hand=%d, suit=%d, SuitLen=%d, fhcp[h][s]=%g, fhcp_adj_suit[h][s]=%g, dpts_suit=%d\n",
                     h, s, slen, fhcp_suit[h][s], fhcp_adj_suit[h][s],dpts_suit[h][s] );
      } /* end for s = CLUBS to Spades*/

           /* Check for Aces and Tens */
      body_cnt = p_hs->hs_totalcounts[idxAces] + p_hs->hs_totalcounts[idxTens] ;
      body_pts[h] = body_cnt / 4 ; /* +1 when  4<= A+T <=7 and +2 if 8 == A+T */

      /* In NT PAV does not count Dpts; so we just need the NTpts + body */
      pav_NTpts[h] = roundf(fhcp[h] + fhcp_adj[h] ) + body_pts[h]   ; /* NT points for hand h -- dont count Dpts for NT */
      if ( pav_NTpts[h] < 0 ) pav_NTpts[h] = 0; /* might go minus with 0 hcp, no aces, and/or a square hand */
      JGMDPRT(7,"PAV NT Result: Hand=%d, NT_pts=%d,  body_pts[h]=%d, body_cnt=%d,fhcp=%g, fhcp_adj=%g \n",
                     h,  pav_NTpts[h], body_pts[h],  body_cnt, fhcp[h], fhcp_adj[h] );
      UEv.nt_pts_seat[h] = pav_NTpts[h] ;
    /* Set the value for a suit contract */
      pav_pts[h] = pav_NTpts[h] + dpts[h];
      JGMDPRT(7,"PAV Suit Prelim Hand=%d pav_pts[h]=%d, NT_pts=%d, dpts=%d\n",  h,  pav_pts[h], pav_NTpts[h], dpts[h] );
   } /* end for each hand */

   /* add +1 for dummy's 5+ suit in NT */
   h_NTdummy =  LptsPAV( phs ) ; /* put the Lpt in one of the global lpts vars; return which hand is NT dummy */
   UEv.nt_pts_seat[h_NTdummy] += lpts[h_NTdummy] ;
   UEv.nt_pts_side = UEv.nt_pts_seat[0] + UEv.nt_pts_seat[1] ; /* dont add Lpt to pav_pts since applies to NT only */

   JGMDPRT(7,"PAV NT pts done.UEv_NTSide_pts=%d, UEv_NT[0]=%d, UEv_NT[1]=%d\n",  UEv.nt_pts_side, UEv.nt_pts_seat[0], UEv.nt_pts_seat[1] );\
   JGMDPRT(7,"Prelim pav_pts=[0]=%d, pav_pts[1]=%d \n",pav_pts[0],pav_pts[1] );

   /* Cant use the TFpts = Do_Df_Fn call because Pav does not count Dfit for both hands if it is 4:4 */

  if ( p_ss->t_fitlen >= 8 ) { /* t_fitlen might be 7 here for a 5-2 fit */
      /* if there is a fit, no need to correct for misfits (if any) so use pav_pts */
      /* if there is a trump fit, add Dfit and Fn Points to the already included Dpts and Honor pts  */
      /* Assume that Declarer and Dummy are defined as per PAV. Decl=Longest trumps or if equal the hand with the most HCP */
      /* If its a 4-4 fit (equal len trumps, only Dummy gets Dfit, Declarer gets Fn (side suit points) */
      h_dummy = p_ss->dummy_h;
      h_decl  = p_ss->decl_h ;
      phs_dummy = phs[h_dummy] ;

      dfit_pts[h_dummy] = DfitPAV (p_ss, h_dummy ) ; /* 4+T:2/1/0 These are in addition to Dpts awarded ealier */
      /* Pavlicek adds FN pts to Decl for side suits and extra trump.
       * But his online evaluator does not match the text description. Here we use what the online evaluator gives.
       */
      Fn_pts[h_decl]    = Fn_ptsPAV(p_ss,h_decl ) ; /* see comments in actual function */
      JGMDPRT(7, "PAV: dfit for dummy[%d]=%d, FN for Decl[%d]=%d\n",h_dummy,dfit_pts[h_dummy],h_decl,Fn_pts[h_decl]);
      if (  (1 == phs_dummy->square_hand) &&
            (0 == phs_dummy->hs_totalcounts[idxAces] ) &&
            (p_ss->t_len[h_dummy] < 4 ) ) {
         hand_adj[h_dummy] = -1 ; /* deduct 1 if 4333 && no Aces && 'minimal fit' */
      }
      pav_pts[h_dummy] += ( dfit_pts[h_dummy] + hand_adj[h_dummy] );
      pav_pts[h_decl]  += Fn_pts[h_decl] ;

      JGMDPRT(7,"PAV TrumpFit Deal=%d, in suit=%d FitLen:%d Decl[%d], Fn=%d Hldf=%d Dummy[%d], Dfit=%d Hldf=%d, SQ_dummy=%d\n",
            gen_num,p_ss->t_suit,p_ss->t_fitlen,h_decl,Fn_pts[h_decl],pav_pts[h_decl],
            h_dummy,dfit_pts[h_dummy],pav_pts[h_dummy], hand_adj[h_dummy] );
   } /* End Trump fit pav_pts all done for this case */

   /* Since there is no 8+ fit, Check for misfits which cannot be ignored. */
   else {
      /* If there is shortness vs pards long (4+) suit, and there is no trump fit, "Do not count for the shortness"
       * This is implies that only the shortness in the misfit suits is not counted. Other shortness OK.
       * This differs from Goren where NO shortness is counted if there is no fit.
       * Unanswered Question: What if there is a 5-2 fit and we end in a suit contract? Should we treat like 8-fit?
       */
      /* we subtract the shortness pts from the hand with the short suit facing partner's long suit */
      for (s=CLUBS; s<=SPADES; s++ ) {
         misfit[s] = misfit_chk(phs , s) ; /* return results in a struct */
         if (misfit[s].mf_type > 3) {  /* Misfit; in the short suit count only the 'corrected' NT pts */
               mf_sh = misfit[s].short_hand ;
               pav_pts[mf_sh] -= dpts_suit[mf_sh][s] ;
               if ( pav_pts[mf_sh] < 0 ) pav_pts[mf_sh] = 0 ; /* a hand can never have -ve pts */
               JGMDPRT(8,"PAV Deal#=%d, No 8+ fit && MISFIT Type:%d sh_len=%d, Deduct[%d] in suit=%d  sh_hand=%d, pav_pts[0]=%d, pav_pts[1]=%d\n",
                  gen_num,misfit[s].mf_type,misfit[s].sh_len, dpts_suit[mf_sh][s],s,mf_sh,pav_pts[0], pav_pts[1] );
         }
         /* No misfit in this suit. no adjustment to pav_pts for a suit contract */
      } /* end for each suit */
   } /* end check for misfit */
   JGMDPRT(7,"PAV Side Fit/Misfit Eval Done  Deal=%d,  End Result for Suit Play: pav_pts[0]=%d, pav_pts[1]=%d\n",
                     gen_num, pav_pts[0], pav_pts[1] );
   /* pav_pts[] now done;  Starting (NT_pts + Dpts) then added Dfit or Fn or corrected for Misfit */
   UEv.hldf_pts_seat[0] = pav_pts[0];
   UEv.hldf_pts_seat[1] = pav_pts[1];
   UEv.hldf_pts_side = UEv.hldf_pts_seat[0] + UEv.hldf_pts_seat[1] ;
   UEv.hldf_suit   = p_ss->t_suit;     /* So we know which dds tricks to count if we are playing in a suit */
   UEv.hldf_fitlen = p_ss->t_fitlen ;
   JGMDPRT(7,"PAV HLDF pts done.UEv_HLDFSide_pts=%d, UEv_HLDF[0]=%d, UEv_HLDF[1]=%d\n",
            UEv.hldf_pts_side, UEv.hldf_pts_seat[0], UEv.hldf_pts_seat[1]);
/* now some debugging fields Standard set even tho some don't apply to this metric */
   UEv.misc_count = 0 ;
      /* The factors that apply to both NT and Suit */
      UEv.misc_pts[UEv.misc_count++] = roundf(fhcp_adj[0]) ;
      UEv.misc_pts[UEv.misc_count++] = body_pts[0];
      UEv.misc_pts[UEv.misc_count++] = lpts[0];   /* possibly +1 for 5card suit in NT */
      /* Factors that apply to suit contracts only */
      UEv.misc_pts[UEv.misc_count++] = Fn_pts[0];     /* Declarer side suit and extra trump length */
      UEv.misc_pts[UEv.misc_count++] = dpts[0] ;
      UEv.misc_pts[UEv.misc_count++] = dfit_pts[0];   /* Dummy support pts typically 3-2-1 with 3, 5-3-1 with 4+ */
      UEv.misc_pts[UEv.misc_count++] = hand_adj[0];   /* square, aceless dummy with minimal (<9) fit */
      /* Ditto for hand 1 */
      UEv.misc_pts[UEv.misc_count++] = roundf(fhcp_adj[1]) ;
      UEv.misc_pts[UEv.misc_count++] = body_pts[1];
      UEv.misc_pts[UEv.misc_count++] = lpts[1];
      UEv.misc_pts[UEv.misc_count++] = Fn_pts[1];
      UEv.misc_pts[UEv.misc_count++] = dpts[1] ;
      UEv.misc_pts[UEv.misc_count++] = dfit_pts[1];
      UEv.misc_pts[UEv.misc_count++] = hand_adj[1];
      JGMDPRT(7,"PAV UEv misc_count=%d,[h0]=%d,%d,%d,%d,%d,%d,%d\n", UEv.misc_count, UEv.misc_pts[0],UEv.misc_pts[1],
                   UEv.misc_pts[2], UEv.misc_pts[3], UEv.misc_pts[4],UEv.misc_pts[5], UEv.misc_pts[6] );
      JGMDPRT(7,"PAV UEv misc_count=%d,[h1]=%d,%d,%d,%d,%d,%d,%d\n", UEv.misc_count, UEv.misc_pts[7],UEv.misc_pts[8],
                  UEv.misc_pts[9],  UEv.misc_pts[10],UEv.misc_pts[11],UEv.misc_pts[12],UEv.misc_pts[13] );

  SaveUserVals( UEv , p_uservals ) ;

  return ( 6 + UEv.misc_count ) ;
} /* end pav_calc */


/* PAV: +1 for 5+ suit in Dummy if playing NT contract*/
int LptsPAV( HANDSTAT_k *phs[] )  {
   int s ;
   int NTdummy = 1;
   if ( phs[0]->hs_totalpoints <= phs[1]->hs_totalpoints ) { NTdummy = 0 ; }
   lpts[NTdummy] = 0 ;
   for (s = CLUBS; s <= SPADES ; s++ ) {
      if ( phs[NTdummy]->hs_length[s] >= 5 ) { lpts[NTdummy] = 1 ; }
   }
   JGMDPRT(7,"LptsPAV: NTDummy=%d, lpts[%d]=%d \n",NTdummy, NTdummy, lpts[NTdummy] );
   return NTdummy ;
}

int DptsPAV( HANDSTAT_k *p_hs, int s ) { /* Count for shortness; deduct the shortness pts later if there is a misfit. Count even short honors */
   int dpts ;
   int ss_idx ;
   int weight ;
   dpts = 0 ;
   if ( 0 == p_hs->hs_length[s] ) { JGMDPRT(8,"PAV Shortness pts returns 3 from Void \n");    return 3 ; }
   if ( 3 <= p_hs->hs_length[s] ) { JGMDPRT(8,"PAV Shortness pts returns 0 from 3+ suit \n"); return 0 ; }
   if ( 1 <= p_hs->hs_length[s] &&  p_hs->hs_length[s] <= 2 ) {
      weight = p_hs->topcards[s][0] + p_hs->topcards[s][1] ;
      ss_idx = ss_index(weight) ;
      dpts = Dpts_GorPav[1][ss_idx] ;  /* ss_idx points to one of 22 possible short suit combos */
      JGMDPRT(8,"PAV Shortness pts returns %d from short suit \n",dpts);
      return dpts ;
   }
   /* NOTREACHED */
   return dpts ;
} /* end Dpts_PAV */

int DfitPAV( UE_SIDESTAT_k *p_ss, int du) { /* phs will point to Dummy's hand -- the one getting the Dfit pts */
   int ss_len, ss_cnt, DFit_pts, s ;
   int DF_PAV[4] =  {2,1,0,0} ; /* Dfit pts  added to orginal Dpts if dummy has 4+ Trump Nothing for Doubletons*/
   HANDSTAT_k *phs_du = p_ss->phs[du];
   JGMDPRT(7, "PAV Calc Dfit for du seat=%d:%c, du=%d, tlen=%d, and ss_len=%d\n",
               p_ss->dummy_seat,compass[du], du, p_ss->t_len[du], p_ss->sorted_slen[du][3] );

   if (p_ss->t_len[du] <= 3 ) { return 0 ; }
   ss_cnt = 0 ;
   DFit_pts = 0 ;
  /* PAV with 4+ Trump counts Dfit for EACH shortness; but Dbltons dont get any extra, so would need two stiffs, or stiff and void */
   for (s=0 ; s<4 ; s++ ) {
      ss_len = phs_du->hs_length[s] ;
      if( ss_len < 3) { DFit_pts += DF_PAV[ ss_len ] ; ss_cnt++ ; }  /* the trump suit will never be a short suit here */
   }
     JGMDPRT(7, "DfitPAV, Dummy=[%d], t_len[du]=%d, from sslen=%d, Tot_dfit=%d, sscnt=%d\n", du,p_ss->t_len[du],ss_len,DFit_pts,ss_cnt);

   return DFit_pts ;
} /* end DfitPAV */

/* Fn Points PAV: With an 8+ fit Declarer adds pts for extra trump length, and for side suit length
 * Rule 1: Every trump over five +2 pts
 * Rule 2: with 9+ fit +1 for 5th trump
 * Rule 3: Every side suit (only 1 unless 5440, or 4441) +1 for each card over 3
 * Rule 4: With a 4-4 fit, -1 from EACH side suit value, i.e. every 4 card side suit=0, 5 cards=1, 6 cards=2 etc.
 */
int Fn_ptsPAV( UE_SIDESTAT_k *p_ss, int dc) { /* phs will point to Declarer's hand -- the one getting the Fn pts */
   int  fn_pts ;
   int s ;
   int tlen_pts, side_suit_pts;
   HANDSTAT_k *phs_dc = p_ss->phs[dc] ;
   JGMDPRT(7, "PAV FN_pts Declarer dc=%d, seat[dc]=%d, \n",dc,seat[dc] ) ;
   fn_pts  = 0 ; tlen_pts = 0 ; side_suit_pts = 0 ;
   if (p_ss->t_len[dc] > 5 ) { tlen_pts = (p_ss->t_len[dc]-5) * 2 ; } /* always +2 for each trump >5 even in 6-2, 7-1 etc. fit */
   if (p_ss->t_fitlen >= 9 ) { tlen_pts++ ; } /* if there is a 9 fit the 5th trump gets +1 5-4, 6-3, 7-2 etc. */
   for (s = CLUBS; s <=SPADES ; s++ ) {
         if (s != p_ss->t_suit && phs_dc->hs_length[s] > 3 ) {
            side_suit_pts += (phs_dc->hs_length[s] - 3) ; }
            if ( (p_ss->t_len[dc] == 4) && (side_suit_pts > 0) ) { /* must be a 4-4 fit if Decl (longest trump hand) has only 4*/
                  side_suit_pts-- ; /* if Decl has only 4 Trump, EVERY side suit downgraded by a pt */
   }
         JGMDPRT(8,"**** FN-SideSuit=%d,  SideSuitLen=%d, FN_SideSuitPts=%d\n",s,phs_dc->hs_length[s], side_suit_pts ) ;
   } /* end side suit loop */
   if ( (p_ss->t_len[dc] == 4) && (side_suit_pts > 0) ) { /* must be a 4-4 fit if Decl (longest trump hand) has only 4*/
      side_suit_pts-- ; /* subtract 1 from side suit pts in a 4-4 fit; 4 card side-suit=0, 9 card side-suit = 5 */
   }
   fn_pts = side_suit_pts + tlen_pts ;
   JGMDPRT(7,"PAV FN pts for hand=[%d] Tot=%d, Tlen=%d, side_suit=%d, fit_len=%d, Decl_Tlen=%d \n",
                                 dc, fn_pts,tlen_pts,side_suit_pts,p_ss->t_fitlen,p_ss->t_len[dc] );
   return fn_pts ;
} /* end Fn_ptsPAV */

int DptsGOREN( HANDSTAT_k *p_hs, int s ) { /* Count for shortness; deduct the shortness pts later if there is a misfit. Count even short honors */
   int dpts ;
   int ss_idx ;
   int weight ;
   dpts = 0 ;
   if ( 0 == p_hs->hs_length[s] ) { JGMDPRT(8,"GOR Shortness pts returns 3 from Void \n");    return 3 ; }
   if ( 3 <= p_hs->hs_length[s] ) { JGMDPRT(8,"GOR Shortness pts returns 0 from 3+ suit \n"); return 0 ; }
   if ( 1 <= p_hs->hs_length[s] &&  p_hs->hs_length[s] <= 2 ) {
      weight = p_hs->topcards[s][0] + p_hs->topcards[s][1] ;
      ss_idx = ss_index(weight) ;
      dpts = Dpts_GorPav[0][ss_idx] ; /* ss_idx points to one of 22 possible short suit combos */
      JGMDPRT(8,"GOR Shortness pts returns %d from short suit \n",dpts);
      return dpts ;
   }
     /* NOT REACHED */
   return dpts ;
} /* end Dpts_GOREN */

/* Extra Ruffing pts for shortness: 4 Trump: +2/+1/0 ; 3 Trump: +1/0/0 so void is worth 4 pts not 3. */
int DfitGOREN( UE_SIDESTAT_k *p_ss, int du) { /* du will point to Dummy's hand -- the one getting the Dfit pts */
   int DFit_pts, s, ss_cnt, ss_len;
   int DF_GOREN[4] =  {2,1,0,0} ;
   HANDSTAT_k *phs_du = p_ss->phs[du];

   /* Goren with 4+ Trump counts Dfit for EACH shortness; but Dbltons dont get any extra, so would need two stiffs, or stiff and void */
   JGMDPRT(7, "GOR Calc Dfit for du seat=%d:%c, du=%d, tlen=%d, and ss_len=%d\n",
               p_ss->dummy_h,compass[du], du, p_ss->t_len[du], p_ss->sorted_slen[du][3] ); /* [3] is the short suit */

   if (p_ss->t_len[du] <  3 ) { return 0 ; }  /* no extra Dfit with < 3 trump but keep +1 for EACH dblton */
   if (p_ss->t_len[du] == 3 ) {
      if (p_ss->sorted_slen[du][3] == 0 ) { return 1 ; } /* a void with 3 trump gets +1 Dfit pt */
      else                                { return 0 ; }
   } /* end tlen == 3 */
   /* assert p_ss->t_len[du] > 3 award Dfit pts */
   ss_cnt = 0 ;
   DFit_pts = 0 ;
   for (s=CLUBS; s<=SPADES; s++ ) {  /* count for EACH shortness */
      if ( s == p_ss->t_suit ) continue ;
      ss_len = phs_du->hs_length[s] ;
      if (ss_len < 2 ) { DFit_pts += DF_GOREN[phs_du->hs_length[s]] ; ss_cnt++ ;}
    }
    JGMDPRT(7, "DfitGOR, Dummy=[%d], t_len[du]=%d, from sslen=%d, Tot_DFit_pts=%d, sscnt=%d\n", du,p_ss->t_len[du],ss_len,DFit_pts,ss_cnt);
   return DFit_pts ;
} /* end DfitGOREN */

/* Declarer adds pts for extra trump length, and for side suit length */
int Fn_ptsGOREN( UE_SIDESTAT_k *p_ss, int dc) { /* phs will point to Declarer's hand -- the one getting the Fn pts */
   int  fn_pts ;
   int tlen_pts;

   JGMDPRT(7, "GOR FN_pts Declarer dc=%d, seat[dc]=%d, \n",dc,seat[dc] ) ;
   fn_pts  = 0 ; tlen_pts = 0 ;
   /* GOREN add for length in Declarer's hand once a trump fit is found: extra trump suit length, NO side suit length */

   /* Check for extra trump length: +1 if 5 */
   if ( p_ss->t_len[dc] >= 5 ) { tlen_pts++ ; }  /* +1 pt for the 5th trump always -- Differs from Pav */
   if (p_ss->t_len[dc] > 5 ) {tlen_pts += (p_ss->t_len[dc] - 5 )*2 ; }         /* +2 for each trump over 5; */
   fn_pts = tlen_pts;
   JGMDPRT(7,"Fn_ptsGOR Decl=[%d;%c] Fn_pts=%d Tlen=%d\n", dc,"NESW"[p_ss->decl_seat],fn_pts,p_ss->t_len[dc] );
   return fn_pts ;
} /* end Fn_ptsGOREN */

int Hf_ptsGOREN (  HANDSTAT_k *p_hs , int tsuit) { /* p_hs points to dummy hand; the only one getting Hf pts */
   /* Goren Hf Points:  With 1-3 hcp in the trump suit, add +1 ; PAV says Dummy only */
   int Hf_pts = 0;
   if ( 0 < p_hs->hs_points[tsuit] &&  p_hs->hs_points[tsuit] <= 3 ) {Hf_pts = 1 ;}
   JGMDPRT(7,"Hf_ptsGOREN =hand=%d, suit=%d, hcp=%d, Hf_pts=%d\n",p_hs->hs_seat, tsuit,p_hs->hs_points[tsuit],Hf_pts ) ;
   return Hf_pts ;
}
/* give any Stiff Q or J in pards 4+ suit full value instead of zero */

