/* File dealmain_subs.c JGM 2022-Feb-17 Code only used by main; no need for protos. */
/* 2022-02-07 Expanding Options with -X and -0 thru -9 */
/* 2022-03-07 Fine tune Debug levels */
/* 2022-10-27 Add -U option for userserver path */
/* 2023/08/04  3.0.0  -- keep in step with dealerv2.c; slight mod to help msg re vers 3.0.0 */
/* 2023/10/30 4.0.0  -- modified gen_rand_slot to use the drand since it is almost twice as fast as mrand when scaling */ 
/* This file should NOT be compiled independently. #inlcude it in dealerv2.c */
#ifndef _GNU_SOURCE
  #define _GNU_SOURCE
#endif
#include "../include/std_hdrs.h"  /* all the stdio.h stdlib.h string.h assert.h etc. */
#include <sys/stat.h>  // for stat structure

#if 0                            /* change to 0 when not testing */
#include "../include/std_hdrs.h"  /* all the stdio.h stdlib.h string.h assert.h etc. */

#include "../include/dealdefs.h"
#include "../include/dealtypes.h"
#include "../include/dealprotos.h"
#include "../include/dealexterns.h"   /* was dealglobals but testing if I can compile globals separately and link it. */
#include "../include/pointcount.h"
#include "../src/UsageMsg.h"
#include "../include/dealdebug_subs.h"
#include "../include/dbgprt_macros.h"
#include "../include/libVersion.h"

#define FATAL_OPTS_ERR -10

/* This next one is in case we want to use 64 bit ints to hold the distribution bits.
 * To allow for 64 shape statements instead of 'only' 32 shape statements.
 * then we can define MAXDISTR as 8*sizeof(long int) or thereabouts
 */
#ifndef MAXDISTR
 #define MAXDISTR 8*sizeof(int)
#endif

#endif

#ifndef UsageMessage
  #include "../src/UsageMsg.h"
#endif
#define OPTSTR "hmqvVg:p:s:x:C:D:L:M:O:P:R:T:N:E:S:W:X:U:Z:0:1:2:3:4:5:6:7:8:9:l:"
void show_options ( struct options_st *opts , int v );
void showargs(int argc, char *argv[]);
void show_script_vars ( struct param_st *p , int v ) ;
int str_UC(char *dst, char *src , int maxn);
int srv_dbg (char *opt_D ) ;

/*
 * get_options function is called AFTER the parsing of the Input file.
 * The input file parser sets the global vars, not the members of this struct
 * Therefore initialize the struct with the already set global vars, then
 * change any that were specified on the command line
 */
 /* get_options function will set the opts in the options struct
 * and also the related global variable(s) for backwards compatibility
 * It should ALWAYS update the related global var, when the equivalent opts->var is changed.
 */
