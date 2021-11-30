/* Program symulujacy KOEWOLUCJE */
/* Kazdy osobnik ma swoj bitowy wzorzec odzywiania i bitowy wzorzec */
/* strategi oslony. Jesli ATAKOWANY.OSLONA AND ATAKUJACY.GEBA>0 to  */
/* znaczy ze atak zakonczyl sie powodzeniem.			    */
/* Osobniki rozmnazaja sie kosztem wolnego miejsca i zgromadzonej energi */
/* Ruchy wlasne, ofiara ataku, jak i moment rozmnazania wybierane sa */
/*losowo, aby nie zaciemniac modelu dodatkowymi parametrami. 	     */

#define huge 
const int BOKSWIATA=50; // swiat jest torusem o obwodzie polodnika = BOKSWIATA
const double WYPOSAZENIE_POTOMSTWA=0.1; // jaka czesc sily oddac potomkowi
const double EFEKTYWNOSC_AUTOTROFA=0.3; // jaka czesc swiatla uzywa autotrof
const int MINIMALNY_WIEK=200; // Rodzi sie z tym wiekiem. Do smierci ma 255-MINIMALNY_WIEK
const int NIEPLODNOSC=10;  // Prawdopodobienstwo rozmnazania jest 1/NIEPLODNOSC
const int PROMIENIOWANIE=64; // Co ile kopiowanych bitow nastepuje mutacja


/* ROZMIAR TYPU BASE DECYDUJE O MOZLIWEJ KOMPLIKACJI SWIATA */
/* JEST TYLE MOZLIWYCH TAXONOW ILE WZORCOW BITOWYCH W base2 */
typedef unsigned char base;   // musi byc bez znaku
typedef unsigned short base2; // musi miescic 2 zmienne base
const base2 MAXBASE2=(base2)0xffffffffL;
const base  AUTOTROF=(base)MAXBASE2;// wzor bitowy autotrofa - swiat go zywi

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "dos&unix.h"
#include "symshell.h"

/* Czesc niezalezna od platformy */
/*********************************/
union wzor
{
base2 	full;	// pelny wzor bitowy "taxonu"
struct{
	base geba;	// bitowy wzorzec trybu odzywiania
	base oslona;	// bitowy wzozec sposobu ochrony
	} w;
};

class indywiduum
{
friend class swiat;
unsigned char wiek; // ile razy byl u sterowania, po przerolowaniu - smierc
unsigned sila;      // zgromadzona energia
wzor w;		    // wzor bitowy taxonu
/* STATIC */
static unsigned plot_mode; 	// co ma byc wyswietlane
static unsigned ile_ind;// ile jest zywych indywiduow
static unsigned ile_tax;// ile taxonow niezerowych
static unsigned ile_big_tax;// ile taxonow liczniejszych niz 10
static unsigned huge liczniki[ MAXBASE2 ];// Liczniki liczebnosci taxonow
public:
indywiduum(){w.full=wiek=sila=0;}
void clear(){w.full=wiek=sila=0;}
int  jest_zywy(){ return (w.full!=0 && sila!=0 && wiek!=0); }
static void set_mode(int p) { if(p>=0 && p<=3) plot_mode=p; }
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
indywiduum ziemia[BOKSWIATA][BOKSWIATA*2];// podloze dla indywiduuw
unsigned long licznik;			  // licznik krokow symulacji
FILE* log;
public:
swiat(char* logname);
~swiat(){ fclose(log); }
void wskazniki(); // wyswietlenie nowych wartosci wskaznikow
void caly_ekran();// odnowienie calego ekranu np po zmianie trybu wyswietlania
void dump();	  // zrzucenie stanu do logu
void init();	  // stan startowy symulacji
void krok();	  // kolejny krok symulacji
int sterowanie(); // ewentualne sterowanie z zewnatrz.
};

/* IMPLEMENTACJA CZESCI NIEZALEZNEJ OD PLATFORMY */
/*************************************************/
swiat::swiat(char* logname)
{
log=fopen(logname,"a");
if(!log)
	{
	perror(logname);
	exit(2);
	}
}

inline void indywiduum::plot(int x,int y)
// wyswietlanie zgodnie z trybem
{
int pom;
switch(plot_mode){
 case 0:pom=w.w.geba;	break;
 case 1:pom=w.w.oslona;	break;
 case 2:pom=sila;	break;
 case 3:pom=wiek;	break;
 default:
	fprintf(stderr,"INTERNAL: plot_mode=%d >3\n",plot_mode);
	abort();
 };
::plot(x,y,pom);
}

void swiat::caly_ekran()
{
assert(indywiduum::plot_mode<4);
int a,b;
indywiduum::ile_ind=0;
for(a=0;a<BOKSWIATA;a++) // po kolejnych wierszach czyli y
	for(b=0;b<BOKSWIATA*2;b++)// po kolumnach w wierszu czyli x
		{
		ziemia[a][b].plot(b,a);
		if(ziemia[a][b].jest_zywy())
			indywiduum::ile_ind++;
		}
assert(indywiduum::plot_mode<4);
}

