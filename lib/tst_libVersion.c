/* File libVersion.c -- routine to return a text string of the library version number and build date 
 * Date        Version  Author   Comment
 * 2023/11/04     1.1   JGM 
 * 2023/12/18     1.2   JGM	added unmask() code  
 * 2024/01/15	  1.3   JGM	Added sort to OPC conversion routine  
 * 2024/05/15     1.3a  JGM     Tweak to Version Number       
*/
#include "libVersion.c"

int main(int argc, char **argv ) {
	char vers[] = LIBVERS ;
	char bdate[]= BUILDDATE ;
	char v_bd[] = LIBVERS "_" BUILDDATE ;
	char buff[128];
	int  bufflen; 
	printf("%s, %s, %s \n",vers, bdate, v_bd );
	bufflen = GetLibVers(buff) ;
	printf("GetLibVersion Returns: %d  , %s ",bufflen, buff ) ;  
	return 0;
}
