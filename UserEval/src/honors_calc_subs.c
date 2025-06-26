/* File honors_calc_subs.c -- by :JGM : code that implements calculating the various short honor deductions */
/* Date      Version  Author  Description
* 2024/108/13 1.0.0    JGM  -- adjShort Honor subs.

*/

#include <assert.h>
#include "../include/dealdefs.h"  /* for enum ranks etc. */
#include "../include/UserEval_types.h"
#include "../include/dbgprt_macros.h"
#include "../include/UserEval_externs.h"

// #include "../include/UserEval_protos.h"  /* for all the Adj functions */

#include <math.h>  /* for type float_t */
#if 0
/* These next two lines for doc only. actual definitions are in UserEval_types.h
 *                     0        1       2     3     4        5       6      7    8    9     10     11       12      13     14       15      
enum metric_ek    { BERGEN=0, BISSEL,  DKP, GOREN, KAPLAN, KARPIN, KARP_B, KnR, LAR, LAR_B, PAV, SHEINW,  ZARBAS, ZARADV, ROTH, metricEND,
             Future: OPC=15, Various HCP scales 20 - 30 etc.               
                    UE_ALL=40,  MixKarp=50, MixLar, SET=88, SYNTST=99, Quit=-1} ;
*/
 /* Notes re various ways evaluating 'points' esp as relates to short Honors.
  * BERGEN: Varies. Usually counts for length as Opener, and for Trump Length AND shortness once a fit is found. lots of uniqueness
  * BISSEL: Deduct -1 or -0.5 as reqd to preserve the relative strengths. Bissell honors are 3,2,1,0 depending on missing higher
  * DKP:    Deduct for any suit where lowest card > Ten. These deductions are BEFORE div by 3;
  * GOREN:  Count shortness initially. Deductions per Pavlicek web site.
  * KAPLAN: Count for length; Deductions for short honors inferred from his book.
  * KARPIN: Count for length; Deductions from Pavlicek website
  * KARP_B: Karpin with BumWrap HCP.
  * KnR:    As per the BW article. Table Entries are placeholders only since the code does it all, these values never referenced
  * LAR:    Count for length; Deductions per his book.
  * LAR_B:  Larsson with BumWrap HCP and Dfit
  * PAV:    Count for shortness. Per the Pavlicek website and online evaluator
  * ZARBAS: Basic Zar Points; HCP + CTLS + (A+B) + (A-D)
  * ZARADV: Basic + Hf_pts + adj for HCP in 2/3 suits + FN_pts for extra trump length etc. No Misfit points.
  * ROTH:   Count both Dpts and Lpts; also Dfit and kind of FN; Dpts demoted often; Lpts 6/7 card suits.
  * metricEND: 15 -
  * ----------------------- additional add on calls Some of these conflict with the Script that makes the Exel workbooks ------
  * OPC=15 when recoded in C if ever,
  * 20 - 29 varies flavors of HCP, but these do not need UserEval, can be done with altcount so long as you don't also call usereveal
  * 30 - 39 various flavors of Dfit and Fn if ever.
  * mixed_KARcalc : Karpin and KARP_B as one set (MixKar aka 50) Karpin and Larsson seem to be the best overall.
  * mixed_LARcalc : Larsson and LAR_B as one set (MixLar aka 51)
  * UE_ALL=40     : BFsuit & BFfitlen + 6 Results from every one of the above 14 Metrics: NT_Side, NTnorth, NTsouth, BestFit: BF_Side, BF_N,BF_S, 
  * set88         : all the metrics in the set88 array set to 1; 2 results per metric: HLDF in BestFit and HL in NT
  * set99         : Debugging values easy to see in GDB or the like to verify Client/Server mmap communications
  * ---- Fut Metric 15=OPC: HCP flavors 3xInts 6xDecNums 1xLTC ----- 
  */
