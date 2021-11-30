/**
 * Program symulujący KOEWOLUCJE v1 rewizytowana po 2010 w celu zapewnienia kompilowalności.
 * Skrótowe opisanie działania modelu poniżej. Dokładny opis znajduje się w artykułach.
 * - Każdy osobnik ma swój bitowy wzorzec odżywiania i
 * - bitowy wzorzec strategi osłony/obrony.
 * - Jeśli ATAKOWANY.OSŁONA and ATAKUJĄCY.GĘBA > 0 to
 * - atak zakończył się powodzeniem i
 * - nastąpi transfer energii proporcjonalny do podobieństwa masek
 * Osobniki rozmnażają się kosztem wolnego miejsca i zgromadzonej energii
 * Ruchy własne, ofiara ataku, jak i moment rozmnażania wybierane są
 * losowo, aby nie zaciemniać modelu dodatkowymi parametrami.
 */

//#define  _CRT_SECURE_NO_WARNINGS //2013 - MSVC++ 2012 required this to compile strcpy()

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <csignal>
//#include "dos&unix.h"
#define USES_STDC_RAND
#include "INCLUDE/Randoms.h"
#include "symshell.h"

#undef NULL

#ifndef NULL
#define NULL nullptr
#endif

#ifdef unix
const unsigned MAXBOKSWIATA=300;// Dłuższy bok prostokątnego świata
const unsigned TAX_OUT=256;// Ile taksonów jest wyświetlane na mapie taksonów. Kiedyś 256 mogło się nie mieścić i było 128
#else
const unsigned MAXBOKSWIATA=100; // Świat jest torusem o obwodzie połódnika = BOKSWIATA
                 // Dawne problemy z kompilatorem Borlanda :-)
			     // 75,100 wpada w cykl z rand ?!
const unsigned TAX_OUT=101;
			     // 101 efekt low-migration , artefakt rand (?)
#endif

unsigned long    MAX_ITERATIONS=0xffffffff; // największa liczba iteracji
unsigned int     BOKSWIATA=MAXBOKSWIATA; // FAKTYCZNIE UŻYWANY BOK ŚWIATA

/// Najważniejsze parametry modelu
int              WSP_KATASTROF=10;    // Wykładnik rozkładu katastrof
double EFEKTYWNOSC_AUTOTROFA=0.99; // Jaką część światła używa autotrof

const double     WYPOSAZENIE_POTOMSTWA=0.1; // jaka część siły/energii oddać potomkowi
const unsigned   MINIMALNY_WIEK=155;    // Rodzi się z tym "wiekiem". Do śmierci ma 255-MINIMALNY_WIEK
const unsigned   NIEPLODNOSC=10;       // Prawdopodobieństwo rozmnażania jest 1/NIEPŁODNOŚĆ
const unsigned   PROMIENIOWANIE=160;  // Co ile kopiowanych bitów następuje mutacja

/* ROZMIAR TYPU BASE DECYDUJE O MOŻLIWEJ KOMPLIKACJI ŚWIATA */
/* JEST TYLE MOŻLIWYCH TAKSONÓW ILE WZORCÓW BITOWYCH W base2 */
typedef unsigned char base;   // musi być bez znaku
typedef unsigned short base2; // musi mieścić 2 * base
const int   MAXINT=0x1fffffffL;
const base2 MAXBASE2=(base2)0xffffffffL;
const base  MAXBASE =(base)MAXBASE2;
const base  AUTOTROF=MAXBASE;// wzór bitowy autotrofa, którego sam świat żywi (nie musi polować)

// Czysto techniczne
unsigned int     textY=(BOKSWIATA>TAX_OUT?BOKSWIATA:TAX_OUT);// POCZĄTEK LINII STATUSU
unsigned int     VisRand=0;        // Czy pokazywać tło randomizer-a (???2013)
unsigned int     LogRatio=10;      // Co ile kroków zapisywać do logu
char             LogName[128]="coewo.log";

/** Cześć niezależna od platformy
 **********************************/
