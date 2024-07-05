/* File src/dealzrd_subs.c -- 2024-01-07 */
/* Useful code to 
 * A) Write dl52 deals to a zrd file that can later be read in using the rplib -L switch
 * B) Read a zrdlib file, decode the deal into dl52 format and use for studies etc.
 *    It will be quicker because the deal will already match the conditions, and will already be solved
 * First batch of code is to write a zrd file
 * Second batch is to read a zrdlib file. zrdlib vars and files are for input
 */
#ifndef GNU_SOURCE
	#define GNU_SOURCE
#endif
#include <assert.h>
#include <errno.h>

#include <stdio.h>
#include <stdlib.h> /* for srand48, mrand48 and friends */
#include <string.h>

#include <unistd.h> /* sysconf, usleep, getopt, write, and other low level i/o */
#define JGMDBG 1

#include "../include/dbgprt_macros.h"    /* JGMDPRT other DBGLOC etc. */
#include "../include/libdealerV2.h"
#include "../include/dealzrd_defs.h"

/* ------------------ File Global Data  ------------------------*/

static ZRD_DEAL_k   zrd_dl ; 		/* Struct containing 13 array of struct of 4 bitfields. zrd_dl.cbf[i].cardN */
static ZRD_TRICKS_k zrd_ddtrix ;	/* Struct containing 5 array of struct of 4 bitfields. zrd_ddtrix.tbf[i].trX */
static ZRD_REC_k    zrd_rec ;      	/* struct to hold the record on disk */
												/* syntax zrd_rec.zd.cbf[i].card3 or zrd_rec.zt.tbf[strain].trN */
static struct zrd_hrec_st zrd_hrec; /* syntax zrd_hrec.seqnum or zrd_hrec.hdrtxt[i] */
static DL52_TRICKS_k dl52_ddres ;  	/* syntax dl52_ddres[h][s] */

static ZRD_DEAL_k 		*pzrd_dl = &zrd_dl ; 			/* pzrd_dl->cbf[i].cardN */
static ZRD_TRICKS_k		*pzrd_ddtrix = &zrd_ddtrix ; 	/* pzrd_ddtrix->tbf[i].trX */
static ZRD_REC_k 			*pzrd_rec = &zrd_rec ; 			/* pzrd_rec->zd.cbf[i].cardN or pzrd_rec->zd.tbf[i].trX */
static struct zrd_hrec_st *pzrd_hrec = &zrd_hrec ; 	/* syntax pzrd_hrec->seqnum or pzrd_hrec->hdrtxt[i] */
static DL52_TRICKS_k 	*pdl52_ddres = &dl52_ddres ;	/* syntax dl52_ddres->[h][s] */ 

static int zstrain[5]= {4,3,2,1,0}; /* C,D,H,S,N --- convert dl52 (0,1,2,3,4) to zrd numbering */
static int zsuit[4]  = {3,2,1,0};   /* C,D,H,S   --- convert dl52 (0,1,2,3)   to zrd numbering */
static int zrpos[13] = {12,11,10,9,8,7,6,5,4,3,2,1,0}; /* 2345 6789 TJQK A position in 13 slot hand*/
static int zseat[4]  = {1,2,3,0}; 						   /* convert N,E,S,W to zrd numbering zN=1,zW=0 */

         /* --- Subroutines  ---*/

/* ------------------ Function Declarations  -----------------*/
	/* functions the user calls */
int zrd_write(  FILE *fzrd, int rectype, DEAL52_k dl ) ;          /* type hdr uses global var options.title. dl irrelevant */
int zrd_getdeal(FILE *fzrd, struct options_st *opts, DEAL52_k dl ) ; /* Decodes zrd_deal into DEAL52_k dl */

	/* Internal Functions for writing a zrd record */
int  dl52card_to_zrd_dl(int p, int s, int r, ZRD_DEAL_k*zrd_dl) ; /* player p, has card SuitRank ; update zrd_deal accordingly */
void dl52_to_zrd_dl( DEAL52_k dl , ZRD_DEAL_k *pzrd_dl ) ;
void dl52Tricks_to_zrdTricks(DL52_TRICKS_k dl52_ddres , ZRD_TRICKS_k  *pzt) ;
int fwrite_zrdrec(FILE *fzrd, ZRD_REC_k *pzrec) ; /* ret 1 on success, -ve number on error */


	/* Internal Functions for reading a zrd record */
