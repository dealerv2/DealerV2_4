/* File zrd_defs.h   2024-01-07 */
#ifndef ZRD_DEFS_H
#define ZRD_DEFS_H
#ifndef _GNU_SOURCE
  #define _GNU_SOURCE
#endif
#define EBADZRD   -3
#define EIOZRD    -2
#define EOFZRD     0
#define ENOZRD_DL -1 
#define ENOLIB_DL -1
#define ZRD_REC_SIZE 23
#define ZRD_BLOCKSIZE 1000
#define ZRD_MAX_RECS 10485760
#define ZRD_MAX_SEED 10485

#define ZRD_DL_OK  1
#define ZRD_HDR  2
#define ZRD_DEAL 4
#include <stdio.h>
typedef char DEAL52_k[52] ;
typedef int  DL52_TRICKS_k[4][5] ;

struct cbf_st {  /* Use chars to avoid spurious alignment gaps */
		 unsigned char card0 : 2 ;
		 unsigned char card1 : 2 ;
		 unsigned char card2 : 2 ;
		 unsigned char card3 : 2 ;
} ; 
struct zpos_st { struct cbf_st cbf[13] ;} ; /* 13 bytes of four 2-bit fields each makes up a 'deal' */
typedef struct zpos_st ZRD_DEAL_k ;    /* syntax:  zdl.cbf[i].cardX */

struct tbf_st {   /* Must use chars else the zrdrec_st wont be 23 bytes per RP spec*/
		 unsigned char trW : 4 ;  /* W tricks */
		 unsigned char trN : 4 ;  /* N tricks */
		 unsigned char trE : 4 ;  /* E tricks */
		 unsigned char trS : 4 ;  /* S tricks */
} ;
struct ztrick_st {struct tbf_st tbf[5] ; } ; /* 5 sets (NT,S,H,D,C) of 4 tricks(W,N,E,S) */
typedef struct ztrick_st ZRD_TRICKS_k ; /* zt.tbf[i].trX --  -- 10 bytes if using chars (or shorts) */

struct zrdrec_st {
	ZRD_DEAL_k zd ; 				// pzrdrec->zd.cbf[i].cardx
	ZRD_TRICKS_k zt ; 			// pzrdrec->zd.zt.tbf[i].trX
} ;
typedef struct zrdrec_st ZRD_REC_k ; 

struct zrd_hrec_st {	/* each hdr rec can hold 19 chars of title. Up to 15 hdr_recs per set to hold complete title */	
	unsigned int  zero28 : 28 ;   /* bit field low 3 Nybles */
	unsigned char seqnum : 4  ;	/* to pack this into the top Nyble it must be type char */
	         char hdrtxt[19]  ;	/* total 23 bytes, but compiler adds an extra byte after this one. */
} ;

  /* -- some unions to make it easy to access several bit fields at once. ----*/
union cbf_ut {				/* a single set of 4 cards */
	struct cbf_st ucbf; 	/* 4 x 2bit fields */
	unsigned char uzp4; /*  players x4 */
} ;

union tbf_ut {				/* 4 tricks in one strain */
	struct  tbf_st utbf;	/* 4 x 4bit fields */
	unsigned short uzt4; /* tricks x 4 One strain, all four compasses range 0x0000 .. 0xDDDD */
} ;

union zrd_rec_ut {
	struct zrdrec_st   zr;
	struct zrd_hrec_st zh;
	char zrd_buff[23] ;  
} ;	

#define MAKE_SEQNUM(a,n) ( ((a) & 0x0F) | ( (0x0F&(n)) <<4) )  /* insert low Nyble of byte n to Hi Nyble of byte a */
#define ZRD_HDR_SEGSZ  19 
#define ZRD_HDR_MAXSZ  284   /* 15 segments of 19 chars, less 1 for NULL at end */

#ifndef C_RANK
	#define C_RANK(c) ( (c) & 0x0F )
	#define C_SUIT(c) ( ((c)>>4 ) & 0x0F)
#endif
#define CBF_CAST_CH(z) ( (unsigned char) ((z).card3<<6 | (z).card2<<4 | (z).card1<<2|(z).card0) )
#define LOW28BITS 0x0FFFFFFF

enum zrd_seat_ek {KIBTZ=-1, WEST=0, NORTH, EAST, SOUTH } ;         /* ZRD numbering */
enum card_rank_ek {spot=-1, Two_rk=0, Three_rk, Four_rk, Five_rk, Six_rk, Seven_rk, Eight_rk, Nine_rk, Ten_rk, Jack_rk, Queen_rk, King_rk, Ace_rk };

int zrd_write(FILE *fzrd, int rectype, DEAL52_k dl ) ;
#endif    /* file guard */