void swiat::wskazniki()
{
assert(indywiduum::plot_mode<4);
/* LOSOWANIE SKAMIENIALOSCI */
unsigned x=Random(BOKSWIATA*2);
unsigned y=Random(BOKSWIATA);  
print(0,BOKSWIATA+1,"%c [%lu] IND:%lu TAX:%lu BIG:%lu ",
	//indywiduum::nazwy[indywiduum::plot_mode],
	char('A'+indywiduum::plot_mode),
	(unsigned long)licznik,
	(unsigned long)indywiduum::ile_ind,
	(unsigned long)indywiduum::ile_tax,
	(unsigned long)indywiduum::ile_big_tax);
if(licznik%2==0)
  fprintf(log,"%lu\t%lu\t%lu\t%lu\t%lu\n",
	(unsigned long)licznik,
	(unsigned long)indywiduum::ile_ind,
	(unsigned long)indywiduum::ile_tax,
	(unsigned long)indywiduum::ile_big_tax,
	(unsigned long)ziemia[y][x].w.full);
assert(indywiduum::plot_mode<4);
}

int swiat::sterowanie()
{
assert(indywiduum::plot_mode<4);
int zmieniony=0;
while(input_ready())
	{
	zmieniony=1;
	putchar(7);
	switch(get_char()){
	case 'g':/* GEBA */ indywiduum::set_mode(0);break;
	case 'o':/*OSLONA */indywiduum::set_mode(1);break;
	case 's':/*SILA  */ indywiduum::set_mode(2);break;
	case 'w':/*WIEK  */ indywiduum::set_mode(3);break;
	case 'q':/*QUIT */ return 0;
	}
	}
assert(indywiduum::plot_mode<4);
if(zmieniony)
	caly_ekran(); // Po zmianie trybu trzeba odnowic calosc
assert(indywiduum::plot_mode<4);
return 1;
}


void swiat::init()
{
assert(indywiduum::plot_mode<4);

ziemia[BOKSWIATA/2][BOKSWIATA].init(MAXBASE2,0xff);

fprintf(log,"%ux%u\tWYP_POT=%f\tEFEKT=%f\tMAX_WIEK=%d\tPLOD=%f\tRTG=%f\n",
BOKSWIATA,BOKSWIATA*2,WYPOSAZENIE_POTOMSTWA,EFEKTYWNOSC_AUTOTROFA,(int)(255-MINIMALNY_WIEK),1./NIEPLODNOSC,1./PROMIENIOWANIE);
fprintf(log,"N#\tIND\tTAX\tBIG\tFOS\n");
assert(indywiduum::plot_mode<4);
}

/* NAJWAZNIEJSZE FUNKCJE - GLOWNA IDEA SYMULACJI */

void indywiduum::init(base2 iwfull, unsigned isila)
 // inicjacja nowego indywiduum
{
assert(indywiduum::plot_mode<4);
w.full=iwfull;
sila=isila;
wiek=MINIMALNY_WIEK;
assert(w.full>0 && w.full<=MAXBASE2);
if(!jest_zywy()) // Jesli parametry startowe sa do D
      { clear(); 
	assert(indywiduum::plot_mode<4);
	return; }
ile_ind++;
assert(indywiduum::plot_mode==0);
assert(w.full==iwfull);
liczniki[w.full]++;
assert(liczniki[w.full]!=0);
assert(w.full==iwfull);
assert(indywiduum::plot_mode==0);
if( liczniki[w.full]==1 ) // pierwszy przedstawiciel taxonu
		ile_tax++;   // wiec liczba taxonow wzrasta
assert(indywiduum::plot_mode<4);
if( liczniki[w.full]==11 ) // osiagnal wartosc >10 duzy taxon - rozwojowy
		ile_big_tax++;
assert(w.full>0 && w.full<=MAXBASE2);
assert(indywiduum::plot_mode<4);
}

void indywiduum::kill()
// -----//---- przez swiat
{
assert(indywiduum::plot_mode<4);
assert(w.full>0 && w.full<=MAXBASE2);
ile_ind--;
liczniki[w.full]--;
if( liczniki[w.full]==0 )	//ostatni przedstawiciel tego taxonu
	ile_tax--;
if( liczniki[w.full]==10 ) // osiagnal wartosc <=10 maly taxon - nie rozwojowy
	ile_big_tax--;
assert(w.full>0 && w.full<=MAXBASE2);
w.full=sila=wiek=0;
assert(indywiduum::plot_mode<4);
}

