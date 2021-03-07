/*................................................OPTIONS.C ......  */
/*..                                                                */
/*                                                                  */
/* options.c - Command line options and configuration file parser   */
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
/*                                                                  */
/*  Includes                                                        */
/*                                                                  */
#ifdef   __MINGW32__
#include <windows.h>
#else
#define _GNU_SOURCE
#include <stdlib.h>
#include <sys/types.h>
#include <sys/time.h>
#include <dirent.h>
#include <fnmatch.h>
#endif /*__MINGW32__*/
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
/**/
#include "options.h"
/*                                                                  */
/*  Local variables                                                 */
/*                                                                  */
static OPTFUNC doit  = NULL;
static ARGFUNC doarg = NULL;
static char snone[10] = "none";
/**/
#ifdef   __MINGW32__
static WIN32_FIND_DATA ffd;
static HANDLE          hff;
#else
static struct dirent  *ffd;
static DIR            *hff;
#endif /*__MINGW32__*/
/* =============== Print a file list  ============================== */
int fgenflist( FILE *of, char *ptrn,  char *fmt )
{
char *p;
#ifndef  __MINGW32__
char *q = NULL;
#endif /*__MINGW32__*/
int i = 0;
if( !of   ) return 0;
if( !ptrn ) return 0;
if( !fmt )  return 0;
/**/
#ifdef   __MINGW32__
hff = FindFirstFile( (LPCSTR)ptrn, &ffd );
if( hff == INVALID_HANDLE_VALUE )
  {
  fprintf( of, fmt, snone );
  return 0;
  }
do {
   p = &ffd.cFileName[0]+strlen(ffd.cFileName)-1;
   while( p > &ffd.cFileName[0]  && *p != '.' ) p--;
   if( *p == '.' ) *p='\0';
   fprintf( of, fmt, ffd.cFileName );
   i++;
   } while( FindNextFile( hff, &ffd ) );
FindClose( hff );
/**/
#else  /*__MINGW32__*/
while( (p = strchr( ptrn, '/' )) ) q = p;
if( q ) *q = '\0';
hff = opendir( ptrn );
if( !hff )
  {
  if( q ) *q = '/';
  fprintf( of, fmt, snone );
  return 0;
  }
while( (ffd = readdir( hff )) )
   {
   if( fnmatch( q?q:ptrn, ffd->d_name, FNM_CASEFOLD ) ) continue;
   p = &ffd->d_name[0]+strlen(ffd->d_name)-1;
   while( p > &ffd->d_name[0]  && *p != '.' ) p--;
   if( *p == '.' ) *p='\0';
   fprintf( of, fmt, ffd->d_name );
   i++;
   };
closedir( hff );
if( q ) *q = '/';
if( !i ) fprintf( of, fmt, snone );
#endif /*__MINGW32__*/
return i;
}
int wgenflist( FILE *of, char *fn, char *ptrn,  char *afmt,  char *bfmt )
{
char *p;
#ifndef  __MINGW32__
char *q = NULL;
#endif /*__MINGW32__*/
int i = 0;
if( !of   ) return 0;
if( !fn   ) return 0;
if( !ptrn ) return 0;
if( !afmt ) return 0;
if( !bfmt ) return 0;
/**/
#ifdef   __MINGW32__
hff = FindFirstFile( (LPCSTR)ptrn, &ffd );
if( hff == INVALID_HANDLE_VALUE )
  {
  fprintf( of, bfmt, snone, snone );
  return 0;
  }
do {
   p = &ffd.cFileName[0]+strlen(ffd.cFileName)-1;
   while( p > &ffd.cFileName[0]  && *p != '.' ) p--;
   if( *p == '.' ) *p='\0';
   if( strcmp( fn, ffd.cFileName ) )
       fprintf( of, afmt, ffd.cFileName, ffd.cFileName );
   else
       fprintf( of, bfmt, ffd.cFileName, ffd.cFileName );
   i++;
   } while( FindNextFile( hff, &ffd ) );
FindClose( hff );
/**/
#else  /*__MINGW32__*/
while( (p = strchr( ptrn, '/' )) ) q = p;
if( q ) *q = '\0';
hff = opendir( ptrn );
if( !hff )
  {
  if( q ) *q = '/';
  fprintf( of, bfmt, snone, snone );
  return 0;
  }
while( (ffd = readdir( hff )) )
   {
   if( fnmatch( q?q:ptrn, ffd->d_name, FNM_CASEFOLD ) ) continue;
   p = &ffd->d_name[0]+strlen(ffd->d_name)-1;
   while( p > &ffd->d_name[0]  && *p != '.' ) p--;
   if( *p == '.' ) *p='\0';
   if( strcmp( fn, ffd->d_name ) )
       fprintf( of, afmt, ffd->d_name, ffd->d_name );
   else
       fprintf( of, bfmt, ffd->d_name, ffd->d_name );
   i++;
   };
closedir( hff );
if( q ) *q = '/';
if( !i ) fprintf( of, bfmt, snone, snone );
#endif /*__MINGW32__*/
return i;
}
int swgenflist( char *pof, int tlen, char *fn, char *ptrn,  char *afmt,  char *bfmt )
{
char  *p;
int    len;
char  *of= pof;
#ifndef  __MINGW32__
int    i = 0;
char  *q = NULL;
#endif /*__MINGW32__*/
if( !of   ) return 0;
if( !tlen ) return 0;
if( !fn   ) return 0;
if( !ptrn ) return 0;
if( !afmt ) return 0;
if( !bfmt ) return 0;
/**/
#ifdef   __MINGW32__
hff = FindFirstFile( (LPCSTR)ptrn, &ffd );
if( hff == INVALID_HANDLE_VALUE )
  {
  if( tlen <= (strlen(snone)+1) ) return 0;
  sprintf( of, bfmt, snone, snone );
  return strlen(of);
  }
do {
   p = &ffd.cFileName[0]+strlen(ffd.cFileName)-1;
   while( p > &ffd.cFileName[0]  && *p != '.' ) p--;
   if( *p == '.' ) *p='\0';
   if( strcmp( fn, ffd.cFileName ) )
     {
     if( tlen <= (strlen(of)+strlen(afmt)+strlen(ffd.cFileName)) ) return 0;
     sprintf( of, afmt, ffd.cFileName, ffd.cFileName );
     }
   else
     {
     if( tlen <= (strlen(of)+strlen(bfmt)+strlen(ffd.cFileName)) ) return 0;
     sprintf( of, bfmt, ffd.cFileName, ffd.cFileName );
     }
   len = strlen(of);
   of   += len;
   tlen -= len;
   if( tlen <= 0 ) return 0;
   } while( FindNextFile( hff, &ffd ) );
FindClose( hff );
/**/
#else  /*__MINGW32__*/
while( (p = strchr( ptrn, '/' )) ) q = p;
if( q ) *q = '\0';
hff = opendir( ptrn );
if( !hff )
  {
  if( q ) *q = '/';
  sprintf( of, bfmt, snone, snone );
  return 0;
  }
while( (ffd = readdir( hff )) )
   {
   if( fnmatch( q?q:ptrn, ffd->d_name, FNM_CASEFOLD ) ) continue;
   p = &ffd->d_name[0]+strlen(ffd->d_name)-1;
   while( p > &ffd->d_name[0]  && *p != '.' ) p--;
   if( *p == '.' ) *p='\0';
   if( strcmp( fn, ffd->d_name ) )
     {
     if( tlen <= (strlen(of)+strlen(afmt)+strlen(ffd->d_name)) ) 
       {
       if( q ) *q = '/';
       return 0;
       }
     sprintf( of, afmt, ffd->d_name, ffd->d_name );
     }
   else
     {
     if( tlen <= (strlen(of)+strlen(bfmt)+strlen(ffd->d_name)) )
       {
       if( q ) *q = '/';
       return 0;
       }
     sprintf( of, bfmt, ffd->d_name, ffd->d_name );
     }
   len = strlen(of);
   of   += len;
   tlen -= len;
   if( tlen <= 0 ) 
     {
     if( q ) *q = '/';
     return 0;
     }
   i++;
   };
closedir( hff );
if( q ) *q = '/';
if( !i ) sprintf( of, bfmt, snone, snone );
#endif /*__MINGW32__*/
return strlen(pof);
}
/* =============== Print out the usage ============================= */
void prtusage( FILE *of, char *cmdline, struct optentry *opts )
{
char *sws;
LSTFUNC   lfun;
char *pe;
int i;
PLOOKUP_T pl = NULL;
/**/
if( !of ) of = stdout;
if( !opts ) return;
fprintf( of, "Usage: %s\n", cmdline );
while( opts->name )
    {
    if( *(opts->set) == '@' ) { opts++; continue; };
    if( *(opts->set) == '$' ) { opts++; continue; };
    sws = opts->set+1;
    fprintf( of, "   " );
    while( *sws )
       {
       if( *sws == '-' )
          {
          if( sws != (opts->set+1) )
              fprintf( of, ", " );
          fprintf( of, "-%s", sws );
          break;
          }
       else
          {
          if( sws != (opts->set+1) )
              fprintf( of, ", " );
          fprintf( of, "-%c", *sws );
          sws++;
          }
       }
    switch( *(opts->set) )
       {
    case ':':  fprintf( of, " <strg>" ); break;
    case '<':  fprintf( of, " <name>" ); break;
    case '.':  fprintf( of, " <path>" ); break;
    case '#':  fprintf( of, " <num>"  ); break;
    case '?':  fprintf( of, " <key>"  ); break;
    case '/':;
    case '|':;
    case '=':  fprintf( of, " <sel>"  ); break;
    case '>':  fprintf( of, " <arg>"  ); break;
       }
    fprintf( of, " .... %s\n", opts->desc );
    if( *(opts->set) == '<' )
      {
      fgenflist( of, opts->form, "           %s\n" );
      }
    if( *(opts->set) == '?' ||
        *(opts->set) == '=' ||
        *(opts->set) == '|' )
      {
      pl = (PLOOKUP_T)(opts->form);
      i = 0;
      if( pl ) 
         while( pl->str )
            {
            switch( (i%4) )
               {
            case 0:
                if( (pl+1)->str )
                    fprintf( of, "           %s",   pl->str );
                else
                    fprintf( of, "           %s\n", pl->str );
                break;
            case 3:
                fprintf( of, ", %s\n", pl->str );
                break;
            default:
                if( (pl+1)->str )
                    fprintf( of, ", %s", pl->str );
                else
                    fprintf( of, ", %s\n", pl->str );
                };
            pl++;
            i++;
            }
      }
    if( *(opts->set) == '/' ) /* User defined list */
      {
      lfun = (LSTFUNC)(opts->form);
      i = 0;
      if( lfun )
         while( (pe = lfun(i)) != NULL )
            {
            switch( (i%4) )
               {
            case 0:
                if( (pl+1)->str )
                    fprintf( of, "           %s",   pe );
                else
                    fprintf( of, "           %s\n", pe );
                break;
            case 3:
                fprintf( of, ", %s\n", pe );
                break;
            default:
                if( (pl+1)->str )
                    fprintf( of, ", %s", pe );
                else
                    fprintf( of, ", %s\n", pe );
                };
            i++;
            }
      }
    opts++;
    }
fprintf( of, "\n" );
}

