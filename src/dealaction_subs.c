/* File dealaction_subs.c   */
/* Date      Version Author   Description
* 1990/12/31 1.0.0   Origin   Original code
* 2021/12/31 2.0.0   JGM      Modified for DealerV2. Fix Avg; Add bktfreq; etc. 
* 2022-02-18 2.1.0   JGM      CSVRPT and PRINTRPT
* 2024/07/03 2.2.0   JGM      Mods to Frequency fmt for bigger numbers, and Pcts for 1D freq.
* 2024/09/07 2.2.1   JGM      malloc/calloc BUG workaround.
* .                  .        .        
*/ 
#ifndef _GNU_SOURCE
  #define _GNU_SOURCE
#endif

#ifndef DDPGM
  #define DDPGM /usr/games/gibcli
#endif
#include "../include/std_hdrs.h"
#include "../include/dealdefs.h"
#include "../include/dealtypes.h"
#include "../include/dealexterns.h"
#include "../include/dealprotos.h"
#include "../include/dealdebug_subs.h"
#include "../include/deal_bktfreq_subs.h"
#include "../include/dbgprt_macros.h"
#include "../include/deal_conversion_subs.h"
#include "../include/libdealerV2.h"

extern void rpt_alloc( void ) ;
/* Code for New Actions by JGM */
void printside (DEAL52_k d, int side ) ;
void printhands_pbn(FILE *fp, int mask, DEAL52_k curdeal ) ;
int unmask(int m) ;		/* m is a bit mask, one bit set - return the int corresponding to that bit; in effect lg(m) */

void do_csvrpt(FILE *fcsv, struct csvterm_st *csvptr ) ;

/* Action Oriented Procedures */
void setup_action () { /* run once right after the parsing done */
  struct action *acp;
  JGMDPRT(6, "Setup Action Entered\n");


  /* Initialize all actions */
  for (acp = actionlist; acp != 0; acp = acp->ac_next) {
    switch (acp->ac_type) {
      default:
        assert (0); /*NOTREACHED */
      case ACT_PRINTCOMPACT:
      case ACT_PRINTPBN:
      case ACT_PRINTEW:
      case ACT_PRINTNS:
      case ACT_PRINTSIDE:
      case ACT_PRINTALL:
      case ACT_PRINTONELINE:
      case ACT_PRINTES:
      case ACT_CSVRPT :
      case ACT_PRINTRPT :
        break;
      case ACT_PRINT: /* if there are several ACT_PRINT actions we dont want to do several mallocs. Memory leak? */
        if( NULL == deallist ) {
				deallist = (DEAL52_k *) mycalloc (maxproduce+1, sizeof (DEAL52_k));
				JGMDPRT(3, "Setup Action ACT_PRINT (maxproduce+1)*52 =%d,  malloc succeeded @dealist=%p\n",
					(maxproduce+1)*52, (void *)deallist );
			}
        break;
      case ACT_AVERAGE:
        acp->ac_int1 = 0 ; // this holds the sum  so far.
        acp->ac_u.acuavg.count = 0; // same as nprod
        acp->ac_u.acuavg.sum = 0 ; // same as ac_int1
        acp->ac_u.acuavg.sqsum = 0 ; //
        acp->ac_u.acuavg.avg = 0.0; // double float pt. qty should be approx 0.5 for rnd()
        acp->ac_u.acuavg.vary= 0.0; // double float pt. qty variance. should be approx 1/sqrt(12) = 0.28867 for rnd()
        JGMDPRT(3, "Average Init Done. ac_u.acuavg.sqsum=%ld In setup_action\n",((long int) acp->ac_u.acuavg.sqsum));
        break;
      case ACT_FREQUENCY:
        acp->ac_u.acu_f.acuf_freqs = (long *) mycalloc (
           acp->ac_u.acu_f.acuf_highbnd - acp->ac_u.acu_f.acuf_lowbnd + 1,
         sizeof (long));
        break;
      case ACT_FREQUENCY2D:
        acp->ac_u.acu_f2d.acuf_freqs = (long *) mycalloc (
           (acp->ac_u.acu_f2d.acuf_highbnd_expr1 - acp->ac_u.acu_f2d.acuf_lowbnd_expr1 + 3) *
           (acp->ac_u.acu_f2d.acuf_highbnd_expr2 - acp->ac_u.acu_f2d.acuf_lowbnd_expr2 + 3),
           sizeof (long));
        break;
      case ACT_EVALCONTRACT:
        /* as of 2023-01-18 each action_tree of type EvalContract carries its own set of counters
         * These are initialized in the code in dealyacc.y that creates the action node
         */

        break;
      case ACT_EXP_SIDE_HLD :    /* export the side's two hands in 'holding' fmt */
      case ACT_EXP_SEAT_HLD :    /* export the seat's hand in 'holding' fmt */
        break ;
      case ACT_BKTFREQ:
        acp->ac_u.acu_bf.bkt_freqs  = alloc_bkts1D(&acp->ac_u.acu_bf.bkt); /* set bkt.Names ptr as a side effect */
        fill_bkt_names(&acp->ac_u.acu_bf.bkt) ;
        break;
      case ACT_BKTFREQ2D:  /* next call sets bktd.Names and bkta.Names ptrs as a side effect */
        acp->ac_u.acu_bf2d.bkt_freqs = alloc_bkts2D(&acp->ac_u.acu_bf2d.bktd, &acp->ac_u.acu_bf2d.bkta ) ;
        fill_bkt_names(&acp->ac_u.acu_bf2d.bktd) ;
        fill_bkt_names(&acp->ac_u.acu_bf2d.bkta) ;
//        if (acp->ac_u.acu_bf2d.bkta.Num > max_bkta_Num ) max_bkta_Num = acp->ac_u.acu_bf2d.bkta.Num  ; /* calloc BUG */
        break;
      } /* end switch acp type */
    } /* end for acp->next action list */
} /* end setup_action */