int set_zrdlib_vars(FILE *fzrd, struct options_st *opts) ; /* compute blocksize etc. for seed offsets */
long int zrd_seekfpos(FILE *fzrd, long int seed ) ;
int zrd_err_check( FILE *fzrd, int byte_cnt, int rsize )  ;
void zrdTricks_to_dl52Tricks( ZRD_TRICKS_k  *pzt, DL52_TRICKS_k dl52_ddres ) ; /* reading zrdTricks into DealerV2 */
int fread_zrdrec(FILE *fzrd,  union zrd_rec_ut *uzrec_ptr ) ; /* return EOFZRD or bytes read */
int zrd_append_htxt(char *pbuff, struct zrd_hrec_st *phrec ) ; /* concat the hdr txt segments into a buffer */
int zrd_decode_deal( char *dl, ZRD_REC_k *pzrec ) ;				/* convert zrd fmt to dl52 fmt incl tricks */
int zrd_getrec (FILE *fzrd, union zrd_rec_ut *uzrec_ptr, struct options_st *opts ) ;

				/* debugging functions */
void show_zrdrec(ZRD_REC_k *pzrdrec ) ;
void dump_zrdrec(ZRD_REC_k *pzrdrec ) ;
void show_zrd_sizes() ;

void show_cbf_st(struct cbf_st cbf, char *msg ) { /*hi to low for easier hex txlate */
   fprintf(stderr,"%s [%d, %d, %d, %d] \n", msg, cbf.card3, cbf.card2, cbf.card1, cbf.card0) ;
	return ; 
}
void show_cbf_ut (union cbf_ut zpu, char *msg ) {	/* hi to low for easier hex txlate */
   fprintf(stderr,"%s, %02X [%d, %d, %d, %d] \n", msg, zpu.uzp4, zpu.ucbf.card3, zpu.ucbf.card2, zpu.ucbf.card1, zpu.ucbf.card0 );
   return ; 
}
void show_tbf_st( struct tbf_st ztx, char *msg) {
   fprintf(stderr,"%s [%d,%d,%d,%d] \n", msg, ztx.trS, ztx.trE, ztx.trN, ztx.trW ) ; /* hi to low to ease hex txlate */
	return ; 
}
void show_tbf_ut( union tbf_ut ztx , char *msg) {
    fprintf(stderr,"%s %04X [%d,%d,%d,%d] \n", 
						 msg, ztx.uzt4, ztx.utbf.trS, ztx.utbf.trE, ztx.utbf.trN, ztx.utbf.trW ); /* hi to lo to ease hex txlate */
	return ; 
}
void show_zrdTricks( ZRD_TRICKS_k  *pzt  ) {
	int zs;
	for (zs=0 ; zs<5 ; zs++) { 
		fprintf(stderr,"%c: W=%d, N=%d, E=%d, S=%d \n","NSHDC"[zs], 
					pzt->tbf[zs].trW, pzt->tbf[zs].trN, pzt->tbf[zs].trE, pzt->tbf[zs].trS);
	}
}
// pzrd_dl is a pointer to the 'deal' in zrd_fmt which is a 13-array of bit_field structures.
int dl52card_to_zrd_dl(int p, int s, int r, ZRD_DEAL_k *pzrd_dl) { /* player p, has card SuitRank ; update zrd_deal accordingly */
	int zr, zs ;
	int zslot  ; 
	int zbfld  ;
	int zcard  ; 
	int zp ;
	int rc ; 
	zp = zseat[p];
	zs = zsuit[s] ; 
	zr = zrpos[r]  ;
	zcard = 13*zs + zr ;
	zslot = zcard / 4  ; 
	zbfld = zcard % 4  ; 
	rc=zslot*1000 + zbfld ;
	JGMDPRT(6," ZRD_CARD: P[%d->%d],S[%d->%d],R[%d->%d],zcard=%d,zslot=%d,zbfld=%d \n",p,zp,s,zs,r,zr,zcard,zslot,zbfld);
	assert(zslot < 13 && zbfld < 4 ) ; 
	switch(zbfld) {
		case 0 : pzrd_dl->cbf[zslot].card0 = zp ; break; /* put zplayer into the relevant bitfield */ 
		case 1 : pzrd_dl->cbf[zslot].card1 = zp ; break; 
		case 2 : pzrd_dl->cbf[zslot].card2 = zp ; break; 
		case 3 : pzrd_dl->cbf[zslot].card3 = zp ; break; 
		default: assert(0) ; break; 
	}
	JGMDPRT(6,"Player[%d] to slot.bfld[%6.2f] ",zp,((0.0+rc)/1000.) ) ;
	DBGDO(6,show_cbf_st(pzrd_dl->cbf[zslot],"Slot Bitfields3,2,1,0=") );
   return rc ; 
} 
/* end dl52card_to_zrd_dl */

