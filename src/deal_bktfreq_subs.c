/* File bktfreq_subs.c */
/* Date      Version Author   Description
* 2022/12/31 1.0.0   JGM      Added bktfreq to Dealerv2 features.
* 2024/07/03 2.2.0   JGM      Mods to Frequency fmt for bigger numbers, and Pcts for 1D freq.
* 2024/09/07 2.2.1   JGM      malloc/calloc BUG workaround.
* 2024/09/08 2.3.0   JGM      Fixing the source of the calloc bug. 
*/
#ifndef GNU_SOURCE
  #define GNU_SOURCE
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include "../include/dbgprt_macros.h"
#ifndef DEAL_BKTFREQ_SUBS_H
  #include "../include/deal_bktfreq_subs.h"
#endif
extern int jgmDebug ;
/* looking for way to avoid these */
// int max_bkta_Num;    /* calloc BUG */
// int *col_tot ;
int int_sz = (int )sizeof(int) ; /* saves a bunch of casting from size_t to int. */

struct MemPtrs_st {
   void *ptr ;
   char descr[24] ;
   int  bytes ;
} ;
struct MemPtrs_st mem_ptrs[100] ;
#define MAX_ALLOCS 100
int    mem_ptrs_cnt = 0 ;
void track_alloc( void *ptr, char *desc, int sz ) { // uses globals mem_ptrs array and mem_ptrs_cnt
   if (mem_ptrs_cnt  >= MAX_ALLOCS ) {
      fprintf(stderr, "MAX_ALLOCS of %d reached. Curr_cnt=%d. Increase Array size and re-compile\n",MAX_ALLOCS, mem_ptrs_cnt );
      return ;
   }
   mem_ptrs[mem_ptrs_cnt].ptr = ptr ;
   mem_ptrs[mem_ptrs_cnt].bytes = sz;
   strncpy(mem_ptrs[mem_ptrs_cnt].descr, desc, sizeof(mem_ptrs[0].descr)-1) ;
   if(jgmDebug >= 7) {
      fprintf(stderr, "TrackAlloc entry %d with Descr[%s] of Size=%d & ptr=%p \n",mem_ptrs_cnt, desc, sz, ptr ) ;
   }
   mem_ptrs_cnt++;
   return ;
}

void rpt_alloc( void ) {
   if (mem_ptrs_cnt == 0 ) return ; 
   fprintf(stderr, "[ idx  ] : [         Description    ] : [  Size  ] : Pointer \n" ) ;
   for (int i=0 ; i < mem_ptrs_cnt ; i++ ) {
      fprintf(stderr, "[%6d] : [%24s] : [%08X] : %p \n", i, mem_ptrs[i].descr, mem_ptrs[i].bytes, mem_ptrs[i].ptr ) ;
   }
}

int *alloc_bkts1D(struct bucket_st *bkt) {
   int *alloc_ptr ;
   alloc_ptr = calloc( (size_t)bkt->Num , sizeof(int) ) ;
   if (NULL == alloc_ptr ) {
      perror(" Cant calloc space for 1D BktNames ");
      exit (-7);
   }
   bkt->Names = alloc_ptr ;
   bkt->Names[0]              = bkt->Lo - bkt->Sz ; // Dummy init for uflow bkt name
   bkt->Names[bkt->Num - 1]   = bkt->Hi + bkt->Sz ; // Dummy init for uflow bkt name
   alloc_ptr  = calloc( (size_t) (bkt->Num) , sizeof(int) ) ; // room to store counters.
   if (NULL == alloc_ptr ) {
      perror(" Cant calloc space for 1D Counters ");
      exit (-7);
   }
   JGMDPRT(4,"ALLOC1D:: Freq1D ptr = %p , Num of Buckets=%d \n",
                       (void *)alloc_ptr, bkt->Num );
   return ( alloc_ptr ) ;  // freq1D set to this value on return
} /* end allocate 1D Ram */