void action () {            /* For each 'Interesting' deal, Walk the action_list and do the actions requested */
  struct action *acp;
  int expr0, expr1, expr2, val1, val2, high1 = 0, high2 = 0, low1 = 0, low2 = 0;
  // char *expbp = export_buff ;
  double  dcount, dsum, dsqsum ;

  int actionitem = 0;  /* Debugging tracer var */
  JGMDPRT(8," .... Just entered Action() actionitem=%d\n",actionitem );
  sortDeal(curdeal) ; /* JGM sort each hand, Spade Ace downto Club deuce. Simplifies many actions. Now uses dsort13 */

  for (acp = actionlist; acp != 0; acp = acp->ac_next) {
    actionitem++;
    JGMDPRT(6, "ActionItem [%3d] of type [%3d] \n", actionitem,acp->ac_type);
    switch (acp->ac_type) {
      default:
        assert (0); /*NOTREACHED */
      case ACT_PRINTCOMPACT:
        printcompact (curdeal);   // could be sped up now that hands are sorted
        if (acp->ac_expr1) {
          expr0 = evaltree (acp->ac_expr1);
          printf ("%d\n", expr0);
        }
        break;
      case ACT_PRINTONELINE:
        printoneline (curdeal);   // could be sped up now that hands are sorted
        if (acp->ac_expr1) {
          expr0 = evaltree (acp->ac_expr1);
          printf ("%d", expr0);
        }
        printf ("\n");
        break;
      case ACT_PRINTES:
        { struct expr *pex = (struct expr *) acp->ac_expr1;
          while (pex) {
            if (pex->ex_tr) {
              expr0 = evaltree (pex->ex_tr);
              printf ("%d", expr0);
            }
            if (pex->ex_ch) {
              printf ("%s", pex->ex_ch);
            }
            pex = pex->next;
          }
        }
        break;
      case ACT_PRINTALL:
        printdeal (curdeal);
        break;
      case ACT_PRINTEW:
        printside (curdeal, SIDE_EW);
        break;
      case ACT_PRINTNS:
        printside (curdeal, SIDE_NS);
        break;
      case ACT_PRINTSIDE:
        printside (curdeal, acp->ac_int1 );
        break;
      case ACT_PRINTPBN:
        if (! quiet)
          printpbn (nprod + boardoffset, curdeal);
        break;
      case ACT_PRINT:
        memcpy (&deallist[nprod-1], curdeal, sizeof (DEAL52_k));  /*save sorted curdeal in malloc'ed area for later printing */
        JGMDPRT(6,"nprod=%d,dealsize=%ld,@deallist[0]=%p,@deal_dest=%p\n",nprod,sizeof(DEAL52_k),(void *)&deallist[0], (void *)(&deallist[nprod-1]) );
        break;
      case ACT_AVERAGE:  /* mods by JGM to a) have a running avg, and b) include Variance as well as avg in final rpt */
        expr0 = evaltree (acp->ac_expr1);   /* calculate the expr we are averaging */
        acp->ac_int1 += expr0 ;                 // old average running sum
        acp->ac_u.acuavg.count++;              // should be same as nprod, or perhaps nprod+1;
        acp->ac_u.acuavg.sum   += expr0 ;       // should be same as ac_int1
        acp->ac_u.acuavg.sqsum += expr0*expr0 ;
        dcount = (double) acp->ac_u.acuavg.count;
        dsum   = (double) acp->ac_u.acuavg.sum;
        dsqsum = (double) acp->ac_u.acuavg.sqsum;
        if (acp->ac_u.acuavg.count > 1 ) {
            acp->ac_u.acuavg.avg  = dsum / dcount ;
            acp->ac_u.acuavg.vary = (dsqsum*dcount - dsum*dsum)/(dcount*(dcount-1.)); // Sqrt this at end of run
      /* var(x) = [sum(x*x) - (sum(x)*sum(x))/n]/(n-1) .. for uniform dist approx 1/12 */
        }
        else { /* avoid division by zero */
           acp->ac_u.acuavg.avg  = dsum ;
           acp->ac_u.acuavg.vary = dsqsum - dsum*dsum ;
        }
        JGMDPRT(7, "Action_ptr=%p, Count=[%ld], Average Expr=[%d], Sum=[%ld], Sumsq=[%ld], RunningAvg=[%g]\n",
           (void *)acp, acp->ac_u.acuavg.count, expr0, ((long int) acp->ac_u.acuavg.sum), ((long int) acp->ac_u.acuavg.sqsum), dsum/dcount );
        break;
      case ACT_FREQUENCY:
        expr0 = evaltree (acp->ac_expr1);
        if (expr0 < acp->ac_u.acu_f.acuf_lowbnd)
          acp->ac_u.acu_f.acuf_uflow++;
        else if (expr0 > acp->ac_u.acu_f.acuf_highbnd)
          acp->ac_u.acu_f.acuf_oflow++;
        else
          acp->ac_u.acu_f.acuf_freqs[expr0 - acp->ac_u.acu_f.acuf_lowbnd]++;
        break;
      case ACT_FREQUENCY2D:
        expr0 = evaltree (acp->ac_expr1);
        expr2 = evaltree (acp->ac_expr2);

        high1 = acp->ac_u.acu_f2d.acuf_highbnd_expr1;
        high2 = acp->ac_u.acu_f2d.acuf_highbnd_expr2;
        low1 = acp->ac_u.acu_f2d.acuf_lowbnd_expr1;
        low2 = acp->ac_u.acu_f2d.acuf_lowbnd_expr2;
        if (expr0 > high1)
          val1 = high1 - low1 + 2;
        else {
          val1 = expr0 - low1 + 1;
          if (val1 < 0) val1 = 0;
        }
        if (expr2 > high2)
          val2 = high2 - low2 + 2;
        else {
          val2 = expr2 - low2 + 1;
          if (val2 < 0) val2 = 0;
        }
        acp->ac_u.acu_f2d.acuf_freqs[(high2 - low2 + 3) * val1 + val2]++;
        break;
      case ACT_EVALCONTRACT :
        evalcontract(acp) ; /*Save the DDtricks for the given side & strain on this deal */
        break;
      case ACT_EXP_SIDE_HLD :
         init_export_buff(export_buff, sizeof(export_buff) );
         Side52_export(export_buff, acp->ac_int1, curdeal ) ;
         fprintf(fexp, "%s\n", export_buff );
         break ;
      case ACT_EXP_SEAT_HLD :
         init_export_buff(export_buff, sizeof(export_buff) );
         Hand52_export(export_buff, acp->ac_int1, curdeal ) ;
         fprintf(fexp, "%s\n", export_buff );
         break ;
     case ACT_CSVRPT:         /* print a new line, of many terms, to the csvrpt file */
        { struct csvterm_st *csvptr = (struct csvterm_st *) acp->ac_expr1; // ptr to list of csvterms
			 do_csvrpt(fcsv, csvptr ) ;
      } /* end case ACT_CSVRPT */
        break;
        /* Next is duplicate of csvrpt but to stdout; allows for both output to file and to screen. */
     case ACT_PRINTRPT:         /* Re-use csvcode identical except for output file destination  */
        { struct csvterm_st *csvptr = (struct csvterm_st *) acp->ac_expr1; // ptr to list of csvterms
			  do_csvrpt(stdout, csvptr) ; 
        } /* end case ACT_PRINTRPT */
        break;
     case ACT_BKTFREQ:
        expr1 = evaltree (acp->ac_expr1);
        upd_bkt1D( expr1, acp->ac_u.acu_bf.bkt_freqs, &acp->ac_u.acu_bf.bkt ) ;
        break;
     case ACT_BKTFREQ2D:
        expr1 = evaltree (acp->ac_expr1);
        expr2 = evaltree (acp->ac_expr2);
        upd_bkt2D( expr1, expr2, acp->ac_u.acu_bf2d.bkt_freqs, &acp->ac_u.acu_bf2d.bktd, &acp->ac_u.acu_bf2d.bkta ) ;
        break;
      } /* end switch acp action_type */
   } /* end for acp action list */
} /* end action() i.e. process the action list */

