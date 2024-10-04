/* File Dfit_Values.c  -- Table documenting how the various metrics calculate 'support' points in a suit contract.
 *  Version  Date       Author  Comment
 *    0.5    2024-08-17  JGM    First ; This file not used for anything except documentation
 */
#ifndef DFIT_VALUES_H
#define DFIT_VALUES_H
#ifndef USEREVAL_TYPE_H
#include "UserEval_types.h"
#endif

 /* The Various Metrics for some this array may be irrelevant as the values are built into the algorithm e.g. KnR
  * BERGEN=0, BISSEL,  DKP, GOREN, KAPLAN, KARPIN, KARP_B, KnR, LAR, LAR_B, PAV, R22, SHEINW, ZAR } ;
  */
 /* The Dfit 4D array; [Metric][8fit,9fit+][2,3,4,5 Trumps][V/S/D] */
 int Dfit_vals[metricEND][2][4][3] = {
    //  Assume Goren if not otherwise known
    //                   2 Trump  3 Trump  4 Trump, 5 Trump                2 Trump  3 Trump  4 Trump, 5 Trump
  { /* Bergen  8Fit */ { {2,0,0}, {3,2,1}, {4,3,1}, {5,3,1}}, /* 9+Fit*/ { {2,0,0}, {3,2,1}, {4,3,1}, {5,3,1} }},
  { /* Bissel No Dfit Pts */{ {0},    {0},     {0},     {0}}, /* 9+Fit*/ { {0},     {0},     {0},     {0}     }},
  { /* DKP     8Fit */ { {0,0,0}, {3,2,1}, {5,3,1}, {5,3,1}}, /* 9+Fit*/ { {0,0,0}, {3,2,1}, {5,3,1}, {5,3,1} }},
  { /* Goren   8Fit */ { {0,0,0}, {3,2,1}, {5,3,1}, {5,3,1}}, /* 9+Fit*/ { {0,0,0}, {3,2,1}, {5,3,1}, {5,3,1} }}, // Replaces SSuit Dpts
  { /* Kaplan  8Fit */ { {0,0,0}, {3,2,1}, {5,3,1}, {5,3,1}}, /* 9+Fit*/ { {0,0,0}, {3,2,1}, {5,3,1}, {5,3,1} }}, // Traditional
  { /* Karpin  8Fit */ { {0,0,0}, {2,1,0}, {3,2,1}, {3,2,1}}, /* 9+Fit*/ { {0,0,0}, {2,1,0}, {3,2,1}, {3,2,1} }},
  { /* KARP_B  8Fit */ { {0,0,0}, {2,1,0}, {3,2,1}, {3,2,1}}, /* 9+Fit*/ { {0,0,0}, {2,1,0}, {3,2,1}, {3,2,1} }}, // Ditto Karpin
  { /* KnR BuiltIn N/A  */  { {0},    {0},     {0},     {0}}, /* 9+Fit*/ { {0},     {0},     {0},     {0}     }},
  { /* Larsson 8Fit */ { {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}}, /* 9+Fit*/ { {0,0,0}, {3,2,1}, {5,3,1}, {5,3,1} }},
  { /* LAR_B   8Fit */ { {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}}, /* 9+Fit*/ { {0,0,0}, {3,2,1}, {5,3,1}, {5,3,1} }}, // Ditto Larsson
  { /*Pavlicek 8Fit */ { {0,0,0}, {0,0,0}, {2,1,0}, {2,1,0}}, /* 9+Fit*/ { {0,0,0}, {0,0,0}, {2,1,0}, {2,1,0} }}, // Add to SSuit
  { /*Sheinwold 8Fit*/ { {0,0,0}, {0,0,0}, {2,1,0}, {2,1,0}}, /* 9+Fit*/ { {0,0,0}, {0,0,0}, {2,1,0}, {2,1,0} }}, // Add to SSuit
  { /*ZAR:No Dfit Pts*/{    {0},     {0},     {0},     {0}},  /* 9+Fit*/ {     {0},     {0},     {0},     {0} }}, // Zar Dfit requires 
  { /*ZARADV:No Dfit */{    {0},     {0},     {0},     {0}},  /* 9+Fit*/ {     {0},     {0},     {0},     {0} }}, // code to calc.
  { /*Roth     8Fit */ { {0,0,0}, {0,0,0}, {2,1,0}, {2,1,0}}, /* 9+Fit*/ { {0,0,0}, {0,0,0}, {2,1,0}, {2,1,0} }}, // Add to SSuit Dpts
} ; /* end Dfit 4D array */

/* Notes:
 * Reference Material often refers to
 *      '9 fit' where they might mean 4 Trump, so does 6-3 qualify?
 *      or to '4 Trump' where they might mean 9 fit so does 4-4 qualify?
 *      and Reference material never states whether in a 4-4 fit (or 5-5 fit) BOTH can count for shortness or not. (PAV does not)
 * KARP_B metric is the same as Karpin but with BumWrap HCP so the same Dfit also.
 * LAR_B is like Larsson but with BumWrap pts. Dfit are same, but not  as in Book. 3-2-1 for 6-3 fit, 5-3-1 for 9+ fit with 4+ trump
 * For Short Suit Methods, Open Question which is easier to code:
 *      PAV where we ADD +2 and +1 to shortness with 4+ Trump; no change for 3 or fewer trump, even in a 9+ fit.
 *      Goren where we REPLACE the original 3-2-1 with the Dfit pts from the Table. Prob Makes no difference.
 * Bergen uses 'N' (number of trumps) with a void, otherwise 3-2-1
 * OPC (not shown but might be used in future) uses 'N-L' with 2,3,or 4 trumps. Nothing for 5 trumps. (Both hands are Decl)
 */
#endif
