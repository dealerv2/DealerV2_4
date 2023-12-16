 /* File deal_bias_subs.c
 * Code related to dealing and shuffling the cards for doing biasdeals.
 * Date        Version  Author   Comment
 * 2023/11/01     1.0   JGM   
 */

/* A bias deal is more complicated than predealing a set of known cards
 * we are specifying not the cards but the exact number of cards in one or more suits.
 * And we don't do this just once at init time; we have to re-select random cards in the suits on every shuffle
 * (at least that's the way I interpret the spec); Would it matter if we just chose the bias cards at random
 * when the program started up and left them unchanged for the length of the run?
 * I don't know. So to play it safe do it this way. 
 * The bad part of this is that it is done on EVERY random deal, perhaps millions of them, not just on the 'interesting' deals.
 * And the shuffle is several times more complicated (and hence slower) than a normal shuffle or a normal predeal shuffle
 * So we can expect a significant impact on the runtimes of simulations using Bias Predeals
 * 
 * Future: allow mixing of Predeal and biasdeal? but not in the same suit? this could get very complicated.
 */
#ifndef JGMDBG
	#define JGMDBG 4
#endif
#ifndef _GNU_SOURCE
  #define _GNU_SOURCE
#endif
#include "../include/std_hdrs.h"
#include "../include/dealdefs.h"
#include "../include/dealtypes.h"
#include "../include/dealexterns.h"      /* included globals set by yyparse esp bias_suits[h][s] */
#include "../include/dealprotos.h"
#include "../include/dealdebug_subs.h"   /* for hexdeal_show which now needs a size parm */
#include "../include/dbgprt_macros.h"
#include "../include/libdealerV2.h"

#include "../include/deal_bias_subs.h"         /* define the API to the bias deal functionality */

/* File Globals Vars for the various Bias Deal functions */
int bias_beg_pos[4]  =  {0, 13, 26, 39};  /* the slots in a deck where the hand or suit begins   */
int bias_end_pos[4]  =  {13,26, 39, 52};  /* the slots in a deck where the hand or suit ends +1  */

/* after initial setup the following should never change */
int bias_suit_tot[4] = {0,0,0,0};			/* the total bias requirements in this suit <= 13 */
int bias_hand_len[4] = {0,0,0,0}; 		   // the number of bias cards that are to be assigned. Should not change during run.
int bias_tot_len     = 0; 						/* the total number of all bias cards in all bias hand/suits */
int bias_hand_cnt    = 0;						/* number of hands with bias suits in them */
int bias_hand_vp[4]  = {13, 13, 13, 13} ; /* vacant places to be filled after bias done. = 13 - bias_hand_len[] */
int bias_hand_TF[4]  = {FALSE, FALSE, FALSE, FALSE }; /* whether the hand has any bias suit requirements */

/* These next ones change as cards are assigned; need to be re-initialized on every deal */
int bias_suit_left[4]=  {13,13,13,13};    /* starts as 13 - bias_suit_tot[s] */
int bias_hand_xs[4]  =  {39,39,39,39};     /* the amount by which the vpop exceeds the vp */

DEAL52_k bias_src  ;    /* initialized from ascending pack */
DEAL52_k bias_pack ;		/* init NO_CARD; add bias cards; fill rest with cards from shuffled small_pack */
int  small_pack_sz ;
int  bias_pack_sz  ;

/* functions */
int bias_vpop(int h ) {  /* the total number of cards from which to draw to fill hand h -- depends on his bias suits */
	int s ;
	int t = 0;  
	for(s=0; s<4; s++ ) {
			if(-1 == bias_suits[h][s] ) {
				t += bias_suit_left[s] ;
			}
	}
	return t ; 
} /* end bias_vpop */

/* for an un-processed bias hand, return the diff between avail and required cards */
int bias_excess(int h ) {  
	if( FALSE == bias_hand_TF[h] ) return BIG_XS ; 
	if( NO_XS == bias_hand_xs[h] ) return NO_XS  ;
	return (bias_vpop(h) - bias_hand_vp[h] ) ;
}

