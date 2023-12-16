/* File deal_scorelib.c -- various routines related to calculating bridge scores */
/* 2023/11/03   1.0		JGM		Extracted from dealeval_subs.c 
 * 
 */

#ifndef _GNU_SOURCE
  #define _GNU_SOURCE
#endif
#include "../include/dbgprt_macros.h"
#include "../include/std_hdrs.h"
#include "../include/dealdefs.h"
#include "../include/dealtypes.h"
#include "../include/dealdebug_subs.h"

#include "../include/deal_scorelib.h"

int IMP_tbl[24] = { 10,   40,   80,  120,  160,  210,  260,  310,  360,  410,  490,  590,
-              740,  890, 1090, 1190, 1490, 1740, 1990, 2240, 2490, 2990, 3490, 3990  };

int imps (int scorediff) {
  int i, j;
  j = abs (scorediff);
  for (i = 0; i < 24; i++)
    if (IMP_tbl[i] >= j) return scorediff < 0 ? -i : i;
  return scorediff < 0 ? -i : i;
} /* end imps() */

int undertricks( int utricks, int vul, int dbl_flag) {  // dbl_flag is either 1 (dbled) or 2 (re-dbled)
    int score_res ;

    JGMDPRT(7, "SCORE_DBL_Undertricks: utricks=%d, vul=%d, dbl_flag=%d\n",utricks, vul, dbl_flag);

    if (vul == 1 ) {
        score_res = (300*utricks - 100) * dbl_flag ; // (200, 500, 800, etc. ) x2 for redoubled)
        return -score_res;
    }
    if (utricks <= 3) {  // Non Vul Dbled first 3 tricks are 100, 300, 500 penalty
        score_res = (200*utricks - 100) * dbl_flag ; // (100, 300, 500) x2 for redoubled
        return -score_res ;
    }
    else {              // non Vul Dbled first 3 tricks are 500, next tricks are 300 each.
        score_res = (300*utricks - 400) * dbl_flag ; // (800, 1100, 1400, etc. ) x2 for redoubled
        return -score_res ;
    }
} /* end undertricks */

int trickscore(int strain, int tricks ) {  // tricks is total taken; only get pts for tricks > 6
    int score_res ;
    score_res = ((strain >= SUIT_HEART) ? 30 : 20) * (tricks - 6);

  /* NT bonus */
  if (strain == SUIT_NT) score_res += 10;
  JGMDPRT(7, "SCORE_DBL_trickscore: strain=%d, tricks=%d, score_res=%d\n",strain, tricks, score_res);
  return score_res ;
}

int undbled_score (int vuln, int suit, int level, int tricks) { // handles undertricks, overtricks and just making
  JGMDPRT(7, "undbled_SCORE: vul=%d,suit=%d,level=%d, tricks=%d\n",vuln, suit, level, tricks);
  int total = 0;
  /* going down. undoubled */
  if (tricks < 6 + level) return -50 * (1 + vuln) * (6 + level - tricks);

  /* Contract made, calculate positive score */
  /* Tricks score */
  total = total + ((suit >= SUIT_HEART) ? 30 : 20) * (tricks - 6);

  /* NT bonus */
  if (suit == SUIT_NT) total += 10;

  /* part score bonus */
  total += 50;

  /* game bonus for NT  minus partscore bonus */
  if ((suit == SUIT_NT) && level >= 3) total += 250 + 200 * vuln;

  /* game bonus for major minus partscore bonus */
  if ((suit == SUIT_HEART || suit == SUIT_SPADE) && level >= 4) total += 250 + 200 * vuln;

  /* game bonus for minor minus partscore bonus */
  if ((suit == SUIT_CLUB || suit == SUIT_DIAMOND) && level >= 5) total += 250 + 200 * vuln;

  /* small slam bonus */
  if (level == 6) total += 500 + 250 * vuln;

  /* grand slam bonus */
  if (level == 7) total += 1000 + 500 * vuln;

  return total;
} /* end undbled score() */

