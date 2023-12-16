/* File include/dealutil_subs.h */
/* handcrafted sorts and random number routines */
#ifndef DEALUTIL_SUBS_H
#define DEALUTIL_SUBS_H
#include "../include/dealtypes.h"
int sort3(  char a[3] );								// sort array of  3 char in Asc order; not used in Dealer
int sort4(  char a[4]); 								// sort array of  4 char in Asc order
int sort13( char a[13] ) ; 								// sort array of 13 char in Asc order
int aidxsort4( int values[4], int tags[4] ); 			// sort array of 4 int tags so that values get accessed in Asc Order
int merge(  char *a, char *b, char *c, int aN, int bN );// merge two arrays sorted in Desc order into a single array. No Size chks.

int dsort13 (char a[13] );								// sort array of 13 char in Desc order
int dsort4( char a[4] ) ;								// sort array of  4 char in Desc order
int didxsort4( int values[4], int tags[4] );			// sort array of 4 int tags so that values get accessed in Desc Order
int dmerge( char *a, char *b, char *c, int aN, int bN );// merge two arrays sorted in Desc order into a single array. No Size chks.

void sortDeal(DEAL52_k dl ) ;  /* Descending order(SA -> C2) for each player uses dsort13 */

long int init_rand48( long int seed );
int gen_rand_slot ( int topval );

void dealerr( char *msg ) ;

/* These next routines are no longer used; replaced by above code */
void dealsort_Q( char *dl_ptr );			  		// sort each hand in the deal in descending order using qsort
void handsort_Q( char *hnd_ptr);			  		// sort a hand in descending order using qsort
int dcmp_card_Q(  const void *x, const void *y ) ; 	// Compare func used to sort hands in descending order

/* This next one obsolete; for a 13 element array dsort13 is faster; for a general array use handsort_Q */
void sortHand(char *hptr, int size) ; /* General purpose func to sort char array of any size into Desc order. No longer used */

#endif