void cleanup_action () {  /* this also does the end-of-run actions like FREQUENCY, AVERAGE, EVALCONTRACT, & PRINT */
  struct action *acp;
  int player, i, n;
  double d_var, dsum, dsumsq, pct, cumpct;
  long int colsum, freqval ;
  DBGDO(5, rpt_alloc( ) );

  for (acp = actionlist; acp != 0; acp = acp->ac_next) {
    switch (acp->ac_type) {
      default:
        assert (0); /*NOTREACHED */
        /* These next actions  need no cleanup */
      case ACT_PRINTALL:
      case ACT_PRINTCOMPACT:
      case ACT_PRINTPBN:
      case ACT_PRINTEW:
      case ACT_PRINTNS:
      case ACT_PRINTSIDE:
      case ACT_PRINTONELINE:
      case ACT_PRINTES:
      case ACT_EXP_SIDE_HLD:
      case ACT_EXP_SEAT_HLD:
      case ACT_CSVRPT :
      case ACT_PRINTRPT :
        break;
      case ACT_EVALCONTRACT:
        showevalcontract (acp, nprod);
        break;
      case ACT_PRINT:
        for (player = COMPASS_NORTH; player <= COMPASS_WEST; player++) {
            /*Note acp->ac_int1 is a bit mask listing each COMPASS we want to print. Each one on separate page  */
            /* e.g. N & W -> 1001 , E & W -> 1010, N & S & E -> 0111 and so on. */
            /* We dont get to choose what order they are printed in  */
            /* so print (north, west) and print (west, north) will both print all the North hands before all the West hands */

          if (!(acp->ac_int1 & (1 << player))) continue;  /* next player */
          if (title_len > 0 ) { printf("%s\n", title ) ; }
          printf ("\n%s hands:\n\n", player_name[player]);
          for (i = 0; i < nprod; i += 4)  /* print hands 4 across at a time. */
                printhands (i+boardoffset, deallist+i, player, nprod - i > 4 ? 4 : nprod - i);
          printf ("\f"); /* end each player with a form feed */
        }  /* end for player */
        break;
      case ACT_AVERAGE:
        if (acp->ac_str1)  printf ("%s: ", acp->ac_str1);
         /* var(x) = [sum(x*x) - (sum(x)*sum(x))/n]/(n-1) */
        dsum   = (double) acp->ac_u.acuavg.sum;
        dsumsq = (double) acp->ac_u.acuavg.sqsum;
        n = ( acp->ac_u.acuavg.count > 1 ) ? ( acp->ac_u.acuavg.count-1 ) : 1 ;
        d_var =  ( dsumsq - dsum*dsum/acp->ac_u.acuavg.count ) / n ;
         // printf ( "Average= %g\n", (double) acp->ac_int1 / nprod);
        // this next print statement is extra. may replace the above.
        printf("Mean=%10.4f, Std Dev=%10.4f, Var=%10.4f, Sample Size=%ld \n",
              (double)acp->ac_u.acuavg.sum/acp->ac_u.acuavg.count,
              sqrt(d_var), d_var, acp->ac_u.acuavg.count );

         JGMDPRT(4, "Original Average=%g\n", (double) acp->ac_int1 / nprod);
         JGMDPRT(4,"Running Totals: avg=%10.4f, sqrt(vary)=%10.4f, vary=%10.4f, count=%ld \n",
              acp->ac_u.acuavg.avg, sqrt(acp->ac_u.acuavg.vary), acp->ac_u.acuavg.vary, acp->ac_u.acuavg.count );
        break;
      case ACT_FREQUENCY: 
        printf ("\nDescription: %s:\n", acp->ac_str1 ? acp->ac_str1 : "");
        printf ("Value\t   Count\t   Pct.\t  CumPct\n");
        /* create a total */
        colsum = 0 ;
        cumpct = 0.0 ;
        if (acp->ac_u.acu_f.acuf_uflow) { colsum += acp->ac_u.acu_f.acuf_uflow ; }
        for (i = acp->ac_u.acu_f.acuf_lowbnd; i <= acp->ac_u.acu_f.acuf_highbnd; i++) {
            colsum += acp->ac_u.acu_f.acuf_freqs[i - acp->ac_u.acu_f.acuf_lowbnd] ;
        }
        if (acp->ac_u.acu_f.acuf_oflow) { colsum += acp->ac_u.acu_f.acuf_oflow ; }
        dsum = (double) colsum /100.0 ;  // pre-do the *100 once for all rows by dividing by 100 here. better 100.0/colsum then multiply?

        /*Print the table */
        if (acp->ac_u.acu_f.acuf_uflow > 0 ) { // only print Low if not zero
            pct = (double)acp->ac_u.acu_f.acuf_uflow/dsum ;
            cumpct += pct ; 
            printf ("Low\t%8ld\t%7.2f\t%7.2f\n", acp->ac_u.acu_f.acuf_uflow, pct, cumpct ); // Mult here?
        }
        for (i = acp->ac_u.acu_f.acuf_lowbnd; i <= acp->ac_u.acu_f.acuf_highbnd; i++) {
            freqval = acp->ac_u.acu_f.acuf_freqs[i - acp->ac_u.acu_f.acuf_lowbnd] ;
            pct = (double)freqval/dsum ;
            cumpct += pct ; 
            printf ("%5d\t%8ld\t%7.2f\t%7.2f\n", i, freqval, pct, cumpct );
        }
        if (acp->ac_u.acu_f.acuf_oflow > 0) {  // only print overflow if not 0
            pct = (double)acp->ac_u.acu_f.acuf_oflow/dsum ;
            cumpct += pct ; 
           printf ("High\t%8ld\t%7.2f\t%7.2f\n", acp->ac_u.acu_f.acuf_oflow, pct, cumpct );
        }
        printf("    \t--------\t-------\nTotal\t%8ld\t%7.2f\n", colsum, 100.0 ) ; 
        break;
      case ACT_FREQUENCY2D: {  // Note Freq2D always prints the Low and High Row and Col even if all zeroes
        int j, n = 0, low1 = 0, high1 = 0, low2 = 0, high2 = 0, sumrow,
          sumtot, sumcol;
        printf ("\nDescription: %s:\n", acp->ac_str1 ? acp->ac_str1 : "");
        high1 = acp->ac_u.acu_f2d.acuf_highbnd_expr1;
        high2 = acp->ac_u.acu_f2d.acuf_highbnd_expr2;
        low1 = acp->ac_u.acu_f2d.acuf_lowbnd_expr1;
        low2 = acp->ac_u.acu_f2d.acuf_lowbnd_expr2;
        // print the col headings
        printf ("        Low");
        for (j = 1; j < (high2 - low2) + 2; j++) printf (" %8d", j + low2 - 1);
        printf ("    High     Sum%s", crlf);
        sumtot = 0;
        for (i = 0; i < (high1 - low1) + 3; i++) {  // process the rows
            sumrow = 0;
            // print the row name
            if (i == 0)                       printf ("Low ");
            else if (i == (high1 - low1 + 2)) printf ("High");
            else                              printf ("%4d", i + low1 - 1);
            // process each col
            for (j = 0; j < (high2 - low2) + 3; j++) {
               n = acp->ac_u.acu_f2d.acuf_freqs[(high2 - low2 + 3) * i + j];
               sumrow += n;
               printf (" %8d", n);
            } // end for cols
            printf (" %8d%s", sumrow, crlf);
            sumtot += sumrow;
        } // end for rows
        // print the total row
        printf ("Sum ");
        for (j = 0; j < (high2 - low2) + 3; j++) { // for each col
            sumcol = 0;
            for (i = 0; i < (high1 - low1) + 3; i++) {  // generate a col total
               sumcol += acp->ac_u.acu_f2d.acuf_freqs[(high2 - low2 + 3) * i + j];
            }
            printf (" %8d", sumcol);
        } // end for each col
        printf (" %8d%s%s", sumtot, crlf, crlf);   // print grand total bottom right corner
        break ;
      } /* end FREQ2D case line 333 */
      case ACT_BKTFREQ:
        show_freq1D (acp->ac_u.acu_bf.bkt_freqs, acp->ac_str1 ? acp->ac_str1 : "", &acp->ac_u.acu_bf.bkt, 'd' ) ;
        break;
      case ACT_BKTFREQ2D:
      
        show_freq2D(acp->ac_u.acu_bf2d.bkt_freqs, acp->ac_str1 ? acp->ac_str1 : "",
                      &acp->ac_u.acu_bf2d.bktd, &acp->ac_u.acu_bf2d.bkta ) ;
        break ;
     } /* end switch ( acp->ac_type ) line 270 approx */
  } /* end for acp action list line 269 */
  DBGDO(3,rpt_alloc( ) ); /* to cover the cases when col_totals calloc'd during cleanup phase */
} /* end cleanup_action line 264 */

