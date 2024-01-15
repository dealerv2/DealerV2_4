/* File deal_conversion_subs.c -- several routines to convert between different deal representations */
/* PBN style deals can refer to one of GIB style which includes  all 4 compass directions  
 * 									 or DDS style which must include only the first compass direction 
 * 2023/11/05	1.0		JGM		Adapted from scratchpad to be part of the library 
 * 
 */

#ifndef _GNU_SOURCE
   #define _GNU_SOURCE
#endif


#define C_Suit(c)    (   ((c)>>4)&0xF     )
#define C_Rank(c)    (   ((c)&0x0F)       )

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "../include/dealdefs.h"
#include "../include/dealtypes.h"
#include "../include/dbgprt_macros.h"
#include "../include/dealutil_subs.h"    	// for dsort13 
#include "../include/deal_conversion_subs.h"
#include "../include/libdealerV2.h"

  /* All functions in this file rely on the Hand or Deal being sorted SA to C2
   * In Dealer this is done as the first step in the 'action' functions so need not be done here.
   * The one exception is formatting a deal for use by the OPC/DOP external script by the functions
   *     ^^^^^^^^^^^^^^^
   * fmtSide_to_cmdbuff, and fmtHand52_to_cmdbuff. which can be called as part of which is done in the condition clause
   * before the deal is sorted. Therefore These functions need to sort the deal/hand to work properly.
   */
   
	/* Global Defs for this File -- Cannot conflict with defs in dealglobals.c/dealexterns.h */
   extern int jgmDebug ;
   int side_hand[2][2] = { {0,2},{1,3} };  /* side 0 aka NS has hands 0 and 2; side 1 aka EW has hands 1 and 3 */
   char seatids[4] = {'N','E','S','W'};
   char suitids[4] = {'C','D','H','S'};
   char rankids[13]= {'2','3','4','5','6','7','8','9','T','J','Q','K','A'};


void init_export_buff( char *bp , size_t buff_len) {
     memset(bp, '\0', buff_len ) ;
}

char *Hand52_export(char *buff, int p, DEAL52_k dl ) {
/* hand is sorted already. dl[p*13+0] = Highest Spade; dl[p*13+12] = lowest club. */
/* Output to look like -N Sxxxx,Hxxxx,Dxx,Cxxx ; no need to put anything for void suits. */
   char suit_sep = ',';
   int curr_suit, card_rank, card_suit;
   int di, count;
   unsigned char kard ;
   char *bp ;
   bp = buff ;
   JGMDPRT(8, "Hand52_export Start:: bp=%p \n",bp ) ;
   di = p*13 ;
   count = 0 ;
   curr_suit = 3 ; // spades
#ifdef JGMDBG
   if (jgmDebug >= 8 ) {
       fprintf(stderr, "Hand52_Export:: p=%d, di=%d, dl[di]=%02x\n",p,di,dl[di] ) ;
       fprintf(stderr, "buff=%p, bp=%p \n", buff, bp );
       }
#endif
   *bp++ = '-'; *bp++ = seatids[p] ; *bp++ = ' ';
   while (count < 13 ) {
       kard = dl[di] ; card_suit = C_SUIT(kard); card_rank = C_RANK(kard) ;
        JGMDPRT(8, "Hand52_Export While 13::Kard=%02x, card_suit=%d, card_rank=%d, count=%d, curr_suit=%d\n",
                                  kard, card_suit, card_rank, count, curr_suit ) ;
       while( curr_suit != card_suit ) curr_suit-- ;
        assert(card_suit == curr_suit) ;
        *bp++ = suitids[card_suit];  /* Write the suit Letter */
        while ( (curr_suit == card_suit) && (count < 13) ) { /* write the cards in this suit */
            kard = dl[di]; card_suit = C_SUIT(kard); card_rank = C_RANK(kard) ;
            JGMDPRT(8,"Hand52_Export While suit::Kard=%02x, card_suit=%d, card_rank=%d, count=%d, curr_suit=%d\n",
                                        kard, card_suit, card_rank, count, curr_suit ) ;
            if (curr_suit != card_suit ) break;
           *bp++ = rankids[card_rank];
           count++; di++;
           JGMDPRT(8," Num[%d]=%c%c ", count, "CDHS"[curr_suit], *(bp-1) );
        } /* end while curr_suit == card_suit */
        JGMDPRT(8,"\n");
       *bp++ = suit_sep;
        curr_suit-- ; /* Move to next suit */
    } /* end while count < 13 */
    assert(count == 13 ) ;
     /* no need to write anything for voids here. wrote 13 cards that's it */
        /* the last char is the suit separator which we don't need after the club suit, so replace it with a space */
        if ( *(bp-1) == suit_sep ) { *(bp-1) = ' ' ; }
        else { fprintf(stderr, "CANT HAPPEN in Hand52_to_Predeal, last char is not a suit_separator %c \n", *(bp-1) ); }
        *bp = '\0' ; // terminate the buffer as a string
        return bp  ; /* return pointer to null byte in case we want to append another hand to the buffer */
} /* end Hand52_export*/