#endif
/* Raw HCP values in suits of length 3 or more. The basis for deductions when stiff or dlbton, or additions when with A or K */
/* Future: Implement these and also std_a, std_t, std_at per Andrews research in a 31 col table for easy lookup of the combos */
/*                           std       Aces C13       BUM-RAP                    KnR          DKP*3        Andrews Fifths  */
float_t hcp_val[][5] = { {4,3,2,1,0}, {6,4,2,1,0}, {4.5,3.0,1.5,0.75,0.25}, {3,2,1,0,0} , {13,9,5,2,0}, {4.,2.8,1.8,1.,0.4 },   
								/* Woolsey                     OPC              BWjgm  a better BumWrap    OPCjgm a better OPC */
                        {4.5,3.0,1.75,0.75,0.0}, {4.5,3.0,1.5,0.5,0} ,  {4.25,3.0,1.75,0.75,0.25}, {4.25,3.0,1.5,0.5,0}
};
float_t stiff_H_adj[][5] = {
   //              A   K   Q   J   T                       A    K   Q   J   T
   /* Bergen */ {-.5, -1, -1, -1, -0.},     /* BISSEL */ { -0, -1, -1, -0, -0. },  /* Stiff J gets 0 bissell pts to begin with */
   /* DKP    */ { -0, -3, -3, -2, -0.},     /* Goren  */ { -0, -2, -2, -1. -0. }, /* Stiff K in NT = 1 */
   /* Kaplan */ { -0, -0, -2, -1, -0.},     /* Karpin */ { -0, -1, -1, -1, -0. }, /* Based on PAV web page JGM added -1 for J */
   /* KARP_B */ { -0, -1,-1,-.75,-.25},     /* KnR    */ { -0,-2.5,-1, -0, -0. },     
   /* LAR    */ { -0, -1, -1,  -1, -0.}, /* based on his book. LAR counts for Len */
   /* LAR_B  */ { -0, -1, -1,-.75,-.25},    /* PAV    */ { -0, -2, -2,  -1, -0.}, /* He says he follows Goren */
   /* SHEINW */ { -0, -1, -1, -1, -0.},     /* per 5 Weeks To Winning Bridge */
   /* ZARBAS */ { -1, -1, -1, -1,  0.},     /* ZARADV */ { -1, -1, -1,  -1,  0.},    /* Stiff A is questionable ?*/
   /* ROTH   */ { -0, -1, -1, -1, -0.},     /* Roth keeps HCP always; he adjusts the Dpts for H, HH, and Hx */
   /* OPC    */ { -1, -1, -1, -0.5, -0},    /* For Doc Only. OPC uses the net after deduction preCalculated */
   // OPC Stiff H values after Deductions (for ref Only): A=3.5, K=2, Q=0.5, J=0, T=0
   /* Unused 16-19 */ {0,0,0,0,0,},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
   // These next values should be in the range of 20 -29.
   /* HCP_T050*/
   /* JGMBW  */ { -0, -1, -1,-.75,-.25}     /* Stiff K=2, Stiff Q=1, Stiff J=0, Stiff T=0 */
} ;

/* dbl_HH_Adj:
 * LAR_B using BumWrap[4.5,3,1.5,.75,.25] for HCP and Ded to reflect H and HH in OPC
 * KARP_B  using BUmWrap with Ded similar to standard Deductions
 * OPC Deductions: (Reference Only OPC not in this code. With 3+ Len Axx=4.5, Kxx=3, Qxx=1.5, Jxx=0.5, KTx=3.4, QJx=3, QTx=2.5, JTx=2 )
 * { AK=-1(6.5), AQ=-1(5.5),AJ=-.5(5.0),  AT=0(4.5),  Ax=0(4.5)., KQ=-.5(4.5),  KJ=0(4.0), KT=0(3.5),  Kx=0.(3.0),
 * QJ=-1(2.0), QT=-1(1.5),Qx=-.5(1.0),JT=-1.5(0.5), Jx=-.5(0.),  Tx=0(0),  xx=0.0}
 */
