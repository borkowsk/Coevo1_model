/************************************************************************/
/*     Definicje dla programu CLAGEN do przenoszenia DOS<->UNIX         */
/*     ========================================================         */
/*  projekt nr 2.5 1994                                                 */
/*                                                                      */
/*    Zakład Systematyki i Geografii Roślin Instytutu Botaniki UW       */
/*                                                                      */
/*                      Wojciech BORKOWSKI                              */
/************************************************************************/
/* DOS keyword */
#include <time.h>

#if defined( unix )
// Library effective with Linux
#include <unistd.h>
#include <stdlib.h>
#elif defined(WIN32)
// Under Windows there is no 'unistd.h'
#include <stdlib.h>
#else
#include <dos.h>
#endif

#if defined( unix ) || defined( GNUC ) || defined(WIN32)
#include <limits.h>
#else
#include <values.h>
#endif

#if defined(MAXINT)
#pragma warning( "MAXINT already defined!" )
#else
#define  MAXINT INT_MAX
#endif

#if defined( unix ) || defined( GNUC ) || defined(WIN32)
#define far
#define near
#define huge
#define cdecl
#endif

/* Blocking switching for unix tty*/
#ifdef unix
int init_input_block_switching(); /* Initialization blocking module */
extern int is_tty;                /* If input is from terminal ==1  */
void blocking_stdin(/* block */); /* Switching blocking on stdin    */
int input_char_ready();           /* Return 1 if is char ready to read from stdin */
#endif

#ifdef unix_BSD_rand

extern long random();
extern int srandom();

#define RANDOM_MAX  (0x7fffffff)
/* Preprocesor nie moze liczyc MAXINT bo jest to wielokrotnie zagnizdzona definicja
#if ( RANDOM_MAX ) != ( MAXINT )
#error int have not 32 bit!
#endif
...a szkoda.
*/
#define Random( _par_ ) ( (int) (((double) (random)() * (_par_) ) / ((double)RANDOM_MAX+1) ) )
#define DRand()         ( (double)random()/(double)(RANDOM_MAX) )
#define Randomize()    { (srandom)( (unsigned) time(NULL) ); }
#define Rand()          ( (int)random() )

#elif defined( unix_V_rand )
#define RANDOM_MAX (0xffffffff)
#define Random( _par_ ) ( (long) ( drand48() * (_par_)  ) )
#define DRand()		( (double)drand48() )
#define Randomize()    { srand48( (long) time(NULL) ); }
#define Rand()          ( (int)lrand48() )

#else
/*  STANDARD "C"  RANDOMIZER  */

#define RANDOM_MAX (RAND_MAX)
#ifdef __cplusplus
inline int Random(int _num_)
		 { return(int)(((double)rand()*(_num_) )/((double)RAND_MAX+1)); }
inline double DRand(){ return (double)rand()/(double)RAND_MAX; }
/* need prototype of time() for C++ Randomize() */
inline void Randomize(void) { srand((unsigned) time(NULL)); }
#else
#define Random(_num_) (int)(((double)rand()*(_num_))/((double)RAND_MAX+1))
#define DRand()        ( (double)rand()/(double)RAND_MAX )
#define Randomize()     srand((unsigned)time(NULL))
#endif
#define Rand()          ( (int)rand() )

#endif

/* USABLE MACROS */
#if defined(unix) || defined(GNUC) || defined(WIN32)
// STANDARD CONSOLE STUFF
#define _SL_STR         "/"
#define _SL_C           '/'
#define NEWTEXTPAGE  putchar('\f');
#define CLRSCR       putchar('\f');
#define DELLINE      putchar(12);
#define PENTER       "(press RETURN)"
#define PAUSE		 { fprintf(stderr,PENTER);gets(); }
#define STOP		 {error(__FILE__,__LINE__,"STOP",-10);}

#else
// BORLAND STUFF FOR DOS!
#include <conio.h>
#define _SL_STR         "\\"
#define _SL_C           '\\'
#define PRINTF       printf
#define NEWTEXTPAGE  gotoxy(1,1)
#define CLRSCR       clrscr()
#define DELLINE      {gotoxy(1,wherey());}
#define PENTER       "(press ENTER)"
#define PAUSE        {PRINTF(PENTER);getch(); }
extern int pass_value;
#define STOP    {fprintf(stderr,"Blad w %s %d!\n Ctrl-break - koniec q-dalej\n",__FILE__,__LINE__);  \
		 do{                  \
		 if(kbhit()) pass_value=getch();}\
		 while(pass_value!='q');         \
		 pass_value='\0'; }

#endif


#if defined(DEBUG)

	#if defined(__TURBOC__)
	// VERY OLD. MAYBE STILL USEABLE IN DOSBOX! DEBUG
	#define HTEST if(heapcheck()<0) {fprintf(stderr," Fatal: Heap corrupted ! %s ,%d\n",__FILE__,__LINE__);abort(); }
	int heapcheck(void);
	#endif

#define PTEST( _P_ ) if((_P_)==NULL){ fprintf(stderr,"Fatal: NULL use !   %s,%d\n",__FILE__,__LINE__);abort(); }

#else

/* NO DEBUG */
#define HTEST			{}
#define PTEST( _P_ )    {}

#endif


