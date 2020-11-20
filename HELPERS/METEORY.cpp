/*
 * SYMULACJA KATASTROF METEORYTOWYCHI - DLA COEWOLUCJI
 * @AUTHOR: Wojciech Borkowski
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "dos&unix.h"
#include "symshell.h"

#ifdef unix
#define  SIZE 300
#define  far
#else
#define  SIZE 150
#endif

unsigned char far Matrix[SIZE][SIZE*2];

void  start_symulation(void)
{
int i,j;
for(i=0;i<SIZE;i++)
	for(j=0;j<SIZE*2;j++)
		 Matrix[i][j]=255;
}



double poison(int n); // generuje rozklad para-poison w zakresie 0..1 o n stopniach
void krater(int x,int y,int r,int kolor); // Robi dziure z obszarze symulacji
int WSPOLCZYNNIK=10;

void  next_step(void)
{
static long step=0;
double power;
int x,y;
step++;
x=Random(SIZE*2);
y=Random(SIZE);
power=poison(WSPOLCZYNNIK);
assert(power>=0 && power<=1);
krater(x,y,power*SIZE,(1-power)*255/*+Random(10)-5*/);
if(step%100==0)
    printbw(0,SIZE,"[%ld] ",step);
}

double poison(int n)
// generuje rozklad para-poison w zakresie 0..1 o n stopniach
{
double pom=1;
for(int i=1;i<=n;i++)
	pom*=DRand(); // Mnozenie przez wartosc od 0..1
assert(pom>=0 && pom<=1);
return pom;
}

inline
void   Torus(int& x,int& y)
{
if(x<0) x=(SIZE*2)-x;
if(y<0) y=SIZE-y;
x%=SIZE*2;y%=SIZE;
}

inline
void ClearPosition(int x,int y,int color)
{
Torus(x,y);
Matrix[y][x]=color;
plot(x,y,color);
}

void ClearLine(int xxp,int yyp,int n,int color)
{
for(int i=xxp;i<xxp+n;i++)
	ClearPosition(i,yyp,color);
}

int ClearEllipse(int xxs,int yys,int bb,int aa,int color)
{
register int xxp,yyp;
register int xs=xxs;
register int ys=yys;
register int yi=0;
register int xi=0;
register int n=0;

long int a2=aa*aa;
long int b2=bb*bb;
long int a2s=a2;
long int b2s=b2*(2*aa+1);
long int DELTAi=2*(1-bb);

if(aa==0 || bb==0)
		return 0;
if(aa<(long)-MAXINT) yi=-MAXINT;
   else if(aa>(long)MAXINT) yi=MAXINT;
	    else   yi=aa;
yyp=MAXINT;
while( yi>=0 )
{
xxp=xs-xi;
if(yyp!=yi) // Nowa linia
	{
	n=2*xi+1;
	yyp= ys-yi;
	ClearLine(xxp,yyp,n,color);
	yyp= ys+yi;
	ClearLine(xxp,yyp,n,color);
	}
	else // Uzupelnienie lini o punkty brzezne
	{
	xxp=xs+xi;  yyp= ys+yi;
		ClearPosition(xxp , yyp , color+2 );
	xxp=xs-xi;  yyp= ys-yi;
		ClearPosition(xxp , yyp , color+2);
	xxp=xs+xi;  yyp= ys-yi;
		ClearPosition(xxp , yyp , color+2);
	xxp=xs-xi;  yyp= ys+yi;
		ClearPosition(xxp , yyp ,  color+2);
	}
yyp=yi; // zapaminetaj do porownania
BEZRYSOWANIA:
if(DELTAi<0L) goto _1300;
   else {if(DELTAi==0L)
		goto _1240;
	  else
		goto _1180; }

_1180:			 /* decyzja */
if( (DELTAi+DELTAi-a2s)<=0L )
		goto _1240;
	else
		goto _1380;
//continue;

_1240: 			/* krok ukosny */
xi++;
yi--;
a2s+=a2+a2;
b2s-=(b2+b2);
DELTAi+=(a2s-b2s);
continue;

_1300:			/* krok poziomy */
if((DELTAi+DELTAi+b2s)>0L) goto _1240;
xi++;
a2s+=a2+a2;
DELTAi+=a2s;
continue;
//goto BEZRYSOWANIA;

_1380:			/* krok pionowy */
yi--;
b2s-=(b2+b2);
DELTAi-=b2s;
}

return 1;
}

void krater(int x,int y,int r,int kolor)
// Robi dziure z obszarze symulacji
{
if(r<=1) ClearPosition(x,y,kolor);
ClearEllipse(x,y,r,r,kolor);
}


void replot()
{
int i,j;
for(i=0;i<SIZE;i++)
	for(j=0;j<SIZE*2;j++)
	  {
	  plot(j,i,Matrix[i][j]);
	  }
printf("RE-PAINT\n");
fflush(stdout);
}

/*  OGOLNA FUNKCJA MAIN */
/************************/

main(int argc,const char* argv[])
{
shell_setup("ASTEROIDS BOMBING",argc,argv);
printf("SYMULACJA KATASTROF METEORYTOWYCH \n");
printf("PARAMETR (N): Wykladnik rozkladu (0..1)^N    { q-quit }\n");

Randomize();

if(argc>1)
	WSPOLCZYNNIK=atoi(argv[1]);

start_symulation();

init_plot(SIZE*2,SIZE,0,1);

while(1)
{
while(input_ready())
	{
	char pom=get_char();
	putchar(pom);
	fflush(stdout);
	if(pom=='\r')
		replot();
   		else
   		if(pom=='q')
	  		goto EXIT;

	}
next_step();
flush_plot();
}

EXIT:
close_plot();
printf("Do widzenia!!!\n");
return 0;
}
