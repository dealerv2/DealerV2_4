/* File deal_ddslib.h -- define the API dealer uses to call dds lib functions */
#ifndef DEAL_DDSLIB_H
#define DEAL_DDSLIB_H

#include <string.h>
#include <stdio.h>

#include "../include/dealdefs.h"
#include "../include/dealtypes.h"
#include "../include/dds_dll.h"         /* minimal api for DDS library */
#include "../include/deal_dds.h"        /* the link between DDS and Dealer */

/* we need to check some of these to see how many use globals */
extern struct ddTableDeal Deal52_to_DDSBIN(DEAL52_k  d);

DDSRES_k true_CalcTable(DEAL52_k  dl, int vul, int dds_mode ) ; // ddTableResults uses [suits][hands] index order.
int      true_SolveBoard(DEAL52_k  curdeal, int compass, int strain ) ;
int      rplib_Par(DDSRES_k *DealerRes, int par_vuln) ;
int dl2dds_vuln(int dealer_vuln);
int dds2dl_strain(int si);
int dl2dds_tricks(DDSRES_k *DealerRes, struct ddTableResults *Res_20);
DDSRES_k true_CalcTable(DEAL52_k  dl, int par_vul, int dds_mode ) ;
int true_SolveBoard (DEAL52_k  curdeal, int h, int s ) ;
int dds_parscore(int compass ); /* uses globals */
int csv_trix( char *buff, int h_mask ); /* uses globals */
int rplib_Par(DDSRES_k *DealerRes, int par_vuln);  /* maybe no globals */

#endif


