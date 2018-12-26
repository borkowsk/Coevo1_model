
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
		 Matrix[i][j]=128;
}

double TorusD(int x1,int y1,int x2,int y2)
{
double pox,poy;
x1%=SIZE*2;y1%=SIZE;
x2%=SIZE*2;y2%=SIZE;
pox=(x2-x1);pox*=pox;
poy=(y2-y1);poy*=poy;
pox=sqrt(pox+poy);
return pox;
}

double poison(int n); // generuje rozklad para-poison w zakresie 0..1 o n stopniach
int WSPOLCZYNNIK=10;
void  next_step(void)
{
static long step=0;
long suma=0;
int oddzielnik;
double power,bum;
int x,y;
int i,j;
int pom;
step++;
x=Random(SIZE*2);
y=Random(SIZE);
power=poison(WSPOLCZYNNIK);
assert(power>=0 && power<=1);
for(i=0;i<SIZE;i++)
	for(j=0;j<SIZE*2;j++)
	 {
	 bum=TorusD(x,y,j,i)/(SIZE*2.);//Odleglosc wzgledna na torusie
	 if(bum!=0)
		 {
		 bum=1-bum;
		 bum=power*(bum*bum*bum);
		 }
		 else bum=1;
	 if(bum>1)
		putchar('*');
	 pom=Matrix[i][j];
	 if(pom<=0) continue;
	 bum=(double)pom-255*bum;
	 if(bum<0)
		bum=0;
	 pom=bum;
	 if(bum-pom>0.5) pom++;
	 suma+=(Matrix[i][j]-pom);
	 Matrix[i][j]=pom;
	 plot(j,i,pom);
	 }
oddzielnik=suma /( (long)SIZE*(long)(SIZE*2) );
if(oddzielnik>=1)
 {
  for(i=0;i<SIZE;i++)
	for(j=0;j<SIZE*2;j++)
		{
		Matrix[i][j]+=oddzielnik;
		plot(j,i,Matrix[i][j]);
		}
 suma-=( (long)SIZE*(long)(SIZE*2) );
 }
print(0,SIZE,"[%ld] power=%e   ",step,power);
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
void replot()
{
int i,j;
for(i=0;i<SIZE;i++)
	for(j=0;j<SIZE*2;j++)
	  {
	  plot(j,i,Matrix[i][j]);
	  }
}
/*  OGOLNA FUNKCJA MAIN */
/************************/

main(int argc,char* argv[])
{
shell_setup("ASTEROIDS BOMBING",argc,argv);
printf("SYMYLACJA KATASTROF METEORYTOWYCH \n");
printf("PARAMETR: Wspolczynnik rozkladu q-quit\n");

Randomize();
if(argc>1)
	WSPOLCZYNNIK=atoi(argv[1]);
init_plot(SIZE*2,SIZE,0,0);
start_symulation();
while(1)
if(input_ready())
{
char pom=get_char();
if(pom=='\r')
	replot();
   else
   if(pom=='q')
	  break;

}
else next_step();

close_plot();
printf("Do widzenia!!!\n");
return 0;
}
