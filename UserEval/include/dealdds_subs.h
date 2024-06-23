/* File deal_dds.h -- to export dealdds_subs IF to Dealer code */
#ifndef DEAL_DDS_H
#define DEAL_DDS_H
#ifndef RET_NO_FAULT
  #define RET_NO_FAULT 1
#endif

/* define these next ones individually to avoid importing a bunch of dealer defs we don't need that might conflict with dds defs*/
typedef char CARD52_k ; 
typedef CARD52_k DEAL52_k[52] ;

struct Dealer_Results_st {      	// Struct passed back from IF to Dealer
    int CacheStatus ;  				// 0 = stale, 1=update mode1 2=update mode2
    int parScore_NS ;				// backwards compat. Score for the Default ParVuln set by -P on cmd line.
    int parScores_NS[4] ;       	// One for each Vulnerability in case user asks. Dealer Coding.
    int tricks[4][5] ;          	// [hands=n,e,s,w][strains = c,d,h,s,n]
    int errcode ; 					// 1 = ok, -1 = failure
    char ParContracts[4][20] ;  	// Typical string= 4H* by NS both or 15 chars plus the Null at end. 
    char ddsmsg[40];  // in case DDS fails get some of the errmsg. Also used for Par contract strings
} ;
typedef struct Dealer_Results_st DDSRES_k ;
extern DDSRES_k dds_res_bin;

extern int dds_mode;
extern int dds_tricks(int compass, int strain );  //uses globals ngen, dds_dealnum, dds_res_bin, dds_mode ngen for cache refresh
extern int dds_parscore(int compass, int vuln ); /* uses globals incl debug counts */
extern int csv_trix( char *buff, int h_mask ) ;  //ngen, dds_dealnum, dds_res_bin, dds_mode are globals
extern int SetDDSmode(int mode);
DDSRES_k true_CalcTable(DEAL52_k  dl, int par_vul, int dds_mode ) ; // mode always 2; par_vul=-1 no Par calcs; dealer coding */
int      true_SolveBoard(DEAL52_k  curdeal, int compass, int strain ) ; // Dealer coding for compass, strain
int CheckCache ( DDSRES_k *Results, int ngen, int ddsnum , int h, int s ) ; // dealdds_subs only at present 

#endif