char *Side52_export(char *buff, int side, DEAL52_k dl ) {
   char *pdbp = buff ;
   init_export_buff(buff, sizeof(export_buff)) ;
   JGMDPRT(8,"Export Side=%d start for Hand=%d\n",side, side_hand[side][0]);
   pdbp = Hand52_export( pdbp, side_hand[side][0], dl ) ;
   *pdbp++ = ' ';
   JGMDPRT(8,"Export Side=%d start for Hand=%d\n",side, side_hand[side][1]);
   pdbp = Hand52_export( pdbp, side_hand[side][1], dl ) ;
   JGMDPRT(8,"Export Side[%d] Result:[%s]\n------------------------------------------\n", side, buff );
   return pdbp ;
} /* end Side52_export */

#define DEALSTRSIZE 76
int PBN_to_Deal52( DEAL52_k dl , char *t ) { /* convert a simple GIB-PBN text string ( printoneline fmt )  to Deal52_k */
    size_t slen ;
    int dlcnt ;
    int tc , seat ;
    enum rank_ek r;
    enum suit_ek s;
    int endofdeal = 0;
    // char *exdl = "n T952.842.A98.432 e KQJ.A3.KQJT2.AKT s 64.KJT7.75.QJ875 w A873.Q965.643.96:"; // example input
    /* get a text string such as:
     * n T952.842.A98.432 e KQJ.A3.KQJT2.AKT s 64.KJT7.75.QJ875 w A873.Q965.643.96:
     * and convert it into a deal.
     * The input string should have exactly (2+13+4)*4 = 76 chars. and should include all 4 compass directions in the string
     */
     slen = strlen(t) ;
     if (slen != DEALSTRSIZE ) {
          fprintf(stderr, "The source PBN deal is in the wrong format.\n Submit PBN deal as a COLON terminated %d char string \n",
                          DEALSTRSIZE );
     }
     s=0; r=0; dlcnt = 0;
     for ( tc=0; tc<slen; tc++) {
         if(endofdeal == 1 ) { break ; } /* ignore extra chars at end of deal. should never be any */
         switch (t[tc]) {
             case 'n' : s=3 ; seat = 0 ; dlcnt = seat*13; break ;/* player 0 is north ; reset suit to spades */
             case 'e' : s=3 ; seat = 1 ; dlcnt = seat*13; break ;
             case 's' : s=3 ; seat = 2 ; dlcnt = seat*13; break ;
             case 'w' : s=3 ; seat = 3 ; dlcnt = seat*13; break ;
             case ' ' : break ;
             case '.' : --s   ; break ; /* marks the end of a suit. decr to next lower suit */
             /* I could group next cases with statements like:
              *  r =  t[tc] - '0' and r = t[tc] -'A' + 10
              * but I like the way this looks :)!
              */
             case 'A' : r = ACE;   dl[dlcnt++] = MAKECARD(s,r) ; break ;
             case 'K' : r = KING;  dl[dlcnt++] = MAKECARD(s,r) ; break ;
             case 'Q' : r = QUEEN; dl[dlcnt++] = MAKECARD(s,r) ; break ;
             case 'J' : r = JACK;  dl[dlcnt++] = MAKECARD(s,r) ; break ;
             case 'T' : r = TEN;   dl[dlcnt++] = MAKECARD(s,r) ; break ;
             case '9' : r = NINE;  dl[dlcnt++] = MAKECARD(s,r) ; break ;
             case '8' : r = EIGHT; dl[dlcnt++] = MAKECARD(s,r) ; break ;
             case '7' : r = SEVEN; dl[dlcnt++] = MAKECARD(s,r) ; break ;
             case '6' : r = SIX;   dl[dlcnt++] = MAKECARD(s,r) ; break ;
             case '5' : r = FIVE;  dl[dlcnt++] = MAKECARD(s,r) ; break ;
             case '4' : r = FOUR;  dl[dlcnt++] = MAKECARD(s,r) ; break ;
             case '3' : r = THREE; dl[dlcnt++] = MAKECARD(s,r) ; break ;
             case '2' : r = TWO;   dl[dlcnt++] = MAKECARD(s,r) ; break ;
             case ':' : endofdeal = 1 ;  break ;
             case '\0': endofdeal = 1 ;  break ;  /* allow pbn str to be null term as well as : term */
             default:
                fprintf(stderr, "Invalid Char in PBN deal source string\n") ;
                fprintf(stderr, "Invalid Char in PBN deal source string\n") ;
                return -1 ;
         } /* end switch */
   } /* end for tc */

    #ifdef JGMDBG
      if (jgmDebug > 6 ) {
          fprintf(stderr, "Created Deal OK [%s]\n",t);
          fprintf(stderr, "Deal in Hex=[");
          for (dlcnt=0 ; dlcnt < 26 ; dlcnt++ ) {
              fprintf(stderr, "%02X ", dl[dlcnt] );
          }
          fprintf(stderr,"\n             ");
          for (dlcnt=26; dlcnt<52 ; dlcnt++) {
              fprintf(stderr, "%02X ", dl[dlcnt] );
          }
          fprintf(stderr,"]\n");
      } /* end if jgmDebug > 6 */
    #endif

    return 1 ;
} /* end PBN_to_Deal52 */

