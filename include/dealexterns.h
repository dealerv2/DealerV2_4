 /* File dealgbls.h  2021-09-17 ;;;JGM  This file is the extern defs for the stuff in dealglbls.c */
 /*  Date      Version   Author  Description
  * 2022/01/02 1.0.0    JGM     Collect all dealer symbolic constants and macros in one place.
  * 2022-02-27 2.1.7    JGM     Mods for Francois Dellacherie enhanced shapes
  * 2022/09/15 2.2.0    JGM     Mods for Bucket Frequency Functionality
  * 2023/01/07 -- Merged in changes from V4 to fix predeal; dealcards_subs.c and globals, etc.
  * 2023/11/03 -- Deleted some variables that have been moved to their own .h file in anticipation of library modules 
  * 				and made the order correspond more closely to globals.c
  * 				Note this file may have more defns in it than globals.c bec of yyparse etc.
  */
#ifndef DEALEXTERNS_H
#define DEALEXTERNS_H
#ifndef _GNU_SOURCE
  #define _GNU_SOURCE
#endif

    #include "std_hdrs.h"
    #include "dealdefs.h"
    #include "dealtypes.h"
    #include "dealdds_subs.h"  	/* because we have some global structs that need dds typedefs */

 extern int jgmDPRT ;          	/* for use by the JGMDPRT and DBGDO macros in dbgprt_macros.h */
 extern int DealMode;   			/* 2023-12-10 Re-Org for Bias Deal */
/* Dealer Variables not related to Flex or Yacc */
 extern int nprod ;
 extern int ngen ;
 extern int deal_err; 
 
 extern char   *crlf ;            /* either \r\n for Windows or \n for Linux/Unix. MacOS=? */
 extern char    card_id[13];
 extern char    rank_id[];
 extern char    strain_id[6];
 extern char    seat_id[4];
 extern char   *player_name[4] ;
 extern char   *suit_name[4];
 extern char    suit_id[5][3];
 extern char    side_seat[2][2];

 extern char    ucrep[] ;
 extern struct  handstat    hs[4] ;
 extern struct  sidestat_st ss[2] ; 
 extern const int beg_pos[4] ;   /* the slots in the deck where the suits or hands begin */
 extern const int end_pos[4] ;   /* the slots in the deck where the suits or hands end +1 -- for use in calls to get_rnd_slot among other things */

    /* cmd line parameter variables  some used by yyparse *and flex */
 extern struct options_st options ; /* defined and init in globals.c */
 extern struct options_st *p_opts ;
 extern struct param_st parm ;         /* for script parameters $0 thru $9 */
 extern int     csv_firsthand;
 extern char    csv_trixbuff[64] ; // room for 20 * (2digits + comma) + NULL and a bit extra
 extern size_t  csv_trixbuff_len ;
 
 /* original cmd line switches -- many also appear in a yyparse action clause. --   would be nicer to put these all in a struct */
 extern int     maxgenerate;
 extern int     progressmeter;
 extern int     Opener;             /* Dealer uses COMPASS_NORTH etc which is an int */
 extern char    opc_opener;         /* opc uses characters W, N , E, S */
 extern int     maxproduce;
 extern int     quiet ;
 extern long    seed, seed_provided ;
 extern int     verbose ; 
 extern int     swapping, swapindex ;
 
 extern int     errflg ;
 extern int     jgmDebug ;          /* cmd line as opposed to #define Debug flag. 0 = None. 9=verbose */
 extern int     srvDebug ;          /* cmd line setting for child server debug level */
 
 extern int     dds_mode;           /* -M 1 use Board Mode fastest for 1-5 results; 2 Use Table Mode, fastest for 5-20 results*/

 extern int     TblModeThreads;

 extern char    title[MAXTITLE+1] ;   /* set from cmd line or from input file directly from dealflex.l */
 extern int     title_len;  /*  >0 valid title;  <0 no title specified =0 suppress zrdhdr record(s) even if title in dli file*/
 
 extern char   *input_file;
 extern FILE   *fexp ;
 extern FILE   *fcsv ;
 extern FILE   *fzrd ;
 extern FILE   *fzrdlib ;
 extern FILE   *flog;     
 
 extern char    zrdlib_default[64] ;  /* path name of the default location for the RP Library file */
 extern int     zrdlib_mode ;
 extern int     zrdlib_blksz ;
 extern int     zrdlib_recs;
 extern int     zrdlib_recnum;
 extern int     zrd_max_seed ;
 extern int     zrd_cnt ;
 extern int     zrdlib_pass_num ;
/* Global Vars to launch DealerServer daemon */
extern char server_dir[] ;    /* Path to the distro version */
extern char server_pgm[] ;    /* Default DealerServer in the current directory. or user sets path name via -U switch */
extern char server_path[];    /* Default, or path set by the -U switch. Should begin with a SLASH not a DOT */
extern pid_t userserver_pid ;

