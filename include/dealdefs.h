  /* File dealdefs.h -- */
#ifndef DEALDEFS_H
#define DEALDEFS_H 1
  /* Date      Version   Author  Description
   * 2022/01/02 1.0.0    JGM     Collect all dealer symbolic constants and macros in one place.
   * 2022/02/09 2.1.5    JGM     FD shapes, and printrpt ported from deal_v3
   * 2022/10/05 2.3.0    JGM     Added Bucket Frequency histograms functionality. Re-Orged globals.c treatment.
   * 2022/11/15 2.5.0    JGM     Added UserEval client / server functionality.
   * 2023/01/07 -- Merged in changes from V4 to fix predeal; dealcards_subs.c and globals, etc.
   * 2023/01/26 2.5.5    JGM     Changed Version to reflect modified EvalContract and working UserEval
   * 2023/05/29 2.5.8    JGM     More UserEval integration; used qsort in some cases.
   * 2023/08/04 3.0.0    JGM     New version number for new GCC compiler version incompatible with old. Minor title fix
   * 2023/08/04 3.0.2	 JGM 	 Redo rplib_fix. Create rp_err_check etc. 
   * 2023/10/10 4.0.0    JGM     Finalize switch to Mint21 code and libraries because of new GCC/g++ compilers 
   * 							 Also modify Precedence and Associativity for ?: to fix nesting bug. 
   * 2023/11/10 4.0.1    JGM	 Cleaned up HASKARD, HAS_KARD, HASCARD, HAS_CARD usage and defintions. eliminated unused ones.
   * 2024/01/13 4.1.0    JGM   Renamed the Library funcs and vars from rp* to zrd*. Created zrd vars for writing own Library.
   * 2024/05/15 4.2.3    JGM   Added roth_calc.c to UserEval req'd some changes to dealdebug.c
   * 2024/05/15 4.2.4    JGM   Modified directory structure. Cleanup pathnames; Prep for Github upload
   * 2024/08/05 4.2.5    JGM   Minor mods to work with modified UserEval  Dealer Server
   * 2024/09/21 4.3.0    JGM   Minor mod to mmap_template
   * 2025/04/05 4.3.1    JGM   Post Xia Upgrade gcc version change
   * 2025/04/16 4.3.2    JGM   Allow one of two names for default Library.
   */

  /* Make the header file guard .. */

#ifndef _GNU_SOURCE
  #define _GNU_SOURCE
#endif
#define BUILD_DATE "2025/04/05"
#ifndef JGMDBG
  #define VERSION "4.3.2"
#else
  #define VERSION "104.3.2"
#endif

#ifndef UNUSED
  #define UNUSED(x) ((void)(true ? 0 : ((x), void(), 0)))
                                // was x++
#endif

#ifndef NOTUSED
   #define NOTUSED(v) ( (void)(v) )
#endif
  /* this next one used in the  dealyacc.y file in bias deal code converts -ve numbers to zero, leaves +ve numbers as is*/
#ifndef TRUNCZ
 #define TRUNCZ(x) ((x)<0?0:(x))
#endif
#ifndef TRUE
	#define TRUE 1
#endif
#ifndef FALSE
	#define FALSE 0
#endif

#define ARRAY_SIZE(arr) ( (sizeof( (arr) ) / sizeof( (arr)[0] ) )
#define MIN(x,y)    ( ( (x) < (y) ) ? (x) : (y) )
#define MAX(x,y)    ( ( (x) > (y) ) ? (x) : (y) )
 // This swap is not as efficient as t=x;x=y;y=t; but is not type dependent nor need a tmp variable.
#define SWAP(x,y) { (x) ^= (y) ; (y) ^= (x) ; (x) ^= (y) ; }
#define CONCAT(p1 , p2) p1 p2

#ifndef DEALER_PATHS_H
#define DEALER_PATHS_H 1
#define SUDO_USER
#ifdef SUDO_USER
  #define DEAL_ROOT "/usr/local/games/"
#elif defined( MY_USER )
   #define DEAL_ROOT "/home/" MY_USER        // pass -DMY_USER=\"$USER\"  to gcc on the cmd line 
#else
  #define DEAL_ROOT "/usr/local/games"
#endif
#define DEAL_VER_DIR "DealerV2_4/"


#define DEAL_DIR   CONCAT(DEAL_ROOT , DEAL_VER_DIR)
#define DAT_DIR    CONCAT( DEAL_DIR , "dat/")
#define EXE_DIR    CONCAT( DEAL_DIR , "bin/")   
#define LIB_DIR    CONCAT( DEAL_DIR , "lib/")
#define SRC_DIR    CONCAT( DEAL_DIR , "src/")
#define HDR_DIR    CONCAT( DEAL_DIR , "include/")
#define DBG_DIR    CONCAT( DEAL_DIR , "Debug")
#define PROD_DIR   CONCAT( DEAL_DIR , "Prod")
#define UEV_DIR    CONCAT( DEAL_DIR , "UserEval")
// no reason yet to define Examples, DebugExamples, Regression
// stdlib is special; it contains copies of standard Linux DLLs (.so) that might be Linux version dependant