int upd_bkt1D( int val, int *bkt_arr, struct bucket_st *bkt ) {
   /* returns the bkt_id, 0..numbkts,  used to store the value */
   /* bkt_arr[0] is LO i.e. values less than lowbound
    * bkt_arr[1] is for lowbound <= val < (lowbound + bkt_sz)
    * bkt_arr[num_bkts-2] is for HI i.e. hibound <= val < hibound+bktsz
    * bkt_arr[num_bkts - 1] is for val >= (hibound + bktsz) (Note the = sign)
    */
    int bkt_id = 0 ;
    if (val <  bkt->Lo ) {
       bkt_arr[0]++ ;
       JGMDPRT(7," :: upd_bkt1D[0] with val=%d NewCount=%d\n", val,bkt_arr[0]) ;
       return (0 ) ;
    }
    if (val >= bkt->Lo + (bkt->Num-2) * bkt->Sz ) {
       bkt_arr[bkt->Num-1]++ ;
       JGMDPRT(7," :: upd_bkt1D[%d] with val=%d NewCount=%d\n", val, (bkt->Num - 1 ), bkt_arr[bkt->Num-1]) ;
       return (bkt->Num - 1 ) ;
    } // OK
    bkt_id = ( 1 + (val - bkt->Lo)/bkt->Sz ) ;
    bkt_arr[bkt_id]++ ;  // OK
    JGMDPRT(7,"::upd_bkt1D[%d]:: with VAl=%d  NewCount=%d \n", bkt_id, val, bkt_arr[bkt_id] );
    return (bkt_id) ;
} /* end upd_bkt */

int show_freq1D (int *freq, char *descr, struct bucket_st *bkt, char direction ) {
   /* freq contains the counts, numbkts is ((hibound - lobound)/bkt_sz + 1) + 2(for uflow and oflow)
    * freq[0] is uflow, freq[bkt->Num - 1] is for oflow
    * uflow and oflow are not printed if they are zero
    * direction is whether to print the whole freq array on one line ('a' = across ) limited in practice to 21 data buckets,
    * or print one line per bucket down the page ('d' = down  Default is down)
    */
        int i ;
        int colsum;
        double dsum, pct, cumpct;
        switch (tolower(direction) ) {
        case 'd':
        default :
        printf ("\nDescription: %s\n", descr);
        printf ("Value\t   Count\t   Pct.\t  CumPct\n");
        /* create a total */
        colsum = 0 ;
        cumpct =0.0;
        if (freq[0] > 0 ) colsum += freq[0];
        for (i = 1; i < (bkt->Num - 1); i++) colsum += freq[i];
        if (freq[bkt->Num - 1] > 0 ) colsum += freq[bkt->Num - 1];        
        dsum = (double) colsum/100.0 ;   // Pre-Mult by 100 

        /*Print the table */

            if (freq[0] > 0 ) {   // only print Low if not zero
               pct = (double)freq[0]/dsum ;
               cumpct += pct ; 
               printf ("Low\t%8d\t%7.2f\t%7.2f\n", freq[0], pct, cumpct );
            }
            for (i = 1; i < (bkt->Num - 1); i++) {
               pct = (double)freq[i]/dsum ;
               cumpct += pct ; 
               printf ("%5d\t%8d\t%7.2f\t%7.2f\n", bkt->Names[i], freq[i],pct, cumpct );
            }
            if (freq[bkt->Num - 1] > 0 ) {      // only print High if not zero
               pct = (double)freq[bkt->Num - 1]/dsum ;
               cumpct += pct;
               printf ("High\t%8d\t%7.2f\t%7.2f\n", freq[bkt->Num - 1], pct, cumpct  );
            }
            printf("    \t--------\nTotal\t%8d\t%7.2f\n", colsum, 100.0 ) ; 
            break ;
       case 'a' :                               // Never used in code?
          printf ("\nDescription %s:\n", descr );
          if (freq[0] > 0 ) printf ("|  Low|");
          for (i = 1; i < (bkt->Num - 1); i++) { printf ("%6d ", bkt->Names[i] ); }
          if (freq[(bkt->Num - 1)] > 0 )  printf ("|  High|");
          printf("\n");
          if (freq[0] > 0 )  printf ("%6d ", freq[0]);
          for(i=1; i<(bkt->Num - 1) ; i++ ) { printf("%6d ",freq[i] )  ; }
          if (freq[(bkt->Num - 1)] > 0 )  printf ("|%6d|", freq[(bkt->Num - 1)]);
          break ;
      } /* end switch direction */
      return(1);
} /* end show freq1D */