int min_idx(int sz, int *val ) {
	int i = 0 ; 
	int t = 999 ;  /* impossibly big value */
	for (int j = 0 ; j < sz ; j++ ) { 
		if (t > val[j] ) {
			t = val[j] ;
			i = j ; 
		}
	} /* end for j */
	JGMDPRT(9,"Min_idx sz=%d, val[0]=%d, Min val[%d]=%d\n",sz,val[0],i,val[i]); 
	return i ; /* the index of the smallest item in the array */
} /* end min_idx */

int 	 		choose_bias_cards( DEAL52_k dst, DEAL52_k src ) ;
CARD52_k 	choose_nobias_card(int h, int beg_idx, DEAL52_k small_pack ) ;
void 	 		re_init_bias_vars() ;
int  	 		fill_bias_hands(DEAL52_k bias_pack, DEAL52_k small_pack ) ;
int 			fill_unbias_hands(DEAL52_k bias_pack, DEAL52_k small_pack ) ;
	
void 			exg_cards(int j, int beg_idx, int exg_gap, DEAL52_k small_pack   ) ; 
void 			exg_cards(int j, int beg_idx, int exg_gap, DEAL52_k small_pack   ) {
	if ( j == beg_idx ) {  
			JGMDPRT(9,"Card=%02x at position %d chosen. No EXG \n",small_pack[j], j ) ;
			return ; 
	} 
		/* the card currently at start was not chosen, so move it somewhere 
		 * we want to move it more than one slot if possible so we don't run into the same issue next time
		 */
	if( beg_idx < (small_pack_sz - exg_gap) ) {
			JGMDPRT(9,"Mov Card=%02x @ %d to %d : Move Card%02x @ %d to %d \n",
						small_pack[beg_idx+exg_gap],(beg_idx+exg_gap),j,small_pack[beg_idx],beg_idx,(beg_idx+exg_gap) ) ;
		  	small_pack[j] = small_pack[beg_idx+exg_gap] ; 			/* save kard at move target to slot that was chosen */
			small_pack[beg_idx+exg_gap] = small_pack[beg_idx] ;	/* save the rejected card at the move target slot */ 
	}
	else {  /* moving exg_gap positions would exceed limits of deck. just exg with the slot where card was taken */
	      JGMDPRT(9,"Move Card=%02x @ %d to %d \n",	small_pack[beg_idx],beg_idx,j) ; 
		   small_pack[j] = small_pack[beg_idx]  ; /* save rejected card in slot of card that was chosen */
	   }
} /* end exg_cards */

