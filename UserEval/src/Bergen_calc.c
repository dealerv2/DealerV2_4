/* File bergen_calc.c */
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
extern float_t shortHon_adj( HANDSTAT_k *p_hs, int suit, int mtag ) ;
extern struct misfit_st misfit_chk( HANDSTAT_k *phs[], int s ) ; 
extern void SaveUserVals(struct UserEvals_st UEv , USER_VALUES_k *p_ures ) ;
extern FIT_PTS_k Do_Df_Fn_pts( UE_SIDESTAT_k *p_ss,
                               int (*calc_dfval)(UE_SIDESTAT_k *p_ss, int h),
                               int (*calc_fnval)(UE_SIDESTAT_k *p_ss, int h)           ) ;
extern void set_dbg_names(int m, char *dbg_func) ; 
 /* Forward Function Definitions */

 int DfitBERG(   UE_SIDESTAT_k *p_ss, int du) ;
 int Fn_ptsBERG( UE_SIDESTAT_k *p_ss, int dc) ;
 int Hf_ptsBERG( HANDSTAT_k    *phs[],int Hf_hand) ;
 int SynBERG(    HANDSTAT_k    *phs,  int suit) ;


// main or userfunc has a) set all the mm_ptrs etc. b) used prolog to zero globals c) filled sidestat global with trump suit choice.

