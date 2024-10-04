/* File sheinwold_calc.c */
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
extern int ss_index(int weight) ; 
extern float_t QuickTricks_suit( HANDSTAT_k *phs , int s ) ;
extern float_t QuickTricks( HANDSTAT_k *phs );

extern void SaveUserVals(struct UserEvals_st UEv , USER_VALUES_k *p_ures ) ;
extern void set_dbg_names(int m, char *dbg_func) ;
  
 /* Forward Function Definitions */
 int DptsSHEINW(   HANDSTAT_k    *p_hs, int s ) ;
 int Hf_ptsSHEINW( HANDSTAT_k *phs_du, HANDSTAT_k *phs_dc, int Hf_hand) ;
 int DfitSHEINW(   UE_SIDESTAT_k *p_ss, int du) ;
 int Fn_ptsSHEINW( UE_SIDESTAT_k *p_ss, int dc) ;

static int Dpts_SHEINW[22] = 
   //               A K Q J T x  AK     Ax  KQ    Kx  QJ  Qx JT  Tx,xx
   {  /*Sheinwold*/ 2,2,2,2,2,2, 1,1,1,1,1, 1,1,1,1,  1,1,1, 1,1, 1,1 } ;


// main or userfunc has a) set all the mm_ptrs etc. b) used prolog to zero globals c) filled sidestat global with trump suit choice.

