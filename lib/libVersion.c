/* File libVersion.c -- routine to return a text string of the library version number and build date 
 * Date        Version  Author   Comment
 * 2023/11/04     1.1   JGM 
 * 2023/12/18     1.2   JGM	added unmask() code  
 * 2024/01/15	   1.3   JGM	Added sort to OPC conversion routine  
 * 2024/05/15     1.3a  JGM   Tweak to Version Number +100 for Debug
 * 2024/06/30     1.4   JGM   Sync version to dealer version pre Github upload 
 * 2025/04/05     1.5   JGM   Recompiled with GCC13 for Version 24.04 of Ubuntu and Mint 22.1 Xia 
 * 2025/07/01     4.3.3 JGM   Harmonize Build Date and Version with DealerV2 main  
*/

#define LIBNAME "libdealerV2.a"
#define BUILDDATE "2025/07/01"
#ifdef JGMDBG
	#define LIBVERS "Version 104.3.3"
#else
   #define LIBVERS "Version 4.3.3"
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
