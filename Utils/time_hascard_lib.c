/* compile this with
 * LDFLAGS="-flto -pthread -O3 -L../lib -ldealerV2"
 * GCCFLAGS="-std=gnu17 -mtune=corei7 -flto -fopenmp -pthread -Wall -pedantic -g -O3 -I../include"
 */
char PGMVERS[]="Version 1.3";
#define _GNU_SOURCE 1
#include "../include/jgmdebug.h"
#include "../include/dbgprt_macros.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>    /* for qsort  */
#include <string.h>    /* for memcpy */
#include <sys/random.h>       	/* for getrandom() */
#include <sys/time.h>  /* for gettimeofday, struct timeval */

#include "../include/libdealerV2.h"

#undef DEBUG
//#define DEBUG 
int jgmDebug = 2 ;
long global_flag[4] = {0};
long elapsed_usec(struct timeval tv_start, struct timeval tv_stop ) {
	long elapsed_sec ; 
	long elapsed_us ; 
	elapsed_sec = (unsigned long)tv_stop.tv_sec - (unsigned long)tv_start.tv_sec ;
	elapsed_us= (unsigned long)tv_stop.tv_usec - (unsigned long)tv_start.tv_usec ;
	#ifdef DEBUG
		printf("StopTime = %ld.%ld, StartTime = %ld.%ld, ElapsedTime = %ld.%ld \n",
			tv_stop.tv_sec,tv_stop.tv_usec,tv_start.tv_sec,tv_start.tv_usec,elapsed_sec,elapsed_us );
	#endif
	return (elapsed_sec * 1000000L) + elapsed_us ; 
  }		 
int hascard1( char *hand, char card) {  /* unsorted hand; for i = 1 to 13 */
	int i = 0 ;
	char *p_card = hand;
	while( i++ < 13) {
		#ifdef DEBUG
			printf("Hascard1: card=%02X *p_card[%d]=%02X\n",card, i, *p_card);
		#endif
		if (*p_card++ == card ) { return 1 ; }
	}
	return 0 ;
} /* end unsorted hascard */
int hascard2( char *hand, char card) {  /* hand sorted desc order */
	char *p_card;
	int i = 0 ; 
	p_card=hand ; 
	#ifdef DEBUG 
		printf("Hascard2: hand start at %p first card=%02X : %02X\n",hand, hand[0], *p_card );
	#endif
	while( (card < *p_card++) && i++ < 13 ) {
		#ifdef DEBUG 
			printf("Hascard2: card=%02X *p_card[%d]=%02X\n",card, (i-1), *(p_card-1) );
		#endif
	} /* either i >= 13 or card >= *(p_card - 1) */
	#ifdef DEBUG 
		printf("Hascard2 While Loop ends with i=%d and *(p_card -1)=%02X (p_card -1)=%p\n",i,*(p_card-1),(p_card-1) );
	#endif
	if (i < 13 && *(p_card-1) == card ) { return 1 ; } /* if it runs off the end of the hand get garbage */
	return 0 ;
}
long time_dsort13 ( long maxloops) {
	long l;
	long time_taken;
	struct timeval tvstart, tvstop;
	char test_arr[14]={'A','1','B','2','C','3','d','4','e','5','f','5','@' };
	char ch_arr[14] ;
	gettimeofday (&tvstart, (void *) 0);
	for (l=0; l< maxloops ; l++ ) {
		memcpy(ch_arr, test_arr, 13) ; 
		dsort13(ch_arr) ;   // Make the deal random again
	}
	gettimeofday (&tvstop, (void *) 0);
    time_taken = elapsed_usec( tvstart, tvstop);
    printf("time_dsort Elapsed uSec = %ld\n",time_taken);
	return time_taken;  
}	  
long time_sortDeal (char *dl,  long maxloops) {
	long l;
	long time_taken;
	struct timeval tvstart, tvstop;
	char sdl[52];
	gettimeofday (&tvstart, (void *) 0);
	for (l=0; l< maxloops ; l++ ) {
		memcpy(sdl, dl, 52) ; 
		sortDeal(sdl) ;
	}
	gettimeofday (&tvstop, (void *) 0);
    time_taken = elapsed_usec( tvstart, tvstop);
    printf("time_dsortDeal Elapsed uSec = %ld\n",time_taken);
	return time_taken;  
}