/* Sheinwold Per Book: 5 Weeks to Winning Bridge 1959,1964 */
int sheinw_calc (UE_SIDESTAT_k *p_ss) {     /* Tag Number: 11 */
   int sheinw_NTpts[2] = {0}; /* Hand value if played in NT */
   int sheinw_pts[2]   = {0}; /* Hand value if played in a suit */
   float_t qtrix[2]    = {0}; /* For interest only now. Maybe use to adjust hand value later? */
   int h, s, h_pard ;
   HANDSTAT_k *phs_dummy, *phs_decl, *phs_pard ;
   HANDSTAT_k *p_hs, *phs[2];
   zero_globals( p_ss->side ) ; 
   phs[0] = p_ss->phs[0];
   phs[1] = p_ss->phs[1];
   p_hs   = p_ss->phs[0] ;
   set_dbg_names(11, "sheinw_calc");
   JGMDPRT(7 , "++++++++++ SHEINW calc for p_ss->side= %d; compass[0]=%c, compass[1]=%c, phs[0]=%p, phs[1]=%p, hcp[0]=%d, hcp[1]=%d\n",
               p_ss->side, compass[0],compass[1],(void *)phs[0], (void *)phs[1], phs[0]->hs_totalpoints, phs[1]->hs_totalpoints ) ;
   for (h = 0 ; h < 2 ; h++) {         /* for each hand */
      p_hs = phs[h] ; 
      h_pard = (h == 0 ) ? 1 : 0 ;
      p_hs = phs[h] ; /* phs array set by prolog to point to hs[north] and hs[south] OR to hs[east] and hs[west] */
      phs_pard = phs[h_pard];
      for (s = CLUBS ; s<= SPADES ; s++ ) {
         fhcp_suit[h][s]  = p_hs->hs_points[s]  ;
         fhcp_adj_suit[h][s] = shortHon_adj(p_hs, s, SHEINW ) ; /* Deduct in a std way in case we play the hand in NT */
         dpts_suit[h][s] = DptsSHEINW( p_hs, s ) ; /* strict 3/2/1 for _EACH_ shortness incl first dblton */
         /*subtract half pt for Qxx or Jxx unless it is in an 8fit suit.  QJx no deduction */
         if ( (p_hs->hs_length[s] >= 3) && ( (phs_pard->hs_length[s] + p_hs->hs_length[s]) < 8) ) {
            if (p_hs->Has_card[s][QUEEN] && p_hs->hs_counts[idxTop4][s] == 1 ) { // Qxx with no A, K or J
               fhcp_adj_suit[h][s] = -0.5 ;
            }
            if (p_hs->Has_card[s][JACK]  && p_hs->hs_counts[idxTop3][s] == 0 ) { // Jxx with No A, K or Q
               fhcp_adj_suit[h][s] = -0.5 ;
            }
         }
         qtrix[h] += QuickTricks_suit(p_hs, s ) ;
         /* Should we adjust for QuickTricks?
          * Book says open any 14, To open 12 needs 2QT; 13 ok with 1.5QT if a Major, or 4=4 Majors
          * Not sure how or IF to allow for this. +/- 1 pt per 0.5QT?
          */
         /*  end hcp and dpts calc for this suit */
         fhcp[h] += fhcp_suit[h][s] ;
         fhcp_adj[h] += fhcp_adj_suit[h][s] ;
         dpts[h] += dpts_suit[h][s] ;

         JGMDPRT(7,"sheinw_calc, Hand=%d, suit=%d, SuitLen=%d, fhcp[h][s]=%g, fhcp-adj[h][s]=%g, Dpts[h][s]=%d, RunQT=%g\n",
                     h, s, p_hs->hs_length[s], fhcp_suit[h][s], fhcp_adj_suit[h][s], dpts_suit[h][s], qtrix[h] );
      } /* end CLUBS <= s <= SPADES */
      /* Check for Aces or lack of +1 for 3-4 Aces, -1 for no Aces*/
      body_pts[h] = (p_hs->hs_totalcounts[idxAces] >= 3 ) ?  1 :
                    (p_hs->hs_totalcounts[idxAces] == 0 ) ? -1 : 0 ;

      JGMDPRT(7,"sheinw_calc, Hand=%d, fhcp[h]=%g, fhcp-adj[h]=%g, Dpts[h]=%d, body_pts[h]=%d,Qtrix=%g \n",
                     h, fhcp[h], fhcp_adj[h], dpts[h],body_pts[h], qtrix[h] );
      /* In NT Do not count Dpts; so we just need the HCP + ShortHonor_ADJ + body */
      sheinw_NTpts[h] = roundf(fhcp[h] + fhcp_adj[h]) + body_pts[h]   ; /* NT points for hand h -- dont count Dpts for NT */
     JGMDPRT(7,"SHEINW NT Result: Hand=%d, NT_pts=%d, fhcp=%g, fhcp_adj=%g, body_pts=%d,Qtricks=%g\n",
                     h,  sheinw_NTpts[h],fhcp[h],fhcp_adj[h], body_pts[h], qtrix[h]);

      UEv.nt_pts_seat[h] = sheinw_NTpts[h] ;
      /* We will add to NT points later when we calculate the Hf and FN pts. Right now we do not know who is Declarer */
      /* get Prelim value for hand in suit contract; will be affected by any later discovered misfits; also by Dfit_pts */
      /* in a suit contract we add the Dpts to the NT pts. The short Hon Ded are chosen so this comes out right */
      sheinw_pts[h] = sheinw_NTpts[h] + dpts[h] ;

      JGMDPRT(7,"SHEINW Suit Prelim Val[%d]=%d :: fhcp=%g fhcp_adj=%g dpts=%d body_pts=%d\n",
                           h, sheinw_pts[h], fhcp[h], fhcp_adj[h], dpts[h], body_pts[h]);
    } /* end foreach hand */
    if (sheinw_NTpts[0] >= sheinw_NTpts[1] ) { h_decl = 0; h_dummy = 1 ; } /* mostly for debugging */
    else                                     { h_decl = 1; h_dummy = 0 ; }

    /* ------------------ now check the side as a whole. Fits, DFit_pts, Hf_pts, FN_pts */
    /* Sheinwold says as Dummy do not count Dpts, count Dfit pts on a 5-3-1 scale with 4+ Trump.
     * with only 3 trump 'subtract a point' so 4-2-0 instead of the PAV/GOR 3-2-1 ?
     * No Deductions for shortness vs pards long suit(s) unlike Goren and Pavlicek
     */
   if ( p_ss->t_fitlen >= 8) {   /* t_fitlen might 7 here which means a 5-2 fit */
      h_dummy = p_ss->dummy_h;
      h_decl  = p_ss->decl_h ;

      JGMDPRT(7," -- TrumpSuit=%d, Decl=%d, Dummy=%d \n", p_ss->t_suit, h_decl, h_dummy);
        /* if there is a trump fit, **Replace** Dummy Dpts with Dfit_pts; **Add** Hf_pts to Dummy.
         *                          **Add** Fn Points to Declarer
         */
      phs_dummy = phs[h_dummy] ;
      phs_decl  = phs[h_decl]  ;
      Fn_pts[h_decl]    = Fn_ptsSHEINW(p_ss, h_decl  ) ; /* 5th Trump +1, 6th & up +2 each.) Also +1 for solid 5+side suit*/
      dfit_pts[h_dummy] = DfitSHEINW  (p_ss, h_dummy ) ; /* REPLACE Dpts with Dfit pts 5/3/1 or 4/2/0 or 0/0/0*/
      sheinw_pts[h_dummy] -= dpts[h_dummy] ;
      hf_pts[h_dummy] = Hf_ptsSHEINW( phs_dummy , phs_decl, p_ss->t_suit ) ; /* +1 for Qx, Jx, in trump and QJ,Qx in side suit. */
      JGMDPRT(7, "SHEINW: dummy[%d] Dfit=%d, Hf=%d, FN for Decl[%d]=%d, Subtracted %d Dpts\n",
                         h_dummy,dfit_pts[h_dummy],hf_pts[h_dummy],h_decl,Fn_pts[h_decl], dpts[h_dummy]);
      sheinw_pts[h_dummy] += ( dfit_pts[h_dummy] + hf_pts[h_dummy] );
      sheinw_pts[h_decl]  += Fn_pts[h_decl];
      JGMDPRT(7,"SHEINW TrumpFit Deal=%d, in suit=%d Len:%d Decl=[%d], Fn=%d, Hldf=%d Dummy=[%d], Dfit=%d Hf=%d, Hldf=%d\n",
            gen_num,p_ss->t_suit,p_ss->t_fitlen,h_decl,Fn_pts[h_decl],sheinw_pts[h_decl],
            h_dummy,dfit_pts[h_dummy],hf_pts[h_dummy],sheinw_pts[h_dummy] );
   } /* End Trump fit sheinw_pts all done for this case */
   else {
      JGMDPRT(7," -- Sheinwold No Trump fit, no Fn, no Dfit, keep Dpts \n") ;
      ; /* NO-OP when JGMDBG is not defined */
   } /* end fitlen >= 8 or not */

   /* Now we add any Hf and Fn pts to the NT values as well */
      UEv.nt_pts_seat[h_decl]  += Fn_pts[h_decl] ;
      UEv.nt_pts_seat[h_dummy] += hf_pts[h_dummy];
      UEv.nt_pts_side = UEv.nt_pts_seat[0] + UEv.nt_pts_seat[1] ;
      JGMDPRT(7,"SHEINW Final NTpts with Hf and Fn incl. Tot=%d,Decl[%d]=%d,Dummy[%d]=%d\n",
                  UEv.nt_pts_side, h_decl, UEv.nt_pts_seat[h_decl], h_dummy, UEv.nt_pts_seat[h_dummy] ) ;
      UEv.hldf_pts_seat[0] = sheinw_pts[0];
      UEv.hldf_pts_seat[1] = sheinw_pts[1];
      UEv.hldf_pts_side = UEv.hldf_pts_seat[0] + UEv.hldf_pts_seat[1] ;
      UEv.hldf_suit   = p_ss->t_suit;     /* So we know which dds tricks to count if we are playing in a suit */
      UEv.hldf_fitlen = p_ss->t_fitlen ;
      JGMDPRT(7,"SHEINW Side Fit/Misfit Eval Done  Deal=%d,  End Result for Suit Play:Tot=%d, Decl[%d]=%d, Dummy[%d]=%d\n",
                                         gen_num, UEv.hldf_pts_side,h_decl,sheinw_pts[h_decl],h_dummy,sheinw_pts[h_dummy] );
/* now some debugging fields */
   UEv.misc_count = 0 ;
      /* The factors that apply to both NT and Suit */
      UEv.misc_pts[UEv.misc_count++] = roundf(fhcp_adj[0]);
      UEv.misc_pts[UEv.misc_count++] = body_pts[0];
      UEv.misc_pts[UEv.misc_count++] = hf_pts[0];
      UEv.misc_pts[UEv.misc_count++] = Fn_pts[0];     /* Declarer side suit and extra trump length */
      /* Factors that apply to suit contracts only */
      UEv.misc_pts[UEv.misc_count++] = dpts[0];       /* distribution aka shortness pts before a fit is found */
      UEv.misc_pts[UEv.misc_count++] = dfit_pts[0];   /* Dummy support pts typically 3-2-1 with 3, 5-3-1 with 4+ */
      UEv.misc_pts[UEv.misc_count++] = roundf(qtrix[0]*100);

      /* same again for hand 1 */
      UEv.misc_pts[UEv.misc_count++] = roundf(fhcp_adj[1]);
      UEv.misc_pts[UEv.misc_count++] = body_pts[1];
      UEv.misc_pts[UEv.misc_count++] = hf_pts[1];
      UEv.misc_pts[UEv.misc_count++] = Fn_pts[1];
      UEv.misc_pts[UEv.misc_count++] = dpts[1];
      UEv.misc_pts[UEv.misc_count++] = dfit_pts[1];
      UEv.misc_pts[UEv.misc_count++] = roundf(qtrix[1]*100);

      JGMDPRT(7,"SHEINW UEv misc_count=%d,[h0]=%d,%d,%d,%d,%d,%d,%d\n", UEv.misc_count, UEv.misc_pts[0],UEv.misc_pts[1],
                  UEv.misc_pts[2], UEv.misc_pts[3],UEv.misc_pts[4],UEv.misc_pts[5], UEv.misc_pts[6]);
      JGMDPRT(7,"SHEINW UEv misc_count=%d,[h1]=%d,%d,%d,%d,%d,%d,%d\n", UEv.misc_count, UEv.misc_pts[7],UEv.misc_pts[8],
                  UEv.misc_pts[9], UEv.misc_pts[10],UEv.misc_pts[11],UEv.misc_pts[12], UEv.misc_pts[13]);

   SaveUserVals( UEv , p_uservals ) ;
   return ( 6 + UEv.misc_count ) ;
} /* end sheinw_calc */




