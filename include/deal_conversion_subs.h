/* File DealConversions.h -- to go with DealConversions.c */
#ifndef DEALCONVERSIONS_H
#define DEALCONVERSIONS_H
#include "../include/dealdefs.h"
#include "../include/dealtypes.h"

void init_export_buff( char *bp , size_t buff_len) ;  /* initialize a buffer for use */

/* These next two return a pointer to the terminating NULL at end of buffer to allow concatenating more than one Predeal in a buffer */
char *Hand52_export(char *buff, int p, DEAL52_k dl );	/* format a DEAL52_k hand as a cmd line Predeal Holding ; does NOT init export Buffer */
char *Side52_export(char *buff, int side, DEAL52_k dl ); /* format both hands of a given side to cmd line Pedeal Holdings; DOES init the Buffer */

 /* convert a simple GIB-PBN text string ( printoneline fmt )  to Deal52_k Will fail if strlen(t) != 76 */
int PBN_to_Deal52( DEAL52_k dl , char *t ) ;

/* Format one or more hands for output in GIB style PBN string in a buffer. 
 * No \n at end. Returns ptr to start of users buffer 
 */
char *fmtHands52_to_PBN( char *buff, int mask, DEAL52_k d ) ; // One or more DEAL52_k hands to a NULL terminated buffer.


	/* Hand52_to_PBNbuff Returns ptr to the NULL at the end of the buffer that contains
	 * the PBN representation of the DEAL52_k hand. Last two chars in the buffer are SPACE NULL
	 * Does NOT put the compass names in the buffer- so can be used to fmt PBN input to DDS 
	 * Does NOT modify the original input. It sorts a local copy of the hand.
	 */
char *Hand52_to_PBNbuff (int p, char *dl52, char *buff ) ; // Fmts hand only; no compass; can be used to setup DDS PBN deal or GIB style 
		// Returns ptr to the NULL at the end of the buffer so can concatenate PBN hands together 

/* These next two are used to output the hand/side in  format DOP/OPC understands. 
 * Puts minus signs for voids, puts / for suit separators, uses -N, -S, -E, -W to specify the hands.
 */		
char *fmtHand52_to_cmdbuff ( char *buff,  int p, DEAL52_k  dl ) ; // returns ptr to null at end of buffer
char *fmtSide_to_cmdbuff(char *buff , int side, DEAL52_k  dl )  ; // returns ptr to null at end of buffer
#endif