/* =============== Print out the active configuration options ====== */
void prtoptions( FILE *of, char *hdr, struct optentry *opts )
{
char _hdr[64];
PLOOKUP_T pl;
/**/
if( !of ) of = stdout;
if( !opts ) return;
if( hdr ) fprintf( of, hdr, __LINE__);
while( opts->name )
    {
    if( opts->con & CON_VAR )
    switch( *(opts->set) )
       {
    case '.':      /* String variable  */
    case ':':;
    case '<':;
    case '@':;
    case '/':;
       fprintf( of, "%s %s\n", opts->name, (char *)opts->var );
       break;
    case '#':      /* Numeric variable */
       fprintf( of, "%s ", opts->name );
       fprintf( of, opts->form, *((double *)(opts->var)) );
       fprintf( of, "\n" );
       break;
    case '?':      /* Lookup */
       fprintf( of, "%s ", opts->name );
       fprintf( of, "%s" , lookup( *((int *)(opts->var)), 
                                   (PLOOKUP_T)(opts->form)));
       fprintf( of, "\n" );
       break;
    case '|':      /* Multisel */
    case '=':      /* Multicheck */
       pl = (PLOOKUP_T)(opts->form);
       if( pl )
         while( pl->str )
            {
            if( pl->id & *((int *)(opts->var)) )
              {
              fprintf( of, "%s ", opts->name );
              fprintf( of, "%s" , pl->str );
              fprintf( of, "\n" );
              }
            pl++;
            }
       break;
    case '+':;     /* Bool variable (on/off toggle) */
    case '-':
    case '^':
       fprintf( of, "%s %s\n", opts->name, *((int *)(opts->var)) ?
               "ON" : "OFF" );
       break;
    case '$':;     /* Subsystem block */
       if( opts->var )
           {
           sprintf( _hdr, "%.63s\n", opts->name );
           prtoptions( of, _hdr, (struct optentry *)(opts->var) );
           }
       }
    opts++;
    }
fprintf( of, "\n" );
}

