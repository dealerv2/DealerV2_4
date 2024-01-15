/* File dealinit_subs.c  JGM 2023--MAR-15
 * Version 2.5.5a -- first version of this file. Collect all the init routines in one place.
 */
#ifndef _GNU_SOURCE
  #define _GNU_SOURCE
#endif
#include "../include/std_hdrs.h"  /* all the stdio.h stdlib.h string.h assert.h etc. */

#include "../include/dealdefs.h"
#include "../include/dealtypes.h"
#include "../include/dealprotos.h"    /* dealprotos now imports libdealerV2.h */
#include "../include/dealexterns.h"   
#include "../include/pointcount.h"
#include "../src/UsageMsg.h"
#include "../include/dealdebug_subs.h"
#include "../include/dbgprt_macros.h"
#include "../include/libdealerV2.h"
#include "../include/deal_bias_subs.h"

#define FATAL_OPTS_ERR -10
#ifndef DDS_TABLE_MODE
   #define DDS_TABLE_MODE 2      // import from the DDS namespace
#endif


CARD52_k  make_card (char rankchar, char suitchar)  ;
int setDealMode( struct options_st *opt_ptr ) ; /* choose a mode from possibly competing options */
void init_deal(int dealing_mode) ; 							/* setup the environment for the chosen dealing mode */
extern void init_bias_deal() ; 						/* one time setup of various bias_deal vars */
extern void re_init_bias_vars() ; 						/* init needed for every deal */
#if 0
int isCard (char crd) { // misnomer; we want the rank (0 .. 12) of the card, not a T/F result
   char Ranks[14]="23456789TJQKA" ;  
   int uc ;
   int card_rank = -1 ;
   uc = toupper(( int ) crd ) ;  
   for (card_rank = 0 ; card_rank<13 ; card_rank++ ) {			
      if ( uc == Ranks[card_rank] )  return card_rank ;
   }
   /* could replace loop with following 
			char *chptr ;
			card_rank = ( (chptr = strchr(Ranks,crd))  ) ? (chptr - &Ranks[0] ) : -1  ; 
   */
   return -1; 
}
#endif
void init_globals ( struct options_st *opts ) {
      /* give sane values to globals not initialized at compile time */
    memset( &parm, 0 , sizeof(parm) ) ;   /* filling with zeros should both terminate the 'strings' and set len to 0 */
    /* The non opt versions of the above will have been compile time init in dealglobals.c */

    /* None of the following are set in the input file and so should be replaced by the options.xxxx variable */
    errflg = 0;     progressmeter = 0;     swapping = 0;     swapindex = 0;     verbose = 1;     quiet = 0 ;   

    /* set these next 3 so that yyparse/flex will fill them in if reqd */
    maxdealer = -1;     maxvuln = -1;     dds_dealnum = -1;

    /* These globals cannot be a compile time init, since the user may re-direct stdin and stdout when calling dealer */
   fexp = stdout ; // FILE *fexp Will be opened by getopts if there is a -X option
   fcsv = stdout ; // FILE *fcsv Will be opened by getopts if there is a -C option

   input_file = '\0' ;    /* NULL pointer */

   /* set some options_st defaults. All set to zero at compile time.
    * Set non zero ones to match the corresponding compile time global
    * TODO eliminate the standalone global and just use the ones in options struct
    */
  opts->verbose      =  1;
  opts->opc_opener   =  3; opts->opener = 'W';
  opts->max_produce  =  0;			
  opts->max_generate =  10000000;		/* 10 Million */
  opts->dds_mode     =  1 ;
  opts->par_vuln     =  0 ;				/* No longer a flag; give valid value */
  opts->nThreads     =  1 ;				/* If we don't need dds mode 2, more threads do not help */
  opts->maxRamMB     = 160 ;
  opts->seed_provided = -1 ;				/* Flag No seed given */
  opts->seed         = 0 ;
  opts->quiet        = 0 ; 
  opts->progress_meter = 0;
  opts->zrd_wanted   = 0 ;
  opts->title_len = -1 ;				/* a title of zero length is valid to suppress zrd hdr records. */
  opts->title[0]='\0';

   return ;
} /* end init_globals */

