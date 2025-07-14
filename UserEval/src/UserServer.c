   /* File {ROOT}/UserEval/src/UserServer.c  -- The Server side  JGM 2022-Nov-15
 *  Version  Date          Comment
 *    0.1    2022-10-20    First Draft
 *    0.5    2022-11-13    Minimal Functionality
 *    0.7    2022-12-02    Was Working. Now moved to its own directory tree.
 *    0.8    2022-12-21    Server setup, sync, and communication with Dealer working well.
 *    0.9    2023-01-01    Bergen, Karpin, Pavlicek metrics coded and working.
 *    0.91   2023-01-13    Larsson metric working; Begin adjHCP redo;
 *    0.9.6  2023-01-13    Refactor Trump Fit into Do_Trump_fit. DKP, Karp_B and Lar_B  etc. working
 *    0.9.7  2023-01-26    KnR with Fit working.
 *    1.0.0  2023-02-17    Most relevant metrics working. Set88 working. Prep for first github upload
 *    1.0.1  2023-03-31    Set88 modified to return both HLDF and NT pts. More Metrics. Bissell,....
 *    1.0.5  2023-04-10    Coded Sheinwold, Goren; more debugging.
 *    1.5    2023-05-26    Coded all metrics including ZarBasic and ZarAdvanced. Dropped Rule22.
 * 	1.5.2  2024-02-26		Roth Evaluation from Roth and Rubens book coding done. Debug started.
 *    1.6.0  2024-08-01    Added code for the 'All' request, a bit different returns from set88
 *    2.0.0  2024-08-12    Refactor Choosing Best Fit Logic and other prolog stuff.
 *    3.0.0  2024-09-13    Added metrics 20-27 for alt_HCP_calc
 *    3.5.0  2024-09-21    Added metric 41 for set of all alt_HCP calcs; Handstat now uses short ints.
 *    3.6.0  2024-10-20    Added metric 42. Like metric 88 plus some RAW and Cooked vals for DBASE loading 
 *    3.6.1  2024-04-05    Post Xia. GCC compiler version change.
 *    3.618  2024-04-05    Tweked output of usereval(20-27) to have the PavBodyVal in slots 6, 7 of the misc area
 */
#ifndef _GNU_SOURCE
  #define _GNU_SOURCE
#endif
/* This is an independent program execve'd by dealer main when the input file calls for usereval(...)
 * This piece looks after the mapping to the shared area, the initializing of the shared semaphores,
 * and the while loop which waits for a semaphore signal from the dealer program.
 *
 * The shared memory area is 4096 bytes. The 'template' provided in the header file is approx 3900 bytes
 * so there is room at the end for further expansion. But that would require mods to the dealer main.
 * The mmap has room for up to 128 four byte ints for each side. Thre are 15 different metrics defined;
 * The user might want up to 8 values from each metric;
 *    In NT:   The individual values for each player and the total for each side. (3 values)
 *    In The side's Best Fit: The values for each player, the total for the side, the suit of the best fit, and the fit length (5)
 * To get 8 results for all the defined metrics will require that at least 120 values be returned for each side.
 * If fewer metrics, or only one is being analyzed, then there is room for many values such as support pts, deductions for misfits,
 * Length points, in fact any factor that  might go into calculating a metric.
 *
 * The user could also write his external program in another language if such language supports
 * access to a Linux shared memory area.
 *
 */
#define SERVER_AUTHOR "JGM"
#define SERVER_BUILD_DATE "2025/07/01"

#ifndef JGMDBG
   #define SERVER_NAME "DealerServer"
   #define SERVER_VERSION "3.6.2"
   #define START_WAIT 500000     // wait half a sec in production version
   int   jgmDebug = 0 ;  /* value will be passed in from Dealer if Debugging wanted */
#else
   #define SERVER_NAME "DealerSrvdbg"
   #define SERVER_VERSION "103.6.2"
   #define START_WAIT 5000000     // wait 5 sec in debug version
   int jgmDebug = 1 ;  /* for Debug Version of Server always have a minimum at least */
#endif


#include "../include/std_hdrs_all.h"
#include "../include/dbgprt_macros.h"        /* JGMDBG macros only generate code if JGMDBG is defined */
#include "../include/Serverdbg_subs.h" 

/* The Interface to Dealer These are symbolic links to the file in the Dealer include directory*/
#include "../include/dealtypes.h"            /* HANDSTAT_k, deal52_k ... */
#include "../include/mmap_template.h"        /* THE key struct and typedefs for IPC between query and reply */
/* The interface to the user supplied calculation routines */
#include "../include/UserEval_types.h"       /* metric enum, various structs for metrics */
#include "../include/UE_util_protos.h"       /* factors and other metric calc sub-routines */
#include "../include/UE_calc_protos.h"       /* metric calculation main routines: goren_calc, roth_calc etc. */
#include "../include/UserEval_externs.h"     /* Global data used by everyone; storage allocated in UserEval_globals.c mostly */
/* File Global Vars; Only used this file; no externs needed. (exception ptrs struct?) */
#include "../include/UserServer_globals.c"  /*  */



/*
 * Functions prototypes
 */
// Setup  Teardown functions
char *link_mmap(int mm_fd );
sem_t *open_semaphore(char *sem_name) ;
void calc_ptrs ( char *p_mm, struct mmap_ptrs_st *p_pst, struct mmap_off_st *p_ost ) ;
void reset_UEsidestat ( int side ) ;
void prolog(int side) ; 
struct gbl_struct_st set_gblquery ( struct query_type_st *pqt ) ;
void eoj_mutex(  ) ;
void server_eoj() ;

// Evaluation Reply Functions
/* Replace all these with one function which accepts arguments to fill in the reply block and the user map area */
extern int analyze_side( UE_SIDESTAT_k  *p_UEss, int side ) ;               
int user_reply( int tag, int ures[], struct reply_type_st *prt, struct query_type_st *pqt );
int userfunc( struct query_type_st *p_q_type, struct reply_type_st *p_r_type, DEALDATA_k *p_dldat,
               USER_VALUES_k *p_nsres,USER_VALUES_k *p_ewres, MMAP_TEMPLATE_k *mm_ptr) ;