/*                AK, AQ, AJ, AT, Ax,  KQ, KJ, KT, Kx,  QJ, QT, Qx,  JT, Jx, Tx,   xx */
float_t dbl_HH_adj[][16] = {
   /* Bergen */ { -0, -0,-.5, -0, -0.,-.5,-.5, -0, -0., -1, -1, -1., -1, -1, -0,  -0.0}, /* Book gives -.5*/
   /* Bissell*/ { -0, -0, -0, -0, -0.,  0,  0, -0, -0., -1, -1, -1., -0, -0, -0,  -0.0}, /* Bissel JT(x), Jx(x), Tx = 0 so no ded */
   /* DKP    */ { -0, -0, -0, -0, -0., -1, -0,  0, -0., -3, -3, -3., -2, -2, -0,  -0.0}, /* unguarded H; but only 1/3 of shwn */
   /* Goren  */ { -0, -0, -0, -0, -0., -0, -0, -0, -0., -1, -1, -1., -1, -1, -0,  -0.0}, /* Goren KQ? KJ? */
   /* Kaplan */ { -0, -0, -0, -0, -0., -0, -0, -0, -0., -1, -0, -0., -0, -0, -0,  -0.0}, /* Per the book. 1964*/
   /* Karpin */ { -0, -0, -0, -0, -0., -1, -1, -0, -0., -1, -1, -1., -1, -1, -0,  -0.0}, /* Per PAV web Page.  */
   /* KARP_B */ { -1, -1,-.5, -0, -0., -1, -0, -0, -0., -1, -1, -1., -1,-.75,-.25,-0.0}, /* Ded for BWjgm? 4.25,3,1.75,0.75,0.25 */
   /* KnR    */ { -0, -0, -0, -0, -0., -0, -0, -0, -0., -0, -0, -0., -0, -0, -0,  -0.0}, /* KnR Dont deduct for dblton Honors */
   /* LAR    */ { -0, -0, -0, -0, -0., -1, -0, -0, -0., -1, -0, -0., -0, -0, -0,  -0.0}, /* based on book */
   /* LAR_B  */ { -0, -.5,-.25,-0,-0., -0, -0, -0,-0.,-.25,-.25,-.5,-.5,-.75,-.25,-0.0}, /* reflects opc values for HH dblton*/
   /* PAV    */ { -0, -0, -0, -0, -0., -1, -1, -0, -0., -1, -1, -1., -1, -1, -0,  -0.0}, /* KQ -1? KJ -1? K -2? */
   /* SHEINW */ { -0, -0, -0, -0, -0., -0, -0, -0, -0., -1, -1, -1., -1, -1, -0,  -0.0}, /* KQ? KJ? assuming NO.*/
   /* ZARBAS */ { -0, -0, -1, -0, -0., -1, -1, -0, -0., -1, -1, -1., -1, -1, -0,  -0.0}, /* Based on 2005 PDF */
   /* ZARADV */ { -0, -0, -1, -0, -0., -1, -1, -0, -0., -1, -1, -1., -1, -1, -0,  -0.0}, /* Based on 2005 PDF */
   /* Roth   */ { -0, -0, -0, -0, -0., -1, -1, -0, -0., -1, -1, -1., -1, -1, -0,  -0.0}, /* Per Book. 1968 */
   /* OPC Net   {  6.5,5.5,5, 4.5, 4.5, 4.5, 4, 3.5,3.,  2, 1.5, 1., .5,  0,  0,   0.0},  Ref Only. The net values post deductions. */
} ;
//                  0          1    2    3      4       5       6      7    8    9     10     11      12      13      14      15      16
//enum metric_ek { BERGEN=0,BISSEL,DKP,GOREN, KAPLAN, KARPIN, KARP_B, KnR, LAR, LAR_B, PAV, SHEINW, ZARBAS, ZARADV,  ROTH, metricEND, OPC
//   20         21       22       23       24       25      26
//  HCPT050, HCPA425, HCPAT475, Bumwrap, Woolsey, And5ths, JGMBW  
//   40   50    51     88    -1
// UE_ALL=40, set_40=40, MixKar=50, MixLar=51, SET=88,Quit=-1} ;