void dl52_to_zrd_dl( DEAL52_k dl , ZRD_DEAL_k *pzrd_dl ) { /* structs are passed on stack; arrays are not */
	int p, s, r , i, di, rc;
	float frc ; 
	CARD52_k kard ; 
	for (p=0; p < 4 ; p++ ) {
	   JGMDPRT(5,"\nGiving Cards to Hand p=%c:\n","NESW"[p]);
		di = 13*p ; 
		for (i=0; i< 13; i++ ) {
			kard = dl[di+i] ; 
			s = C_SUIT(kard);
			r = C_RANK(kard);
			rc = dl52card_to_zrd_dl(p,s,r,pzrd_dl) ; 
			frc = (float)(rc/1000.) ;
			JGMDPRT(6,"Card[%c%c]->Player[%d] in [0-12Slot.bf[%6.2f] : %d\n","CDHS"[s],"23456789TJQKA"[r], p, frc, rc);
		}
	}
#ifdef JGMDBG
	JGMDPRT(7,"\n Recap dl52_to_ZRD Showing slots 0..12 cards 3,2,1,0 to match xxd order\n");
	for (i=0 ; i<13 ; i++ ) {
		DBGDO(7,show_cbf_st (pzrd_dl->cbf[i], "Slot:3,2,1,0") );
	}
	JGMDPRT(7,"\n");	
#endif
} 
/* end dl52_to_pzrd_dl */

void dl52Tricks_to_zrdTricks(DL52_TRICKS_k dl52_ddres , ZRD_TRICKS_k  *pzt) {
	int s, zs; 
	for (s=0; s<5 ; s++ ) {  /* clubs to NT */
		zs = zstrain[s] ;
		pzt->tbf[zs].trW = dl52_ddres[3][s] ;  /* zrd_trix[zs]->trW ? */
	   pzt->tbf[zs].trN = dl52_ddres[0][s] ;
	   pzt->tbf[zs].trE = dl52_ddres[1][s] ;
	   pzt->tbf[zs].trS = dl52_ddres[2][s] ;
	   JGMDPRT(5,"ZRD_TX s=%d,zs=%d, :%c: N=%d, E=%d, S=%d, W=%d \n",s,zs,"NSHDC"[zs], 
	   pzt->tbf[zs].trN, pzt->tbf[zs]. trE, pzt->tbf[zs].trS, pzt->tbf[zs].trW );
	}
}
/* end dl52Tricks_to_zrd */

/* encode curdeal, and dds.res.bin and write out, or write a set of hdr recs using the global options.title */
int zrd_write(FILE *fzrd, int rectyp, DEAL52_k dl ) { /* uses global options struct */
	int rc = 0 ; 
	int i,l,n,k ; 
	if(rectyp >=  ZRD_DEAL ) {
		dl52_to_zrd_dl(dl, pzrd_dl ) ;
		if (1 == options.zrd_dds ) {
			dds_parscore(0, 0) ; /* Dummy call to dds to fill the cache. ignore return value */
			memcpy(pdl52_ddres, &dds_res_bin.tricks, sizeof(dds_res_bin.tricks) ) ; 
		}
		else { memset(pdl52_ddres, 0, sizeof(dl52_ddres) ) ; }
		dl52Tricks_to_zrdTricks(dl52_ddres, pzrd_ddtrix ) ; 
		pzrd_rec->zd = *(pzrd_dl) ;			// structure assignment; no need for memcpy
		pzrd_rec->zt = *(pzrd_ddtrix) ; 		//    ditto
		rc = fwrite_zrdrec(fzrd, pzrd_rec ) ; 
		if ( rc < 0 ) {
			perror("Cant zrd_write zrd_rec -- Aborting run .... ");
			exit(-1) ; 
		}
		return (rc) ; 
	} /* end rectyp ZRD_DEAL */
	/* Default is no title unless user (or dli file) gives one. 
	 * A default title when none was specified might be an unwelcome surprise to the user 
	 */
   else if ( (ZRD_HDR == rectyp) && (options.title_len > 0) ) { /* User gives Non Zero Len title. write it to the zrdrec file one segment at a time */
			l = options.title_len + 1 ; /* for the NULL at the end */
			n = 1 ; 
			k = 0 ;
			pzrd_hrec->zero28 = 0 ; 
			JGMDPRT(8,"Writing zrd HDR rec; Title len=%d, segment#=%d, Title pos=%d\n",l, n, k ) ; 
			while (l > 0 ) {
				i = (l > ZRD_HDR_SEGSZ ) ? ZRD_HDR_SEGSZ  : l ;
				memset(pzrd_hrec->hdrtxt, '\0', ZRD_HDR_SEGSZ) ; /* to prevent left over chars in last segment */	
				strncpy(pzrd_hrec->hdrtxt, &options.title[k] , i ) ;
				pzrd_hrec->seqnum = n++ ;  
				JGMDPRT(8,"Writing zrd HDR rec; Chars Remaining=%d, segment#=%d, Title pos=%d, char wrote=%d\n",l, n, k, i ) ;
				rc = fwrite_zrdrec(fzrd, (ZRD_REC_k *)pzrd_hrec) ; 
				if ( rc < 0 ) {
					perror("Cant zrd_write Title Segment -- Aborting run .... ");
					exit(1) ; 
				}
				
				l -= i ; /* chars remaining to be copied */
				k += i ; /* next position in the buffer title */
			} /* end while the last segment will have a null somewhere because title has a null*/
			assert( n < 16 && l==0 && k>=options.title_len ) ; 
	} /* end header record writing */
   return (0) ; 	
} /* end zrd_write */
 
