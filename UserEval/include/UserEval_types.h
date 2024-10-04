/* File UserEval_types.h  -- Types and Typedefs
 *  Version  Date          Comment
 *    0.5    2022-12-02    First
 *    1.0    2023-03-24    Production
 *    1.1    2023-03-27    Replaced 'CPU' with 'BISSEL' (bissell)
 *    1.2    2023-?? ??    Coded Goren
 *    1.5    2023-05-24    Coded all metrics including ZarBasic and ZarAdvanced. Dropped Rule22.
 *    1.6    2024-01-20    Added Roth metric;
 *    2.0    2024-08-02    Changed names in struct TRUMP_FIT_k and in TRUMP_SUIT_k for consistency
 */
#ifndef USEREVAL_TYPES_H
#define USEREVAL_TYPES_H
#ifndef MMAP_TEMPLATE_H
   #include "../include/mmap_template.h"        /* Will also include dealtypes.h */
#endif
/* may need to invent another set of these, for custom mods.
 * cant insert stuff in the middle now because New Features in DealerV2 and Pavlicek's Library of solved Deals
 * adj_short_honors table is tied to these. New ones may use CPU, GOREN etc. Not in alpha order
 * The Query tags in sort of alpha order: The adj_hcp arrays use these values to lookup adjustments.
 *                     0        1       2     3       4      5       6      7     8    9     */
enum metric_ek    { BERGEN=0, BISSEL,  DKP, GOREN,  KAPLAN, KARPIN, KARP_B, KnR, LAR, LAR_B, 
                // 10     11       12      13      14       15 */
                   PAV, SHEINW,  ZARBAS, ZARADV, ROTH, // OPC ::Recode from Perl to C someday and add it here.

                   metricEND, // == 15 currently 
                   
               //  Added 2024-09-14  Metrics 20 - 29 for various HCP approaches. No Dpts, or DfPts etc. for now.
                   HCPT050=20, HCPA425, HCPAT475, Bumwrap, Woolsey, And5ths, BWjgm, OPCjgm,
               //  Future: 30 - 39 Using some of the above with Dfit, Lpts, HLDF etc.
               
               //  Added 2024-08-01 SET_40 Returns 8 values for each of 15 metrics (120 total)
                   ALL_UE = 40 , SET_40=40,
               //     50          51  possibly more metrics for things like Quick Tricks etc.
                   MixKARPIN=50, MixLARSSON=51, /* MixKARPIN = KARPIN and KARP_B, MixLARSSON=LAR and LAR_B for easy comparaison */
               // Set88 returns two values for each metric: the value of the side if played in best fit, and if played in NT.
                   SET=88, SET_88=88, TSTALL=99, Quit=-1} ;
                   
enum card_rank_ek { Two_rk=0, Three_rk, Four_rk, Five_rk, Six_rk, Seven_rk, Eight_rk, Nine_rk, Ten_rk, Jack_rk, Queen_rk, King_rk, Ace_rk,spot_rk=-1 };
enum pt_count_ek  { Tens=0, Jacks, Queens, Kings, Aces, Top2, Top3, Top4, Top5, C13, HCP, none=-1 } ;  /* for altcount[count][suit]  */
enum ss_type_ek { ss_A, ss_K, ss_Q, ss_J, ss_T, ss_x, ss_AK, ss_AQ, ss_AJ, ss_AT, ss_Ax,
                  ss_KQ, ss_KJ, ss_KT, ss_Kx, ss_QJ, ss_QT, ss_Qx, ss_JT, ss_Jx, ss_Tx, ss_xx, ss_END, ss_xxx=-1 } ;

struct gbl_struct_st {			/* convenient place for query details */
	int g_tag  ;
	int g_side ;						/* as passed in query req pkt, or as deduced from the compass */
	int g_compass ; 				/* -1 or as passed in by query pkt */
	int g_suit ;						/* -1 or as passed in by query pkt */
	int g_idx  ;
} ;
typedef struct FitPoints_st {
   int df_val[2];
   int fn_val[2] ;
}  FIT_PTS_k;

typedef struct Alt_HCP_st {
   float hand_suit_ahcp[2][4] ;
   float hand_ahcp[2];
   float side_ahcp ;
   int   hand_suit_rhcp[2][4] ;  // rounded? versions of floats or scaled by 100 versions?
   int   hand_rhcp[2];
   int   side_rhcp ;
} ALT_HCP_k ;

