/* File metrics_util_subs.c */
/* 2022-12-21 JGM  Redid the misfit_chk routine. Removed the misfit and waste point calculations.
 * 						Added compass[] to prolog
 * 2024-01-26 JGM	 Restructured code into separate functions: Fill_side_fitstat, SetTrumps, SetDeclarer, choose_fit shoose_7fit
 * 
 */
#include "../include/std_hdrs_all.h"
#include "../include/UserEval_types.h"
#include "../include/UserEval_externs.h"
#include "../include/dbgprt_macros.h"
#include "../include/mmap_template.h"
#include "../include/UE_util_protos.h"
#define ISMAJOR(s) ( ( (s) >= HEARTS  ) ? 1 : 0 )
#define ISMINOR(s) ( ( (s) <= DIAMONDS) ? 1 : 0 )
#define WEAKER(x,y) (( (x) < (y) ) ? (x) : (y)  )

  /* external Function Declarations */
extern void Analyze_side( UE_SIDESTAT_k *p_uess, int side ) ;

	/* Forward Function Declarations */
int didxsort4( int v[4], int x[4] ) ;
int dmerge( char *a, char *b, char *c, int aN, int bN ) ;
int dsort4( char a[4] ) ;
int dsort13 (char a[13] ) ;
int isBalanced(HANDSTAT_k *phs ) ;


/* Util Functions Code */
void zero_globals ( int side) { /* zero globals for the current side. do not zero globals for the other side in case of caching? */
   memset(&UEv,    0,sizeof(struct UserEvals_st ) );
   memset(&misfit, 0,sizeof(struct misfit_st) * 4 );
   memset(&TFpts,  0,sizeof(struct FitPoints_st)  );

   side_nt_pts =   0;
   side_hldf_pts = 0;
   misfit_cnt = 0 ;
   h_decl = 0 ; h_dummy = 1 ;
   for (int h = 0 ; h < 2 ; h++ ) {
      hcp[h] = 0 ;
      hcp_adj[h] = 0 ;
      dpts[h] = 0 ;
      lpts[h] = 0 ;
      syn_pts[h] = 0 ;
      hf_pts[h] = 0 ;
      dfit_pts[h] = 0 ;
      TFpts.df_val[h] = 0 ;
      Fn_pts[h] = 0 ;
      TFpts.fn_val[h] = 0 ;
      hand_adj[h] = 0 ;
      body_pts[h] = 0 ;
      seat_hdlf_pts[h] = 0 ;
      seat_nt_pts[h] = 0 ;
      fhcp[h] = 0.0;
      fhcp_adj[h] = 0.0 ;
      for (int s=0 ; s<4 ; s++ ) {
         fhcp_suit[h][s]=0.0; fhcp_adj_suit[h][s]=0.0;
         hf_pts_suit[h][s]=0; syn_pts_suit[h][s]=0; lpts_suit[h][s]=0; dpts_suit[h][s]=0 ;
         freak[h][s] = 0 ; suit_qual[h][s] = 0 ; 
      }
  } /* end for h=0 or 1 */
   JGMDPRT(8, "zero_globals returns OK\n");
  return ;
} /* end zero globals */
void reset_UEsidestat( int side ) {
     /* Zero out those parts of UEsidestat that vary from deal to deal. Leave other fields such as pointers, unchanged */
  UEsidestat[side].t_suit = -1 ; UEsidestat[side].t_rank=0; UEsidestat[side].t_fitlen=0;
  UEsidestat[side].t_len[0]=0; UEsidestat[side].t_len[1]=0;
  UEsidestat[side].decl_h=0; UEsidestat[side].decl_seat=0; UEsidestat[side].dummy_h=1 ; UEsidestat[side].dummy_seat=1 ;
  memset(UEsidestat[side].sorted_slen, 0, 8*sizeof(int) ); memset(UEsidestat[side].sorted_sids, 0, 8*sizeof(int) );
  memset(UEsidestat[side].fitlen, 0, 4*sizeof(int) ); memset(UEsidestat[side].fitids, 0, 4*sizeof(int) ) ;
  UEsidestat[side].pav_body[0] = 0 ; UEsidestat[side].pav_body[1] = 0;

  return ;
}



int arr_min(int arr[], size_t nelem ) {  /* really like to return both the min value and the index that points there. */
   int min = arr[0] ;
   for (int i = 1 ; i < nelem ; i++) {
      if (arr[i] < min ) {min = arr[i] ; }
   }
   return min ;
}
/* These next 2 compare functions could be used in our own sorts, or to call the library quicksort 
 * for these to work they must be modified to cast the input void x and y ptrs to the correct type
 * Here we make px and py ptr->ints, but in other cases we might need ptr->char etc.
 * 2024-08-15: Not used currently?
 */
