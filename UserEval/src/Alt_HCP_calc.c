/* File AltHCP_calc.c */
/* Date        Version  Author   Description
 * 2024/09/12  1.0      JGM      Implement all possible HCP scales; may finally get to use alternate syntax for usereval
 *
 */
#define GNU_SOURCE
#include "../include/std_hdrs_all.h"
#include "../include/UserEval_types.h"
#include "../include/UserEval_externs.h"
#include "../include/dbgprt_macros.h"
#include "../include/mmap_template.h"
#define ALTHCP_OFFSET 20   /* the metrics begin at 20, but the tables begin at zero */
typedef float Hsyn_pts(int m , int s, HANDSTAT_k *p_hs, int mask ) ;
typedef Hsyn_pts *pHsyn_pts ;
// pHsyn_pts pHsyn_tbl[] = {NULL, NULL, NULL, NULL, NULL, NULL, SynBWjgm, SynOPCjgm } ;

char curr_m_name[32] ;

/* External functions -- metrics_util_subs.c , short_honors_subs.c */
extern void zero_globals( int side ) ; 
// extern float_t calc_alt_hcp( HANDSTAT_k *p_hs, int suit, int mtag ) ; /* this is for DKP, KnR, Bissel, not for Bumwrap etc. */
extern float_t shortHon_adj( HANDSTAT_k *p_hs, int suit, int mtag ) ;
extern void SaveUserVals(struct UserEvals_st UEv , USER_VALUES_k *p_ures ) ;
extern void set_dbg_names(int m, char *dbg_func) ;
extern int Pav_round(float val, int body ) ;
extern int Pav_body_val( HANDSTAT_k  *p_hs ) ;
 
 /* Forward Function Definitions */
float OPCjgmSyn (int m , int s, HANDSTAT_k *p_hs, int mask ) ;
void set_curr_m_name( int metric ) ; 

