/* JGM -- File Serverdebug_subs.c -- Routines for debugging output mostly */
/* Try not to include any functions that are defined elsewhere in dealer
 * so that including this one will not result in conflicts
 */
#include <assert.h>
#include <stdio.h>          /* for perror(), printf, fprintf */
#include <stdlib.h>         /* for srand48, mrand48 and friends */
#include <string.h>        /*strlen etc. also memcpy */
#include <unistd.h>        /* fsync, sysconf, read, write, */


#include "../include/dealdefs.h"
#include "../include/dealtypes.h"
#include "../include/UserEval_types.h"
#include "../include/dbgprt_macros.h"
#include "../include/Serverdbg_subs.h"

typedef char      CARD52_k ;       /* Dealerv2 card normal char with special type  */          
typedef CARD52_k  DEAL52_k[52] ;

extern int hascard(char deal[], int p,  char card ) ;
extern int jgmDebug ;
void show_curdeal(DEAL52_k d) { /* ouput deal in easy to read, verbose fmt; does not rely on deal being analyzed or sorted */
    int  pnum , cardcount;
    char *pname="NESW";
    char *sname="CDHS";
    char *rname="23456789TJQKA";
    int r, s;
    char sid ;
    pnum = 0 ;

    cardcount = 0;
    int crd ;
    char pn, rn ;
    pn='*'; rn='-';
    /* this is already a debugging routine; no need to use DBGPRT */
    fprintf(stderr, "Debugging Deal in dump_curdeal\n");
 //    for (pnum = COMPASS_NORTH; pnum <= COMPASS_NORTH; pnum++) {
    for (pnum = COMPASS_NORTH; pnum <= COMPASS_WEST; pnum++) {
        cardcount = 0;
        pn = pname[pnum];
        fprintf(stderr, "\ndump_curdeal Looping for Player number %d : %c ",pnum, pname[pnum]);
        fprintf (stderr,  " Player[%c ] \n", pname[pnum]);
        for (s = SPADES; s >= CLUBS; s--) {  /* s must be signed for this to work */
          assert(CLUBS <= s && s <= SPADES); /* if s is not signed this will not be true */
          sid = sname[s];
          fprintf(stderr, "Suit %2d=[%c] { ", s, sid);
          for (r = ACE; r >= TWO; r--) {     /* r goes from Ace to Deuce. r must be a signed int, not an unsigned */
              assert(TWO <= r && r<= ACE );  /* if r is not signed this will not be true */
              rn = rname[r];
              crd = MAKECARD (s, r);
              fprintf(stderr, "%02X => %d:%c ", crd, r, rn); // Comment out.
              if (hascard(d, pnum, crd)) {
                     fprintf(stderr, "%c%c ",sid, rn );
                  cardcount++ ;
              } /* end if hascard */
            } /* end for r = ACE */
            fprintf (stderr, "}\n");
        } /* end for s suit */
        fprintf(stderr, "Done for player%d=%c, found=%d\n", pnum, pn, cardcount);
        assert(cardcount == 13 );  /* must find 13 cards for each player */
  } /* end for player */
  fprintf(stderr, "----------------dump curr deal done ----------------\n");
  fsync(2) ;
} /* end show curr deal */

void hexdeal_show(DEAL52_k dl ) { /* one line output of the coded cards in hex */
    int i ;
    i = 0;
    fprintf (stderr, " dl=[");
    for (i=0; i<52 ; i++ ) {  /* print hex deal */
        fprintf(stderr, "%02x ", dl[i] );
    } /* end hex deal */
    fprintf(stderr, "/]\n");
    fsync(2);
} /* end hexdeal_show */