struct KnR_points_st {  /* The values will be x100 so we can use ints */
   int knr_honor_pts ;  /* A=3, K=2, Q=1? etc. */
   int knr_short_pts;   /* Each Void=3, Each Stiff=2, Each Dblton except the first = 1 etc. */
   int knr_qual_pts ;   /* (Suitlen/10) * Suithcp. Suit HCP are different from Honor pts */
   int knr_adj ;        /* for Square hand mostly -- the first Dblton adj is not tracked separately */
   int knr_tot_pts ;    /* Total of the above; The traditional ones with No Dfit or Fn */
   int knr_dfit ;       /* Dummy pts: +50% or +100% of shortness pts with an 8fit or a 9 fit. Per the text */
   int knr_Fn_pts ;     /* Declarer pts: +25%, 50%, 100% of the shortness pts for 8, 9, or longer fits. Per the text */
   int knr_misfit_adj;  /* Deduction of shortness pts if a misfit; assume short pts vs Partner's 5+ suit */
   int knr_body_val;    /* Pavlicek Rounding Factor sum(3*Tens + 2*Nines + Eights) */
   int knr_rounded_pts; /* the x100 pts brought into the 0 - 40 pt range using Pav rounding method */
} ;
/* Struct to collect several details re fit(s) one place */
typedef struct UEsidestat_st {
   int side   ;  /* NS=0, EW=1 */
   int t_suit ;  /* best fit suit; longest fit if 2+ equal len choose Major over minor, then most balanced, then weakest */
   int t_rank;
   int t_fitlen;
   int t_len[2];  			/* length in trump suit by hand index: 0=N or E, 1=S or W */
   int decl_h;   				/* 0 or 1 */
   int dummy_h;
   int decl_seat; 			/* Compass Number 0=North, 3=West */
   int dummy_seat;
   int sorted_slen[2][4];  /* suit lengths in desc order longest first[0] shortest last[3]*/
   int sorted_sids[2][4];  /* the suit_ids in desc order of length e.g. 4=1=5=3 would give D,S,C,H i.e. 1,3,0,2 */
   int fitlen[4] ;			/* sorted fitlen array used to select trump suit */
   int fitids[4];				/* sorted so that fitids[i] is the number of the suit whose length is fitlen[i] */
   HANDSTAT_k *phs[2];     /* Pointers * to results of Dealerv2 analyze() function; hcp, suit lens, LTC's etc.*/
   int pav_body[2];        /* useful to many metrics; calc once here ... */
}  UE_SIDESTAT_k ;


/* Misfit, Waste, and No Waste struct.
 * there will be an array of four of these, one per suit -- we use this struct to calc misfit pts, and waste/nowaste pts
 * misfit values are coded: 0 => no misfit; 3 => shortness vs 3 card suit; 4 => vs 4 card suit; 5 vs 5+ suit
 * waste  values are coded: -3 => HCP vs a void, -2 => HCP vs a stiff, 0 => no waste deductions either no short, or +ve for no_waste
 * no_waste vals are coded: +3 =>xxx(x) vs Void, +2 =>xxx(x) vs Stiff, +1 => Ax(x) vs stiff, 0 No shortness, or else wastage.
 * */
typedef struct misfit_st {
   int mf_type   ;   	/* 0 no misfit; +3 misfit vs 3 card suit ; +4 misfit vs 4 card suit; +5 misfit vs 5+ suit */
   int waste     ;   	/* 0 no waste -3 waste vs a void, -2 waste vs a stiff (not Ax(x) */
   int no_waste  ;   	/* 0 no no_waste; +3 xxx(x) vs a void. +2 xxx(x) vs a stiff. +1 Ax(x) vs a stiff */
   int long_hand  ;     /* 0 or 1 the hand with at least 3 in the suit this is the hand index i.e. 0 or 1 not the compass */
   int short_hand ;     /* 0 or 1 the hand with a stiff or void in the suit */
   int lh_len;          /* Long hand len 3+ */
   int sh_len;          /* 0 (void) or 1 (stiff) Short hand len */
   int lh_hcp;          /* hcp in long hand. if xxx(x) or Axx(x) get a plus; otherwise get a minus; minus cannot exceed hcp value */
} MIS_FIT_k ;

typedef  // this union holds the results for one side. the usual case for this kind of query
   union res_ut {
      int res[128] ;               // quick easy access if user can keep track himself.
      struct    {  					  // suggested layout for convenience. will correspond to syntax in dealer input file -- Not used as of 2024/may
         int side_tot[32] ;        //0-31 32 metrics for side as a whole. Usual case otherwise dealer is adequate.
         int side_by_suit[4][8] ;  //32-63  32 diff metrics for the side as a whole, 8 for each suit. [CDHS][wxyz]
         int hand_tot[2][16];      //64-95 hand results  16 diff metrics total for each hand
         int hand_suit[2][4][4];   //96-127  results by suit; 4 diff metrics for each hand-suit combo.
      } dr ;                       // detailed result
  // Syntax to access the above: uval.res[i] or uval.dr.hand_suit[h][s][i] or ueval_ptr->res[j] or ueval_ptr->dr.hand_tot[h][i]
} UE_VALUES_k;

struct UserEvals_st {
   /* the most useful set; NT pts, and pts when playing in longest/best (or asked for) fit. */
   int nt_pts_side;
   int nt_pts_seat[2];
   int hldf_pts_side;
   int hldf_pts_seat[2] ;
   /* misc fields in case we want details re adj, length, syn, Dfit, Fn etc. */
   int misc_pts[122] ;  /* allow for a total of 128 values returned. 126, 127 will be used for hldf_suit, hldf_fitlen if there is room */
   int misc_count ;     /* SaveUserEvals will use this field  */
   int hldf_suit  ;     /*      ditto */
   int hldf_fitlen;     /*      ditto */
} ;

/* set of 6 results per Metric. Used by set40 code */
struct EvalAll_res_st {
   int nt_pts_side;  /* Evaluation in NT:      NS/EW side_total, north/east, south/west */
   int nt_pts_seat[2];
   int bf_pts_side;  /* Evaluation in BestFit: NS/EW side_total, north/east, south/west */
   int bf_pts_seat[2];
} ;

#endif /* file guard */
