/*................................................OPTIONS.H ......  */
/*..                                                                */
/*                                                                  */
/* options.h - Command line options and configuration file parser   */
/*                                                                  */
/* This version prepared for Redhat linux - Jan. 2000.              */
/* options.c was created as the part of pmaild.                     */
/*  Application - SGA-7N Project, PP test suite                     */
/*              - LAWANCE Project, SIT                              */
/*  Created:    01-18-2000  -  Gyorgy Horvath, DTT-TUB              */
/*  Modified:   03-21-2000  -  Gy.H. - Ported from Linux            */
/*  Modified:   05-12-2000  -  Gy.H. - Subsystems added             */
/*  Modified:   01-26-2004  -  Gy.H. - LAWANCE under Linux          */
/*  Modified:   2008-09-26  -  Gy.H. - Ported to Win32              */
/*  Modified:   2009-01-30  -  Gy.H. - PMCGI parts                  */
/*  Modified:   2009-02-05  -  Gy.H. - WEB and Form stuff added     */
/*  Modified:   2009-02-11  -  Gy.H. - prterr,prtwarn added         */
/*  Modified:   2009-03-14  -  Gy.H. - Filelis added                */
/*  Modified:   2009-03-18  -  Gy.H. - sprwebopt and co. added      */
/*  Modified:   2009-03-19  -  Gy.H. - uptick (RDTSC) added         */
/*  Modified:   2009-03-26  -  Gy.H. - safe_fopen,fclose            */
/*  Modified:   2009-03-31  -  Gy.H. - form2argv bug                */
/*  Modified:   2009-04-16  -  Gy.H. - form2argv is public          */
/*  Modified:   2009-04-22  -  Gy.H. - lookix added                 */
/*  Modified:   2009-05-12  -  Gy.H. - logtell added                */
/*  Modified:   2009-05-16  -  Gy.H. - _strmovl, _strmfill          */
/*  Modified:   2009-05-26  -  Gy.H. - double argument on #         */
/*  Modified:   2009-06-10  -  Gy.H. - user defined list            */
/*  Modified:   2009-09-01  -  Gy.H. - adj. for sateve              */
/*  Modified:   2009-09-14  -  Gy.H. - Multi-platform (Win32/Linux) */
/*                                                                  */
/* Command line option entry structure:                             */
/* name: The long name of the option (used by -- format)            */
/* con:  Applicable: 1: on the console 2: in html forms (3: both)   */
/* set:  Control and switch characters                              */
/*          :  processed as generic string                          */
/*          .  processed as a path variable                         */
/*          #  processed as a numeric variable                      */
/*          ?  lookup symbolic names for numeric value              */
/*          =  multiple checkbox (named bits up to 32)              */
/*          |  multiple selection (up to 32 items)                  */
/*          +  processed as a switch ( set var to TRUE  )           */
/*          -  processed as a switch ( set var to FALSE )           */
/*          ^  toggle On/Off  switch ( console actions  )           */
/*          *  call to a function if present                        */
/*          !  call to a function if present, post termination      */
/*          >  call to a function with argument                     */
/*          <  input file from list presented                       */
/*          /  user defined list element (string from list)         */
/*          $  Subsystem Arguments go here                          */
/*          @  Serialized arguments without leading - or --         */
/*       Further characters are the switches.                       */
/*       The '-' in this set involves '--' GNU style parsing        */
/*       and must be the last element in the set.                   */
/* form: Format string for printing/parsing the argument            */
/* desc: Short description of the switch (for usage)                */
/* var:  Address of storage/function for the arguments/switch       */
/*..                                                                */
/*  Disclaimer   - (C) Gyorgy Horvaath - DTT-TUB - 2000-2008        */
/*..                                                                */
/*..                                                                */
#ifndef _OPTIONS_H_DEFINED
#define _OPTIONS_H_DEFINED 1
/**/
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
/**/
/* Manifest constants */
#define MAXSTRLEN      512
#define MAXLINELENGTH 32000
/*                                                                  */
/* Type defineants                                                  */
/*                                                                  */
struct optentry
{
char *name;      /* Option name            */
int   con;       /* Console applicable     */
char *set;       /* Switch character(s)    */
char *form;      /* Format string          */
char *desc;      /* Descrtiption           */
void *var;       /* Points to the variable */
int   len;       /* Storage size           */
};

