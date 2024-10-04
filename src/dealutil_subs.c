/* File dealutil_subs.c
 * several hand optimized sort routines for sorting hands, suits, and values 
 * Date        Version  Author   Comment
 * 2023/10/29     1.0   JGM   Created as part of bias_deal effort
 * 2023/11/04     1.1   JGM   Mods for turning into a library module
 * 2024/05/15     1.2   JGM    Fixed bug in aidxsort4 and didxsort4 where idx values not properly swapped.
 */
/* Eventually these should replace the other home grown insertion sort routines in files:
 * dealaction_subs.c, dealcard_subs.c, Dealer_DDS_IF.c, dealeval_subs.c and Utils/libbridge.c
 * as well as perhaps some merge routines although that is not really necessary
 */
#ifndef _GNU_SOURCE
  #define _GNU_SOURCE
#endif

#include "dbgprt_macros.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>           	/* exit(), malloc(), free(), rand48,   */
#include <string.h>    			/* for memcpy */
#include <sys/random.h>       	/* for getrandom() */
#include <sys/time.h>  			/* for gettimeofday, struct timeval */

#include "../include/dealexterns.h"
#include "../include/libdealerV2.h"

#include "../include/dealutil_subs.h"  /* having this helps keep the protos and code in sync */

void dealerr( char *msg ) {
	fprintf(stderr, "ERR**: %s\n",msg) ; 
}

int unmask(int m) {		/* m is a bit mask, one bit set - return the int corresponding to that bit; in effect lg(m) */
	int v = 0 ; 
	while (m > 1 ) {
		v++ ; 
		m = m >> 1 ; 
	}
  return v ; 
}

/* ---- Hand Crafted Sort Routines --------*/
/* These routines take advantage of the fact that in bridge we only ever have to sort arrays of 13 or 4
 * using optimized swapping code, and known types of elements, these routines are 2.5 to 3.5 times faster
 * than the qsort in glibc.
 * Efficiency being the prime concern these functions do NO error checking; if you call them with incorrect arguments
 * you may smash your stack, abort your program, or allow a buffer overflow exploit
 */
/* strictly speaking these should all be unsigned char for DealerV2; but we rely on none of the cards having hi bit set */
/* Descending versions */
int dmerge( char *a, char *b, char *c, int aN, int bN ) { /* no error checking; a, b, c must point to valid arrays of char */
   int i = 0 ; int j = 0 ; int k = 0 ;
   int kmax =aN + bN ;
   while (k < kmax) {
      if      ( i == aN )     { *(c + k++) = *(b + j++);  }
      else if ( j == bN )     { *(c + k++) = *(a + i++);  }
      else if (a[i] > b[j] )  { *(c + k++) = *(a + i++);  }
      else                    { *(c + k++) = *(b + j++);  }
   }
   assert (i == aN && j == bN ) ;
   return 0 ;
}

int dsort3( char a[3] ) {
	/* this 3 item sort is never called. but in theory the sort4 routine could start by calling it. */  
   char t ;
         if (*(a+1) > *(a) ) { t=*(a+1) ; *(a+1) = *(a) ; *(a) = t ; }         // sort first two elements; *(a) now > *(a+1)
         if (*(a+2) > *(a) ) { t=*(a+2); *(a+2)=*(a+1);*(a+1)=*(a);*(a)=t;}    // 3rd elem to *(a); shuffle other two up one
         else if (*(a+2) > *(a+1) ) {t=*(a+2); *(a+2)=*(a+1); *(a+1) = t ; }   // swap 2nd and 3rd elems	
   return 0 ; 
} /* end dsort3 */

int dsort4( char a[4] ) { /*  optimized version no error checking */
   char t ;
         if (*(a+1) > *(a) ) { t=*(a+1) ; *(a+1) = *(a) ; *(a) = t ; }      // sort first two elements; *(a) now > *(a+1)
         if (*(a+2) > *(a) ) { t=*(a+2); *(a+2)=*(a+1);*(a+1)=*(a);*(a)=t;}     // 3rd elem to *(a); shuffle other two up one
         else if (*(a+2) > *(a+1) ) {t=*(a+2); *(a+2)=*(a+1); *(a+1) = t ; }      // swap 2nd and 3rd elems

         if      (*(a+3) >= *(a) ) { t=*(a+3);*(a+3)=*(a+2);*(a+2)=*(a+1);*(a+1)=*(a);*(a)=t; } // shuffle all up one place
         else if (*(a+3) >= *(a+1) ) { t=*(a+3);*(a+3)=*(a+2);*(a+2)=*(a+1);*(a+1)=t; }         // shuffle 1..3 up one place
         else if (*(a+3) >  *(a+2) ) { t=*(a+3);*(a+3)=*(a+2);*(a+2)=t;}                        // swap *(a+2) with *(a+3)
	return 0 ; 
} /* end dsort4 */

