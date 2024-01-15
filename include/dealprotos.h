/* File dealprotos.h -- define function prototypes both those used in actions from Flex and Yacc and also ones used by main */
/* 2023/01/07 -- Merged in changes from V4 to fix predeal; dealcards_subs.c and globals, etc. 
 * 2023/10/27 -- added bias_len, bias_totsuit in prep for functioning bias_deal
 * 2023/11/04 -- Removing proto definitions for functions in the library; get these via include libdealerV2.h 
 */

#ifndef DEALPROTOS_H
#define DEALPROTOS_H
#ifndef _GNU_SOURCE
  #define _GNU_SOURCE
#endif
#include <stdio.h>
#include <stdlib.h>
#include "dealtypes.h"  /* will include dealdefs etc. */

/* interfaces to various dealerv2 library modules  these are all in ../include/libdealerV2.h*/
#include "dbgprt_macros.h"
#include "dealdebug_subs.h"
#include "deal_knr.h"
#include "dealdeck_subs.h"		/* create deck, Shuffle Deck, print Deck, etc. */
#include "deal_scorelib.h"
#include "dealutil_subs.h"   /* sort, get random numbers, .. */

/* Dealer_DDS_IF.h not needed? */


/* &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&& */
        /* routines in, or called by, yyparse action clauses */
        /* Some of these functions are in the dealyacc.y file, some are in the dealparse_subs.c file    */
extern int              d2n(char shape[4]) ;   /* 4char dist string like 4315 to an int -- also non numeric dist strings */
extern struct tree      *var_lookup(char *s, int mustbethere) ;                 /* yyparse action clause */
extern struct action    *newaction(int type, struct tree *p1, char *s1, int, struct tree * ) ; /* yyparse action clause */
extern struct tree      *newtree (int, struct tree*, struct tree*, int, int);   /* yyparse action clause */
extern struct expr      *newexpr(struct tree* tr1, char* ch1, struct expr* ex1); /* yyparse action clause */
extern void              bias_deal(int suit, int compass, int length) ;         /* yyparse action clause */
extern int               bias_len(int compass) ;                                /* yyparse action clause */
extern int               bias_totsuit(int suit) ;                               /* yyparse action clause */
extern void              predeal_holding(int compass, char *holding) ;          /* yyparse action clause */
extern void              predeal (int player, CARD52_k onecard);                /* called by predeal_holding */
extern void              insertshape(char s[4], int any, int neg_shape) ;       /* yyparse action clause */
extern void              new_var(char *s, struct tree *t) ;                     /* yyparse action clause */
extern void              setshapebit (int cl, int di, int ht, int sp, int msk, int excepted); /* called by action clause */
extern void              clearpointcount ();                 /* yyparse action clause set 13 elem array to zero*/
extern void              clearpointcount_alt (int cin);      /* yyparse action clause affects alternate counts */
extern void              pointcount (int index, int value);  /* yyparse action clause affects HCPs*/
extern void              zerocount (int points[13]);         /* called by clearpointcount etc. */
extern struct tree       *newquery(int tag, int side, int compass, int suit, int idx); /* yyparse action clause */
extern char              *newpar_cstr(int vuln) ;           /* yyparse action clause. see dealparse_subs */
extern struct csvterm_st *new_csvterm (struct tree *tr1, char *str1, int hand_mask,  int trix_mask, int par_mask, struct csvterm_st *csv1);
//                                  expr tree        string    hands-to-print trick-set mask               ptr->next csvlist item


/* Evaluation and condition stuff called by main, after the parsing during the evaluation phase */
#define interesting() ((int)evaltree(decisiontree))
extern void          analyze (DEAL52_k d, struct handstat *hsbase);
extern int           evaltree (struct tree *t);

    /* routines called during the post parse, evaluation phase */

