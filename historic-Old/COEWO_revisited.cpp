/* Program symulujacy KOEWOLUCJE */
/* Kazdy osobnik ma swoj bitowy wzorzec odzywiania i bitowy wzorzec */
/* strategi oslony. Jesli ATAKOWANY.OSLONA AND ATAKUJACY.GEBA>0 to  */
/* znaczy ze atak zakonczyl sie powodzeniem.			    */
/* Osobniki rozmnazaja sie kosztem wolnego miejsca i zgromadzonej energi */
/* Ruchy wlasne, ofiara ataku, jak i moment rozmnazania wybierane sa */
/*losowo, aby nie zaciemniac modelu dodatkowymi parametrami. 	     */

#define  _CRT_SECURE_NO_WARNINGS //2013 - MSVC++ 2012 required this to complie strcpy()

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <signal.h>
//#include "dos&unix.h"
#define USES_STDC_RAND
#include "INCLUDE/Random.h"
#include "SYMSHELL/symshell.h"

#ifdef unix
const unsigned MAXBOKSWIATA=200;
const unsigned TAX_OUT=256;
#else
const unsigned MAXBOKSWIATA=100; // swiat jest torusem o obwodzie polodnika = BOKSWIATA
const unsigned TAX_OUT=101;
#endif
			     // 75,100 wpada w cykl z rand ?!
			     // 101 efekt low-migration , artefakt rand
unsigned long MAX_ITERATIONS=0xffffffff; // najwieksza liczba iteracji
unsigned BOKSWIATA=MAXBOKSWIATA; // FAKTYCZNIE UZYWANY BOK SWIATA
unsigned textY=(BOKSWIATA>TAX_OUT?BOKSWIATA:TAX_OUT);
int      WSP_KATASTROF=10;    // Wykladnik rozkladu katastrof
const double WYPOSAZENIE_POTOMSTWA=0.1; // jaka czesc sily oddac potomkowi
double EFEKTYWNOSC_AUTOTROFA=0.99; // jaka czesc swiatla uzywa autotrof
const unsigned MINIMALNY_WIEK=155;    // Rodzi sie z tym wiekiem. Do smierci ma 255-MINIMALNY_WIEK
const unsigned NIEPLODNOSC=10;       // Prawdopodobienstwo rozmnazania jest 1/NIEPLODNOSC
const unsigned PROMIENIOWANIE=160; // Co ile kopiowanych bitow nastepuje mutacja
unsigned VisRand=0;       // Czy pokazywac tlo randomizera (???2013)
unsigned LogRatio=1;      // Co ile krokow zapisywac do logu
char     LogName[128]="coewo.log";

/* ROZMIAR TYPU BASE DECYDUJE O MOZLIWEJ KOMPLIKACJI SWIATA */
/* JEST TYLE MOZLIWYCH TAXONOW ILE WZORCOW BITOWYCH W base2 */
typedef unsigned char base;   // musi byc bez znaku
typedef unsigned short base2; // musi miescic 2 zmienne base
const int   MAXINT=0x1fffffffL;
const base2 MAXBASE2=(base2)0xffffffffL;
const base  MAXBASE =(base)MAXBASE2;
const base  AUTOTROF=MAXBASE;// wzor bitowy autotrofa - swiat go zywi