void predeal_cmdparms(int compass, char *holding) {
/* yyparse may have filled in the Predeals if called for in input file
 * But cmd line Predeals will over-ride those
 * aside: predeal function is in file: dealparse_subs.c
 */
         char suit, ch;
         CARD52_k card ;
        size_t holding_len ;
        int  l  ;
        int cnt = 0 ;
        holding_len = strlen(holding) ;
        JGMDPRT(4, "Predeal_cmdparms:: Compass=%d, holding=[%s]\n", compass, holding) ;
        suit = ' ';
        for (l = 0 ; l < holding_len; l++ ) {
            ch = *(holding + l) ;  /* ch should be an uppercase letter, digit, or some sort of punctuation */
			ch = toupper( (int) ch ) ; 
            if (ch == 'C' || ch == 'D' || ch == 'H' || ch == 'S' ) { suit = ch ; }
            else if ( NO_CARD != (card=make_card(ch,suit) ) ) {   /* new strchr version of make_card ; FUT convert yacc, flex */
				predeal(compass, card  );  						 /* only predeal valid cards */
				cnt++ ;
			}
            /* if not a suit or card  make_card returns NO_CARD which we ignore. */
        }
        JGMDPRT(4,"Predeal_cmdparms:: Holding len=%ld, cards predealt = %d\n", holding_len, cnt ) ;
} /* end predeal_cmdparms() */

void initdistr () {
   /* Various handshapes can be asked for. For every shape the user is
   interested in a shape number is generated. In every distribution that fits that
   shape the corresponding bit is set in the distrbitmaps 4-dimensional array.
   This makes looking up a shape a small constant cost.
   [Note the shape number must be between 0 and 31 to match the number of bits in an int. Hence at most 32 shape statements in input file]
   */
  int ***p4, **p3, *p2;
  int clubs, diamonds, hearts;

  /* Allocate the four dimensional pointer array. calloc will set it to zero. */

  for (clubs = 0; clubs <= 13; clubs++) {
    p4 = (int ***) mycalloc ((unsigned) 14 - clubs, sizeof (*p4));
    distrbitmaps[clubs] = p4;  /* there is now an array of 14 pointers whose index is the number of clubs 0 .. 13 ] */
    for (diamonds = 0; diamonds <= 13 - clubs; diamonds++) {
      p3 = (int **) mycalloc ((unsigned) 14 - clubs - diamonds, sizeof (*p3));
      p4[diamonds] = p3;				
      for (hearts = 0; hearts <= 13 - clubs - diamonds; hearts++) {
        p2 = (int *) mycalloc ((unsigned) 14 - clubs - diamonds - hearts, sizeof (*p2));
        p3[hearts] = p2;
      }
    }
  }
} /* end initdistr */

void init_cards() {
  newpack(fullpack);   /* fill fullpack with cards in order from North SA downto West C2 */
  full_size = 52   ; 
  memset(stacked_pack, NO_CARD, 52) ;  /* stacked pack used for preDeals */
  stacked_size = 0 ; 
  memcpy(small_pack, fullpack, 52 ) ;  /* Shuffle works from small_pack not fullpack */
  small_size = 52 ;
  memset(curdeal, NO_CARD, 52 ) ;     /* so that we know which spaces need filling later */
}

void setup_rng( struct options_st *opts ) {
   /*
   * Using glibc standard lib routines rand48 and srand48.
   * If no seed provided, use function init_rand48 which uses kernel as seed source
   */
  if (jgmDebug >= 3 ) { JGMDPRT(3,"Initializing RNG. Seed_provided=%ld, seed=%ld\n",seed_provided,seed); }
  if (!opts->seed_provided) {
    opts->seed = SRANDOM(0) ; /* use init_rand48() to init the RNG with a seed that the kernel provides */
  }
  else {
    opts->seed = SRANDOM(opts->seed) ; /* use init_rand48() to init the RNG with the users seed */
  }
  JGMDPRT(3,"RNG initialized with Seed val=%ld\n",opts->seed );
} /* end setup RNG */

