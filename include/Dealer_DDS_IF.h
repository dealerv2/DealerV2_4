/* File Dealer_DDS_IF.h -- Imports the minimal amount of DDS types and protos to Dealer code */
#ifndef DEALER_DDS_IF_H
#define DEALER_DDS_IF_H
#include "../include/dealdefs.h"
#include "../include/dealtypes.h"
#include "../include/dds_dll.h"
#include "../include/dealdds_subs.h" 

struct ddTableDeal Deal52_to_DDSBIN( DEAL52_k  d);
struct ddTableDealPBN Deal52_to_DDSPBN ( DEAL52_k  d );
int Deal52_to_Holding( DEAL52_k  d , unsigned int kards[DDS_HANDS][DDS_SUITS] );
    /* a couple of debugging routines */
void showRawResults (  struct ddTableResults *dd );  // ddTableResults uses [strains][hands] index order
void showReturns(  DDSRES_k *dd ) ;
int GIB_to_Deal52( DEAL52_k  dl , char *t ); /* convert a GIB style ( printoneline fmt ) text string to Deal52 see also PBN_to_Deal52 in deck_subs*/
void dump_Deal(struct deal_st dl) ;


#endif