/* Czesc niezalezna od platformy */
/*********************************/
int parse_options(int argc,const char* argv[])
{
const char* pom;
for(int i=1;i<argc;i++)
    {
    if( *argv[i]=='-' ) /* Opcja X lub symshella */
		continue;
    if((pom=strstr(argv[i],"BOK="))!=NULL) //Nie NULL czyli jest
	{
	BOKSWIATA=atol(pom+4);
    if(BOKSWIATA<2 || BOKSWIATA>MAXBOKSWIATA)
		{
	fprintf(stderr,"Niewlasciwy BOK ==  %u\n",BOKSWIATA);
		return 0;
		}
	}
    else
    if((pom=strstr(argv[i],"BUM="))!=NULL) //Nie NULL czyli jest
    {
    WSP_KATASTROF=atol(pom+4);
    if(WSP_KATASTROF<=0)
	{
	fprintf(stderr,"Ujemny wykladnik katastrof. Katastrofy wylaczone\n");
	}
    }
    else
    if((pom=strstr(argv[i],"AUTO="))!=NULL) //Nie NULL czyli jest
    {
    EFEKTYWNOSC_AUTOTROFA=atof(pom+5);
    if(EFEKTYWNOSC_AUTOTROFA<=0)
		{
        fprintf(stderr,"Efektywnosc autotrofa musi byc w zakresie 0..0.99\n");
		return 0;
        }
    printf("Ilosc iteracji ustawiona na %lu\n",MAX_ITERATIONS);
    }
    else
    if((pom=strstr(argv[i],"MAX="))!=NULL) //Nie NULL czyli jest
    {
    MAX_ITERATIONS=atol(pom+4);
    if(MAX_ITERATIONS<=0)
		{
	fprintf(stderr,"Ilosc iteracji nie moze byc <=0\n");
		return 0;
        }
    printf("Ilosc iteracji ustawiona na %lu\n",MAX_ITERATIONS);
    }
    else
    if((pom=strstr(argv[i],"LOG="))!=NULL) //Nie NULL czyli jest
    {
    LogRatio=atol(pom+4);
    if(LogRatio<=0)
		{
	fprintf(stderr,"Czestosc zapisu nie moze byc <=0\n");
		return 0;
        }
    }
    else
    if((pom=strstr(argv[i],"PLIK="))!=NULL) //Nie NULL czyli jest
    {
    strcpy(LogName,pom+5);
    }
	else /* Ostatecznie wychodzi ze nie ma takiej opcji */
	{
	fprintf(stderr,"Bledna opcja %s\n",argv[i]);
	fprintf(stderr,"MOZLIWE TO:\n");
    fprintf(stderr,"BOK=NN BUM=NN LOG=N \n");
	return 0;
	}
    }
return 1;
}

union wzor
{
base2 	_full;	// pelny wzor bitowy "taxonu"
struct{
	base geba;	// bitowy wzorzec trybu odzywiania
	base oslona;	// bitowy wzozec sposobu ochrony
	} w;
};

//unsigned huge liczniki[ MAXBASE2+1 ];// Liczniki liczebnosci taxonow

class indywiduum
{
friend class swiat;
unsigned char wiek; // ile razy byl u sterowania, po przerolowaniu - smierc
unsigned sila;      // zgromadzona energia
wzor w;		    // wzor bitowy taxonu
/* STATIC */
static unsigned max;//jaki jest najwiekszy taxon
static unsigned max_change;//i czy ostatnio max sie zmienil
static unsigned plot_mode; 	// co ma byc wyswietlane
static unsigned ile_ind;// ile jest zywych indywiduow
static unsigned ile_tax;// ile taxonow niezerowych
static unsigned ile_big_tax;// ile taxonow liczniejszych niz 10
static unsigned liczniki[ (long)MAXBASE2+1 ];// Liczniki liczebnosci taxonow
public:
static void set_mode(int p) { if(p>=0 && p<=4) plot_mode=p; }
static unsigned char tax_val(base2);
inline static   void tax(base2);  // rejestracja zmiany wartosci taxonuna ekranie
indywiduum(){w._full=wiek=sila=0;}
void clear(){w._full=wiek=sila=0;}
int  jest_zywy(){ return (w.w.oslona!=0 && w.w.geba!=0 && sila!=0 && wiek!=0); }
inline void plot(int x,int y); // wyswietlanie zgodnie z trybem
void init(base2 iwfull, unsigned isila); // inicjacja nowego indywiduum
void init(indywiduum& rodzic); // inicjacja nowego jako potomka starego
void kill();		       // smierc indywiduum
void kill(indywiduum& zabojca);// usmiercenie indywiduum przez drugie
void uplyw_czasu();	       // oddzialywanie czasu
static base2 kopioj(base2 r);  // kopiuje genotyp z mozliwa mutacja
};

class swiat
{
indywiduum ziemia[MAXBOKSWIATA][MAXBOKSWIATA*2];// podloze dla indywiduuw
unsigned long licznik;			  // licznik krokow symulacji
FILE* log;
public:
void Torus(int& x,int& y)
{
if(x<0) x=(BOKSWIATA*2)-x;
if(y<0) y=BOKSWIATA-y;
x%=BOKSWIATA*2;y%=BOKSWIATA;
}
void ClearPosition(int x,int y)
{
Torus(x,y);
if(!ziemia[y][x].jest_zywy()) return;
ziemia[y][x].kill();
ziemia[y][x].plot(x,y);
}
void ClearLine(int xxp,int yyp,int n);
void Krater(int x,int y,int r); // Robi dziure z obszarze symulacji
swiat(char* logname);
~swiat(){ fclose(log); }
void wskazniki(); // wyswietlenie nowych wartosci wskaznikow
void mapa_taxonow();//wyswietlenie mapy taxonow
void caly_ekran();// odnowienie calego ekranu np po zmianie trybu wyswietlania
void dump();	  // zrzucenie stanu do logu
void init();	  // stan startowy symulacji
void kataklizm(); // wygenerowanie katastrofy
void krok();	  // kolejny krok symulacji
int sterowanie(); // ewentualne sterowanie z zewnatrz.
};