/* con flags */
#define CON_VAR      0x0001
#define CON_WEB      0x0002
#define CON_DEBOUNCE 0x8000


#define OPTSTRG( name ) (void *)name, sizeof(name)

struct optchoice
{
int  choice;                              /* No. of the choice      */
char **set;                               /* List of options        */
};


typedef int   (*OPTFUNC)( int argc, char *argv[] );
typedef int   (*ARGFUNC)( char *parg );
typedef char *(*LSTFUNC)( int no );
typedef struct _LOOKUP_T
{
int       id;     /* code to lookup             */
char     *str;    /* text for code              */
} LOOKUP_T, *PLOOKUP_T;
/*                                                                  */
/* Function Prototypes                                              */
/*                                                                  */
void prtusage(   FILE *of, char *cmdline, struct optentry *opts );
void prtoptions( FILE *of, char *hdr, struct optentry *opts );
int  sprintopt( char *bu, struct optentry *opts );
int  fgenflist( FILE *of, char *ptrn,  char *fmt );
int  getoptions( int argc, char **argv, struct optentry *options  );
int  getformopt( char *form, struct optentry *options  );
int  formoptions( char *form, struct optentry *options  );
struct optentry *findopt( char *name, struct optentry *options );
int  getconfig( FILE *cf, struct optentry *options  );
/**/
void weboptions( FILE *of, int id, char *tdata, unsigned long tlen, struct optentry *opts );
int  wgenflist( FILE *of, char *fn, char *ptrn,  char *afmt,  char *bfmt );
int  swgenflist( char *of, int tlen, char *fn, char *ptrn,  char *afmt,  char *bfmt );
void sprwebrst( void );
int  sprwebopt( char *of, int olen, struct optentry *opts, char *tdata );
char ** form2argv( char *form, int *pargc );
/**/
char *lookup( int id,    PLOOKUP_T tab );
int  lookdn(  char *str, PLOOKUP_T tab );
int  lookix(  char *str, PLOOKUP_T tab );
int  mgetline( FILE *ifile, char *currline, int nolf );
int  putline( FILE *ofile, char *currline, int u2dos );
void turnoffall( struct optentry *opts );
int  turnoffcon( struct optentry *opts );
unsigned long filelen( FILE *inf );
/**/
char *_strmcat(  char *s1,  char *s2, int upto );
char *_strmcpy(  char *s1,  char *s2, int upto );
char *_strmgetline( char *s1, char *s2, int upto );
int   _strmcpwrap( char *s1, char *s2, int upto, int indp, int ind, int wp, char *bkind );
char *_strmovl( char *s1, char *s2, int upto );
char *_strmfill( char *s1, char cf, int upto );
int   _lookup(   char *str, char *table[] );
int   _mistrcmp( char *s1,  char *s2 );
int   _ilookup(  char *str, char *table[] );
/**/
void prtinf(  char *fmt, ... );
void prterr(  char *fmt, ... );
void prtwarn( char *fmt, ... );
void loginf(  char *logfn, char *fmt, ... );
void logwarn( char *logfn, char *fmt, ... );
void logerr(  char *logfn, char *fmt, ... );
unsigned long logtell( char *logfn );
void wrtinf(  FILE *fout,  char *fmt, ... );
void wrtwarn( FILE *fout,  char *fmt, ... );
void wrterr(  FILE *fout,  char *fmt, ... );
/**/
unsigned long uptick( void );
FILE *safe_fopen( char *name, char *newname, char *mode );
int   safe_fclose( FILE *fout, char *name, char *newname );
/**/
#ifdef __cplusplus
}
#endif /* __cplusplus        */
#endif /* _OPTIONS_H_DEFINED */
/*..                                                                */
/*...................... END OF ..................OPTIONS.H ......  */
