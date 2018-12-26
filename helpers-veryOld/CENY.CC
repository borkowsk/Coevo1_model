/*
Ceny oslon i gab
*/
#include <iostream.h>
#include <assert.h>

char* bity(unsigned char b);
unsigned cena(unsigned char b);
unsigned  not(unsigned char b);

main()
{
unsigned int i;
cout<<"  bity  \t c.geby \t c.oslony\t "/*alt.c.os*/"\n";
for(i=1;i<=255;i++)
	{
	cout<<bity(i)<<'\t';
	cout.width(8);cout<<i<<'\t';
	cout.width(8);cout<<not(i)<<'\t';
//	cout.width(8);cout<<cena(i)<<'\t';
	cout<<'\n';
	}
 cout.flush();
}

char* bity(unsigned char b)
{
static char tab[9];
tab[8]='\0';
unsigned char mask=0x80;
for(int i=0;i<8;i++)
	{
	tab[i]='0'+((b&mask)!=0);
	mask>>=1;
	}
return tab;
}

unsigned not(unsigned char b)
{
return (unsigned char)~b;
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