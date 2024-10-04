/* File UE_calc_protos.h  -- Function prototypes relating to calculations
 *  Version  Date       Author   Comment
 *    0.5    2024-08-13   JGM    First
 */
#ifndef UE_CALC_PROTOS_H
#define UE_CALC_PROTOS_H
#ifndef USEREVAL_TYPES_H
   #include "../include/UserEval_types.h"        /* Will also include dealtypes.h */
#endif

extern int bergen_calc(    UE_SIDESTAT_k *p_UEss )  ;   // incl: adj-3; qual-suit; len; fit ; short Hon adj
extern int bissell_calc(   UE_SIDESTAT_k *p_UEss )  ;   // Bissell per Bridge Encyclopedia and internet. not so much PAV.
extern int dkp_calc(       UE_SIDESTAT_k *p_UEss )  ;   // LJP per DK; Sht_H adj; no Len; no D ; no dfit ; basic;
extern int goren_calc(     UE_SIDESTAT_k *p_UEss )  ;   // Per the PAV web page; shortness; dfit ; short H ded;
extern int kaplan_calc(    UE_SIDESTAT_k *p_UEss )  ;   // per the book, Contract Bridge Complete
extern int karpin_calc(    UE_SIDESTAT_k *p_UEss )  ;   // per PAV web page; HCP + Len + Dfit + short H ded
extern int karpb_calc(     UE_SIDESTAT_k *p_UEss )  ;   // Karpin calc with BumWrap Points
extern int knr_calc(       UE_SIDESTAT_k *p_UEss )  ;   // per JGM fixes, incl Dfit as in the text
extern int lar_calc(       UE_SIDESTAT_k *p_UEss )  ;   // from Larsson's book; hcp + len + body + syn + dfit
extern int lar_b_calc(     UE_SIDESTAT_k *p_UEss )  ;   // Larsson with Bum_Wrap points.
extern int pav_calc(       UE_SIDESTAT_k *p_UEss )  ;   // per PAV website; hcp + shortness + some dfit + short H adj
extern int roth_calc(      UE_SIDESTAT_k *p_UEss )  ;   // per 1968 book, Modern Bridge Bidding Complete Roth & Rubens
extern int sheinw_calc(    UE_SIDESTAT_k *p_UEss )  ;   // per the book, 5 Weeks to Winning Bridge 1959,1964
extern int zaradv_calc(    UE_SIDESTAT_k *p_UEss )  ;   // Zar Basic + HF pts, Fn pts and Dfit_pts (Trump Ruff Power). No misfit pts.
extern int zarbas_calc(    UE_SIDESTAT_k *p_UEss )  ;   // per the 2005 download PDF from Zar Petkov HCP+CTLS+(a+b)+(a-d)+SynZar+short H

extern int alt_HCP_calc(   UE_SIDESTAT_k *p_UEss, int tag )  ;   // 8 different ways of counting HCP 

extern int set40_calc(     UE_SIDESTAT_k *p_UEss )  ;   // tag 40 fills slots 0 .. 89 with 6 values from each of 15 metrics and HLDF.suit & fitlen in slots 126,127
extern int set41_calc(     UE_SIDESTAT_k *p_UEss )  ;   // tag 41 fills slots 0 .. 47 with 6 values from each of 8 alt HCP metrics.

extern int mixed_Karpin_calc(  UE_SIDESTAT_k *p_UEss ); // tag 50 returns two sets of HLDF numbers, karp_b  in 0..5 and karpin  in 6 .. 11
extern int mixed_Larsson_calc( UE_SIDESTAT_k *p_UEss ); // tag 51 returns two sets of HLDF numbers, lar_b in 0..5 and larsson in 6 .. 11 
                                         
extern int set88_calc(     UE_SIDESTAT_k *p_UEss )  ;   // tag 88 fills slots 0 .. set88_sz*2 with the side_total value from the various metrics NT and HLDF
extern int test_calc (     UE_SIDESTAT_k *p_UEss ) ;    // tag 99 returns bogus numbers in various slots. Used to test exg between Dealer and UserServer
extern int unkn_err(       UE_SIDESTAT_k *p_UEss )  ;   // called if erroneous query tag; returns -1

  /* protos from short_honors_subs.c */
#include <math.h>  /* for type float_t */
extern float_t lookup_adj_stiff_H( enum metric_ek m, int w1) ;
extern float_t lookup_adj_HH( enum metric_ek m, int w2) ;
extern float_t excp_adj_fhcp( HANDSTAT_k *phs, int suit, int tag ) ;
extern float_t shortHon_adj ( HANDSTAT_k *phs, int suit, int tag ) ;
extern float_t calc_alt_hcp ( HANDSTAT_k *phs, int suit, int mtype);
extern float_t excp_adj_fhcp( HANDSTAT_k *phs, int suit, int tag ) ;
extern int     ss_index(      int weight ) ;



#endif /* end file guard */
