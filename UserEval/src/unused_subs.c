/* File unused_subs.c */
/* Date        Version  Author   Description
 * 2024/08/17  1.0      JGM      Possibly useful code from Factors.c, metrics_calcs.c, that is not currently used
 *
 */
#define GNU_SOURCE
#include "../include/std_hdrs_all.h"
#include "../include/UserEval_types.h"
#include "../include/UserEval_externs.h"
#include "../include/dbgprt_macros.h"
#include "../include/mmap_template.h"

// main or userfunc has a) set all the mm_ptrs etc. b) used prolog to zero globals c) filled sidestat global with trump suit choice.
/* From Wikipedia: Combo Count for Length AND shortness in a hand; seem to be two different methods. Not sure why. */
int ComboLD_pts(HANDSTAT_k *phs , int type ) {
   JGMDPRT(7, "ComboLD_pts, Type=%d \n",type) ;
   int s ;
   int ss_len, ls_len, ls2_len, s_len ; /* suit lens needed for calcs. suit nums needed for Debug */
   ss_len = 13 ;
   ls_len = 0  ;
   ls2_len = 0 ;
   int ld_pts = 0 ; /* points for length AND shortness */

   /* Combo Pts Type=1: Void=2, Stiff=1, L = (length - 4) Gives: (4333=0, 4432=0, 5332 = 1 etc. for each suit. */
   if (1 == type ) {
      for (s = CLUBS ; s <=SPADES ;  s++ ) {
         s_len = phs->hs_length[s] ;
         if      ( 0 == s_len ) { ld_pts += 2 ; }
         else if ( 1 == s_len ) { ld_pts += 1 ; }
         else if ( 4 <  s_len ) { ld_pts += ( s_len - 4 ) ; }
         //printf( "Type1: suit[%c] len = %d, cumul ld pts = %d \n","CDHS"[s], length[s], ld_pts ) ;
      }  /* end for each suit*/
      JGMDPRT(7,"ComboLD_pts, Type=%d, Combo_ld_pts=%d \n",type,ld_pts) ;
      return (ld_pts ) ;
   } /* end if type 1 */

   /* Type=2 : longest + 2nd longest - shortest - 5 :: 4333 = -1 4432 = 1 ; 5332 = 1 etc. */
   for (s = CLUBS ; s <=SPADES ;  s++ ) {  /* find shortest and longest suits */
      s_len = phs->hs_length[s] ;
      if (s_len <  ss_len ) {ss_len = s_len ;  }
      if (s_len >= ls_len ) {ls2_len = ls_len;  ls_len = s_len ;  } /* demote the previous longest to 2nd longest */
      else if (s_len > ls2_len ) { ls2_len = s_len ; }
      // printf( "Type2: After suit[%c] of len=%d, ss = %d, ls = %d, ls2 = %d\n",
      //    "CDHS"[s], s_len, ss, ls, ls2 ) ;
   } /* end for each suit type2 */
   ld_pts = ls_len + ls2_len - ss_len - 5 ;
   // printf("Result COMBO LD Pts Type 2 = %d\n",ld_pts);
   JGMDPRT(7,"ComboLD_pts, Type=%d, ss=%d, ls2=%d, ls=%d ComboPts=%d\n",type,ss_len, ls2_len, ls_len, ld_pts) ;
   return (ld_pts) ;
} /* end combo_LD_pts() */

