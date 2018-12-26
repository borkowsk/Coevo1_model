
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
for(i=0;i<256U; i++)
    for(j=0;j<256U; j++)
	  {
          pom=(unsigned char)(i&j);
         
          plot(j,i,pom);
	  }
printc(0,256,1,128,"Pattern: x and y");
}

/*  OGOLNA FUNKCJA MAIN */
/************************/

main(int argc,char* argv[])
{
shell_setup("2D BIT \"AND\" PATTERN ",argc,argv);
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
