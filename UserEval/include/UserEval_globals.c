/* File UserEval_globals.c -- Global Vars -- allocate storage in this one.
 *  Version  Date          Comment
 *    0.5    2022-12-02    First
 *    0.6    2022-12-15    Removed Globals that only the Server main uses. This file only for the calulation routine globals.
 *    0.7    2023-04-02    Added include for ss_values array.
 *    0.8    2023-05-24    Added Suit_Contract struct
 *    1.0    2024-08-01    Included ALL_UE code
 */
#include "../include/std_hdrs_all.h"
#include "../include/UserEval_types.h"
#ifndef MMAP_TEMPLATE_H
   #include "../include/mmap_template.h"        /* Will also include dealtypes.h */
#endif

int gen_num  ;
int prod_num ;
int seat[2] ;                /* Server is called with a side; these are the two seats(0..3) related to that side. */
char compass[2];             /* seat names for debug statements */

// Global vars used in most of the metric routines
/* prolog stuff; uses vars setup by the Server Infrastructure */
	struct gbl_struct_st gbl ;	/* tag, side, compass, suit, idx from the query pkt */
   struct gbl_struct_st *p_gbl = &gbl ;
    
	HANDSTAT_k        *phs[2] ;         /* pointers to two related HandStat structs; N/S or E/W */
	USER_VALUES_k     *p_uservals  ;

   UE_SIDESTAT_k      UEsidestat[2] ; /* suit lens, fitlens, bestfit suit, declarer in best fit, etc. */
   UE_SIDESTAT_k     *p_UEss  ;       /* points to current UEsidestat in use for metric calcs */

/* results stuff */

 struct misfit_st    misfit[4]; /* could have 4 misfits e.g. 6=5=1=1 vs 1=1=5=6 :(!  */
 struct UserEvals_st UEv   ; /* repo for everything of interest calculated by the various xxxxx_calc functions */
 struct FitPoints_st TFpts = { .df_val={0,0}, .fn_val={0,0} }; /* Dummy Dfit_pts, and Declarer FNpts when a fit is found */


/* Temp vars used by most routines.*/

 enum suit_ek suit ;     // CLUBS, DIAMONDS, HEARTS, SPADES
 enum metric_ek metric ;  // BERG, BISS, DKP, GOREN, KAPL, KARP, KARPB, KnR, LAR, LAR_B, PAV, SHEIN, ZAR_B, ZAR_A, ROTH=14                       

 int h_dummy , h_decl ;
 int misfit_cnt ;
 int side_nt_pts, seat_nt_pts[2], side_hldf_pts, seat_hdlf_pts[2] ; /* these are the most likely results wanted */
 int hcp[2], hcp_adj[2], dpts[2], lpts[2], dfit_pts[2], syn_pts[2], hf_pts[2], Fn_pts[2], hand_adj[2], body_pts[2];
 int dpts_suit[2][4], lpts_suit[2][4], syn_pts_suit[2][4], hf_pts_suit[2][4],freak[2][4] ;
 
 /* Some calcs will be done in float; cast to int or int*100 at the end*/
 float_t fhcp[2], fhcp_adj[2], fhcp_suit[2][4], fhcp_adj_suit[2][4], suit_qual[2][4];

 // These globals no longer needed. Use global UEfitstat[h] for these 
 //int sorted_slen[2][4], sorted_slen_idx[2][4] ;   /* Suit lengths and 'names' sorted longest to shortest */
 //int sorted_fitlen[4] , sorted_fitlen_idx[4] ;
 //int gbl_fitlen, gbl_fitsuit;   

/* Never Used? */
 struct Metric_Def_st {
	 enum metric_ek metric_id  ;
	 char metric_descr[32];
	 int (*ufunc)(int tag) ;  /* *ufucn(int tag) and ufunc(int tag) both give an error saying that ufunc is a function. */
} ;
struct Metric_Def_st MetricObj[32];

char dbg_func_name[64] = " ";
int  curr_metric_num = 16 ; 
char curr_metric_name[16] = " ";
char metric_names[][16] = { "Bergen","Bissel","Kleinman","Goren","Kaplan","Karpin","Karpin_B","KnR_Fit",
                            "Larsson","Larsson_B","Pavlicek","Sheinwold","Zar_Basic","Zar_Full","Roth","Err_Unused" };

/*
 * The Query tags in alpha? order: The adj_hcp arrays use these values to lookup adjustments. ~ means not coded yet.
 *    0        1       2     3      4      5        6      7    8    9     10     11       12      13     14     15       
   BERGEN=0, BISSEL,  DKP, GOREN, KAPLAN, KARPIN, KARP_B, KnR, LAR, LAR_B, PAV, SHEINW,  ZARBAS, ZARADV, ROTH, metricEND, ,
// possibly add metrics in the 50 - 79 range to implement different hand factors like quicktricks, or quicklosers, or shortest suit etc.
        Set40=40, MixKARPIN=50, MixLARSSON=51,            SET=88, SYNTST=99, Quit=-1} ;
*/


/* end file globals for User Calc Routines */

