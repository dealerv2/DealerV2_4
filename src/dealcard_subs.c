/* File dealcard_subs.c
 * Code related to dealing and shuffling the cards. Pulled from dealeval_subs when dealing re-organized
 * Date        Version  Author   Comment
 * 2023/01/06     1.0   JGM   Sorting deal to facilitate DDS functions broke the Predeal logic of previous code. Major rewrite needed
 */
#ifndef _GNU_SOURCE
  #define _GNU_SOURCE
#endif
#include "../include/std_hdrs.h"
#include "../include/dbgprt_macros.h"
#include "../include/libdealerV2.h"
#include "../include/dealdefs.h"
#include "../include/dealtypes.h"
#include "../include/dealexterns.h"
#include "../include/dealprotos.h"

#include "../include/dealdebug_subs.h"   /* for hexdeal_show which now needs a size parm */
#include "../include/dbgprt_macros.h"
#include "../include/dealutil_subs.h"    /* for  sorts, etc. */
#include "../include/deal_bias_subs.h"
void init_cards() ;

void swap2 (DEAL52_k  d, int p1, int p2) { /* Local keep for swapping */
  CARD52_k  t;
  int i;
  p1 *= 13;
  p2 *= 13;
  for (i = 0; i < 13; ++i) {
    t = d[p1 + i];
    d[p1 + i] = d[p2 + i];
    d[p2 + i] = t;
  }
} /* end swap2 */

/* put newly shuffled small_pack into curdeal which already has predeal cards in it from stacked_pack */
void merge_deal(DEAL52_k  d) {  /* local */
   int i = 0 ;
   int j = 0 ;
   for (i = 0 ; i < 52 ; i++) {
      if (d[i] == NO_CARD) {
          d[i] = small_pack[j++] ; /* the non-predealt cards can go in any free slot in curdeal */
          JGMDPRT(9,"Merging small_pack[%d]=%02x into curdeal[%d] \n",j, small_pack[j],i );
      }
   }
   assert( j == small_size ) ;              /* all of the leftover cards should have been copied to curdeal */
   return ;
}
/*
 * Generate next deal; shuffle, swap, stacked-shuffle, bias-shuffle, or read from Library.
 *
 * Called by dealerv2 main loop passing in DealMode and curdeal
 * Returns 1 (+ve number) on success, and -1 (-ve number) on failure.
 * As of 2023-12-10 the only possible failure is too many wraparounds of RP Library file or an IO err 
 */
int deal_cards(int dmode, DEAL52_k  d ) {  

   /* PreRun_ErrCheck() will have enforced one of the following mutually incompatible options
    *     1. Library Mode
    * 	 2. Bias Deal (predeal distribution)
    * 	 3. Predeal specific cards.
    * 	 4. Swapping
    */
   int rc = DL_OK; /*assume success */
   int lib_rc = DL_OK;
	switch (dmode) {
		case DEF_MODE : /* No zrdlib, no swap, no predeal, no bias */
		default :
			Shuffle(d, 52) ;
			JGMDPRT(8,"Vanilla Case Shuffle Done. \n");
			deal_err = DL_OK ; 
			return DL_OK;
			break  ; 
		case PREDEAL_MODE : {		/* Pre: stacked_pack and small_pack setup */
			memcpy(d , stacked_pack, 52 ) ;   /* initialize the deal with the predealt cards and NO_CARD otherwise */
			Shuffle(small_pack, small_size) ; /* Knuth algorithm on the remaining cards */
			merge_deal(d) ;                    /* merge the shuffled remaining cards into the curdeal setup by stacked_pack*/
			JGMDPRT(7,"Predeal Case Merge Deal Done. small_size=%d\n", small_size);
			deal_err = DL_OK ; 
			return DL_OK;
			break  ;
		}
		case SWAP_MODE : {
				/* Swapping should always be non zero here; do the swap  or generate a new random deal if swapindex == 0*/
			if ( swapindex != 0 ) {  //Swapindex gets set to zero after each swapping group so we gen a new random deal
				JGMDPRT(8,"Swapping=%d, Swapindex=%d \n",swapping, swapindex);
				switch (swapindex) {
				case 1:        swap2 (d, 1, 3);        break;
				case 2:        swap2 (d, 2, 3);        break;
				case 3:        swap2 (d, 1, 2);        break;
				case 4:        swap2 (d, 1, 3);        break;
				case 5:        swap2 (d, 2, 3);        break;
				}
			}
			else { /* swapindex == 0 so must shuffle a new random deal */
				JGMDPRT(8,"Swapping=%d, swapindex=%d (zero?) calling Shuffle for new random deal \n",swapping,swapindex ) ;
				Shuffle(d, 52) ; /* per Knuth algorithm */
			}
		   ++swapindex;  /* update the swapping status */
			if ((swapping == 2 && swapindex > 1) || (swapping == 3 && swapindex > 5)) {
				swapindex = 0;  /* reset to zero so we gen a new random deal next time */
			}
			deal_err = DL_OK ; 
			return DL_OK;
			break ;
	  } /* end swap mode */
	  case LIB_MODE : {
		  lib_rc = zrd_getdeal(fzrdlib, &options, d) ; /* sets curdeal and also the dds_res_bin 20 results struct */
			return lib_rc ; /* DL_OK or ENOZRD_DL if wrap count exceeded*/
			break ; 
	  }
	  case BIAS_MODE : {
		  rc = make_bias_deal(curdeal);
		  deal_err = rc ; 
		  return rc;		
		  break ; 
	  }
	
	} /* end switch(dmode) */
} /* end deal_cards */

/* setup_deal called by init_subs -> protos */