int fill_bkt_names(struct bucket_st *bkt ) {  // Used by both 1D and 2D. calloc allowed for two extra Names: High and Low
   int idx ;
   int uflow_idx, oflow_idx ;
   // bkt_id 0 is for underflow
   // bkt_id bkt.Num-1  is for overflow. Struct.Num is Data Buckets + 2 . Data goes from 1 to Num-2; Num-1 is Oflow.
   // 10 data buckets means bkts 1 .. 10 get real names.
   uflow_idx = 0 ;
   oflow_idx = bkt->Num - 1 ;
   int *pnames = bkt->Names ;
   for (idx = 1 ; idx <= (bkt->Num - 2)  ; idx++ ) {
      *(pnames + idx) =  bkt->Lo + (idx-1) * bkt->Sz;
   }

   *(pnames + uflow_idx) =  bkt->Lo - bkt->Sz; // Dummy name for the uflow area. Could be Misunderstood. Maybe fix.
   *(pnames + oflow_idx) =  bkt->Hi + bkt->Sz ; // Dummy name for the oflow area
   // with Lo,Hi,Sz = 2,8,2 we should get bkt_id= 0  1  2  3  4  5
   //                                     Name =  0  2  4  6  8  10
     #ifdef JGMDBG
        if(jgmDebug >= 4) {
         printf("FILL BKT NAMES Result for Bucket %c \n",bkt->Dir );
         for(idx=0; idx < bkt->Num ; idx++ ) {printf(" %4d",idx); }
         printf("\n");
         for(idx=0; idx < bkt->Num ; idx++ ) {printf(" %4d",bkt->Names[idx]); }
         printf("\n");
         //show_array(bkt->Num, bkt->Names) ;
      } /* end jgmDebug >= 4 */
    #endif
   return ( 1 ) ;
} /* end fill bkt names */

int *alloc_bkts2D ( struct bucket_st *d_bkt, struct bucket_st *a_bkt ) {
// alloc two vectors for the BktNames, and one 2d area for the Freq counters. bkt.Num is 2 more than data buckets.
   int *alloc_ptr ;
   alloc_ptr = calloc( (size_t)d_bkt->Num , sizeof(int) ) ;
   if (NULL == alloc_ptr ) {
      perror(" Cant calloc space for 2D BktNamesDown ");
      exit (-7);
   }
   track_alloc(alloc_ptr, "d_bkt_Names", d_bkt->Num * int_sz  ) ; 
   d_bkt->Names = alloc_ptr ;
   d_bkt->Names[0]              = d_bkt->Lo - d_bkt->Sz ; // Dummy init for uflow bkt name
   d_bkt->Names[d_bkt->Num - 1] = d_bkt->Hi + d_bkt->Sz ; // Dummy init for uflow bkt name

   alloc_ptr = calloc((size_t)a_bkt->Num , sizeof(int) ) ;
   if (NULL == alloc_ptr ) {
      perror(" Cant calloc space for 2D BktNamesAcross ");
      exit (-7);
   }
   track_alloc(alloc_ptr, "a_bkt_Names", a_bkt->Num * int_sz) ; 
   a_bkt->Names = alloc_ptr ;
   a_bkt->Names[0]              = a_bkt->Lo - a_bkt->Sz ; // Dummy init for uflow bkt name
   a_bkt->Names[a_bkt->Num - 1] = a_bkt->Hi + a_bkt->Sz ; // Dummy init for uflow bkt name

   alloc_ptr  = calloc( (size_t) (a_bkt->Num * d_bkt->Num ) , sizeof(int) ) ; // Both Nums already two extra.
   if (NULL ==  alloc_ptr ) {
      perror(" Cant calloc space for 2D Bkts Freq Counters ");
      exit (-7);
   }
   track_alloc(alloc_ptr, "2D_bkt_freq_ctrs", a_bkt->Num * d_bkt->Num * int_sz) ; 
   JGMDPRT(4,"ALLOC2D:: Freq2D ptr = %p , Num of Buckets=%d \n",
                       (void *)alloc_ptr, (a_bkt->Num * d_bkt->Num )  );
   return ( alloc_ptr ) ;  // freq2D set to this value on return
} /* end allocate 2D Ram */

int find_bkt_id(int val, struct bucket_st *bkt ) { /* used only in the 2D case */
    int bkt_id ;
    int uflow = 0 ;
    int oflow = bkt->Num - 1 ;
    if (val < bkt->Lo ) {
       JGMDPRT(6,"FIND_BKT_ID:: val=%d, bkt->Lo=%d, bkt->Sz= %d, bkt_id=%d \n", val, bkt->Lo, bkt->Sz, uflow ) ;
       return (uflow ) ;
    }  // uflow
    if (val >= bkt->Lo + ( oflow ) * bkt->Sz ) {
       JGMDPRT(6,"FIND_BKT_ID:: val=%d, bkt->Lo=%d, bkt->Sz= %d, bkt_id=%d \n", val, bkt->Lo, bkt->Sz, oflow ) ;
       return ( oflow ) ;
    } // oflow
    bkt_id = ( 1 + (val - bkt->Lo )/bkt->Sz );
    /* ex Lo=100, Hi=4100, Sz=400 ; bkt->Num =  1 + (4100 - 100)/400 = 11 */
    JGMDPRT(6,"FIND_BKT_ID:: val=%d, bkt->Lo=%d, bkt->Sz= %d, bkt_id=%d \n", val, bkt->Lo, bkt->Sz, bkt_id ) ;
    return (bkt_id) ;
} /* end find_bkt_id */