int DptsSHEINW( HANDSTAT_k *p_hs, int s ) { /* Count for shortness; deduct the shortness pts later if there is a misfit. Count even short honors */
   int dpts ;
   int ss_idx ;
   int weight ;
   dpts = 0 ;
   if ( 0 == p_hs->hs_length[s] ) { JGMDPRT(7,"SHEINW Shortness pts returns 3 from Void \n");    return 3 ; }
   if ( 3 <= p_hs->hs_length[s] ) { JGMDPRT(7,"SHEINW Shortness pts returns 0 from 3+ suit \n"); return 0 ; }
   if ( 1 <= p_hs->hs_length[s] &&  p_hs->hs_length[s] <= 2 ) {
      weight = p_hs->topcards[s][0] + p_hs->topcards[s][1] ;
      ss_idx = ss_index(weight) ;
      dpts = Dpts_SHEINW[ss_idx] ;
      JGMDPRT(7,"SHEINW Shortness pts returns %d from short suit[%d] \n",dpts,s);
      return dpts ;
   }
     /* NOT REACHED */
   return dpts ;
} /* end Dpts_SHEINW */

int DfitSHEINW( UE_SIDESTAT_k *p_ss, int du) { /* du will point to Dummy's hand -- the one getting the Dfit pts */
   int DFit_pts, s, ss_cnt, ss_len;
   int DF_SHEINW[4] =  {5,3,1,0} ;     /* The values with 4 Trump. 3 Trump is one less, and 2 Trump is zero. */
   HANDSTAT_k *phs_du = p_ss->phs[du] ;
   JGMDPRT(7, "SHEINW Calc Dfit for du seat=%d:%c, du=%d, tlen=%d, and all short suits\n",
               p_ss->dummy_h,compass[du], du, p_ss->t_len[du] );

   if (p_ss->t_len[du] <= 2 ) { return 0 ; }  /* this will replace any Dpts previously assigned */

   ss_cnt = 0 ;
   DFit_pts = 0 ;
   for (s=CLUBS; s<=SPADES; s++ ) { /* Dfit for EACH shortness */
      if ( s == p_ss->t_suit ) continue ;
      ss_len = phs_du->hs_length[s] ;
      if (ss_len <= 2 ) {
         DFit_pts += DF_SHEINW[ss_len] ; ss_cnt++ ; /* V + Stiff will get 8 Df pts with 4 trump*/
         if (p_ss->t_len[du] == 3 )  { DFit_pts-- ; } /*Subtract 1 if only 3 trumps; thus Void=4, Stiff=2, Dblton=0 V+Stiff= 6 still */
         JGMDPRT(7, "Dfit_SHEINW--Dummy=%d, suit=%d, ss_len=%d, DfitPts=%d, Tot_DFit_pts=%d, sscnt=%d\n",
                                 du, s, ss_len, DF_SHEINW[ss_len], DFit_pts, ss_cnt);
      }
    }
   JGMDPRT(7, "Total Dfit_SHEINW, Dummy=[%d], t_len[du]=%d, Tot_DFit_pts=%d, sscnt=%d\n", du,p_ss->t_len[du],DFit_pts,ss_cnt);

   return DFit_pts ;
} /* end DfitSHEINW */