int hcp_reply(      USER_VALUES_k *pr, struct reply_type_st *prt, DEALDATA_k *pq,  struct query_type_st *pqt);
int alt_hcp_reply(  USER_VALUES_k *pr, struct reply_type_st *prt, DEALDATA_k *pq,  struct query_type_st *pqt);
int bergen_reply(   USER_VALUES_k *pr, struct reply_type_st *prt, DEALDATA_k *pq,  struct query_type_st *pqt);
int bissell_reply(  USER_VALUES_k *pr, struct reply_type_st *prt, DEALDATA_k *pq,  struct query_type_st *pqt);
int dkp_reply(      USER_VALUES_k *pr, struct reply_type_st *prt, DEALDATA_k *pq,  struct query_type_st *pqt);
int goren_reply(    USER_VALUES_k *pr, struct reply_type_st *prt, DEALDATA_k *pq,  struct query_type_st *pqt);
int karpb_reply(    USER_VALUES_k *pr, struct reply_type_st *prt, DEALDATA_k *pq,  struct query_type_st *pqt);
int kaplan_reply(   USER_VALUES_k *pr, struct reply_type_st *prt, DEALDATA_k *pq,  struct query_type_st *pqt);
int karpin_reply(   USER_VALUES_k *pr, struct reply_type_st *prt, DEALDATA_k *pq,  struct query_type_st *pqt);
int knr_reply(      USER_VALUES_k *pr, struct reply_type_st *prt, DEALDATA_k *pq,  struct query_type_st *pqt);
int larsson_reply(  USER_VALUES_k *pr, struct reply_type_st *prt, DEALDATA_k *pq,  struct query_type_st *pqt);
int lar_b_reply(    USER_VALUES_k *pr, struct reply_type_st *prt, DEALDATA_k *pq,  struct query_type_st *pqt);
int pav_reply(      USER_VALUES_k *pr, struct reply_type_st *prt, DEALDATA_k *pq,  struct query_type_st *pqt);
int roth_reply(     USER_VALUES_k *pr, struct reply_type_st *prt, DEALDATA_k *pq,  struct query_type_st *pqt);
int sheinw_reply(   USER_VALUES_k *pr, struct reply_type_st *prt, DEALDATA_k *pq,  struct query_type_st *pqt);
int zarbas_reply(   USER_VALUES_k *pr, struct reply_type_st *prt, DEALDATA_k *pq,  struct query_type_st *pqt);
int zaradv_reply(   USER_VALUES_k *pr, struct reply_type_st *prt, DEALDATA_k *pq,  struct query_type_st *pqt);

int mixed_KARreply( USER_VALUES_k *pr, struct reply_type_st *prt, DEALDATA_k *pq,  struct query_type_st *pqt);
int mixed_LARreply( USER_VALUES_k *pr, struct reply_type_st *prt, DEALDATA_k *pq,  struct query_type_st *pqt);
int test_reply(     USER_VALUES_k *pr, struct reply_type_st *prt, DEALDATA_k *pq,  struct query_type_st *pqt);
int set88_reply(    USER_VALUES_k *pr, struct reply_type_st *prt, DEALDATA_k *pq,  struct query_type_st *pqt);
int set40_reply(    USER_VALUES_k *pr, struct reply_type_st *prt, DEALDATA_k *pq,  struct query_type_st *pqt);
int set41_reply(    USER_VALUES_k *pr, struct reply_type_st *prt, DEALDATA_k *pq,  struct query_type_st *pqt);
int set42_reply(    USER_VALUES_k *pr, struct reply_type_st *prt, DEALDATA_k *pq,  struct query_type_st *pqt);
int unk_reply(      USER_VALUES_k *pr, struct reply_type_st *prt, DEALDATA_k *pq,  struct query_type_st *pqt);

typedef int (*pREPLY_FUNC_k)( USER_VALUES_k *pr, struct reply_type_st *prt, DEALDATA_k *pq,  struct query_type_st *pqt);

// Error and Debug functions -- see also Serverdebug_subs.c
static void die(char *msg) {   perror(msg);   exit(255); }

// DBGLOC and JGMDPRT are  macros defined if JGMDBG is defined, Do nothing definitions otherwise

int setup_logfile(char *template) ;