/* IMPLEMENTACJA CZESCI NIEZALEZNEJ OD PLATFORMY */
/*************************************************/
void swiat::ClearLine(int xxp,int yyp,int n)
{
for(int i=xxp;i<xxp+n;i++)
	ClearPosition(i,yyp);
}
swiat::swiat(char* logname)
{
log=fopen(logname,"a");
if(!log)
	{
	perror(logname);
	exit(2);
	}
licznik=0;
}

inline void indywiduum::plot(int x,int y)
// wyswietlanie zgodnie z trybem
{
int pom;
switch(plot_mode){
 case 0:pom=w.w.geba; break;
 case 1:pom=w.w.oslona;	break;
 case 2:if(sila)pom=sila/16+1;
	   else pom=0;
	break;
 case 3:pom=wiek;	break;
 case 4:/* hidden */    return;
 default:
	fprintf(stderr,"INTERNAL: plot_mode=%d >3\n",plot_mode);
	abort();
 };
::plot(x,y,(pom<256?pom:255));
}

unsigned char indywiduum::tax_val(base2 f)
{
	if(liczniki[f]==0) return 0;
	return (unsigned char)(( log10(double(liczniki[f]))/log10(double(max)) ) * 255); //bo MSVC++ ma:  error C2668: 'log10' : ambiguous call to overloaded function
}

void indywiduum::tax(base2 f)
{
unsigned x=(f&0xff00)>>8;
unsigned y=f&0x00ff;
assert( y>0 && y<=MAXBASE);
assert( x>0 && x<=MAXBASE);

unsigned char c=indywiduum::tax_val(f);// geby na x-ach
if(TAX_OUT==128)
     {  y/=2; }
::plot(x,y,c);
}

void swiat::mapa_taxonow()
{
unsigned x,y,c;
for(unsigned i=0;i<256U;i++)
   for(unsigned j=0;j<256U;j++)
     {
     x=j;
     y=i;
     c=indywiduum::tax_val(x*256+y);// geby na x-ach
     if(TAX_OUT==128)
	  { y/=2; }
     plot(x,y,c);
     }
indywiduum::max_change=0;// Obraz juz aktulny
}

void swiat::caly_ekran()
{
int a,b;
if(indywiduum::plot_mode==4)
  mapa_taxonow();
  else
  {
  indywiduum::ile_ind=0;
  for(a=0;a<BOKSWIATA;a++) // po kolejnych wierszach czyli y
	for(b=0;b<BOKSWIATA*2;b++)// po kolumnach w wierszu czyli x
		{
		ziemia[a][b].plot(b,a);
		if(ziemia[a][b].jest_zywy())
			indywiduum::ile_ind++;
		}
  }
}

void swiat::wskazniki()
{
/* LOSOWANIE SKAMIENIALOSCI */
unsigned x=RANDOM(BOKSWIATA*2);
unsigned y=RANDOM(BOKSWIATA);
printc(0,textY,0,128,"%c [%lu] IND:%lu TAX:%lu BIG:%lu ",
	//indywiduum::nazwy[indywiduum::plot_mode],
	char('A'+indywiduum::plot_mode),
	(unsigned long)licznik,
	(unsigned long)indywiduum::ile_ind,
	(unsigned long)indywiduum::ile_tax,
	(unsigned long)indywiduum::ile_big_tax);
if(licznik%LogRatio==0)
  fprintf(log,"%lu\t%lu\t%lu\t%lu\t%u\n",
	(unsigned long)licznik,
	(unsigned long)indywiduum::ile_ind,
	(unsigned long)indywiduum::ile_tax,
	(unsigned long)indywiduum::ile_big_tax,
/* Wazna jest zeby specjalizacja pokarmowa byla wazniejsza od oslony */
       (unsigned)(( (unsigned)ziemia[y][x].w.w.geba*((unsigned)MAXBASE+1) )+ziemia[y][x].w.w.oslona)
       );
if(indywiduum::plot_mode==4 && indywiduum::max_change)
	mapa_taxonow();
}

