/**
  * COEVOLUTIONS v1 simulator revisited after 2010 to ensure compilability.
  * Brief description of the operation of the model below.
  * (A detailed description can be found in the papers)
  * - Each individual has its own nutrition and bit pattern
  * - bit pattern for cover / defense strategy.
  * - If ATTACKED.OBRONA and ATTACKER.GĘBA > 0 then
  * - the attack was successful and
  * - there will be an energy transfer proportional to the similarity of the masks
  * Individuals reproduce at the expense of free space and accumulated energy
  * Own movements, the victim of the attack and the moment of reproduction are selected
  * randomly so as not to obscure the model with additional parameters.
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
const unsigned MAX_WORLD_SIDE=300;// Longer side of the rectangular world
const unsigned TAX_OUT=256;// How many taxa are displayed on the taxa map.
                           // Once upon a time, 256 might not have been fit, and it was 128
#else
const unsigned MAX_WORLD_SIDE = 100; // The world is a torus with the circumference of the meridian = BOX FLOWER
                    // Old problems with the Borland compiler :-)
                    // 75,100 fall into a cycle with rand?!
const unsigned TAX_OUT = 101;
                    // 101 low-migration effect, rand (?) Artifact
#endif

unsigned long    MAX_ITERATIONS=0xffffffff; // the greatest number of iterations
unsigned int     WORLD_SIDE=MAX_WORLD_SIDE; // ACTUALLY USED SIDE OF THE WORLD

/// The most important parameters of the model
int              DISASTER_EXP=10;  // Exponent of disaster size distribution
double AUTOTROPH_EFFICIENCY=0.99;  // The efficiency of autotrophy, which is
                                   // how much light the autotrophs use
const double     OFFSPRING_DOWRY=0.1;// what part of the strength/energy
                                     // should be given to the offspring - the dowry of the offspring
const unsigned   MINIMAL_AGE=155;    // It is born with this "age". He has a 255-MINIMUM_Age until his death
const unsigned   INFERTILITY=10;     // The probability of breeding is 1 / INFERTILITY
const unsigned   RADIATION=160; // Every how many copied bits there is a mutation,

/* SIZE OF TYPE 'base' DECIDES THE POSSIBLE WORLD COMPLICATION */
/* THERE ARE AS MANY POSSIBLE TAXA AS MANY BIT PATTERNS IN base2 */
typedef unsigned char base;   // must be unsigned
typedef unsigned short base2; // must fit 2 * base
const int   MAXINT=0x1fffffffL;
const base2 MAX_BASE2=(base2)0xffffffffL;
const base  MAX_BASE =(base)MAX_BASE2;
const base  AUTOTROPH=MAX_BASE;// the bit pattern of the autotrophy that the world itself feeds (does not have to hunt)

// Purely technical
unsigned int     textY=(WORLD_SIDE>TAX_OUT?WORLD_SIDE:TAX_OUT);// BEGINNING OF STATUS LINE
unsigned int     VisRand=0;        // Do you want to show the background of the randomizer (??? 2013)
unsigned int     LogRatio=10;      // How many steps do we need to log?
char             LogName[128]="coewo.log";

/** Platform independent part
 **********************************/