void setup_dds_mode ( struct options_st *opts ) {
  void ZeroCache( DDSRES_k *Results) ;
  void  SetResources(  int maxMemoryMB,  int maxThreads); /* for DDS */
/* If user has set TABLEMODE (-M 2) but neglected to set Threads (-R n), then we use the TblModeDefault value */
  if ( (opts->dds_mode == 2 ) && (opts->nThreads < 2) ) {
     opts->nThreads = TblModeThreads ;
     opts->maxRamMB = 160 * TblModeThreads ;
  } /* else dds_mode is 1 OR user has spec'd both Mode and Threads values */
	
	/* DDS docs say max Threads is set at 1.5 x nCores. 
	 * but finding nCores on Linux is questionable?
	 * In any case on my 4 core / 8 'cpu' intel chip, I cannot get more than 8 threads
	 * Regardless of how many more I ask for. Firefox on the other hand can get 128 threads!
	 */
  SetResources(opts->maxRamMB, opts->nThreads) ; /* 160MB/Thread max */
  ZeroCache(&dds_res_bin) ; /* dds_res_bin is a global struct holding tricks for 20 combos of leader and strain */
   JGMDPRT(3,"DDS_mode Set. Mode=%d, Threads=%d, Ram=%d MB \n",opts->dds_mode, opts->nThreads, opts->maxRamMB ) ;
   return ;
} /* end setup_dds_mode */