int didxsort4( int v[4], int x[4] ) { 
	/* descending idx sort where we sort the suits in order based on a suit property such as len or hcp etc.
	 * could also be used to sort the hands in (desc) order based on the number of spades, etc.
	 * we compare the 'property' to determine if to swap or not; 
	 * if we do swap, we swap both the val/poperty pair and also the two indices
	 */
	 // v is the value/property array, x is the index array. x starts out as 0,1,2,3 always.
   char t ; int s ;
       if (*(v+1) > *(v) ) {      // sort first two elements; *(v) now > *(v+1)
			 t=*(v+1) ; *(v+1) = *(v) ; *(v) = t ; 
			 s=*(x+1) ; *(x+1) = *(x) ; *(x) = s ; 
		 } 
       if (*(v+2) > *(v) ) {     // 3rd elem to *(v); shuffle other two up one
			 t=*(v+2); *(v+2)=*(v+1);*(v+1)=*(v);*(v)=t;
			 s=*(x+2); *(x+2)=*(x+1);*(x+1)=*(x);*(x)=s;
		 } 
       else if (*(v+2) > *(v+1) ) {     // swap 2nd and 3rd elems
			 t=*(v+2); *(v+2)=*(v+1); *(v+1) = t ;
			 s=*(x+2); *(x+2)=*(x+1); *(x+1) = s ;  
		 } 

       if      (*(v+3) >= *(v) ) {   	// shuffle all up one place
			 t=*(v+3);*(v+3)=*(v+2);*(v+2)=*(v+1);*(v+1)=*(v);*(v)=t; 
			 s=*(x+3);*(x+3)=*(x+2);*(x+2)=*(x+1);*(x+1)=*(x);*(x)=s; 
		 } 
       else if (*(v+3) >= *(v+1) ) {  // shuffle 1..3 up one place 
			 t=*(v+3);*(v+3)=*(v+2);*(v+2)=*(v+1);*(v+1)=t;
			 s=*(x+3);*(x+3)=*(x+2);*(x+2)=*(x+1);*(x+1)=s;  
		 }           
       else if (*(v+3) >  *(v+2) ) {   // swap *(v+2) with *(v+3)
			 t=*(v+3);*(v+3)=*(v+2);*(v+2)=t;
			 s=*(x+3);*(x+3)=*(x+2);*(x+2)=s;
		 }                      
	return 0 ; 
} /* end didxsort4 */

/* Use this sort for the normal Hand sort to put the cards in desc order */
int dsort13 (char a[13] ) { /* Desc Sort used to sort a hand. optimized version does not check for errors */
   char u[13], v[13], w[13] ;
   dsort4(&a[0] );       					
   dsort4(&a[4] );       					
   dsort4(&a[8] );       					
   dmerge(&a[0], &a[4], u , 4 , 4 ) ;  		
   dmerge(&u[0], &a[8], v , 8 , 4 ) ;  		
   dmerge(&v[0], &a[12],w , 12, 1 ) ;  
   memcpy(a , w, 13 ) ;
   return 0 ;
} /* end dsort13 */
	
/* Ascending versions of sorts */
int merge( char *a, char *b, char *c, int aN, int bN ) { /* no error checking; a, b, c must point to valid arrays of char */
   int i = 0 ; int j = 0 ; int k = 0 ;
   int kmax =aN + bN ;
   while (k < kmax) {
      if( i == aN)            { *(c + k++) = *(b + j++);  }
      else if ( j == bN )     { *(c + k++) = *(a + i++);  }
      else if (a[i] < b[j] )  { *(c + k++) = *(a + i++);  }
      else                    { *(c + k++) = *(b + j++);  }
   }
   assert (i == aN && j == bN ) ;
   return 0 ;
}

