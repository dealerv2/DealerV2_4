/* file dealdeck_subs.h */
#ifndef DEALDECK_SUBS_H
#define DEALDECK_SUBS_H
#include "../include/std_hdrs.h"
#include "../include/dealdefs.h"
#include "../include/dealtypes.h"
#include "../include/dealdebug_subs.h"   /* for hexdeal_show which now needs a size parm */
#include "../include/dbgprt_macros.h"

void Shuffle(DEAL52_k deck, int sz ) ;  /* Knuth Shuffle */
int idx2Player(int dl_idx) ; 		/* number 0..51 to compass direction */
void newdeck (DEAL52_k d, char dir) ;	/* freshpack in order A (C2 to SA) or D (SA to C2) */
void newpack (DEAL52_k d) ;				/* legacy freshpack in order SA to C2 */
int pack_size(DEAL52_k d) ;				/* count number of cards that are 'real' i.e. not NO_CARD */
int compress_pack(char *d, int dsz ) ; /*put all 'real' cards into slots 0..n and all NO_CARD into slots n+1 .. dsz-1 */
int card_Rank( char card ) ;

#endif