char *Hand52_to_PBNbuff (int p, char *dl52, char *buff ) { // Returns ptr to the NULL at the end of the buffer 
	/* Returns ptr to the NULL at the end of the buffer that contains
	 * the PBN representation of the DEAL52_k hand. Last two chars in the buffer are SPACE NULL
	 * Does NOT put the compass names in the buffer- so can be used to fmt PBN input to DDS 
	 * Does NOT modify the original input. It sorts a local copy of the hand.
	 */
   int curr_suit, card_rank, card_suit;
   int count;
   char *bp ;
   char hand52[13];
   unsigned char kard ;
   char suit_sep = '.';
   memcpy(hand52, &dl52[p*13], 13) ; 
   dsort13(hand52) ;  /* hand sorted. dl[p*13+0] = Highest Spade; dl[p*13+12] = lowest club. */
   bp = buff ;
   count = 0 ;
   curr_suit = 3 ; // spades
   while (count < 13 ) {  // a hand ALWAYS has exactly 13 cards
       kard = hand52[count] ; card_suit = C_SUIT(kard); card_rank = C_RANK(kard) ;
       assert( 0x00 <= kard && kard <= 0x3C) ; 
       while( curr_suit != card_suit ) { /* write a suit separator for missing suits spades downto first one*/
            *bp++ = suit_sep;
            JGMDPRT(6,"Wrote Void for suit %d \n",curr_suit ) ; 
            curr_suit-- ;
            assert( curr_suit >= 0 ) ;
        } /* end while curr_suit != card_suit */
        assert(card_suit == curr_suit) ;
        while ( (curr_suit == card_suit) && (count < 13) ) { /* write the cards in this suit */
            kard = hand52[count]; card_suit = C_SUIT(kard); card_rank = C_RANK(kard) ;
            if (curr_suit != card_suit ) break;
           *bp++ = rankids[card_rank];
           count++;
           JGMDPRT(7," Num[%d]=%c%c ", count, "CDHS"[curr_suit], *(bp-1) ) ; 
        } // end while curr_suit
        JGMDPRT(7, "\n"); 
       *bp++ = suit_sep;
        curr_suit-- ; /* Move to next suit */
    } /* end while count < 13 line 191 */
    assert(count == 13 ) ;
    // Normal case curr_suit is -1; void clubs curr_suit = 0, void clubs, diamonds, and hearts curr_suit = 2
    // In case there were voids at the end of 13 cards
        while ( curr_suit >= 0 ) { /* write a suit separator for missing suits after the last one downto clubs*/
            *bp++ = suit_sep ;
            curr_suit-- ;
        }
        JGMDPRT(7,"suit=%d last_buff=%c card_count=%d \n",curr_suit, *(bp-1), count);
        /* the last char is the suit separator which we don't need after the club suit, so replace it with a space */
        if ( *(bp-1) == suit_sep ) { *(bp-1) = ' ' ; }
        else { fprintf(stderr, "CANT HAPPEN in Hand52_to_Buff, last char is not a suit_separator %c \n", *(bp-1) ); }
        *bp = '\0' ; // terminate the buffer as a string
        return bp  ; /* return pointer to null byte in case we want to append another hand to the buffer */
} /* end Hand52_to_PBNbuff */

