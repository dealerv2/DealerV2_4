/* File SS_Values.c  -- Table documenting how the various metrics account for short honors
 *  Version  Date       Author  Comment
 *    0.5    2024-08-17  JGM    First ; This file not used for anything except documentation
 */
#ifndef SS_VALUES_H
#define SS_VALUES_H
#ifndef USEREVAL_TYPE_H
#include "UserEval_types.h"
#endif
/* ---------------  Decided not to use this table as it is too hard to keep straight dpts and HCP ---- */
/* Table of Short Suit Values in NT and Suits by Metric ID
 * We give the NET value since it is too complicated to give a corrected value
 * then correct the corrected value depending on what flavor of short suit evaluation the metric does
 * For metrics that evaluate length not shortness, the initial valuation for NT and suit, will be the same.
 * When we do not know the method used, eg ?? then we assume one of the 'standard' deductions, Goren, Kaplan, Bergen ??
 */
 /* The 22 items in question from Stiff Ace down to dblton spots */

 /* The Various Metrics for some this array may be irrelevant as the values are built into the algorithm e.g. KnR
  * BERGEN=0, BISSEL,  DKP, GOREN, KAPLAN, KARPIN, KARP_B, KnR, LAR, LAR_B, PAV, SHEINW, ZAR, Roth} ;
  */
 /* The 3D array; [Metric][NT=0,Suit=1][22 ShortSuitItemsList]*/
 float ss_vals[metricEND][2][ss_END] = {
    //            <------------No Trump Value ----------------------->  <----------- a-priori Value in Suit Contract----->
    //              A         x  AK     Ax KQ     Kx  QJ    JT    Tx      A          x  AK     Ax KQ    Kx  QJ     JT    Tx
  {  /* Bergen  */ {4,2,1,0,0,0, 7,6,5,4,4, 5,4,3,3,  2,1,1, 1,0, 0,0 }, {4,2,1,0,0,0, 7,6,5,4,4, 5,4,3,3, 2,1,1, 1,0, 0,0 }},
  {  /* Bissel  */ {3,2,1,0,0,0, 6,5,4,3,3, 4,3,2,2,  2,1,1, 0,0, 0,0 }, {3,2,1,0,0,0, 6,5,4,3,3, 4,3,2,2, 2,1,1, 0,0, 0,0 }},
  {  /* DKP N/A */ {12,8,4,1,0,0, 21,17,14,13,13, 13,10,9,9, 6,4,4, 1,1, 0,0}, /*13-9-5-2 Minus 1 for no low card or unguarded Q,J*/
                   {12,8,4,1,0,0, 21,17,14,13,13, 13,10,9,9, 6,4,4, 1,1, 0,0}},
  {  /* Goren   */ {4,1,0,0,0,0, 7,6,5,4,4, 5,4,3,3,  2,1,1, 0,0, 0,0 }, {6,3,2,2,2,2, 8,7,6,5,5, 6,5,4,4, 4,2,2, 1,1, 1,1 }},
  {  /* Kaplan  */ {4,3,0,0,0,0, 7,6,5,4,4, 5,4,3,3,  2,2,2, 1,1, 0,0 }, {4,3,0,0,0,0, 7,6,5,4,4, 5,4,3,3,  2,2,2, 1,1, 0,0 }},
  {  /* Karpin  */ {4,2,1,0,0,0, 7,6,5,4,4, 4,3,3,3,  2,1,1, 0,0, 0,0 }, {4,2,1,0,0,0, 7,6,5,4,4, 4,3,3,3,  2,1,1, 0,0, 0,0 }},
  {  /* KARP_B  */ {3.5,2,0.5,0,0,0, 6.5,5.5,5,4.5,4.5, 4.5,4,3.5,3, 2,1.5,1, 0.5,0,0,0 },
                   {3.5,2,0.5,0,0,0, 6.5,5.5,5,4.5,4.5, 4.5,4,3.5,3, 2,1.5,1, 0.5,0,0,0 }},
  {  /* KnR-N/A */ {3,0.5,0,0,0, 5,4,4,4,4, 3,2,2,2,  1,1,1, 0,0, 0,0 }, {3,0.5,0,0,0, 5,4,4,4,4, 3,2,2,2,  1,1,1, 0,0, 0,0 }},
  {  /* Larsson */ {4,2,1,0,0,0, 7,6,5,4,4, 4,4,3,3,  2,2,2, 1,1, 0,0 }, {4,2,1,0,0,0, 7,6,5,4,4, 4,4,3,3,  2,2,2, 1,1, 0,0 }},
  {  /* LAR_B   */ {4.5,2,0.5,0,0,0, 7.5,6,5,4.75,4.5, 4.5,3.75,3.25,3, 2,1.5,1, .5,0, 0,0 },
                   {4.5,2,0.5,0,0,0, 7.5,6,5,4.75,4.5, 4.5,3.75,3.25,3, 2,1.5,1, .5,0, 0,0 }},
  {  /* Pavlicek*/ {4,1,0,0,0,0, 7,6,5,4,4, 4,3,3,3,  2,1,1, 0,0, 0,0 }, {6,3,2,2,2,2, 8,7,6,5,5, 5,4,4,4,  3,2,2, 1,1, 1,1 }},
  {  /*Sheinwold*/ {4,2,0,0,0,0, 7,6,5,4,4, 5,4,3,3,  2,1,1, 0,0, 0,0 }, {6,4,2,2,2,2, 8,7,6,5,5, 6,5,4,4,  3,2,2, 1,1, 1,1 }},
  {  /*ZAR:NoCtl*/ {3,2,1,0,0,0, 7,6,5,4,4, 4,3,3,3,  2,1,1, 0,0, 0,0 }, {3,2,1,0,0,0, 7,6,5,4,4, 4,3,3,3,  2,1,1, 0,0, 0,0 }},
  {  /* Roth    */ {4,3,2,1,0,0, 7,6,5,4,4, 5,4,3,3,  3,2,2, 0,0, 0,0 }
// ZAR Subtract 1   * * * *                 * *       * * *  * *          * * * *                 * *       * * *  * *
} ; /* end 3 D array */
    //            <------------No Trump Value ----------------------->  <----------- a-priori Value in Suit Contract----->
    //             A         x   AK      Ax  KQ    Kx QJ     JT   Tx      A         x   AK      Ax KQ     Kx  QJ    JT    Tx


/* Notes:
 * Bissel Honor values (downto the T) are 3 minus Number of missing higher honors. so KQ = 2+2, AK = 3+3 etc.
 *              Nice way to incorporate synergy of touching honors
 * DKP is Using Little Jack Points (13-9-5-2). Array N/A. DK -1 for any suit whose lowest card is higher than a T. +1 if with A or K
 * KARPB based on OPC -- Q(1.5 or 2), J(0,0.5,1), and T(0,.5,1) are variable depending on higher honors.
 * LAR_B based on BumWrap fixed values: A=4.5, K=3, Q=1.5, J=0.75, T=0.25  with ad-hoc deductions
 * Goren short honor deductions: Stiff K, Q -2, Stiff J -1 ; Q-J, Q-x or J-x -1
 * Pavlicek NT values are deduced from the suit values:
 *      e.g. suit stiff K=3; ergo -2 for the stiff K also stiff Q like Goren ; PAV KQ, KJ, QJ, Qx, Jx also deduce -1
 * Sheiwold Guessing based on, Quote: "Qx, Jx not worth full value" and "Stiff K = 4 and hope"
 * ZAR -- Using PAV description. Start with 6-4-2-1; Any stiff -1; Dblton KQ, KJ, QJ, Qx, Jx, -1
 */
#endif