int make_bias_deal(DEAL52_k curdeal) { /* the Main routine in this file */
	 int i;
    int rc ; 
    int tmp_sz = 0 ;  
     
/*
 * 1) reset the vars for this deal incl bias_src, dealt_cnt etc. 
 * 2) randomly choose_bias_cards from deck bias_src and put into bias_pack; by shuffling suit that has bias requirements 
 * 3) copy left over bias_src to small_pack; assert small_pack_size + bias_len(0 ..3) == 52
 * 4) shuffle the small_pack
 * 5) fill bias_pack from shuffled small_pack and copy to curdeal
*/
	 JGMDPRT(7, "In Make Bias Deal. Calling re_init_bias_vars\n");
    re_init_bias_vars() ;  /* this re-inits the necessary ones. Not the same as initial setup. most vars are globals */
    JGMDPRT(8,"Calling choose_bias_cards \n");
    bias_pack_sz = choose_bias_cards(bias_pack, bias_src) ; /* satisfy the bias conditions by putting cards into bias_pack */
	 
	 tmp_sz = pack_size(bias_src) ; 
	 JGMDPRT(8, "choose_bias_cards returns bias_pack_sz =%d, bias_src_sz=%d\n",bias_pack_sz, tmp_sz );
	
    assert(bias_pack_sz == (bias_len(0) + bias_len(1) + bias_len(2) + bias_len(3)) );
    JGMDPRT(9, "ChooseBiasCards OK. assert bias_pack_sz passes. %d\n\tShowing Remaining Src then Bias Pack\n",bias_pack_sz ) ;
    DBGDO(9,hexdeal_show(bias_src, 52) ); 
    DBGDO(9,hexdeal_show(bias_pack,52) ); 
 #ifdef JGMDBG   
    /* this next bit just for debugging */
    int bias_src_sz = 0 ;
    int bias_src_taken = 0; 
     for (i=0; i < 52 ; i++ ) {
		  if (NO_CARD != bias_src[i] ) bias_src_sz++ ; 
		  else bias_src_taken ++ ; 
	  }
    JGMDPRT(8, "There are [ %d ] cards taken and [%d] cards remaining in bias_src\n",bias_src_taken, bias_src_sz );
#endif
/* copy cards that were not taken by choose_bias to small_pack */
	small_pack_sz = 0 ;        
  for (i=0 ; i < 52 ; i++ ) {			
	  if (bias_src[i] != NO_CARD ) { small_pack[small_pack_sz++] = bias_src[i] ;}
  }
  JGMDPRT(8, "Small Pack Size=%d, bias_pack_sz = %d , bias_src_size=%d, bias_src_taken=%d\n",
						small_pack_sz, bias_pack_sz, bias_src_sz, bias_src_taken ) ; 
  assert(bias_pack_sz + small_pack_sz == 52);
  JGMDPRT(8,"Choose Bias Cards Done. Calling Shuffle on small_pack, bias_pack_sz=%d, small_pack_sz=%d\n",bias_pack_sz,small_pack_sz );
  Shuffle(small_pack, small_pack_sz) ;   /* needs libdealerV2.a */	
  JGMDPRT(8, "Small Pack Shuffled -- size=%d\n Showing small_pack before calling fill_bias_hands",small_pack_sz);
  DBGDO(8, sr_deal_show(small_pack) ); 
  /* Fill out the hands having bias suits, with cards in unbiased suits */
  rc = fill_bias_hands(bias_pack, small_pack ) ; /* returns number of cards in bias_pack or -ve number if ERR*/
  JGMDPRT(8,"Done Filling Hands with Bias. RC=%d, bias_pack_sz=%d bias_hand_cnt=%d\n",rc, bias_pack_sz, bias_hand_cnt) ;
  if (rc < 0 ) {deal_err = DL_ERR_BIAS ; return DL_ERR_BIAS ; } /* discard this deal if we cant satisfy bias requirements */ 
  bias_pack_sz = rc  ; 
  assert( bias_pack_sz == bias_hand_cnt * 13 ) ; /* all biashands now have 13 cards each */

   /* Filled Bias hands with bias and non-bias cards. Fill the other hands */
   compress_pack((char *)small_pack,52) ;   
   rc = fill_unbias_hands(bias_pack, small_pack) ;
   JGMDPRT(8,"fill_unbiased_hands done. bias_pack_sz=%d, rc=%d\n",bias_pack_sz, rc );
   assert( rc == 52 ) ;  
   
   DBGDO(8, hexdeal_show(bias_pack, 52) );
	
   memcpy(curdeal, bias_pack, 52*sizeof(CARD52_k) ) ; /* curdeal now a random deal with bias cards */
   DBGDO(9, hexdeal_show(curdeal, 52) );  
   DBGDO(9, sr_deal_show(bias_pack) ); 
   JGMDPRT(8,"Succesful Bias Deal for ngen=%d, nprod=%d \n",ngen,nprod);
   deal_err = DL_OK ;  /* deal was successful */
   return DL_OK;   
	
}
void re_init_bias_vars() { /* do this for every deal; use setup_bias_vars for stuff that is done only once at beg of run */
	int h, s; 
	// DEAL52_k bias_src;
	memcpy(bias_src, asc_pack, 52) ; 	
	memset(small_pack,NO_CARD, 52) ; 
	memset(bias_pack, NO_CARD, 52) ; 	/*set all the bias_pack to NO_CARD need this for fill_bias_pack to work*/
	bias_pack_sz = 0 ;
	
	for (s=0; s<4; s++ ) {
		bias_suit_left[s] = 13 - bias_suit_tot[s];
		JGMDPRT(9,"ReInitBiasVars : Suit=%d, BiasTot=%d, Left=%d\n", s,bias_suit_tot[s], bias_suit_left[s] ) ; 
	}
	for (h=0; h<4; h++ ) {
		if(bias_hand_TF[h] == TRUE ) {  /* false ones fixed at BIG_XS */
			bias_hand_xs[h] = 0 ;      /* routine bias_excess does not update if > 52 */
			bias_hand_xs[h] = bias_excess(h) ;
		}
		JGMDPRT(9,"ReInitBiasVars : Hand=%d, vp=%d, vpop=%d, xs=%d, \n",
											h,bias_hand_vp[h], bias_vpop(h), bias_hand_xs[h]);											
	} /* end for h */
	
	JGMDPRT(8,"re_init_bias_vars Done. bias_src[51]=%02x, bias_pack[0]=%02x\n",bias_src[51],bias_pack[0]);
}		/* end re-init bias vars */
	