/* Format one or more hands for output in GIB style PBN string in a buffer. No \n at end. Returns ptr to start of users buffer */
char *fmtHands52_to_PBN( char *buff, int mask, DEAL52_k d ) {  // No newline at end of print. Caller to put \n or comma as reqd.
/* Format hands in the same format as printoneline does but option to print 1,2,3,or 4 hands */
/* Hands are always printed in order of N,E,S,W but some may be omitted e.g. might be only E and S */
/* also do not print newline at end. */
/* sample pbnfuff: n .AKQJ987.5432.A3 e A5432..JT98.KQJ2 s KQJ.6543.AKQ.T98 w T9876.T2.76.7654 */
  char pt[] = "nesw";
  char *pbn_ptr;
  int  p;
  for (p=0 ; p< 4; p++ ) {
     if ( !(mask & 1 << p ) ) continue ; /* skip this player if he was not called for */
    *pbn_ptr++ = pt[p]; *pbn_ptr++ = ' ' ; // player names are followed by a space */
    pbn_ptr = Hand52_to_PBNbuff (p, (char *)d, pbn_ptr ); // append a hand to end of pbnbuff; returns ptr to null at end.
  }
  /* return tNULL terminated pbn format buffer to user; */
  return buff ; 

} /* end fmtPBN_Hands52 */

/* Format a hand or side into a buffer suitable for input from  the cmd line to DOP or KnR and the like
 * slashes between suits, null suits shown as minus sign, compass name preceeded with minus sign
 * Ex: -E AKQJ/5432/-/T9876 -W -/AKQJT9/KJT/5432 
 */