int main(int argc, char *argv[] ) {
   pid_t my_pid ;
   int urc ;
   int mm_fd ;
   int dealerv2_dbg ;
   int dl_dupfd2 ;
   FILE *fdl_stderr ; 
   char          *mm_ptr;  /* The base address of the mmap page -- has to be char ptr for offset math to work */
   mmap_hdr_k    *phdr;
   query_type_k  *pqt;
   reply_type_k  *prt;
   DEALDATA_k    *pdldat;
   USER_VALUES_k *p_nsres;
   USER_VALUES_k *p_ewres;
   MMAP_TEMPLATE_k *p_mm_base ;
   DEAL52_k        *pcurdeal;

    my_pid = getpid() ;
    if (argc < 2 )  {die("DealerServer Missing FD arg. Aborting..."); }
    if ( (0 == strcmp(argv[1], "-V")) || (0 == strcmp(argv[1], "-h")) ) {
       printf("%s Version %s for DealerV2.  Author=%s Build Date=%s\n",SERVER_NAME, SERVER_VERSION,SERVER_AUTHOR,SERVER_BUILD_DATE ) ;
       printf("Usage: %s <mmap_fd> [dbg_verbosity]  -- note no commas, no minus signs \n", argv[0]);
       if (argc >= 3 ) { jgmDebug = atoi( argv[2] ) ; }
       #ifdef JGMDBG
         JGMDPRT(0, "JGMDBG IS DEFINED= %d in Serverpid=%d; jgmDebug=%d Server_Version = %s \n",
               JGMDBG, my_pid, jgmDebug, SERVER_VERSION );
		 #endif
       return(0);
    }

    if (argc >= 3 ) {
       jgmDebug = atoi( argv[2] ) ;
    }
    if (jgmDebug >= 1 ) {   // print to stdout so appears on main pgm screen. stderr has been redirected ... 
		printf("%s:%d UE::pid=%d, Argc=%d, argv[0]= %s ,  mmap_fd=%s,  jgmDebug=%s \n",__FILE__,__LINE__,my_pid,argc,argv[0],argv[1],argv[2] );
      /* and some heading lines in the debug / error log file */
      fprintf(stderr, "%s Version %s for DealerV2.  Author=%s Build Date=%s\n",SERVER_NAME, SERVER_VERSION,SERVER_AUTHOR,SERVER_BUILD_DATE ) ;
      fprintf(stderr,"%s:%d UE::pid=%d, Argc=%d, argv[0]= %s ,  mmap_fd=%s,  jgmDebug=%s \n",__FILE__,__LINE__,my_pid,argc,argv[0],argv[1],argv[2] ); 
	 }

    JGMDPRT(1, "JGMDPRT is DEFINED in Serverpid=%d; jgmDebug=%d Server_Version = %s\n",
               my_pid, jgmDebug,SERVER_VERSION );
    mm_fd = atoi(argv[1] );
   /* Initialize IPC mechanisms */
   mm_ptr = link_mmap( mm_fd ) ; /* sets (char *)mm_ptr */
   p_mm_base = (void *)mm_ptr ;
   dealerv2_dbg = atoi(mm_ptr + 4090) ; /* main pgm wrote nnEOF. to 4090 */  
   JGMDPRT(1,"In %s:: link_mmap returns map_ptr=%p p_mm_tmpl=%p copied jgmDBG=%d\n",
               SERVER_NAME,(void *)mm_ptr, (void *)p_mm_base, dealerv2_dbg ) ; // to stderr
 // printf("UE::In %s:: link_mmap returns map_ptr=%p p_mm_tmpl=%p\n",SERVER_NAME,(void *)mm_ptr, (void *)p_mm_base ) ;  // to stdout

   /*
    * Use the data in the mmap_hdr area to find the names and offsets used by the client: dealerv2
    */
    calc_ptrs( mm_ptr, &ptrs, &offs ) ;  // populate the structs, incl the UEsidestat ptrs, using only the mmap_template defn */
    phdr =    ptrs.header;
    pqt =     ptrs.query;
    prt =     ptrs.reply;
    pdldat =  ptrs.dldata;
    pcurdeal= ptrs.p_deal;
    p_nsres = ptrs.nsres ;
    p_ewres = ptrs.ewres ;

    for (int h = 0 ; h < 4 ; h++ ) { hs_ptr[h] = ptrs.phs[h] ; } /* so everything can work off a HANDSTAT typedef */
 
    DBGDO(4,show_mmap_ptrs(mm_ptr, &ptrs));
    DBGDO(4,show_mmap_offs(&offs) ); 
   JGMDPRT(5, "Addresses of some globals\n");
   JGMDPRT(5,"UEsidestat[] is at %p, ptrs struct is at %p, (unUsed?) hs_ptr[0] is at %p with value  %p\n",
                     (void *)&UEsidestat[0], (void *)&ptrs, (void *)&hs_ptr[0], (void *)hs_ptr[0] ) ;
   
    strncpy(query_sema, phdr->q_sema_name, 31 ) ;
    strncpy(reply_sema, phdr->r_sema_name, 31 ) ;
    strncpy(mmap_fname, phdr->map_name,   sizeof(mmap_fname) ) ;
    dl_dupfd2 = phdr->stderr_fd ; 
    JGMDPRT(2, "^^^UserEval Local Copy from shared area: map_name=%s, q_sema=%s, r_sema=%s, dl_dupfd2=%d\n",
               mmap_fname, query_sema,  reply_sema,dl_dupfd2 ); // to redirected stderr
 //printf("UE::^^^UserEval Local Copy from shared area: map_name=%s, q_sema=%s, r_sema=%s\n",mmap_fname, query_sema,  reply_sema ); // to stdout


    p_qsem = open_semaphore(query_sema) ;
    p_rsem = open_semaphore(reply_sema) ;
    fdl_stderr = fdopen(dl_dupfd2, "w+");
    JGMDPRT(2, "^^^UserEval:: Addr of reply sema=%p Addr of query sema=%p\n",(void *)p_rsem, (void *)p_qsem  ) ;
  #ifdef JGMDBG
    if(dealerv2_dbg >0) { /* Dont chatter at caller if his debug setting is zero */
      fprintf(fdl_stderr, "Hello Parent stderr from Child DealerServer on local fd=%d\n",dl_dupfd2 ) ; 
    }
  #endif
   /*
    * Server ready to re-act to query posts and issue replies
    */
    if( jgmDebug > 1 ) { /* sleep in so a) user can note Debug Filename, and b) we can attach GDB to this process if we want to. */
		if(dealerv2_dbg >0 ) { /* Dont chatter at caller if his debug setting is zero */
            fprintf(fdl_stderr, "UE:: UserEval Version=%s Sleep for %d MicroSec then; waiting for semaphore \n",SERVER_VERSION, START_WAIT) ;
      }
		usleep(START_WAIT) ;  /* wait for some usec to allow user to note msgs half-sec, or 5 secs if debug version */
	 }
  while( 1 ) {
    sem_wait(p_qsem) ;
    JGMDPRT(7,"in Server:: Woke from p_qsem wait with Query Tag[%i], Descr=[%s], side=%d, idx=%d\n",
               pqt->query_tag, pqt->query_descr,pqt->query_side, pqt->query_idx) ;
   //printf("%s:%d in Server:: Woke from p_qsem wait with Query Tag[%i], Descr=[%s], side=%d, idx=%d\n", __FILE__,__LINE__,
   //            pqt->query_tag, pqt->query_descr,pqt->query_side, pqt->query_idx) ;
    if(pqt->query_tag < 0 ) {
       JGMDPRT(3,"Server pid=%d Got QUIT Query. Descr=%s \n",my_pid, pqt->query_descr) ;
       server_eoj(mm_ptr, PageSize ) ;
       JGMDPRT(1,"%s cleanup done. Exiting from while(1) main loop \n",SERVER_NAME);
       if(dealerv2_dbg > 0 ) {
          fsync(1) ;       /* dont write to middle of caller's output. Wait till he is done. */
          fprintf(fdl_stderr, "%s cleanup done. Exiting from while(1) main loop \n",SERVER_NAME); /* if user suppresses output then don't chatter */
       }
       exit(0) ;
       /* NOTREACHED */
    }
    sortDeal( (char *)pcurdeal ) ; /* Dealer does not sort the Deal unless it passes the condition clause. */
                                    /* ?? More efficient to only sort the side  we care about at this point ??*/
    
    #ifdef JGMDBG
      if (jgmDebug >= 7) {
         fprintf(stderr, "*------%s.%d Begin Sorted Deal Number %d -------*\n", __FILE__,__LINE__,pdldat->curr_gen );
         show_hands_pbn( 15 , (char *)pcurdeal ) ;
      }
    #endif
    
    JGMDPRT(7,"Calling userfunc with pqt->tag=%d\n", pqt->query_tag ) ; 
    urc = userfunc(pqt, prt, pdldat, p_nsres, p_ewres, p_mm_base) ; /*userfunc uses pqt->query_tag, to call relevant xxxx_reply() */
    if (-1 == urc ) {
		 char msgbuff[80] ; 
		 sprintf(msgbuff,"Fatal in User_eval; Aborting. Must Manually stop dealverV2 PID=%d",my_pid) ; 
		 die(msgbuff) ;
	 }
/* got OK result. Tell client */
   sem_post(p_rsem) ;
   JGMDPRT(5,"ServerMain:: Dealnum=%d user_eval done for q_tag[%d], r_tag[%d], r_descr=[%s] Waiting for next Query. zzzz \n",
                      pdldat->curr_gen, pqt->query_tag, prt->reply_tag, prt->reply_descr);
   DBGDO(7, show_user_res( "Server Main Loop", p_uservals, 0, (UEv.misc_count+6) )  ) ;
  } /* end while(1) */
  return (0) ;  /* NOT REACHED */
}/* end main User_Eval */
void prolog ( int side ) {  /* Server mainline has filled the ptrs struct of type mmap_ptrs_st and gbl struct */
   /* prolog is called everytime the usereval cache needs filling that is everytime the UserEval server runs.
    * so if the side changes the global ptrs change but the ptrs setup in ptrs, and UEsidstat don't change since each side
    * has its own. What does change on each deal is the trump suit stuff in UEsidestat, and all the temp vars that zero globals does.
    * So: Constant for the run: ptrs struct and ptrs in UESidestat.
    *     Constant for the deal: the t_suit, sorted_slens, and fitlens, the Declarer and dummy etc.
    *     Varies for each metric: The vars that zero globals resets
    */
   gen_num = ptrs.dldata->curr_gen ;
   prod_num = ptrs.dldata->curr_prod;
   reset_UEsidestat( side ) ; /* zero the parts of UEsidestat that change; keep the parts (esp handstat ptrs) that don't. */
   
   if (SIDE_NS == side ) {
      seat[0] = COMPASS_NORTH ;
      seat[1] = COMPASS_SOUTH ;
      phs[0] = ptrs.phs[COMPASS_NORTH] ;
      phs[1] = ptrs.phs[COMPASS_SOUTH] ;
      compass[0] = 'N';
      compass[1] = 'S';
      p_uservals = ptrs.nsres ;
      p_UEss = &UEsidestat[0] ;
      JGMDPRT(7,"Success - Prolog::NS:: GenNum=%d and Prod Num=%d sets p_UEss=%p phs[0]=%p,phs[1]=%p seats=[%d, %d]\n",
         gen_num,prod_num, (void *)p_UEss, (void *)p_UEss->phs[0],(void *)p_UEss->phs[1], phs[0]->hs_seat, phs[1]->hs_seat) ; 
   }
   else  {
      seat[0] = COMPASS_EAST ;
      seat[1] = COMPASS_WEST ;
      phs[0] = ptrs.phs[COMPASS_EAST] ;
      phs[1] = ptrs.phs[COMPASS_WEST] ;
      compass[0] = 'E';
      compass[1] = 'W';
      p_uservals = ptrs.ewres ;
      p_UEss = &UEsidestat[1] ;
      JGMDPRT(7,"Success - Prolog::EW:: GenNum=%d and Prod Num=%d sets p_UEss=%p phs[0]=%p,phs[1]=%p seats=[%d, %d]\n",
         gen_num,prod_num, (void *)p_UEss, (void *)p_UEss->phs[0],(void *)p_UEss->phs[1], phs[0]->hs_seat, phs[1]->hs_seat) ; 
   }
    return ; 
} /* end prolog */

