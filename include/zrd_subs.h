/* File deal_zrd_subs.h -- for DEAL52_k <=> ZRD_k coding and decoding routines */
#ifndef DEAL_ZRD_H
#define DEAL_ZRD_H
#include "../include/dealtypes.h"

enum zrd_seat_ek {KIBTZ=-1, WEST=0, NORTH, EAST, SOUTH } ;         /* RP numbering */
enum dl_seat_ek {COMPASS_N=0, COMPASS_E, COMPASS_S, COMPASS_W } ; /* Dealer numbering */
// enum dl_suit_ek {NONE=-1, CLUBS, DIAMONDS, HEARTS, SPADES, NOTRUMP } ; /* Dealer numbering for Makecard stuff */
enum card_rank_ek {spot=-1, Two_rk=0, Three_rk, Four_rk, Five_rk, Six_rk, Seven_rk, Eight_rk, Nine_rk, Ten_rk, Jack_rk, Queen_rk, King_rk, Ace_rk };

         /* --- Defines ---*/
#define ZRD_SIZE 23
#define ZRD_HDR_SIZE 19
#define MAKECARD(suit,rank) ( (char) (((suit)<<4) | ((rank)&0x0F)) )
// #define C_RANK(c) ( (c) & 0x0F )
// #define C_SUIT(c) ( ((c)>>4 ) & 0x0F)

         /* --- types ---*/
#define HI_NYBBLE( ch ) ( ((ch) >> 4 ) & 0x0F )   /* Top 4 bits of a Byte; used to extract tricks */
#define LO_NYBBLE( ch ) (  (ch) & 0x0F ) 	      /* Lo 4 bits of a Byte ; uese to extract tricks */
typedef unsigned char BYTE_k;
typedef char DEAL_k[52];         /* char instead of unsigned char; easier compatibility and cards never use hi-bit anyway*/
struct zrdRec_st {
     BYTE_k card[13] ;
     BYTE_k trick[10];
} ;
union zsep_ut {
   BYTE_k zerobyte[4];    /* all zeros except perhaps for the low Nybble of byte[3] which could be the header record segment number*/
   unsigned int izero;
} ;
struct pbn_st {
   char deal_ch[76];          /* col 76 for the null terminator */
   short int deal_tr[20];     /* tricks never use the hi bit and signed is easier to use than unsigned */
} ;
struct Deal52Res_st {
	DEAL52_k dl52 ; 		/* the text for header record segment also goes here if any */
	int  trix52[4][5] ;     /* [NESW][CDHSN] */ /* header  record segment number goes into trix52[0][0] */
} ;
union zrd_hdr_ut {
	char hdr_text[305] ;  /* 16 * 19 + 1 byte for the null terminator */
	char hdr_part[16][19] ;
};


int zrd_to_Deal52Res(struct zrdRec_st *zrdrec, struct Deal52Res_st *dl52Res ) ;
void append_zrdhdr_part(union zrd_hdr_ut *zrd_hdr, struct Deal52Res_st *dl52Rec ) ;
int dl52_to_zrdrec(struct Deal52Res_st *dl52Rec, struct zrdRec_st *zrdrec );

#endif   /* ifndef file guard */


