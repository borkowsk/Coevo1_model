
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <values.h>
#include <iostream.h>
#ifdef unix
#include <unistd.h>
#else
#include <dos.h>
#endif
#include "bits.h"
#include "dos&unix.h"
#include "symshell.h"
#define XSIZE 256
#ifdef __TURBOC__
#define YSIZE 180
#else
#define YSIZE 256
#endif

void replot()
{
unsigned i,j;
unsigned pom;
for(i=0;i<256U; i++)
    for(j=0;j<256U; j++)
	  {
	  pom= bits(i)+bits(j); // suma bitow
	  pom=16-pom;		// ile usunieto! SPECJALIZACJA na tym polega.
	  pom*=10;		//Pomnozona przez 10 -> Wiekszy zakres kolorow
//	cerr<<pom<<'\n';
	  pom+=5;		//Zeby 0 bitow nie bylo czarne
          plot(j,i,pom);
	  }
printc(0,256,1,128,"Specialisation pattern");
}

/*  OGOLNA FUNKCJA MAIN */
/************************/

main(int argc,char* argv[])
{
shell_setup("BIT SPECIALISATION PATTERN OF LEVELS",argc,argv);
printf("COLORS= 165 q-quit\n");
Randomize();

init_plot(XSIZE,YSIZE,0,1);

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