/* Bergen. From Book Points Schmoints and lots of iNet resources. Long suits, Adjust-3 and many other factors */
int bergen_calc( UE_SIDESTAT_k *p_ss) {    /* Tag Number: 0 */
   int berg_NTpts[2]     = {0}; /* Hand Value in NT: Honors, Length, Body, Syn/Qual etc.*/
   int berg_pts[2] = {0};       /* Hand Value in Suit: NT above + Dfit + Fn + Hf ... etc.*/
   int h, s , adj3_cnt;
   int slen ;
   zero_globals( p_ss->side ) ; 
   HANDSTAT_k *p_hs, *phs[2];
   phs[0] = p_ss->phs[0];
   phs[1] = p_ss->phs[1];
   p_hs   = p_ss->phs[0] ;
   set_dbg_names(0, "bergen_calc");
   
   /*
    * Bergen: hand[0] incl ADJ-3 to get starting points. Then hand[1] ditto.
    *         then Dfit, Hf, Fn_pts and misfit corrections for the pair; then final total
    */
   JGMDPRT( 7 , "++++++++++ berg_calc for side= %d; compass[0]=%c, compass[1]=%c, phs[0]=%p, phs[1]=%p, hcp[0]=%d, hcp[1]=%d\n",
               p_ss->side, compass[0],compass[1],(void *)phs[0], (void *)phs[1], phs[0]->hs_totalpoints, phs[1]->hs_totalpoints ) ;
   for (h = 0 ; h < 2 ; h++) { /* for each hand in the side */
      p_hs = phs[h] ; /* phs array set by prolog to point to hs[north] and hs[south] OR to hs[east] and hs[west] */
      for (s = CLUBS ; s<= SPADES ; s++ ) {
         slen = p_hs->hs_length[s] ;
         if (0 == slen ) {
            fhcp_suit[h][s]  = 0.0 ;
            fhcp_adj_suit[h][s] = 0.0 ;
         }
         else if ( slen <= 2 ) {
         fhcp_suit[h][s]  = p_hs->hs_points[s]  ;
         fhcp_adj_suit[h][s] = shortHon_adj(p_hs, s, BERGEN ) ; /* Deduct in a std way in case we play the hand in NT */
         }
         else { /* no shortness pts; straight HCP */
            fhcp_suit[h][s]  = p_hs->hs_points[s] ;
            fhcp_adj_suit[h][s] = 0.0 ;
         }
         syn_pts_suit[h][s] = SynBERG(p_hs, s) ;  // Quality suit 3 of top5 in 4+ suit = +1; applies to all suits not just trump
         /* simpler to do inline than call Lpts_std  'Pts Schmoints' book does not say if Len or short method for opener. */
         if (p_hs->hs_length[s] > 4 ) {
            lpts_suit[h][s] = (p_hs->hs_length[s] - 4 ) ; // +1 for 5 card suit, +2 for six, +3 for 7 etc.
            JGMDPRT(8,"L_ptsBERG, suit=%d, len=%d, Lpts_suit=%d\n", s, p_hs->hs_length[s], lpts_suit[h][s] );
         }
         fhcp[h]     += fhcp_suit[h][s]    ;
         fhcp_adj[h] += fhcp_adj_suit[h][s];
         syn_pts[h]  += syn_pts_suit[h][s] ;
         lpts[h]     += lpts_suit[h][s]    ;
         JGMDPRT(8,"bergen_calc, Hand=%d, suit=%d, SuitLen=%d, fhcp[h][s]=%g, fhcp-adj[h][s]=%g, Lpts[h][s]=%d, Synpts=%d \n",
                     h, s, p_hs->hs_length[s], fhcp_suit[h][s], fhcp_adj_suit[h][s], lpts_suit[h][s],syn_pts_suit[h][s] );
      } /* end for s = CLUBS to SPADES*/

      if (p_hs->square_hand) { hand_adj[h] = -1 ;}
      else                   { hand_adj[h] =  0 ;}
      JGMDPRT(8,"SqBERG, seat[%c] hand_adj=%d\n", compass[h], hand_adj[h] );
      adj3_cnt = ( p_hs->hs_totalcounts[idxTens]   + p_hs->hs_totalcounts[idxAces]
                  -p_hs->hs_totalcounts[idxQueens] - p_hs->hs_totalcounts[idxJacks] ) ;
      if      (adj3_cnt <= -6 )  { body_pts[h] += -2 ; } // Downgrade
      else if (adj3_cnt <= -3 )  { body_pts[h] += -1 ; } // Downgrade
      else if (adj3_cnt >=  6 )  { body_pts[h] += +2 ; } // Upgrade
      else if (adj3_cnt >=  3 )  { body_pts[h] += +1 ; } // Upgrade
      berg_NTpts[h] = lround(fhcp[h]+fhcp_adj[h]) + syn_pts[h] + lpts[h] + body_pts[h] + hand_adj[h] ; /* starting points for hand h */
      /* JGM::2025-04-12 a hand can NEVER have negative points no matter the deductions */
      if (berg_NTpts[h] < 0 ) berg_NTpts[h] = 0 ; 
      JGMDPRT(8,"Adj3 BERG, seat[%c] adj3_cnt=%d, body_pts=%d, Berg_NT_Tot=%d\n", compass[h], adj3_cnt, body_pts[h], berg_NTpts[h] );
      UEv.nt_pts_seat[h] = berg_NTpts[h] ;
   } /* end for each hand */
   UEv.nt_pts_side = UEv.nt_pts_seat[0] + UEv.nt_pts_seat[1] ;
   JGMDPRT(7,"Berg Both Hands Start/NT pts done. pts[0]=%d, pts[1]=%d, UEv_Side_pts=%d\n", berg_NTpts[0],berg_NTpts[1], UEv.nt_pts_side );

   /* now do the stuff for the side in a suit contract: Hf, Dfit and Fn Mainly.
    * Berg does not do Waste or Mirror
    * But if there is a misfit he throws out any Lpts assigned
    */
      hf_pts[0] = Hf_ptsBERG( phs , 0 ) ; /* Hand 0 is the one getting Hf pts for help in pards long suit. */
      hf_pts[1] = Hf_ptsBERG( phs , 1 ) ;
      berg_pts[0] = berg_NTpts[0] + hf_pts[0];
      berg_pts[1] = berg_NTpts[1] + hf_pts[1];
      JGMDPRT(7,"Berg: Suit: Bpts[0]=%d, Hfpts[0]=%d, Bpts[1]=%d, Hfpts[1]=%d\n",berg_pts[0],hf_pts[0],berg_pts[1],hf_pts[1] );

   /* Bergen Dfit; No dfit with trumps <= 2 ; with 3: 3/2/1 ; with 4: 4/3/1 ; with 5: 5/3/1 4=4 Fit BOTH hands get Dfit */
      TFpts = Do_Df_Fn_pts(p_ss, DfitBERG, Fn_ptsBERG) ; /* Set the globals dfit_pts[],Fn_pts[], and struct TFpts */
     
      JGMDPRT(7,"Berg:Df[0]=%d,Fn[0]=%d, Df[1]=%d,Fn[1]=%d\n",dfit_pts[0],Fn_pts[0],dfit_pts[1],Fn_pts[1] ) ;
      berg_pts[0] += dfit_pts[0] + Fn_pts[0] ;
      berg_pts[1] += dfit_pts[1] + Fn_pts[1] ;
      UEv.hldf_pts_seat[0] = berg_pts[0] ;
      UEv.hldf_pts_seat[1] = berg_pts[1] ;
      UEv.hldf_pts_side = berg_pts[0] + berg_pts[1] ;
      UEv.hldf_suit = p_ss->t_suit ;
      UEv.hldf_fitlen = p_ss->t_fitlen ; 
      JGMDPRT(7,"Berg HLDF Totals for side Before Misfit Check=%d,%d,%d\n",
                                    UEv.hldf_pts_side,UEv.hldf_pts_seat[0],UEv.hldf_pts_seat[1] );
  /* Bergen: Dont count L pts if there is a misfit;
   * Maybe he means don't count Lpts for a L suit facing shortness, keep other Lpts ??
   */
  /* check for misfits in each suit  This may matter if we play in NT even if there is an 8+ fit */
  /* We assume Bergen considers shortness vs 4+ suit a misfit; vs a 3crd suit not a misfit... */
     for (s=CLUBS ; s<=SPADES; s++ ) {
        misfit[s] = misfit_chk(phs, s) ;
        if (  misfit[s].mf_type > 3 ) { misfit_cnt = 1 ; } /* len pts only assigned for 4+ so waste wont matter */
     }
     if (0 < misfit_cnt ) { /* there is at least one misfit Subtract any Length pts previously assigned to the NT pts*/
        UEv.nt_pts_seat[0] -= lpts[0] ; if (UEv.nt_pts_seat[0]  < 0 ) UEv.nt_pts_seat[0]  = 0 ; /* JGM:: 2025-04-12 A hand can NEVER have negative points */
        UEv.nt_pts_seat[1] -= lpts[1] ; if (UEv.nt_pts_seat[1]  < 0 ) UEv.nt_pts_seat[1]  = 0 ;
        UEv.nt_pts_side = UEv.nt_pts_seat[0] + UEv.nt_pts_seat[1] ;
        JGMDPRT(7, "Berg MISFIT calcs Done removed lpts %d, %d\n", lpts[0], lpts[1]);
        /* no further adj for wastage vs shortness,  or for shortness vs len*/
      }
// N:(hcpadj,Lpt,Body,Syn,Dpt,Dfit,Fn,hand_adj); S:(hcpadj,Lpt,Body,Syn,Dpt,Dfit,Fn,hand_adj)
   UEv.misc_count = 0 ;
      /* The factors that apply to both NT and Suit */
      UEv.misc_pts[UEv.misc_count++] = hcp_adj[0];
      UEv.misc_pts[UEv.misc_count++] = lpts[0];       /* +1 for each card over 5, in all suits. */
      UEv.misc_pts[UEv.misc_count++] = body_pts[0];   /* adjust-3 values */
      UEv.misc_pts[UEv.misc_count++] = syn_pts[0];  /* Quality Suit; 3 of top 5 */
      /* Factors that apply to suit contracts only */
      UEv.misc_pts[UEv.misc_count++] = dpts[0];       /* distribution aka shortness pts before a fit is found */
      UEv.misc_pts[UEv.misc_count++] = dfit_pts[0];   /* Dummy support pts typically 3-2-1 with 3, N-3-1 with 4+ */
      UEv.misc_pts[UEv.misc_count++] = Fn_pts[0];     /* Declarer side suit and extra trump length */
      UEv.misc_pts[UEv.misc_count++] = hf_pts[0];
      UEv.misc_pts[UEv.misc_count++] = hand_adj[0];  /* Square hand -- either or both */

      /* ditto hand 1 */
      UEv.misc_pts[UEv.misc_count++] = hcp_adj[1];
      UEv.misc_pts[UEv.misc_count++] = lpts[1];
      UEv.misc_pts[UEv.misc_count++] = body_pts[1];
      UEv.misc_pts[UEv.misc_count++] = syn_pts[1];
      UEv.misc_pts[UEv.misc_count++] = dpts[1];
      UEv.misc_pts[UEv.misc_count++] = dfit_pts[1];
      UEv.misc_pts[UEv.misc_count++] = Fn_pts[1];
      UEv.misc_pts[UEv.misc_count++] = hf_pts[1];
      UEv.misc_pts[UEv.misc_count++] = hand_adj[1];
      // N:(hcpadj,Lpt,Body,Syn);(Dpt,Dfit,Fn,hand_adj); S:(hcpadj,Lpt,Body,Syn);(Dpt,Dfit,Fn,hand_adj)
  /* now put the results into the user result area at p_uservals */
  JGMDPRT(7, "Berg Calcs complete. Saving %d values at userval address %p \n", (6+UEv.misc_count), (void *)p_uservals );
  fsync(2);
  SaveUserVals( UEv , p_uservals ) ;
  return ( 6 + UEv.misc_count ) ; /*number of results calculated */
} /* end bergen_calc */