int swiat::sterowanie()
{
int zmieniony=0;
if(licznik>=MAX_ITERATIONS) return 0;
while(input_ready())
	{
	zmieniony=1;
    // putchar(7);
	switch(get_char()){
	case 'g':/* GEBA */ indywiduum::set_mode(0);break;
	case 'o':/*OSLONA */indywiduum::set_mode(1);break;
	case 's':/*SILA  */ indywiduum::set_mode(2);break;
	case 'w':/*WIEK  */ indywiduum::set_mode(3);break;
	case 't':/*TAXONY*/ indywiduum::set_mode(4);break;
	case '+':VisRand=1;break;
	case '-':VisRand=0;break;
	case 'b':{
		int x=RANDOM(BOKSWIATA*2);
		assert(x>=0 && x<BOKSWIATA*2);
		int y=RANDOM(BOKSWIATA);
		assert(y>=0 && y<BOKSWIATA);
		double power=DRAND();
		assert(power>=0 && power<=1);
		Krater(x,y,int(power*BOKSWIATA));
		}break;
        case 'f':fflush(log);break;
    case -1:
	case 'q':/*QUIT */ return 0;
	}
	}
if(zmieniony)
	caly_ekran(); // Po zmianie trybu trzeba odnowic calosc
return 1;
}


void swiat::init()
{
ziemia[BOKSWIATA/2][BOKSWIATA].init(MAXBASE2,0xff);

fprintf(log,"%ux%u\tWYP_POT=%f\tEFEKT=%f\tMAX_WIEK=%d\tPLOD=%f\tRTG=%f\tBUM=0.5^%d\n",
BOKSWIATA,BOKSWIATA*2,WYPOSAZENIE_POTOMSTWA,EFEKTYWNOSC_AUTOTROFA,(int)(255-MINIMALNY_WIEK),1./NIEPLODNOSC,1./PROMIENIOWANIE,WSP_KATASTROF);
fprintf(log,"N#\tIND\tTAX\tBIG\tFOS\n");
}


/* NAJWAZNIEJSZE FUNKCJE - GLOWNA IDEA SYMULACJI */

void indywiduum::init(base2 iwfull, unsigned isila)
 // inicjacja nowego indywiduum
{
w._full=iwfull;
sila=isila;
wiek=MINIMALNY_WIEK;
assert(w._full>0);
// Sprawdzenie czy oslona nie jest za dobra i czy inne parametry sa OK
if(w.w.oslona==0 || !jest_zywy())
      {
      clear();
      return;
      }
ile_ind++;
iwfull=w.w.geba*256+w.w.oslona;
liczniki[iwfull]++;
assert(*(liczniki+iwfull));
if(liczniki[iwfull]>max)
		{
		max=liczniki[iwfull];
		max_change=1;
		}
if(plot_mode==4)
	tax(iwfull);//wyswietlanie taxonu
if( liczniki[iwfull]==1 ) // pierwszy przedstawiciel taxonu
		ile_tax++;   // wiec liczba taxonow wzrasta
if( liczniki[iwfull]==11 ) // osiagnal wartosc >10 duzy taxon - rozwojowy
		ile_big_tax++;
}

void indywiduum::kill()
// -----//---- przez swiat
{
assert(w.w.oslona>0);
assert( w._full>0 );
ile_ind--;
base2 wfull=w.w.geba*256+w.w.oslona;
liczniki[wfull]--;
if(plot_mode==4)
	tax(wfull);//wyswietlanie taxonu
if( liczniki[wfull]==0 )	//ostatni przedstawiciel tego taxonu
	ile_tax--;
if( liczniki[wfull]==10 ) // osiagnal wartosc <=10 maly taxon - nie rozwojowy
	ile_big_tax--;
assert(w._full>0 );
w._full=sila=wiek=0;
}

void indywiduum::init(indywiduum& rodzic)
 // inicjacja nowego jako potomka starego
{
w._full=kopioj(rodzic.w._full);
unsigned uposazenie=unsigned(rodzic.sila*WYPOSAZENIE_POTOMSTWA);
unsigned cena=w.w.geba + (base)(~w.w.oslona) + uposazenie; // Oslona 0 jest najdrozsza
if( rodzic.sila<=cena )  // Nie ma sily na potomka
	{ w._full=0; return; }
rodzic.sila-=cena; 	 // Placi za wyprodukowanie i wyposazenie
assert(rodzic.sila!=0);
init(w._full,uposazenie);   // prawdziwa inicjacja
}