int fwrite_zrdrec(FILE *fzrd, ZRD_REC_k *pzrec) { /* low level; write and err check */
	int cnt ;
	cnt = fwrite(pzrec, 23, 1 , fzrd) ;  /* *buff, size, nmemb, File* */
	if ( ferror( fzrd ) ) {
		perror("zrdrec write error" ) ;
		fprintf(stderr, "ERR**: Error Writing ZRD Library file.\n");
		return EIOZRD ; 
	}
	return cnt ; /* should always be 1 unless an error */
}
/* end fwrite_zrdrec */	

/* low level read one record and error check No recovery, No rewind, No skipping headers etc. */
int fread_zrdrec(FILE *fzrd,  union zrd_rec_ut *uzrec_ptr ) {
	int byte_cnt ; 
	int zrd_read_err = 0 ; 
	byte_cnt = fread(uzrec_ptr->zrd_buff, 1, sizeof(ZRD_REC_k), fzrd) ; /* read 23 items of size 1 into char buffer */
	zrd_read_err = zrd_err_check(fzrd, byte_cnt, ZRD_REC_SIZE) ;
   switch (zrd_read_err) {
	   case EIOZRD :  /* I/O error. Abort run */  
		   fprintf(stderr, "I/O error on Library file Aborting....\n" );
			exit(-1);
			break ; 
		case EBADZRD :  /* Bad Data Record. Abort run */ 
			fprintf(stderr, "Bad Data Record in Library file at recnum=%d. zrdlib_pass_num=%d. Aborting....\n",
						zrdlib_recnum, zrdlib_pass_num ) ; 
			exit(-1);
			break ; 
		case EOFZRD : /* eof on file. Rewind to start and read again */ 
			if( nprod < options.max_produce) { /* Reached eof before necessary records got */
				fprintf(stderr, "[%s%d] EOF in Pass Num %d occurred at zrd_cnt=%d, zrdlib_recnum=%d ngen=%d Rewinding File \n",
					__FILE__,__LINE__,zrdlib_pass_num,zrd_cnt,zrdlib_recnum,ngen);
			}
			return EOFZRD ; 
			break ; 
		default :  /* valid zrd_rec of 23 bytes read OK */
			return ZRD_REC_SIZE ; 
		   break ; 
	} /* end switch */
	/* NOTREACHED */
	return ZRD_REC_SIZE ; 
} /* end read_zrd_file */
/* end fread_zrdrec */

/* Uses global variable dds_res_bin[4][5] for tricks portion of zrd_file record also record counts etc. */
int zrd_getdeal(FILE *fzrd, struct options_st *opts, DEAL52_k dl ) { /* Decodes zrd_deal into DEAL52_k dl */
   union zrd_rec_ut uzrec; 
   union zrd_rec_ut *uzrec_ptr = &uzrec; /* uzptr->zr.zd.cbf[i].cardX uzptr->zr.zt.tbf[i].trX  uzptr->zh.hdrtxt[]  uzptr->zrd_buff[] */
   int zrd_rec_type ; /* ZRD_HDR or ZRD_DEAL */
   int found_hdrs = 0 ; 
   
   int rc ;   /* returns ENOLIB_DL  or DL_OK */

   /* If 1st 14 'cards' are zero it means West has them. Impossible.
    * So RP says it is some sort of separator record. RPLIB has none of these. JGM ZRD files allow for titles etc.
    */
   zrd_rec_type = -1 ; 
	do { 
      zrd_getrec(fzrd, uzrec_ptr, opts);  /* returns 23 for normal data record. 0 at eof */
      if ( 0 == uzrec_ptr->zh.zero28 )  { /* (one of) header record(s) making up a title */
			zrd_rec_type = ZRD_HDR ; 
			found_hdrs = 1 ; 
         JGMDPRT(8,"Found a null separator at record number %d first bytes in buff=[%02x, %02x, %02x, %02x ]\n",
					zrdlib_recnum, uzrec_ptr->zrd_buff[0],uzrec_ptr->zrd_buff[1],uzrec_ptr->zrd_buff[2],uzrec_ptr->zrd_buff[3] );
         JGMDPRT(8,".zero28=%d, seq_num = %d \n",uzrec_ptr->zh.zero28 , uzrec_ptr->zh.seqnum ) ;  
         zrd_append_htxt(opts->title, &uzrec_ptr->zh ) ;         
      } 
      else { zrd_rec_type = ZRD_DEAL ; }
   } while ( zrdlib_pass_num < 2 && zrd_rec_type != ZRD_DEAL ) ;
   if ( 1 == found_hdrs ) {
		opts->title[MAXTITLE] = '\0' ; /* should not be reqd as the last seg of a hdr set should be null termed*/ 
		opts->title_len = strlen(opts->title) ; 
		title_len = opts->title_len ; 
		strncpy(title , opts->title, opts->title_len) ;
	} 
   /* end do-while -- either a valid record or an exceeded wrap count */
     // DBGDO(9,dump_zrdrec(&uzrec_ptr->zr) ); 
      DBGDO(9,show_zrdrec(&uzrec_ptr->zr) );
     if(zrdlib_pass_num >= 2 ) {  
		  deal_err = ENOLIB_DL ; 
		  return ENOLIB_DL ;  /* ran out of records in the library before producing the required number */	  
	 }
	 zrd_cnt++;   /* the number of valid actual deal records read. (not separators) akin to ngen */
     /* Got a valid zrdLib record. Decode it into deal52 format, and tricks. */ 
    rc = zrd_decode_deal(dl, &uzrec_ptr->zr ) ;  /* will fill dl, and also the global dds_res_bin */
    deal_err = rc ;    /* zrd_decode always returns ZRD_DL_OK; no error conditions possible */
    return rc ; /* ZRD_DL_OK */
    
} /* end zrd_getdeal */