int asc_cmpxy(  const void *x, const void *y) {
   const int *px, *py ;
   px = x ;
   py = y;
 /* this next bit results in an Ascending sort. to Get descending reverse the < and > symbols, or change 1 to -1 and -1 to 1 */
   if      ( (*px) > (*py) ) { return  1  ; }
   else if ( (*px) < (*py) ) { return -1 ; }
   return 0 ;
}
int desc_cmpxy( const void *x, const void *y) {
   const int *px, *py ;
   px = x ;
   py = y;
 /* this next bit results in an Descending sort. to Get Ascending reverse the < and > symbols, or change 1 to -1 and -1 to 1 */
   if      ( (*px) > (*py) ) { return -1  ; }
   else if ( (*px) < (*py) ) { return  1 ; }
   return 0 ;
}


/*
 * Fill the misfit structure;
 * Called once for each suit; Will use this struct to figure out Misfit pts, and/or waste/nowaste points if any
 * no_waste values are coded: 3 => xxx  vs void,  2 => xxx vs stiff, 1 => Ax(x) vs stiff, 0 there is no short, or there is waste.
 * waste values are coded:   -3 => H(x) vs void, -2 => H(x) vs stiff
 * misfit values are coded:   0 => no misfit, 5 => shortness vs 5+ suit, 4 => short vs 4 card suit, 3 => short vs 3 card suit
 */
struct misfit_st misfit_chk(HANDSTAT_k *phs[], int s ) {   /* Assume Void or Stiff vs 5+ suit; OPC says also xx vs 5+ suit */
   int lh, sh ;
   struct misfit_st mf = {.mf_type = 0, .waste = 0, .no_waste = 0  } ;
   /* misfit fields are the hand-index (0,1) not the compass value (0..3) */
   JGMDPRT(8,"GenCnt=%d, ProdNum=%d. seat[0]=%c, Misfit Check for suit=%d Entered \n", gen_num, prod_num,"NSEW"[seat[0]], s);
      mf.no_waste = 0 ; mf.waste = 0 ; mf.mf_type = 0 ;
      sh = 0 ; lh = 1 ; /* assume   */
      if (phs[0]->hs_length[s] >= phs[1]->hs_length[s] ) { lh = 0 ; sh = 1; }
      /* now proceed to check if the shorter hand has shortness - stiff or void -- in this suit */
      if (0 == phs[sh]->hs_length[s] ) {  /* Is there a void in the sh hand? */
         if      (  (0 == phs[lh]->hs_points[s]) && (phs[lh]->hs_length[s] >= 3) ) { mf.no_waste = 3; } /*xxx vs void */
         else if (   0 <  phs[lh]->hs_points[s] ) { mf.waste = -3 ; } /* wasted values vs void */

         if       (phs[lh]->hs_length[s] >= 5) { mf.mf_type = 5 ; } /* void vs 5+ suit */
         else if  (phs[lh]->hs_length[s] == 4) { mf.mf_type = 4 ; } /* void vs 4+ suit */
         else if  (phs[lh]->hs_length[s] == 3) { mf.mf_type = 3 ; } /* void vs 3+ suit may not apply; or may apply to LAR */
         else     { mf.mf_type = 0 ; }                              /* no misfit vs suits 2 or less */
         mf.short_hand = sh; mf.sh_len = 0 ;
         mf.long_hand = lh;  mf.lh_len = phs[lh]->hs_length[s]; mf.lh_hcp=phs[lh]->hs_points[s];
          JGMDPRT(8,"Hand=%d VOID in %c ; lh_len=%d, lh_hcp=%d, waste=%d, Type=%d, \n",
             mf.short_hand, "CDHS"[s], mf.lh_len, mf.lh_hcp, mf.waste, mf.mf_type );
      } /* end void */
      else if (phs[sh]->hs_length[s] == 1) {  /* Is there a stiff ? */
         if      ((0 == phs[lh]->hs_points[s]) && (phs[lh]->hs_length[s] >= 3) ) { mf.no_waste = 2; } /* xxx vs stiff */
         else if ((4 == phs[lh]->hs_points[s]) && (phs[lh]->Has_card[s][Ace_rk]) && (phs[lh]->hs_length[s] >= 2)) { /* Ax(x) vs stiff*/
                        mf.no_waste = 1;
         } /* end Ax(x) vs stiff */
         else if (0 <  phs[lh]->hs_points[s] ) { mf.waste = -2 ;  } /* Kx(x),Qx(x),Jx(x) vs stiff*/

         if      (phs[lh]->hs_length[s] >= 5) { mf.mf_type = 5 ; } /* stiff vs 5+ suit */
         else if (phs[lh]->hs_length[s] == 4) { mf.mf_type = 4 ; } /* stiff vs 4+ suit */
         else if (phs[lh]->hs_length[s] == 3) { mf.mf_type = 3 ; } /* stiff vs 3+ suit may not apply; or may apply to LAR */
         mf.short_hand = sh;  mf.sh_len = 1 ;
         mf.long_hand  = lh;  mf.lh_len = phs[lh]->hs_length[s];
         mf.lh_hcp=phs[lh]->hs_points[s];
         JGMDPRT(8,"Hand=%d STIFF in %c ; lh_len=%d, lh_hcp=%d, waste=%d, Type=%d, \n",
             mf.short_hand, "CDHS"[s], mf.lh_len, mf.lh_hcp, mf.waste, mf.mf_type );
      } /* end stiff */
      JGMDPRT(8,"Misfit/Waste Check Done:gen_num=%d, suit=%d misfit=%d, waste=%d, no_waste=%d, lh_len=%d, sh_len=%d, lh_hcp=%d \n",
           gen_num, s,  mf.mf_type, mf.waste, mf.no_waste, mf.lh_len, mf.sh_len, mf.lh_hcp ) ;
   return (mf) ;
} /* end misfit_chk() */