/*
 * This next function belongs in a separate file to allow for easy recompilation
 */
int userfunc( struct query_type_st *pqt, struct reply_type_st *prt, DEALDATA_k *p_dldat,
               USER_VALUES_k *p_nsres,USER_VALUES_k *p_ewres, MMAP_TEMPLATE_k *mm_ptr)
{
    USER_VALUES_k *res_ptr;
    gbl = set_gblquery( pqt ) ;		/* fill gbl struct from the query pkt */
    if (pqt->query_side == 0 ) { res_ptr = p_nsres ; }
    else                       { res_ptr = p_ewres ; }
    JGMDPRT(7,"^^^userfunc Prolog and Analyze: for tag=[%d] Descr=[%s] using reply ptr=%p\n",pqt->query_tag, pqt->query_descr, (void *)prt ) ;
    JGMDPRT(7,"Side=%d, Res_ptr=%p \n", pqt->query_side, (void *)res_ptr ) ;

    prt->reply_tag = pqt->query_tag ;  // setup a default. May be over-ridden by code in switch
    strcpy( prt->reply_descr ,  "Normal Server Return" );

    /* now do code that will be needed by every metric; prolog( side)  and analyze_side(  *p_UEss, side ) */
    prolog( pqt->query_side ) ; /* Set the side globals like seat etc. */
    
   /* fill global struct UEsidestat[side] for later use returns the t_suit, or -1 if no fit*/
   JGMDPRT(7,"User Func Calling analyze_side with p_UEss=%p and side=%d\n", (void *)p_UEss, pqt->query_side );
    analyze_side( p_UEss,  pqt->query_side ) ;
/*
 * The Query tags in alpha order: The adj_hcp arrays use these values to lookup adjustments.
 *       0        1       2     3      4       5       6      7    8    9     10     11       12      13     14     15       FUT 15 OPC, 16 DKCCCC,
      BERGEN=0, BISSEL,  DKP, GOREN, KARP_B, KAPLAN, KARPIN, KnR, LAR, LAR_B, PAV, SHEINW,  ZARBAS, ZARADV, ROTH, metricEnd,
 * Query Tags for various flavors of HCP scales Omit C13, and Work Count which are covered already in dealerV2
     HCPT050=20, HCPA425, HCPAT475, BUMWRAP, WOOLSEY, AND5THS, BWjgm, OPCjgm
        20          21       22       23       24       25       26     27
 *   Tags for queries that return several metrics at once.
      SET_40=40, ALL_UE=40, SET_41=41, MixKar=50, MixLar=51,
 *    SET_88=88, SYNTST=99,
 *     Fut 80 (lots of results re tricks) + 81 vals; 81 lots of results re Suit lengths, and QuickTricks and QuickLosers
      Quit=-1} ;
*/
   JGMDPRT(4,"UserFunc Switch statement with Tag=%d GenNum=%d, ProdNum=%d\n", pqt->query_tag, gen_num, prod_num );
    switch (pqt->query_tag) {
      case 'B':
      case  BERGEN:  bergen_reply(  res_ptr, prt, p_dldat, pqt);  break ; /* Bergen   */
      case 'b':
      case  BISSEL:  bissell_reply( res_ptr, prt, p_dldat, pqt);  break ; /* Bissel from Bridge Encyclopedia and Pavlicek */
      case 'D':
      case  DKP :    dkp_reply(     res_ptr, prt, p_dldat, pqt); break ; /* D Kleinman LJP from NoTrump Zone */
      case 'G':
      case  GOREN :  goren_reply(   res_ptr, prt, p_dldat, pqt);  break ; /* Goren -- shortness pts    */
      case 'E': /* for Edgar */
      case KAPLAN :  kaplan_reply(  res_ptr, prt, p_dldat, pqt);  break ; /* Kaplan -- length pts. from 1960's book */
      case 'K':
      case KARPIN :  karpin_reply(  res_ptr, prt, p_dldat, pqt);  break ; /* Karpin - length pts. Src: Pavlicek website     */
      case 'J':      /* was JGM1 mods to Karpin */
      case KARP_B:   karpb_reply(   res_ptr, prt, p_dldat, pqt);  break ; /* Karpin with BumWrap points  */
      case 'k':
      case KnR :     knr_reply(     res_ptr, prt, p_dldat, pqt);  break ; /* KnR 4C's with Dfit  */
      case 'L':
      case LAR :     larsson_reply( res_ptr, prt, p_dldat, pqt);  break ; /* Larsson -- mild length pts */
      case 'M' :     /* was MORSE mods to Larsson */
      case LAR_B:    lar_b_reply(   res_ptr, prt, p_dldat, pqt);  break ; /* Larsson with BumWrap and Dfit mods   */
      case 'P':
      case PAV :     pav_reply(     res_ptr, prt, p_dldat, pqt);  break ; /* PAV -- shortness pts. from  Website. Like Goren minor mods */
      case 'R':
      case ROTH :    roth_reply(    res_ptr, prt, p_dldat, pqt);  break ; /* ROTH -- from 1968 Book. Dpts(shortness), Lpts,Dfit,FN */
      case 'S':
      case SHEINW :  sheinw_reply(  res_ptr, prt, p_dldat, pqt);  break ; /* Sheinwold from book. Short suits. */
      case 'z':
      case ZARBAS :  zarbas_reply(  res_ptr, prt, p_dldat, pqt);  break ; /* Basic Zar pts from the 2005 PDF download */
      case 'Z':
      case ZARADV:   zaradv_reply(  res_ptr, prt, p_dldat, pqt);  break ; /* Basic Zar plus HF, FN, HCP in 2/3 suits etc */

      case 20 : case 21: case 22: case 23: case 24: case 25: case 26: case 27:
                     alt_hcp_reply( res_ptr, prt, p_dldat, pqt);  break ; /* all alt_hcp done by same function */

      case 40:    set40_reply(   res_ptr, prt, p_dldat, pqt);  break ; /* all HLDF style metrics with 6 values per metric returned. */
      case 41:    set41_reply(   res_ptr, prt, p_dldat, pqt);  break ; /* all alt HCP style metrics with 6 values per metric returned */
      case 42:    set42_reply(   res_ptr, prt, p_dldat, pqt);  break ; /* All HLDF style metrics @2 vals (BF&NT) per + extra 2 RAW/Cooked vals for 5 of them */  

      case 50:    mixed_KARreply(   res_ptr, prt, p_dldat, pqt);  break ; /* karpin and karp_b     */
      case 51:    mixed_LARreply(   res_ptr, prt, p_dldat, pqt);  break ; /* larsson and lar_b     */

      
      case 88:     set88_reply(     res_ptr, prt, p_dldat, pqt);  break ; /* do all the metrics for which there is code */
      case 99:     test_reply(      res_ptr, prt, p_dldat, pqt);  break ; /* fill the slots with coded values to test syntax  */
      case 'Q':
      case Quit:
            server_eoj(mm_ptr, PageSize) ; /* Dealer is finished. Cleanup our stuff  */
            JGMDPRT(1,"%s cleanup done. Exiting from Quit is mainloop switch statement \n",SERVER_NAME);
            exit (0) ;
            /*NOT REACHED */
            break ;
      default  : unk_reply(    res_ptr, prt, p_dldat, pqt); break ;      /* Unknown      */
   } /* end switch */

   // msync((void *)mm_ptr, PageSize, MS_SYNC) ; //msync should not be req'd as we dont care about the backing file, only the in RAM copy.
   if (jgmDebug >= 5 ) {
         JGMDPRT(5,"Switch Done. prt.tag=[%d], prt.descr=[%s] \n", prt->reply_tag, prt->reply_descr );
         show_reply_type( prt) ;
         fsync(2); /* debug */
   }

   return (1) ;
} /* end userfunc */