/* select 'n' random cards of the correct suit into the correct hand from a fresh pack 
 * Post: all required bias cards are in bias deck, and removed from bias_src
 *       also bias_deck_sz equal number of 'real' cards in bias_deck
 * 		hence bias_hand_len[h] should point to the first empty slot past the beg of hand[h]
 */
int choose_bias_cards( DEAL52_k dst, DEAL52_k bias_src ) { /* uses the global variables bias_suits[h][s] */
	int h,s,l;
	int dst_pos;  
	int src_pos ;
	int hand_beg_pos[4] = {0,13,26,39};
	int bias_slen;
	int dealt_cnt[4][4] = {0};   
	bias_pack_sz = 0 ; 
	for(s=SUIT_CLUB; s<=SUIT_SPADE; s++ ) {
		bias_slen = bias_totsuit(s);
		if(0 >= bias_slen ) continue ;
		JGMDPRT(8, "Suit %d has bias requirement=%d CShuffling Suit @src_pos=%d Size=13\n",s,bias_slen,src_pos ); 
		src_pos = s * 13 ; 
		Shuffle(&bias_src[src_pos],13) ;  /* Knuth Shuffle the suit */
		DBGDO(8,hexdeal_show(bias_src,52)) ;

		for(h=COMPASS_NORTH; h<=COMPASS_WEST; h++ ) {
			if(0 >= bias_suits[h][s] ) continue ;
			dst_pos = hand_beg_pos[h];           /* begin where we left off after previous suit */
			assert(NO_CARD == bias_pack[dst_pos]); /* the starting point in this hand should be unoccupied */
			for (l=0; l< bias_suits[h][s] ; l++ ) {
				bias_pack[dst_pos++] = bias_src[src_pos] ; 
				bias_src[src_pos++]  = NO_CARD ;
				bias_pack_sz++;
				dealt_cnt[h][s]++ ; 
				JGMDPRT(9,"Card=%02x given to hand=%d at dst_pos=%d from src_pos=%d bias_pack_sz=%d\n",
				      bias_pack[dst_pos - 1],    h,     (dst_pos -1),    (src_pos -1 ), bias_pack_sz );
			} /* end for l */
			hand_beg_pos[h] = dst_pos ;              /* save where the next suit will begin */
			assert(bias_suits[h][s] == dealt_cnt[h][s] ) ; /* this will fail if we have -1 compared to 0 */
		} /* end for h */
		
	} /* end for suit */
	assert (bias_pack_sz == bias_tot_len ) ; 
   return bias_pack_sz ; 
} /* end choose_bias_cards */