/* 6 most likely results and optionally several components results for possible debugging */
void SaveUserVals(struct UserEvals_st UEv , USER_VALUES_k *p_ures ) {
   /* we don't automatically include UEv.t_suit, and UEv.t_fitlen so as not to break existing dli files */
   int j, n ;
   JGMDPRT(9, "SavingUserVals: @UserRes= %p, NT:%d,%d,%d  HLDF:%d,%d,%d Number of Misc=%d\n", (void *)p_uservals,
         UEv.nt_pts_side,UEv.nt_pts_seat[0],UEv.nt_pts_seat[1],
         UEv.hldf_pts_side,UEv.hldf_pts_seat[0],UEv.hldf_pts_seat[1], UEv.misc_count );

   p_ures->u.res[0] = UEv.nt_pts_side ;
   p_ures->u.res[1] = UEv.nt_pts_seat[0] ;
   p_ures->u.res[2] = UEv.nt_pts_seat[1] ;
   p_ures->u.res[3] = UEv.hldf_pts_side ;
   p_ures->u.res[4] = UEv.hldf_pts_seat[0] ;
   p_ures->u.res[5] = UEv.hldf_pts_seat[1] ;
   n = 6 ;
   JGMDPRT(7,"SaveUEvals [0..2]NT=%d,%d,%d [3..5] HLDF=%d,%d,%d\n",
             p_ures->u.res[0], p_ures->u.res[1], p_ures->u.res[2], p_ures->u.res[3], p_ures->u.res[4], p_ures->u.res[5] );
   for (j = 0 ; j < UEv.misc_count ; j++, n++ ) {  /* copy over the misc fields if any */
      p_ures->u.res[n] = UEv.misc_pts[j] ;
      JGMDPRT(9,"Saving Result %d from misc_count %d value=%d\n",n,j, p_ures->u.res[n] );
   }
   if (n < 126 ) {  /* if we have room put the new hldf helper vals in last two slots */
      p_ures->u.res[126] = UEv.hldf_suit;
      p_ures->u.res[127] = UEv.hldf_fitlen;
   }
   return ;
}
//pbn fmt suit sep is dot, hand sep is space. voids are deduced from that.
/* hand MUST be sorted. dl[p*13+0] = Highest Spade; dl[p*13+12] = lowest club. */
char *Hand52_to_pbnbuff (int p, char *dl, char *buff ) {
   char r_ids[] = "23456789TJQKA";
   int curr_suit, card_rank, card_suit;
   int di, count;
   char *bp ;
   unsigned char kard ;
   char suit_sep = '.';
   di = p*13 ;
   bp = buff ;
   count = 0 ;
   dsort13( &dl[di] );  /* ensure the hand is sorted */ 
   curr_suit = 3 ; // spades
   while (count < 13 ) {  // a hand ALWAYS has exactly 13 cards
       kard = dl[di] ; card_suit = C_SUIT(kard); card_rank = C_RANK(kard) ;
       while( curr_suit != card_suit ) { /* write a suit separator for missing suits spades downto first one*/
            *bp++ = suit_sep;
            JGMDPRT(9, "Wrote Void for suit %d \n",curr_suit ) ;
            curr_suit-- ;
        } /* end while curr_suit != card_suit */
        assert(card_suit == curr_suit) ;
        while ( (curr_suit == card_suit) && (count < 13) ) { /* write the cards in this suit */
            kard = dl[di]; card_suit = C_SUIT(kard); card_rank = C_RANK(kard) ;
            if (curr_suit != card_suit ) break;
           *bp++ = r_ids[card_rank];
           count++; di++;
           // JGMDPRT(10," Card Num[%d] in suit = %c%c written to pbnbuff", count, "CDHS"[curr_suit], *(bp-1) ) ;
        } // end while curr_suit
        JGMDPRT(10,"\n");
       *bp++ = suit_sep;
        curr_suit-- ; /* Move to next suit */
    } /* end while count < 13*/
    assert(count == 13 ) ;
    // Normal case curr_suit is -1; void clubs curr_suit = 0, void clubs, diamonds, and hearts curr_suit = 2
    // In case there were voids at the end of 13 cards
        while ( curr_suit >= 0 ) { /* write a suit separator for missing suits after the last one downto clubs*/
            *bp++ = suit_sep ;
            curr_suit-- ;
        }
        /* the last char is the suit separator which we don't need after the club suit, so replace it with a space */
        if ( *(bp-1) == suit_sep ) { *(bp-1) = ' ' ; }
        else { fprintf(stderr, "CANT HAPPEN in Hand52_to_Buff, last char is not a suit_separator %c \n", *(bp-1) ); }
        *bp = '\0' ; // terminate the buffer as a string
        return bp  ; /* return pointer to null byte in case we want to append another hand to the buffer */
} /* end Hand52_to_pbnbuff */