/* Dpts_vals is 'cooked' such that hcp + hcp_adj + Dpts_val gives the correct answer for the given metric.
 * does not always come out to +2 for a stiff and +1 for a dblton
 *
 * Roth is the exception. He adjusts HCP only in suits; short honors in NT are not adjusted
*/
enum {GOREN_dpts_idx=0, PAV_dpts_idx, SHEINW_dpts_idx, ROTH_dpts_idx} ;
int Dpts_vals[][22] = {
   //               A K Q J T x  AK     Ax  KQ    Kx  QJ  Qx JT  Tx,xx
   {  /* Goren   */ 2,2,2,2,2,2, 1,1,1,1,1, 1,1,1,1,  1,1,1, 1,1, 1,1 }, // Stiff K = 3 - 2 + 3 = 4
   {  /* Pavlicek*/ 2,2,2,2,2,2, 1,1,1,1,1, 1,1,1,1,  1,1,1, 1,1, 1,1 },
   {  /*Sheinwold*/ 2,2,2,2,2,2, 1,1,1,1,1, 1,1,1,1,  1,1,1, 1,1, 1,1 },
   {  /* Roth    */ 2,2,2,2,2,2, 1,1,1,1,1, 1,1,1,1,  1,1,1, 1,1, 1,1 }, /* These Dpts are demoted to zero if no suppt*/
} ;

float_t lookup_adj_stiff_H(enum metric_ek m, int w1) { /* metric  and top card bitmask */
   float_t adj_pts;
   int adjx100 ;
   switch(w1) {  /* should really use CardAttr_RO[1][ACE] ... etc.  instead of 64 */
      case 64 : adj_pts = stiff_H_adj[m][0]  ; break ;  /* stiff A */
      case 32 : adj_pts = stiff_H_adj[m][1]  ; break ;  /* stiff K */
      case 16 : adj_pts = stiff_H_adj[m][2]  ; break ;  /* stiff Q */
      case 8  : adj_pts = stiff_H_adj[m][3]  ; break ;  /* stiff J */
      case 4  : adj_pts = stiff_H_adj[m][4]  ; break ;  /* stiff T */
      default : adj_pts = 0.0                ; break ;  /* stiff spot */
   } /* end switch(w1) */
   adjx100 = adj_pts * 100. ;
   JGMDPRT(8,"Adj for stiff; weight=%d, Metric=%d StiffAdjx100=%d\n", w1, m, adjx100 );
   return (adj_pts) ;
} /* end lookup adj stiff honor */

float_t lookup_adj_HH(enum metric_ek m, int w2) { /* metric and top two cards bit mask */
   float_t adj_pts;
   int adjx100 ;
   switch(w2) { /* should really use (CardAttr_RO[1][ACE] + CardAttr_RO[1][KING] )  here instead of 96*/
      case 96 : adj_pts = dbl_HH_adj[m][0]  ; break ; // AK 64 + 32
      case 80 : adj_pts = dbl_HH_adj[m][1]  ; break ; // AQ
      case 72 : adj_pts = dbl_HH_adj[m][2]  ; break ; // AJ
      case 68 : adj_pts = dbl_HH_adj[m][3]  ; break ; // AT
      case 65 : adj_pts = dbl_HH_adj[m][4]  ; break ; // Ax 64 + 1
      case 48 : adj_pts = dbl_HH_adj[m][5]  ; break ; // KQ 32 + 16
      case 40 : adj_pts = dbl_HH_adj[m][6]  ; break ; // KJ
      case 36 : adj_pts = dbl_HH_adj[m][7]  ; break ; // KT
      case 33 : adj_pts = dbl_HH_adj[m][8]  ; break ; // Kx
      case 24 : adj_pts = dbl_HH_adj[m][9]  ; break ; // QJ 16 + 8
      case 20 : adj_pts = dbl_HH_adj[m][10] ; break ; // QT
      case 17 : adj_pts = dbl_HH_adj[m][11] ; break ; // Qx
      case 12 : adj_pts = dbl_HH_adj[m][12] ; break ; // JT 8 + 4
      case  9 : adj_pts = dbl_HH_adj[m][13] ; break ; // Jx 8 + 1
      case  5 : adj_pts = dbl_HH_adj[m][14] ; break ; // Tx 4 + 1
      default : adj_pts = 0.0               ; break ; // xx
   } /* end switch(w2) */
   adjx100 = adj_pts * 100.0 ;
   JGMDPRT(8,"AdjShortHonors: metric=%d, HH weight=%d, adjx100=%d \n",m, w2, adjx100 );
   return (adj_pts) ;
} /* end lookup adj Two Honors Dblton */