long time_memcpy (char *dst, char *src, long maxloops) {
	long l;
	long time_taken;
	struct timeval tvstart; 
	struct timeval tvstop;
	gettimeofday (&tvstart, (void *) 0);
	for (l=0; l< maxloops ; l++ ) {
		memcpy(dst,src, 52) ;   // Make the deal random again
		global_flag[0]=*dst ; global_flag[4] = *(dst + 51); /* junk statements to foil optimizer */
	}
	gettimeofday (&tvstop, (void *) 0);
    time_taken = elapsed_usec( tvstart, tvstop);
    printf("time_memcpy Elapsed uSec = %ld\n",time_taken);
	return time_taken;  
}	  

 int main (int argc, char **argv)  {
	 printf("START PGMVERS = %s\n", PGMVERS );
	char dl[52];
	char sdl[52];
	char card ; 
    long l, maxloops ;
    long copy_usec, dsort13_us, sortDeal_us;
    double time_sorted, time_unsorted, time_ignored, time_each;
    struct timeval tvstart, tvstop;
    long found1_cnt = 0 ;
    long found2_cnt = 0 ; 
    long found3_cnt = 0 ; 
    printf("Size of time_t=%ld, Size of suseconds_t=%ld \n",sizeof(time_t), sizeof(suseconds_t) );
/* prologue */
	newdeck(dl , 'D' ) ; /* the four hands will be north east south west */
	init_rand48( 11 ) ; 
	Shuffle(dl, 52) ;
	sr_deal_show(dl) ; 
	
	printf("**** Starting Various Time Tests with seed = 11; Deal Shuffled ****\n"); 
	
	/* card = heart  9 halfway thru */
	card = MAKECARD(2,7) ;
	// int slot = gen_rand_slot(52) ;
  #ifdef DEBUG
    int found ; 
	found = hascard1(dl, card) ;
	printf("Heart 9 Hascard1 in North returns Found=%d\n",found);
	found = hascard1(dl+13, card) ;
	printf("Heart 9 Hascard1 in East returns Found=%d\n",found);
	found = hascard1(dl+26, card) ;
	printf("Heart 9 Hascard1 in South returns Found=%d\n",found);
	found = hascard1(dl+39, card) ;
	printf("Heart 9 Hascard1 in West returns Found=%d\n",found);
	card = MAKECARD(0,0) ; 
	found = hascard1(dl+39, card) ;
	printf("Club 2 Hascard1 in West returns Found=%d\n",found);	
	// return 0 ;
	printf("Sorting Deal and running hascard2\n");
	sortDeal(dl);
	sr_deal_show(dl); 
	card = MAKECARD(2,7) ;
	found = hascard2(dl, card) ;
	printf("Heart 9 Hascard2 in North returns Found=%d\n",found);
	found = hascard2(dl+13, card) ;
	printf("Heart 9 Hascard2 in East returns Found=%d\n",found);
	found = hascard2(dl+26, card) ;
	printf("Heart 9 Hascard2 in South returns Found=%d\n",found);
	found = hascard2(dl+39, card) ;
	printf("Heart 9 Hascard2 in West returns Found=%d\n",found);
	card = MAKECARD(0,0) ; 
	found = hascard2(dl+39, card) ;
	printf("Club 2 Hascard2 in West returns Found=%d\n",found);	
	// return 0 ; 
#endif
    maxloops = 1000L;
    
    if (argc > 1 ) {maxloops = (long) atoi(argv[1]) ;} /* parm is in Millions of Loops */
    #ifndef DEBUG
		maxloops = 1000000 * maxloops; // allow to spec v.few loops for debugs
	#endif
    copy_usec = time_memcpy(sdl, dl, maxloops) ; 
    dsort13_us= time_dsort13(maxloops); 
    sortDeal_us=time_sortDeal(dl, maxloops); 
    printf("Maxloops=%ld memcpy Overhead=%ld usec, dsort13_us=%ld, sortDeal_us=%ld\n", 
			maxloops, copy_usec, dsort13_us, sortDeal_us);
	printf("Sort Times less the memcpy times: dsort13=%ld, sortDeal=%ld \n",
			(dsort13_us - copy_usec), (sortDeal_us - copy_usec) ) ;
 printf("Stop PGMVERS %s\n",PGMVERS); 
 return 0 ; 
 // [.............................Unsorted............................................]    
 // hascard1 unsorted hand search each hand to simulate finding and not finding cards
 printf("Begining %ld Loops for Unsorted Hands ",maxloops);
 gettimeofday (&tvstart, (void *) 0);
 for (l=0; l< maxloops ; l++ ) {
	// sortDeal(dl) ;   // sort the deal once ; applies to all players.
	for (int p=0 ; p<4 ; p++ ) { /* player North to West */
		 if( hascard1(dl+p*13,card) > 0 ) {found1_cnt++; }  /* search card in each hand dont assume sorted */
    }
    
  }
     gettimeofday (&tvstop, (void *) 0);
   printf(" Start at %ld secs \n",tvstart.tv_sec);
   printf("Stop Loop Cnt=%ld for Unsorted Hands Stop at %ld secs \n",l, tvstop.tv_sec);
     time_unsorted = (double) ((double)tvstop.tv_sec + (double)tvstop.tv_usec / 1000000.0 -
                              ((double)tvstart.tv_sec + (double)tvstart.tv_usec / 1000000.0) );
     printf ("\tTime needed for %ld x  un-Sorted Deal all seats %10.6f sec \n", maxloops, time_unsorted );

 // [.............................Ignore............................................]
  printf("Begining %ld Loops for Ignore Sort ",maxloops);
  sortDeal(dl) ;   // sort the deal to make sure
  gettimeofday (&tvstart, (void *) 0);
  for (l=0; l< maxloops ; l++ ) {
	for (int p=0 ; p<4 ; p++ ) { /* player North to West */
		 if (hascard1(dl+p*13,card)) {found3_cnt++ ;}   /* search card in each hand */
    }
  }
   gettimeofday (&tvstop, (void *) 0);
   printf(" Start at %ld secs \n",tvstart.tv_sec);
   printf("Stop Loop Cnt=%ld for Ignore Sort Stop at %ld secs \n",l, tvstop.tv_sec);
   time_ignored = (double) ((double)tvstop.tv_sec + (double)tvstop.tv_usec / 1000000.0 -
                              ((double)tvstart.tv_sec + (double)tvstart.tv_usec / 1000000.0) );
   printf ("\tTime needed for %ld x  Ignore Sort all seats %10.6f sec \n", maxloops, time_ignored );
   
 // [.............................Sorted ............................................]
 // hascard2 -- sorted hand
  printf("Begining %ld Loops for   Sorted Hands ",maxloops);
  sortDeal(dl) ; // sort the deal once ; applies to all players.
  gettimeofday (&tvstart, (void *) 0);
   // printf("Begining %ld Loops for   Sorted Hands Start at %ld secs \n",maxloops, tvstart.tv_sec);
  for (l=0; l< maxloops ; l++ ) {
	for (int p=0 ; p<4 ; p++ ) { /* player North to West */
		 if( hascard2(dl+p*13,card) ) {found2_cnt++; }  /* search card in each hand */
    } /* end for player */
  }  /* end for loops */
   gettimeofday (&tvstop, (void *) 0);
   printf(" Start at %ld secs \n",tvstart.tv_sec);
   printf("Stop Loop Cnt=%ld for   Sorted Hands Stop at %ld secs \n",l, tvstop.tv_sec);
   time_sorted= (double) ((double)tvstop.tv_sec + (double)tvstop.tv_usec / 1000000.0 -
                              ((double)tvstart.tv_sec + (double)tvstart.tv_usec / 1000000.0) );
   printf ("\tTime needed for %ld x     Sorted Deal all seats %10.6f sec \n", maxloops, time_sorted );
 // [..........................Sorted Each Time ............................................]
 // hascard2 -- sorted hand
  printf("Begining %ld Loops for   Sorted Hands ",maxloops);
  Shuffle(dl, 52) ; // randomize the Deal
  gettimeofday (&tvstart, (void *) 0);
   // printf("Begining %ld Loops for   Sorted Hands Start at %ld secs \n",maxloops, tvstart.tv_sec);
  for (l=0; l< maxloops ; l++ ) {
	  memcpy(sdl, dl, 52) ;   // Make the deal random again
	  sortDeal(sdl) ;         // now sort it
	for (int p=0 ; p<4 ; p++ ) { /* player North to West */
		 if( hascard2(dl+p*13,card) ) {found2_cnt++; }  /* search card in each hand */
    } /* end for player */
  }  /* end for loops */
   gettimeofday (&tvstop, (void *) 0);
   printf(" Start at %ld secs \n",tvstart.tv_sec);
   printf("Stop Loop Cnt=%ld for   Sorted Hands Stop at %ld secs \n",l, tvstop.tv_sec);
   time_each= (double) ((double)tvstop.tv_sec + (double)tvstop.tv_usec / 1000000.0 -
                              ((double)tvstart.tv_sec + (double)tvstart.tv_usec / 1000000.0) );
   printf ("\tTime needed for %ld x     Sorted Deal all seats %10.6f sec \n", maxloops, time_each );

 // [.........................................................................]


	  printf("------------------ Summary %ld Loops-------------------- \n",maxloops);
	  printf(" Unsorted Hand     %10.6f \n",time_unsorted) ; 
	  printf(" Ignored Sort      %10.6f \n",time_ignored) ; 
	  printf("   Sorted Hand     %10.6f \n",time_sorted) ;
	  printf("Sorted Each Hand   %10.6f \n",time_each) ;
	  printf("Adjusted Each Hand %10.6f \n",time_each - (double)copy_usec/1000000.0) ;
	  
	  printf("\n") ;
	  printf("Junk Output to fool optimier:%ld, %ld, %ld\n",found1_cnt,found2_cnt, found3_cnt) ;

      return 0;
}
#if 0

#endif  // if 0