char alt_HCP_names[][16] = {"HCPT050","HCPA425","HCPAT475","Bumwrap","Woolsey","And5ths","BWjgm","OPCjgm" } ;
/* the value of the stiff honor in NT; not counting any extra Dpts; -1 for stiff K, Q, Stiff J,T worth 0 */
float_t alt_stiff_H_pts[][5] = {
    /*              A     K     Q     J     T                          A     K     Q     J     T */
   /* HCPT050 */ { 4.00, 2.00, 1.00, 0.00, 0.00},     /* HCPA425 */ { 4.25, 2.00, 1.00, 0.00, 0.00},    
   /* HCPAT475*/ { 4.50, 2.00, 1.00, 0.00, 0.00},     /* Bumwrap */ { 4.50, 2.00, 0.50, 0.00, 0.00},  
   /* Woolsey */ { 4.50, 2.00, 0.75, 0.00, 0.00},     /* And5ths */ { 4.00, 1.80, 0.80, 0.00, 0.00}, 
   /* BWjgm   */ { 4.25, 2.00, 0.75, 0.00, 0.00},     /* OPCjgm  */ { 4.25, 2.00, 0.50, 0.00, 0.00},
} ;
   /*
    * There is little consistency among experts re deducting for doubleton Honors.
    * Those that use shortness based metrics tend to deduct more. Those that use Length based metrics deduct fewer
    * In most cases where there is no published data, JGM is just guessing at reasonable values
    * In particular in the cases where the Honors (incl Ten's) have a non-standard value.
    * QJ tight is better than QT tight; but is QT tight better than Qx?
    * Those that do deduct never work in fractions so one can assume that their choice was either to deduct 1pt or deduct nothing.
    * In JGM's case he is willing to work in fractions so he can give QJ=3 - 1, QT=2.5 - 1 and Qx= 2 - 1; thus giving the T some value.
    * But JT and Jx tight are likely equally worthless.
    */

   /*               AK,   AQ,   AJ,   AT,   Ax,   KQ,   KJ,   KT,   Kx,   QJ,   QT,   Qx,   JT,   Jx,   Tx,  xx */
   // Roth & Pavlicek     ......................   -1,  -1,                -1,  -1,   -1.,   -1,   -1,   
   // Karpin                                       -1,  -1,                -1,  -1,   -1.,   -1,   -1,
   // Kaplan                                                               -1
   // Larsson .................................... -1                      -1
   // Sheinwold                                                            -1,  -1,   -1.,   -1,   -1,
   float_t alt_dbl_HH_pts[][16] = { /* the net value of the Dblton honor(s) in NT; not counting any extra Dpts */
   /*               AK,   AQ,   AJ,   AT,   Ax,   KQ,   KJ,   KT,   Kx,   QJ,   QT,   Qx,   JT,   Jx,   Tx,  xx */
   /* HCPT050 */ { 7.00, 6.00, 5.00, 4.50, 4.00, 4.00, 3.25, 3.00, 3.00, 2.00, 1.50, 1.00, 0.00, 0.00, 0.00, 0.00}, /* JGM guesses */
   /* HCPA425 */ { 7.25, 6.25, 5.25, 4.25, 4.25, 4.00, 3.25, 3.00, 3.00, 2.00, 1.00, 1.00, 0.00, 0.00, 0.00, 0.00},
   /* HCPAT475*/ { 7.25, 6.25, 5.25, 4.75, 4.25, 4.00, 3.25, 3.00, 3.00, 2.00, 1.50, 1.00, 0.00, 0.00, 0.00, 0.00},
   /* Bumwrap */ { 7.50, 6.00, 5.25, 4.75, 4.50, 3.50, 3.00, 3.00, 3.00, 1.25, 1.00, 0.75, 0.00, 0.00, 0.00, 0.00},
   /* Woolsey */ { 7.50, 6.25, 5.25, 4.50, 4.50, 3.75, 3.00, 3.00, 3.00, 1.50, 1.00, 0.75, 0.00, 0.00, 0.00, 0.00},
   /* And5ths */ { 6.80, 5.80, 5.00, 4.40, 4.00, 3.60, 3.20, 2.80, 2.80, 1.80, 1.20, 0.80, 0.00, 0.00, 0.00, 0.00},
   /* BWjgm   */ { 7.25, 6.00, 5.00, 4.50, 4.25, 3.75, 3.25, 3.00, 3.00, 1.50, 1.00, 0.75, 0.00, 0.00, 0.00, 0.00},
   /* OPCjgm  */ { 6.25, 5.25, 4.75, 4.25, 4.25, 4.50, 3.25, 3.00, 3.00, 2.00, 1.50, 1.00, 0.50, 0.00, 0.00, 0.00}, 
} ;
float_t alt_Hxx_pts[][5] = {  /* starting HCP value in 3+ suit. */
   //               A     K     Q     J     T                          A     K     Q     J     T
   /* HCPT050 */ { 4.00, 3.00, 2.00, 1.00, 0.50},     /* HCPA425 */ { 4.25, 3.00, 2.00, 1.00, 0.00},   
   /* HCPAT475*/ { 4.25, 3.00, 2.00, 1.00, 0.50},     /* Bumwrap */ { 4.50, 3.00, 1.50, 0.75, 0.25},  
   /* Woolsey */ { 4.50, 3.00, 1.75, 0.75, 0.00},     /* And5ths */ { 4.00, 2.80, 1.80, 1.00, 0.40}, 
   /* BWjgm   */ { 4.25, 3.00, 1.75, 0.75, 0.25},     /* OPCjgm  */ { 4.25, 3.00, 1.50, 0.50, 0.00}, /* Q, J, T with companions get extra */
} ;
void set_curr_m_name( int metric )  {
   strncpy(curr_m_name, alt_HCP_names[metric - ALTHCP_OFFSET], sizeof(curr_m_name -1 ) );
}