int DfitMOPC( UE_SIDESTAT_k *p_ss, int du ) { 
   int ss_len, t_len, dfit ;
   int idx_short ;
   int DF_MOPC[2][4] = { {3,2,1,0},{4,3,2,0} } ; /* Dfit pts with 3/4 trump for V/S/D/Longer -- Only if 9+ fit */
   int v0, s1, db ; /* Debug vars */
   t_len = p_ss->t_len[du];
   ss_len = p_ss->sorted_slen[du][3] ; // [3] is the shortest suit in the hand
   JGMDPRT(7, "MOPC Calc Dfit for du seat=%d:%c, du=%d, tlen=%d, and ss_len=%d\n",
               p_ss->dummy_h,compass[du], du, p_ss->t_len[du], p_ss->sorted_slen[du][3] );
   v0 = 0 ; s1 = 0 ; db = 0 ;
   /*
    * Modfied OPC -- Only for "9 fit"  Dfit = #Trumps - short_suit Length; also no Dfit for 2 trumps (opc would count V=2,S=1)
    */
   if ( (p_ss->t_len[du] <= 2 ) || (p_ss->t_fitlen < 9)  ) { return 0 ; }
   // if ( p_ss->t_fitlen < 9 ) || p_ss->t_len[du] < 4 ) { return 0 ;} /* this is the other choice for larsson */
   /* We have a 9 card fit. count for 1 shortest suit only */
   if      ( 0 == ss_len ) { idx_short = 0 ; v0++ ;}
   else if ( 1 == ss_len ) { idx_short = 1 ; s1++ ;}
   else if ( 2 == ss_len ) { idx_short = 2 ; db++ ;}
   else                    { idx_short = 3 ; }
   if (3 == t_len  ) { dfit = DF_MOPC[0][idx_short] ; }  /* 3 card support; */
   else              { dfit = DF_MOPC[1][idx_short] ; }  /* 4+ support; */
   JGMDPRT(7, "DfitMOPC Returning, Dummy=[%d], dfit=%d, from p_ss->ss_len=%d,t_len[du]=%d: DBG:%d,%d,%d\n",
                  du, dfit, ss_len, t_len,v0,s1,db ) ;
   return dfit ;
} /* end DfitOPC */
/* A common way of counting support points; Any 8+ fit; 5/3/1 with 4+ suppt, 3/2/1 with 3+ suppt -- ONE short suit only */
int DfitSTD( UE_SIDESTAT_k *p_ss, int du )  { 
   int ss_len, t_len, dfit ;
   int idx_short ;
   int DF_STD[2][4] = { {3,2,1,0},{5,3,1,0} } ; /* Dfit pts with 3/4 trump for V/S/D/Longer any 8+ fit  */
   int v0, s1, db ; /* Debug vars */
   t_len = p_ss->t_len[du];  
   ss_len = p_ss->sorted_slen[du][3] ;  // [3] is the shortest suit in the hand
   JGMDPRT(7, "STD Calc Dfit for du seat=%d:%c, du=%d, tlen=%d, and ss_len=%d\n",
               p_ss->dummy_h,compass[du], du, p_ss->t_len[du], p_ss->sorted_slen[du][3]  );
   v0 = 0 ; s1 = 0 ; db = 0 ;
   if ( (p_ss->t_len[du] <= 2 ) ) { return 0 ; } /* a 6-2 or 7-2 fit does not get any suppt pts */
   if      ( 0 == ss_len ) { idx_short = 0 ; v0++ ;}
   else if ( 1 == ss_len ) { idx_short = 1 ; s1++ ;}
   else if ( 2 == ss_len ) { idx_short = 2 ; db++ ;}
   else                    { idx_short = 3 ; }
   if (3 == t_len  ) { dfit = DF_STD[0][idx_short] ; }  /* 3 card support; */
   else              { dfit = DF_STD[1][idx_short] ; }  /* 4+ support; */
   JGMDPRT(7, "DfitSTD Returning, Dummy=[%d], dfit=%d, from p_ss->ss_len=%d,t_len[du]=%d: DBG:%d,%d,%d\n",
                  du, dfit, ss_len, t_len,v0,s1,db ) ;
   return dfit ;
} /* end DfitSTD */

/* Defaults when a placeholder needed for DoTrumpFit */
int Fn_ptsNONE( UE_SIDESTAT_k *p_ss, int dc ) { return 0 ; }
int DfitNONE(   UE_SIDESTAT_k *p_ss, int du ) { return 0 ; }

inline int LptsSTD(HANDSTAT_k *phs, int suit ) { // such a simple function make it inline
   if (phs->hs_length[suit] > 4 ) {
            return (phs->hs_length[suit] - 4 ) ; // +1 for 5 card suit, +2 for six, +3 for 7 etc.
   }
   return 0 ;
}


