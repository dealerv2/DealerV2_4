/* File libdealerV2.h -- collection of all the header files that specify the I/F to the library modules */
#ifndef LIBDEALERV2_H
#define LIBDEALERV2_H
// header files that define the dealerv2 landscape
#include "../include/dealdefs.h"
#include "../include/dealtypes.h"
#include "../include/dealexterns.h"
#include "../include/dbgprt_macros.h"
// header files that define the interfaces to the various  library modules
#include "../include/deal_conversion_subs.h" /* Convert between Deal52, GIB-PBN, DDS-PBN, etc. */
#include "../include/dealdebug_subs.h"   	/* many different funcs to printout various dealer structs and vars */
#include "../include/dealdeck_subs.h"       /* Freshpack, Shuffle, Deal, etc. */
#include "../include/deal_knr.h"            /* Calc KnR and Suit Qual for each hand */
#include "../include/deal_scorelib.h"       /* convert tricks and contract to score and IMPs */
#include "../include/dealutil_subs.h"       /* init_rand, gen_rand_slot, various sort routines */
#include "../include/libVersion.h"    		/* for GetLibVersion */

#endif

