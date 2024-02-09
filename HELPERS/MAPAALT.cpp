/**
 * ALTERNATYWNA MAPA INTERAKCJI - DRAPIEŻNIK/OFIARA DLA KOEWOLUCJI
 * @AUTHOR: Wojciech Borkowski
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dos&unix.h"
#include "symshell.h"
/* Funkcje ustalające ceny dla "gęb" i "osłon" */
unsigned cena(unsigned char b);
unsigned  Not(unsigned char b);

/* Tablice na liczniki do średnich */
long poziom[256];/* suma wierszy */
long pion[256]; /* suma kolumn */

void replot()
{
unsigned int i,j,w;
unsigned char pom;
 for(i=1;i<256; i++)
    {
    w=cena(i); // wiersze drukowane w kolejności cen
    for(j=1;j<256; j++)
	  {
      pom=(unsigned char)
          (255*
	  (((unsigned char)(i&j))/double(j) *
          ((unsigned char)(i&j))/double(i))
          );
      pion[j]+=pom;
      poziom[w]+=pom;
      plot(j,w,pom);
	  }
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
    w=cena(i);
    poziom[w]/=255;
    assert(poziom[w]<=255);
    plot(257,w,poziom[w]);
    plot(258,w,poziom[w]);
    plot(259,w,w);
    plot(260,w,w);
    }
}

/**  OGÓLNA FUNKCJA MAIN
 *************************/

int main(int argc,const char* argv[])
{
shell_setup("ALT. INTERACTION MAP",argc,argv);
printf("MAPA INTERAKCJI - DRAPIEŻNIK/OFIARA DLA KOEWOLUCJI\n");
printf("COLORS= 256 q-quit\n");
Randomize();
init_plot(260,260,0,0);
replot();
flush_plot();
while(1)
{
char pom=get_char();
if(pom=='\r')
	replot();
   else if(pom=='q' || pom==-1) break;
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
unsigned char mask=0x1;// od tyłu - najmłodszy bit jest nawięcej wart! (?)
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