int zrd_decode_deal( char *dl, ZRD_REC_k *pzrec ) { /* convert zrd cards to dl52 fmt; fill tricks cache also */

	int ddsres[4][5] ; /*[N,E,S,W][C,D,H,S,N] */ 
   int crdpos;
   int npos, epos, spos, wpos ;
   int pnum,j;
   int rank, suit ;
   char kard ;  
   char zrd_deal[52]; 
   union cbf_ut zucbf ;  ; 	/* 4 x 2bit fields zucbf.ucbf.cardX */
   /* A zrdLib record starts with Spade Ace, then Spade King, and so on down to Club Deuce */
      suit = SPADES ;
      rank = Ace_rk ;
      kard = MAKECARD(suit, rank ) ;
      npos = 0; epos=13; spos=26; wpos=39 ; /* The slots in the Deal52 buffer where each hand begins. */

      for (crdpos=0 ; crdpos < 13 ; crdpos++ ) {  /* the whole deal is done in 13 bytes 4 cards each byte*/
         zucbf.ucbf = pzrec->zd.cbf[crdpos];
         JGMDPRT(9, "Crdpos=%d, cardByte=%02x\n" ,crdpos, zucbf.uzp4) ;
         for (j = 0 ; j<4 ; j++ ) {  /* each byte holds 4 cards */
            pnum = (zucbf.uzp4 & 0x03 ); /* process the right two bits -- little endian fmt zrd player number*/
            zucbf.uzp4  = zucbf.uzp4 >> 2 ;
            kard = MAKECARD(suit, rank ) ; /* set the current card for this crdpos */
            switch (pnum) {                /* put the current card into the designated hand */
               case zWEST : zrd_deal[wpos++] = kard ; break ;
               case zEAST : zrd_deal[epos++] = kard ; break ;
               case zNORTH: zrd_deal[npos++] = kard ; break ;
               case zSOUTH: zrd_deal[spos++] = kard ; break ;
               default : printf("[%s:%d]Can't happen in switch pnum = %d \n",__FILE__,__LINE__, pnum ) ;
            }
           JGMDPRT(8, " Card %c%c at pos %d:%d assigned to %c; seatcounts: N=%d,E=%d,S=%d,W=%d \n",
                     "CDHS"[suit],rank_id[rank],crdpos,j, "WNES"[pnum],npos,(epos-13),(spos-26),(wpos-39) ) ;
            if (--rank < 0 ) { /* have we done all of current suit?  */
               rank = Ace_rk;    /* then start new suit */
               suit-- ;
            }
         } /* end for J 0 .. 3 */
      } /* end for crdpos 0 .. 12 -- all 52 cards done*/
      memcpy(dl, zrd_deal, 52) ; 
      DBGDO(8, sr_deal_show( (char *)zrd_deal ) );

      memset(ddsres, 0, sizeof(ddsres) );
		zrdTricks_to_dl52Tricks(&pzrec->zt, ddsres ) ;  
		DBGDO(8, show_zrdTricks( &pzrec->zt  ) );  
      JGMDPRT(8, " ----- Record %d all done. npos=%d, epos=%d, spos=%d, wpos=%d --------\n",
										zrdlib_recnum, npos, epos, spos, wpos ) ;
      /* now use the local results to setup the global results area as if we had solved the board in mode 2 */
      dds_res_bin.errcode = 1 ; /* RETURN_NO_FAULT */
      memcpy(dds_res_bin.tricks, ddsres, sizeof(ddsres) ) ;
      dds_res_bin.parScore_NS = 0 ; /* par score will be calculated if input file calls for it. */
      dds_res_bin.CacheStatus = CACHE_OK ;
      dds_dealnum = ngen ;   /* make the cache current */
      deal_err = ZRD_DL_OK;
      return ZRD_DL_OK ;
} /* end zrd_decode_deal */