/* no test for jgmDebug since that should be done before calling if reqd */
void show_hands_pbn( int mask, DEAL52_k d ) {
/* Clone of printhands_pbn in action subs.c except always goes to stderr */
  char pt[] = "nesw";
  int  p;
  char pbn_buff[128], *pbn_ptr ;
  pbn_ptr=pbn_buff ;
  for (p=0 ; p< 4; p++ ) {
     if ( !(mask & 1 << p ) ) continue ; /* skip this player if he was not called for */
    *pbn_ptr++ = pt[p]; *pbn_ptr++ = ' ' ; // player names are followed by a space */
    pbn_ptr = Hand52_to_pbnbuff (p, (char *)d, pbn_ptr ); // append a hand to end of pbnbuff; returns ptr to null at end.
  }
  /* pbnbuff formatted now print it out */
  *pbn_ptr++ = '\n'; *pbn_ptr='\0';
  fprintf(stderr, "%s",pbn_buff ) ;

} /* end show_hands_pbn */

void swap(int* xp, int* yp) {
   int temp = *xp;
   *xp = *yp;
   *yp = temp;
}
int dsort_i4( int a[4] ) { /*  optimized version no error checking used to sort lengths and hcp */
   int t ;
         if (*(a+1) > *(a) ) { t=*(a+1) ; *(a+1) = *(a) ; *(a) = t ; }      // sort first two elements; *(a) now > *(a+1)
         if (*(a+2) > *(a) ) { t=*(a+2); *(a+2)=*(a+1);*(a+1)=*(a);*(a)=t;}     // 3rd elem to *(a); shuffle other two up one
         else if (*(a+2) > *(a+1) ) {t=*(a+2); *(a+2)=*(a+1); *(a+1) = t ; }      // swap 2nd and 3rd elems

         if      (*(a+3) >= *(a) ) { t=*(a+3);*(a+3)=*(a+2);*(a+2)=*(a+1);*(a+1)=*(a);*(a)=t; } // shuffle all up one place
         else if (*(a+3) >= *(a+1) ) { t=*(a+3);*(a+3)=*(a+2);*(a+2)=*(a+1);*(a+1)=t; }         // shuffle 1..3 up one place
         else if (*(a+3) >  *(a+2) ) { t=*(a+3);*(a+3)=*(a+2);*(a+2)=t;}                        // swap *(a+2) with *(a+3)
	return 0 ; 
} /* end dsort4 */
int didxsort4( int v[4], int x[4] ) { 
	/* descending idx sort where we sort the suits in order based on a suit property/val (v) such as len or hcp etc.
	 * could also be used to sort the hands in (desc) order based on the number of spades, etc.
	 * we compare the 'property' to determine if to swap or not; 
	 * if we do swap 'v', we swap both the val/poperty pair and also the two indices 'x'
	 */
	 // v is the value/property array, x is the index array. x starts out as 0,1,2,3 always.
   char t ; int s ;
       if (*(v+1) > *(v) ) {      // sort first two elements; *(v) now > *(v+1)
			 t=*(v+1) ; *(v+1) = *(v) ; *(v) = t ; 
			 s=*(x+1) ; *(x+1) = *(x) ; *(x) = s ; 
		 } 
       if (*(v+2) > *(v) ) {     // 3rd elem to *(v); shuffle other two up one
			 t=*(v+2); *(v+2)=*(v+1);*(v+1)=*(v);*(v)=t;
			 s=*(x+2); *(x+2)=*(x+1);*(x+1)=*(x);*(x)=s;
		 } 
       else if (*(v+2) > *(v+1) ) {     // swap 2nd and 3rd elems
			 t=*(v+2); *(v+2)=*(v+1); *(v+1) = t ;
			 s=*(x+2); *(x+2)=*(x+1); *(x+1) = s ;  
		 } 

       if      (*(v+3) >= *(v) ) {   	// shuffle all up one place
			 t=*(v+3);*(v+3)=*(v+2);*(v+2)=*(v+1);*(v+1)=*(v);*(v)=t; 
			 s=*(x+3);*(x+3)=*(x+2);*(x+2)=*(x+1);*(x+1)=*(x);*(x)=s; 
		 } 
       else if (*(v+3) >= *(v+1) ) {  // shuffle 1..3 up one place 
			 t=*(v+3);*(v+3)=*(v+2);*(v+2)=*(v+1);*(v+1)=t;
			 s=*(x+3);*(x+3)=*(x+2);*(x+2)=*(x+1);*(x+1)=s;  
		 }           
       else if (*(v+3) >  *(v+2) ) {   // swap *(v+2) with *(v+3)
			 t=*(v+3);*(v+3)=*(v+2);*(v+2)=t;
			 s=*(x+3);*(x+3)=*(x+2);*(x+2)=s;
		 }                      
	return 0 ; 
} /* end didxsort4 */
int dmerge( char *a, char *b, char *c, int aN, int bN ) { /* no error checking; a, b, c must point to valid arrays of char */
   int i = 0 ; int j = 0 ; int k = 0 ;
   int kmax =aN + bN ;
   while (k < kmax) {
      if      ( i == aN )     { *(c + k++) = *(b + j++);  }
      else if ( j == bN )     { *(c + k++) = *(a + i++);  }
      else if (a[i] > b[j] )  { *(c + k++) = *(a + i++);  }
      else                    { *(c + k++) = *(b + j++);  }
   }
   assert (i == aN && j == bN ) ;
   return 0 ;
}
int dsort4( char a[4] ) { /*  optimized version no error checking used to sort cards */
   char t ;
         if (*(a+1) > *(a) ) { t=*(a+1) ; *(a+1) = *(a) ; *(a) = t ; }      // sort first two elements; *(a) now > *(a+1)
         if (*(a+2) > *(a) ) { t=*(a+2); *(a+2)=*(a+1);*(a+1)=*(a);*(a)=t;}     // 3rd elem to *(a); shuffle other two up one
         else if (*(a+2) > *(a+1) ) {t=*(a+2); *(a+2)=*(a+1); *(a+1) = t ; }      // swap 2nd and 3rd elems

         if      (*(a+3) >= *(a) ) { t=*(a+3);*(a+3)=*(a+2);*(a+2)=*(a+1);*(a+1)=*(a);*(a)=t; } // shuffle all up one place
         else if (*(a+3) >= *(a+1) ) { t=*(a+3);*(a+3)=*(a+2);*(a+2)=*(a+1);*(a+1)=t; }         // shuffle 1..3 up one place
         else if (*(a+3) >  *(a+2) ) { t=*(a+3);*(a+3)=*(a+2);*(a+2)=t;}                        // swap *(a+2) with *(a+3)
	return 0 ; 
} /* end dsort4 */
/* Use this sort for the normal Hand sort to put the cards in desc order */
int dsort13 (char a[13] ) { /* Desc Sort used to sort a hand. optimized version does not check for errors */
   char u[13], v[13], w[13] ;
   dsort4(&a[0] );       					
   dsort4(&a[4] );       					
   dsort4(&a[8] );       					
   dmerge(&a[0], &a[4], u , 4 , 4 ) ;  		
   dmerge(&u[0], &a[8], v , 8 , 4 ) ;  		
   dmerge(&v[0], &a[12],w , 12, 1 ) ;  
   memcpy(a , w, 13 ) ;
   return 0 ;
} /* end dsort13 */
void sortDeal(DEAL52_k dl ) { // Sort each hand in the deal in Desc order; helps printouts esp PBN/GIB ones and "has_card" funcs
    int p ;
    char *h_ptr ;
    for (p = 0 ; p < 4 ; p++ ) { /* p is the player, 0=North, etc. We sort each quarter of the deal separately */
        h_ptr = (char *) &dl[p*13];
        JGMDPRT(7, "Sorting hand[%d] starting with card [%02x] -> [%02x]\n", p, dl[p*13], *h_ptr );
        dsort13((char *) h_ptr)  ;
    }
} /* end sortDeal */

