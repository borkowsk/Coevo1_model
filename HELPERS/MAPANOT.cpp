/**
 * MAPA INTERAKCJI - DRAPIEŻNIK/OFIARA DLA KOEWOLUCJI
 * @AUTHOR: Wojciech Borkowski
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dos&unix.h"
#include "symshell.h"
/* Funkcje ustalajace ceny dla "geb" i "oslon" */
unsigned cena(unsigned char b);
unsigned  Not(unsigned char b);

/* Tablice na liczniki do srednich */
long poziom[256];/* suma wierszy */
long pion[256]; /* suma kolumn */

void replot()
{
unsigned int i,j;
unsigned char pom;
 for(i=1;i<256; i++)
    for(j=1;j<256; j++)
	  {
      pom=(unsigned char)
          (255*
	  (((unsigned char)(i&j))/double(j) *
          ((unsigned char)(i&j))/double(i))
          );
      pion[j]+=pom;
      poziom[i]+=pom;
      plot(j,i,pom);
	  }
for(i=1;i<256; i++)
    {
    pion[i]/=255;
    assert(pion[i]<=255);
    plot(i,257,pion[i]);
    plot(i,258,pion[i]);
    plot(i,259,i);
    plot(i,260,i);
    }
for(i=1;i<256; i++)
    {
    poziom[i]/=255;
    assert(poziom[i]<=255);
    plot(257,i,poziom[i]);
    plot(258,i,poziom[i]);
    plot(259,i,Not(i));
    plot(260,i,Not(i));
    }
}

/*  OGOLNA FUNKCJA MAIN */
/************************/

main(int argc,const char* argv[])
{
shell_setup("(NOT) INTERACTION MAP",argc,argv);
printf("MAPA INTERAKCJI - DRAPIEZNIK/OFIARA DLA COEWOLUCJI\n");
printf("COLORS= 256 q-quit\n");
Randomize();
init_plot(260,260,1,1);
replot();
flush_plot();
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

unsigned cena(unsigned char b)
{
unsigned c=0;
unsigned char mask=0x1;// od tylu - najmlodszy bit jest nawiecej wart!
for(int i=0;i<8;i++)
	{
	c<<=1;
	if((b&mask)==0)
		c++;
	mask<<=1;
	}
assert(c<=255);
return c;
}

unsigned Not(unsigned char b)
{
return (unsigned char)~b;
}