extern char 	opc_cmd_buff[128];
extern char 	opc_pgm_path[32] ;
extern int 		opc_pgmlen ;
extern int  	opc_dealnum ;   	/* -1 will force the opc cache to be updated the first time opc() is called */
extern struct opc_Vals_st opcRes ;

 extern char export_buff[128] ;   /* Max (never happen) predeal export is 96 chars. if doing all 4 hands which is not supported yet*/
 extern char zrd_default[64] ;        /* ../dealLib.zrd */
	/* Deck, Normal Predeal, and Shuffling Vars */
 extern DEAL52_k       asc_pack;    /* pack in order C2 up   to SA */
 extern DEAL52_k       fullpack;	/* pack in order SA down to C2 */
 extern DEAL52_k       stacked_pack;
 extern DEAL52_k       curdeal;
 extern DEAL52_k      *deallist;
 extern DEAL52_k       small_pack;   /* 2023-01-05 cards left after predeal done */
 extern int            small_size;   /* number of cards left after predeal done  */
 extern int            stacked_size;
 extern int            full_size ;
 
	/* Bias Dealing Variables. Some set by yyparse directly */

 extern int            bias_suits[4][4];  /* set by yyparse. The number of cards to predeal a given hand in given suit.*/
 extern int            bias_deal_wanted;  /* global flag set by yyparse, ref'd by shuffle and deal code */
 extern int 		     bias_tot_len ; 		/* the total number of all bias cards in all bias hand/suits */
 extern int            bias_hand_len[4]; 
 extern int            bias_hand_idx[4];
 extern unsigned int   RANKMASK[16] ; /* To set Dealer version of RANKBITS in a DEAL64_k binary deal */
/*
 * Various handshapes can be asked for. For every shape the user is
 * interested in a number is generated. In every distribution that fits that
 * shape the corresponding bit is set in the distrbitmaps 4-dimensional array.
 * This makes looking up a shape a small constant cost.
 * limited to 32 shape/FDshape statements, based on the number of bits in an int.
 * Each shape statement can hold hundreds of shapes esp if generated by an FDshape statement.
 */
 extern int ***distrbitmaps[14];        /* 4 Dimensional structure for Distributions */
 
/* Global vars for communication between Flex and Bison, and Dealer */
 extern struct tree    defaulttree ;
 extern struct tree   *decisiontree ;
 extern struct action  defaultaction ;
 extern struct action *actionlist;
 extern struct var    *vars;
 
 extern int 	will_print;         /* Is there a print action? if yes default produce = 40 */ 
 extern int     maxdealer;        	/* Flex */
 extern int     maxvuln;          	/* Flex */
 extern int 	userserver_reqd ;   /* Flex if usereval statement seen in input file */
 extern int 	predeal_compass ;   /* global variable for predeal communication */ 
 extern int 	shapeno;
 extern int 	use_compass[NSEATS]; /* Global var: 1 means this seat's hand needs to be analyzed */
 extern int 	use_side[2] ;       /* Global var: 1 means we need both hands for this side and the side also */
 extern int 	dds_dealnum ; 		/* -1 no DDS needed; 0 DDS needed; >0 ngen number of last fetch */
 extern int 	alt_tbl_idx ;  		/* Global var set by Yacc file code to track which altcount is being modified */
 extern int 	pointcount_index;   /* Global var set by Yacc file code to track which rank is being modified */
 extern int     lino ;              /* incremented in yylex() when comments etc found. Not any more ?*/

	/* Some Globals from other .h files and lib modules not in globals.c */

 extern DDSRES_k dds_res_bin;		/* from dealdds_subs.c and dealdds_subs.h */

	/* Constants for Evaluation and Scoring */
#ifndef POINTCOUNT_H
  #include "pointcount.h"
#endif
 extern int     ltc_weights[13] ;       /* Weight that allows coding the top cards in a suit. A=32, K=16, Q=8, J=4, T=2 x=1*/
 extern int     points[13] ;            /* Goren (or other) HCP values A=4, K=3 etc. */
 /* the pointcount array of various point count arrays. Needs the file pointcount.h to be included with this file.*/
 extern int 	tblPointcount [idxEnd][13] ;
 extern int 	alt_tbl_idx;
 extern int 	pointcount_index;
 /* Some ReadOnly Attributes of various cards Currently Controls is the main one, also Kleinman pts and ltc weights(dup) */
 extern int 	CardAttr_RO [idxEndRO][13] ;

/* table that converts a score diff to IMPs */
 extern const int imp_tbl[24] ;

 /* some debugging stuff */
extern int      treelev;                /* the level we are at in the decision tree */
extern int      showtree;               /* default to show it if in DBG mode */
extern int      treedepth;              /* in case we want to debug the tree walking */
extern int      num_trees ;
extern struct treeptr_st tree_ptrs[];


extern int dbg_dds_lib_calls;
extern int dbg_dds_res_calls;
extern int dbg_parscore_calls;
extern int dbg_tdd_calls;
extern int dbg_dd_calls;
extern int dbg_opc_calls;
extern int dbg_opc_cmd_calls;

extern int dbg_userserver_extcalls ;
extern int dbg_userserver_askquery ;

#endif /* ifndef DEALEXTERNS_H */