/* Has_card array mow makes this simpler */
void fprintcompact (FILE * f, DEAL52_k d, int ononeline) {  /* this is the routine used to spec to GIB */
  char pt[] = "nesw";
  int s, p, r;
  for (p = COMPASS_NORTH; p <= COMPASS_WEST; p++) { /*after each hand print newline (for compact) or not (oneline)*/
    fprintf (f, "%c ", pt[p]);
    for (s = SUIT_SPADE; s >= SUIT_CLUB; s--) {
      for (r = 12; r >= 0; r--)  { /* r goes from Ace to deuce This loop would give an enum problems*/
        if ( HAS_CARD ( (p), (s), (r) ) ) {  /* Look up the card in the Has_card array */
          fprintf (f, "%c", ucrep[r]);
        }
      }
      if (s > 0) fprintf (f, ".");  /* dont print a dot after clubs, the last suit printed */
    } /* end for suit */
    fprintf (f, ononeline ? " " : "\n");
  } /* end for p = compass */
}   /* end printcompact -- The one line version does not put a \n at the end of output. allows integer expr to follow */

void printdeal (DEAL52_k d) {   /* the PRINTALL action. Print All 4 hands on the fly. 4 across*/
  int suit, player, rank, cards;
  static int do_title = 1 ;
  if (do_title == 1 && title_len > 0 ) {
      printf("%18s[%s]\n"," ",title);
      do_title = 0;
  }
  printf ("%4d. North              East                South               West \n", (nprod + boardoffset) );

  for (suit = SUIT_SPADE; suit >= SUIT_CLUB; suit--) {
    printf("     "); /* indent on page to allow room for board number */
    cards = 10;
    for (player = COMPASS_NORTH; player <= COMPASS_WEST; player++) {
      while (cards < 10) {
        printf ("  ");
        cards++;
      }
      cards = 0;
      for (rank = 12; rank >= 0; rank--) {
        if ( HAS_CARD(player, suit, rank) ) { 
          printf ("%c ", ucrep[rank]);
          cards++;
        }
      } /* end for rank */
      if (cards == 0) {
        printf ("- ");
        cards++;
      }
    } /* end for player */
    printf ("\n");
  }  /* end for suits */
  printf ("\n");
  if ( (((nprod) % 10) == 0) && (title_len > 0)) { printf("\f"); do_title = 1 ; } /* 10 deals per page then a form feed and another title */
}  /* end printdeal */

    /*     ----- REDO ------
     * The print(compass list) action: print a given player's hands. At action_cleanup time*/
    /* 'n-hands' across at a time (n=4 usually) from an array of hands saved during the generation phase
     * This routine could be made much more efficient by using the fact that the hands are sorted.
     */