void enforce_zrdlib_mode(FILE *fzrdlib,  struct options_st *opt_ptr) {
  int set_zrdlib_vars(FILE *libfile, struct options_st *opts); /* calc the blocksize, number of records etc. */
  long int zrd_seekfpos(FILE *fzrdlib, long int seed ) ;
  int myseed;
  long zrdlib_pos = 0;
  zrdlib_mode = opt_ptr->zrdlib_mode ;
  JGMDPRT(3, "Lib Mode Specified. Ignoring impossible options \n");
 
       predeal_compass = -1 ;    /* reset predeal_compass in case yyparse found predeal spec in input file */
       if (stacked_size > 0 ) {  /* there was predealing in the yacc file */
          fprintf(stderr, "Cannot pre Deal hands from input file if using Library mode. Ignoring. \n");
          init_cards() ; /* undo any previous deck stacking. */
                         /* Not really needed since Lib mode just overwrites curdeal but a good safety net. */
       }
       if (opt_ptr->swapping > 0 ) {
          fprintf(stderr, "Swapping not possible if using Library mode. Ignoring. \n");
          opt_ptr->swapping = 0 ;  // reverse swapping setting.
          swapping = opt_ptr->swapping;
       }

       /* Repurpose the seed spec, if any, to mean an offset into the Library file */
       zrdlib_recs = set_zrdlib_vars(fzrdlib, opt_ptr) ; /* calculate DB size, then calc blksize and  max seed accordingly */
       JGMDPRT(3,"Set_zrdlib_vars Returns Globals: Recs=%d, Blksz=%d, Max_seed=%d, zrdlib_recnum=%d\n",
				zrdlib_recs, zrdlib_blksz, zrd_max_seed, zrdlib_recnum);
       if( opt_ptr->zrd_seed >= 0 ) { /* in Lib mode treat seed as an offset into the lib file */
            myseed = (opt_ptr->zrd_seed < zrd_max_seed) ? opt_ptr->zrd_seed : zrd_max_seed ;
            zrdlib_pos = zrd_seekfpos(fzrdlib, myseed) ;
            if( zrdlib_pos < 0 ) {
               fprintf(stderr, "Seeking to position of myseed=%d failed. Will start at beginning \n", myseed);
               myseed = 0 ; 
               zrdlib_pos = zrd_seekfpos(fzrdlib, 0) ;
            }
            JGMDPRT(3,"LIB Mode: myseed=%d, CmdLine_Seed=%ld,  MaxLib_seed=%d, LibRecs=%d, seek_to_seed_pos=%ld \n",
									myseed, opt_ptr->zrd_seed, zrd_max_seed, zrdlib_recs, zrdlib_pos ) ; 
       }
      zrdlib_recnum = zrdlib_pos / ZRD_REC_SIZE; 
       JGMDPRT(2,"enforce_zrdlib_mode Seeking to ZRD_Record # %u at beginning of zrdlib_recnum=%d, zrdlib_recs=%d, zrdlib_blksz=%d\n",
									(myseed * zrdlib_blksz + 1 ), (zrdlib_recnum+1), zrdlib_recs, zrdlib_blksz );

       if (opt_ptr->max_generate > zrdlib_recs ) {
          fprintf(stderr, "dealinit_subs.c::Finalize Options::ERR maxgenerate too big. Library does not contain %d records. Setting to %d \n",
                                    maxgenerate,zrdlib_recs ) ;
          opt_ptr->max_generate = zrdlib_recs ; 
          maxgenerate =  zrdlib_recs ;
          if (opt_ptr->max_produce > opt_ptr->max_generate) {
             opt_ptr->max_produce = opt_ptr->max_generate ;
             maxproduce = opt_ptr->max_produce ;
          }
       }
       /* now make sure to reset all impossible option flags */
       opt_ptr->dds_mode = DDS_TABLE_MODE ;  dds_mode = DDS_TABLE_MODE ; /* dummy; bec Lib mode returns all 20 results */
       opt_ptr->swapping = 0;                swapping = 0 ;
       predeal_compass = -1 ;
       memset(opt_ptr->preDeal,     0 , sizeof(opt_ptr->preDeal)     ) ;
       memset(opt_ptr->preDeal_len, 0 , sizeof(opt_ptr->preDeal_len) ) ;
       JGMDPRT(4, "ZRDLIB mode enforced zrdlib_recs=%d, zrdlib_blksz=%d, Start of zrdlib_recnum=%d, zrd_max_seed=%d\n",
							zrdlib_recs, zrdlib_blksz, (zrdlib_recnum+1), zrd_max_seed ) ; 
        JGMDPRT(4,"*---------Enforce_zrdlib_mode all Done----------* \n");
       
       return ; 
} /* end enforce_zrdlib_mode */

 /* one time initialization to predeal stacked cards, or setup bias deal infrastructure */