int parse_options(int argc,const char* argv[])
{
const char* pom;
for(int i=1;i<argc;i++)
    {
    if( *argv[i]=='-' ) // X11 or symshell option
		continue;       // we ignore it here
    if((pom=strstr(argv[i],"SIDE="))!=NULL) // Not NULL, which is such an option
	{
	WORLD_SIDE=atol(pom+4);
    if(WORLD_SIDE<2 || WORLD_SIDE>MAX_WORLD_SIDE)
		{
	    fprintf(stderr,"Invalid SIDE ==  %u\n",WORLD_SIDE);
		return 0;
		}
	}
    else
    if((pom=strstr(argv[i],"BOLID="))!=NULL) // Not NULL, it means it exists
    {
    DISASTER_EXP=atol(pom+4);
    if(DISASTER_EXP<=0)
	{
	fprintf(stderr,"Negative exponent of disasters. Disasters disabled\n");
	}
    }
    else
    if((pom=strstr(argv[i],"AUTO="))!=NULL) // Not NULL, it means it exists
    {
    AUTOTROPH_EFFICIENCY=atof(pom+5);
    if(AUTOTROPH_EFFICIENCY<=0)
		{
        fprintf(stderr,"The autotroph's efficiency must be in the range of 0..0.99\n");
		return 0;
        }
    printf("Ilość iteracji ustawiona na %lu\n",MAX_ITERATIONS);
    }
    else
    if((pom=strstr(argv[i],"MAX="))!=NULL) // Not NULL, it means it exists
    {
    MAX_ITERATIONS=atol(pom+4);
    if(MAX_ITERATIONS<=0)
		{
	    fprintf(stderr,"The maximum number of iterations cannot be <= 0\n");
		return 0;
        }
    printf("Maximum number of iterations set to %lu\n",MAX_ITERATIONS);
    }
    else
    if((pom=strstr(argv[i],"LOG="))!=NULL) // Not NULL, it means it exists
    {
    LogRatio=atol(pom+4);
    if(LogRatio<=0)
		{
	    fprintf(stderr,"The log rate cannot be <= 0\n");
		return 0;
        }
    }
    else
    if((pom=strstr(argv[i],"FILE="))!=NULL) // Not NULL, it means it exists
    {
    strcpy(LogName,pom+5);
    }
	else /* Ultimately it turns out that there is no such option */
	{
	fprintf(stderr,"Wrong option %s\n",argv[i]);
	fprintf(stderr,"POSSIBLE OPTIONS ARE:\n");
    fprintf(stderr,"SIDE=NN AUTO=.XX BOLID=NN MAX=NNNN LOG=N FILE=S\n");
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

//unsigned huge liczniki[ MAX_BASE2+1 ];// Liczniki liczebności taksonów

class indywiduum
{
friend class swiat; // Direct access for a class world to these private fields
unsigned char wiek; // How many times has he been at the CPU control? After rewinding the variable, it dies.
unsigned sila;      // accumulated energy
wzor w;		        // taxon bit pattern
/* STATIC FIELDS */
static unsigned max;        //How big is the largest taxon?
static unsigned max_change; //Has the maximum changed recently?
static unsigned plot_mode; 	//What is to be displayed?
static unsigned ile_ind;    //How many live individuals are there?
static unsigned ile_tax;    //How many non-zero taxa?
static unsigned ile_big_tax;//How many taxa are more than 10 individuals?
static unsigned liczniki[ (long)MAX_BASE2+1 ];// Taxa counters
/* METHODS */
public:
static void set_mode(int p) { if(p>=0 && p<=4) plot_mode=p; }
static unsigned char  tax_val(base2);    // TODO Why not inline?
static inline   void  tax(base2);        // registration of the change in the taxon value on the screen
static inline   base2 kopioj(base2 r);   // copies the genotype with a possible mutation

int  jest_zywy(){ return (w.w.oslona!=0 && w.w.geba!=0 && sila!=0 && wiek!=0); }
inline void init(base2 iwfull, unsigned isila); // initiation of a new individual
inline void init(indywiduum& rodzic);   // the initiation of the new as a descendant of the old one
inline void kill();		                // death of the individual
inline void kill(indywiduum& zabojca);  // killing the individual by the 'killer' (energy flows)
inline void uplyw_czasu();	            // the impact of the flowing time
inline void plot(int x,int y);          // display individual according to the 'plot_mode' mode
/*CONSTRUCTOR*/
    indywiduum(){w._full=wiek=sila=0;}
    void clear(){w._full=wiek=sila=0;} // Something like constructor
};

class swiat
{
indywiduum ziemia[MAX_WORLD_SIDE][MAX_WORLD_SIDE*2];// substrate for individuals
unsigned long licznik;			  // simulation step counter
FILE* log;
/* METHODS */
public:
static inline void torus(int& x, int& y)
{
if(x<0) x=(WORLD_SIDE*2)-x;
if(y<0) y=WORLD_SIDE-y;
x%=WORLD_SIDE*2;y%=WORLD_SIDE;
}
inline void clearPosition(int x, int y);
inline void clearLine(int xxp, int yyp, int n);
void krater(int x, int y, int r); // Makes a circular hole in the simulation area
void wskazniki(); // display of the current values of indicators
void mapa_taxonow();// display a map of the current taxa
void caly_ekran();// Restoration of the entire screen, e.g. after changing the display mode.
void init();	  // the initial state of the simulation
void kataklizm(); // generating a disaster of a specific size
void krok();	  // next step of simulation
int  sterowanie();// Possible external control (keyboard)
//void dump();	  // dump the simulation state to a file
/* CONSTRUCTOR & DESTRUCTOR */
    swiat(char* logname);
    ~swiat(){ fclose(log); }
};

/** PLATFORM INDEPENDENT PART IMPLEMENTATION
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

/// It displays according to the mode
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
  for(a=0;a<WORLD_SIDE;a++) // Iterating over successive lines, i.e. after 'y'
	for(b=0;b<WORLD_SIDE*2;b++) // Iterating over the columns in a row, i.e. x
		{
		ziemia[a][b].plot(b,a);
		if(ziemia[a][b].jest_zywy())
			indywiduum::ile_ind++;
		}
  }
}

char nazwy[]="GOSWT";// added again in 2021 because it got lost somewhere :-)

void swiat::wskazniki()
{
/* Draw a fossil! */
unsigned x=RANDOM(WORLD_SIDE*2);
unsigned y=RANDOM(WORLD_SIDE);
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
       (unsigned)(( (unsigned)ziemia[y][x].w.w.geba*((unsigned)MAX_BASE+1) )+ziemia[y][x].w.w.oslona)
       );
if(indywiduum::plot_mode==4 && indywiduum::max_change)
	mapa_taxonow();
}

void swiat::init()
{
ziemia[WORLD_SIDE/2][WORLD_SIDE].init(MAX_BASE2,0xff);

fprintf(log,"%ux%u\tWYP_POT=%f\tEFEKT=%f\tMAX_WIEK=%d\tPLOD=%f\tRTG=%f\tBUM=0.5^%d\n",
WORLD_SIDE,WORLD_SIDE*2,OFFSPRING_DOWRY,AUTOTROPH_EFFICIENCY,(int)(255-MINIMAL_AGE),1./INFERTILITY,1./RADIATION,DISASTER_EXP);
fprintf(log,"N#\tIND\tTAX\tBIG\tFOS\n");
}


/** THE MOST IMPORTANT FUNCTIONS - IMPLEMENTATION OF THE MAIN SIMULATION IDEA
 * ***************************************************************************/

/// inicjacja nowego indywiduum
void indywiduum::init(base2 iwfull, unsigned isila)
{
w._full=iwfull;
sila=isila;
wiek=MINIMAL_AGE;
assert(w._full>0);
// Check if the armor is too good and if the other parameters are OK
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
	tax(iwfull);            // displaying a taxon
if( liczniki[iwfull]==1 )   // the first representative of the taxon
		ile_tax++;          // so the number of taxa increases
if( liczniki[iwfull]==11 )  // It reached a value of> 10, which is a large taxon - promising
		ile_big_tax++;
}

/// Killing the individual by the world
void indywiduum::kill()
{
assert(w.w.oslona>0);
assert( w._full>0 );
ile_ind--;
base2 wfull=w.w.geba*256+w.w.oslona;
liczniki[wfull]--;
if(plot_mode==4)
	tax(wfull);             // displaying a taxon
if( liczniki[wfull]==0 )	// This is the last representative of this taxon
	ile_tax--;              // Thus, the number of taxa is decreasing
if( liczniki[wfull]==10 )   // It reached the value of <= 10, so it is already a small taxon
	ile_big_tax--;
assert(w._full>0 );
w._full=sila=wiek=0;
}

/// The initiation of a new individual as a descendant of an existing one
void indywiduum::init(indywiduum& rodzic)
{
w._full=kopioj(rodzic.w._full);
unsigned uposazenie=unsigned(rodzic.sila*OFFSPRING_DOWRY);
unsigned cena=w.w.geba + (base)(~w.w.oslona) + uposazenie; // Shield 0 is the most expensive
if( rodzic.sila<=cena ) // There is no energy to have a descendant
	{ w._full=0; return; }
rodzic.sila-=cena; 	        // He pays for production and equipment
assert(rodzic.sila!=0);
init(w._full,uposazenie);   // The real initiation of the prepared descendant
}

/// Killing the individual by the other with the flow of energy
void indywiduum::kill(indywiduum& zabojca)
{
if(zabojca.sila==0) return; // unable to kill
assert(w.w.oslona>0);
assert(w._full>0);
/* The killer gets a certain amount of the victim's strength */
/* proportional to the number of bits of the victim's shield matches his attack mask */
if(w.w.oslona!=0)
   if(zabojca.w.w.geba!=0)
       zabojca.sila+=unsigned(sila *
    double(w.w.oslona & zabojca.w.w.geba)/(zabojca.w.w.geba)*
    double(w.w.oslona & zabojca.w.w.geba)/(w.w.oslona)
    );
assert(zabojca.sila!=0);
assert(w._full>0);
/* Then the victim dies */
kill();
}

/// the law of time - everything grows old
void indywiduum::uplyw_czasu()
{
assert(w.w.oslona>0);
assert(w._full>0);
wiek++;	       // Normal aging
if(w.w.geba==AUTOTROPH) //IF IS AN  A U T O T R O P H ...
	{
    sila+=unsigned(100*AUTOTROPH_EFFICIENCY-1);// without a throw, gcc was clinging, and that's how it should be!
	assert(sila!=0);
	}
	else
	sila--;// Metabolic energy consumption
assert(w._full>0);
}

/// copying the genotype with a possible mutation
base2 indywiduum::kopioj(base2 r)
{
base2 mask=( RANDOM(RADIATION) );
if(mask<=16) // A makeshift - not transferable if base> 16 bit ??? TODO: THIS WAS LONG AGO !!!
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
long ile= long(WORLD_SIDE)*long(2*WORLD_SIDE); // How many individual draws per MonteCarlo step?
licznik++;
for(long i=0;i<ile;i++) // Take a MonteCarlo step
	{
	unsigned x=RANDOM(WORLD_SIDE*2);
	unsigned y=RANDOM(WORLD_SIDE);
    if(VisRand)
		plot(x,y,255+128);// added 09/26/2013 (but what?)
	if(!ziemia[y][x].jest_zywy()) // this one is dead!
		continue;		// processing of the next one!
	ziemia[y][x].uplyw_czasu();
	if(!ziemia[y][x].jest_zywy())  // This one cannot live anymore
		{
		ziemia[y][x].kill();
		plot(x,y,0);		// blur the point on the screen
		continue;           // go work on the next one!
		}
	unsigned a=RANDOM(8);// movement in one of eight directions
	assert(a<8);
	unsigned x1=(x+kierunki[a].x) % (WORLD_SIDE*2);
	unsigned y1=(y+kierunki[a].y) % WORLD_SIDE;
	if(!ziemia[y1][x1].jest_zywy()) // is this a free seat?
	   {
	   if(RANDOM(INFERTILITY)==0)   // proliferation
		{
		ziemia[y1][x1].init(ziemia[y][x]);
		}
		else			            // displacement
		{
		ziemia[y1][x1]=ziemia[y][x];
		ziemia[y][x].clear();
		}
	   ziemia[y1][x1].plot(x1,y1);
	   ziemia[y][x].plot(x,y);
	   }
	   else  // There is one alive in the randomly selected cell.
	   if( ziemia[y][x].w.w.geba!=AUTOTROPH && //An attempt is made to eat if the active one is not an autotroph
	      (ziemia[y1][x1].w.w.oslona & ziemia[y][x].w.w.geba) != 0)
		   {
		   ziemia[y1][x1].kill(ziemia[y][x]);
		   plot(x1,y1,0);
		   }
	}
kataklizm(); // one, even a tiny cataclysm at the Monte-Carlo step
}

double poison(int n); // generate a para-poison distribution in the range 0..1 with 'n' degrees

void  swiat::kataklizm(void)
{
double power;
int x,y;
if(DISASTER_EXP<0)
   return; //spontaneous disasters disabled
x=RANDOM(WORLD_SIDE*2);
y=RANDOM(WORLD_SIDE);
power=poison(DISASTER_EXP);
assert(power>=0 && power<=1);
krater(x, y, power * WORLD_SIDE);
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
            case 'g':/* MUG  */ indywiduum::set_mode(0);break;
            case 'o':/*SHIELD*/ indywiduum::set_mode(1);break;
            case 's':/*STRENGTH*/indywiduum::set_mode(2);break;
            case 'w':/*AGE */   indywiduum::set_mode(3);break;
            case 't':/*TAXs*/   indywiduum::set_mode(4);break;
            case '+':VisRand=1;break;
            case '-':VisRand=0;break;
            case 'b':{
                int x=RANDOM(WORLD_SIDE*2);
                assert(x>=0 && x<WORLD_SIDE*2);
                int y=RANDOM(WORLD_SIDE);
                assert(y>=0 && y<WORLD_SIDE);
                double power=DRAND();
                assert(power>=0 && power<=1);
                krater(x, y, int(power * WORLD_SIDE));
            }break;
            case 'f':fflush(log);break;
            case -1:
            case 'q':/*QUIT */ return 0;
        }
    }
    if(zmieniony)
        caly_ekran(); // After changing the mode, you need to renew the whole screen
    return 1;
}