void sr_deal_dump(DEAL52_k real_dl ) { /* two line output of the cards in SuitRank */
    char rns[] = "23456789TJQKA-";
    char sns[] ="CDHS";
    char rn, sn , sp;
    int s, r , i ;
    DEAL52_k dl ;
    memcpy(dl, real_dl, 52) ;
    sortDeal(dl) ;
    i = 0;
    fprintf (stderr," NOCARD=[%02x] Showing Deal using CARD_SUIT and CARD_RANK in sr_deal_dump\n",NO_CARD);
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
} /* end sr_deal_dump */

void dump_curdeal( void ) { /* output deal in easy to read, verbose fmt Uses the Has_card[][] array in hand stat NOT DEAL52_k dl */
    int  pnum , cardcount;
    char *pname="NESW";
    char *sname="CDHS";
    char *rname="23456789TJQKA";
    int r, s;
    char sid ;
    pnum = 0 ;
    /* using globals hs_ptr[4] to access the hand stat */

    cardcount = 0;
    char pn, rn ;
    pn='*'; rn='-';
    /* this is already a debugging routine; no need to use DBGPRT -- all output goes to stderr */
    fprintf(stderr, "Debugging Deal in dump_curdeal; ngen=%d, nprod=%d\n", ptrs.dldata->curr_gen, ptrs.dldata->curr_prod );
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
              if(hs_ptr[pnum]->Has_card[s][r] ) {
                  fprintf(stderr, "%c%c ",sid, rn );
                  cardcount++ ;
              } /* end if haskard */
            } /* end for r = ACE */
            fprintf (stderr, "}\n");
        } /* end for s suit */
        fprintf(stderr, "Done for player%d=%c, found=%d\n", pnum, pn, cardcount);
        assert(cardcount == 13 );  /* must find 13 cards for each player */
  } /* end for player */
  fprintf(stderr, "----------------dump curr deal done ----------------\n");
  fsync(2) ;
} /* end dump curr deal */
float_t QuickTricks_suit(HANDSTAT_k *phs , int s ) {
      int s_len, w1, w2 ;
      s_len = phs->hs_length[s] ;
      w1 = phs->topcards[s][0] ;          /* Highest card might be just 1 for a spot */
      w2 = phs->topcards[s][1] + w1 ;     /* Top two cards; might be just 2 for two spots */
      if ( 0 == s_len ) {return 0.0 ;  }
      if ( 1 == s_len ) {
         switch ( w1 ) {
            case 64 : return 1.0 ; /* stiff A */
            case 32 : return 0.0 ; /* stiff K = 0.5 ??? */
            default : return 0.0 ;
         }
      } /* end len == 1 */
      else if (2 <= s_len ) {
         switch ( w2 ) {
            case 96 : return 2.0 ;  /* AK */
            case 80 : return 1.5 ;  /* AQ */
            case 72 : return 1.0 ;  /* AJ */
            case 68 : return 1.0 ;  /* AT */
            case 65 : return 1.0 ;  /* Ax */
            case 48 : return 1.0 ;  /* KQ */
            case 40 : return 0.5 ;  /* KJ */
            case 36 : return 0.5 ;  /* KT */
            case 33 : return 0.5 ;  /* Kx */
            default : return 0.0 ;
         }
      } /* end 2<= len */
      return 0.0 ; /* NOT REACHED */
}
float_t QuickTricks(HANDSTAT_k *phs ) {
   int s ;
   float_t qtrix = 0.0;
   for (s=CLUBS ; s<=SPADES; s++ ) {
      qtrix += QuickTricks_suit( phs, s ) ;
      JGMDPRT(8,"QuickTricks: suit=%d, RunningTotQT=%g\n",  s, qtrix ) ;
   } /* end for each suit */
   return qtrix ;
} /* end QuickTricks */