int crd_rank(int suit, int hi_rk, int lo_rk,  HANDSTAT_k *p_hs) {
   for (int r=hi_rk ; r >= lo_rk ; r-- ) {
      if (r == p_hs->Has_card[suit][r] ) return r ;
   }
   return -1 ; /* -1 means card rank is not between hi_rk and lo_rk */
}
float Stiff_Hon_pts(int metric, int suit, HANDSTAT_k *p_hs ) {    // called when suit length == 1
                           // stiff_table_pts[0 .. 4] = ACE, KING, QUEEN, JACK, TEN
   float sHp ; 
   int m = metric - ALTHCP_OFFSET ;   // 20 #define as start of Alt Honors Metrics so HCPT050 = 20, And5ths = 25 etc.
   int w_mask = p_hs->topcards[suit][0] ;   // bit mask for the highest (only here) card in suit.
   switch (w_mask) {
      case  64 : sHp = alt_stiff_H_pts[m][0] ; break ; /* stiff A */
      case  32 : sHp = alt_stiff_H_pts[m][1] ; break ; /* stiff K */
      case  16 : sHp = alt_stiff_H_pts[m][2] ; break ; /* stiff Q */
      case   8 : sHp = alt_stiff_H_pts[m][3] ; break ; /* stiff J */
      case   4 : sHp = alt_stiff_H_pts[m][4] ; break ; /* stiff T */
      default : sHp = 0.0; break;
   }
   JGMDPRT(8,"Stiff Hon Pts m=%d, metric=%d, w_mask=%d, sHp=%g\n",m,metric,w_mask,sHp);
   return sHp ;
} /* end stiff H pts */

float Dblton_Hon_pts(int metric, int suit, HANDSTAT_k *p_hs ) {    // called when suit length == 1
   int m = metric - ALTHCP_OFFSET ; // 20 #define as start of Alt Honors Metrics so HCPT050 = 20, And5ths = 25 etc.
   int w_mask = p_hs->topcards[suit][0] + p_hs->topcards[suit][1] ;    // bit mask for top 2 cards in suit.
   float dblton_H_pts ;
   switch (w_mask) {
      case 96 : dblton_H_pts = alt_dbl_HH_pts[m][0]  ; break ; // AK 64 + 32
      case 80 : dblton_H_pts = alt_dbl_HH_pts[m][1]  ; break ; // AQ
      case 72 : dblton_H_pts = alt_dbl_HH_pts[m][2]  ; break ; // AJ
      case 68 : dblton_H_pts = alt_dbl_HH_pts[m][3]  ; break ; // AT
      case 65 : dblton_H_pts = alt_dbl_HH_pts[m][4]  ; break ; // Ax 64 + 1
      case 48 : dblton_H_pts = alt_dbl_HH_pts[m][5]  ; break ; // KQ 32 + 16
      case 40 : dblton_H_pts = alt_dbl_HH_pts[m][6]  ; break ; // KJ
      case 36 : dblton_H_pts = alt_dbl_HH_pts[m][7]  ; break ; // KT
      case 33 : dblton_H_pts = alt_dbl_HH_pts[m][8]  ; break ; // Kx
      case 24 : dblton_H_pts = alt_dbl_HH_pts[m][9]  ; break ; // QJ 16 + 8
      case 20 : dblton_H_pts = alt_dbl_HH_pts[m][10] ; break ; // QT
      case 17 : dblton_H_pts = alt_dbl_HH_pts[m][11] ; break ; // Qx
      case 12 : dblton_H_pts = alt_dbl_HH_pts[m][12] ; break ; // JT 8 + 4
      case  9 : dblton_H_pts = alt_dbl_HH_pts[m][13] ; break ; // Jx 8 + 1
      case  5 : dblton_H_pts = alt_dbl_HH_pts[m][14] ; break ; // Tx 4 + 1
      default : dblton_H_pts = 0.0                   ; break ; // xx
   } /* end switch(w_mask) */
   JGMDPRT(8,"Dblton_H Pts m=%d, metric=%d, w_mask=%d, HH_Pts=%g\n",m,metric,w_mask,dblton_H_pts);  
   return dblton_H_pts;
} /* end dblton_HH_pts */