extern CONTRACT_k    decode_contract(int coded_c) ;  /* from parse_subs for use building the action tree */
extern void          fmt_contract_str(char *str, int level, int strain, int dbl, int vul );
extern int           get_tricks (int pn, int dn);
extern int           dd (DEAL52_k d, int l, int c);     /* get cached results of true_dd */
extern int           true_dd (DEAL52_k d, int l, int c); /* call the GIB DD executable */
extern int           gib_tricks (char);  /* Convert GIB trix char [0-9A-D] to an integer */

/* from dealdds_subs.c */
extern int dds_tricks   (int compass, int strain ) ; /* reads the DDSRES_k struct and returns the number of tricks asked for */
extern int dds_parscore (int compass, int vuln   ) ; /* reads the DDSRES_k struct and returns the par score asked for */
extern int csv_trix     ( char *buff, int h_mask ) ; /* fmts buff with a list of trick counts for the hands asked for */

/* from UserServer_subs.c */
extern int ask_query( int qtag, int side, int qcoded ) ;

/* RP Library related subs. dealzrd_subs.c */
extern long int zrd_seekfpos( FILE *zrdlib_file, long int seed ) ;
extern int zrd_getdeal( FILE *zrdlib_file, struct options_st *opts, DEAL52_k dl ) ;

/* print and other action stuff Called by main action subs if the deal is 'interesting' ie meets the condition */
extern void action();
extern void cleanup_action() ;  /* do tasks that can only be done at end of run such as average, evalcontract, print(compass) etc. */
extern void evalcontract( struct action *acp) ;
extern void showevalcontract( struct action *acp, int nh);
extern void printdeal( DEAL52_k d);
extern void printside( DEAL52_k d, int side); /*JGM replaced printew with this one. */
extern void printhands( int boardno, DEAL52_k *dealp, int player, int nhands);
extern int  printpbn( int, DEAL52_k);
extern char *Hand52_to_pbnbuff( int p, char *dl, char *buff ) ;
extern void printhands_pbn( FILE *fp, int mask, DEAL52_k curdeal ) ;
extern void fprintcompact(  FILE * f, DEAL52_k d, int oneline); /* used for both GIB input and oneline printout */
#define printoneline(d) (fprintcompact(stdout, d, 1))
#define printcompact(d) (fprintcompact(stdout, d, 0))  /* this one used to format the input to the GIB dd solver */
void do_csvrpt( FILE *fcsv, struct csvterm_st *csvptr ) ; /* used by CSVRPT and PRTRPT */


        /* Miscellaneous Dealer Specific stuff */
        /*     Initializations */
extern void          initdistr ();
extern void          initevalcontract ();
extern void          setup_action ();
extern void          initprogram( struct options_st *opt_ptr) ;

       /* dealcard_subs stuff -- see also lib module dealdeck_subs.h*/
extern void         setup_deal ();
extern int          deal_cards (int mode, DEAL52_k d) ;

		/* Start usereval server stuff */
extern pid_t         setup_userserver( char *pathname ) ;
extern int           cleanup_userserver ( pid_t pid ) ;


        /* low level functions and macros These may be useful in more than just the dealer.c file */
extern int           hascard (DEAL52_k d, int player, CARD52_k onecard) ; /* accesses global var handstat[h].Has_card[s][r] array */
extern int           hasKard (DEAL52_k d, int player, CARD52_k thiscard) ; /* the slow version of the above */
extern CARD52_k      make_card (char rankchar, char suitchar);       /* used in yylex() */
extern int           make_contract (char *c_str ) ;  /* Flex passes yytext[1]... = [1-7][CDHSN][x]{0,2} */
extern char         *mycalloc (size_t nel, size_t siz);   /* alloc RAM num_elem * sizeof(elem) */
extern char         *mystrcpy( char *s) ;                /* re-implement strdup - Windows? */

        /* Yacc, Lex, and error stuff */
extern void          error   (char *);
extern void          yyerror (char *);
extern int           yyparse ( void );
extern int           yywrap  ( void );

/* &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&& */

#endif /* end ifndef DEALPROTOS_H */
















