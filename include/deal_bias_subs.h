/* File deal_bias_subs.h -- API to produce a biased deal */
/* 2023-12-10  -- First version */

#ifndef DEAL_BIAS_SUBS_H
#define DEAL_BIAS_SUBS_H
#include "../include/dealtypes.h"
 extern int            bias_deal_wanted;   /* global flag set by yyparse, ref'd by shuffle and deal code */
 extern int            bias_suits[4][4];   /* set by yyparse. The number of cards to predeal a given hand in given suit.*/
 extern int 		     bias_tot_len ; 		 /* the total number of all bias cards in all bias hand/suits */
 extern int            bias_hand_len[4];   /* number predealt bias cards. Zero means none, or specifies a void */
/* after initial setup the following should never change */
 extern int            bias_suit_tot[4] ;			/* the total bias requirements in this suit <= 13 */
 extern int            bias_hand_cnt    ;					/* number of hands with bias suits in them */
 extern int            bias_hand_vp[4]  ; /* vacant places to be filled after bias done. = 13 - bias_hand_len[] */
 extern int            bias_hand_TF[4]  ; /* whether the hand has any bias suit requirements */

void init_bias_deal ()  ;           	/* done once per run  */
void re_init_bias_vars() ;					/* done once per deal */
int make_bias_deal(DEAL52_k curdeal);	/* the main function  */
#endif 