void zrdTricks_to_dl52Tricks( ZRD_TRICKS_k  *pzt, DL52_TRICKS_k dl52_ddres ) { /* convert zrd tricks to dl52 tricks */

	int s, zs; 
	DBGDO(9,show_zrdTricks( pzt ) );
	for (s=0; s<5 ; s++ ) {  /* clubs to NT */
		zs = zstrain[s] ;								
		dl52_ddres[3][s] = pzt->tbf[zs].trW ;  /* zrd_trix[zstrain].trW to dl52_ddsres[west][strain] */
	   dl52_ddres[0][s] = pzt->tbf[zs].trN ;
	   dl52_ddres[1][s] = pzt->tbf[zs].trE ;
	   dl52_ddres[2][s] = pzt->tbf[zs].trS ;
	   JGMDPRT(5,"ZRD_TX s=%d,zs=%d, :%c: N=%d, E=%d, S=%d, W=%d \n",s,zs,"NSHDC"[zs], 
	   pzt->tbf[zs].trN, pzt->tbf[zs].trE, pzt->tbf[zs].trS, pzt->tbf[zs].trW );
	}
	return ; 
}
/* end zrdTricks_to_dl52Tricks */

int zrd_append_htxt(char *pbuff, struct zrd_hrec_st *phrec ) {  /* concat hdr txt segments into a buffer */
	int snum ;
	int txtpos ; 
	char *pztxt ; 
	snum = phrec->seqnum ; 
	pztxt = phrec->hdrtxt ; 
	txtpos = (snum -1) * 19 ; 
	strncpy( (pbuff + txtpos), pztxt, 19 ) ; /* does not add a null if none exists in the 19 bytes */ 
	return 1 ; 
}
	
/* read a binary record from the zrdlib.zrd database Wrap around if not enough records obtained. Uses zrd globals */
int zrd_getrec (FILE *fzrd, union zrd_rec_ut *uzrec_ptr, struct options_st *opts ) {
   int rc;
   rc = fread_zrdrec( fzrd, uzrec_ptr) ; /* Returns EOFZRD or number of bytes read which should be ZRD_REC_SIZE */
   if (EOFZRD == rc ) {     /* try wrapping around once only; user may have started in middle of file */ 
    	fprintf(stderr, "[%s%d] EOF in Pass Num %d occurred at zrd_cnt=%d, zrdlib_recnum=%d ngen=%d Rewinding File \n",
					__FILE__,__LINE__,zrdlib_pass_num,zrd_cnt,zrdlib_recnum,ngen);
				fseek(fzrd, 0, SEEK_SET ) ;           /* rewind to beginning */
				zrdlib_recnum = 0 ;        				/* recnum tracks errors; want to know actual one in the file */
				zrdlib_pass_num++;							/* main loop will check this and goto EOJ once gets >= 2 */
      
				/* Now try reading first record in file */
				rc = fread_zrdrec( fzrd, uzrec_ptr) ;
				if ( rc != ZRD_REC_SIZE ) { /* Must get 23 bytes from start of file or something is very wrong */
					fprintf(stderr, "After Rewind. zrdlib_pass_num=%d Err=%d. Cannot read first record in Library file. Aborting... \n",
                                          zrdlib_pass_num,  rc);
					exit(-1);
				}
			} /* end if EOFZRD */
				/* assert valid zrd rec here. Deal or Header. Either after rewind or in normal sequence */
		    JGMDPRT(8, "zrd_getrec returning byte_cnt = %d to get_zrdeal recnum[%d]=zrd_rec[%02x %02x] \n",
					rc, zrdlib_recnum, CBF_CAST_CH(uzrec_ptr->zr.zd.cbf[0]),CBF_CAST_CH(uzrec_ptr->zr.zd.cbf[1]) );
		    return rc ; 
} /* end get_zrd_rec */			

void dump_zrdrec(ZRD_REC_k *zrdrec ) { /* dump the bytes in hex */
   int k ;
   union tbf_ut utx ;			/* syntax zutbf.utbf.trX   or zutbf.uzt4 */
   union cbf_ut uzd ;  			/* syntax zucbf.ucbf.cardN or zucbf.uzp4 */
   fprintf(stderr,"ZRDREC_Cards[ ") ;
   for (k=0; k<13; k++ ) {
		uzd.ucbf = zrdrec->zd.cbf[k] ;
      fprintf(stderr, "%02x ", uzd.uzp4 ); 
	}
   fprintf(stderr, " ]:: Tricks WNES:[ " ) ;
   for (k=0; k<5; k++ ) {
		utx.utbf = zrdrec->zt.tbf[k] ; 
      fprintf(stderr, "%c:%04x ","NSHDC"[k], utx.uzt4) ; // 16 bits, 4 trick values W,N,E,S
   }
   fprintf(stderr, "]\n");
} /* end dump_zrdrec*/
/* end dump_zrdrec */

