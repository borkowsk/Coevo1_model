/*
 * MAPA INTERAKCJI - DRAPIEZNIK/OFIARA DLA COEWOLUCJI
 * @AUTHOR: Wojciech Borkowski
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dos&unix.h"
#include "symshell.h"

void replot()
{
unsigned int i,j;
unsigned char pom;
for(i=1;i<256U; i++)
    for(j=1;j<256U; j++)
	  {
      pom= //(unsigned char)
          (255*
          (((unsigned char)(i&j))/double(j) *
          ((unsigned char)(i&j))/double(i))
          );
      plot(j,i,pom);
	  }
printc(0,256,1,128,"Pattern: (x and y)/x * (x and y)/y * MAX");
}

/*  OGOLNA FUNKCJA MAIN */
/************************/

main(int argc,const char* argv[])
{
shell_setup("PATTERN OF INTERACTIONS",argc,argv);
printf("MAPA INTERAKCJI - DRAPIEZNIK/OFIARA DLA COEWOLUCJI\n");
printf("COLORS= 256 q-quit\n");
Randomize();
init_plot(256,256,0,1);
while(1)
{
char pom=get_char();
if(pom=='\r')
	replot();
   else if(pom=='q') break;
putchar(pom);
fflush(stdout);
}
close_plot();
printf("Do widzenia!!!\n");
return 0;
}