/* Honor combinations, and length => playing tricks Used when Pre-Empting at 3+ level*/
float_t Pav_playing_tricks (HANDSTAT_k *phs ) {
   float_t ptrix = 0.0 ;
   int w1, w2, w3 ; /* weights of the top 3 cards in the suit; */
   int s;
   int s_len ;
   for (s=CLUBS ; s<=SPADES; s++ ) {
      s_len = phs->hs_length[s] ;
      w1 = phs->topcards[s][0] ;          /* Highest card might be just 1 for a spot */
      w2 = phs->topcards[s][1] + w1 ;     /* Top two cards; might be just 2 for two spots */
      w3 = phs->topcards[s][2] + w2 ;     /* Top three cards; might be just 3 for three spots */
      if ( 0 == s_len ) { continue ; }
      if ( 1 == s_len ) {
         switch ( w1 ) {
            case 64 : ptrix += 1.0 ; continue ; /* stiff A */
            case 32 : ptrix += 0.5 ; continue ; /* stiff K */
            default : continue ;
         }
      } /* end len == 1 */
      else if (2 == s_len ) {
         switch ( w2 ) {
            case 96 : ptrix += 2.0 ; continue ; /* AK */
            case 80 : ptrix += 1.5 ; continue ; /* AQ */
            case 72 : ptrix += 1.5 ; continue ; /* AJ */
            case 68 : ptrix += 1.0 ; continue ; /* AT */
            case 65 : ptrix += 1.0 ; continue ; /* Ax */
            case 48 : ptrix += 1.0 ; continue ; /* KQ */
            case 40 : ptrix += 0.5 ; continue ; /* KJ */
            case 36 : ptrix += 0.5 ; continue ; /* KT */
            case 33 : ptrix += 0.5 ; continue ; /* Kx */
            case 24 : ptrix += 0.5 ; continue ; /* QJ */
            case 20 : ptrix += 0.5 ; continue ; /* QT */
            case 17 : ptrix += 0.5 ; continue ; /* Qx */
            default : continue ;
         }
      } /* end len == 2 */
      else { /* len >= 3 */
         switch ( w3 ) {
            case 112 : ptrix += 3.0 ; break ;    /* AKQ */
            case 104 : ptrix += 2.5 ; break ;    /* AKJ */
            case 100 : ptrix += 2.0 ; break ;    /* AKT */
            case  97 : ptrix += 2.0 ; break ;    /* AKx */
            case  88 : ptrix += 2.5 ; break ;    /* AQJ */
            case  84 : ptrix += 2.0 ; break ;    /* AQT */
            case  81 : ptrix += 1.5 ; break ;    /* AQx */
            case  76 : ptrix += 1.5 ; break ;    /* AJT */
            case  73 : ptrix += 1.5 ; break ;    /* AJx */
            case  69 : ptrix += 1.0 ; break ;    /* ATx */
            case  66 : ptrix += 1.0 ; break ;    /* Axx */
            case  56 : ptrix += 2.0 ; break ;    /* KQJ */
            case  52 : ptrix += 1.5 ; break ;    /* KQT */
            case  49 : ptrix += 1.0 ; break ;    /* KQx */
            case  44 : ptrix += 1.5 ; break ;    /* KJT */
            case  41 : ptrix += 1.0 ; break ;    /* KJx */
            case  37 : ptrix += 0.5 ; break ;    /* KTx */
            case  34 : ptrix += 0.5 ; break ;    /* Kxx */
            case  28 : ptrix += 1.0 ; break ;    /* QJT */
            case  25 : ptrix += 1.0 ; break ;    /* QJx */
            case  21 : ptrix += 0.5 ; break ;    /* QTx */
            case  18 : ptrix += 0.5 ; break ;    /* Qxx */
            case  13 : ptrix += 0.5 ; break ;    /* JTx */
            default : break ;
         } /* end switch ( w3 ) */
         /* Now add playing tricks for extra length */
         if (s_len > 3 ) { ptrix += s_len - 3 ; } /* extra playing trick for each card over 3 */
      } /* end else len >= 3 */
      JGMDPRT(8,"PAV_PlayingTricks: suit=%d, slen=%d, w1=%d, w2=%d, w3=%d, RunningTotPtrix=%g\n",
                                    s, s_len, w1, w2, w3, ptrix ) ;
   } /* end for each suit */

   return ptrix ;
} /* end PAV_playing_tricks */