void show_zrdrec(ZRD_REC_k *zrdrec ) { /* decode the bytes in friendly fashion */
	char zsc[4]={'S','H','D','C'};
	char zrc[14]="AKQJT98765432" ; // 13 ranks plus the NULL for EOS
	char zpc[4]={'W','N','E','S'};
	char sep   =':';
	int j ; 
	int p[4] ;
	int k = 0;
	int numcards = 0 ; 
	int zsuit = 0 ; /* start with spades */
	int zrank = 0 ; /* start with the Ace */
	fprintf(stderr,"-------ZRDREC--------\n");
	while(numcards < 52 ) {
		p[0] = zrdrec->zd.cbf[k].card0;  /* get the 4 cards in slot K of zrdrec */
		p[1] = zrdrec->zd.cbf[k].card1;
		p[2] = zrdrec->zd.cbf[k].card2;
		p[3] = zrdrec->zd.cbf[k].card3;
		k++ ;									/* new slot in zrdrec */
		for (j=0; j<4; j++) { /* print 4 cards */
			fprintf(stderr,"%c%c:%c ",zsc[zsuit],zrc[zrank],zpc[p[j]] ) ; //e.g. DJ:S 
			numcards++;
			zrank++ ; 
			if (zrank >= 13) {       /* done 13 cards this suit, start new suit */
				zrank = 0 ; 		 	 /* start new suit with the A */
				zsuit++ ;
				if(zsuit == 2) fprintf(stderr, "\n"); /* Start Diamonds on new line */
			}
		} /* end for j */
		JGMDPRT(9,"Numcards=%d, slotNum=%d \n",numcards,k);
		assert( k <=13 && numcards <= 52  );
	} /* end while numcards ; cards all done */
	fprintf(stderr, "\n ----- Cards Done -------\n");

	fprintf(stderr, "Tricks WNES");
	char eos = ']' ;
	for(k=0; k< 5; k++) { /* show the tricks NT:WNES, S:WNES,H:WNES,D:WNES,C:WNES */ 
		sep = (k < 4 ) ? ':' : ' ';
		fprintf(stderr, "[%c%c","NSHDC"[k],sep );
		fprintf(stderr, "%2d,%2d,%2d,%2d",zrdrec->zt.tbf[k].trW,zrdrec->zt.tbf[k].trN,zrdrec->zt.tbf[k].trE,zrdrec->zt.tbf[k].trS);
		if ( k== 4 ) eos = ' ';
		fprintf(stderr, "%c ", eos);
	} /* end for k < 5 */
	fprintf(stderr, "\n -----  ZRD REC Done -------\n");
} /* end show_zrdrec */
/* end show_zrdrec */	

void show_zrd_sizes(void) {   /*ensure that we are compatible with rpdd.zrd format */
	ZRD_REC_k zrdrec ; 
	fprintf(stderr, "Structure and Type sizes \n");
	fprintf(stderr, "struct cbf_st  =%4ld,  struct tbf_st       =%4ld\n",sizeof(struct cbf_st),sizeof(struct tbf_st));
	fprintf(stderr, "union cbf_ut   =%4ld,  union tbf_ut        =%4ld\n",sizeof(union cbf_ut),sizeof( union tbf_ut));
	fprintf(stderr, "struct zpos_st =%4ld,  typedef ZRD_DEAL_k  =%4ld\n",sizeof(struct zpos_st),sizeof(ZRD_DEAL_k));
	fprintf(stderr, "struct ztrick_st=%4ld, typedef ZRD_TRICKS_k=%4ld\n",sizeof(struct ztrick_st),sizeof(ZRD_TRICKS_k)); 			 
   fprintf(stderr, "struct zrdrec_st=%4ld, typedef ZRD_REC_k   =%4ld\n",sizeof(struct zrdrec_st),sizeof(ZRD_REC_k));
   fprintf(stderr, "       zrdrec.zd=%4ld,         zrdrec.zt   =%4ld\n",sizeof(zrdrec.zd),sizeof(zrdrec.zt));
   fprintf(stderr, "union zrd_rec_ut=%4ld,                          \n",sizeof(union zrd_rec_ut));
   
}

