/* File deal_scorelib.h -- interface to deal_scorelib.c routines */
/*
 * 2023/11/03	1.0		JGM		Initial. Routines extracted from dealeval_subs.c 
 */
#ifndef DEAL_SCORELIB_H
#define DEAL_SCORELIB_H
/* the coded contract is an int that encapsulates the strain, level, and Doubled/Redoubled status. Does not include Vulnerability*/
int GET_dbled( int coded_contract) ; /* 0=not dbled, 1= dbled, 2=re_dbled */
int GET_strain(int coded_contract) ; /* 0 = Clubs, 4 = NT */
int GET_level( int coded_contract) ; /* 1 to 7 */

int calc_score(int vul, int dbl_flag, int strain, int level, int tricks_taken ) ; /* tricks_taken= 0 .. 13 NOT 1 .. 7 */
int dbled_making ( int dbl_flag, int vul, int strain, int tricks_taken) ;         /* score for making DBLed or ReDbled contract. */
int imps (int scorediff) ;                                                        /* convert a score difference to IMPS (plus or minus) */                                             
int score(int vul, int coded_contract, int tricks_taken ) ;                       /* decode the contract then call calc_score */
int trickscore(int strain, int tricks_taken ) ;	                                  /*  score for tricks_taken > 6 ; does not include game or slam bonus */
int undertricks( int utricks, int vul, int dbl_flag) ;  /* score (Re)Doubled UnderTricks(1..13)  DBLed dbl_flag=1 ReDBLed dbl_flag=2*/
int undbled_score (int vuln, int suit, int level, int tricks) ; /* handles undertricks, overtricks and just making; includes game and slam bonus */
/* there is also a function decode_contract from parse_subs that returns a structure. 
 * Used to build the action tree.
 * Not needed in this library since we have the GET functions 
 */
#endif