#define SERVER_PGM CONCAT(EXE_DIR, "DealerServer" ) // Note we use the Prod version of UserEval server even for DBG version of Dealerv2
#define OPC_PGM    CONCAT(EXE_DIR, "dop" )    // ln -s DEAL_ROOT/DOP/dop
#define FDP_PGM    CONCAT(EXE_DIR, "fdp")     // ln -s LIB_DIR/fdp // In /usr/bin linux utility 'fdp' is related to graphviz
#define TINYLIB    CONCAT(DAT_DIR, "rpdd_Lib.zrd" ) // A small 20 deal file included in GitHub repo
#define BIGLIB     CONCAT(DAT_DIR, "rpLib.zrd"    ) // The full 240MB library if user has installed separately
#define ZRD_LIB    BIGLIB                           // -L= will use BIGLIB if avail, else will fall back to TINYLIB
/* this next one added because Linux has a BRIDGE utility that refers to ethernet cards */
/* will have to make sure there is an ln /usr/games/gibcli to /usr/games/bridge which is the real name of the GIB binary*/
#define DD_PGM "/usr/games/gibcli"
#endif  /* DEALER_PATHS_H*/


#define SERVER_PATH_SIZE 255
#define SUCCESS 1
#define FAILED  0
#define NIL ((struct tree *) 0)
#define CACHE_INV 0
#define CACHE_OK  1
#define CACHE_UPD 2
#define BIG_XS    999    /* for BIAS DEAL heuristic */
#define NO_XS      99    /* for BIAS DEAL heuristic */


/* ENUMS need spot, no suit = -1 to force enum type to signed int instead of unsigned int */
enum rank_ek     {TWO=0, DEUCE=0, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE, TEN, JACK, QUEEN, KING, ACE, SPOT=-1 } ;
enum suit_ek     {CLUBS=0, DIAMONDS, HEARTS, SPADES, NOTRUMP, nosuit=-1 } ;
enum dealmode_ek {INV_MODE= -1, DEF_MODE = 0, LIB_MODE, BIAS_MODE, PREDEAL_MODE, SWAP_MODE   };
enum deal_err_ek {DL_ERR_FATAL=-9, EIOERR=-4, DL_ERR_BIAS= -2, DL_ERR_ZRDLIB= -1, DL_OK = 1 };
enum csv_type_ek {INV_CSV=-1, CSV_EXPR=1, CSV_STR=2, CSV_HMASK=4, CSV_TMASK=8, CSV_PARCONTRACT=16 } ;
enum csv_par_ek  {INV_CSVP=-1, CSV_NVUL=1, CSV_NSVUL=2, CSV_EWVUL=4, CSV_BVUL=8, CSV_QVUL=16  } ; /* bit mask for par vulnerabilities */ 

 /* these next ones mostly by JGM */
 /* These next three numbers do NOT include room for a terminating NULL char */
 #define DEALSTRSIZE 76
 #define COMPACT_DEAL_SIZE 96
 #define PBNLINE 69
 #define MAXLINE 255			/* allocate 256 i.e. MAXLINE + 1 */
 #define MAXBUFFSZ 255    /* Should not be any buffers in Dealer that are bigger than 255 */

/* ZRD_BLOCKSIZE will be adjusted at run time based on the number of records in the Library file
 * The max_seed value will also be adjusted based on the blocksize
 * If there are more than 10000 records in the Library file the blocksize will be 1000
 */
#define ZRD_REC_SIZE 23
#define ZRD_BLOCKSIZE 1000
#define ZRD_MAX_SEED 10485                 // because the Pavlicek Library has 10,485,760 deals in it.
#define ZRD_MAX_LIBRECS 10485760

#define MAXTITLE        255
#define LTC_VOID_WEIGHT 128
#define SUIT_CLUB       0
#define SUIT_DIAMOND    1
#define SUIT_HEART      2
#define SUIT_SPADE      3
#define SUIT_NT         4
#define NSUITS          4
#define NSTRAINS        5

  /* following macros changed by ;;; JGM  */
  /* was shift <<6, >>6 and mask 0x3F  */
  /* if shift by 4  then card can be a signed char, and we avoid a bunch of obnoxious casting */
#define CARD_RANK(c) (  (int) ( (c)&0x0F )     )
#define CARD_SUIT(c) ( ((int) ( (c)>>4)&0x0F)  )
#define MAKECARD(suit,rank) ( (char) (((suit)<<4) | ((rank)&0x0F)) )
#ifndef C_SUIT
  #define C_SUIT(c)    ( (int) ((c)>>4)&0xF )
  #define C_RANK(c)    ( (int)  (c)&0xF     )
#endif
#define NO_CARD     0x7F   /* was 0xFF; changed when card became signed char */
#define IS_CARD(c)   ( (( 0x00 <= (c) && (c) <= 0x3C ) ? 1 : 0) )