/* Check if we have the controls for a NT slam; we are not considering shortness as a control in this routine. *
 * Note that this is a rudimentary check; to see if we have one (or more) uncontrolled suits
 */
#define NOT_OK 0
#define OK 1
int NT_slam_ok (HANDSTAT_k *phs[2] ) {
   int s;
   int first_ctls = 0 ;
   int sec_ctls   = 0 ;
   int ctled_suits= 0 ;
   for (s=CLUBS; s<=SPADES ; s++ ) {
      if (phs[0]->hs_control[s] == 0 && phs[1]->hs_control[s] == 0 ) { return NOT_OK ; }
      if (phs[0]->hs_control[s] >= 2 || phs[1]->hs_control[s] >= 2 ) { first_ctls++ ; ctled_suits++; continue ; }
      sec_ctls++ ; ctled_suits++ ; 
   }
   /* might have AKx/Axx/Axx/xxxx where first ctls == 3 and 2nd ctls == 1 but not in fourth suit. */
   if (ctled_suits < 4 ) { return NOT_OK ; }
   
   if ( first_ctls == 4 ) { return OK ; }
   if ( first_ctls <  3 ) { return NOT_OK ; } /* 4 controlled suits might be Kxx, Kxx, Kxx, Kxx */
   if ( first_ctls == 3 && sec_ctls == 0 ) { return NOT_OK ; }
   return OK ; /* 3 suits with at least first round control, and 4th suit with second round control */
} /* end NT_slam_ok */


void show_sorted_slen(UE_SIDESTAT_k *p_ss ) {
   int h, s ;
   fprintf(stderr, "%s:%d::ShowSortedSlen\n", __FILE__, __LINE__ ); 
  for (h=0 ; h < 2 ; h++ ) {
    fprintf(stderr, "\tHand[%d]: ", h);
    for (s = 0  ; s < 4 ; s++ ) {
      fprintf(stderr, "Idx=%d Suit=%c Suitlen=%d : ", s, "CDHS"[p_ss->sorted_sids[h][s]] , p_ss->sorted_slen[h][s] ) ; 
    }
    fprintf(stderr, "\n");
  }
  return ; 
} /* end show_sorted_slen */
void show_sorted_fits(UE_SIDESTAT_k *p_ss ) {
    int s ;
    fprintf(stderr,"%s:%d::Show_Sorted_Fits:: ", __FILE__, __LINE__ ); 
    for (s = 0  ; s < 4 ; s++ ) {
      fprintf(stderr, "Idx=%d FitSuit=%c Fitlen=%d : ", s, "CDHS"[p_ss->fitids[s]] , p_ss->fitlen[s] ) ; 
    }
    fprintf(stderr, "\n");
    return ; 
} /* end show_sorted_fits */


int isBalanced(HANDSTAT_k *phs ) {
	int dbltons =  0 ;
	if(phs->square_hand ) { return 1 ; } /* 4333 Balanced */
	for (int s=0 ; s<4 ; s++ ) {
		if ( phs->hs_length[s] < 2 ) { return 0 ; } /* stiff or void Not balanced */
		if ( phs->hs_length[s] > 5 ) { return 0 ; } /* 6+ suit. Not balanced */
		if ((phs->hs_length[s] == 2) && (++dbltons > 1 ) ) { return 0 ; } /* 2+ Dbltons eg 5422, 6322, Not Balanced */
	}
	return 1 ; /* balanced */
}
/* end is balanced */

