#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#ifdef unix
#include <unistd.h>
#else
#include <dos.h>
#endif
#include <string.h>
#include <iostream.h>
#include <fstream.h>
#include "dos&unix.h"
#include "symshell.h"

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

struct rec
{
unsigned long pop;
unsigned tax;
unsigned big;
unsigned fos;
#ifdef __TURBOC__
unsigned t1,t2,t3;//To 16 B
#endif
};

#ifdef __TURBOC__
#define TAB_SIZE   25000UL
int xsize=320;
int ysize=184;
rec  huge Table[TAB_SIZE];
#else
#define TAB_SIZE  100000UL
int xsize=640;
int ysize=256;
rec       Table[TAB_SIZE];
#endif

char* filename=NULL; 	//Nazwa pliku
char  opis[1024]; 	//Opis z pliku
long num=0;		//maksymalny numer rekordu
long Number_of_records=0; // ile wczytano faktycznie rekordow (w pliku moze byc co ktorys!)
long max_pop=0;		// Wartosci maksymalne do skalowania
long max_tax=0;         // wykresow
long max_big=0;         // wynikowych
int  view_mode=1;	// Tryb wyswietlania

void load_next_record(istream& in)
{
long pop,tax,big,fos;
if(pEOF(in))
	{
	sleep(0);
	return;
	}
in>>num>>pop>>tax>>big>>fos;
in.ignore();//eol
if(num%4==0)
printc(0,ysize,1,128,"%ld) %ld %ld %ld %ld     \t",
					num, pop,tax,big,fos);
if(max_pop<pop)
	max_pop=pop;
if(max_tax<tax)
	max_tax=tax;
if(max_big<big)
	max_big=big;
Table[Number_of_records].pop=pop;
Table[Number_of_records].tax=tax;
Table[Number_of_records].big=big;
Table[Number_of_records].fos=fos;
Number_of_records++;
}

void replot()
{
long i;
unsigned x,y,ox=0;
unsigned o_pop=ysize,o_tax=ysize,o_big=ysize;
clear_screen();
double xscale=xsize/double(Number_of_records+1);
double pop_scale=ysize/double(max_pop);
double tax_scale=ysize/double(max_tax);
double big_scale=ysize/double(max_big);
for(i=0;i<Number_of_records; i++)
	  {
	  x=i*xscale;
	  y=ysize-Table[i].pop*pop_scale;
	  line(ox,o_pop,x,y,32);
	  o_pop=y;
	  y=ysize-Table[i].tax*tax_scale;
	  line(ox,o_tax,x,y,150);
	  o_tax=y;
	  y=ysize-Table[i].big*big_scale;
	  line(ox,o_big,x,y,230);
	  o_big=y;
	  ox=x;
	  }
printc(0,ysize,1,128,"%8ld steps ",num);
printc(char_width('8')*15,ysize,0,32,"POP. ");
printc(char_width('8')*(15+5),ysize,0,150,"TAX. ");
printc(char_width('8')*(15+10),ysize,0,230,"BIG. ");
printc(char_width('8')*(15+15),ysize,0,128,"\t%s",opis);
}

void replot2()
{
long i;
unsigned x,y,ox=0;
clear_screen();
if(Number_of_records==0)
	return;

}

void replot()
{
if(view_mode==1) replot1();
	else 	 replot2();
}

void change_view_mode()
{
view_mode=(++view_mode)/2;
replot();
}

/*  OGOLNA FUNKCJA MAIN */
/************************/

main(int argc,char* argv[])
{
if(argc<2)
    {
    cout<<"PRZEBIEG SYMULACJI:\n";
    cout<<"WYWOLANIE:\n"<<argv[0]<<" coewo.log [opcje]\n";
    exit(1);
    }
if(argv[2] && isdigit(argv[2][0]) )
	xsize=atol(argv[2]);
if(argv[3] && isdigit(argv[3][0]) )
	ysize=atol(argv[3]);
if(xsize<=0 || ysize<=0)
     {
     cerr<<"Invalid screen size: "<<xsize<<"x"<<ysize;
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
cout<<"PRZEBIEG SYMULACJI:\n"<<opis;
cout<<" COLORS= 256 q-quit v-view mode\n";
cout.flush();
Randomize();
init_plot(xsize,ysize,0,1);
while(1)
{
char pom;
if(input_ready())
  {
  pom=get_char();
  if(pom=='\r')
      replot();
  if(pom=='v')
	change_view_mode();
  else if(pom=='q') break;
  }
load_next_record(input);
flush_plot();
}
close_plot();
cout<<"Do widzenia!!!\n";
return 0;
}