void printhands (int boardno, DEAL52_k *dealp, int player, int nhands) {

  int i, suit, rank, cards;
  JGMDPRT(6,"printhands called with boardno=%d, player=%d, nhands=%d, dealptr=%p \n",boardno,player,nhands,(void *)dealp);
  for (i = 0; i < nhands; i++) {
    printf ("%4d.%15c", boardno + i + 1, ' '); /* print board number as a heading */
  }
  printf ("\n");
  for (suit = SUIT_SPADE; suit >= SUIT_CLUB; suit--) {
    cards = 10;  /* left justify the first hand */
    for (i = 0; i < nhands; i++) {
       JGMDPRT(6,"hand=%d,dealp=%p\n",i,(void *) &dealp[i] );
      while (cards < 10) {  /* spaces until next hand column */
        printf ("  ");
        cards++;
      }
      cards = 0;
      for (rank = 12; rank >= 0; rank--) { /* call the slow version of hascard since the handstat array not current */
        if (hasKard (dealp[i], player, MAKECARD (suit, rank))) {
          printf ("%c ", ucrep[rank]);
          cards++;
        }
      }
      if (cards == 0) {
        printf ("- ");
        cards++;
      }
    } /* end for hands */
    printf ("\n");
  }
  printf ("\n");
} /* end printhands */

/* REWRITE TO USE SORTED DEAL THEN use Deal52_to_GIB to create the text string */
void printside (DEAL52_k d, int side ) {  /* JGM Replacement for printew to allow NS also and with title, board#s player names */
     /* This function prints a partnerships hands (N/S or E/W with S/W on the left)
        primarily intended for examples of auctions with 2 players only.  HU & JGM  */
  static int do_title = 1 ;
  int suit, player, rank, cards;
  int players[2] ;
  int pnum;
  if   ( side == 0 ) { players[0] = COMPASS_SOUTH; players[1] = COMPASS_NORTH ; }
  else               { players[0] = COMPASS_WEST;  players[1] = COMPASS_EAST ;  }
  if (do_title == 1 && title_len > 0 ) {
      printf("%18s[%s]\n"," ",title);
      do_title = 0;
  }
  printf ("%4d. %-5s              %-5s\n", (nprod + boardoffset), player_name[players[0]], player_name[players[1]] );
  JGMDPRT(7,"Printing parnership side [%d] players=[%s , %s]\n",
                                     side, player_name[players[0]], player_name[players[1]]);
  for (suit = SUIT_SPADE; suit >= SUIT_CLUB; suit--) {
    printf("     "); /* indent on page to allow room for board number */
    cards = 10;
    for (pnum = 0 ; pnum < 2 ; pnum++) {
       player=players[pnum];
       while (cards < 10) { /* if we are doing the second player, pad the line */
          printf ("  ");
          cards++;
        }
        cards = 0;   /* reset card count for this player */
         /* HAS_CARD now just checks the Has_card[][] array in handstat. avoids brute force scanning */
        for (rank = 12; rank >= 0; rank--) {
            if ( HAS_CARD(player, suit, rank) ) {
            printf ("%c ", rank_id[rank]);
            cards++;
          }
        } /* done one suit for this player */
        if (cards == 0) {  /* this player had a void in this suit */
          printf ("- ");
          cards++;
        }
        /* done a(nother) player. go to the next player or next suit; */
        /* done a suit for both players -- move on to next suit */
    } /* end for pnum = 0 or 1 */
    printf("\n");
  } /* end for suit = spades downto clubs */
   printf ("\n");
   if ( (((nprod) % 10) == 0) && (title_len > 0)) { printf("\f"); do_title = 1 ; } /* 10 deals per page then a form feed and another title */
}  /* end print side */