CARD52_k choose_nobias_card(int h, int beg_idx, DEAL52_k small_pack ) {
	/*return a card that does not violate bias contraints from shuffled small_pack; update small_pack beg_pos */
	CARD52_k kard;
	int j;
	int s;
	int exg_gap = 5;   // if a card rejected, move it by a gap so it does not keep getting selected next time.
	j = beg_idx-1;
	do {				/* find a real card that is NOT in a suit with bias constraints */
			j++ ; 
			while(NO_CARD == small_pack[j] && j < small_pack_sz ) { j++ ; } /* skip over NO_CARD slots; should never be any above beg_idx */
			if (j >= small_pack_sz) {  /* FAIL */
				JGMDPRT(8, "Cant Meet Bias Req at: %s [%d] : No valid small_pack card j=%d >= small_pack_sz[%d]\n",__FILE__,__LINE__,j,small_pack_sz);
				JGMDPRT(8, "Rejecting NGEN=%d with NPROD=%d. Trying again. \n",ngen,nprod);
				return NO_CARD ; 
			}
			kard = small_pack[j] ; 
			s    = CARD_SUIT( kard ) ; /* since kard is never NO_CARD here, then 's' is always 0..3 */
	} while (bias_suits[h][s] != -1 ) ; /* keep looking until we find a card in a suit with no bias requirements. */

	exg_cards(j, beg_idx, exg_gap, small_pack ) ; /* if card at beg_idx not chosen, move it somewhere safe */

	small_pack[beg_idx] = NO_CARD  ;       /* mark beg_idx slot as taken. DO..WHILE skips NO_CARD slots */
	DBGDO(9, hexdeal_show(small_pack, small_pack_sz) ); 
	return kard ; 
} /* end choose_nobias_card */

/* After the bias requirements are met 
 * fill the rest of the deal with the cards remaining in the small_pack
 * To minimize the chance of collisions or failling fill the hands that have the most
 * constraints first, then the hands that have least or no constraints
 * 
 * Returns size of bias pack (should be 52) or -ve number if could not meet bias requirements 
 */
