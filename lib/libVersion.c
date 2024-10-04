/* File libVersion.c -- routine to return a text string of the library version number and build date 
 * Date        Version  Author   Comment
 * 2023/11/04     1.1   JGM 
 * 2023/12/18     1.2   JGM	added unmask() code  
 * 2024/01/15	   1.3   JGM	Added sort to OPC conversion routine  
 * 2024/05/15     1.3a  JGM   Tweak to Version Number +100 for Debug
 * 2024/06/30     1.4   JGM   Sync version to dealer version pre Github upload   
*/

#define LIBNAME "libdealerV2.a"
#define BUILDDATE "2024/09/20"
#ifdef JGMDBG
	#define LIBVERS "Version 101.4"
#else
   #define LIBVERS "Version 1.4"
#endif
#include <stdio.h>
#include <string.h>
char Version_1_4[]=LIBVERS "_:_" BUILDDATE; /* define this symbol so that it shows up in nm listing of the library */
int GetLibVers(char replybuff[128] ) {
    int reply_len ; 
    // printf("%s\n",Version_1_1);
	snprintf(replybuff,127,"%s : %s : %s", LIBNAME, LIBVERS,BUILDDATE ) ;
	reply_len = (int) strlen(replybuff) ; 
	return (reply_len); 
}