void indywiduum::init(indywiduum& rodzic)
 // inicjacja nowego jako potomka starego
{
assert(indywiduum::plot_mode<4);
w.full=kopioj(rodzic.w.full);
base2 cena=w.w.geba + (base)(~w.w.oslona) + rodzic.sila*WYPOSAZENIE_POTOMSTWA; // Oslona 0 jest najdrozsza
if( rodzic.sila<=cena )  // Nie ma sily na potomka
	{ w.full=0; return; }
rodzic.sila-=cena; 	 // Placi za wyprodukowanie i wyposazenie
assert(rodzic.sila!=0);
init(w.full,rodzic.sila*WYPOSAZENIE_POTOMSTWA);   // prawdziwa inicjacja
}

void indywiduum::kill(indywiduum& zabojca)
// usmiercenie indywiduum przez drugie
{
assert(indywiduum::plot_mode<4);
assert(w.full>0 && w.full<=MAXBASE2);
/* Zabojca dostaje pewna czesc sily */
zabojca.sila+=sila * double(w.w.oslona & zabojca.w.w.geba)/(w.w.oslona);
assert(zabojca.sila!=0);
assert(w.full>0 && w.full<=MAXBASE2);
/* Potem ofiara ginie */
kill();
}

void indywiduum::uplyw_czasu()
// prawo czasu - wszystko sie starzeje
{
assert(indywiduum::plot_mode<4);
assert(w.full>0 && w.full<=MAXBASE2);
wiek++;	       // Normalne starzenie sie
if(w.w.geba==AUTOTROF) //JEST  A U T O T R O F E M
	{
	sila+=100*EFEKTYWNOSC_AUTOTROFA-1;
	assert(sila!=0);
	}
	else
	sila--;// Metaboliczne zurzycie energii
assert(w.full>0 && w.full<=MAXBASE2);
assert(indywiduum::plot_mode<4);
}

base2 indywiduum::kopioj(base2 r)
// kopiuje genotyp z mozliwa mutacja
{
assert(indywiduum::plot_mode<4);
/*
base2 mask=( Random(PROMIENIOWANIE) );
if(mask<=16) // Prowizorka - nieprzenosne jesli base >16bitowe
	{
	mask=0x1<<mask;
	r^=mask;
	}
assert(indywiduum::plot_mode<4);
*/
return r;
}

struct vector2
{ signed char x,y; };

void swiat::krok()
{
assert(indywiduum::plot_mode<4);
static vector2 kierunki[]={{1,1},{-1,-1},{1,-1},{-1,1},
			   {0,1},{ 1, 0},{0,-1},{-1,0} };
long ile= long(BOKSWIATA)*long(2*BOKSWIATA); // ile na krok MonteCarlo
licznik++;
for(long i=0;i<ile;i++) // rob krok MonteCarlo
	{
	unsigned x=Random(BOKSWIATA*2);
	unsigned y=Random(BOKSWIATA);
	if(!ziemia[y][x].jest_zywy()) // jest martwy
		continue;		// obrob nastepnego
	ziemia[y][x].uplyw_czasu();
	if(!ziemia[y][x].jest_zywy())  // nie moze dalej zyc
		{
		ziemia[y][x].kill();
		plot(x,y,0);		// zamaz
		continue;               // obrob nastepnego
		}
	unsigned a=Random(8);
	assert(a<8);
	unsigned x1=(x+kierunki[a].x) % (BOKSWIATA*2);
	unsigned y1=(y+kierunki[a].y) % BOKSWIATA;
	if(!ziemia[y1][x1].jest_zywy()) // wolne miejsce
	   {
	   if(Random(NIEPLODNOSC)==0)     // rozmnazanie
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
assert(indywiduum::plot_mode<4);
}


/*  OGOLNA FUNKCJA MAIN */
/************************/
swiat tenSwiat("coewol.log");
main()
{
printf("CO-EWOLUCJA: program symulujacy kooewolucje wielu gatunkow\n");
printf("POLECENIA: 'g': GEBA 'o':OSLONA 's':SILA 'w':WIEK 'q':QUIT\n");
printf("LICZBA MOZLIWYCH KLONOW=%lu\n",(unsigned long)MAXBASE2);
getchar();
if(sizeof(base)*2!=sizeof(base2))
	{
	fprintf(stderr,"Niewlasciwe rozmiary dla typow bazowych:2*%u!=%u\n",
		sizeof(base),sizeof(base2));
	exit(1);
	}
Randomize();
init_plot(BOKSWIATA*2,BOKSWIATA+10,0,0);
tenSwiat.init();
tenSwiat.wskazniki();
tenSwiat.caly_ekran();
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

/* STATIC ALLOCATION */
unsigned indywiduum::plot_mode=0;// co ma byc wyswietlane
unsigned indywiduum::ile_ind=0;// ile jest zywych indywiduow
unsigned indywiduum::ile_tax=0;// ile taxonow niezerowych
unsigned indywiduum::ile_big_tax=0;// ile taxonow liczniejszych niz 10
unsigned huge indywiduum::liczniki[ MAXBASE2 ];// Liczniki liczebnosci taxonow

