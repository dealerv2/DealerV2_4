/* File deal_knr.h -- revised CCCC.h for use in libdealerV2.a 
 * 2023/11/07   1.0		JGM 		see c4.h for original D. Suits version 
 */
#ifndef DEAL_KNR_H
#define DEAL_KNR_H

#include "../include/dealdefs.h"    /* To define the HAS_CARD macro */

int suit_quality(  int , int  ) ;   /* internal routine */
int eval_cccc( int ) ;				/* internal routine */

int quality (int, int);     /* KnR Suit Quality. */
int cccc (int);             /* KnR calculation as fixed by JGM */

#endif /* DEAL_KNR_H */