/* =============== Print an option into a buffer =================== */
int sprintopt( char *of, struct optentry *opts )
{
PLOOKUP_T pl;
if( !of )   return 0;
if( !opts ) return 0;
switch( *(opts->set) )
   {
case '.':      /* String variable  */
case ':':;
case '<':;
case '@':;
case '/':;
   sprintf( of, "%s", (char *)opts->var );
   break;
case '#':      /* Numeric variable */
   sprintf( of, opts->form, *((double *)(opts->var)) );
   break;
case '?':      /* Lookup */
   sprintf( of, "%s" , lookup( *((int *)(opts->var)),
                               (PLOOKUP_T)(opts->form)));
   break;
case '|':      /* Multisel */
case '=':      /* Multicheck */
   pl = (PLOOKUP_T)(opts->form);
   if( pl )
     while( pl->str )
        {
        if( pl->id & *((int *)(opts->var)) )
          {
          sprintf( of, "%s" , pl->str );
          }
        pl++;
        }
   break;
case '+':;     /* Bool variable (on/off toggle) */
case '-':
case '^':
   sprintf( of, "%s", *((int *)(opts->var)) ?
           "ON" : "OFF" );
   break;
   }
return strlen(of);
}
/**/
static void      *svold = NULL;
static PLOOKUP_T  spl;
void sprwebrst( void )
{
svold = NULL;
}
/* =============== Print an option into a buffer =================== */
int sprwebopt( char *of, int olen, struct optentry *opts, char *tdata )
{
char       _hdr[128];
char      *alta;
char      *altb;
char      *p;
LSTFUNC    lfun;
char      *pe;
int        i;
/**/
if( !of )   return 0;
if( !olen)  return 0;
if( !opts ) return 0;
alta=altb=NULL;
if( tdata )
  {
  i=0;
  while( i<sizeof(_hdr) )
    {
    switch( *tdata )
      {
    case '\0': ;
    case '\n': ;
    case '\r': ;
    case '$':
      _hdr[i] = 0;
      goto GotOne;
    case '?':
      _hdr[i] = 0;
      alta = &_hdr[i+1];
      break;
    case ':':
      _hdr[i] = 0;
      altb = &_hdr[i+1];
      break;
    default:
      _hdr[i] = *tdata;
      break;
      }
    tdata++;
    i++;
    }
  }
/**/
GotOne:
/**/
switch( *(opts->set) )
   {
case '.':      /* String variable  */
case ':':;
case '@':;
   if( olen < strlen((char *)opts->var) ) return 0;
   sprintf( of, "%s", (char *)opts->var );
   break;
case '<':;
   swgenflist( of, olen, (char *)opts->var, opts->form,
     "<option value=\"%s\"> %s </option>\n",
     "<option value=\"%s\" selected=\"selected\"> %s </option>\n" );
   break;
case '#':      /* Numeric variable */
   if( alta && altb )
     {
     if( olen < strlen(alta) ) return 0;
     if( olen < strlen(altb) ) return 0;
     sprintf( of, "%s", *((int *)(opts->var)) ? alta : altb );
     }
   else
     {
     if( olen < (strlen(opts->form)+10) ) return 0;
     sprintf( of, opts->form, *((double *)(opts->var)) );
     }
   break;
case '?':      /* Lookup */
   spl = (PLOOKUP_T)(opts->form);
   p = of;
   if( spl )
   while( spl->str && opts->var )
     {
     if( (olen-strlen(p)) < 56 ) return 0;
     sprintf( p, "<option value=\"%s\" ", spl->str );
     p += strlen(p);
     if( spl->id == *((int *)(opts->var)) )
       {
       sprintf( p, "selected=\"selected\"" );
       p += strlen(p);
       }
     sprintf( p, "> %s </option>\n", spl->str );
     p += strlen(p);
     spl++;
     }
   break;
case '/':      /* User defined */
   lfun = (LSTFUNC)(opts->form);
   i=0;
   p = of;
   if( lfun )
    while( (pe=lfun(i))!=NULL )
     {
     if( (olen-strlen(p)) < 56 ) return 0;
     sprintf( p, "<option value=\"%s\" ", pe );
     p += strlen(p);
     if( !strcasecmp( pe, ((char *)(opts->var))) )
       {
       sprintf( p, "selected=\"selected\"" );
       p += strlen(p);
       }
     sprintf( p, "> %s </option>\n", pe );
     p += strlen(p);
     i++;
     }
   break;
case '=':      /* Multicheck */
   if( svold != opts->var )
     {
     spl = (PLOOKUP_T)(opts->form);
     svold = opts->var;
     }
   if( spl->str && opts->var )
     {
     if( spl->id & *((int *)(opts->var)) )
       {
       if( olen < 9 ) return 0;
       sprintf( of, "checked" );
       }
     spl++;
     }
   break;
case '|':      /* Multisel */
   if( svold != opts->var )
     {
     spl = (PLOOKUP_T)(opts->form);
     svold = opts->var;
     }
   if( spl->str && opts->var )
     {
     if( spl->id & *((int *)(opts->var)) )
       {
       if( olen < 20 ) return 0;
       sprintf( of, "selected=\"selected\"" );
       }
     spl++;
     }
   break;
case '+':;     /* Bool variable (on/off toggle) */
case '-':
case '^':
   if( alta && altb )
     {
     if( olen < strlen(alta) ) return 0;
     if( olen < strlen(altb) ) return 0;
     sprintf( of, "%s", *((int *)(opts->var)) ?
           alta : altb );
     }
   else
     {
     if( olen < 9 ) return 0;
     sprintf( of, "%s", *((int *)(opts->var)) ?
           "checked" : " " );
     }
   break;
case '!':; /* Execute */
case '*':
   if( (doit = (OPTFUNC)(opts->var)) != NULL ) 
        doit( 0, NULL );
   break;
case '$':;     /* Subsystem block */
   /* TODO */
default:
   break;
   }
return strlen(of);
}
/* =============== HTML  out the active configuration options ====== */
void weboptions( FILE *of, int id, char *tdata, unsigned long tlen, struct optentry *optin )
{
struct optentry *opts;
char      _hdr[128];
void     *vold;
char     *alta;
char     *altb;
LSTFUNC   lfun;
char     *pe;
int      i;
PLOOKUP_T pl=NULL;
/**/
vold=NULL;
/**/
if( !of ) of = stdout;
if( !optin) return;
if( !tdata) return;
if( !tlen ) return;
/* Parse template for arguments */
while( tlen )
  {
  if( *tdata == '@' )
    {
    fprintf( of, "%i", id );
    tdata++;
    tlen--;
    continue;
    }
  else if( *tdata == '$' )
    {
    alta=altb=NULL;
    tdata++;
    tlen--;
    i=0;
    while( tlen && i<sizeof(_hdr) )
      {
      switch( *tdata )
        {
      case '$':
        _hdr[i] = 0;
        goto GotOne;
      case '?':
        _hdr[i] = 0;
        alta = &_hdr[i+1];
        break;
      case ':':
        _hdr[i] = 0;
        altb = &_hdr[i+1];
        break;
      default:
        _hdr[i] = *tdata;
        break;
        }
      tdata++;
      tlen--;
      i++;
      }
    break;
    }
  else 
    {
    fwrite( tdata, 1, 1, of );
    tdata++;
    tlen--;
    continue;
    }
/**/
/* Process one argument */
GotOne:
/**/
  opts = optin;
  while( opts->name )
    {
    if( strcmp( _hdr, opts->name ) )
      {
      opts++;
      continue;
      }
    if( opts->con & CON_WEB )
    switch( *(opts->set) )
       {
    case '.':      /* String variable  */
    case ':':;
    case '@':;
       fprintf( of, "%s", (char *)opts->var );
       break;
    case '<':;
       wgenflist( of, (char *)opts->var, opts->form,
         "<option value=\"%s\"> %s </option>\n",
         "<option value=\"%s\" selected=\"selected\"> %s </option>\n" );
       break;
    case '#':      /* Numeric variable */
       if( alta && altb )
         fprintf( of, "%s", *((int *)(opts->var)) ? alta : altb );
       else
         fprintf( of, opts->form, *((double *)(opts->var)) );
       break;
    case '?':      /* Lookup */
       pl = (PLOOKUP_T)(opts->form);
       while( pl->str && opts->var )
         {
         fprintf( of, "<option value=\"%s\" ", pl->str );
         if( pl->id == *((int *)(opts->var)) )
             fprintf( of, "selected=\"selected\"" );
         fprintf( of, "> %s </option>\n", pl->str );
         pl++;
         }
       break;
    case '/':      /* User defined */
       lfun = (LSTFUNC)(opts->form);
       i=0;
       if( lfun )
        while( (pe=lfun(i))!=NULL )
         {
         fprintf( of, "<option value=\"%s\" ", pe );
         if( !strcasecmp( pe, ((char *)(opts->var))) )
           fprintf( of, "selected=\"selected\"" );
         fprintf( of, "> %s </option>\n", pe );
         i++;
         }
       break;
    case '=':      /* Multicheck */
       if( vold != opts->var )
         {
         pl = (PLOOKUP_T)(opts->form);
         vold = opts->var;
         }
       if( pl->str && opts->var )
         {
         if( pl->id & *((int *)(opts->var)) )
             fprintf( of, "checked" );
         pl++;
         }
       break;
    case '|':      /* Multisel */
       if( vold != opts->var )
         {
         pl = (PLOOKUP_T)(opts->form);
         vold = opts->var;
         }
       if( pl->str && opts->var )
         {
         if( pl->id & *((int *)(opts->var)) )
             fprintf( of, "selected=\"selected\"" );
         pl++;
         }
       break;
    case '+':;     /* Bool variable (on/off toggle) */
    case '-':
    case '^':
       if( alta && altb )
         {
         fprintf( of, "%s", *((int *)(opts->var)) ?
               alta : altb );
         }
       else
         {
         fprintf( of, "%s", *((int *)(opts->var)) ?
               "checked" : " " );
         }
       break;
    case '!':; /* Execute */
    case '*':
       if( (doit = (OPTFUNC)(opts->var)) != NULL )
            doit( 0, NULL );
       break;
    case '$':;     /* Subsystem block */
       /* TODO */
    default:
       break;
       }
    break;
    }
  tdata++;
  tlen--;
  }
}
/* =============== Process options in forms posted ================= */
static int cntargs( char *form )
{
int retval = 0;
if( !form )  return 0;
if( !*form ) return 0;
while( *form )
   {
   if( *form == '&' ) retval++;
   if( *form == '=' ) retval++;
   form++;
   }
return retval+1;
}
/**/
char ** form2argv( char *form, int *pargc )
{
char hexstr[4];
char *p;
char *q;
char **retval;
char **argv;
int  sw=1;
int argc = cntargs( form );
/**/
p = malloc( strlen(form)+(4*argc)+20 );
if( !p ) return NULL;
retval = malloc( sizeof(char *)*((2*argc)+8) );
if( !retval )
  {
  free(p);
  return NULL;
  }
/**/
argc = 0;
argv = retval;
*(argv++) = p;
strcpy( p, "form" );
p+=strlen(p);
*(p++)=0;
argc++;
while( *form )
  {
  *argv = p;
  if( sw )
    {
    *(p++) = '-';
    *(p++) = '-';
    }
  while( *form )
    {
    p[1] = '\0';
    switch( *form )
      {
    case '%':
      form++;
      hexstr[0] = *(form++);
      hexstr[1] = *form;
      hexstr[2] = '\0';
      sscanf( hexstr, "%2x", (unsigned int *)p );
      break;
    case '+': ;
      *p = ' ';
      break;
    case '=': ;
      *p = '\0';
      q = strchr( *argv, '.' );  /* Bitmap Buttons hack for .x, .y */
      if( q ) *q = '\0';
      if( !form[1] )
        {
        argc--;
        argv--;
        break;
        }
      if( !strcmp( *argv+2, "op" ) )
        {
        p = *argv; /* Skip op=. . . */
        p--;
        argc--;
        argv--;
        sw=1;
        }
      else
        {
        sw=0;
        }
      break;
    case '&':
      sw=1;
      *p = '\0';
      break;
    default:
      *p = *form;
      break;
      }
/**/
    if( !*form || *form=='&' || *form=='=' )
      {
      p++;
      form++;
      break;
      }
    p++;
    form++;
    }
  argc++;
  argv++;
  }
/**/
argv++;
*argv = NULL;
if( pargc ) *pargc = argc;
/**/
return retval;
}
/**/
int getformopt( char *form, struct optentry *options  )
{
int report,argc;
char **argv;
argv = form2argv( form, &argc );
if( !argv ) return 0;
if( !argc ) return 0;
report = getoptions( argc, argv, options );
free( argv[0] );
free( argv    );
return report;
}
/* =============== Process command line options ==================== */
int getoptions( int argc, char **argv, struct optentry *options  )
{
int report = 1;
struct optentry *opts;
struct optentry *args;
char *sws;
char  actsw;
if( !options ) return 0;
args = options;
argc--;
argv++;
while( argc>0 )
 {
 report = 0;
 opts = options;
 if( **argv == '-' )
   {
/* Scan for switches (with leading - or --) */
   while( opts->name )
    {
    actsw = (*argv)[1];
    sws = opts->set+1;
    while( *sws )
     {
     if( *sws == '-' )
       {
       if( actsw == '-' && 
           !strcmp( &sws[1], &((*argv)[2]) ) )
          {
          actsw = *(opts->set+1);
          sws = opts->set+1;
          }
       else
          {
          break;
          }
       }
     if( *sws == actsw )
       {
       report = 1;
       switch( *(opts->set) )
          {
       case '.':  /* Path variable */
       case ':':; /* Free string   */
       case '/':; /* User defined list */
       case '<':; /* Input filename without extension */
          argv++;
          argc--;
          if( !argc ) break;
          if( opts->var ) _strmcpy( (char *)(opts->var), *argv, opts->len );
          while( (opts+1)->name ) opts++;
          break;
       case '#':  /* Integer number */
          argv++;
          argc--;
          if( !argc ) break;
          if( opts->var ) sscanf( *argv, opts->form, opts->var );
          while( (opts+1)->name ) opts++;
          break;
       case '?':  /* Lookup */
          argv++;
          argc--;
          if( !argc ) break;
          if( opts->var ) 
            *((int *)(opts->var)) = lookdn( *argv, (PLOOKUP_T)(opts->form) );
          while( (opts+1)->name ) opts++;
          break;
       case '|':; /* Short Multisel/named bits */
       case '=':  /* Short Multicheck */
          argv++;
          argc--;
          if( !argc ) break;
          if( opts->var ) 
            *((int *)(opts->var)) |= lookdn( *argv, (PLOOKUP_T)(opts->form) );
          while( (opts+1)->name ) opts++;
          break;
       case '!':  /* Execute - post term */
          if( (doit = (OPTFUNC)(opts->var)) != NULL ) 
            {
            doit( argc, argv );   
            return 2;
            }
          while( (opts+1)->name ) opts++;
          break;
       case '*':  /* Execute */
          if( (doit = (OPTFUNC)(opts->var)) != NULL ) 
            report = doit( argc, argv );
          while( (opts+1)->name ) opts++;
          break;
       case '>':  /* Execute with 1 arg */
          argc--;
          argv++;
          if( !argc ) return report;
          if( (doarg= (ARGFUNC)(opts->var)) != NULL )
            report = doarg( *argv );
          while( (opts+1)->name ) opts++;
          break;
       case '+':  /* Set it true */
          if(opts->var) *((int *)(opts->var)) = 1;
          while( (opts+1)->name ) opts++;
          break;
       case '-':  /* Set it false */
          if(opts->var) *((int *)(opts->var)) = 0;
          while( (opts+1)->name ) opts++;
          break;
       case '^':  /* Toggle */
          if( !(opts->con & CON_DEBOUNCE) )
            {
            opts->con |= CON_DEBOUNCE;
            if(opts->var) *((int *)(opts->var)) ^= 1;
            }
          while( (opts+1)->name ) opts++;
          break;
          }
       break;
       }
     sws++;
     }
    opts++;
    }
   argc--;
   argv++;
   }
 else
   {
/* Scan for arguments (without leading - or --) */
   while( args->name )
    {
    if( *(args->set) == '@')
      {
      if( argc && args->var ) _strmcpy( (char *)(args->var), *argv, args->len );
      report++;
      args++;
      break;
      }
    args++;
    }
   argc--;
   argv++;
   }
 }
return report;
}
/* =============== Process command line options ==================== */
struct optentry *findopt(  char *name, struct optentry *options  )
{
char *q;
struct optentry *p = options;
if( !p ) return NULL;
q=strchr( name, '?' );
if(q) *q=0;
while( p->name )
  {
  if( !strcmp( name, p->name  ) ) goto Foundit; /* chek the name */
  if( !strcmp( name, p->set+2 ) ) goto Foundit; /* Check the long switches */
  if( !strcmp( name, p->set+4 ) ) goto Foundit;
  p++;
  }
if(q) *q='?';
return NULL;
Foundit:
  if(q) *q='?';
  return p;
}
/**/
static char currline[MAXLINELENGTH];
static int getcfreentrant = 0;
/* =============== Process a configuration file ==================== */
int getconfig( FILE *cf, struct optentry *options  )
{
char *p;
struct optentry *opts;
int  flag   = 0;
int  report = 0;
if( !options ) return 0;
if( !cf      ) return 0;
while( mgetline( cf, currline, 1 ) )
 {
 if(!currline[0]        ) continue;
 if( currline[0] == '#' ) continue;
 if( currline[0] == '[' && getcfreentrant ) return 0;
GotIt:
 p = currline;
 while( !strchr( " :=-\t", *p ) ) p++;
 *(p++) = '\0';
 while( strchr( " :=-\t", *p ) ) p++;
 opts = options;
 while( opts->name )
    {
    if( !strcmp( opts->name, currline ) )
       {
       report = 1;
       switch( *(opts->set) )
          {
       case '@':  /* Arguments */
       case ':':  /* Free format */
       case '.':  /* Path variable */
       case '/':  /* User defined list */
       case '<':; /* Input filename without extension */
          if( opts->var ) _strmcpy( (char *)(opts->var), p, opts->len );
          break;
       case '#':  /* Integer number */
          if( opts->var ) sscanf( p, opts->form, opts->var );
          break;
       case '?':  /* Lookup */
          if( opts->var ) 
            *((int *)(opts->var)) = lookdn( p, (PLOOKUP_T)(opts->form) );
          break;
       case '|': ;/* Multicheck */
       case '=':  /* Multisel */
          if( opts->var ) 
            *((int *)(opts->var)) |= lookdn( p, (PLOOKUP_T)(opts->form) );
          break;
       case '+': ;/* Set it true/false */
       case '-': ;
       case '^':
          if( strchr( "TtYy1", *p ) ||
              strchr( "nN", p[1]) )  flag = 1;
          else                       flag = 0;
          if(opts->var) *((int *)(opts->var)) = flag;
          break;
       case '$':  /* Subsystem  */
          if( opts->var )
            {
            getcfreentrant++;
            getconfig( cf, (struct optentry *)(opts->var) );
            getcfreentrant--;
            opts++;
            goto GotIt;
            }
          break;
          }
       }
    opts++;
    }
 }
return report;
}

