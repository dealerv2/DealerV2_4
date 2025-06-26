/* File deal_rounding_subs.c
 * routines to round floats and dotnums to ints in a bridge hand context;
 * Using Richard Pavlicek's idea that when the fractional part is 'close to' one-half (0.45 <= x <= 0.55)
 * round a hand with 'good body' up. Otherwise round up or down normally.
 * 'Good Body' is when the value: (3*Tens + 2* Nines + Eights) is >= 12;
 * Date        Version  Author   Comment
 * 2024/08/11     1.0    JGM   
  */
#ifndef _GNU_SOURCE
  #define _GNU_SOURCE
#endif

#include "../include/dbgprt_macros.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>           	/* exit(), malloc(), free(), rand48,   */
#include <string.h>    			/* for memcpy */
#include <sys/random.h>       	/* for getrandom() */
#include <sys/time.h>  			/* for gettimeofday, struct timeval */

#include "../include/dealexterns.h"
#include "../include/libdealerV2.h"

#include "../include/deal_rounding_subs.h"  /* having this helps keep the protos and code in sync */
/* Pavlicek 'Body' count. >= 12 typically has 'Body' */
int Pav_body_val( HANDSTAT_k  *p_hs ) {
   int s;
   int body = 0 ;
   for (s=0; s<4 ; s++ ) {
      if (p_hs->Has_card[s][EIGHT] ) { body++ ;   }
      if (p_hs->Has_card[s][NINE]  ) { body += 2; }
      if (p_hs->Has_card[s][TEN]   ) { body += 3; }
   }
   return( body ) ; /* if >= 12 Good Body. Round Up */
} /* end Pav_body_val */

int Dotnum2Int( int dotnum , int body ) { /* dotnums are all values such as xx.d or xx.dd times 100 so at most two digit fractions */
   int q , r ;
   int scale = 100 ;    /* All dotnums in dealer are scaled by 100. e.g. Ace=4.25 becomes Ace=450; ditto KnR/CCCC pts, LTC etc. */
   q = dotnum / scale ;
   r = dotnum % scale ;
   if ( r > 55 || (45 <= r && r <= 55 && body  >= 12 ) ) { q++ ; } /* round up */
   /* NOTREACHED */
   return q ;
} /* end Dotnum2Int	*/

int Pav_round(float val, int body ) {
   int p, q , r ; 
   q = val ;
   r = (val - q ) * 100 ; /* convert remainder to int between 0 .. 100 */
   p = q ; 
   if ( r > 55 || (45 <= r && r <= 55 && body  >= 12 ) )  { p = q+1; } /* round up */
   return p;
}

int deal_round( float val , HANDSTAT_k *p_hs ) {
   int body ;
   int rval ;
   body = Pav_body_val( p_hs ) ;
   rval = Pav_round( val, body ) ;
   return rval ;
}