/* Declarer adds pts for extra trump length, and for side suit length */
int Fn_ptsSHEINW( UE_SIDESTAT_k *p_ss, int dc) {
   int  fn_pts = 0 ;
   int tlen_pts = 0 ;
   int s;

   HANDSTAT_k *phs_dc = p_ss->phs[dc] ;
   HANDSTAT_k *phs_du = p_ss->phs[!dc] ;  /* if dc=0 then !dc=1 and vice versa */


   JGMDPRT(7, "SHEIN FN_pts Declarer decl=%d, dc=%d, seat[0]=%d,seat[1]=%d \n",p_ss->decl_h, dc, seat[0], seat[1] ) ;
   fn_pts  = 0 ; tlen_pts = 0 ;
   /* SHEINW add for length in Declarer's hand once a trump fit is found: extra trump suit length, SOLID side suit length */

   /* Check for extra trump length: +1 if 5 */
   if ( p_ss->t_len[dc] >= 5 ) { tlen_pts++ ; }  /* +1 pt for the 5th trump always -- Differs from Pav */
   if ( p_ss->t_len[dc] >  5 ) { tlen_pts += (p_ss->t_len[dc] - 5 )*2 ; }         /* +2 for each trump over 5; */
   JGMDPRT(7,"Fn_ptsSHEINW for hand[%d] Trump suit[%d] Tlen=%d, Extra FN_pts=%d\n",dc,p_ss->t_suit,p_ss->t_len[dc],tlen_pts );
   for (s=CLUBS; s<=SPADES ; s++ ) {
      if (s != p_ss->t_suit && phs_dc->hs_counts[idxTop4][s] == 4 && phs_dc->hs_length[s] >= 5 ) { fn_pts++ ; } /* +1 for SOLID 5+ side suit */
      if (s != p_ss->t_suit && phs_dc->hs_length[s] >=5 && phs_du->hs_length[s] >= 3 ) {
          fn_pts += 1 + ( phs_dc->hs_length[s] - 5) * 2 ;
       } /* 1 + (L-5)*2 for ANY 5+ suit that has been raised. */
   JGMDPRT(7,"Fn_ptsSHEINW for hand[%d] Sidesuit[%d] Extra FN_pts=%d\n",dc,s,fn_pts );
   }
   fn_pts += tlen_pts ;
   JGMDPRT(7,"Tot Fn_pts SHEINW Decl=[%d;%c] Tot_Fn_pts=%d Tlen_pts=%d, Tlen=%d\n",
                                 dc,"NESW"[p_ss->decl_h],fn_pts,tlen_pts,p_ss->t_len[dc] );
   return fn_pts ;
} /* end Fn_ptsSHEINW */

