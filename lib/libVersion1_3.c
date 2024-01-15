/* File libVersion.c -- routine to return a text string of the library version number and build date 
 * Date        Version  Author   Comment
 * 2023/11/04     1.1   JGM 
 * 2023/12/18     1.2   JGM		added unmask() code  
 * 2024/01/15		1.3   JGM		Added sort to OPC conversion routine  
*/
#define LIBNAME "libdealerV2.a"
#define LIBVERS "Version 1.3"
#define BUILDDATE "2024/01/15"
#include <stdio.h>
#include <string.h>
char Version_1_3[]=LIBVERS ; 
int GetLibVers(char replybuff[128] ) {
    int reply_len ; 
    // printf("%s\n",Version_1_1);
	snprintf(replybuff,127,"%s : %s : %s", LIBNAME, LIBVERS,BUILDDATE ) ;
	reply_len = (int) strlen(replybuff) ; 
	return (reply_len); 
}
