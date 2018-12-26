#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Boolean random generator */
static unsigned mask=0x4000;
static buforek=0;
inline int randomBOOLEAN()
{
if(mask==0x4000)
	{
	mask=1;
	buforek=random(RAND_MAX);
	}
	else mask>>=1;
return (buforek & mask)==mask;
}

main()
{
randomize();
long int licznik_0=0,licznik_1=0,z;
for(long int i=0;i<=100000;i++)
      {
//      if((z=random/*BOOLEAN*/(2))==0)
      if((z=randomBOOLEAN())==0)
	  licznik_0++;
	  else
	  licznik_1++;
      putchar(z+1);
      }
printf("0:%ld 1:%ld\n",licznik_0,licznik_1);
}