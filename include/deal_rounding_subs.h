/* File include/deal_rounding_subs.h */
/* handcrafted sorts and random number routines and others as they occur */
#ifndef DEAL_ROUNDING_SUBS_H
#define DEAL_ROUNDING_SUBS_SUBS_H
#include "../include/dealtypes.h"
int Pav_body_val( HANDSTAT_k  *p_hs ) ;        
int Dotnum2Int( int dotnum , int body );
int Pav_round(float val, int body )    ;
int deal_round( float val , HANDSTAT_k *p_hs );

#endif /* end file guard */