/* Pavlicek 'Body' count. >= 12 typically has 'Body' */
int Pav_body_val( HANDSTAT_k  *p_hs ) {
   int s;
   int body = 0 ;
   for (s=0; s<4 ; s++ ) {
      if (p_hs->Has_card[s][EIGHT] ) body++ ;
      if (p_hs->Has_card[s][NINE]  ) body += 2;
      if (p_hs->Has_card[s][TEN]   ) body += 3; 
   }
   // JGMDPRT(8, "PAV BODY p_hs=%p, returns %d\n", (void *)p_hs, body ) ; 
   return( body ) ; /* if >= 12 Good Body. Round Up */
} /* end Pav_body_val */

int Dotnum2Int( int dotnum , int body ) {
   int q , r ;
   int scale = 100 ;    /* All dotnums in dealer are scaled by 100. e.g. Ace=4.25 becomes Ace=450; ditto KnR/CCCC pts, LTC etc. */
   q = dotnum / scale ;
   r = dotnum % scale ;
   if ( r > 55 || (45 <= r && r <= 55 && body  >= 12 ) ) { q++ ; } /* round up */
   /* NOTREACHED */
   return q ;
} /* end Dotnum2Int	*/

int Pav_round(float_t val, int body ) {
   int p, q , r ; 
   q = val ;                /* floor of val */
   r = (val - q ) * 100.0 ; /* convert remainder to int between 0 .. 100 */
   p = q ;                    /* p = floor of val */
   if ( r > 55 || (45 <= r && r <= 55 && body  >= 12 ) )  { p = q+1; } /* round up */
   return p;
}
void show_user_res(char *caller, USER_VALUES_k *p_results, int first, int last ) {
   int r ;
   fprintf(stderr, "SHOWING USER RESULTS from caller=[%s] at %p from %d to %d \n", caller, (void *) p_results, first, last ) ; 
   for ( r = first; r <= last ; r++ ) {
      fprintf(stderr, "[%d]=%d%c", r, p_results->u.res[r], ( ((r +1) % 10) == 0 ) ? '\n' : ' ' );
   }
   fprintf(stderr, "\n");
   return ;
}

void show_set40_res( struct EvalAll_res_st ue40_res ) {
   fprintf(stderr, "NT: %d = %d + %d ; HLDF: %d = %d + %d \n",
      ue40_res.nt_pts_side, ue40_res.nt_pts_seat[0], ue40_res.nt_pts_seat[1],
      ue40_res.bf_pts_side, ue40_res.bf_pts_seat[0], ue40_res.bf_pts_seat[1] );
   return ;
}

void show_UEsidestat( UE_SIDESTAT_k *p_ss ) { /* called with DBGDO(n, show_UEsidestat(p_ss) */
   fprintf(stderr, "UEsidestat for side=%d\n",p_ss->side ) ;
   fprintf(stderr, "\t t_suit=%d, t_rank=%d, t_fitlen=%d, t_len=[%d:%d], Decl=[%d:%d], Dummy=[%d,%d] \n",
            p_ss->t_suit, p_ss->t_rank, p_ss->t_fitlen, p_ss->t_len[0],p_ss->t_len[1], 
                                                               p_ss->decl_h,p_ss->decl_seat,p_ss->dummy_h,p_ss->dummy_seat );
   fprintf(stderr,"\tH=0 Sorted SuitLen:id=[%d:%d] [%d:%d] [%d:%d] [%d:%d]\n", p_ss->sorted_slen[0][0],p_ss->sorted_sids[0][0],
            p_ss->sorted_slen[0][1],p_ss->sorted_sids[0][1], p_ss->sorted_slen[0][2],p_ss->sorted_sids[0][2], p_ss->sorted_slen[0][3],p_ss->sorted_sids[0][3] );
  fprintf(stderr,"\tH=1 Sorted SuitLen:id=[%d:%d] [%d:%d] [%d:%d] [%d:%d]\n", p_ss->sorted_slen[1][0],p_ss->sorted_sids[1][0],
            p_ss->sorted_slen[1][1],p_ss->sorted_sids[1][1], p_ss->sorted_slen[1][2],p_ss->sorted_sids[1][2], p_ss->sorted_slen[1][3],p_ss->sorted_sids[1][3] );
  fprintf(stderr,"\t    Sorted FitLen:id [%d:%d] [%d:%d] [%d:%d] [%d:%d]\n",p_ss->fitlen[0], p_ss->fitids[0],
            p_ss->fitlen[1], p_ss->fitids[1],p_ss->fitlen[2], p_ss->fitids[2],p_ss->fitlen[3], p_ss->fitids[3] );
  fprintf(stderr, "\t Pav_body=[%d:%d] Handstat_ptrs=%p   %p\n",p_ss->pav_body[0],p_ss->pav_body[1], (void *)p_ss->phs[0], (void*)p_ss->phs[1] ) ;
  return  ;
}

void set_dbg_names(int m_num, char *funcname ) { /* set the names of the non-HCP metrics; Bergen, Bissel ... ZarBas */
   char curr_metric_name[32];
   char dbg_func_name[32];
   strncpy(dbg_func_name, funcname, (sizeof(dbg_func_name) - 1) ) ;
   strncpy(curr_metric_name, metric_names[m_num], sizeof(curr_metric_name) -1 ) ;
   return ;
}