void init_deal(int dealing_mode ) {   /* One Time Initialize curdeal taking into account the Predeal requirements in stacked pack  */
  int i;
  int stk_cnt   = 0 ;
  int full_cnt  = 0 ;
  int small_cnt = 0 ;
 
  JGMDPRT(4, "First Time setup dealing mode=%d\n", dealing_mode); 
  switch(dealing_mode) {
	case DEF_MODE :
			init_cards() ;
			memcpy(curdeal, fullpack, 52) ;   /* vanilla mode works on curdeal */
			break ; 
	case LIB_MODE :
			init_cards() ;   /* libmode ignores Dealer cards; generates deal stricly from library */
			break ;  /* end  case LIB_MODE */
	case SWAP_MODE :
			init_cards();
			memcpy(curdeal, fullpack, 52) ;   /* swap mode works on curdeal */
			break ;  /* end case SWAP_MODE */
	case BIAS_MODE :
			init_bias_deal(); /*bias hand len, bias_hand_vp, bias_suit_tot */
			JGMDPRT(5,"BIAS DEAL finalized. BiasHand Vacant[%d, %d, %d, %d]\n",
					bias_hand_vp[0],bias_hand_vp[1],bias_hand_vp[2],bias_hand_vp[3]); 
			re_init_bias_vars();  /* set bias_src, small_pack, bias_pack, suit_left, hand_xs etc. */
			break ; /* end case BIAS_MODE */
	case PREDEAL_MODE : {
      sortDeal(stacked_pack);    /* Sort each of the stacked hands in order of SA to C2 -- helps later processing if Deal is sorted. */
      /* put the predealt cards into curdeal-- note predealt cards are hand specific you cant just put them anywhere */
      memset(curdeal, NO_CARD, 52 ) ;  /* so we know what is left to fill after stacked pack copied in */
      JGMDPRT(4, "First Time setup PREDEAL_MODE stacked_size=%d, \n", stacked_size);
      for ( i = 0 ; i < 52 ; i++ ) {
         if (stacked_pack[i] != NO_CARD ) {
            stk_cnt++ ;
            curdeal[i] = stacked_pack[i]; /* predealt card is already in the correct hand in stacked_pack */
         }
      } /* end for i < 52 */
      assert( stk_cnt == stacked_size ) ;
      // memcpy(curdeal, stacked_pack, 52) ; /* simpler than the above loop, but then we would not have stk_cnt */
      if (jgmDebug >= 4 ) {
         JGMDPRT(4, "stacked_size = %d DONE  Stacking First Time curdeal=\n",stacked_size);
         hexdeal_show(curdeal, 52);
      }
      /* put the cards remaining in fullpack into small_pack, and init the rest of curdeal also */
      JGMDPRT(4, "Now setting up small_pack with what is left of full_pack\n");
      for (i = 0; i < 52; i++) {
         if (fullpack[i] != NO_CARD ) { /* fullpack was updated by the parse phase along with stacked pack. */
            JGMDPRT(6,"Fill small pack slot %d with card %02x from fullpack slot %d\n",small_cnt,fullpack[i],i);
            full_cnt++;
            small_pack[small_cnt++] = fullpack[i] ;
         }
      } /* end for i < 52 */
      JGMDPRT(4,"Small Pack setup with %d cards Stack_cnt=%d, Total=%d\n",small_cnt, stk_cnt, small_cnt+stk_cnt ) ;
      assert( (small_cnt + stk_cnt) == 52 ) ; /* the predealt and non-predealt cards should total 52*/
      small_size = small_cnt ;                /* save small_cnt into global var small_size for later use by Shuffle() */

      JGMDPRT(3,"curdeal setup COMPLETE; stack_cnt=%d, small_cnt=%d, total_dealt=%d\n", stacked_size, small_size, (stacked_size+small_size));
		#ifdef JGMDBG
			if (jgmDebug >= 4 ) {
				JGMDPRT(4,"setup_deal R end esults: curDeal, stacked_pack, and small_pack \n");
				hexdeal_show(curdeal, 52) ;
				hexdeal_show(stacked_pack, 52) ;
				hexdeal_show(small_pack, small_size) ;
			}
		#endif
			break ; /* end PREDEAL_MODE */
	} /* end predeal mode */
	
  } /* end switch(dealing_mode) */ 
  return ; 
} /* end init_deal */