float_t excp_adj_fhcp(HANDSTAT_k *phs, int suit, int tag ) { /* switch statement goes here */ return 0.0 ; }

/* shortHon_adj handles the cases where the HCP adjustments are in the table. Other cases will need new code */
float_t shortHon_adj (HANDSTAT_k *phs, int suit, int tag ) {
   float_t adj_pts ;   // usually negative.
   int w1, w2 ;

   if (tag >= metricEND ) { adj_pts = excp_adj_fhcp(phs, suit, tag) ; return adj_pts; }
   assert(BERGEN <= tag && tag < metricEND);
/*
 * The Query tags in alpha? order: The adj_hcp arrays use these values to lookup adjustments. 
 /                  0          1    2    3      4       5       6      7    8    9     10     11      12      13      14      15      40    50    51     88    -1
//enum metric_ek { BERGEN=0,BISSEL,DKP,GOREN, KAPLAN, KARPIN, KARP_B, KnR, LAR, LAR_B, PAV, SHEINW, ZARBAS, ZARADV,  ROTH, metricEND,UE_ALL,Mix1, Mix2, SET=88,Quit=-1} ;
// possibly add metrics in the 50 - 79 range to implement different hand factors like quicktricks, or quicklosers, or shortest suit etc.
                    SET=88, SYNTST=99, Quit=-1} ;
*/
   if (phs->hs_length[suit] >= 3 || 0 == phs->hs_length[suit] ) { return 0.0 ; }

   w1 = phs->topcards[suit][0] ;      /* w1 is a bit mask for stiff */
   w2 = phs->topcards[suit][1] + w1 ; /* w2 is a bit mask for dblton */
 
   if (phs->hs_length[suit] == 1 ) { adj_pts = lookup_adj_stiff_H( tag , w1 ) ; } /* end length == 1 */
   else                            { adj_pts = lookup_adj_HH(      tag , w2 ) ; } /* Len must be two here */
   JGMDPRT(6,"shortHonAdj Metric=%d, seat=%c, suit=%c, len=%d, w1=%d, w2=%d adjpts=%g\n",
               tag, "NESW"[phs->hs_seat], "CDHS"[suit], phs->hs_length[suit], w1, w2,adj_pts ) ;

   return (adj_pts);
} /* end shortHon_adj */

/* HCP_Scale: 0=std 1=Aces C13 2=BUM-RAP 3=KnR 4=DKP*3 5=Andrews Fifths 6=Woolsey 7=OPC(needs Syn) 8=BWjgm 9=OPCjgm(no syn)  */
float_t calc_alt_hcp (HANDSTAT_k *phs, int suit, int mtype ) {
   float alt_hcp = 0.0 ; 
   float alt_syn = 0.0 ;
   int r, j ;
   /* Map the metric number to the HCP scale to use */
   /* HCP_Scale: 0=std 1=Aces C13 2=BUM-RAP 3=KnR 4=DKP*3 5=Andrews Fifths 6=Woolsey 7=OPC(needs Syn) 8=BWjgm 9=OPCjgm(no syn)  */
   int hcp_type[] = {0, 0, 4, 0, 0, 0, 8,     3, 0, 8,   0, 0, 0, 0,   0,    0, 0, 0, 0, 0, 0} ;
   /*                     DKP   KAP   KARPB  KNR   LAR_B P  S  Zar    Roth   Future      These are the Metrics that need translation */
   int type = hcp_type[mtype];
   for (r= ACE , j=0 ; r >= TEN ; r--, j++ ) {
      if ( phs->Has_card[suit][r]   ) {
         alt_hcp += hcp_val[type][j] ;
         JGMDPRT(8, "calc_alt_hcp mType=%d, type=%d, suit=%c,  HasCard rank=%d, j=%d, alt_hcp_tot=%g \n",
                                    mtype, type, "CDHS"[suit], r, j, alt_hcp ) ;
      }
      /* For metric 9 aka LAR_B use OPC approach; a protected Q gets full 2 pts, a Jack a full 1 pt. 
       *     give Q (1.5)  an extra 0.5 if with K or A in a 3+ suit
       *     give J (0.75) an extra 0.25 if with Q or T in a 3+ suit
       *     give T (0.25) an extra 0.25 if with J or 9 in a 3+ suit
       */
   }
   if (phs->hs_length[suit] >= 3 && mtype == 9 ) {
          if (phs->Has_card[suit][QUEEN] && ( phs->Has_card[suit][ACE]   || phs->Has_card[suit][KING]) ) {alt_syn += 0.5 ; }
          if (phs->Has_card[suit][JACK]  && ( phs->Has_card[suit][QUEEN] || phs->Has_card[suit][TEN] ) ) {alt_syn += 0.25; }
          if (phs->Has_card[suit][TEN]   && ( phs->Has_card[suit][JACK]  || phs->Has_card[suit][NINE]) ) {alt_syn += 0.25; }
      }
      alt_hcp += alt_syn;
      JGMDPRT(9,"Lar_B_AltSyn Chk pts(Q,J,T) for mtype[%d] = %g New Suit Total=%g\n",mtype,alt_syn, alt_hcp );
   return alt_hcp ;
/* What code is doing in above loop
   if ( phs->Has_card[suit][ACE]   ) { alt_hcp += hcp_val[type][0] ; }
   if ( phs->Has_card[suit][KING]  ) { alt_hcp += hcp_val[type][1] ; }
   if ( phs->Has_card[suit][QUEEN] ) { alt_hcp += hcp_val[type][2] ; }
   if ( phs->Has_card[suit][JACK]  ) { alt_hcp += hcp_val[type][3] ; }
   if ( phs->Has_card[suit][TEN]   ) { alt_hcp += hcp_val[type][4] ; }
*/
}

