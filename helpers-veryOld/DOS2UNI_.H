/************************************************************************/
/*      Definicje programu CLAGEN do przenoszenia DOS<->UNIX            */
/*     ======================================================           */
/*  projekt nr 2.5 1994                                                */
/*                                                                      */
/*    Zaklad Systematyki i Geografii Roslin Instytutu Botaniki UW       */
/*                                                                      */
/*                      Wojciech BORKOWSKI                              */
/************************************************************************/
/* DOS keyword */
#include <time.h>
#if defined( unix ) || defined( GNUC )

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
int input_char_ready();          /* Return 1 if is char ready to read from stdin */

#endif

/* Randomizer compatybility */
#ifdef unix_BSD

extern long random();
extern int srandom();
#define RAND_MAX  2147483646
#define Random( _par_ ) ( (int) (((double) (random)() * (_par_) ) / (RAND_MAX+1) ) )
#define Randomize()    { (srandom)( (unsigned) time(NULL) ); }
#define Rand()          ( (int)random() )

#elif defined( unix_V )

#define Random( _par_ ) ( (long) ( drand48() * (_par_)  ) )
#define Randomize()    { srand48( (long) time(NULL) ); }
#define Rand()          ( (int)lrand48() )

#else 
/*  STANDARD "C"  RANDOMIZER  */
#ifdef __cplusplus
inline int Random(int _num_)
		 { return(int)(((long)rand()*_num_ )/(RAND_MAX+1)); }
/* need prototype of time() for C++ Randomize() */
inline void Randomize(void) { srand((unsigned) time(NULL)); }
#else
#define Random(_num_) (int)(((long)rand()*(_num_))/(RAND_MAX+1))
#define Randomize()     srand((unsigned)time(NULL))
#endif
#define Rand()          ( (int)rand() )

#endif

/* USEABLE MACROS */
#ifdef unix

#define _SL_STR         "/"
#define _SL_C           '/'
#define NEWTEXTPAGE putchar('\f');
#define CLRSCR      putchar('\f');
#define DELLINE     putchar(12);
#define PENTER      "(press RETURN)"
#define PAUSE      { fprintf(stderr,PENTER);gets(); }
#define STOP    {error(__FILE__,__LINE__,"STOP",-10);}

#else
#include <conio.h>
#define _SL_STR         "\\"
#define _SL_C           '\\'
#define PRINTF      printf
#define NEWTEXTPAGE gotoxy(1,1)
#define CLRSCR      clrscr()
#define DELLINE    {gotoxy(1,wherey());}
#define PENTER          "(press ENTER)"
#define PAUSE      {PRINTF(PENTER);getch(); }
extern int pass_value;
#define STOP    {fprintf(stderr,"Blad w %s %d!\n Ctrl-break - koniec q-dalej\n",__FILE__,__LINE__);  \
		 do{                  \
		 if(kbhit()) pass_value=getch();}\
		 while(pass_value!='q');         \
		 pass_value='\0'; }
#endif



#ifdef DEBUG

#       ifdef unix

#define HTEST

#       else

#define HTEST if(heapcheck()<0) {fprintf(stderr," Fatal: Heap corrupted ! %s ,%d\n",__FILE__,__LINE__);abort(); }
int heapcheck(void);

#       endif

#define PTEST( _P_ ) if((_P_)==NULL){ fprintf(stderr,"Fatal: NULL use !   %s,%d\n",__FILE__,__LINE__);abort(); }

#else

/* NO DEBUG */
#define HTEST
#define PTEST( _P_ )

#endif


