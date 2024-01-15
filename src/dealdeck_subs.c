/* File dealdeck_subs.c
 * Code related to creating new decks, shuffling decks, converting PBN to Deal52 
 * Date        Version  Author           Comment
 * 2023/11/02     1.0   JGM   	Creating static libraries for the various pieces of dealer code so I can build scaffolding
 */
#ifndef _GNU_SOURCE
  #define _GNU_SOURCE
#endif
#include "../include/dbgprt_macros.h"
#include "../include/std_hdrs.h"
#include "../include/dealdefs.h"
#include "../include/dealtypes.h"
#include "../include/dealdebug_subs.h"   /* for hexdeal_show which now needs a size parm */

#include "../include/dealutil_subs.h"  /* for gen_rand_slot */
#include "../include/dealdeck_subs.h"   /* helps keep .h and .c in sync */
/* count number of real cards in pack */			
int pack_size(DEAL52_k d) {
	int sz = 0 ; 
	int d_sz = sizeof(DEAL52_k) / sizeof(CARD52_k) ; 
	for (int i = 0 ; i < d_sz; i++ ) {
		if (IS_CARD( d[i] ) ) sz++ ;
	}
	return sz ;
}

/* put all NO_CARD entries in high positions, and real cards starting at slot zero */
int compress_pack(char *d, int dsz ) {
	int i ;
	char *pd = d ; 
	int psz = 0 ;
	char td[52] ; /* should really malloc this of size dsz for generatlity */
	char *ptd   = &td[0] ;
	
	for (i=0; i< dsz ; i++ ) {
		if( IS_CARD( *pd ) ) { *ptd++ = *pd ; }  // 0x00 <= *pd <= 0x3C 
		pd++ ;
	}
	psz = (ptd - td) ; 
	assert(psz <= dsz) ; 
	memset(d, NO_CARD, dsz) ; 
	memcpy(d, td, psz) ;   /* the real cards now in the low positions; the top positions filled with NO_CARD */
	return psz ;
}


/* Fisher-Yates perfect shuffle Algorithm Per Knuth */
void Shuffle ( DEAL52_k deck, int sz ) {  /* used for curdeal or small_pack */
    CARD52_k  card ;
   int i, j, top;
   JGMDPRT(9,"Shuffle deck size=%d\n",sz ) ;
   top = sz - 1; 
   for (i=top; i > 0 ; i-- ) {  // 1 <= i <= n-1
		j = gen_rand_slot( i+1 ) ; // ( n ) is NOT Fisher-Yates & leads to shuffle that does NOT return all permutations;
		// j ← random integer such that 0 ≤ j ≤ i   [Note the equal sign.]
		/* exg via a  temp var turns out  to be faster than doing 3 exclusive OR's */
		card = deck[i];
		deck[i] = deck[j];
		deck[j] = card ; 
	//	fprintf(stdout, "Knuth Swapped from slot[%d] to slot[%d]\n", i,j);
	}
        JGMDPRT(9, "*---- Post Shuffle Deal of size=%d  ----- *\n", sz);
        DBGDO(9, hexdeal_show(deck, sz) );
} /* end Shuffle */

int idx2Player(int dl_idx) {
	int p ; 
	p = (dl_idx < 13 ) ? COMPASS_NORTH :
	    (dl_idx < 26 ) ? COMPASS_EAST  :
	    (dl_idx < 39 ) ? COMPASS_SOUTH : COMPASS_WEST ; 
    return p ; 
} 

void newdeck  ( DEAL52_k d, char dir) { /* fill a deck with cards from C2 to sA (dir=A, or SA to C2 dir=D */
  int place;
  int rank;
  int suit;
  place = 0;
  switch (dir) {
	  case 'A' :
	  case 'a' :
		for (suit = CLUBS; suit <= SPADES; suit++) {  /* newdeck is in order from Club deuce up to spade Ace. */
			for (rank = TWO; rank <=ACE; rank++)     {
				d[place++] = MAKECARD (suit, rank);
			}
		}
		break ;
	  case 'D' :
	  case 'd' :
		for (suit = SPADES; suit >= CLUBS; suit--) { /* newpack in order from AS down to Club deuce. why this order?*/
			for (rank = ACE; rank >=TWO; rank--)     {
				d[place++] = MAKECARD (suit, rank);
			}
		}
		break ;
	} /* end switch (dir) */ 
  return ;	  
} /* end newdeck */

void newpack  ( DEAL52_k d) { /* Fill a deck with cards from SA downto C2 */
  int place;
  int rank;
  int suit;
  place = 0;
  for (suit = SPADES; suit >= CLUBS; suit--) { /* newpack in order from AS down to Club deuce. why this order?*/
    for (rank = ACE; rank >=TWO; rank--)     {
      d[place++] = MAKECARD (suit, rank);
    }
   }
   return ;
}  /* end newpack */

/* rank of a card letter using DEAL52 coding. DDS coding would be this +2 so deuce has rank of 2 not zero */
/* The code that does this in the current dealerv2 is (misnamed) isCard( char c ) in dealinit_subs.c */
int card_Rank( char card ) {
   int rank ; 
   static char Ranks[14]="23456789TJQKA";
   char *chptr ; 
	rank = ( (chptr = strchr(Ranks,card))  ) ? (chptr - &Ranks[0] ) : -1  ;
	return rank ; 
}