int Hf_ptsSHEINW ( HANDSTAT_k *phs_du, HANDSTAT_k *phs_dc, int tsuit) {
   /* Sheinwold Hf: Either adds +1 for Qx(x), Jx(x), or QJx(x) in Tsuit; Dummy +1 also for Qx or QJ in side suit */
   int Hf_pts = 0;
   int Hf_side_suit = 0 ;
   int s ;
   //fprintf(stderr, "########################## HfPts_Sheinwold for Trump suit=%d ###################\n",tsuit) ;
   //fprintf(stderr, "phs_Decl->hs_seat=%d, phs_du->hs_seat=%d\n",phs_dc->hs_seat, phs_du->hs_seat );
   //fprintf(stderr, "Declarer Trump HCP=%d, Dummy Trump HCP=%d \n",phs_dc->hs_points[tsuit], phs_du->hs_points[tsuit]);

   /* This next bit adds 1 Hf pt for Q, J, or QJ in the trump suit by EITHER. Jxxx vs Qxxx BOTH get 1 Hf pt */
   if ( (phs_du->hs_points[tsuit] <= 3) && (phs_du->Has_card[tsuit][QUEEN] ||  phs_du->Has_card[tsuit][JACK]) ) {
      Hf_pts++ ;
      JGMDPRT(7,"Hf Point assigned to DUMMY    in Trump suit %d\n",tsuit);
   }
    if ( (phs_dc->hs_points[tsuit] <= 3) && (phs_dc->Has_card[tsuit][QUEEN] ||  phs_dc->Has_card[tsuit][JACK]) ) {
      Hf_pts++ ;
      JGMDPRT(7,"Hf Point assigned to DECLARER in Trump suit %d\n",tsuit);
   }
    /* Dummy can also add an Hf pt for the Q or QJ in Declarer's (4+ ?) side suit */
   for (s = CLUBS; s <= SPADES ; s++ ) {   /* +1 for Qx or QJ in pard's side suit */
 /*     fprintf(stderr, "Checking Side Suit=%d, DeclLen=%d, Dummy: Len=%d,HCP=%d,Top3=%d,HasQueen=%d\n",
                  s,  phs_dc->hs_length[s],phs_du->hs_length[s],phs_du->hs_points[s],phs_du->hs_counts[idxTop3][s],phs_du->Has_card[s][QUEEN] );
*/

      if ( (phs_dc->hs_length[s] >=4 ) && (s != tsuit) &&
           (phs_du->hs_counts[idxTop3][s] == 1) && phs_du->Has_card[s][QUEEN] && (phs_du->hs_length[s] >= 2) ) {
         Hf_side_suit++ ;
         JGMDPRT(7,"Hf Point assigned to Dummy in Side Suit %d\n",s);
      }
   }
   /* The Hf pts are all ascribed to 'dummy' even in the case when it is only Decl getting them e.g. Jxx vs AQxxx say
    * but the long hand is the declarer by the Pavlicek logic
    */
   JGMDPRT(7,"Hf_ptsSHEINW =Dummyhand=%d, Tsuit=%d, Tsuit_hcp=%d, Tot_Hf_pts=%d, Hf_ssPts=%d\n",
               phs_du->hs_seat, tsuit, phs_du->hs_points[tsuit], Hf_pts, Hf_side_suit ) ;
   return (Hf_pts + Hf_side_suit) ;
} /* end Hf_ptsSHEINW */