int main(int argc,const char* argv[])
{
shell_setup("CO-EVOLUTION",argc,argv);
printf("CO-EVOLUTION: a program that simulates the co-evolution of many species\n");
printf("COMMANDS: 'g': MUG 'o': SHIELD 's': STRENGTH 'w': AGE 't': TAXES\n"
              "'b':BOLID   +-:SRC.CLEANING 'f':FLUSH LOG 'q':QUIT\n");
printf("NUMBER OF POSSIBLE CLONES=%lu MAXINT=%d\n",(unsigned long)MAX_BASE2,MAXINT);

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
    fprintf(stderr,"No memory!\n");
    exit(1);
    }

RANDOMIZE();

init_plot( WORLD_SIDE*2+1, textY,0,1); //(WORLD_SIDE*2>256?WORLD_SIDE*2:256) ???

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
printf("Goodbye!!!\n");
return 0;
}

/// generate a para-poison distribution in the range 0..1 with 'n' degrees
double poison(int n)
{
double pom=1;
for(int i=1;i<=n;i++)
	pom*=DRAND(); // Multiplication by a value from 0..1
assert(pom>=0 && pom<=1);
return pom;
}


/* STATICS */
unsigned indywiduum::max=0;// what is the largest taxon
unsigned indywiduum::max_change=0;//... and whether the maximum changed recently
unsigned indywiduum::plot_mode=0;// what is to be displayed
unsigned indywiduum::ile_ind=0;// how many living individuals are there
unsigned indywiduum::ile_tax=0;// how many non-zero taxa
unsigned indywiduum::ile_big_tax=0;// how many taxa are more than 10
unsigned indywiduum::liczniki[ (long)(MAX_BASE2+1) ];// Taxa counters