void indywiduum::kill(indywiduum& zabojca)
// usmiercenie indywiduum przez drugie
{
if(zabojca.sila==0) return; //niezdolny zabijac
assert(w.w.oslona>0);
assert(w._full>0);
/* Zabojca dostaje pewna czesc sily */
/* proporcjonalna do tego ile bitow oslony pasuje do jego geby */
if(w.w.oslona!=0)
   if(zabojca.w.w.geba!=0)
       zabojca.sila+=unsigned(sila *
    double(w.w.oslona & zabojca.w.w.geba)/(zabojca.w.w.geba)*
    double(w.w.oslona & zabojca.w.w.geba)/(w.w.oslona)
    );
assert(zabojca.sila!=0);
assert(w._full>0);
/* Potem ofiara ginie */
kill();
}

void indywiduum::uplyw_czasu()
// prawo czasu - wszystko sie starzeje
{
assert(w.w.oslona>0);
assert(w._full>0);
wiek++;	       // Normalne starzenie sie
if(w.w.geba==AUTOTROF) //JEST  A U T O T R O F E M
	{
    sila+=unsigned(100*EFEKTYWNOSC_AUTOTROFA-1);// bez zutu gnu sie czepia a tak ma byc!
	assert(sila!=0);
	}
	else
	sila--;// Metaboliczne zurzycie energii
assert(w._full>0);
}

base2 indywiduum::kopioj(base2 r)
// kopiuje genotyp z mozliwa mutacja
{
base2 mask=( RANDOM(PROMIENIOWANIE) );
if(mask<=16) // Prowizorka - nieprzenosne jesli base >16bitowe
	{
	mask=0x1<<mask;
	r^=mask;
	}
return r;
}

struct vector2
{ signed char x,y; };

void swiat::krok()
{
static vector2 kierunki[]={{1,1},{-1,-1},{1,-1},{-1,1},
			   {0,1},{ 1, 0},{0,-1},{-1,0} };
long ile= long(BOKSWIATA)*long(2*BOKSWIATA); // ile na krok MonteCarlo
licznik++;
for(long i=0;i<ile;i++) // rob krok MonteCarlo
	{
	unsigned x=RANDOM(BOKSWIATA*2);
	unsigned y=RANDOM(BOKSWIATA);
    if(VisRand)
		plot(x,y,255+128);//+ 26.09.2013
	if(!ziemia[y][x].jest_zywy()) // jest martwy
		continue;		// obrob nastepnego
	ziemia[y][x].uplyw_czasu();
	if(!ziemia[y][x].jest_zywy())  // nie moze dalej zyc
		{
		ziemia[y][x].kill();
		plot(x,y,0);		// zamaz
		continue;               // obrob nastepnego
		}
	unsigned a=RANDOM(8);
	assert(a<8);
	unsigned x1=(x+kierunki[a].x) % (BOKSWIATA*2);
	unsigned y1=(y+kierunki[a].y) % BOKSWIATA;
	if(!ziemia[y1][x1].jest_zywy()) // wolne miejsce
	   {
	   if(RANDOM(NIEPLODNOSC)==0)     // rozmnazanie
		{
		ziemia[y1][x1].init(ziemia[y][x]);
		}
		else			  // przemieszczenie
		{
		ziemia[y1][x1]=ziemia[y][x];
		ziemia[y][x].clear();
		}
	   ziemia[y1][x1].plot(x1,y1);
	   ziemia[y][x].plot(x,y);
	   }
	   else  // Naprawde jest zywy - proba zjedzenia, jesli atakujacy nie jest autotrofem
	   if( ziemia[y][x].w.w.geba!=AUTOTROF &&
	      (ziemia[y1][x1].w.w.oslona & ziemia[y][x].w.w.geba) != 0)
		   {
		   ziemia[y1][x1].kill(ziemia[y][x]);
		   plot(x1,y1,0);
		   }
	}
kataklizm(); // jeden, chocby maly na krok monte-carlo
}

double poison(int n); // generuje rozklad para-poison w zakresie 0..1 o n stopniach

void  swiat::kataklizm(void)
{
double power;
int x,y;
if(WSP_KATASTROF<0)
   return; //spontaniczne katastrofy wylaczone
x=RANDOM(BOKSWIATA*2);
y=RANDOM(BOKSWIATA);
power=poison(WSP_KATASTROF);
assert(power>=0 && power<=1);
Krater(x,y,power*BOKSWIATA);
}