int get_options (int argc, char *argv[], struct options_st *opts) {
    int opt_c;
    int stat_rc ;
    struct stat statbuff ;
    char mybuff[128];		// for printing library version 
    // int  mybuff_len ;
    char *pch , ch ; 		// temp vars for use by case 'Z' or case 'l'
    char zrd_optstr[8]; 	//      ditto
    char log_optstr[8];    //  temp vars for use by case 'l' 

    opterr = 0;
    opts->options_error = 0 ;  /* assume success */
    /* make sure that all the opts have sane default values. Use value set in globals.c if available */

    opts->max_generate = 0 ;      /* set to the global default to keep in sync */
    opts->max_produce = 0 ;           /*  Ditto */
    opts->seed = seed ;                /* global */
    opts->seed_provided = -1 ;         /* Default to no cmd line spec. Note a cmd line spec of 0 will get random seed */
    opts->dbg_lvl = jgmDebug ;         /* global */
    opts->srv_dbg_lvl = srvDebug;      /* global */
    opts->opc_opener = opc_opener ;    /* global */
    opts->opener = Opener ;            /* global */
    opts->dds_mode = dds_mode ;        /* global */
    opts->progress_meter = progressmeter; /* global */
    opts->quiet = quiet ;              /* global */
    opts->swapping = swapping;         /* global */
    opts->title_len = -1 ;              /* Flag: -1 no title given; >0 Title Given; 0 Null title given to suppress zrdhdr */
    opts->upper_case = 1 ;     		   /* removed obsolete cmd line switch */
    opts->verbose = verbose ;          /* global */
    opts->nThreads = 1 ;        			/* default More does not help unless DDS mode 2 is needed */
    opts->maxRamMB = 160 * opts->nThreads ;
    opts->csv_fmode[0] = '\0';        // if null no csvfile specified.
    opts->csv_fname[0] = '\0';
    strcpy(opts->userpgm, server_path) ;  // /usr/local/games/DealerV2_4/DealerServer from globals.c will not be used if no UserEval server wanted.
    opts->ex_fname[0]  = '\0';
    memset(opts->preDeal_len, 0, 4*sizeof(int) ) ; /* preDeal stores the -N,-E,-S,-W opt settings */
    memset(opts->preDeal, '\0' , 128 ) ;
    strcpy(opts->zrdlib_fname ,"=");    /* = is translated to the default location at initialization time. */
    opts->zrdlib_mode  = 0 ;           /* default to NO RPLIB file of solved deals -L*/
    opts->zrd_seed = seed ;            /* No effect unless the program is run in Library(-L) mode */
    
    opts->zrd_wanted = 0 ;					/* write our own zrd file (-Z switch) */
    opts->zrd_fname[0]  = '\0';			/* No default ZRD filename. Must be provided */ 

    opts->log_wanted = 0 ;					/* write interesting deals to LOG file in DL52 fmt (-l switch) */
    opts->log_fname[0]  = '\0';			/* No default LOG filename. Must be provided */     

    while ((opt_c = getopt(argc, argv, OPTSTR)) != EOF) {
      switch(opt_c) {
        case 'h': {
            fprintf(stdout, "--- HELP " VERSION " COMING --- \n");  /* concatenate the strings. VERSION includes quotes */
            fprintf(stdout, "%s Usage: -[options] [input_filename | stdin] [>output_file]\n", argv[0]);
            fprintf(stdout,"%s", UsageMessage);   /* UsageMessage in src/UsageMsg.h */
            fprintf(stdout, "--- HELP " VERSION " DONE --- \n");
            opts->options_error = 4 ;
            break;
        }
      case 'g':
        opts->max_generate = atoi( optarg );    maxgenerate = opts->max_generate;
        break;
      case 'm':
        opts->progress_meter = 1;              progressmeter = opts->progress_meter;
        break;
      case 'p':
        opts->max_produce = atoi( optarg );     maxproduce = opts->max_produce;
        break;
      case 'q':
        opts->quiet ^= 1;                       quiet = opts->quiet;
        break;
      case 's':
        opts->seed_provided = 1;                seed_provided = opts->seed_provided;
        opts->seed = atol( optarg );            seed          = opts->seed;
        opts->zrd_seed = opts->seed ;
        if (seed == LONG_MIN || seed == LONG_MAX) {
            fprintf (stderr, "Seed overflow: seed must be greater than %ld and less than %ld\n",
              LONG_MIN, LONG_MAX);
            exit (-1);
        }
        break;
      case 'v':
        verbose ^= 1 ;                          opts->verbose = verbose;
        break;
      case 'x':                  // replaced the three switches 0,2,3 with one switch that takes a value
         opts->swapping  = atoi( optarg ) ;     swapping = opts->swapping ;
         JGMDPRT(2,"Swapping Option = %d \n", opts->swapping ) ;
         break;
      case 'V':
        if (opts->title_len > 0  ) { printf("Title: %s\n", opts->title); }
        printf ("Version info....\n");
        printf ("Revision: %s \n", VERSION );
        printf ("Build Date:[%s] \n", BUILD_DATE );
        printf ("$Authors: Hans, Henk, JGM $\n");
		  GetLibVers(mybuff) ; 
		  printf("Library ID= %s \n", mybuff ) ; 
        #ifdef JGMDBG
          printf("JGMDBG is defined. Debugging printing to stderr is active\n");
        #endif
        opts->options_error = 3 ; // Version Info
        return 3 ;
        break ;

/* options added by JGM: -C, -D,  -M, -O, -P, -R, -T, -N, -E, -S, -W, -X, -U -Z and -0 thru -9  */
      case 'C':         /* Filename for CSV report. Normally opened with append unless preceded by w: */
         if ( optarg[0] == 'w' && optarg[1] == ':' ) {
            strncpy(opts->csv_fname, &optarg[2], 127 ) ;
            opts->csv_fmode[0] = 'w' ; opts->csv_fmode[1] = '\0' ;
         }
         else {
            strncpy(opts->csv_fname, optarg, 127 ) ;
            opts->csv_fmode[0] = 'a' ;  opts->csv_fmode[1] = '\0' ;
         }
         fcsv = fopen(opts->csv_fname , opts->csv_fmode) ;
         if (fcsv == NULL ) {
            perror("ERROR!! Open CSV Report file FAILED");
            fprintf(stderr, "ERROR!! Cant open [%s] for %s \n",opts->csv_fname, opts->csv_fmode );
            opts->options_error = FATAL_OPTS_ERR - 1;
         }
         JGMDPRT(2,"CSVRPT File %s opened in %s Mode\n",opts->csv_fname, opts->csv_fmode ); 
         break ;
      case 'D':
        opts->dbg_lvl = atoi( optarg ) ;
        jgmDebug = opts->dbg_lvl;
        opts->srv_dbg_lvl = srv_dbg( optarg ) ; /* check if the D option was x.y or .y or 0.y where y is debug verbosity for server */
        srvDebug = opts->srv_dbg_lvl ;
        break ;
     case 'L':  // RPLIB file of solved deals
         strncpy( opts->zrdlib_fname, optarg, sizeof(opts->zrdlib_fname)-1 ) ;
         if ( strcmp(opts->zrdlib_fname, "=") == 0 ) {      /* equal sign is shorthand for default rplib path name */
            strncpy(opts->zrdlib_fname, biglib, sizeof(opts->zrdlib_fname)-1 ); /* Try the full version if available */
            fzrdlib = fopen(opts->zrdlib_fname , "r" ) ;  
            if (fzrdlib == NULL ) {  /* full version failed. Try the tiny version */
			   strncpy(opts->zrdlib_fname, tinylib, sizeof(opts->zrdlib_fname)-1 );
			   if (fzrdlib == NULL ) {  /* both versions failed. User must specify exact filename */
			      perror("ERROR!! Open both choices for default RP LIB file for reading FAILED. Try entering complete filename");
                  fprintf(stderr, "ERROR!! Cant open [%s] for read. Aborting program \n",opts->zrdlib_fname );
                  opts->options_error = FATAL_OPTS_ERR - 2 ;
                  break ;  /* exit case L */
               } /* end tiny try failed */
            }    /* end  full version try failed */
         }       /* end user wanted default */
         else  { /* User not trying default name. Try the name he entered on cmd line */
			 fzrdlib = fopen(opts->zrdlib_fname , "r" ) ;  
			 if (fzrdlib == NULL ) {  /*Users filename failed Abort Program with err msg */
			      perror("ERROR!! Open RP LIB file for reading FAILED.");
                  fprintf(stderr, "ERROR!! Cant open [%s] for read. Aborting program \n",opts->zrdlib_fname );
                  opts->options_error = FATAL_OPTS_ERR - 2 ;
                  break ; /* exit case L */
               } /* end users cmdline entry try */
		}   /* end non default filename */ 
         /* either cmdline or one of the defaults succeeded */
         assert( fzrdlib != NULL );
         opts->zrdlib_mode = 1 ;
         opts->zrd_seed = (opts->seed_provided > 0 ) ? opts->seed : 0 ;  /* Note 0 is a valid seed in RP_lib mode */
         JGMDPRT(2,"RP DD File '%s' Opened OK \n",opts->zrdlib_fname ); /* cant seek yet. seed may not be final */
         break ; /* exit case L */
     case 'M':
        opts->dds_mode = atoi( optarg ) ;
        if (1 <= opts->dds_mode && opts->dds_mode <= 2 ) { dds_mode = opts->dds_mode ; }
        else opts->dds_mode = dds_mode ; // if invalid use the  compile time value
        break ;
     case 'O':
        opts->opc_opener = *optarg ;  // now convert the char to an compass direction
        opts->opener = (*optarg == 'N' || *optarg == 'n' ) ? COMPASS_NORTH :
                       (*optarg == 'E' || *optarg == 'e' ) ? COMPASS_EAST  :
                       (*optarg == 'S' || *optarg == 's' ) ? COMPASS_SOUTH :
                       (*optarg == 'W' || *optarg == 'w' ) ? COMPASS_WEST  : COMPASS_WEST ;
        if (opts->opener == COMPASS_WEST ) opts->opc_opener = 'W' ; // default in case invalid entry
        break ;
      case 'P':  /* set the vulnerability for Par calculations */
        opts->par_vuln = atoi( optarg ) ; // Will Need to translate from Dealer coding to DDS coding when calling Par function
        if(opts->par_vuln < 0 || opts->par_vuln > 3 ) {
			  opts->par_vuln = 0 ; //default to NON VUL if invalid entry
			  fprintf(stderr, "Invalid Vulnerability -- Setting to Nobody Vul \n");
			  opts->options_error = 6 ; // invalid vuln value Not Fatal
		  }
        break ;
      case 'R':
        opts->nThreads=atoi( optarg ) ;
        if (opts->nThreads > 10 ) {
           fprintf(stderr, "Invalid value[%d] for R opt(0..12) setting to 9.\n",opts->nThreads);
           opts->nThreads = 8 ;
           opts->options_error = 6 ; // invalid thread value Not Fatal
        }
        opts->maxRamMB = 160*opts->nThreads ; /* DDS needs 160MB per thread */
        break ;
      case 'T':
        strncpy( opts->title, optarg, 200); /* docs say 100; pgm allows 255 */
        strncpy(title, optarg, 200);
        opts->title[200] = '\0';             /* if strncpy truncates, there will be no NUL unless we add one. */
        title[200] = '\0';
        opts->title_len = (int)strlen( opts->title ); title_len = opts->title_len;
        break;
      case 'W':
        opts->preDeal_len[3] = str_UC( opts->preDeal[3], optarg, 31) ;
        break;
      case 'E':
        opts->preDeal_len[1] = str_UC( opts->preDeal[1], optarg, 31) ;
        break;
      case 'S':
        opts->preDeal_len[2] = str_UC( opts->preDeal[2], optarg, 31) ;
        break;
      case 'N':
        opts->preDeal_len[0] = str_UC( opts->preDeal[0], optarg, 31) ;
        break;
      case 'X':
         strncpy(opts->ex_fname, optarg, 127 ) ;
         fexp = fopen(optarg , "w" ) ;
         if (fexp == NULL ) {
            perror("ERROR!! Open eXport file for writing FAILED");
            fprintf(stderr, "ERROR!! Cant open [%s] for write \n",optarg );
            opts->options_error = FATAL_OPTS_ERR - 3 ;
         }
         break ;
      case 'U':
         stat_rc = stat(optarg, &statbuff) ; /* stat returns zero if successful Not documented */
         if ( 0 != stat_rc ) {
            perror("stat -U User Eval program path FAILED. Aborting... ") ;
            fprintf(stderr, "Path [%s] cannot be opened. Return code = %d\n" , optarg, stat_rc );
            opts->options_error = FATAL_OPTS_ERR - 4 ;
         }
         else {
				strncpy(opts->userpgm, optarg, SERVER_PATH_SIZE ) ;
				strncpy(server_path,   optarg, SERVER_PATH_SIZE  ) ;
				server_path[SERVER_PATH_SIZE] = '\0' ; /* server_path is SERVER_PATH_SIZE+1 byte array */
			}
		break ;
         
	   case 'Z':         /* Filename for deals saved in zrd fmt Normally opened with append unless preceded by w: */
			opts->zrd_fmode[0] = 'a' ; opts->zrd_fmode[1] = '\0';  /* append mode */
			opts->zrd_dds  = 1 ;  /* assume user wants solved deals */
			opts->zrd_wanted=1 ;  /* wants zrd, even if no solutions */
	      pch = strchr(optarg, ':' ) ; /* is there a colon? i.e. [[w][N]:]<filename> */
	      if (pch == NULL ) { 
				strcpy(opts->zrd_fname, optarg) ; 
			}
			else { /* filename preceded by options N, w, or both */ 
				strcpy(opts->zrd_fname, (pch+1) );
				*pch = '\0';
				strcpy(zrd_optstr, optarg) ;
				JGMDPRT(2,"ZRD OptStr=%s, Fname=%s\n",zrd_optstr, opts->zrd_fname ) ;
				pch = &zrd_optstr[0] ;  
				while ( (ch = *pch++) ) {
					if (ch == 'w' ) opts->zrd_fmode[0] = 'w' ; 
					if (ch == 'N' || ch == 'n' ) opts->zrd_dds = 0 ;   /* no dds solutions */
					if (ch == ':' ) break ; 
				}
			} /*end Z options else */
			JGMDPRT(2,"optstr=%s, zrd_fmode=%s,zrd_dds=%d, zrd_fname=%s\n",
						zrd_optstr,opts->zrd_fmode,opts->zrd_dds, opts->zrd_fname ) ; 
         fzrd = fopen(opts->zrd_fname , opts->zrd_fmode) ;
         if (fzrd == NULL ) {
            perror("ERROR!! Open ZRD Output file FAILED");
            fprintf(stderr, "ERROR!! Cant open [%s] for %s \n",opts->zrd_fname, opts->zrd_fmode );
            opts->options_error =  FATAL_OPTS_ERR - 8;
         }
         else {
					JGMDPRT(2,"ZRD Out File %s opened in %s Mode\n",opts->zrd_fname, opts->zrd_fmode ); 
					;
			}
      break;         
	   case 'l':         /* Filename for deals saved in DL52 fmt Normally opened with append unless preceded by w: */
			opts->log_fmode[0] = 'a' ; opts->log_fmode[1] = '\0';  /* append mode */
			opts->log_dds  = 1 ;  /* assume user wants solved deals */
			opts->log_wanted=1 ;  /* wants log, even if no solutions */
	      pch = strchr(optarg, ':' ) ; /* is there a colon? i.e. [[w][N]:]<filename> */
	      if (pch == NULL ) { 
				strcpy(opts->log_fname, optarg) ; 
			}
			else { /* filename preceded by options N, w, or both */ 
				strcpy(opts->log_fname, (pch+1) );
				*pch = '\0';
				strcpy(log_optstr, optarg) ;
				JGMDPRT(2,"LOG OptStr=%s, Fname=%s\n",log_optstr, opts->log_fname ) ;
				pch = &log_optstr[0] ;  
				while ( (ch = *pch++) ) {
					if (ch == 'w' ) opts->log_fmode[0] = 'w' ; 
					if (ch == 'N' || ch == 'n' ) opts->log_dds = 0 ;   /* no dds solutions */
					if (ch == ':' ) break ; 
				}
			} /*end l options else */
			JGMDPRT(2,"optstr=%s, log_fmode=%s,log_dds=%d, log_fname=%s\n",
						log_optstr,opts->log_fmode,opts->log_dds, opts->log_fname ) ; 
         flog = fopen(opts->log_fname , opts->log_fmode) ;
         if (flog == NULL ) {
            perror("ERROR!! Open LOG52 Output file FAILED");
            fprintf(stderr, "ERROR!! Cant open [%s] for %s \n",opts->log_fname, opts->log_fmode );
            opts->options_error =  FATAL_OPTS_ERR - 8;
         }
         else {
					JGMDPRT(2,"LOG52 Out File %s opened in %s Mode\n",opts->log_fname, opts->log_fmode ); 
					;
			}
      break;
      
      /* Next ten options set the scripting variables stored in the global struct 'parm' */
      /* parm.script_var[i] SHOULD always have a NULL since optarg will be NUL terminated, and should never be > PARAM_SIZE */
      case '0' :
         strncpy(parm.script_var[0], optarg, PARAM_SIZE) ;
         parm.script_var_len[0] = strlen(parm.script_var[0]) ;
         parm.scripting = 1 ;
         break ;
      case '1' :
         strncpy(parm.script_var[1], optarg, PARAM_SIZE) ;
         parm.script_var_len[1] = strlen(parm.script_var[1]) ;
         parm.scripting = 1 ;
         break ;
      case '2' :
         strncpy(parm.script_var[2], optarg, PARAM_SIZE) ;
         parm.script_var_len[2] = strlen(parm.script_var[2]) ;
         parm.scripting = 1 ;
         break ;
      case '3' :
         strncpy(parm.script_var[3], optarg, PARAM_SIZE) ;
         parm.script_var_len[3] = strlen(parm.script_var[3]) ;
         parm.scripting = 1 ;
         break ;
      case '4' :
         strncpy(parm.script_var[4], optarg, PARAM_SIZE) ;
         parm.script_var_len[4] = strlen(parm.script_var[4]) ;
         parm.scripting = 1 ;
         break ;
      case '5' :
         strncpy(parm.script_var[5], optarg, PARAM_SIZE) ;
         parm.script_var_len[5] = strlen(parm.script_var[5]) ;
         parm.scripting = 1 ;
         break ;
      case '6' :
         strncpy(parm.script_var[6], optarg, PARAM_SIZE) ;
         parm.script_var_len[6] = strlen(parm.script_var[6]) ;
         parm.scripting = 1 ;
         break ;
      case '7' :
         strncpy(parm.script_var[7], optarg, PARAM_SIZE) ;
         parm.script_var_len[7] = strlen(parm.script_var[7]) ;
         parm.scripting = 1 ;
         break ;
      case '8' :
         strncpy(parm.script_var[8], optarg, PARAM_SIZE) ;
         parm.script_var_len[8] = strlen(parm.script_var[8]) ;
         parm.scripting = 1 ;
         break ;
      case '9' :
         strncpy(parm.script_var[9], optarg, PARAM_SIZE) ;
         parm.script_var_len[9] = strlen(parm.script_var[9]) ;
         parm.scripting = 1 ;
         break ;
/*  -- end of JGM added cmd line switches */

      default :
            fprintf(stderr, "DEFAULT CASE FALL THROUGH\n");
      case '?' : // there was an invalid option invalid char is in optopt
            fprintf(stderr, "%s Usage: -[options] [input_filename | stdin] [>output_file]\n", argv[0]);
            fprintf(stderr, "Valid Options == [%s]\n", OPTSTR );
            fprintf(stderr, "\t\t\t[%c] is an invalid option\n", optopt ) ;
            opts->options_error = 3 ;  // invalid option found
            break;
      } /* end switch(opt) */
		  /* end switch(opt) */
   } /* end while getopt*/
     /* end while getopt*/
   if (opts->dbg_lvl >= 4 ) { fprintf(stderr, "Command Line switches done proceeding with main program \n") ; }
   #ifdef JGMDBG
	  extern int jgmDPRT ;
	  jgmDPRT = jgmDebug ;
   #endif
   return opts->options_error ;

} /* end getopts */
/* end getopts */