/* End userfunc */
void calc_ptrs (  char *p_mm, struct mmap_ptrs_st *p_pst, struct mmap_off_st *p_ost ) {
   p_ost->header     = offsetof(MMAP_TEMPLATE_k , mm_hdr_dat      ) ;
   p_ost->query      = offsetof(MMAP_TEMPLATE_k , mm_qtype_dat    ) ;
   p_ost->reply      = offsetof(MMAP_TEMPLATE_k , mm_rtype_dat    ) ;
   p_ost->dldata     = offsetof(MMAP_TEMPLATE_k , mm_deal_data    ) ;
   p_ost->nsres      = offsetof(MMAP_TEMPLATE_k , mm_user_nsvals  ) ;
   p_ost->ewres      = offsetof(MMAP_TEMPLATE_k , mm_user_ewvals  ) ;
   p_ost->cache      = offsetof(MMAP_TEMPLATE_k , mm_cache        ) ;
   p_ost->hs_arr     = offsetof(MMAP_TEMPLATE_k , mm_deal_data.hs ) ;
   p_ost->curdeal    = offsetof(MMAP_TEMPLATE_k , mm_deal_data.curdeal );
   p_pst->header      = (void *) (p_mm + p_ost->header ) ;
   p_pst->query       = (void *) (p_mm + p_ost->query  ) ;
   p_pst->reply       = (void *) (p_mm + p_ost->reply  ) ;
   p_pst->dldata      = (void *) (p_mm + p_ost->dldata ) ;
   p_pst->nsres       = (void *) (p_mm + p_ost->nsres  ) ;
   p_pst->ewres       = (void *) (p_mm + p_ost->ewres  ) ;
   p_pst->cache       = (void *) (p_mm + p_ost->cache  ) ;
   p_pst->p_deal      = (void *) (p_mm + p_ost->curdeal) ;

   char *hs_base ;
   hs_base = (void *) (p_mm + p_ost->hs_arr ) ; 
   for (int h=0 ; h<4 ; h++ ) {
      p_pst->phs[h] = (void *)(hs_base + h*sizeof(HANDSTAT_k) );
      hs_ptr[h] = p_pst->phs[h];  /* maybe never used global */
   }
   /* do the one time setup of pointers in the UEsidestat[] */
   UEsidestat[0].side = 0;
   UEsidestat[1].side = 1;
   UEsidestat[0].phs[0] = ptrs.phs[0]; // North
   UEsidestat[0].phs[1] = ptrs.phs[2]; // South
   UEsidestat[1].phs[0] = ptrs.phs[1]; // East
   UEsidestat[1].phs[1] = ptrs.phs[3]; // West
   DBGDO(3,show_mmap_ptrs(p_mm , p_pst) ) ;
   DBGDO(3,show_mmap_offs(p_ost) );
   DBGDO(3,show_mmap_sizes() );
   JGMDPRT(3, "Global HandStat_ptrs: [N]=%p, [E]=%p, [S]=%p, [W]=%p \n", (void *)hs_ptr[0],(void *)hs_ptr[1],(void *)hs_ptr[2],(void *)hs_ptr[3] ) ;
   return ;
} /* end calc_ptrs */