int dbled_making ( int dbl_flag, int vul, int strain, int ctricks) { // score for making  dbled or redbled contract exactly
    #ifdef JGMDBG
    if(jgmDebug >= 7 )
        fprintf(stderr, "SCORE_DBL_making: ctricks=%d, strain=%d, vul=%d, dbl_flag=%d\n",ctricks, strain, vul, dbl_flag);
    #endif
    int t0;
    int score_res = 0;

    t0 = trickscore(strain, ctricks );
    if (dbl_flag == 1 ) t0 += t0 ;
    else                t0 = t0 * 4 ; // Redoubled
    score_res = t0 ;
     if(jgmDebug >= 7) { JGMDPRT(7, "trick score = %d\n ",score_res ); }
    // Game and Slam Bonuses do not get x2; and you must bid 6 to get slam

    if ( t0 >= 100 ) {  // game bonus
        score_res += 300 + vul*200 ;
     if(jgmDebug >= 7) {    JGMDPRT(7,"+game Bonus=%d\n ",score_res  ); }
    }
    else {  // part_score bonus
        score_res += 50 ;
    }
    score_res += 50*dbl_flag;  //  insult bonus 50 for dbled, 100 for redbled

    if ( ctricks == 12 ) { // small slam bonus
        score_res += 500 + 250*vul ;
    }
    if ( ctricks == 13 ) { // Grand Slam bonus
        score_res += 1000 + 500*vul ;
     }
     if(jgmDebug >= 7) {JGMDPRT(7,"returning  DBL/RDBL making score=%d\n ", score_res ); }
    return score_res ;
} /* end dbled_making */
/* the coded contract is an int that encapsulates the strain, level, and Doubled/Redoubled status. Does not include Vulnerability*/
int GET_dbled( int coded_contract) { return coded_contract / 40 ; } /* 0=not dbled, 1= dbled, 2=re_dbled */
int GET_strain(int coded_contract) { return (coded_contract % 40) % 5 ; } /* 0 = Clubs, 4 = NT */
int GET_level( int coded_contract) { return (coded_contract % 40) / 5 ; } /* 1 to 7 */

int calc_score(int vul, int dbl_flag, int strain, int level, int tricks ) {
	int ctricks ; 
	int t0, t1 ; 
	int score_res ; 
    ctricks = 6 + level ;           /* number of tricks contracted for */
    #ifdef JGMDBG
      if(jgmDebug >= 7 )
        fprintf(stderr, "NEW_SCORE:: Vul=%d, DBLFLG=%d, Strain=%d, Level=%d,tricks=%d\n",
        vul,dbl_flag,strain,level,tricks);
    #endif
    if (dbl_flag == 0 ) {   /* undoubled use simple score function */
        if(jgmDebug >= 7) { JGMDPRT(7,"SCORE_DBL:: Calling undbled_score-- ctricks =%d\n",ctricks); }
       return undbled_score( vul, strain,  level,  tricks) ;
    }
    assert (dbl_flag ==1 || dbl_flag == 2 ) ;

    if ( ctricks > tricks ) {  /* contract went down */
       score_res = undertricks( (ctricks - tricks), vul, dbl_flag) ;
       if(jgmDebug >= 7) { JGMDPRT(7,"NEW_SCORE_DBL::Returning UnderScore=%d\n",score_res); }
       return score_res ;
    }
    score_res = dbled_making(dbl_flag, vul, strain, ctricks);  // includes, partscore/game/slam & insult bonus for DBL or RDBL
    if(jgmDebug >= 7) { JGMDPRT(7,"NEW_SCORE_DBL:: Making=%d\n",score_res); }
    if ( ctricks == tricks ) { /* contract made; no overtricks */
       return score_res ;
    }
        /* Contract made with overtricks. Add the Overtricks bonus to the making score */
    t1 = tricks - ctricks ;
    if(jgmDebug >= 7) { JGMDPRT(7,"SCORE_DBL:: Overtricks=%d \n",t1); }
    t0 = t1*100*(1+vul); // NV dbled is 100/trick, Vul Dbled is 200/trick
    t0 = t0*dbl_flag ; // RDBL OT are 2x DBL OT.
    score_res += t0 ;
     if(jgmDebug >= 7) { JGMDPRT(7,"SCORE_DBL:: Final result for +ve DBLed contract = %d\n",score_res ); }
    return score_res ;
} /* end calc_score */

int score(int vul, int coded_contract, int tricks ) {
    int dbl_flag, strain, level ;
    int t0, score_res;

    dbl_flag = coded_contract / 40 ;  // 0, 1, or 2
    t0 = coded_contract % 40 ;        // 5..39
    strain = t0 % 5 ;                 // 0 = Clubs, 4 = NT
    level  = t0 / 5 ;                // 1 - 7
    score_res = calc_score(vul, dbl_flag, strain, level, tricks) ;
    return score_res ; 
}