void sr_deal_show(DEAL52_k dl ) { /* two line output of the cards in SuitRank */
    char rns[] = "23456789TJQKA-";
    char sns[] ="CDHS";
    char rn, sn , sp;
    int s, r , i ;
    i = 0;
    fprintf (stderr," NOCARD=[%02x] Showing Deal using CARD_SUIT and CARD_RANK in sr_deal_show\n",NO_CARD);
    fprintf (stderr,"SR1=[");
   for (i=0; i<52 ; i++ ) {
       if (dl[i] == NO_CARD) { sn='-';rn='x' ; }
       else {
          s=CARD_SUIT(dl[i]); sn=sns[s];
          r=CARD_RANK(dl[i]); rn=rns[r];
       }
       sp = ((i+1)%13 == 0) ? ':' : ' ';
       fprintf(stderr,"%c%c%c", sn,rn,sp );
       if ( 25 == i ) fprintf(stderr, "\n     ");

    }
    fprintf (stderr,"]\n");
    fsync(2);
} /* end sr_deal_show */

void sr_hand_show(int p, DEAL52_k dl ) {  /* two line output of the cards in SuitRank */
    char rns[] = "23456789TJQKA";
    char sns[] ="CDHS";
    char rn, sn ;
    int s, r ,  di ;
    di = p*13 ;
    fprintf (stderr," Showing Hand  using CARD_SUIT and CARD_RANK in sr_hand_show\n");
    for (s=0; s<p+1 ; s++ ) {fprintf(stderr, "        "); }
    fprintf (stderr,"SR1=[");
    for (di = p*13 ; di < (p+1)*13 ; di++ ) {
             s=C_SUIT(dl[di]); sn=sns[s];
             r=C_RANK(dl[di]); rn=rns[r];
             fprintf (stderr,"%c%c ", sn,rn );
    }
    fprintf (stderr, "]\n");
    fsync(2);
} /* end sr_hand_show */

void show_query_type( struct query_type_st *pqt) {  /* call with DBGDO(n, .... ) */
 #ifdef JGMDBG
   DBGLOC( "Query Type:: Descr[%s], Tag=[%i]: [", pqt->query_descr, pqt->query_tag);
   for (int v=0 ; v<8; v++) {fprintf(stderr,"%d,", pqt->q_vals[v] ) ;  }
    fprintf(stderr, "]\n");
 #endif
   return ;
}
void show_reply_type(struct reply_type_st *prt) {
 #ifdef JGMDBG
   DBGLOC( "^^^Server SHOWReply_type:: Descr[%s], Tag=[%d]: Values[", prt->reply_descr, prt->reply_tag);
   for (int v=0 ; v<8; v++) {fprintf(stderr,"%d, ", prt->r_vals[v] );  }
   fprintf(stderr, "]\n");
 #endif
   return ;
}

int show_mmap(char *mm_ptr, int len ) {            /* verbose debug */
   char cbuff[128], xbuff[128];
   char *cptr=cbuff;
   char *xptr=xbuff;
   int i = -1 ;
 #ifdef JGMDBG
   sprintf(cbuff,"%4x:",0); /* starting offset */
   sprintf(xbuff,"    :");
   cptr = cbuff + 5 ;
   xptr = xbuff + 5 ;

   for (i = 1 ; i <= len ; i++, mm_ptr++ ) {
      sprintf(cptr, "%c  ", (*mm_ptr) ? *mm_ptr : '.' ) ; cptr += 3 ;
      sprintf(xptr, "%2x ",  *mm_ptr                  ) ; xptr += 3 ;
      if ( (i%32) == 0 && i > 0 ) {
         *cptr++ = ';'; *cptr++ = '\n'; *cptr = '\0';
         *xptr++ = ';'; *xptr++ = '\n'; *xptr = '\0';
         fprintf(stderr,"%s", cbuff);
         fprintf(stderr,"%s", xbuff);
         cptr = cbuff;
         xptr = xbuff;
         sprintf(cbuff,"%4x:",i);
         sprintf(xbuff,"    :");
         cptr = cbuff + 5 ;
         xptr = xbuff + 5 ;
      }
   } /* end for */
   --i ;
    if( (i%32) != 0 ) {    /* new line and new offset every 32 bytes */
         *cptr++='<';  *cptr++='\n'; *cptr='\0';
         *xptr++='^';  *xptr++='\n'; *xptr='\0';
         fprintf(stderr,"%s", cbuff) ;
         fprintf(stderr,"%s", xbuff) ;
   }
      //fprintf(stderr,"\n");
      fsync(2);
   #endif
  return (i) ;
}