int parse_options(int argc,const char* argv[])
{
const char* pom;
for(int i=1;i<argc;i++)
    {
    if( *argv[i]=='-' ) // Opcja X11 lub symshell-a
		continue;       // tu ignorujemy
    if((pom=strstr(argv[i],"BOK="))!=NULL) //Nie NULL, czyli jest taka opcja
	{
	BOKSWIATA=atol(pom+4);
    if(BOKSWIATA<2 || BOKSWIATA>MAXBOKSWIATA)
		{
	    fprintf(stderr,"Niewłaściwy BOK ==  %u\n",BOKSWIATA);
		return 0;
		}
	}
    else
    if((pom=strstr(argv[i],"BUM="))!=NULL) //Nie NULL, czyli jest
    {
    WSP_KATASTROF=atol(pom+4);
    if(WSP_KATASTROF<=0)
	{
	fprintf(stderr,"Ujemny wykładnik katastrof. Katastrofy wyłączone\n");
	}
    }
    else
    if((pom=strstr(argv[i],"AUTO="))!=NULL) //Nie NULL, czyli jest
    {
    EFEKTYWNOSC_AUTOTROFA=atof(pom+5);
    if(EFEKTYWNOSC_AUTOTROFA<=0)
		{
        fprintf(stderr,"Efektywność autotrofa musi być w zakresie 0..0.99\n");
		return 0;
        }
    printf("Ilość iteracji ustawiona na %lu\n",MAX_ITERATIONS);
    }
    else
    if((pom=strstr(argv[i],"MAX="))!=NULL) //Nie NULL, czyli jest
    {
    MAX_ITERATIONS=atol(pom+4);
    if(MAX_ITERATIONS<=0)
		{
	fprintf(stderr,"Liczba iteracji nie może być <=0\n");
		return 0;
        }
    printf("Ilość iteracji ustawiona na %lu\n",MAX_ITERATIONS);
    }
    else
    if((pom=strstr(argv[i],"LOG="))!=NULL) //Nie NULL, czyli jest
    {
    LogRatio=atol(pom+4);
    if(LogRatio<=0)
		{
	fprintf(stderr,"Częstość zapisu nie może być <=0\n");
		return 0;
        }
    }
    else
    if((pom=strstr(argv[i],"PLIK="))!=NULL) //Nie NULL, czyli jest
    {
    strcpy(LogName,pom+5);
    }
	else /* Ostatecznie wychodzi, że nie ma takiej opcji */
	{
	fprintf(stderr,"Błędna opcja %s\n",argv[i]);
	fprintf(stderr,"MOŻLIWE TO:\n");
    fprintf(stderr,"BOK=NN BUM=NN LOG=N \n");
	return 0;
	}
    }
return 1;
}

union wzor
{
base2 	_full;	// pełny wzór bitowy "taksonu"
struct{
	base geba;	// bitowy wzorzec trybu odżywiania
	base oslona;// bitowy wzorzec sposobu ochrony
	} w;
};

//unsigned huge liczniki[ MAXBASE2+1 ];// Liczniki liczebności taksonów

class indywiduum
{
friend class swiat; // Bezpośredni dostęp klasy świata do tych pól prywatnych
unsigned char wiek; // Ile razy był u sterowania? Po przewinięciu zmiennej umiera.
unsigned sila;      // zgromadzona energia
wzor w;		        // wzór bitowy taksonu
/* STATIC FIELDS */
static unsigned max;        //Jaki jest największy takson
static unsigned max_change; //i czy ostatnio maksimum się zmieniło?
static unsigned plot_mode; 	//Co ma byc wyświetlane?
static unsigned ile_ind;    //Ile jest żywych indywiduów?
static unsigned ile_tax;    //Ile taksonów niezerowych?
static unsigned ile_big_tax;//Ile taksonów liczniejszych niż 10?
static unsigned liczniki[ (long)MAXBASE2+1 ];// Liczniki liczebności taksonów
/* METHODS */
public:
static void set_mode(int p) { if(p>=0 && p<=4) plot_mode=p; }
static unsigned char  tax_val(base2);    // TODO Dlaczego nie inline?
static inline   void  tax(base2);        // rejestracja zmiany wartości taksonu na ekranie
static inline   base2 kopioj(base2 r);   // kopiuje genotyp z możliwą mutacją

int  jest_zywy(){ return (w.w.oslona!=0 && w.w.geba!=0 && sila!=0 && wiek!=0); }
inline void init(base2 iwfull, unsigned isila); // inicjacja nowego indywiduum
inline void init(indywiduum& rodzic);   // inicjacja nowego jako potomka starego
inline void kill();		                // śmierć indywiduum
inline void kill(indywiduum& zabojca);  // uśmiercenie indywiduum przez 'zabójcę' (przepływy energii)
inline void uplyw_czasu();	            // oddziaływanie czasu
inline void plot(int x,int y);          // wyświetlanie zgodnie z trybem 'plot_mode'
/*KONSTRUKTOR*/
    indywiduum(){w._full=wiek=sila=0;}
    void clear(){w._full=wiek=sila=0;}
};