int printpbn (int board, DEAL52_k d) {  /* Rudimentary PBN report primarily to exg with others */
                                    /* the variables maxdealer and maxvulnerable will show up here */
  /* Symbols for the cards */
  char representation[] = "23456789TJQKA";
  /* Mnemonics for vulnerability and dealer */
  char *vulner_name[] = { "None",  "NS",   "EW",    "All"  };
  char *dealer_name[] = { "N",     "E",    "S",     "W"    };
  /* Who's vulnerable on which boards */
  int board_vul[] = { 0,1,2,3, 1,2,3,0, 2,3,0,1, 3,0,1,2 };

  /* Local variables */
  time_t timet;
  // size_t len;
  char timearray[12];
  int player, suit, rank;
  int board_idx  = board - 1 ; /* offset into the VUL and Dealer name arrays */ 

  printf ("[Event \"Hand simulated by DealerV2_4 with file %s, seed %lu\"]\n",  input_file, seed);
  printf ("[Site \"-\"]\n");
  /* next two optional tags, not part of PBN Minimal Tag Set -- , added by JGM */
  if (strlen(title) > 0 ) { printf("[Description \"%s\"]\n", title); }
  printf("[Generator \"Dealer Version 2.4 by Hans, Henk, and JGM\"]\n");

  /* Today's date */
  timet = time(&timet);
  strftime (timearray, 12, "%Y.%m.%d", localtime(&timet));
  printf ("[Date \"%s\"]\n", timearray);

  printf ("[Board \"%d\"]\n", board);

  /* Blank tags for the players */
  printf ("[West \"-\"]\n");
  printf ("[North \"-\"]\n");
  printf ("[East \"-\"]\n");
  printf ("[South \"-\"]\n");

  /* Dealer, rotates unless set by the user */
  if ((maxdealer < 0) || (maxdealer > 3)) {
     printf ("[Dealer \"%s\"]\n", dealer_name[board_idx%4]);
  } else {
    printf ("[Dealer \"%s\"]\n", dealer_name[maxdealer]);
  }

  /* Vulnerability, rotates unless set by the user */
  if ((maxvuln < 0) || (maxvuln > 3)) {
     printf ("[Vulnerable \"%s\"]\n", vulner_name[board_vul[board_idx%16]]);
  } else {
     printf ("[Vulnerable \"%s\"]\n", vulner_name[maxvuln]);
  }
  /* JGM 2021/12/06 -- Vers 2.1 (2017) of the PBN std says that the first player in the Deal tag
   * should be the Dealer. So the following code should be changed to rotate the order of the
   * cards output. Given that there is already a Dealer name output I don't think it's necessary
   */

  /* Print the cards */
  printf ("[Deal \"N:");
  for (player=COMPASS_NORTH; player<=COMPASS_WEST; player++) {
     for (suit = SUIT_SPADE; suit>= SUIT_CLUB; suit--) {
        for (rank=12; rank >= 0; rank--) {
          if ( HAS_CARD(player,suit,rank) ) {
              printf ("%c", representation[rank]);
           }
        }
        if (suit > SUIT_CLUB) { printf (".");}
     }
     if (player < COMPASS_WEST) {printf (" ");}
  }
  printf ("\"]\n");

  /* Blank tags for declarer etc */
  printf ("[Declarer \"?\"]\n");
  printf ("[Contract \"?\"]\n");
  printf ("[Result \"?\"]\n");
  printf ("\n");
  return 0;
} /*end printpbn */

/* All this Eval Contract stuff completely redone by JGM 2021 and again in 2023 See Docs*/

void show_evalres( struct action *acp ) { /* Debug routine; called inside if jgmDebug statement */
   int lvl, side, strain ;
   struct contract_st *ac_cp ;
   ac_cp = &acp->ac_u.acucontract ;
   side = acp->ac_int1 ;
   strain = ac_cp->strain;
   fprintf(stderr, "Prod Num=%d :: EvalContract Counts for side=%d, strain=%d Contract=%s[ ",nprod,side,strain,ac_cp->c_str);
   for (lvl=0; lvl < 14 ; lvl++ ) { fprintf(stderr,"%2d ", ac_cp->trix[lvl] ) ; }
   fprintf(stderr, "]\n");
}
/*
 * Show the average score obtained, the make pct and the fail pct for the contract.
 * Show also the missed game and slam percents, and the cost per board of missing said games and slams
 */