/* =============== Returns the file length ======================== */
unsigned long filelen( FILE *inf )
{
unsigned long retval = 0LU;
if( !inf ) return retval;
fseek( inf, 0LU, SEEK_END );
retval = ftell( inf );
rewind( inf );
return retval;
}

/* =============== Read a line from file ========================== */
int mgetline( FILE *ifile, char *currline, int nolf )
{
int i;
char *t = currline;
*t = '\0';
i = 0;
Loop:
   *t = '\0';
   if( !fread( t, 1, 1, ifile ) ) return(0);
   if( *t )
     {
     if( *t == 0x0a ) { if(!nolf) t++; *t='\0'; return( 1 ); };
     if( *t == 0x0d && nolf ) goto Loop;
     if( i < MAXLINELENGTH-2 )
       {
       i++;
       t++;
       }
     goto Loop;
     }
   else
     {
     return(0);
     }
}

/* =============== Write a line to file =========================== */
int putline( FILE *ofile, char *currline, int u2dos )
{
int rep;
char *p;
int len = strlen(currline);
if( !len )   return(0);
if( !ofile ) return(0);
if( len )
 if( u2dos && (currline[len-1]=='\n') &&
     !( (len>1) && (currline[len-2] == '\r' )))
  {
  p = &(currline[len-1]);
  p[0] = '\r';
  p[1] = '\n';
  p[2] = '\0';
  len++;
  }
rep = fwrite( currline, len, 1, ofile );
return (rep == 1) ? 1 : 0;
}
/* =============== Table lookups ===================================*/
static char missed[] = "........ (unknown)";
char *lookup( int id,    PLOOKUP_T tab )
{
char *retval = missed;
sprintf( missed, "%.8X (unkown)", id );
if( !tab ) return retval;
while( tab->str )
  {
  if( id == tab->id ) return tab->str;
  tab++;
  }
return retval;
}
/**/
int lookdn( char *str,    PLOOKUP_T tab )
{
int retval = 0;
if( !tab ) return retval;
while( tab->str )
  {
  if( !strcasecmp( str, tab->str ) ) return tab->id;
  tab++;
  }
return retval;
}
/**/
int lookix( char *str,    PLOOKUP_T tab )
{
int i      =  0;
if( !tab ) return -1;
while( tab->str )
  {
  if( !strcasecmp( str, tab->str ) ) return i;
  i++;
  tab++;
  }
return -1;
}