/*  OGOLNA FUNKCJA MAIN */
/************************/
//void (*signal (int sig, void (*func)(...)))(...);

void MySignalHook(int par)
{
fprintf(stderr," Recived signal %d\n",par);
fflush(NULL);
close_plot();
exit(par);
}

void install_signal_hooks()
{
#ifndef __NONCOPATIBLE__
typedef void (*SIG_TYPE)(int);
#else
typedef void (*SIG_TYPE)(...);
#endif
signal( SIGINT ,(SIG_TYPE)MySignalHook);
#ifdef unix
signal( SIGHUP ,(SIG_TYPE)MySignalHook);
signal( SIGPIPE ,(SIG_TYPE)MySignalHook);
#endif
signal( SIGTERM ,(SIG_TYPE)MySignalHook);
//signal(  ,MySignalHook());
}

int main(int argc,const char* argv[])
{
shell_setup("CO-EVOLUTION",argc,argv);
printf("KOEWOLUCJA: program symulujacy kooewolucje wielu gatunkow\n");
printf("POLECENIA: 'g': GEBA 'o':OSLONA 's':SILA 'w':WIEK 'q':QUIT\n");
printf("LICZBA MOZLIWYCH KLONOW=%lu MAXINT=%d\n",(unsigned long)MAXBASE2,MAXINT);

if(!parse_options(argc,argv))
        exit(1);

if(sizeof(base)*2!=sizeof(base2))
	{
	fprintf(stderr,"Niewlasciwe rozmiary dla typow bazowych:2*%u!=%u\n",
		sizeof(base),sizeof(base2));
	exit(1);
	}
swiat& tenSwiat=*new swiat(LogName);
if(&tenSwiat==NULL)
    {
    fprintf(stderr,"Brak pamieci!\n");
    exit(1);
    }

RANDOMIZE();

init_plot( BOKSWIATA*2+1, textY,0,1); //(BOKSWIATA*2>256?BOKSWIATA*2:256) POCO???

tenSwiat.init();
tenSwiat.wskazniki();
tenSwiat.caly_ekran();
install_signal_hooks();
while(1)
	{
	tenSwiat.krok();
	tenSwiat.wskazniki();
	flush_plot();
	if(!tenSwiat.sterowanie())
		break;
	}
close_plot();
printf("Do widzenia!!!\n");
return 0;
}

double poison(int n)
// generuje rozklad para-poison w zakresie 0..1 o n stopniach
{
double pom=1;
for(int i=1;i<=n;i++)
	pom*=DRAND(); // Mnozenie przez wartosc od 0..1
assert(pom>=0 && pom<=1);
return pom;
}


/* STATIC ALLOCATION */
unsigned indywiduum::max=0;//jaki jest najwiekszy taxon
unsigned indywiduum::max_change=0;//i czy ostatnio max sie zmienil
unsigned indywiduum::plot_mode=0;// co ma byc wyswietlane
unsigned indywiduum::ile_ind=0;// ile jest zywych indywiduow
unsigned indywiduum::ile_tax=0;// ile taxonow niezerowych
unsigned indywiduum::ile_big_tax=0;// ile taxonow liczniejszych niz 10
unsigned indywiduum::liczniki[ (long)MAXBASE2+1 ];// Liczniki liczebnosci taxonow
				   // W DOSie sprawia klopot, stad cast

/* KRATER - TRANSLATED FROM BASIC CODE IMPLEMENTED ELLIPSE DRAWING
   BASED ON BRESENHAM ALGORITM                                      */
void swiat::Krater(int x,int y,int r)
{
// Robi dziure z obszarze symulacji
if(r<=1)
	{ ClearPosition(x,y); return; }
//ClearEllipse(x,y,r,r);// expanded below
//if(r>5)
//    putchar(7);
int xxs=x;
int yys=y;
int bb=r;
int aa=r;
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
		return ;
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
	ClearLine(xxp,yyp,n);
	yyp= ys+yi;
	ClearLine(xxp,yyp,n);
	}
	else // Uzupelnienie lini o punkty brzezne
	{
	xxp=xs+xi;  yyp= ys+yi;
		ClearPosition(xxp , yyp );
	xxp=xs-xi;  yyp= ys-yi;
		ClearPosition(xxp , yyp );
	xxp=xs+xi;  yyp= ys-yi;
		ClearPosition(xxp , yyp );
	xxp=xs-xi;  yyp= ys+yi;
		ClearPosition(xxp , yyp );
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

}
}