void showevalcontract (struct action *acp, int nh) {
  int ddtricks , uscore, dscore, tricks_cnt, score_tot, success_cnt, fail_cnt ;
  double avg_score, success_pct, fail_pct, missed_game_pct, missed_slam_pct, G_imps_per_board, S_imps_per_board ;
  int s, l, v, dbl, side, coded, game_lvl, missed_games, missed_slams, missed_G_imps, missed_S_imps ;
  struct contract_st *ac_cp ;
  char side_str[2][3] = {"NS", "EW"};
  side = acp->ac_int1 ;
  s    = acp->ac_u.acucontract.strain;
  l    = acp->ac_u.acucontract.level ;
  v    = acp->ac_u.acucontract.vul ;
  dbl  = acp->ac_u.acucontract.dbl ;
  coded= acp->ac_u.acucontract.coded;
  ac_cp = &acp->ac_u.acucontract ;
  if      (s > SPADES )   {game_lvl = 3 ; } /* must be NT */
  else if (s > DIAMONDS ) {game_lvl = 4 ; } /* must be a Major */
  else                    {game_lvl = 5 ; }
  missed_games  = 0 ;
  missed_G_imps = 0 ;
  missed_S_imps = 0 ;
      /* -- check to see if there was a missed game or slam */
  if (l < game_lvl) {
     for (ddtricks= game_lvl+6; ddtricks < 14 ; ddtricks++ ) {
        missed_games  += ac_cp->trix[ddtricks] ;
        missed_G_imps += ac_cp->trix[ddtricks]*imps(undbled_score(v,s,game_lvl, ddtricks) - undbled_score(v,s,l,ddtricks) ) ;
        JGMDPRT(4, "DDlvl=%d,GameLvl+6=%d,Occurs=%d,GameScore=%d,contractScore=%d\n",
        ddtricks, game_lvl+6, ac_cp->trix[ddtricks], undbled_score(v,s,game_lvl, ddtricks), undbled_score(v,s,l,ddtricks) );
     }
  }
  if ( l < 6 ) {
     missed_slams  = ac_cp->trix[12] + ac_cp->trix[13] ;
     missed_S_imps = ac_cp->trix[12] * imps( undbled_score(v,s,6, 12) - undbled_score(v,s,l,12) ) +
                     ac_cp->trix[13] * imps( undbled_score(v,s,7, 13) - undbled_score(v,s,l,13) ) ;
  }

  JGMDPRT(4,"EvalContract acp=%p,ac_cp=%p, trix[8]=%d missed_Games=%d\n",
                        (void *)acp, (void *)ac_cp, ac_cp->trix[8], missed_games );
  // fmt_contract_str(acp->ac_u.acucontract.c_str, l,s,dbl,v);  now done in the yacc file. can be used for dbg
        // Total score in this contract over the course of the run.
        score_tot   = 0 ;
        success_cnt = 0 ;
        fail_cnt    = 0 ;
        JGMDPRT(4, "Contract=::[%s] NumberOfHands=%d, Level=%d,Suit=%d,Vul=%d , Dbl=%d ..\n",
            acp->ac_u.acucontract.c_str, nh, l, s, v, dbl );
    if (0 == dbl ) {
        for (ddtricks = 0; ddtricks < 14; ddtricks++) {  // For tricks taken DD 0..13
          uscore = undbled_score (v, s, l, ddtricks);
          tricks_cnt  = ac_cp->trix[ddtricks];     /* we saved the results of DD analysis for this Side's contract(s) */
          score_tot +=  uscore * tricks_cnt ;           /* uscore * number of times it occurred */
          if (uscore > 0 ) { success_cnt += tricks_cnt ; }
          if (uscore < 0 ) { fail_cnt    += tricks_cnt ; }
          if (tricks_cnt > 0 ) { /* interested only in cases that occurred */
               JGMDPRT(5, "\tddtricks=%d, tricks_cnt=%d, uscore=%d, succ_cnt=%d, fail_cnt=%d  \n",
                                             ddtricks, tricks_cnt, uscore, success_cnt, fail_cnt) ;
          }
        }
        JGMDPRT(4,  "Undoubled COntract Tot_Score=%d, SuccessTot=%d, FailTot=%d, NH=%d \n",
                                                score_tot, success_cnt, fail_cnt, nh ) ;
    }  /* end undoubled */
    else {  // contract was dbled or redbled
        for (ddtricks = 0; ddtricks < 14; ddtricks++) {  // For tricks taken DD 0..13
            dscore = score(v, coded, ddtricks );
            tricks_cnt  = ac_cp->trix[ddtricks];     /* we saved the results of DD analysis for this Side's contract(s) */
            score_tot +=  dscore * tricks_cnt ;           /* uscore * number of times it occurred */
            if (dscore > 0 ) { success_cnt += tricks_cnt ; } // There is no such thing as a score of zero in bridge!
            if (dscore < 0 ) { fail_cnt    += tricks_cnt ; }
        }
         JGMDPRT(4, "Doubled/RDBLD Contract Tot_Score=%d, SuccessTot=%d, FailTot=%d, NH=%d \n",
                                                score_tot, success_cnt, fail_cnt, nh ) ;
     } /* end doubled or redoubled */
        avg_score   = (double)(score_tot)/nh ;
        success_pct = (double)(100*success_cnt)/nh ;
        fail_pct    = (double)(100*fail_cnt)/nh ;
        missed_game_pct = (double)(100.0*missed_games)/nh;
        missed_slam_pct = (double)(100.0*missed_slams)/nh;
        G_imps_per_board = ((double)missed_G_imps)/nh;
        S_imps_per_board = ((double)missed_S_imps)/nh;

        printf("\nContract %6s by %2s Avg Result =% 8.2f, Made pct=% 7.2f, Fail pct=% 7.2f\n",
                acp->ac_u.acucontract.c_str, side_str[side], avg_score, success_pct, fail_pct );
        printf("Missed Game_pct=% 7.2f, Missed Slam_pct=% 7.2f, Missed Game Imps/board=% 7.2f, Missed Slam Imps/board=% 7.2f\n",
        missed_game_pct, missed_slam_pct,  G_imps_per_board, S_imps_per_board );
}
/* end showevalcontract */