/* =============== Utility string functions ====================== */
char *_strmcat( char *s1, char *s2, int upto )
{
int proceed = 0;
if( !s1 )       return NULL;
upto--;
if( upto <= 0 ) return NULL;
while( upto-- )
   {
   if( proceed )
     {
     if( !(*s2) ) break;
     *(s1++) = *(s2++);
     }
   else
     {
     if( !(*s1) )
       {
       proceed++;
       if( !s2 )       return s1+1;
       *(s1++) = *(s2++);
       continue;
       }
     s1++;
     }
   }
*s1 = '\0';
return s1;
}
/**/
char *_strmcpy( char *s1, char *s2, int upto )
{
if( !s1 )       return NULL;
*(s1) = '\0';
if( !s2 )       return s1;
upto--;
if( upto <= 0 ) return s1;
while( upto-- )
   {
   if( !*s2 ) break;
   *(s1++) = *(s2++);
   }
*s1 = '\0';
return s1;
}
/**/
char *_strmgetline( char *s1, char *s2, int upto )
{
if( !s1 )       return NULL;
*(s1) = '\0';
if( !s2 )       return NULL;
upto--;
if( upto <= 0 ) return NULL;
while( upto-- )
   {
   if( !*s2 )   return NULL;
   if(  *s2 == '\n' ) 
     {
     s2++;
     break;
     }
   *(s1++) = *(s2++);
   }
*s1 = '\0';
return s2;
}
/* Copy tesxt string, word wrapping, and indentation             */
/* s1: target, s2: source, upto: target length, ind: indentation */
/* wp: wrap position, bkind: break indicator characters          */
int   _strmcpwrap( char *s1, char *s2, int upto, int indp, int ind, int wp, char *bkind )
{
char *s1s;
char *s2s;
int   ip;
int   ips;
int   i,nlfs;
/**/
if( !s1 )       return 0;
*(s1) = '\0';
if( !s2 )       return 0;
upto--;
if( upto <= 0 ) return 0;
nlfs=0;
ip=indp;
if( ip<0 ) ip=-indp;
ips=0;
s1s=NULL;
s2s=NULL;
if( indp>0 ) while( upto-- && indp-- ) *(s1++) = ' ';
while( upto-- > 0 )
   {
   if( !*s2 ) break;
   if( strchr( bkind, *s2 ) )
     {
     s1s=s1;
     s2s=s2;
     ips=ip;
     }
   *(s1++) = *(s2++);
   ip++;
   if( *(s1-1) == '\n' )
     {
     for( i=0; i<ind; i++, upto-- ) *(s1++) = ' ';   /* Indent */
     s1s=s2s=NULL;
     ips=0;
     ip=ind;
     nlfs++;
     continue;
     }
   if( ip > wp )
     {
     if( s1s )    /* wrap smoothly. . . */
       {
       s1 = s1s+1;
       s2 = s2s+1;
       upto = upto + (ip=ips);
       ip = ips;
       }
     *(s1++) = '\n';
     for( i=0; i<ind; i++, upto-- ) *(s1++) = ' ';   /* Indent */
     s1s=s2s=NULL;
     ips=0;
     ip=ind;
     nlfs++;
     }
   }
if( *(s1-1) != '\n' ) *(s1++) = '\n';
*s1 = '\0';
return nlfs;
}
/**/
char *_strmovl( char *s1, char *s2, int upto )
{
if( !s1 )       return NULL;
if( !s2 )       return s1;
upto--;
if( upto <= 0 ) return s1;
while( upto-- )
   {
   if( !*s2 ) break;
   *(s1++) = *(s2++);
   }
return s1;
}
/**/
char *_strmfill( char *s1, char cf, int upto )
{
if( !s1 )       return NULL;
upto--;
if( upto <= 0 ) return s1;
while( upto-- ) *(s1++) = cf;
*s1 = '\0';
return s1;
}