/*
 * mmap the fd that our parent has passed us.
 */
char *link_mmap(int fd) {
      size_t len;
      char *mm_ptr ;

      PageSize = sysconf(_SC_PAGESIZE);
      if ( PageSize < 0) {die("sysconf() error cant get PAGESIZE"); }
      off_t offset = 0;      /* start at beginning of file */
      len = PageSize;        /* Map one page */
      mm_ptr = mmap(NULL,                          /* Let Kernel Choose addr in my space */
                      len,                         /* Map one Page */
                      PROT_READ|PROT_WRITE,        /* Allow R/W access to the region; cannot conflict with file perms */
                      MAP_SHARED,                  /* Allow other procs to also map and see our updates. */
                      fd,                          /* The related fd. Will be of no further use in the parent proc */
                      offset );                    /* We want to start at zero */
      if (mm_ptr == MAP_FAILED ) {die(" Server:: mmap of mm_fd failed"); }
      JGMDPRT(1,"Server SUCCESS! fd[%d] mapped to child address=%p\n", fd, mm_ptr );
      return (mm_ptr) ;
} /* end link_mmap */

sem_t *open_semaphore(char *sem_name) {
   sem_t *p_mtx ;
   p_mtx = sem_open(sem_name, O_CREAT, 0666, 0);
   if (p_mtx == SEM_FAILED ) die("Main Cannot Open/Create semaphore ");
   return (p_mtx) ;
}

/* The gbl struct is for the cases where the query specifies a hand/suit/side combination rather than just a side.
 * Never used in the current code
 */
struct gbl_struct_st set_gblquery ( struct query_type_st *pqt ) {
	struct gbl_struct_st gbl ;
	gbl.g_tag = pqt->query_tag ;
	gbl.g_side = pqt->query_side ;  /* side always set by code in the parser */
	gbl.g_compass = (pqt->query_hflag) ? pqt->query_hand : - 1; /* 0=N, 3=W */
	gbl.g_suit = (pqt->query_sflag) ? pqt->query_suit : - 1; 
	gbl.g_idx  = pqt->query_idx ;
  return gbl ;
}

/* Uses global vars for this file */
void eoj_mutex(  ) {
   sem_close(p_rsem);
   sem_close(p_qsem);
   sem_unlink(query_sema) ;
   sem_unlink(reply_sema) ;
   return ;
}