char *fmtHand52_to_cmdbuff ( char *buff,  int p, DEAL52_k  dl ) { // returns ptr to null at end of buffer
/* hand is sorted. dl[p*13+0] = Highest Spade; dl[p*13+12] = lowest club. */
/* OPC fmt for a hand/deal is :-N AKQJ2/-/T54/Q6542 and so on for -S, -E, -W in any order. Voids are dashes. Suit Sep is slash */
   char suit_sep = '/'; 
   int curr_suit, card_rank, card_suit;
   int di, count;
   unsigned char kard, prev_kard ;

   char *bp ;
   bp = buff ;

   JGMDPRT(9, "Hand52:: bp=%p \n",bp ) ;

   di = p*13 ;
   count = 0 ;
   curr_suit = 3 ; // spades
   JGMDPRT(7, "Hand52_to_buff:: p=%d, di=%d, dl[di]=%02x\n",p,di,dl[di] ) ;
   JGMDPRT(7,  "buff=%p, bp=%p \n", buff, bp );
	prev_kard = 0x00 ; 
   while (count < 13 ) {
       kard = dl[di] ; card_suit = C_SUIT(kard); card_rank = C_RANK(kard) ;
       assert(kard > prev_kard ); /* in case the hand was not properly sorted */
        JGMDPRT(8,"Top Big While::Kard=%02x, card_suit=%d, card_rank=%d, count=%d, curr_suit=%d\n",
                                                kard, card_suit, card_rank, count, curr_suit ) ; 
       while( curr_suit != card_suit ) { /* write void mark for missing suits */
            *bp++ = '-';
            *bp++ = suit_sep;
             curr_suit-- ;
        } /* end while curr_suit != card_suit */
        assert(card_suit == curr_suit) ;
        while ( (curr_suit == card_suit) && (count < 13) ) { /* write the cards in this suit */
            kard = dl[di]; card_suit = C_SUIT(kard); card_rank = C_RANK(kard) ;
            JGMDPRT(9,"Top Small While::Kard=%02x, card_suit=%d, card_rank=%d, count=%d, curr_suit=%d\n",
                                                kard, card_suit, card_rank, count, curr_suit ) ;
            if (curr_suit != card_suit ) break;
            *bp++ = rankids[card_rank];
            prev_kard = kard ; 
            count++; di++;
            JGMDPRT(9," Num[%d]=%c%c ", count, "CDHS"[curr_suit], *(bp-1) ) ; 
         }  /* end while count < 13 write cards in this suit */
        JGMDPRT(9,"\n"); 
       *bp++ = suit_sep;
        curr_suit-- ; /* Move to next suit */
    } /* end while count < 13 -- line 1038*/
    assert(count == 13 ) ;

    // Normal case curr_suit is -1; void clubs curr_suit = 0, void clubs, diamonds, and hearts curr_suit = 2
    // In case there were voids at the end of 13 cards
        while ( curr_suit >= 0 ) {
            *bp++ = '-' ;
            *bp++ = suit_sep ;
            curr_suit-- ;
        }
        /* the last char is the suit separator which we don't need after the club suit, so replace it with a space */
        if ( *(bp-1) == suit_sep ) { *(bp-1) = ' ' ; }
        else { fprintf(stderr, "CANT HAPPEN in Hand52_to_Buff, last char is not a suit_separator %c \n", *(bp-1) ); }
        *bp = '\0' ; // terminate the buffer as a string
        return bp  ; /* return pointer to null byte in case we want to append another hand to the buffer */
} /* end fmtHand52_to_cmdbuff Line 250*/

char *fmtSide_to_cmdbuff(char *buff , int side, DEAL52_k  dl ) {
  char side_hands[]="NSEW";
  int h ;
  char *bp ;
  bp = buff ;
  // If we pass in the beginning of the opc_cmd_buff we need these next ones. If just pass in the start pos in buff, no need
  // size_t buff_len ;
  // buff_len = strlen(buff);
  // bp = buff + buff_len ;
  /* so use this to append to a OPC buff that already contains pgm name, and pass in pos to write first char to. */
  h = side ;
  *bp++ = '-'; *bp++ = side_hands[side*2]; *bp++ = ' '; *bp='\0';

  JGMDPRT(7,"Format Side:: bp=%p \n", bp ) ;
  JGMDPRT(8, "fmt_side Hand=%c,%d %p\n", side_hands[side*2], h, bp ) ; 
  DBGDO( 9, sr_hand_show(h, dl) );

  dsort13(dl + h*13) ; /* need this as the opc verb can be called in the condition clause before the deal is sorted */
  bp = fmtHand52_to_cmdbuff(bp, h, dl) ; // [-E AK2/QJ43/-/AJ9432 ]
  *bp++ = ' ';
   
   h = 2 + side ; // have done first hand of side, either north(0), or east(1) so now do south(2) or west(3)
  *bp++ = '-'; *bp++ = side_hands[1+side*2]; *bp++ = ' '; *bp='\0';
	dsort13(dl + h*13) ; /* need this as the opc verb can be called in the condition clause before the deal is sorted */
   bp = fmtHand52_to_cmdbuff(bp, h, dl) ; //[-E xxx/xxxx/xxx/xxx -W xx/xxxxx/xx/xxxx ]
  
  *bp++ = ' ';
  *bp='\0';
  JGMDPRT(8,"fmt_side Hand=%c,%d bp=%p \n", side_hands[1+side*2], h, bp ) ;
  JGMDPRT(8, "fmt_side -- Hand52 to buff done for h=%d, bp=%p \n",h, bp) ;
  JGMDPRT(8, "fmt_side returns %p\n", bp ) ;

  return bp ;
} /* end fmtSide_to_cmdbuff */