void evalcontract(struct action *acp ) { /* Updates the counts if the deal is 'interesting' */
  struct contract_st *ac_cp ;
  int strain, side ;
  ac_cp = &acp->ac_u.acucontract ;
  strain = ac_cp->strain ;
  side = acp->ac_int1 ;

  int ddtricks;
//    ddtricks = dd(curdeal, side, strain ) ; // using GIB
    if(0 == side ) {
      ddtricks = dds_tricks( 2, strain ) ;  // if NS make S declarer
    }
    else { ddtricks = dds_tricks( 3, strain ) ; } // if EW make W declarer

    ac_cp->trix[ddtricks]++ ; /* add +1 to the number of times ddtricks were taken in this contract */
	 DBGDO(6, show_evalres(acp) );
    return ;
} /* end evalcontract */

void printhands_pbn( FILE *fp, int mask, DEAL52_k d ) {   // No newline at end of print. Caller to put \n or comma as reqd.
/* print the hands in the same format as printoneline does but option to print 1,2,3,or 4 hands */
/* Hands are always printed in order of N,E,S,W but some may be omitted e.g. might be only E and S */
/* also do not print newline at end. */
/* sample pbnfuff n .AKQJ987.5432.A3 e A5432..JT98.KQJ2 s KQJ.6543.AKQ.T98 w T9876.T2.76.7654 */
/* note that voids are not shown explicitely with a - sign, but instead can be inferred from the dots. */
/* A space indicates end of a hand. */

  char pt[] = "nesw";
  int  p;
  char pbn_buff[128], *pbn_ptr ;
  pbn_ptr=pbn_buff ;
  for (p=0 ; p< 4; p++ ) {
     if ( !(mask & 1 << p ) ) continue ; /* skip this player if he was not called for */
    *pbn_ptr++ = pt[p]; *pbn_ptr++ = ' ' ; // player names are followed by a space */
    pbn_ptr = Hand52_to_PBNbuff (p, (char *)d, pbn_ptr ); // append a hand to end of pbnbuff; returns ptr to null at end.
  }
  /* pbnbuff formatted now print it out */
  fprintf(fp, "%s",pbn_buff ) ; /* no newline at this point. */

} /* end printhands_pbn */

void do_csvrpt(FILE *fcsv, struct csvterm_st *csvptr ) { 
			char sep = ' ';
			int  vuln  ;
			int  expr0 ; 
			
			sep = ' ' ; 				/* precede first item with a space; all susequent ones with commas */
          while (csvptr) {  		/* walk the list of csvterms until we reach end */
             /* take appropriate action for this kind of csvterm. Only one should be set per term in the list */
            if (csvptr->csv_tr) {  /* term is some kind of expression */
              expr0 = evaltree (csvptr->csv_tr); /* returns int only */
              fprintf (fcsv, "%c%d", sep, expr0);          /* dont need quotes around numbers */
              sep = ',';
              JGMDPRT(4, "doCSVRPT expr= %d\n", expr0 );
            } // end if csvptr->csv_tr
            if (csvptr->csv_str) {  /* term is some kind of string. we put single quotes around it in case it has commas */
               fprintf (fcsv, "%c'%s'", sep, csvptr->csv_str); /*put quotes around a string in case it contains commas */
               sep = ',';
               JGMDPRT(4,  "CSVRPT Str== %s\n",csvptr->csv_str );
            } // end if string csvptr->csv_str
            if (csvptr->csv_hands) { /* print the requested hands, in gibpbn format ; order is N,E,S,W */
               fprintf(fcsv, "%c",sep) ; /* leave printhands_pbn as a generic routine; no leading comma or trailing \n*/
               printhands_pbn(fcsv, csvptr->csv_hands, curdeal ) ; /* csv_hands is a bit mask of compasses to print */
               sep = ',' ;
               JGMDPRT(4,  "CSVRPT Print Hands. Hands_mask= %d\n",csvptr->csv_hands );
            }  // end if hands
            if (csvptr->csv_trix) {  /* user wants tricks in all 5 strains for some set of hands */
               csv_trixbuff_len = csv_trix(csv_trixbuff, csvptr->csv_trix) ; //fmt a buff with comma sep trick counts
               fprintf(fcsv,"%c%s",sep, csv_trixbuff) ;
               sep = ',';
               JGMDPRT(4,  "CSVRPT Print Tricks. HandMask= %d\n",csvptr->csv_hands );
            }
            if (csvptr->csv_parvul) {
					vuln = unmask(csvptr->csv_parvul) ;
					vuln = (0 <= vuln && vuln <= 3 ) ? vuln : options.par_vuln ; /* use default value if vuln out of range */
					dds_parscore(0, vuln) ;  /* dummy call to ensure that the parcontracts have been calculated for this deal */ 
					fprintf(fcsv, "%c'%s'",sep,dds_res_bin.ParContracts[vuln] ) ;
					sep = ',' ;
               JGMDPRT(4,  "CSVRPT Print ParContract parvul=%d\n",csvptr->csv_parvul );
				}
            sep = ',';  // this one should replace all the others above
            csvptr = csvptr->next;
          } /* end while csvptr -- reached end of csvlist */
         JGMDPRT(4, "doCSVRPT end of list printing NewLine\n" );
         fprintf(fcsv, "\n") ;  /* print a newline after the last item is done */
         sep = ' '; /*re-init for next line in csv report */
         return ; 
} /* end do_csvrpt */



