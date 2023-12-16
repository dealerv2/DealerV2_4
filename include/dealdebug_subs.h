#ifndef DEALDBG_SUBS_H
#define DEALDBG_SUBS_H
#include "../include/dealtypes.h"
#include "../include/dealdeck_subs.h"
extern int  jgmDebug ; 
extern void show_Deal52(DEAL52_k d) ; 
extern int  gotKard (DEAL52_k  d, int player, CARD52_k  thiscard);
extern void dump_curdeal( DEAL52_k  dl);
extern void hexdeal_show( DEAL52_k  dlx, int sz );
extern void sr_deal_show( DEAL52_k  dlx );
extern void sr_hand_show(int h,  DEAL52_k  dlx) ;
extern void show_hands_pbn( int mask,  DEAL52_k  d ) ;
extern void showtreenode(int tlev, struct tree *tr);
extern void showvartree(struct tree *t) ;
extern void showvarlist(struct var *v ) ;
extern void showdecisiontree(struct tree *t);
extern void showactionexpr( struct action *acp ) ;
extern void showactionlist(struct action *a );
extern void showdistrbits(int ***distrbitmaps[14] ) ;
extern void showAltCounts( void ) ;
extern void show_hands_PBN( int mask, DEAL52_k  d ) ;
extern void show_opcVals(struct sidestat_st *resp ) ; 
extern void show_opcRes(struct opc_Vals_st *resp ) ;

#endif

