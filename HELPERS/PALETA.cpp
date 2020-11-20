/*
 * PALETA KOLORÓW - DLA COEWOLUCJI
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
int i;
for(i=0;i<256;i++)
	  {
	  line(i,0,i,50,i);
	  }
}

/*  OGOLNA FUNKCJA MAIN */
/************************/

main(int argc,const char* argv[])
{
shell_setup("COLOR SCALE",argc,argv);
printf("SKALA KOLOROW DLA SYMULACJI\n");
printf("ILOSC KOLOROW= 256 q-quit\n");
Randomize();
init_plot(256,50,0,0);

while(1)
{
char pom=get_char();
if(pom=='\r')
	replot();
   else if(pom=='q') break;
}

close_plot();
printf("Do widzenia!!!\n");
return 0;
}