void server_eoj(char *mm_ptr, size_t PageSize) {
   msync(mm_ptr, PageSize, MS_SYNC) ; // flush any outstanding reply
   JGMDPRT(2,"Calling munmap with mm_ptr=%p, PageSize=%ld \n", (void *)mm_ptr, PageSize ) ;
   if ( munmap(mm_ptr, PageSize ) <    0 )  {
     perror("munmap() error");
  }
   sem_post(p_rsem) ; /* in case the client is still waiting */
   eoj_mutex() ;  /* tell kernel we finished with semaphores */
   return ;
}
int hcp_reply(      USER_VALUES_k *pr, struct reply_type_st *prt, DEALDATA_k *pdl_dat,  struct query_type_st *pqt) {
   strcpy(prt->reply_descr, "Work_HCP_only" ) ;
   prt->reply_tag = pqt->query_tag ;
   for (int i = 0 ; i < 4 ; i++ ) {
      pr->u.res[i] = 0 - pdl_dat->hs[0].hs_points[i] ; /* is this a testing function only? Why 0 - ?*/
   }
   return (0) ;
}
int alt_hcp_reply(  USER_VALUES_k *pr, struct reply_type_st *prt, DEALDATA_k *pdl_dat,  struct query_type_st *pqt) {
   int num_res =0 ;
   strcpy(prt->reply_descr, "ALT_HCP" ) ;
    prt->reply_tag = pqt->query_tag ;
   num_res = alt_HCP_calc(  p_UEss,prt->reply_tag  )  ;  // will use the global gbl struct to get tag/metric and hand suit etc. 
   JGMDPRT(6,"alt_hcp_calc returns %d fields calculated\n",num_res) ;
   return (num_res) ;

}
int bergen_reply(   USER_VALUES_k *pr, struct reply_type_st *prt, DEALDATA_k *pdl_dat,  struct query_type_st *pqt) {
   int num_res = 0;
   strcpy(prt->reply_descr, "Bergen" ) ;
   prt->reply_tag = pqt->query_tag ;
   num_res = bergen_calc( p_UEss )  ; 
   JGMDPRT(6,"=====Bergen_calc returns %d fields calculated, res[0]=%d, res[3]=%d\n",num_res, pr->u.res[0],pr->u.res[3]) ;
   return (num_res) ;
}
int bissell_reply(  USER_VALUES_k *pr, struct reply_type_st *prt, DEALDATA_k *pdl_dat, struct query_type_st *pqt) {
   int num_res = 0 ;
   strcpy(prt->reply_descr, "Bissell" ) ;
   prt->reply_tag = pqt->query_tag ;
   num_res = bissell_calc( p_UEss )  ; 
   JGMDPRT(6,"=====Bissell_calc returns %d fields calculated, res[0]=%d, res[3]=%d\n",num_res, pr->u.res[0],pr->u.res[3]) ;
   return (num_res) ;
}
int dkp_reply(      USER_VALUES_k *pr, struct reply_type_st *prt, DEALDATA_k *pdl_dat,  struct query_type_st *pqt) {
   strcpy(prt->reply_descr, "Kleinman" ) ;
   int num_res = 0;
   prt->reply_tag = pqt->query_tag ;
   num_res = dkp_calc(  p_UEss )  ;
   JGMDPRT(6,"=====DKP_calc returns %d fields calculated, res[0]=%d, res[3]=%d\n",num_res, pr->u.res[0],pr->u.res[3]) ;
   return (num_res) ;
}
int goren_reply(    USER_VALUES_k *pr, struct reply_type_st *prt, DEALDATA_k *pdl_dat,  struct query_type_st *pqt) {
   strcpy(prt->reply_descr, "Goren" ) ;
   int num_res = 0;
   prt->reply_tag = pqt->query_tag ;
   num_res = goren_calc( p_UEss )  ;
   JGMDPRT(6,"=====Goren_calc returns %d fields calculated, res[0]=%d, res[3]=%d\n",num_res, pr->u.res[0],pr->u.res[3]) ;
   return (num_res) ;
}
int kaplan_reply(   USER_VALUES_k *pr, struct reply_type_st *prt, DEALDATA_k *pdl_dat,  struct query_type_st *pqt) {
   int num_res = 0 ;
   strcpy(prt->reply_descr, "Kaplan" ) ;
   prt->reply_tag = pqt->query_tag ;
   num_res = kaplan_calc(  p_UEss )  ;
   JGMDPRT(6,"kaplan_calc returns %d fields calculated\n",num_res) ;
   return (num_res) ;
}
int karpin_reply(   USER_VALUES_k *pr, struct reply_type_st *prt, DEALDATA_k *pdl_dat,  struct query_type_st *pqt) {
   int num_res = 0 ;
   strcpy(prt->reply_descr, "Karpin" ) ;
   prt->reply_tag = pqt->query_tag ;
   num_res = karpin_calc(  p_UEss )  ;
   JGMDPRT(6,"karpin_calc returns %d fields calculated\n",num_res) ;
   return (num_res) ;
}
int karpb_reply(    USER_VALUES_k *pr, struct reply_type_st *prt, DEALDATA_k *pdl_dat,  struct query_type_st *pqt) {
   int num_res =0 ;
   strcpy(prt->reply_descr, "KARP_B" ) ;
    prt->reply_tag = pqt->query_tag ;
   num_res = karpb_calc(  p_UEss )  ;
   JGMDPRT(6,"karpb_calc returns %d fields calculated\n",num_res) ;
   return (num_res) ;
}
int knr_reply(      USER_VALUES_k *pr, struct reply_type_st *prt, DEALDATA_k *pdl_dat,  struct query_type_st *pqt) {
   int num_res = 0 ;
   strcpy(prt->reply_descr, "KnR_Four_Cs with Fit Adj" ) ;
   prt->reply_tag = pqt->query_tag ;
   JGMDPRT(7,"KnR_reply Calling knr_calc\n") ;
   num_res = knr_calc(  p_UEss )  ;
   JGMDPRT(6,"knr_calc returns %d fields calculated\n",num_res) ;
   return (num_res) ;
}
int larsson_reply(  USER_VALUES_k *pr, struct reply_type_st *prt, DEALDATA_k *pdl_dat,  struct query_type_st *pqt) {
   int num_res = 0 ;
   strcpy(prt->reply_descr, "Larsson" ) ;
   prt->reply_tag = pqt->query_tag ;
   num_res = lar_calc(  p_UEss )  ;
   JGMDPRT(6,"lar_calc returns %d fields calculated\n",num_res) ;
   return (num_res) ;
}
int lar_b_reply(    USER_VALUES_k *pr, struct reply_type_st *prt, DEALDATA_k *pdl_dat,  struct query_type_st *pqt) {
   int num_res = -1 ;
   strcpy(prt->reply_descr, "lar_b" ) ;
    prt->reply_tag = pqt->query_tag ;
   num_res = lar_b_calc(  p_UEss )  ;
   JGMDPRT(6,"lar_b_calc returns %d fields calculated\n",num_res) ;
   return (num_res) ;
}
int pav_reply(      USER_VALUES_k *pr, struct reply_type_st *prt, DEALDATA_k *pdl_dat,  struct query_type_st *pqt) {
   int num_res = -1 ;
   strcpy(prt->reply_descr, "Pavlicek" ) ;
   prt->reply_tag = pqt->query_tag ;
   num_res = pav_calc(  p_UEss )  ;
   JGMDPRT(6,"pav_calc returns %d fields calculated\n",num_res) ;
   return (num_res) ;
} /* end pav_reply */
int sheinw_reply(   USER_VALUES_k *pr, struct reply_type_st *prt, DEALDATA_k *pdl_dat,  struct query_type_st *pqt) {
   strcpy(prt->reply_descr, "Sheinwold" ) ;
   int num_res = 0;
   prt->reply_tag = pqt->query_tag ;
   num_res = sheinw_calc( p_UEss )  ;
   JGMDPRT(6,"=====sheinw_calc returns %d fields calculated, res[0]=%d, res[3]=%d\n",num_res, pr->u.res[0],pr->u.res[3]) ;
   return (num_res) ;
} /* end sheinw_reply */
int roth_reply(     USER_VALUES_k *pr, struct reply_type_st *prt, DEALDATA_k *pdl_dat,  struct query_type_st *pqt) {
   strcpy(prt->reply_descr, "Roth" ) ;
   int num_res = 0;
   prt->reply_tag = pqt->query_tag ;
   /* Future: pass the suit parameter to roth_calc? pqt->query_suit */
   num_res = roth_calc(  p_UEss )  ;
   JGMDPRT(6,"=====roth_calc returns %d fields calculated, res[0]=%d, res[3]=%d\n",num_res, pr->u.res[0],pr->u.res[3]) ;
   return (num_res) ;
} /* end roth_reply */
int zarbas_reply(   USER_VALUES_k *pr, struct reply_type_st *prt, DEALDATA_k *pdl_dat,  struct query_type_st *pqt) {
   strcpy(prt->reply_descr, "Zar Basic" ) ;
   int num_res = 0;
   prt->reply_tag = pqt->query_tag ;
   num_res = zarbas_calc(  p_UEss )  ;
   JGMDPRT(6,"=====zarbas_calc returns %d fields calculated, res[0]=%d, res[3]=%d\n",num_res, pr->u.res[0],pr->u.res[3]) ;
   return (num_res) ;
} /* end zarbas_reply */
int zaradv_reply(   USER_VALUES_k *pr, struct reply_type_st *prt, DEALDATA_k *pdl_dat,  struct query_type_st *pqt) {
   strcpy(prt->reply_descr, "Zar Advanced" ) ;
   int num_res = 0;
   prt->reply_tag = pqt->query_tag ;
   num_res = zaradv_calc( p_UEss )  ;
   JGMDPRT(6,"=====zaradv_calc returns %d fields calculated, res[0]=%d, res[3]=%d\n",num_res, pr->u.res[0],pr->u.res[3]) ;
   return (num_res) ;
} /* end zaradv_reply */
int set88_reply(    USER_VALUES_k *pr, struct reply_type_st *prt, DEALDATA_k *pdl_dat,  struct query_type_st *pqt) {
   int num_res = -1 ;
   strcpy(prt->reply_descr, "Set88 Query" ) ;
   prt->reply_tag = pqt->query_tag ;
   num_res = set88_calc( p_UEss )  ;
   JGMDPRT(6,"set88_calc returns %d fields calculated\n",num_res) ;
   return (num_res) ;
} /* end set88_reply */
int set40_reply(    USER_VALUES_k *pr, struct reply_type_st *prt, DEALDATA_k *pdl_dat,  struct query_type_st *pqt) {
   int num_res = -1 ;
   strcpy(prt->reply_descr, "40 All_UE Query" ) ;
   prt->reply_tag = pqt->query_tag ;
   num_res = set40_calc(  p_UEss )  ;
   JGMDPRT(7,"set40_reply_calc returns %d fields calculated\n",num_res) ;
   return (num_res) ;
} /* end set40 reply */
int set41_reply(    USER_VALUES_k *pr, struct reply_type_st *prt, DEALDATA_k *pdl_dat,  struct query_type_st *pqt) {
   int num_res = -1 ;
   strcpy(prt->reply_descr, "41 All_ALTHCP Query" ) ;
   prt->reply_tag = pqt->query_tag ;
   num_res = set41_calc(  p_UEss )  ;
   JGMDPRT(5,"set41_reply_calc returns %d fields calculated\n",num_res) ;
   return (num_res) ;
} /* end set41 reply */
int set42_reply(    USER_VALUES_k *pr, struct reply_type_st *prt, DEALDATA_k *pdl_dat,  struct query_type_st *pqt) {
   int num_res = -1 ;
   strcpy(prt->reply_descr, "42 ALL Side + Raw Query" ) ;
   prt->reply_tag = pqt->query_tag ;
   num_res = set42_calc(  p_UEss )  ;
   JGMDPRT(4,"set42_reply_calc returns %d fields calculated\n",num_res) ;
   return (num_res) ;
} /* end set41 reply */
int unk_reply(      USER_VALUES_k *pr, struct reply_type_st *prt, DEALDATA_k *pdl_dat,  struct query_type_st *pqt) {
   strcpy(prt->reply_descr, "Unknown" ) ;
   prt->reply_tag = pqt->query_tag ;
   for (int i = 0 ; i < 4 ; i++ ) {
      pr->u.res[i] = 0 - pdl_dat->hs[0].hs_points[i] ;
   }
   fprintf(stderr, "ERROR!! Unknown Metric %d in unk_reply called from userfunc \n", pqt->query_tag ); 
 return (0) ;
}
int mixed_KARreply( USER_VALUES_k *pr, struct reply_type_st *prt, DEALDATA_k *pdl_dat,  struct query_type_st *pqt) {
   int num_res =0 ;
   strcpy(prt->reply_descr, "MIX Test Karpin(Milton) vs JGM(BumWrap)" ) ;
    prt->reply_tag = pqt->query_tag ;
   num_res = mixed_Karpin_calc( p_UEss )  ;
   JGMDPRT(6,"mixed_KARcalc returns %d fields calculated\n",num_res) ;
 return (num_res) ;
}
int mixed_LARreply( USER_VALUES_k *pr, struct reply_type_st *prt, DEALDATA_k *pdl_dat,  struct query_type_st *pqt) {
   int num_res =0 ;
   strcpy(prt->reply_descr, "MIX Test Karpin(Milton) vs JGM(BumWrap)" ) ;
    prt->reply_tag = pqt->query_tag ;
   num_res = mixed_Larsson_calc( p_UEss )  ;
   JGMDPRT(6,"mixed_LARcalc returns %d fields calculated\n",num_res) ;
 return (num_res) ;
}
int test_reply(     USER_VALUES_k *pr, struct reply_type_st *prt, DEALDATA_k *pq,  struct query_type_st *pqt) {
   int num_res =0 ;
   strcpy(prt->reply_descr, "Testing Reply" ) ;
    prt->reply_tag = pqt->query_tag ;
   JGMDPRT(6, "**** Calling Testing Calc with pqt->query_side=%d \n",pqt->query_side );
   num_res = test_calc( p_UEss )  ;
   JGMDPRT(6,"testing_calc returns %d fields calculated\n",num_res) ;
 return (num_res) ;
}
/* Debug function called via DBGDO (n ... ) */