/* HAS_CARD redefined. 2022/11/10; Analyze now populates the array Has_card[h][13] simplifying a frequent lookup */
#define HAS_CARD(p,s,r) hs[(p)].Has_card[ (s) ][ (r) ]				// 0 if not in hand, 1 if in hand

#define NSEATS          4
#define SIDE_NS         0
#define SIDE_EW         1
#define COMPASS_NORTH   0
#define COMPASS_EAST    1
#define COMPASS_SOUTH   2
#define COMPASS_WEST    3
#define COMPASS_NS      4
#define COMPASS_EW      5

  /* These next ones were defined in tree.h Used only to set the maxvul global for use by the printpbn function*/
#define VULNERABLE_NONE 0
#define VULNERABLE_NS   1
#define VULNERABLE_EW   2
#define VULNERABLE_ALL  3
#define NVULS           4

/* these are used by evalcontract */
#define NON_VUL 0
#define VUL     1

  /* Types of Expression trees. This looks like it could be in an enum but never mind */
#define TRT_NUMBER      0
#define TRT_AND2        1
#define TRT_OR2         2
#define TRT_CMPEQ       3
#define TRT_CMPNE       4
#define TRT_CMPLT       5
#define TRT_CMPLE       6
#define TRT_CMPGT       7
#define TRT_CMPGE       8
#define TRT_LENGTH      9
#define TRT_ARPLUS      10
#define TRT_ARMINUS     11
#define TRT_ARTIMES     12
#define TRT_ARDIVIDE    13
#define TRT_ARMOD       14
#define TRT_HCPTOTAL    15
#define TRT_HCP         16
#define TRT_SHAPE       17
#define TRT_NOT         18
#define TRT_HASCARD     19
#define TRT_IF          20
#define TRT_THENELSE    21
#define TRT_LOSERTOTAL  22
#define TRT_LOSER       23
#define TRT_TRICKS      24
#define TRT_RND         25
#define TRT_CONTROL     26
#define TRT_CONTROLTOTAL   27
#define TRT_SCORE       28
#define TRT_IMPS        29
#define TRT_CCCC        30
#define TRT_QUALITY     31
#define TRT_PT0TOTAL    32
#define TRT_PT0         33
#define TRT_PT1TOTAL    34
#define TRT_PT1         35
#define TRT_PT2TOTAL    36
#define TRT_PT2         37
#define TRT_PT3TOTAL    38
#define TRT_PT3         39
#define TRT_PT4TOTAL    40
#define TRT_PT4         41
#define TRT_PT5TOTAL    42
#define TRT_PT5         43
#define TRT_PT6TOTAL    44
#define TRT_PT6         45
#define TRT_PT7TOTAL    46
#define TRT_PT7         47
#define TRT_PT8TOTAL    48
#define TRT_PT8         49
#define TRT_PT9TOTAL    50
#define TRT_PT9         51
/* Next added by JGM   */
#define TRT_DECNUM      52
#define TRT_LTC         53
#define TRT_LTCTOTAL    54
#define TRT_DDS         55
#define TRT_PAR         56
#define TRT_OPCCOMPASS  57
#define TRT_OPCSIDE     58
#define TRT_USEREVAL    59

/* ---Action Types another possible enum --- */

#define ACT_PRINTALL     0
#define ACT_PRINT        1
#define ACT_AVERAGE      2
#define ACT_FREQUENCY    3
#define ACT_PRINTCOMPACT 4
#define ACT_PRINTONELINE 5
#define ACT_PRINTPBN     6
#define ACT_PRINTEW      7
#define ACT_PRINTNS      8
#define ACT_FREQUENCY2D  9
#define ACT_PRINTES      10
#define ACT_EVALCONTRACT 11
#define ACT_PRINTSIDE    12
#define ACT_EXP_SIDE_HLD 13
#define ACT_EXP_SEAT_HLD 14
#define ACT_CSVRPT       15
#define ACT_PRINTRPT     16
#define ACT_BKTFREQ      17
#define ACT_BKTFREQ2D    18


/* Constants for CCCC and Quality */
#define C4_TYPE         int
#define RK_ACE          12
#define RK_KING         11
#define RK_QUEEN        10
#define RK_JACK          9
#define RK_TEN           8
#define RK_NINE          7
#define RK_EIGHT         6

#define TWO_TO_THE_13 (1<<13)

/*          Distribution Shape clauses 
 * Since int is 4 bytes get 32 different shape statements.
 * On 64 bit system could get 64 shape statements if were to use long ints.
 * This would require changing several declarations in various locations.
 */
#define MAXDISTR    8*sizeof(int)
/*
#define MAXDISTR    8*sizeof(long)
*/

/* --------------------JGM LATER ADD ONS ------------------------------- */
  // #define RANDOM drand48
#define RANDOM(top)  gen_rand_slot( (top) )  // returns int in range 0 .. top-1 inclusive by truncating a double 
#define SRANDOM(s)   init_rand48((s))

#endif /*ifndef DEALDEFS_H */