int _lookup( char *str, char *table[] )
{
int i = 0;
if( !table )  return 0;
if( !str   )  return 0;
if( !*str   ) return 0;
while( *table )
     {
     if( !strcmp( str, *table ) ) return i;
     i++;
     table++;
     }
return 0;
}

int _mistrcmp( char *s1, char *s2 )
{
if( !s1 ) return 1;
if( !s2 ) return -1;
while( *s1 && *s2 && toupper(*s1) == toupper(*s2) )
    {
    s1++;
    s2++;
    }
return (*s1)-(*s2);
}

int _ilookup( char *str, char *table[] )
{
int i = 0;
if( !table )  return 0;
if( !str   )  return 0;
if( !*str   ) return 0;
while( *table )
     {
     if( !_mistrcmp( str, *table ) ) return i;
     i++;
     table++;
     }
return 0;
}

/* =============== Turn off all ON/OFF variable =================== */
void turnoffall( struct optentry *opts )
{
if( !opts ) return;
while( opts->name )
    {
    switch( *(opts->set) )
       {
    case '+':;     /* Bool variable  (toggle) */
    case '-':;
    case '^':
       if( opts->var ) *((int *)(opts->var)) = 0;
       break;
       }
    opts++;
    }
}
/* =============== Turn off console ON/OFF variable =============== */
int  turnoffcon( struct optentry *opts )
{
int changed  = 0;
if( !opts ) return 0;
while( opts->name )
    {
    switch( *(opts->set) )
       {
    case '+':;     /* Bool variable  (toggle) */
    case '-':;
    case '^':
       opts->con &= (~CON_DEBOUNCE );
       if( opts->var && opts->con ) *((int *)(opts->var)) = 0;
       changed = 1;
       break;
       }
    opts++;
    }
return changed;
}
/**/
/* =============== Logger functions =============================== */
#ifdef   __MINGW32__
void _prtuni( FILE *fout, char locus, char *fmt, __VALIST ap )
#else
void _prtuni( FILE *fout, char locus, char *fmt, va_list ap )
#endif /*__MINGW32__*/
{
char       p[34]                                                   ;
#ifdef   __MINGW32__
SYSTEMTIME t                                                       ;
#else
struct timeval t                                                   ;
struct tm     *pt                                                  ;
#endif /*__MINGW32__*/
if( !fout) return                                                  ;
if( !fmt ) return                                                  ;
if( !*fmt) return                                                  ;
/**/
#ifdef   __MINGW32__
GetLocalTime( &t )                                                 ;
sprintf( p, 
         "%.4u.%.2u.%.2u-%.2u:%.2u:%.2u.%.3u",
         t.wYear,t.wMonth,t.wDay,
         t.wHour,t.wMinute,t.wSecond,t.wMilliseconds )             ;
#else
gettimeofday( &t, NULL );
pt = localtime( &t.tv_sec );
p[0]=0;
if( pt )
   sprintf( p,
         "%.4u.%.2u.%.2u-%.2u:%.2u:%.2u.%.3u",
         pt->tm_year+1900,pt->tm_mon+1,pt->tm_mday,
         pt->tm_hour,pt->tm_min,pt->tm_sec,
         (unsigned int)(t.tv_usec/1000LU) )                        ;
#endif /*__MINGW32__*/
fprintf( fout, "%s %c ", p, locus )                                ;
/**/
vfprintf( fout, (const char *)fmt, ap )                            ;
fflush( fout )                                                     ;
}
/* ............... To standard IO ................................. */
void prtinf( char *fmt, ... )
{
va_list    ap                                                      ;
va_start( ap, fmt )                                                ;
_prtuni( stdout, 'I', fmt, ap )                                    ;
}
/**/
void prterr( char *fmt, ... )
{
va_list    ap                                                      ;
va_start( ap, fmt )                                                ;
_prtuni( stderr, 'E', fmt, ap )                                    ;
}
/**/
void prtwarn( char *fmt, ... )
{
va_list    ap                                                      ;
va_start( ap, fmt )                                                ;
_prtuni( stderr, 'W', fmt, ap )                                    ;
}
/**/
/* ............... To named file .................................. */
void loginf( char *logfn, char *fmt, ... )
{
va_list  ap                                                        ;
FILE    *fout = NULL                                               ;
if( logfn ) 
  if( *logfn )
      fout = fopen( logfn, "ab" )                                  ;
if( !fout ) fout = stdout                                          ;
va_start( ap, fmt )                                                ;
_prtuni( fout, 'I', fmt, ap )                                      ;
if( fout != stdout ) fclose( fout )                                ;
}
/**/
void logwarn( char *logfn, char *fmt, ... )
{
va_list  ap                                                        ;
FILE    *fout = NULL                                               ;
if( logfn ) 
  if( *logfn )
      fout = fopen( logfn, "ab" )                                  ;
if( !fout ) fout = stderr                                          ;
va_start( ap, fmt )                                                ;
_prtuni( fout, 'W', fmt, ap )                                      ;
if( fout != stderr ) fclose( fout )                                ;
}
/**/
void logerr( char *logfn, char *fmt, ... )
{
va_list  ap                                                        ;
FILE    *fout = NULL                                               ;
if( logfn ) 
  if( *logfn )
      fout = fopen( logfn, "ab" )                                  ;
if( !fout ) fout = stderr                                          ;
va_start( ap, fmt )                                                ;
_prtuni( fout, 'E', fmt, ap )                                      ;
if( fout != stderr ) fclose( fout )                                ;
}
/**/
unsigned long logtell( char *logfn )
{
unsigned long  retval = 0LU                                        ;
FILE          *fout   = NULL                                       ;
if( logfn ) 
  if( *logfn )
      fout = fopen( logfn, "rb" )                                  ;
if( !fout ) return retval                                          ;
fseek( fout, 0LU, SEEK_END );
retval = ftell( fout )                                             ;
fclose( fout )                                                     ;
return retval                                                      ;
}
/* ............... To opened file handle........................... */
void wrtinf( FILE *fout,  char *fmt, ... )
{
va_list  ap                                                        ;
if( !fout ) fout = stdout                                          ;
va_start( ap, fmt )                                                ;
_prtuni( fout, 'I', fmt, ap )                                      ;
}
/**/
void wrtwarn( FILE *fout,  char *fmt, ... )
{
va_list  ap                                                        ;
if( !fout ) fout = stderr                                          ;
va_start( ap, fmt )                                                ;
_prtuni( fout, 'W', fmt, ap )                                      ;
}
/**/
void wrterr( FILE *fout,  char *fmt, ... )
{
va_list  ap                                                        ;
if( !fout ) fout = stderr                                          ;
va_start( ap, fmt )                                                ;
_prtuni( fout, 'E', fmt, ap )                                      ;
}
/* =============== Other useful stuff============================== */
unsigned long uptick( void )
{
unsigned long ul;    
asm volatile( "rdtsc" :"=a"(ul) ::"edx" );
return ul;
}
/* ............... Safe fopen and close ........................... */
FILE *safe_fopen( char *name, char *newname, char *mode )
{
_strmcpy( newname, name,   MAXSTRLEN-4 );
_strmcat( newname, ".new", MAXSTRLEN-4 );
return( fopen(newname, mode) );
}
int safe_fclose( FILE *fout, char *name, char *newname )
{
if( fout )
  {
  fflush( fout );
  fclose( fout );
  }
#ifdef   __MINGW32__
DeleteFile( name );
return MoveFile( newname, name );
#else
remove( name );
return !rename( newname, name );
#endif /*__MINGW32__*/
}
/*..                                                                */
/*...................... END OF ..................OPTIONS.C ......  */