float Long_Hxx_pts( int metric, int suit, HANDSTAT_k *p_hs ) {
   int m = metric - ALTHCP_OFFSET ; // 20 #define as start of Alt Honors Metrics so HCPT050 = 20, And5ths = 25 etc.
   int w_mask = p_hs->topcards[suit][0] + p_hs->topcards[suit][1] + p_hs->topcards[suit][2] ;   // bit mask for top 3 cards in suit.
   float lHp, lHp2  ;
   lHp2 = 0.0;
   JGMDPRT(8,"Long_Hxx_Pts m=%d, metric=%d, w_mask=%d\n",m,metric,w_mask);
   switch (w_mask) {
      case 112 :  // AKQ 64 + 32 + 16
               lHp = alt_Hxx_pts[m][0] + alt_Hxx_pts[m][1] + alt_Hxx_pts[m][2] ;       
               if (OPCjgm == metric ) {lHp2 = OPCjgmSyn(metric, suit, p_hs, w_mask) ; }
               else {
                   if(p_hs->Has_card[suit][JACK] ) lHp2  = alt_Hxx_pts[m][3] ;
                   if(p_hs->Has_card[suit][TEN]  ) lHp2 += alt_Hxx_pts[m][4] ;
               }
               lHp += lHp2 ;
                                                                        break ;
      case 104 :  // AKJ
               lHp = alt_Hxx_pts[m][0] + alt_Hxx_pts[m][1] + alt_Hxx_pts[m][3] ;
               if(OPCjgm == metric ) {lHp2 = OPCjgmSyn(metric, suit, p_hs, w_mask) ; }
               else {
                   if(p_hs->Has_card[suit][TEN]  ) lHp2 = alt_Hxx_pts[m][4] ;
               }
               lHp += lHp2 ; 
                                                                        break ;
      case 100 :  // AKT
               lHp = alt_Hxx_pts[m][0] + alt_Hxx_pts[m][1] + alt_Hxx_pts[m][4];
               if(OPCjgm == metric ) {
                  lHp2 = OPCjgmSyn(metric, suit, p_hs, w_mask) ;
                  lHp += lHp2 ;
                }
                                                                        break ;
      case  97 : // AKx  64 + 32 + 1 -- there is no 2
               lHp = alt_Hxx_pts[m][0] + alt_Hxx_pts[m][1]   ;
                                                                        break ; 
      case  88 : // AQJ  64 + 16 + 8
               lHp = alt_Hxx_pts[m][0] + alt_Hxx_pts[m][2] + alt_Hxx_pts[m][3]  ;
               if(OPCjgm == metric ) {lHp2 = OPCjgmSyn(metric, suit, p_hs, w_mask) ; }
               lHp += lHp2 ; 
                                                                        break ; 
      case  84 : // AQT
               lHp = alt_Hxx_pts[m][0] + alt_Hxx_pts[m][2] + alt_Hxx_pts[m][4]  ;
               if(OPCjgm == metric ) {lHp2 = OPCjgmSyn(metric, suit, p_hs, w_mask) ; }
               lHp += lHp2 ;
                                                                        break ; 
      case  81 : // AQx
               lHp = alt_Hxx_pts[m][0] + alt_Hxx_pts[m][2] ;
               if(OPCjgm == metric ) {lHp2 = OPCjgmSyn(metric, suit, p_hs, w_mask) ; }
               lHp += lHp2 ; 
                                                                        break ;
      case  76 : // AJT  64 + 8 + 4
               lHp = alt_Hxx_pts[m][0] + alt_Hxx_pts[m][3] + alt_Hxx_pts[m][4]     ;
               if(OPCjgm == metric ) {lHp2 = OPCjgmSyn(metric, suit, p_hs, w_mask) ; }
               lHp += lHp2 ;                                            break ; 
      case  73 :  // AJx
               lHp = alt_Hxx_pts[m][0] + alt_Hxx_pts[m][3] ;
               if(OPCjgm == metric ) {lHp2 = OPCjgmSyn(metric, suit, p_hs, w_mask) ; }
               lHp += lHp2 ; 
                                                                        break ;     
      case  69 : // ATx   64 + 4 + 1  
               lHp = alt_Hxx_pts[m][0] + alt_Hxx_pts[m][4];             break ;     
      case  66 : // Axx 64 + 1 + 1
               lHp = alt_Hxx_pts[m][0] ;                                break ;
      case  56 : // KQJ 32 + 16 + 8
               lHp = alt_Hxx_pts[m][1] + alt_Hxx_pts[m][2] + alt_Hxx_pts[m][3]   ;
               if(OPCjgm == metric ) {lHp2 = OPCjgmSyn(metric, suit, p_hs, w_mask) ; }
               lHp += lHp2;                                             break ; 
      case  52 : // KQT
               lHp = alt_Hxx_pts[m][1] + alt_Hxx_pts[m][2] + alt_Hxx_pts[m][4]   ;
               if(OPCjgm == metric ) {lHp2 = OPCjgmSyn(metric, suit, p_hs, w_mask) ; }
               lHp += lHp2;                                             break ; 
      case  49 : // KQx
               lHp = alt_Hxx_pts[m][1] + alt_Hxx_pts[m][2]  ;
               if(OPCjgm == metric ) {lHp2 = OPCjgmSyn(metric, suit, p_hs, w_mask) ; }
               lHp += lHp2 ; 
                                                                        break ;
      case  44 : // KJT 32 + 8 + 4
               lHp = alt_Hxx_pts[m][1] + alt_Hxx_pts[m][3] + alt_Hxx_pts[m][4]   ;
               if(OPCjgm == metric ) {lHp2 = OPCjgmSyn(metric, suit, p_hs, w_mask) ; }
               lHp += lHp2;                                             break ; 
      case  41 : // KJx
               lHp = alt_Hxx_pts[m][1] + alt_Hxx_pts[m][3] ;
               if(OPCjgm == metric ) {lHp2 = OPCjgmSyn(metric, suit, p_hs, w_mask) ; }
               lHp += lHp2 ; 
                                                                        break ;               
      case  37 : // KTx
               lHp = alt_Hxx_pts[m][1] + alt_Hxx_pts[m][4] ;
               if(OPCjgm == metric ) {lHp2 = OPCjgmSyn(metric, suit, p_hs, w_mask) ; }
               lHp += lHp2 ; 
                                                                        break ;  
      case  34 : // Kxx 32 + 1 + 1
               lHp = alt_Hxx_pts[m][1];                                 break ;
      case  28 : // QJT 16 + 8 + 4
               lHp = alt_Hxx_pts[m][2] + alt_Hxx_pts[m][3] + alt_Hxx_pts[m][4]; 
               if(OPCjgm == metric ) {lHp2 = OPCjgmSyn(metric, suit, p_hs, w_mask) ; }
               lHp += lHp2;                                             break ;                 
      case  25 : // QJx
               lHp = alt_Hxx_pts[m][2] + alt_Hxx_pts[m][3] ;
               if(OPCjgm == metric ) {lHp2 = OPCjgmSyn(metric, suit, p_hs, w_mask) ; }
               lHp += lHp2 ; 
                                                                        break ;
      case  21 : // QTx
               lHp = alt_Hxx_pts[m][2] + alt_Hxx_pts[m][4] ;
               if(OPCjgm == metric ) {lHp2 = OPCjgmSyn(metric, suit, p_hs, w_mask) ; }
               lHp += lHp2 ; 
                                                                        break ;                 
      case  18 : // Qxx 
               lHp = alt_Hxx_pts[m][2];                                 break ;
      case  13 : // JTx 8 + 4 + 1
               lHp = alt_Hxx_pts[m][3] + alt_Hxx_pts[m][4] ;
               if(OPCjgm == metric ) {lHp2 = OPCjgmSyn(metric, suit, p_hs, w_mask) ; }
               lHp += lHp2 ; 
                                                                        break ;            
      case  10 : // Jxx 8 + 1 + 1
               lHp = alt_Hxx_pts[m][3];                                 break ;  
      case   6 : // Txx 4 + 1 + 1
               lHp = alt_Hxx_pts[m][4];                                 break ; 
      case   3 : // xxx x + 1 + 1
                 lHp = 0.0;                                             break ; 
      default  : lHp = 0.0;                                             break ; 
   } /* end switch(w_mask) */    
   return lHp ;
} /* end Long_Hxx_pts */

  /* This next routine is called if the metric is OPCjgm AND the Top 3 cards contain at least one of Q | J | T  */
  /* The 'unguarded' value of the honors (Q=1.5, J=0.5, T=0.0) has already been counted. This routine adds the upgrades. */