void show1D_arr( int *arr, int NC ) {
			for (int nc=0; nc< NC; nc++ ) {
			fprintf(stderr,"%d ", arr[nc] );
		}
		fprintf(stderr,"\n");
	}
   
void show2D_arr( int *arr, int NR, int NC ) {
	int nr, nc ; 
	for (nr=0 ; nr< NR ; nr++ ) {
		fprintf(stderr,"NR=%d: ",nr);
		for (nc=0; nc< NC; nc++ ) {
			fprintf(stderr,"%d ", *(arr + nr*NC + nc) );
		}
		fprintf(stderr,"\n");
	} 
   return;
}	

void show_mmap_offs( struct mmap_off_st *offs) {
   fprintf(stderr," header_off  = %zd\n", offs->header  ) ;
   fprintf(stderr," query_off   = %zd\n", offs->query   ) ;
   fprintf(stderr," reply_off   = %zd\n", offs->reply   ) ;
   fprintf(stderr," dl_data_off = %zd\n", offs->dldata  ) ;
   fprintf(stderr," nsres_off   = %zd\n", offs->nsres   ) ;
   fprintf(stderr," ewres_off   = %zd\n", offs->ewres   ) ;
   fprintf(stderr," cache_off   = %zd\n", offs->cache   ) ;
}

void show_mmap_sizes( void ) {
   fprintf(stderr,"Size of map_template_k %zd\n", sizeof(MMAP_TEMPLATE_k  ));
   fprintf(stderr,"Size of mmap_hdr_k     %zd\n", sizeof(mmap_hdr_k       ));
   fprintf(stderr,"Size of query_data_k   %zd\n", sizeof(query_type_k     ));
   fprintf(stderr,"Size of reply_data_k   %zd\n", sizeof(reply_type_k     ));
   fprintf(stderr,"Size of USER_VALUES_k  %zd\n", sizeof(USER_VALUES_k    ));
   fprintf(stderr,"Size of UEVAL_CACHE_k  %zd\n", sizeof(UEVAL_CACHE_k    ));
   fprintf(stderr,"Size of DEALDATA_k     %zd\n", sizeof(DEALDATA_k       ));
   fprintf(stderr,"\tSize of Handstat_k   %zd\n", sizeof(HANDSTAT_k       ));
   fprintf(stderr,"\tSize of Sidestat_k   %zd\n", sizeof(SIDESTAT_k       ));
   
   return ;
}
void show_mmap_ptrs(char *p_mm, struct mmap_ptrs_st *ptrs ) {
   fprintf(stderr, "%s.%d show_mmap_ptrs\n",__FILE__,__LINE__ ) ;
   fprintf(stderr," Server Base Pointer: %p \n", (void *)p_mm    ) ;
   fprintf(stderr," \theader_ptr  = %p\n", (void *)ptrs->header  ) ;
   fprintf(stderr," \tquery_ptr   = %p\n", (void *)ptrs->query   ) ;
   fprintf(stderr," \treply_ptr   = %p\n", (void *)ptrs->reply   ) ;
   fprintf(stderr," \tdldata_ptr  = %p\n", (void *)ptrs->dldata  ) ;
   fprintf(stderr," \tnsres_ptr   = %p\n", (void *)ptrs->nsres   ) ;
   fprintf(stderr," \tewres_ptr   = %p\n", (void *)ptrs->ewres   ) ;
   fprintf(stderr," \tcache_ptr   = %p\n", (void *)ptrs->cache   ) ;
   fprintf(stderr," \tptrs.phs[0] = %p\n", (void *)ptrs->phs[0]  ) ;
   fprintf(stderr," \tptrs.phs[1] = %p\n", (void *)ptrs->phs[1]  ) ;
   fprintf(stderr," \tptrs.phs[2] = %p\n", (void *)ptrs->phs[2]  ) ;
   fprintf(stderr," \tptrs.phs[3] = %p\n", (void *)ptrs->phs[3]  ) ;
   fprintf(stderr," \tptrs.p_deal = %p\n", (void *)ptrs->p_deal  ) ;
   return ;

} /* end show_mmap_ptrs */





