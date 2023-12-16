#define _GNU_SOURCE 1
#include "../include/jgmdebug.h"
#include "../include/dbgprt_macros.h"
#include "../include/libbridge.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>    /* for qsort, drand48, etc. */
#include <string.h>    /* for memcpy */
#include <sys/time.h>  /* for gettimeofday, struct timeval */
#include <sys/random.h>
#include <math.h>

/* testing some timing of various random calls; can we improve on modulo division
 * the following macros work;  this SWAP is faster than 3 XORs with no Temp variable.
 */
 #define MIN( A , B ) ( (B) < (A) ) ? (B) : (A)
 #define MAX( A , B ) ( (B) < (A) ) ? (A) : (B)
 #define SWAP(A , B ) { int _tmpZQX = (A) ; (A) = (B) ; (B) = _tmpZQX ; }
#undef DEBUG

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
#ifdef JGMDBG
    fprintf(stderr, "DONE Initializing RNG from jgm init_program-> init_rand48, %d, %d, %d, %ld\n",
                su.sss[0], su.sss[1], su.sss[2], u_seed48);
#endif
    return u_seed48;
} /* end init_rand48 */

int gen_mrand_slot ( int topval ) { /* return a random number in the range [0 .. (topval - 1)] */
        /* mrand48 returns a number between in range -2^31 .. +2^31 i.e. a 32 bit range. */
        return ( abs( (int) (mrand48() % (long int) topval) ) ) ;
}

int gen_drand_slot ( int topval ) { /* return a random number in the range [0 .. (topval - 1)] */
        /* drand48 returns a number between in range [0.0 .. 1.0) */
        return ((int) ( drand48() * (double) topval ) );
}

int gen_lrand_slot ( int topval ) { /* return a random number in the range [0 .. (topval - 1)] */
        /* mrand48 returns a number between in range -2^31 .. +2^31 i.e. a 32 bit range. */
        return ( (int) (lrand48() % (long int) topval) ) ;
}
 int main (int argc, char **argv)  {
	 int l ;
	 double elapsed_time ;
	 double rnd_sum, rnd_sumsq,  rndval_avg, rndval_var, rndval_sd ; 
	 long int seed ;
	 int rndval ;
	 int topval = 100 ; 
	 int maxloops = 100000000; /* 100 Million */
	 char rnd_typ = 'D' ; 
	 char rnd_parm ; 
		seed = init_rand48(919) ; 
	    if (argc > 1 ) {maxloops = atoi(argv[1]) ;}
	    if (argc > 2 ) { 
			rnd_parm = argv[2][0] ;  printf("Parsing %c \n", rnd_parm );
	        switch (rnd_parm) {
			case 'l' :
			case 'L' :
				rnd_typ = 'L' ;
				break ; 
			case 'm' :
			case 'M' :
				rnd_typ = 'M';
				break ;
			default :
				rnd_typ = 'D' ; 
			} /* end switch */
		} /* end if argc > 2 */	
	    printf("Will run %d loops of type %c \n",maxloops, rnd_typ);

        struct timeval tvstart, tvstop;
        
        switch ( rnd_typ ) {
		case 'D' :
            gettimeofday (&tvstart, (void *) 0);
			for (l=0; l< maxloops ; l++ ) {
				rndval = gen_drand_slot(topval) ; 
				rnd_sum += rndval ; 
				rnd_sumsq += rndval * rndval ; 
			}		
			gettimeofday (&tvstop, (void *) 0);
			elapsed_time = (double) (tvstop.tv_sec + tvstop.tv_usec / 1000000.0 -
                              (tvstart.tv_sec + tvstart.tv_usec / 1000000.0) );
            break ; 
 		case 'M' :
            gettimeofday (&tvstart, (void *) 0);
			for (l=0; l< maxloops ; l++ ) {
				rndval = gen_mrand_slot(topval) ; 
 				rnd_sum += rndval ; 
				rnd_sumsq += rndval * rndval ; 
			}         
			gettimeofday (&tvstop, (void *) 0);
			elapsed_time = (double) (tvstop.tv_sec + tvstop.tv_usec / 1000000.0 -
                              (tvstart.tv_sec + tvstart.tv_usec / 1000000.0) );
            break ;           
 		case 'L' :
            gettimeofday (&tvstart, (void *) 0);
			for (l=0; l< maxloops ; l++ ) {
				rndval = gen_lrand_slot(topval) ; 
 				rnd_sum += rndval ; 
				rnd_sumsq += rndval * rndval ; 
			}         

			gettimeofday (&tvstop, (void *) 0);
			elapsed_time = (double) (tvstop.tv_sec + tvstop.tv_usec / 1000000.0 -
                              (tvstart.tv_sec + tvstart.tv_usec / 1000000.0) );
            break ; 
       default :
			printf("Cant happen! Bad Random Type\n");
			break ;
		} /* end switch */
		printf("Seed = %ld Generated %d random numbers of type %c in %10.6f sec \n",seed, maxloops, rnd_typ, elapsed_time ) ; 
		rndval_avg = rnd_sum / maxloops ;
		rndval_var = (rnd_sumsq - rnd_sum*rnd_sum/maxloops) / (maxloops ) ; // maxloops - 1 ?
		rndval_sd  = sqrt(rndval_var) ;
		printf("Mean = %10.4f , Std Dev = %10.4f , Variance = %10.4f , Sample Size = %d \n",
			rndval_avg, rndval_sd, rndval_var, maxloops ) ; 
		

	return 0 ; 
} /* end main */
#if 0
------------------ Results -------------------------
When it comes to generating an random number in a range, it depends a lot on the optimization level of GCC. 
The D method which returns [0.0 .. 1.0) is fastest with -Og (debugging level of optimization)
The L method which returns an integer in the rand 0 .. 2^31 - 1 is fastest with -O3 (D is about 8% slower)
The M method which returns a signed integer in the range -2^31 .. +2^31 -1 is always slowest.
-------------------------------------------------------
greg21@trojan:~/DealerV4/Utils$ echo $GCCFLAGS
-std=gnu17 -mtune=corei7 -flto -fopenmp -pthread -Wall -pedantic -O3 -I../include  //<<<== Note -O3