/* Merge the options set in yyparse with those from the cmd line; ensure no conflicts */
void finalize_options ( struct options_st *opt_ptr) {
   /* Over-ride what was in input file with what was entered on the cmd line via switches
    * If the opts value was not set on the cmd line, and there was one set in the input file copy input value to opts struct
    * also does some rudimentary consistency checking
    */

    /* if the use_side flag is set, we need to set the use_compass flags for both seats regardless of what the parser did*/
    if (use_side[0] == 1 ) {use_compass[0] = 1; use_compass[2]= 1;  } /* NS */
    if (use_side[1] == 1 ) {use_compass[1] = 1; use_compass[3]= 1;  } /* EW */
    JGMDPRT(3,"Finalize Options. use_side=[%d,%d] \n", use_side[0], use_side[1] );
    if (opt_ptr->opc_opener != '\0' ) { /* cmd line entry (if any) has highest priority*/
        Opener = opt_ptr->opener;       /* Get opts function will have set both */
        opc_opener = opt_ptr->opc_opener ;
    }
    else {  /* Keep opts struct in sync with the default or anything that Flex has set */
         opt_ptr->opener = Opener ;
         opt_ptr->opc_opener = seat_id[Opener] ;
         opc_opener = opt_ptr->opc_opener;
    }
     /* need to check the title stuff here, since yyparse() may have set it also */
    if( opt_ptr->title_len >= 0 ) {
       JGMDPRT(4,"Setting Title to CmdlineTitle=[%s],CmdLineTitleLen=[%d]\n",
                  opt_ptr->title, opt_ptr->title_len ) ;
       title_len = opt_ptr->title_len  ;
       strncpy( title, opt_ptr->title, MAXTITLE - 1 );
    }
    else if (title_len > 0 ) {
       JGMDPRT(4,"Setting Title to input file Title=[%s],title_len=[%d]\n",title, title_len );
       opt_ptr->title_len = title_len ;
       strncpy(opt_ptr->title, title, title_len+1) ;
    }
    JGMDPRT(4,"Final Title=[%s], Final TitleLen=[%d]\nOpt_Ptr title=[%s],Opt_ptr_>titleLen=[%d]\n",
                  title, title_len, opt_ptr->title, opt_ptr->title_len ) ;

	 if (1 == opt_ptr->zrd_wanted && 1 == opt_ptr->zrd_dds ) { /* might have -Z N:<fname> */
		 opt_ptr->dds_mode = DDS_TABLE_MODE ;  dds_mode = DDS_TABLE_MODE ;
	 }
    if (opt_ptr->max_generate > 0 ) { maxgenerate = opt_ptr->max_generate ; }
    else if (maxgenerate > 0 )      { opt_ptr->max_generate = maxgenerate ; }

     /* need to check the seed stuff here, since yyparse() may have set it also */
    if( opt_ptr->seed_provided > 0 ) {  /* getopts will set this to 1 if there is a -s switch on cmd line */
       seed_provided = opt_ptr->seed_provided  ;
       seed = opt_ptr->seed; opt_ptr->zrd_seed = seed ;
       JGMDPRT(4, "Cmd Line  Seed[%ld] sets global seed and rp_seed\n", opt_ptr->seed ) ;
    }
    else if (seed_provided) {  /* flex will set this to 1 if there is a seed in the input file */
       opt_ptr->seed_provided = seed_provided  ;
       opt_ptr->seed = seed ; opt_ptr->zrd_seed = seed ;
    JGMDPRT(4, "Input file  Seed[%ld] sets opt_ptr->seed =%ld and opt_pt->zrd_seed=%ld\n", seed, opt_ptr->seed,opt_ptr->zrd_seed ) ;
    }
    else { JGMDPRT(4, "No Seed provided. Will use kernel entropy\n"); }

    
    /* ------------ Now check the options that affect how the cards are dealt -------- */
	 DealMode = setDealMode(opt_ptr) ; 
	 
    #ifdef JGMDBG
      if (jgmDebug >=4 ) {
         fprintf(stderr, "%s.%d:: Predeal check, stacked_size=%d Next: FullPack, StackedPack, curdeal\n",
                         __FILE__,__LINE__,stacked_size);
         sr_deal_show(fullpack);
         sr_deal_show(stacked_pack);
         sr_deal_show(curdeal) ;
      }
    #endif
    JGMDPRT(4, "Done Finalize Options DealMode=%d, dds_mode=%d, Dbg_Verbosity=%d, vers=%s\n", 
						DealMode,dds_mode, jgmDebug, VERSION ) ;
} /* end finalize_options */