float OPCjgmSyn (int m , int s, HANDSTAT_k *p_hs, int mask ) {  // DOP seems too aggressive overall. OPCjgm less so
   // these are not syn_pts as in suit quality(3 of top 5 etc); more like joint_honor points since upgrade depends mostly on only 1 card.
   float syn_pts = 0.0 ;
   JGMDPRT(8,"OPCjgmSyn m=%d, mask=%d, p_hs->seat=%d, Suit=%c\n",m, mask,p_hs->hs_seat,"CDHS"[s]);  
   switch (mask) {
  /*AKQ*/    case 112: syn_pts += 0.5 ;                                          // upgrade Q +.5 to 2
                        if(p_hs->Has_card[s][JACK] ) {
                           syn_pts += 0.5 ;                                      // Guarded J = 1.0 ; J started with 0.5
                           if(p_hs->Has_card[s][TEN]  ) syn_pts += 1.0 ;         // TEN with JACK = 1.0  T started with 0.0
                        }                                                        // dont count 9 if already counting AKQJT
                        else if(p_hs->Has_card[s][TEN]  ) {syn_pts += 0.75 ; }   // T with Q = 1.0 in DOP; in OPCjgm = 0.75 
                        break ;
   /*AKJ*/    case 104: syn_pts += 0.5;                                          // upgraded Jack from 0.5 to 1
                        if(p_hs->Has_card[s][TEN]  ) {
                           syn_pts += 1.0 ;                                      // T with J = 1.0 
                           if( p_hs->Has_card[s][NINE] ) syn_pts += 0.5 ;        // 9 with J and T and one higher honor = .5 in OPCjgm, +1 in DOP 
                        }
                        break ;
   /*AKT*/    case 100: syn_pts += 0.25;   break;                                // Ten with K worth 0.25 in OPCjgm, 0.5 in DOP
   /*AQJ*/    case  88: syn_pts += 1.0;                                          // upgrade Q +.5 to 2, J +.5 to 1.0 
                        if(p_hs->Has_card[s][TEN]  ) {
                           syn_pts += 1.0 ;                                      // T with J = 1.0 
                           if( p_hs->Has_card[s][NINE] ) syn_pts += 0.5 ;        // 9 with J and T and one higher honor = .5 in OPCjgm, +1 in DOP */
                        }
                        break ;
   /*AQT*/    case  84: syn_pts += 1.25;   break;                                 // Upgrade Q +.5 to 2.0, Ten with Q worth 0.75 in OPCjgm, 1.0 in DOP
   /*AQx*/    case  81: syn_pts += 0.5;   break;                                 // Upgrade Q +.5 to 2.0                         
   /*AJT*/    case  76:
                     syn_pts += 1.5;                                             // upgrade J +.5 to 1.0 and T = 1.0 
                     if( p_hs->Has_card[s][NINE] ) syn_pts += 0.5 ;              // 9 with J and T and one higher honor = .5 in OPCjgm, +1 in DOP */
                     break;
   /*AJx*/    case  73: syn_pts += 0.5;   break;                                 // Upgrade J to 1.0              
   /*KQJ*/    case  56:
                      syn_pts += 1.0 ;                                           // upgrade Q +.5 to 2, J +.5 to 1.0 
                      if(p_hs->Has_card[s][TEN]  ) {
                           syn_pts += 1.0 ;                                      // T with J = 1.0 
                           if( p_hs->Has_card[s][NINE] ) syn_pts += 0.5 ;        // 9 with J and T and one higher honor = .5 in OPCjgm, +1 in DOP 
                      }
                     break ;
   /*KQT*/    case  52: syn_pts += 1.25;  break;                                 // Upgrade Q +.5 to 2.0, Ten with Q worth 0.75 in OPCjgm, 1.0 in DOP
   /*KQx*/    case  49: syn_pts += 0.5;   break;                                 // Upgrade Q +.5 to 2.0
   /*KJT*/    case  44:
                     syn_pts += 1.5;                                             // upgrade J +.5 to 1.0 and T = 1.0 
                     if( p_hs->Has_card[s][NINE] ) syn_pts += 0.5 ;              // 9 with J and T and one higher honor = .5 in OPCjgm, +1 in DOP 
                     break;
   /*KJx*/    case  41: syn_pts += 0.5;   break;                                 // Upgrade J to 1.0                          
   /*KTx*/    case  37: syn_pts += 0.25;  break;                                 // T with K worth 0.25 in OPCjgm, 0.5 in DOP
   /*QJT*/    case  28:
                     syn_pts += 2.0;                                             // upgrade Q +.5 to 2, J +.5 to 1.0 and T = 1.0 */
                     if( p_hs->Has_card[s][NINE] ) syn_pts += 0.5 ;              // 9 with J and T and one higher honor = .5 in OPCjgm, +1 in DOP 
                     break;                                       
   /*QJx*/    case  25: syn_pts += 1.0;   break;                                 // upgrade Q +.5 to 2, J +.5 to 1.0
   /*QTx*/    case  21: syn_pts += 0.75;  break;                                 // upgrade T to 0.75 (opc T=1.0, no upgrade Q)
   /*JTx*/    case  13: syn_pts += 1.5;   break;                                 // upgrade J +.5 to 1,T = 1.0   
              default : syn_pts = 0.0 ;   break;
   } /* end switch mask */
   JGMDPRT(7,"OPCjgmSyn Returns syn_pts=%g TEN=%d,JACK=%d,QUEEN=%d,KING=%d,ACE=%d\n",syn_pts,
         p_hs->Has_card[s][TEN],p_hs->Has_card[s][JACK],p_hs->Has_card[s][QUEEN],p_hs->Has_card[s][KING],p_hs->Has_card[s][ACE]);
   return syn_pts ; 
} /* end OPCjgmSyn */

