#include <fstream.h>
#include <iomanip.h>
const BEG=255;
const END=0;
double tablica[256];

char* bity(unsigned char b);
unsigned cena(unsigned char b);
unsigned  not(unsigned char b);

main()
{
ofstream out("wyplaty.txt");
int i,j;
out<<"geby   :\t";
for(i=BEG;i>=END;i--)
	out<<bity(i)<<'\t';
out<<'\n'<<"ceny   :\t";
for(i=BEG;i>=END;i--)
       {out.width(8);out<<i<<'\t';}
out<<'\n';
for(j=BEG;j>=END;j--)
 {
 out<<bity(j)<<'\t';
 for(i=BEG;i>=END;i--)
	out<<bity(j&i)<<'\t';
 out<<'\n';
 out.width(8);
 out<<cena(j)<<'\t';
 for(i=BEG;i>=END;i--)
	if(j>0)
	{
	out.width(8);
	out.precision(3);
	out<<(double)(j&i)/j<<'\t';
	}
	else
	out<<"   -    "<<'\t';
 out<<'\n';
 out.width(8);
 out<<not(j)<<'\t';
 for(i=BEG;i>=END;i--)
	if(i>0)
	{
	out.width(8);
	out.precision(3);
	out<<(double)(j&i)/i<<'\t';
	}
	else
	out<<"   -    "<<'\t';
 out<<'\n';
 out.width(8);
 out<<" %i*%j  "<<'\t';
 for(i=BEG;i>=END;i--)
	if(i>0 && j>0)
	{
	out.width(8);
	out.precision(3);
	out<<((double)(j&i)/i)*((double)(j&i)/j)<<'\t';
	}
	else
	out<<"   -    "<<'\t';
 out<<'\n';
 out.flush();
 }
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
return c;
}