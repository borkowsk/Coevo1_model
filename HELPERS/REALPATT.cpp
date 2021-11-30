/**
 * REALNIE STWIERDZONE KLONY W SYMULACJI - DLA KOEWOLUCJI
 * @AUTHOR: Wojciech Borkowski
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#ifdef unix
#include <unistd.h>
#else
#include <dos.h>
#endif
#include <string.h>
#include <iostream>
#include <fstream>

using namespace std;

#include "dos&unix.h"
#include "symshell.h"

#ifdef  __TURBOC__
#define  SIZE 128U
#define MAXLICZNIK 255
typedef unsigned char licznik;
#else
#define  SIZE 256U
#define MAXLICZNIK (MAXINT)
typedef unsigned int licznik;
#endif

const char* filename=NULL; //Nazwa pliku
char  opis[1024]; //Opis z pliku
unsigned char far tab[SIZE][SIZE]; // tablica liczników
unsigned int Max=2;
long num=0;

unsigned char tax_val(licznik f)
{
if(f==0) return 0;
return (log10(f)/log10(Max))*255;
}

#ifdef __BORLANDC__
inline
int pEOF(istream& s )
{
int p=s.eof();
if(p)
     return p;
p=s.get();
if(p==EOF)
	return 1;
s.putback(p);
return 0;
}
#else
inline int pEOF(istream& s )
{
return s.eof();
}
#endif

void load_next_record(istream& in)
{
unsigned long pop,tax,big,fos;
if(pEOF(in))
	{
	sleep(0);
	return;
	}
in>>num>>pop>>tax>>big>>fos;
in.ignore();//eol
if(fos!=0)
printc(0,SIZE+char_height('X')*2,1,128,"%ld) %ld %ld %ld %ld     \t",
					num, pop,tax,big,fos);
if(fos!=0)
  {
  fos&=0x0000ffff; //low 16 bits
  unsigned x=(fos&0xff00)>>8;//gęba
  unsigned y=(fos&0x00ff);//osłona
  if( y==0 )
	fprintf(stderr,"Invalid FOS value=%lx (x=%x y=%x)\n",fos,x,y);
#if SIZE==128
    x/=2;
    y/=2;
#endif
  if(tab[y][x]<MAXLICZNIK)
	    tab[y][x]++;
  if(tab[y][x]>Max)
        Max=tab[y][x];
  plot(x,y,tax_val(tab[y][x]));
  }
}

void replot()
{
unsigned int i,j;
unsigned char pom;
for(i=0;i<SIZE; i++)
    for(j=0;j<SIZE; j++)
	  {
	  pom=tax_val(tab[i][j]);
	  plot(j,i,pom);
	  }
printc(0,SIZE,1,128,"Real existed clones of %s",filename);
printc(0,SIZE+char_height('X'),1,128,"%s",opis);
printc(0,SIZE+char_height('X')*2,1,128,"%ld records             \t",num);
}

/**  OGÓLNA FUNKCJA MAIN
 *************************/

main(int argc,const char* argv[])
{
if(argc<2)
    {
    cout<<"REALNIE STWIERDZONE KLONY W SYMULACJI:\n";
    cout<<"WYWOŁANIE:\n"<<argv[0]<<" coewo.log [opcje]\n";
    exit(1);
    }
filename=argv[1];
ifstream input(filename);
if(!input.good())
    {
    cerr<<"Can't open file "<<filename<<'\n';
    exit(1);
    }
input.get(opis,sizeof(opis));
input.ignore();
input.ignore(1024,'\n');
shell_setup(argv[1],argc,argv);
cout<<"REALNIE STWIERDZONE KLONY W SYMULACJI:\n"<<opis;
cout<<"COLORS= 256 q-quit\n";
cout.flush();
Randomize();
init_plot(SIZE,SIZE,0,3);
while(1)
{
char pom;
if(input_ready())
  {
  pom=get_char();
  if(pom=='\r')
      replot();
  else if(pom=='q') break;
  }
load_next_record(input);
flush_plot();
}
close_plot();
cout<<"Do widzenia!!!\n";
return 0;
}