void show_options ( struct options_st *opts , int v ) {
    fprintf(stderr, "Showing Options with Verbosity = %d\n",v);
    fprintf(stderr, "\t %s=[%d]\n", "g:Maxgenerate", opts->max_generate ) ;
    fprintf(stderr, "\t %s=[%d]\n", "m:ProgressMeter", opts->progress_meter ) ;
    fprintf(stderr, "\t %s=[%d]\n", "p:Maxproduce", opts->max_produce ) ;
    fprintf(stderr, "\t %s=[%d]\n", "q:Quiet", opts->quiet ) ;
    fprintf(stderr, "\t %s=[%ld]\n","s:Seed", opts->seed ) ;
    fprintf(stderr, "\t %s=[%d ; %d]\n", "v:Verbose", opts->verbose, verbose ) ;
    fprintf(stderr, "\t %s=[%d]\n", "x:eXchange aka Swapping", opts->swapping ) ;
    fprintf(stderr, "\t %s=[%s] mode=[%s]\n", "C:Fname",   opts->csv_fname, opts->csv_fmode   ) ;
    fprintf(stderr, "\t %s=[%d] set to %d Server Debug=%d\n", "D:Debug Verbosity", opts->dbg_lvl, jgmDebug, srvDebug ) ;
    fprintf(stderr, "\t %s=[%s] zrdlib_mode[%d] seed=[%ld]\n", "L:Fname", opts->zrdlib_fname,opts->zrdlib_mode,opts->zrd_seed);
    fprintf(stderr, "\t %s=[%d] set to %d\n", "M:DDS Mode", opts->dds_mode, dds_mode ) ;
    fprintf(stderr, "\t %s=[%c, %d]\n", "O:Opener", opts->opc_opener, opts->opener  ) ;
    fprintf(stderr, "\t %s=[%d]\n", "P:Par Vuln", opts->par_vuln  ) ;
    fprintf(stderr, "\t %s=[%d]\n", "R:MaxThreads", opts->nThreads ) ;
    fprintf(stderr, "\t %s=[%d]\n", "R:MaxRamMB", opts->maxRamMB ) ;
    fprintf(stderr, "\t %s=[%s],len=%d\n", "T:Title", opts->title,opts->title_len ) ;
    fprintf(stderr, "\t %s=[%s]\n", "N:PreDeal", opts->preDeal[0] ) ;
    fprintf(stderr, "\t %s=[%s]\n", "S:PreDeal", opts->preDeal[2] ) ;
    fprintf(stderr, "\t %s=[%s]\n", "E:PreDeal", opts->preDeal[1] ) ;
    fprintf(stderr, "\t %s=[%s]\n", "W:PreDeal", opts->preDeal[3] ) ;
    fprintf(stderr, "\t %s=[%s]\n", "X:Fname",   opts->ex_fname   ) ;
    fprintf(stderr, "\t %s=[%s] %s \n", "U:Fname",   opts->userpgm, (userserver_reqd > 0 ) ? "Will Run" : "Not Needed"  ) ;
    fprintf(stderr, "\t %s=[%s] mode=[%s] DDtricks=%c\n", "Z:Fname", opts->zrd_fname, opts->zrd_fmode,"NY"[opts->zrd_dds]) ;
    fprintf(stderr, "\t %s=[%s] mode=[%s] DDtricks=%c\n", "l:Fname", opts->log_fname, opts->log_fmode,"NY"[opts->log_dds]) ;
   return ;
} /* end show opts */

