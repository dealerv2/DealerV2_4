/* File UE_util_protos.h  -- Function prototypes relating to supporting routines
 *  Version  Date       Author  Comment
 *    0.5    2024-08-17  JGM    First
 */
#ifndef UE_UTIL_PROTOS_H
#define UE_UTIL_PROTOS_H
#ifndef USEREVAL_TYPES_H
   #include "../include/UserEval_types.h"        /* Will also include dealtypes.h */
#endif
#include <math.h>                               /* for float_t */
   /* Mostly related to hand evaluation some not used but defined anyway */
extern void zero_globals( int side ) ; 
extern void Analyze_side( UE_SIDESTAT_k *p_uess, int side ) ;
extern int isBalanced(HANDSTAT_k *phs ) ;
extern struct misfit_st misfit_chk(HANDSTAT_k *phs[], int s ) ;
extern int NT_slam_ok (HANDSTAT_k *phs[2] ) ;
extern int Pav_body_val( HANDSTAT_k  *p_hs )  ;
extern float_t Pav_playing_tricks (HANDSTAT_k *phs ) ; 
extern int Pav_round(float val, int body ) ;
extern float_t QuickTricks(HANDSTAT_k *phs ) ;
extern float_t QuickTricks_suit(HANDSTAT_k *phs , int s ) ;
extern int Dotnum2Int( int dotnum , int body );
extern FIT_PTS_k Do_Df_Fn_pts( UE_SIDESTAT_k *p_ss,
                        int (*calc_dfval)( UE_SIDESTAT_k *p_ss, int h),
                        int (*calc_fnval)( UE_SIDESTAT_k *p_ss, int h) ) ;
         

   /* Common Bookkeeping functions */
extern void SaveUserVals(struct UserEvals_st UEv , USER_VALUES_k *p_ures ) ;
extern void zero_globals ( int side ) ;
extern void prolog ( int side )  ;

   /* mostly sorting functions */
extern int arr_min(int arr[], size_t nelem ) ;
extern int asc_cmpxy(  const void *x, const void *y) ;
extern int desc_cmpxy( const void *x, const void *y) ;
extern int didxsort4( int v[4], int x[4] ) ;
extern int dmerge( char *a, char *b, char *c, int aN, int bN ) ;
extern int dsort4( char a[4] ) ;
extern int dsort13 (char a[13] ) ;
extern int dsort_i4( int a[4] ) ;
extern void sortDeal(DEAL52_k dl )  ;
extern void swap(int* xp, int* yp) ;


  /* Mostly Debugging Functions some from metric_util_subs, some from Serverdebug_subs*/
extern int   make_test_evals( struct detailed_res_st *p_ures) ;
extern char *Hand52_to_pbnbuff( int p, char *dl, char *buff ) ;
extern void dump_curdeal( void ) ;   /* needs deal to be analyzed */
extern void show_hands_pbn( int mask, DEAL52_k d ) ;
extern void show_set40_res( struct EvalAll_res_st ue40_res ) ;
extern void show_sorted_fits( UE_SIDESTAT_k *p_ss ) ;
extern void show_sorted_slen( UE_SIDESTAT_k *p_ss ) ;
extern void show_user_res( char *caller, USER_VALUES_k *p_results, int first, int last ) ;
void show_UEsidestat( UE_SIDESTAT_k *p_ss ) ;
extern void sr_deal_dump( DEAL52_k real_dl ) ;  /* deal in two line suit-rank format */
extern void show2D_arr( int *arr, int NR, int NC ) ;
extern void show1D_arr( int *arr, int NC ) ;
void set_dbg_names(int mnum, char *funcname ) ;

#endif  /* end file guard */