int alt_HCP_calc( UE_SIDESTAT_k *p_ss, int metric ) { /* calculate the HCP for the two seats and the side, using alternate methods and scales */
   int suit, slen ;
   int h    ;
   float suit_alt_hcp[2][4] = { {0.0, 0.0, 0.0, 0.0}, {0.0, 0.0, 0.0, 0.0} } ;
   float hand_alt_hcp[2]    = {0.0, 0.0};
   float side_alt_hcp       = 0.0;
   int hand_alt_ihcp[2], suit_alt_ihcp[2][4] ;  // For the Rounded values of the results.
   HANDSTAT_k *p_hs ; 
   JGMDPRT(6,"Alt_HCP_calc for metric= %d\n",metric ) ;
   for (h=0; h<2 ; h++ ) {
      for (suit=0 ; suit<4 ; suit++ ) {
         p_hs = p_ss->phs[h] ;
         slen = p_hs->hs_length[suit] ;
         JGMDPRT(8,"Hand=%c, Suit=%c, slen=%d\n",compass[h],"CDHS"[suit],slen ) ;
         if ( 3 <= slen )      { suit_alt_hcp[h][suit] = Long_Hxx_pts(  metric, suit, p_hs) ; }
         else if ( 2 == slen ) { suit_alt_hcp[h][suit] = Dblton_Hon_pts(metric, suit, p_hs) ; } 
         else if ( 1 == slen ) { suit_alt_hcp[h][suit] = Stiff_Hon_pts( metric, suit, p_hs) ; }
         else                  { suit_alt_hcp[h][suit] = 0.0 ; }
         hand_alt_hcp[h] += suit_alt_hcp[h][suit] ;
         suit_alt_ihcp[h][suit] = roundf(suit_alt_hcp[h][suit] * 100.0 );  /* convert a float like 7.25 to int 725 */
         JGMDPRT(8,"Suit Results  Float=%g, intx100=%d \n",suit_alt_hcp[h][suit], suit_alt_ihcp[h][suit] );
      } /* end for suit */
      side_alt_hcp += hand_alt_hcp[h] ;
      hand_alt_ihcp[h]   = roundf(hand_alt_hcp[h] * 100.0 );  /* convert a float like 18.25 to int 1825; roundf needed for andrews */
      UEv.nt_pts_seat[h] = Pav_round(hand_alt_hcp[h], p_ss->pav_body[h] )  ;  // rounded pts
      JGMDPRT(7, "Hand Results for %c  Float=%g, intx100=%d  Round=%d \n",compass[h],hand_alt_hcp[h], hand_alt_ihcp[h],UEv.nt_pts_seat[h] );   
   } /* end for hand */
   UEv.nt_pts_side = UEv.nt_pts_seat[0] + UEv.nt_pts_seat[1] ; /* No HLDF pts for strict HCP */
   UEv.hldf_pts_seat[0] = hand_alt_ihcp[0] ; /* so use the fields for something else */ 
   UEv.hldf_pts_seat[1] = hand_alt_ihcp[1] ; /* convert a float like 27.25 to 2775 int */
   UEv.hldf_pts_side    = roundf(side_alt_hcp * 100.0 ) ;
   UEv.misc_count = 0 ;
   UEv.misc_pts[UEv.misc_count++] =  p_ss->pav_body[0] ;
   UEv.misc_pts[UEv.misc_count++] =  p_ss->pav_body[1] ;
   for (h=0; h<2; h++ ) {
      for (suit = 0 ; suit < 4 ; suit++ ) {
         UEv.misc_pts[UEv.misc_count++] = suit_alt_ihcp[h][suit];
      }
   }
     /* now put the results into the user result area at p_uservals */
  SaveUserVals( UEv , p_uservals ) ;
  p_uservals->u.res[126] =  p_ss->pav_body[0] ;  // put the Pav Rounding Value in a standard place.
  p_uservals->u.res[127] =  p_ss->pav_body[1] ;
  JGMDPRT(6, "alt_HCP_calc Done; metric=%d, Rounded=[%d, %d + %d ], Raw = [%d, %d + %d]\n", metric, 
        UEv.nt_pts_side, UEv.nt_pts_seat[0],  UEv.nt_pts_seat[1], UEv.hldf_pts_side, UEv.hldf_pts_seat[0],  UEv.hldf_pts_seat[1]);
  return ( 6 + UEv.misc_count ) ;
 // return the number of results in the UEv struct for copy over to mmap area.
} /* end alt_HCP_calc */
   
   