greg21@trojan:~/DealerV4/Utils$ gcc $GCCFLAGS -c TimeRandom.c
greg21@trojan:~/DealerV4/Utils$ gcc $GCCFLAGS -o TimeRandom  TimeRandom.o -lm

greg21@trojan:~/DealerV4/Utils$ ./TimeRandom 100000000 L
Will run 100000000 loops of type L 							 ****************
Seed = 919 Generated 100000000 random numbers of type L in   * 0.860850 sec *
Mean =    49.5026 , Std Dev =    28.8651 , Variance =   833.1950 , Sample Size = 100000000 
greg21@trojan:~/DealerV4/Utils$ ./TimeRandom 100000000 D
Will run 100000000 loops of type D 							 ****************
Seed = 919 Generated 100000000 random numbers of type D in   * 0.937636 sec *
Mean =    49.5031 , Std Dev =    28.8671 , Variance =   833.3090 , Sample Size = 100000000 
greg21@trojan:~/DealerV4/Utils$ ./TimeRandom 100000000 M
Will run 100000000 loops of type M 							 **************** 
Seed = 919 Generated 100000000 random numbers of type M in   * 1.562758 sec * 
Mean =    49.4960 , Std Dev =    28.8649 , Variance =   833.1807 , Sample Size = 100000000 
++++++++++++++++++++++++ With Debugging ++++++++++++++++++++++++++++++++++
greg21@trojan:~/DealerV4/Utils$ ./dbgRandom 1000000000 L
Will run 1000000000 loops of type L 
Seed = 919 Generated 1000000000 random numbers of type L in  17.443367 sec 
Mean =    49.5000 , Std Dev =    28.8655 , Variance =   833.2197 , Sample Size = 1000000000 
greg21@trojan:~/DealerV4/Utils$ ./dbgRandom 1000000000 D
Will run 1000000000 loops of type D 
Seed = 919 Generated 1000000000 random numbers of type D in   9.723855 sec 
Mean =    49.5008 , Std Dev =    28.8667 , Variance =   833.2855 , Sample Size = 1000000000 
greg21@trojan:~/DealerV4/Utils$ ./dbgRandom 1000000000 M
Will run 1000000000 loops of type M 
Seed = 919 Generated 1000000000 random numbers of type M in  25.515292 sec 
Mean =    49.4987 , Std Dev =    28.8661 , Variance =   833.2498 , Sample Size = 1000000000
#endif