/*
 * Short suit evaluation routines. For stiff Honors, and Honors dblton. Each metric is slightly different.
 * and Short suit methods such as PAV, Goren, and Sheinwold, have different valuations in suit and NT.
 */
/* given the weight of the short suit(s) return the index into the ss_Values array */
/* The indexes returned 0 .. 21 could be replaced by the ss_xx names from the ss_enum_ek definition */
int ss_index(int weight) {
         switch ( weight  ) {                  /* Weight is for top TWO slots only, NOT top 3*/
         case 192 : return 0  ; // A(64) + Void(128) ie. stiff Ace
         case 160 : return 1  ; // K(32) + Void(128) ie. stiff King
         case 144 : return 2  ; // Q(16) + Void(128) ie. stiff Queen
         case 136 : return 3  ; // J(8)  + Void(128) ie. stiff Jack
         case 132 : return 4  ; // T(4)  + Void(128) ie. stiff T
         case 129 : return 5  ; // x(1)  + Void(128) ie. stiff x
         case  96 : return 6  ; /* AK */
         case  80 : return 7  ; /* AQ */
         case  72 : return 8  ; /* AJ */
         case  68 : return 9  ; /* AT */
         case  65 : return 10 ; /* Ax */
         case  48 : return 11 ; /* KQ */
         case  40 : return 12 ; /* KJ */
         case  36 : return 13 ; /* KT */
         case  33 : return 14 ; /* Kx */
         case  24 : return 15 ; /* QJ */
         case  20 : return 16 ; /* QT */
         case  17 : return 17 ; /* Qx */
         case  12 : return 18 ; /* JT */
         case   9 : return 19 ; /* Jx */
         case   5 : return 20 ; /* Tx */
         case   2 : return 21 ; /* xx */
         /* Now stiffs with no void included in the weight alternate mode of access using weight of top slot only. */
         case 64 : return 0 ;  /* Stiff A */
         case 32 : return 1 ;  /* Stiff K */
         case 16 : return 2 ;  /* Stiff Q */
         case  8 : return 3 ;  /* stiff J */
         case  4 : return 4 ;  /* stiff T */
         case  1 : return 5 ;  /* stiff x */
         default : fprintf(stderr, "Cant happen. Invalid weight in ss_index. %s:%d \n",__FILE__, __LINE__ );
                   assert(0) ;
         } /* end switch */
         /* NOT REACHED */
   return -1 ;
} /* end ss_index */