class swiat
{
indywiduum ziemia[MAXBOKSWIATA][MAXBOKSWIATA*2];// podłoże dla indywiduów
unsigned long licznik;			  // licznik kroków symulacji
FILE* log;
/* METHODS */
public:
static inline void torus(int& x, int& y)
{
if(x<0) x=(BOKSWIATA*2)-x;
if(y<0) y=BOKSWIATA-y;
x%=BOKSWIATA*2;y%=BOKSWIATA;
}
inline void clearPosition(int x, int y);
inline void clearLine(int xxp, int yyp, int n);
void krater(int x, int y, int r); // Robi okrągłą dziurę w obszarze symulacji
void wskazniki(); // wyświetlenie nowych wartości wskaźników
void mapa_taxonow();//wyświetlenie mapy taksonów
void caly_ekran();// Odnowienie całego ekranu np. po zmianie trybu wyświetlania.
void init();	  // stan startowy symulacji
void kataklizm(); // wygenerowanie katastrofy
void krok();	  // kolejny krok symulacji
int  sterowanie();// Ewentualne sterowanie z zewnątrz.
//void dump();	  // zrzucenie stanu do logu
/* CONSTRUCTOR & DESTRUCTOR */
    swiat(char* logname);
    ~swiat(){ fclose(log); }
};

/** IMPLEMENTACJA CZĘŚCI NIEZALEŻNEJ OD PLATFORMY
 *************************************************/
void swiat::clearPosition(int x, int y)
{
    torus(x, y);
    if(!ziemia[y][x].jest_zywy()) return;
    ziemia[y][x].kill();
    ziemia[y][x].plot(x,y);
}

void swiat::clearLine(int xxp, int yyp, int n)
{
for(int i=xxp;i<xxp+n;i++)
    clearPosition(i, yyp);
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

/// wyświetlanie zgodnie z trybem
void indywiduum::plot(int x,int y)
{
int pom;
switch(plot_mode){
 case 0:pom=w.w.geba; break;
 case 1:pom=w.w.oslona;	break;
 case 2:if(sila) pom=sila/16+1;
	   else pom=0;
	break;
 case 3:pom=wiek;	break;
 case 4:/* hidden */    return;
 default:
	fprintf(stderr,"INTERNAL: plot_mode=%d >3\n",plot_mode);
	abort();
 }
::plot(x,y,(pom<256?pom:255));
}

unsigned char indywiduum::tax_val(base2 f)
{
	if(liczniki[f]==0) return 0;
	return (unsigned char)(( log10(double(liczniki[f]))/log10(double(max)) ) * 255);
}

inline void indywiduum::tax(base2 f)
{
unsigned x=(f&0xff00)>>8;
unsigned y=f&0x00ff;
//assert( y>0 && y<=MAXBASE);
//assert( x>0 && x<=MAXBASE);
unsigned char c=indywiduum::tax_val(f);// gęby na x-ach
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
     c=indywiduum::tax_val(x*256+y);// gęby na x-ach
     if(TAX_OUT==128)
	  { y/=2; }
     plot(x,y,c);
     }
indywiduum::max_change=0;// Obraz już aktualny
}

void swiat::caly_ekran()
{
int a,b;
if(indywiduum::plot_mode==4)
  mapa_taxonow();
  else
  {
  indywiduum::ile_ind=0;
  for(a=0;a<BOKSWIATA;a++) // po kolejnych wierszach, czyli y
	for(b=0;b<BOKSWIATA*2;b++)// po kolumnach w wierszu, czyli x
		{
		ziemia[a][b].plot(b,a);
		if(ziemia[a][b].jest_zywy())
			indywiduum::ile_ind++;
		}
  }
}