int fill_bias_hands(DEAL52_k bias_pack,	DEAL52_k small_pack ) {
	  /* Fill out the hands having bias suits, with cards from unbiased suits */
     /* The one with the tightest restrictions first. */
	int h, i, j;
	int dst_pos ;
	int src_pos = 0 ;			/* start at beginning of shuffled small_pack; top positions in small pack are NO_CARD */
	CARD52_k kard ; 
	JGMDPRT(8, "Filling Bias Hands. bias_pack_sz=%d, small_pack_sz=%d\n",bias_pack_sz, small_pack_sz ) ; 
	DBGDO(9, hexdeal_show(small_pack, 52 ) ); 
	DBGDO(9, hexdeal_show(bias_pack , 52 ) ); 
	#ifdef JGMDBG
	for (i=0; i<4; i++ ) {   // debugging loop;
		if(TRUE == bias_hand_TF[i] ) {
			dst_pos = bias_hand_len[i] + bias_beg_pos[i];  /* bias_hand_len does not change during run */
			JGMDPRT(9,  "FILL_BIAS: Hand=%d, bh_len=%d, dst_pos=%d, card@dst_pos=%02x BiasXS[i]=%d\n",
									i,   bias_hand_len[i],dst_pos,  bias_pack[dst_pos], bias_hand_xs[i] );
		}
	} 
	#endif
	
	for (i=0; i<4; i++ ) {
		if(FALSE == bias_hand_TF[i] ) { continue ; } /* bypass unbiased hands for now */
		h = min_idx( 4, &bias_hand_xs[0] ) ;     /* h is hand with least flexibility */
		dst_pos = bias_beg_pos[h] + bias_hand_len[h] ; /* start past the bias cards in hand h*/
		JGMDPRT(9,"Fill_bias_hands i=%d, h=%d, bias_len[h]=%d, XS[h]=%d, dst_pos=%d, kard=%02X\n",
									i,h,bias_hand_len[h],bias_hand_xs[h],dst_pos,bias_pack[dst_pos] );
		assert(bias_pack[dst_pos] == NO_CARD);         /* there should be an empty slot here */
		while( dst_pos < bias_end_pos[h] ) { 			  /* fill the current hand with cards from small_pack */
			assert(src_pos < small_pack_sz ) ; 
			kard = choose_nobias_card(h , src_pos, small_pack ) ;
			JGMDPRT(9, "CHOSE kard=%02x for Hand[%d] DstPos[%d] src_pos=%d \n", kard, h, dst_pos, src_pos ) ;
			if (NO_CARD == kard ) {  /* could not meet bias requirements */
				deal_err = DL_ERR_BIAS ;
				return DL_ERR_BIAS ;
			} 
			bias_suit_left[CARD_SUIT(kard)]-- ; /* reduce the inventory ; will affect other hands' XS */
			bias_pack[dst_pos++] = kard ;
			bias_pack_sz++ ; 
			assert(small_pack[src_pos] == NO_CARD ) ; /* temporary chosecard puts NO_CARD in lowest spot chosen */
			src_pos++ ;									
		}  /* hand h filled; bias_suit_left[s] updated; so recalc the xs for the remaining hands */
		assert(52 == bias_pack_sz + (small_pack_sz - src_pos) ) ;
		bias_hand_xs[h] = NO_XS ; /* large value so it never comes back as the min one */
		for (j=0; j<4 ; j++) {	if (bias_hand_xs[j] < 52)  bias_hand_xs[j] = bias_excess(j) ; }
		JGMDPRT(9,"FillBiasHand[%d] Done. bias_pack_sz=%d, small_sz=%d, src_pos=%d, New XS=[%d,%d,%d,%d] \n",
							h, bias_pack_sz, small_pack_sz, src_pos,
							bias_hand_xs[0],bias_hand_xs[1],bias_hand_xs[2],bias_hand_xs[3]);
		JGMDPRT(9,"Bias Pack, then small_pack src_pos=%d, kard@src_pos=%02X,bias_sz=%d vvv\n",
						src_pos,small_pack[src_pos], bias_pack_sz);			
		DBGDO(9, hexdeal_show(bias_pack,52) ); 
		DBGDO(9, hexdeal_show(small_pack, small_pack_sz ) );
		if(i < 2 ) {Shuffle(&small_pack[src_pos],(small_pack_sz - src_pos ) ) ;} /* cards remaining in small_pack = (small_pack_sz - src_pos) */
		DBGDO(9, hexdeal_show(small_pack,52) );
	} /* end for i < 4 */ 
	JGMDPRT(8, "Fill Bias Hands Done. BiasPackSz=%d Bias_hand_cnt=%d\n",bias_pack_sz, bias_hand_cnt);
	return bias_pack_sz;
} /* end fill_bias_hands */