/** CRATER - TRANSLATED FROM BASIC CODE IMPLEMENTED ELLIPSE DRAWING
   BASED ON BRESENHAM ALGORITHM                                      */
void swiat::krater(int x, int y, int r) // Makes a hole in the simulation area
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
if(yyp!=yi) // New line
	{
	n=2*xi+1;
	yyp= ys-yi;
        clearLine(xxp, yyp, n);
	yyp= ys+yi;
        clearLine(xxp, yyp, n);
	}
	else // Supplementing this line with marginal points
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
yyp=yi; // remember for comparison
BEZRYSOWANIA: //???
if(DELTAi<0L) goto _1300;
   else {if(DELTAi==0L)
		goto _1240;
	  else
		goto _1180; }

_1180:			 /* decision */
if( (DELTAi+DELTAi-a2s)<=0L )
		goto _1240;
	else
		goto _1380;
//continue;//???

_1240: 			/* diagonal step  */
xi++;
yi--;
a2s+=a2+a2;
b2s-=(b2+b2);
DELTAi+=(a2s-b2s);
continue;

_1300:			/* horizontal step */
if((DELTAi+DELTAi+b2s)>0L) goto _1240;
xi++;
a2s+=a2+a2;
DELTAi+=a2s;
continue;
//goto BEZRYSOWANIA;

_1380:			/* vertical step */
yi--;
b2s-=(b2+b2);
DELTAi-=b2s;
}

}
}