int sort3( char a[3] ) { // Ascending optimized version does not check for errors; sorts first 3 elements of a */
	char t ; 
         if (a[1] < a[0] ) { t = a[0] ; a[0] = a[1] ; a[1] = t ; }      // sort first two elements; a[0] now < a[1]
         if (a[2] < a[0] ) { t = a[0]; a[0]=a[2];a[2]=a[1];a[1]=t;}     // 3rd elem to a[0]; shuffle other two up one
         else if (a[2] < a[1] ) { t = a[1] ; a[1] = a[2] ; a[2] = t ; } // swap 2nd and 3rd elems
    return 0 ; 
}  /* end sort3 */

int sort4(char a[4]) { /* Ascending optimized version of sort; does not check for errors */
   char t ;
         if (a[1] < a[0] ) { t=a[1] ; a[1] = a[0] ; a[0] = t ; }      // sort first two elements; a[0] now < a[1]
         if (a[2] < a[0] ) { t=a[2]; a[2]=a[1];a[1]=a[0];a[0]=t;}     // 3rd elem to a[0]; shuffle other two up one
         else if (a[2] < a[1] ) {t=a[2]; a[2]=a[1]; a[1] = t ; }      // swap 2nd and 3rd elems

         if      (a[3] <= a[0] ) { t=a[3];a[3]=a[2];a[2]=a[1];a[1]=a[0];a[0]=t; } // shuffle all up one place
         else if (a[3] <= a[1] ) { t=a[3];a[3]=a[2];a[2]=a[1];a[1]=t; }           // shuffle 1..3 up one place
         else if (a[3] <  a[2] ) { t=a[3];a[3]=a[2];a[2]=t;}                      // swap a[2] with a[3]
	return 0 ; 
} /* end sort4 */

int sort13 (char a[13] ) { /* Ascending sort optimized version does not check for errors */
   char u[13], v[13], w[13] ;
   sort4(&a[0] );       					
   sort4(&a[4] );       					
   sort4(&a[8] );       					
   merge(&a[0], &a[4], u , 4 , 4 ) ;  		
   merge(&u[0], &a[8], v , 8 , 4 ) ;  		
   merge(&v[0], &a[12],w , 12, 1 ) ;  
   memcpy(a , w, 13 ) ;
   return 0 ;
} /* end sort13 */

int aidxsort4( int v[4], int x[4] ) {
   char t ; int s ;
       if (*(v+1) < *(v) ) {      // sort first two elements; *(v) now > *(v+1)
			 t=*(v+1) ; *(v+1) = *(v) ; *(v) = t ; 
			 s=*(x+1) ; *(x+1) = *(x) ; *(x) = s ; 
		 } 
       if (*(v+2) < *(v) ) {     // 3rd elem to *(v); shuffle other two up one
			 t=*(v+2); *(v+2)=*(v+1);*(v+1)=*(v);*(v)=t;
			 s=*(x+2); *(x+2)=*(x+1);*(x+1)=*(x);*(x)=s;
		 } 
       else if (*(v+2) < *(v+1) ) {     // swap 2nd and 3rd elems
			 t=*(v+2); *(v+2)=*(v+1); *(v+1) = t ;
			 s=*(x+2); *(x+2)=*(x+1); *(x+1) = s ;  
		 } 

       if      (*(v+3) <= *(v) ) {   	// shuffle all up one place
			 t=*(v+3);*(v+3)=*(v+2);*(v+2)=*(v+1);*(v+1)=*(v);*(v)=t; 
			 s=*(x+3);*(x+3)=*(x+2);*(x+2)=*(x+1);*(x+1)=*(x);*(x)=s; 
		 } 
       else if (*(v+3) <= *(v+1) ) {  // shuffle 1..3 up one place 
			 t=*(v+3);*(v+3)=*(v+2);*(v+2)=*(v+1);*(v+1)=t;
			 s=*(x+3);*(x+3)=*(x+2);*(x+2)=*(x+1);*(x+1)=s;  
		 }           
       else if (*(v+3) <  *(v+2) ) {   // swap *(v+2) with *(v+3)
			 t=*(v+3);*(v+3)=*(v+2);*(v+2)=t;
			 s=*(x+3);*(x+3)=*(x+2);*(x+2)=s;
		 }                      
	return 0 ; 
} /* end aidxsort4 */	 

void sortDeal(DEAL52_k dl ) { // Sort each hand in the deal in Desc order; helps printouts esp PBN/GIB ones and "has_card" funcs
    int p ;
    char *h_ptr ;
    for (p = 0 ; p < 4 ; p++ ) { /* p is the player, 0=North, etc. We sort each quarter of the deal separately */
        h_ptr = (char *) &dl[p*13];
        JGMDPRT(7, "Sorting hand[%d] starting with card [%02x] -> [%02x]\n", p, dl[p*13], *h_ptr );
        dsort13((char *) h_ptr)  ;
    }
} /* end sortDeal */