int zrd_err_check( FILE *fzrd, int byte_cnt, int rsize ) {
		size_t  zrd_fpos ;
		if (rsize == byte_cnt ) { return rsize ;} // got what we wanted; should be 23 ZRD_REC_SIZE
      if (feof(fzrd) ) {clearerr(fzrd) ; return EOFZRD; }
      zrd_fpos = ftell(fzrd) ; 
      if (ferror(fzrd) ) { 
				perror("zrd_err_check on ZRD_FILE") ; 
				fprintf(stderr, " IO error on zrd file at fpos=%lu\n",zrd_fpos ) ; 
				clearerr(fzrd) ; 
				return EIOZRD ; 
		}
      
      // This next bit should not happen; ZRD generated file always has  23bytes per record.
      fprintf(stderr, "Error! Bad zrd record.  %d Bytes READ at fpos=%lu \n", byte_cnt, zrd_fpos) ; 
      return EBADZRD  ;
} /* end zrd_err_check */

// init the globals int zrdlib_recs, zrdlib_blksz, zrdlib_recnum, zrdlib_pass_num, zrd_max_seed, zrd_cnt 
int set_zrdlib_vars(FILE *fzrdlib, struct options_st *opts) {
   long int fpos = 0 ;
   int rc ;
   rc = fseek(fzrdlib, 0 , SEEK_END) ;  /* seek to EOF */
   if (rc == 0 ) fpos = ftell(fzrdlib) ; /* what if its not zero? and the seek failed? */
   zrdlib_recs = fpos / ZRD_REC_SIZE ;
   if (zrdlib_recs < 100) {             /* Debug */
      zrdlib_blksz = 1 ;    /* */
      zrd_max_seed = zrdlib_recs / zrdlib_blksz - 1 ;
   }   
   else if (zrdlib_recs < 1000) {             /* Testing Library */
      zrdlib_blksz = 10 ;    /* */
      zrd_max_seed = zrdlib_recs / zrdlib_blksz - 1 ;
   }
   else if (zrdlib_recs < 10000) {  /* partial library */
      zrdlib_blksz = 100 ;
      zrd_max_seed = zrdlib_recs / zrdlib_blksz - 1 ;
   }
   else {                        /* Big or full size Library */
      zrdlib_blksz = ZRD_BLOCKSIZE ;
      zrd_max_seed = (ZRD_MAX_SEED < zrdlib_recs/ZRD_BLOCKSIZE ) ? ZRD_MAX_SEED : zrdlib_recs/ZRD_BLOCKSIZE - 1 ;
   }
   rc = fseek(fzrdlib, 0 , SEEK_SET) ; /* set back to beginning or we get immediate EOF on first read */
   zrdlib_recnum=0;
   JGMDPRT(3,"ZRDLIB Globals. zrdlib_recs=%d, zrdlib_blksz=%d, zrd_max_seed=%d, zrdlib_recnum=%d, Fsize=%ld, libfile=%s\n",
                           zrdlib_recs,zrdlib_blksz,zrd_max_seed,zrdlib_recnum, fpos, opts->zrdlib_fname );
   return zrdlib_recs;
}

long int zrd_seekfpos(FILE *fzrdlib, long int seed ) {
   /* 'seed' var in this case is the number of 1000 record blocks to skip from the start of the file.
    * as the biggest the 'real' file can be is 10,485,760 deals, a seed for the 'real' should be 0 <= seed <= 10,485 
    * When using smaller 'testing' files, the seeds need to be smaller
    */
   long int myseed = seed ;
   if (seed < 0 || seed > zrd_max_seed ) {
      fprintf(stderr, "Seed=%ld must be between zero and %d when using this Library file. Continuing from record zero. \n" ,
            seed, zrd_max_seed ) ;
      myseed = 0 ;
   }
   int rc ;
   long int zrdlib_pos ;
   zrdlib_pos = myseed * zrdlib_blksz * ZRD_REC_SIZE ;
   rc = fseek(fzrdlib, zrdlib_pos, SEEK_SET ) ;
   zrdlib_recnum = myseed * zrdlib_blksz; /* Only inc recnum when we do a read.  We are now before the actual record*/
   JGMDPRT(2,"SEEK_zrdlib: Seed=%ld,recnum = %d, zrdlib=%ld, fseek_rc=%d\n",seed,zrdlib_recnum,zrdlib_pos,rc) ;
   if (rc != 0 ) {
      perror("Seek to pos calculated from seed on zrdlib file FAILED! Reset-ing to start of file (pos=0)\n");
      fprintf(stderr, "Orig-Seed=%ld My-Seed=%ld, calculated position=%ld, calc_blksz=%d, zrdlib_recs=%d \n",
               seed, myseed, zrdlib_pos, zrdlib_blksz, zrdlib_recs ) ;
      zrdlib_pos = 0 ;
      rc = fseek(fzrdlib, zrdlib_pos, SEEK_SET) ;
      if (rc != 0 ) { 
            perror("Recovery seek to start of Library file FAILED!!. Aborting run. \n"); 
            exit(-1);
      }
      zrdlib_recnum = 1 ;  /* using zrdlib_recnum as 1 based; i.e. first record is #1 */
   }

   return zrdlib_pos ;
} /* end zrd_seekfpos */