int upd_bkt2D(int val_dwn, int val_acr, int *freq2D, struct bucket_st *d, struct bucket_st *a) {
int offset2D ;
int d_bkt_id, a_bkt_id ;
   d_bkt_id = find_bkt_id(val_dwn, d) ;
   a_bkt_id = find_bkt_id(val_acr, a) ;
   offset2D =  (d_bkt_id * a->Num ) + a_bkt_id ; // freq2D[dwn][acr]
   (*(freq2D + offset2D))++ ;                  // update the correct cell in the tbl

   JGMDPRT(6,"UPD2D:: val_dwn=%d, d_bkt_id=%d, val_acr=%d, a_bkt_id=%d, offset2D=%d, NewCounter[d][a]=%d \n",
                     val_dwn, d_bkt_id, val_acr, a_bkt_id, offset2D, *(freq2D + offset2D) ) ;
   return offset2D ;  // return the offset to the updated counter.
} // end upd_bkt2D

int show_freq2D(int *freq2D, char *description, struct bucket_st *d_bkt, struct bucket_st *a_bkt ) {
   int r, c, oflow_row, oflow_col, row_g_tot, col_g_tot, ctr_val;
   int row_tot;
   int ctr_offset ;
   printf("\nDescription: %s \n", description ) ;
   int *col_totals ;
   char col_tot_descr[32];
   strncpy(col_tot_descr, description, 23 ) ; 

   oflow_col = a_bkt->Num - 1 ;
   oflow_row = d_bkt->Num - 1 ;
   row_g_tot = 0 ;  // keep both of these for cross check
   col_g_tot = 0 ;
   JGMDPRT(5,"Show Freq2d Calling: calloc nelem=%d Alloc Size=%ld \n", a_bkt->Num, sizeof(int)*a_bkt->Num ) ; 
   col_totals = (int *)calloc( (size_t)a_bkt->Num, sizeof(int) ) ;   /* cant calloc here; there is a BUG */
   if (NULL == col_totals ) { perror("show_freq2D cant allocate RAM for Column Totals"); return(-7) ; }
   JGMDPRT(5,"Show Freq2d malloc Returns ptr=%p nelem=%d Alloc Size=%ld \n",(void*)col_totals, a_bkt->Num, a_bkt->Num * sizeof(int) ) ; 
   
   track_alloc(col_totals, col_tot_descr, a_bkt->Num  * int_sz ) ;
   DBGDO(5,rpt_alloc( ) );
   // across is the number of columns, down is the number of rows.
   // Print the Heading line         Low      1      2      3     4   High    Sum
   printf("           Low");  // c == 0
   for (c = 1 ; c < oflow_col; c++ ) {
      printf(" %8d", a_bkt->Names[c] ) ;
      col_totals[c] = 0;  // not necessary since calloc has zeroed them. ?
   }
   printf("     High    Sum\n"); // c == oflow_col
   // print the rows of counters starting with the underflow row (0)
   for (r = 0 ; r <= oflow_row ;  r++ ) {
      if (0 == r ) { printf(" Low "); }                     // do the row name.
      else if ( oflow_row == r ) { printf("High "); }
      else { printf("%4d ",d_bkt->Names[r] ) ; }
      row_tot = 0;
      ctr_offset = (r * a_bkt->Num);
      for (c = 0 ; c <= oflow_col; c++ ) { // include uflow and oflow cols
         ctr_val = *(freq2D + ctr_offset) ;
         ctr_offset++ ;
         row_tot    += ctr_val ;
         col_totals[c] += ctr_val ;
         printf(" %8d",ctr_val ) ;
      } // end for c
      printf(" %8d\n", row_tot ) ;
      row_g_tot +=  row_tot ;
   } // end for r Done all rows except the col total row.
  printf("Sum  ");
  for (c = 0 ; c <= oflow_col ; c++ ) {
      printf(" %8d", col_totals[c] ) ;
      col_g_tot += col_totals[c] ;
   }
   printf(" %8d\n", col_g_tot );
   if (row_g_tot != col_g_tot ) {
      printf("\n** CANT HAPPEN: Row Grand Total[%d] is NOT EQUAL to Col Grand Total[%d] \n", row_g_tot, col_g_tot );
   }
   free((void*)col_totals) ;  /* free RAM per this local var; Ready for next call to showfreq2D */
   // JGMDPRT(7, "post free col_tot = %p \n",(void *)col_totals );
   col_totals = 0 ; /* free leaves ptr unchanged. don't want ptr accessing free'd RAM */
   return(1) ;
} /* end show_freq2D */



