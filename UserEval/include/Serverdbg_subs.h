#ifndef SERVER_DBG_SBUS
#define SERVER_DBG_SUBS
#include "../include/mmap_template.h"
typedef char      CARD52_k ;       /* Dealerv2 card normal char with special type  */          
typedef CARD52_k  DEAL52_k[52] ;

void show_reply_type(struct reply_type_st *prt) ;
void show_query_type( struct query_type_st *pqt) ;
void show_mmap_ptrs(char *p_mm, struct mmap_ptrs_st *ptrs ) ;
void show_mmap_sizes( void ) ;
void show_mmap_offs( struct mmap_off_st *offs) ;
int show_mmap(char *mm_ptr, int len ) ;  /* show mmap contents in hex and char; very verbose debug */


void show2D_arr( int *arr, int NR, int NC ) ;
void show1D_arr( int *arr, int NC ) ;

extern int hascard(char deal[], int p,  char card ) ; 
void sr_hand_show(int p, DEAL52_k dl ) ;
void sr_deal_show(DEAL52_k dl ) ;
void hexdeal_show(DEAL52_k dl ) ;
void show_curdeal(DEAL52_k d)  ;

#endif /* file guard */