char nazwy[]="GOSWT";// dodane ponownie w 2021 roku, bi gdzieś zginęło :-)

void swiat::wskazniki()
{
/* LOSOWANIE SKAMIENIAŁOŚCI */
unsigned x=RANDOM(BOKSWIATA*2);
unsigned y=RANDOM(BOKSWIATA);
printc(0,textY,0,128,"%c [%lu] IND:%lu TAX:%lu BIG:%lu ",
	char(nazwy[indywiduum::plot_mode]),
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
    /* Ważne jest, żeby specjalizacja pokarmowa była istotniejsza od osłony */
       (unsigned)(( (unsigned)ziemia[y][x].w.w.geba*((unsigned)MAXBASE+1) )+ziemia[y][x].w.w.oslona)
       );
if(indywiduum::plot_mode==4 && indywiduum::max_change)
	mapa_taxonow();
}

void swiat::init()
{
ziemia[BOKSWIATA/2][BOKSWIATA].init(MAXBASE2,0xff);

fprintf(log,"%ux%u\tWYP_POT=%f\tEFEKT=%f\tMAX_WIEK=%d\tPLOD=%f\tRTG=%f\tBUM=0.5^%d\n",
BOKSWIATA,BOKSWIATA*2,WYPOSAZENIE_POTOMSTWA,EFEKTYWNOSC_AUTOTROFA,(int)(255-MINIMALNY_WIEK),1./NIEPLODNOSC,1./PROMIENIOWANIE,WSP_KATASTROF);
fprintf(log,"N#\tIND\tTAX\tBIG\tFOS\n");
}


/** NAJWAŻNIEJSZE FUNKCJE - IMPLEMENTACJA GŁÓWNEJ IDEI SYMULACJI
 * **************************************************************/

/// inicjacja nowego indywiduum
void indywiduum::init(base2 iwfull, unsigned isila)
{
w._full=iwfull;
sila=isila;
wiek=MINIMALNY_WIEK;
assert(w._full>0);
// Sprawdzenie, czy osłona nie jest za dobra i czy inne parametry są OK
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
	tax(iwfull);            // wyświetlanie taksonu
if( liczniki[iwfull]==1 )   // pierwszy przedstawiciel taksonu
		ile_tax++;          // więc liczba taksonów wzrasta
if( liczniki[iwfull]==11 )  // Osiągnał wartość >10, czyli to duży takson - rozwojowy
		ile_big_tax++;
}

/// Zabicie indywiduum przez świat
void indywiduum::kill()
{
assert(w.w.oslona>0);
assert( w._full>0 );
ile_ind--;
base2 wfull=w.w.geba*256+w.w.oslona;
liczniki[wfull]--;
if(plot_mode==4)
	tax(wfull);//wyświetlanie taksonu
if( liczniki[wfull]==0 )	// To ostatni przedstawiciel tego taksonu
	ile_tax--;
if( liczniki[wfull]==10 )   // Osiągnął wartość <=10, czyli to mały takson
	ile_big_tax--;
assert(w._full>0 );
w._full=sila=wiek=0;
}

/// Inicjacja nowego indywiduum jako potomka istniejącego
void indywiduum::init(indywiduum& rodzic)
{
w._full=kopioj(rodzic.w._full);
unsigned uposazenie=unsigned(rodzic.sila*WYPOSAZENIE_POTOMSTWA);
unsigned cena=w.w.geba + (base)(~w.w.oslona) + uposazenie; // Osłona 0 jest najdroższa
if( rodzic.sila<=cena ) // Nie ma siły na potomka
	{ w._full=0; return; }
rodzic.sila-=cena; 	        // Płaci za wyprodukowanie i wyposażenie
assert(rodzic.sila!=0);
init(w._full,uposazenie);   // realna inicjacja przygotowanego
}