/* DfitBergen: Count for EACH shortness -- Voids = number of trumps */
int DfitBERG( UE_SIDESTAT_k *p_ss, int du) {
   int  ss_len, t_len, dfit ;
   t_len  = p_ss->t_len[du] ;
   if (t_len <= 2 ) { return 0 ; }
   HANDSTAT_k *p_hs = p_ss->phs[du];
   JGMDPRT(7, "BERG Calc Dfit hs_ptr=%p seat=%c, du=%d, tlen=%d, and ss_len=%d\n",
              (void *)p_hs, compass[du], du, p_ss->t_len[du], p_ss->sorted_slen[du][3] ); /* shortest suit is [3] */
   dfit = 0 ;
   for (int s=CLUBS; s<=SPADES; s++ ) { /* Count for EACH shortness as that seems to be the usual approach */
      if (s == p_ss->t_suit ) continue ;
      ss_len = p_hs->hs_length[s] ;
      if      ( 0 == ss_len ) { dfit += t_len ; }  /* Voids pts = # of trumps. Very agressive */
      else if ( 1 == ss_len ) { dfit += (t_len == 3) ? 2 : 3 ; } /* 4+ trumps, stiff gets 3 pts */
      else if ( 2 == ss_len ) { dfit += 1 ; }
   }
   JGMDPRT(7, "DfitBERG Returning, Dummy=[%d:%c], Tlen[du]=%d, dfit_pts=%d\n",du, compass[du],t_len, dfit ) ;
   return dfit ;
} /* end DfitBERG */
/* Bergen FN: A kind of catchall routine for the points that Opener adds to his hand after a raise; Not for a 4-4 fit */
/* Only needs the phs for the hand getting the Fn pts usually -- not both as would be the case for OPC . */
int Fn_ptsBERG(UE_SIDESTAT_k *p_ss, int dc ) {
   int  dbltons, fn_pts, t5, v0, s1 ;
   int suit ;
   HANDSTAT_k *p_hs = p_ss->phs[dc] ;
   JGMDPRT(7, "BERG Calc FN_pts for dc seat=%d:%c, dc=%d, tlen=%d, and ss_len=%d\n",
               seat[dc],compass[dc], dc, p_ss->t_len[dc], p_ss->sorted_slen[dc][3] ); /* shortest suit len is [3] */
   fn_pts  = 0 ;
   dbltons = 0 ;
   t5=0; v0=0; s1=0;
   if (5 == p_ss->t_len[dc] ) { fn_pts += 1 ; t5 = 1; }
   else if ( 5 < p_ss->t_len[dc] ) { fn_pts += 1 + (p_ss->t_len[dc] - 5)*2 ; t5 = fn_pts ;  } /* 2 pts for every trump >5 +1 for 5th trump */

   /* now add for short suits in Declarer's hand could do this with p_ss and sorted_slen[dc][suit] */
   for (suit = CLUBS; suit<=SPADES; suit++ ) {
      if      ( 0 == p_hs->hs_length[suit] ) { fn_pts += 3 ; v0 = 3 ; } /* 3 pts for a void  -- Internet page says 4*/
      else if ( 1 == p_hs->hs_length[suit] ) { fn_pts += 2 ; s1 = 2 ; } /* 2 pts for a stiff */
      else if ( 2 == p_hs->hs_length[suit] ) { dbltons += 1; } /* Count doubletons. */
   }
   if (dbltons >= 2 ) { fn_pts += 1 ; } /* 1 pt for 2 or 3 dbltons */
   JGMDPRT(7,"Fn_BERG Done. Decl=[%d:%c] Fn_pts=%d from 5+Trump=%d, void_pts=%d, stiff_pts=%d, dbltons_cnt=%d \n",
                           dc, compass[dc], fn_pts, t5, v0, s1, dbltons ) ;
   return fn_pts ;
} /* end Fn_ptsBERG */
int Hf_ptsBERG( HANDSTAT_k *phs[] , int Hf_hand ) { /* Fairly agressive; counts for a stiff K,Q,J vs 6+ suit */
   JGMDPRT(7,"Hf_ptsBERG, for short? hand # %d, seat=%c\n",Hf_hand, compass[Hf_hand]) ;
    /* Bergen Hf Points:  With 1-3 hcp in ALL 8+ fit suits,  add +1 Hf pts; max 2 Hf pts per hand */
    int long_hand , s, Hf_pts  ;
    int lh_len, Hf_len, Hf_hcp ;
    long_hand = ( 1 == Hf_hand ) ? 0 : 1 ; /* the 'other' hand is long_hand*/
    Hf_pts = 0 ;
    for (s=CLUBS; s<=SPADES; s++ ) {
       lh_len = phs[long_hand]->hs_length[s] ;
       Hf_len = phs[Hf_hand]->hs_length[s] ;
       Hf_hcp = phs[Hf_hand]->hs_points[s] ;
       if ( 1 <= Hf_hcp && Hf_hcp <= 3 &&
          ( ( (lh_len + Hf_len) >= 8 ) || ( lh_len >=5 && (lh_len + Hf_len) >= 7 ) ) ) {
          Hf_pts += 1 ; /* Kxxx vs Qxxx (both), or Axxxx vs Jx (+1), or AJxxxx vs Q (+1) etc. */
       }

    } /* end for Hf Points calc */
    JGMDPRT(7,"BERG Hand=%c, gets a total of %d Hf points (Max of 2) added\n", compass[Hf_hand],Hf_pts );
    if (Hf_pts > 2 ) { Hf_pts = 2 ; }

    return Hf_pts ;
} /* end Hf_pts_berg */
int SynBERG(HANDSTAT_k *phs, int suit ) { /* Bergen 'Quality Suit' pts */
    JGMDPRT(8,"SynBERG, Suit=%c Len=%d, Top5=%d\n","CDHS"[suit],phs->hs_length[suit],phs->hs_counts[idxTop5][suit]) ;
      if ( (phs->hs_length[suit] >= 4 ) && (phs->hs_counts[idxTop5][suit] >= 3 ) ) { //  Bergen Quality +1 for 3/top5 in 4+ suit;
         return(1) ;
      }
      return(0) ;
}