void init_bias_deal() { /* complete the once per run setup of bias vars */
	/* some of these inits are re-calculated in re_init_bias_vars but are needed here anyway */
	int h, s;
	newdeck(asc_pack,'A');
	// tst_shuffle(asc_pack) ; 
	// newdeck(asc_pack,'A');
	
	for (s=0 ; s < 4 ; s++ ) {
		bias_suit_tot[s] = bias_totsuit(s) ; 
		bias_suit_left[s]=13 - bias_suit_tot[s] ; 
   }
   bias_tot_len = 0 ; 
	for (h=0; h < 4 ; h++ ) {
		if(bias_suits[h][0] < 0 && bias_suits[h][1] < 0 && bias_suits[h][2] < 0 && bias_suits[h][3] < 0 ) {
			bias_hand_TF[h] = FALSE ; 
			bias_hand_len[h] = 0 ;
			bias_hand_vp[h] = 13 ;
			bias_hand_xs[h] = BIG_XS ;  
		}
		else { 
			bias_hand_TF[h] = TRUE ;  
			bias_hand_len[h] = bias_len(h);  /* bias_len == 0 could be unbiased, or could be requiring void */
			bias_tot_len += bias_hand_len[h];
			bias_hand_vp[h] = 13 - bias_hand_len[h] ;  /* vacant_places aka non-bias slots in h */
			bias_hand_xs[h] = bias_excess(h) ;			 /* measure of difficulty in filling hand. lower=harder*/
			bias_hand_cnt++ ; 
		}
		JGMDPRT(9,"Init Bias done for hand=%d, bias_hand_len=%d, XS=%d count hands with bias=%d\n",
						h,bias_hand_len[h], bias_hand_xs[h],bias_hand_cnt );
  }
  JGMDPRT(8, "Init Bias Deal done. bias_tot_len=%d, bias_hand_cnt=%d \n", bias_tot_len, bias_hand_cnt );
}  /* end init_bias_deal -- done once per run as part of finalize options */			

	
int fill_unbias_hands(DEAL52_k bias_pack, DEAL52_k small_pack ) {
	int h, rc ;
	int dst_pos = 0 ; 
	int src_pos = 0 ; 
	CARD52_k kard   ;  
	small_pack_sz = pack_size(small_pack); /* account for cards taken by fill_bias_hands */
	Shuffle(small_pack, small_pack_sz) ; 
	JGMDPRT(9,"fill %d unbiased hands from shuflled small_pack of size=%d\n\t Showing Small Pack...\n",
					(4 - bias_hand_cnt), small_pack_sz  );
	DBGDO(9, hexdeal_show(small_pack, 52 ) ); 
	
	for (h=0; h<4; h++ ) {
		if(bias_hand_TF[h] == TRUE ) continue ; /* bias hands are done already. bypass them */
		dst_pos = bias_beg_pos[h]  ; /* start pos */
		JGMDPRT(9,"Fill unbiased hand h=%d, bias_hand_len[h]=%d,bias_hand_xs[h]=%d, dst_pos=%d, kard=%02X\n",
									h,bias_hand_len[h],bias_hand_xs[h],dst_pos,bias_pack[dst_pos] );
		assert(bias_pack[dst_pos] == NO_CARD);         /* there should be an empty slot here */
		while( dst_pos < bias_end_pos[h] ) { 			  /* fill the current hand with cards from small_pack */
			assert(src_pos < small_pack_sz ) ; 
			kard = small_pack[src_pos];              /* unbiased hands just get next shuffled card */
			JGMDPRT(9, "CHOSE kard=%02x for Hand[%d] DstPos[%d] src_pos=%d \n", kard, h, dst_pos, src_pos ) ;
			if (NO_CARD == kard ) {  /* Cant happen. something wrong with small_pack */
				deal_err = DL_ERR_FATAL ;
				fprintf(stderr, "ERR** at %s:%d Fatal Error. Corrupted small_pack  at src_pos=%d\n",
										__FILE__,__LINE__,src_pos );
				assert( 0 ) ;
			} 
			bias_pack[dst_pos++] = kard ;
			bias_pack_sz++ ; 
			src_pos++ ;									/* just take the shuffled cards in any convenient order */
		}
		/* hand h filled; */
		assert(52 == bias_pack_sz + (small_pack_sz - src_pos) ) ;
		JGMDPRT(8,"FillBiasHand[%d] Done. Showing bias_pack......\n",h );
		DBGDO(8, hexdeal_show(bias_pack,52) );
	
	} /* end for h<4 */
	rc = bias_pack_sz;
	return rc ; 
} /* end fill_unbias_hands */
		 
		
	   
					
	