/* Random number Functions */
long int init_rand48( long int seed ) {
    union {
        unsigned short int sss[3] ;
                      char sbuff[6];
    } su;
    size_t numbytes;
    long int u_seed48, two16, two32 ;
    two16 = 1UL << 16 ;
    two32 = two16 * two16 ;

    if (seed != 0L ) { /* user has provided his own seed so just use it */
        srand48(seed) ;
        return seed ;
    }
    /* user has not specified a seed, so get one from the kernel and use seed48 instead of srand48*/
    numbytes = getrandom(su.sbuff, sizeof(su.sbuff), 0) ;
    if ( !numbytes ) {
       perror("getrandom FAILED. Cannot seed rand48") ;
       exit (-1) ;
    }
    assert( numbytes == 6 ) ;
    seed48(su.sss) ;   /* ignore seed48 return value of pointer to internal buffer */
    u_seed48 = (long int)su.sss[0] + (long int)su.sss[1]*two16 + (long int)su.sss[2]*two32;
    JGMDPRT(3,"No Seed Provided. DONE Initializing RNG init_rand48, %d, %d, %d, %ld\n",
                su.sss[0], su.sss[1], su.sss[2], u_seed48);
    return u_seed48;
} /* end init_rand48 */

int gen_rand_slot ( int topval ) { /* return a random number in the range [0 .. (topval - 1)] -- Note Closed interval */
        /* mrand48 returns a number in range -2^31 .. +2^31 i.e. a 32 bit range. 
         * lrand48 and nrand48 range is 31 bits [0 .. 2^31)
         * drand48 returns a double in the range [0.0 .. 1.0); It is faster to mult double than modulo div an int.
         * especially when using modulo division on a long signed integer. Modulo division is more prone to biased results
         */
        return ( (int) (drand48() * (double) topval) ) ; 
        /* Explanatory Math notes:
         * drand48 is a 48 bit double [0.0 .. 1.0) i.e. 0 - 0.99999999, 
         * so (int) (drand48 * topval) gets 0 .. topval-1
         * if we do rnd(100) we get the expected Variance of 28.866.. ; 
         * if we do rnd(101) our Variance is 29.149...
         * Therefore to shuffle a deck of size N with indices of 0 .. N-1 we want an int in the range 0 .. N-1
         * and we call gen_rnd_slot( 52 ) 
         */
 }


/* --------------------- Unused Routines but potentially useful ----------------- */
/* these next 3 routines all use qsort, the glibc library sort. Not used in Dealerv2 since dsort13 and dsort4 are faster */
int dcmp_card_Q(  const void *x, const void *y ) {  // Descending qsort compare function for cards which are just chars
   if ( *(char *)x < *(char *)y ) return  1 ;
   if ( *(char *)x > *(char *)y ) return -1 ;
   return 0 ;                       // Should never happen
}

void handsort_Q( char *hnd_ptr) { /* Sort 13 cards in Descending Order Spade Ace downto Club Deuce */
   qsort(hnd_ptr, 13, sizeof(char), dcmp_card_Q ) ;
}

void dealsort_Q( char *dl_ptr ) { /* Sort the 4 hands individually using qsort from the system library*/
   char * h_ptr ;
   for (int p = 0 ; p < 4 ; p++ ) {
      h_ptr = dl_ptr + 13*p ;
      handsort_Q(h_ptr) ;
   }
}

/* This next routine obsoleted by dsort13 
 * simple  Descending Order Ace to Deuce Insertion sort:
 * Test show that for 13 elems Insertion is faster than Shell or Selection
 * for a 52 element array as opposed to 4x13 elements Shell sort is about 10% faster than Insertion */
void sortHand(char *hnd_ptr, int size) {
	if (size == 13 ) {	dsort13( hnd_ptr ) ; 	return ; }
	
    int key, j, step;
    /* we can use this to sort the 'hands' in deal. pass it deal[0], deal[13], deal[26] or deal[39] with size of 13 */
  for (step = 1; step < size; step++) {
    key = *(hnd_ptr + step);
    j = step - 1;
    while (key > *(hnd_ptr + j) && j >= 0) {
      *(hnd_ptr + j+1) = *(hnd_ptr + j);
      --j;
    }
    *(hnd_ptr + j+1) = key;
  } /* end for step */
} /* end sortHand */