void init_runtime(struct options_st *opts) {
    if (opts->zrdlib_mode != 1 ) {   /* If we are not in Library Mode */
		setup_rng( opts ) ;         /* used seed value to init the rng */
		seed = opts->seed ;         /* set the global to match what setup_rng has done; if seed was zero, it was set by kernel */
		setup_dds_mode(opts) ;
      /*
       * Initialize the Deck with predealt cards from the stacked_pack, and leftover cards from small_pack
       * Wait till now because program opts may over-ride the parser
       * cant predeal in Library mode
       */
     }
     init_deal(DealMode);
   if (options.par_vuln < 0 ) {options.par_vuln = 0 ; } /* set default to none_vul if user did not set on cmd line. */
   if (maxgenerate == 0) {
       maxgenerate = 10000000 ; /* 10 Million */
       if (opts->zrdlib_mode == 1 && maxgenerate > zrdlib_recs ) {
          maxgenerate = zrdlib_recs; 
       }
       opts->max_generate = maxgenerate;
   }
   if (maxproduce == 0)  maxproduce = ((actionlist == &defaultaction) || will_print) ? 40 : maxgenerate;
   if (maxproduce > maxgenerate ) maxproduce = maxgenerate ; 
   JGMDPRT(2, "Maxgenerate=%d, Maxproduce=%d, DDS Threads=%d, UserServerReqd=%d\n",
               maxgenerate, maxproduce, opts->nThreads, userserver_reqd );
    /* { ASSERT: maxproduce <= maxgenerate; opts->max_generate == maxgenerate; opts->max_produce == maxproduce 
     *           if (zrdlib_mode == 1) then zrdlib_recs >= maxgenerate
     *                                     seed <= zrdlib_recs / zrd_blksize
     * }
     */
   /* V4.5 ensure the two HCP tables are in sync yyparse may have changed one but not the other*/
   for (int i=0 ; i<13 ; i++ ) { points[i] = tblPointcount[idxHcp][i] ; }

   /*
    * Start the user server if there was a usereval statement in the input file
    */
   if (userserver_reqd != 0 )  {
       userserver_pid = setup_userserver( server_path ) ;
       if (userserver_pid == -1 ) {
          perror(" Creating the server process returns failed PID. Aborting... ");
          assert(0) ;
       }
       JGMDPRT(3,"UserServer daemon started with pid=%d \nfrom path=[%s]\n",userserver_pid,server_path);
    } /* user server daemon started */

   if (srvDebug > 0 ) {
      // sleep(1) ;
      usleep(5000); /* 5 msec Give the child time to tell us his logfile/errmsg file name. before we start our own output */
      printf("\f") ; /* issue a form feed to start the Dealer output */
      fsync(1);  /* clear out stdout before starting real info */
   }
      /* Initialize the seat fields in handstat array for later use by the Server */
   for (int i=0 ; i < 4 ; i++ ) { hs[i].hs_seat = i ; }

   /* Walk the action list initializing any actions that need it */
   setup_action();
   return ;
}  /* end init runtime */