void showargs(int argc, char *argv[]) {
    int argnum = 0 ;
    while (argnum < argc ) {
        printf("Argnum:%d = %s \n",argnum, *(argv+argnum) );
        argnum++;
    }
    return ;
}

  /* These will only show the ones actually set on the cmd line */
void show_script_vars ( struct param_st *p , int v ) {
   int i ;
   if( !p->scripting ) { return ;  } /* no script vars set */
   fprintf(stderr, "Showing Script Vars with Verbosity = %d\n",v);
   for (i = 0 ; i < 10 ; i ++ ) {
      if (p->script_var_len[i] > 0 ) {  fprintf(stderr, "\t [$%d]=%s\n", i, p->script_var[i] ); }
   }
   return ;
} /* end show script_vars */

/* Command Line Parsing via the main function and a while loop returns 0 no errors, 1, 2, 3,  errors*/
int str_UC(char *dst, char *src , int maxn) {
   int p = 0;
   while ( *src != '\0'  && p < maxn ) {
      *dst++ = toupper((unsigned char) *src++ ) ;
      p++;
   }
   *dst='\0';
   return p ;
} /* end str_UC */
int srv_dbg (char *opt_D ) {
   char *dot_pos;
   if ( (dot_pos = strchr(opt_D, '.' )) || (dot_pos = strchr(opt_D, ':' )) ) {
      return ( atoi( (dot_pos +1 ) ) );
   }
   return 0 ;
} /* end srv_dbg */