/// Uśmiercenie indywiduum przez drugie z przepływem energii
void indywiduum::kill(indywiduum& zabojca)
{
if(zabojca.sila==0) return; //niezdolny zabijać
assert(w.w.oslona>0);
assert(w._full>0);
/* Zabójca dostaje pewna część siły */
/* proporcjonalną do tego ile bitów osłony ofiary pasuje do jego maski ataku */
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

/// prawo czasu - wszystko sie starzeje
void indywiduum::uplyw_czasu()
{
assert(w.w.oslona>0);
assert(w._full>0);
wiek++;	       // Normalne starzenie się
if(w.w.geba==AUTOTROF) //GDY JEST  A U T O T R O F E M ...
	{
    sila+=unsigned(100*EFEKTYWNOSC_AUTOTROFA-1);// bez rzutu gcc się czepiało, a tak ma byc!
	assert(sila!=0);
	}
	else
	sila--;// Metaboliczne zużycie energii
assert(w._full>0);
}

/// kopiowanie genotypu z możliwą mutacją
base2 indywiduum::kopioj(base2 r)
{
base2 mask=( RANDOM(PROMIENIOWANIE) );
if(mask<=16) // Prowizorka - nieprzenośne jeśli base > 16 bitowe ??? TODO: TAK BYŁO DAWNO TEMU!!!
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
long ile= long(BOKSWIATA)*long(2*BOKSWIATA); // ile losowań indywiduów na krok MonteCarlo
licznik++;
for(long i=0;i<ile;i++) // rob krok MonteCarlo
	{
	unsigned x=RANDOM(BOKSWIATA*2);
	unsigned y=RANDOM(BOKSWIATA);
    if(VisRand)
		plot(x,y,255+128);//dodano 26.09.2013 (ale co?)
	if(!ziemia[y][x].jest_zywy()) // jest martwy
		continue;		// obrób następnego
	ziemia[y][x].uplyw_czasu();
	if(!ziemia[y][x].jest_zywy())  // nie może dalej zyc
		{
		ziemia[y][x].kill();
		plot(x,y,0);		// zamaż punkt na ekranie
		continue;           // idź obrób następnego!
		}
	unsigned a=RANDOM(8);// ruch w jednym z ośmiu kierunków
	assert(a<8);
	unsigned x1=(x+kierunki[a].x) % (BOKSWIATA*2);
	unsigned y1=(y+kierunki[a].y) % BOKSWIATA;
	if(!ziemia[y1][x1].jest_zywy()) // czy wolne miejsce?
	   {
	   if(RANDOM(NIEPLODNOSC)==0)   // rozmnażanie
		{
		ziemia[y1][x1].init(ziemia[y][x]);
		}
		else			            // przemieszczenie
		{
		ziemia[y1][x1]=ziemia[y][x];
		ziemia[y][x].clear();
		}
	   ziemia[y1][x1].plot(x1,y1);
	   ziemia[y][x].plot(x,y);
	   }
	   else  // W wylosowanym polu jest jakiś żywy. Jest próba zjedzenia, jeśli aktywny nie jest autotrofem
	   if( ziemia[y][x].w.w.geba!=AUTOTROF &&
	      (ziemia[y1][x1].w.w.oslona & ziemia[y][x].w.w.geba) != 0)
		   {
		   ziemia[y1][x1].kill(ziemia[y][x]);
		   plot(x1,y1,0);
		   }
	}
kataklizm(); // jeden, choćby malutki kataklizm na krok monte-carlo
}

double poison(int n); // generuje rozkład para-poison w zakresie 0..1 o 'n' stopniach

void  swiat::kataklizm(void)
{
double power;
int x,y;
if(WSP_KATASTROF<0)
   return; //spontaniczne katastrofy wyłączone
x=RANDOM(BOKSWIATA*2);
y=RANDOM(BOKSWIATA);
power=poison(WSP_KATASTROF);
assert(power>=0 && power<=1);
krater(x, y, power * BOKSWIATA);
}


/**  INTERFEJSY I OGÓLNA FUNKCJA MAIN
 ***************************************/

//void (*signal (int sig, void (*func)(...)))(...);
void MySignalHook(int par)
{
fprintf(stderr," Received signal %d\n",par);
fflush(NULL);
close_plot();
exit(par);
}

void install_signal_hooks()
{
#ifndef __NONCOMPATIBLE__
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
//signal(  ,MySignalHook());//MORE?
}

int swiat::sterowanie()
{
    int zmieniony=0;
    if(licznik>=MAX_ITERATIONS) return 0;
    while(input_ready())
    {
        zmieniony=1; // putchar(7);???
        switch(get_char()){
            case 'g':/* GĘBA */ indywiduum::set_mode(0);break;
            case 'o':/*OSŁONA*/ indywiduum::set_mode(1);break;
            case 's':/*SIŁA */  indywiduum::set_mode(2);break;
            case 'w':/*WIEK */  indywiduum::set_mode(3);break;
            case 't':/*TAXs*/   indywiduum::set_mode(4);break;
            case '+':VisRand=1;break;
            case '-':VisRand=0;break;
            case 'b':{
                int x=RANDOM(BOKSWIATA*2);
                assert(x>=0 && x<BOKSWIATA*2);
                int y=RANDOM(BOKSWIATA);
                assert(y>=0 && y<BOKSWIATA);
                double power=DRAND();
                assert(power>=0 && power<=1);
                krater(x, y, int(power * BOKSWIATA));
            }break;
            case 'f':fflush(log);break;
            case -1:
            case 'q':/*QUIT */ return 0;
        }
    }
    if(zmieniony)
        caly_ekran(); // Po zmianie trybu trzeba odnowić całość ekranu
    return 1;
}

int main(int argc,const char* argv[])
{
shell_setup("CO-EVOLUTION",argc,argv);
printf("KOEWOLUCJA: program symulujący kooewolucję wielu gatunków\n");
printf("POLECENIA: 'g':GĘBA 'o':OSŁONA 's':SIŁA 'w':WIEK 't':TAKSONY\n"
              "'b':BOLID   +-:SRC.CLEANING 'f':FLUSH LOG 'q':QUIT\n");
printf("LICZBA MOŻLIWYCH KLONÓW=%lu MAXINT=%d\n",(unsigned long)MAXBASE2,MAXINT);

if(!parse_options(argc,argv))
        exit(1);

if(sizeof(base)*2!=sizeof(base2))//static assert wtedy nie istniało
	{
	fprintf(stderr,"Niewłaściwe rozmiary dla typów bazowych:2*%lu!=%lu\n",
		(unsigned long) sizeof(base),(unsigned long) sizeof(base2));
	exit(1);
	}
swiat& tenSwiat=*new swiat(LogName);
if(&tenSwiat==NULL)
    {
    fprintf(stderr,"Brak pamięci!\n");
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
// generuje rozkład para-poison w zakresie 0..1 o 'n' stopniach
{
double pom=1;
for(int i=1;i<=n;i++)
	pom*=DRAND(); // Mnożenie przez wartość od 0..1
assert(pom>=0 && pom<=1);
return pom;
}


/* STATICS */
unsigned indywiduum::max=0;//jaki jest największy takson
unsigned indywiduum::max_change=0;//... i czy ostatnio maksimum zmienione
unsigned indywiduum::plot_mode=0;// co ma byc wyświetlane
unsigned indywiduum::ile_ind=0;// ile jest żywych indywiduów
unsigned indywiduum::ile_tax=0;// ile taksonów niezerowych
unsigned indywiduum::ile_big_tax=0;// ile taksonów liczniejszych niż 10
unsigned indywiduum::liczniki[ (long)(MAXBASE2+1) ];// Liczniki liczebności taksonów
				                    // W DOS'ie sprawiał kłopot, stąd wprowadzono cast

/** KRATER - TRANSLATED FROM BASIC CODE IMPLEMENTED ELLIPSE DRAWING
   BASED ON BRESENHAM ALGORITHM                                      */
void swiat::krater(int x, int y, int r) // Robi dziurę w obszarze symulacji
{
if(r<=1)
	{ clearPosition(x, y); return; }
//ClearEllipse(x,y,r,r);// expanded below
//if(r>5)
//    putchar(7); //??
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
        clearLine(xxp, yyp, n);
	yyp= ys+yi;
        clearLine(xxp, yyp, n);
	}
	else // Uzupełnienie linii o punkty brzeżne
	{
	xxp=xs+xi;  yyp= ys+yi;
        clearPosition(xxp, yyp);
	xxp=xs-xi;  yyp= ys-yi;
        clearPosition(xxp, yyp);
	xxp=xs+xi;  yyp= ys-yi;
        clearPosition(xxp, yyp);
	xxp=xs-xi;  yyp= ys+yi;
        clearPosition(xxp, yyp);
	}
yyp=yi; // zapamiętaj do porównania
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

_1240: 			/* krok ukośny */
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