/* compare the cmd line, and the input file dealing options and choose one */
int setDealMode( struct options_st *opt_ptr ) {
	int libmode = opt_ptr->zrdlib_mode;
	int biasmode = bias_deal_wanted;
	int cmd_parm_predeal = opt_ptr->preDeal_len[0] >0 || opt_ptr->preDeal_len[1] >0 || 
								  opt_ptr->preDeal_len[2] >0 || opt_ptr->preDeal_len[3] >0  ;
	int predeal_mode =  stacked_size ; 
	int swapmode = opt_ptr->swapping ; 
   int runmode = DEF_MODE ; /* the default normal random deals */
   JGMDPRT(4,"setDealMode libmode=%d, biasmode=%d, cmd_parm_predeal=%d, predeal_mode=%d, swapmode=%d, runmode=%d\n",
				libmode,biasmode,cmd_parm_predeal, predeal_mode, swapmode, runmode ) ;
	 if (1 == libmode ) { 
		runmode = LIB_MODE ; 
		if (biasmode > 0 ) {
			dealerr("setDealMode::Cant have bias deals and Library mode. Discarding Bias Deal request") ;
			bias_deal_wanted = 0 ; 
		}
		if (predeal_mode > 0 || cmd_parm_predeal > 0) {
			dealerr("setDealMode::Cant have predealt cards and Library mode. Discarding Predeal request") ;
			predeal_compass = -1 ;  /* reverse the flag that yyparse may have set */
			stacked_size = 0 ;
			/* dis-allow predealing from the cmd line */
       memset(opt_ptr->preDeal,     0 , sizeof(opt_ptr->preDeal)     ) ;
       memset(opt_ptr->preDeal_len, 0 , sizeof(opt_ptr->preDeal_len) ) ;
		}
		if (swapmode > 0 ) {
			dealerr("setDealMode::Cant have swapping and Library mode. Discarding Swapping request") ;
			opt_ptr->swapping = 0 ; 
		}
	}  /* end 1 == libmode */
	else if ( biasmode > 0 ) {
		runmode = BIAS_MODE ; 
		if (predeal_mode > 0 || cmd_parm_predeal > 0) {
			dealerr("setDealMode::Cant mix predealt cards and Bias predeal. Discarding Predeal Cards request") ;
			predeal_compass = -1 ;  /* reverse the flag that yyparse may have set */
			stacked_size = 0 ; 
			/* dis-allow predealing from the cmd line */
       memset(opt_ptr->preDeal,     0 , sizeof(opt_ptr->preDeal)     ) ;
       memset(opt_ptr->preDeal_len, 0 , sizeof(opt_ptr->preDeal_len) ) ;
		}
		if (swapmode > 0 ) {
			dealerr("setDealMode::Cant have swapping and predealing. Discarding Swapping request") ;
			opt_ptr->swapping = 0 ; 
		}
	}  /* end biasmode */
	else if (cmd_parm_predeal > 0 ) {
		runmode = PREDEAL_MODE ; 
		if (predeal_mode > 0 ) {
			dealerr("setDealMode::Cmd Line Predeal Overrides Input File Predeal") ;
        init_cards() ;  
        for (int i =0 ; i < 4 ; i++ ) { /* setup stacked_pack, small_pack; curdeal not setup till deal_cards() */
           if (opt_ptr->preDeal_len[i] > 0 ) {
              JGMDPRT(4, "Calling Predeal hand = %c [%s] \n", "NESW"[i], opt_ptr->preDeal[i] );
              predeal_cmdparms(i, opt_ptr->preDeal[i] ) ;  /* sets stacked size,full_size etc. */
           }
        } /* end for */
        if (swapmode > 0 ) {
				dealerr("setDealMode::Cant have swapping and predealing. Discarding Swapping request") ;
				opt_ptr->swapping = 0 ; 
			}
		} /* end if predeal_mode */
	} /* end cmdparm predeal */
		
	else if ( predeal_mode > 0 ) {
		runmode = PREDEAL_MODE ; 
		if (swapmode > 0 ) {
			dealerr("Cant have swapping and predealing. Discarding Swapping request") ;
			opt_ptr->swapping = 0 ; 
		}
	} 
	else if (swapmode > 0 ) { 
		runmode = SWAP_MODE ; 
	} 
	else { 
		runmode = DEF_MODE ; 
	}  /* end if-else if - chain */
	/* clean up any side effects of discarded options; ensure that chosen option setup OK. */
	switch (runmode) {
		case DEF_MODE :
			init_cards();
			break ;
		case LIB_MODE :
			init_cards() ; 
			enforce_zrdlib_mode(fzrdlib, opt_ptr ) ; 
			JGMDPRT(5, "ZRDLIB mode enforced zrdlib_recs=%d, zrdlib_blksz=%d, Start of zrdlib_recnum=%d, zrd_max_seed=%d\n",
							zrdlib_recs, zrdlib_blksz, (zrdlib_recnum+1), zrd_max_seed ) ; 
			break ;		
		case BIAS_MODE :
			init_cards() ;
			break ;
		case PREDEAL_MODE :				
			/*nothing to do; predealing done by yyparse */
			/* cmdparm predeal done above */
			break ;	
		case SWAP_MODE :
			init_cards();
			break ;	
			
		default : fprintf(stderr, "%s:%d Can't happen in PreRunErrCheck\n", __FILE__, __LINE__ );
	} /* end switch runmode */
	JGMDPRT(4,"setDealMode Done. DealMode=%d\n",runmode);
	return runmode ; 
} /* end setDealMode */





