/*................................................ ttysh.c .......  */
/*..                                                                */
static char  versionstring[511]= "ttysh version 1.1.4";
/*                                                                  */
/* ttysh.c - Teletype Shell (dumb terminal, and diagnostics)        */
/*                                                                  */
/* This version prepared for WIN32 (XP PRO SP2) and linux-x86-32    */
/*                                                                  */
/*  Application - Hungaro Digitel Satellite monitoring              */
/*                                                                  */
/*  Created:    2004.04.14  -  Gyorgy Horvath, DTMI-BUTE            */
/*  Modified:   2004.04.14  -  Gy.H. - As a part of LAWANCE system  */
/*  Modified:   2009.03.31  -  Gy.H. - Ported to Win32 (for CPE)    */
/*  Modified:   2009.09.21  -  Gy.H. - Common source (HDT sateve)   */
/*  Modified:   2009.10.01  -  Gy.H. - RTS/DTR/break added          */
/*  Modified:   2010.09.24  -  Gy.H. - Standalone tool              */
/*  Modified:   2020.06.22  -  Gy.H. - Timestamp markers added      */
/*                                                                  */
/*  Usage: ttysh device [options]                                   */
/*  Notes:                                                          */
/*   - device can be a COM port, e.g. \\.\com1                      */
/*   - defaults for COMx are 115200,8,N,1                           */
/*   - use Ctrl+Q (cQ) to escape by default                         */
/*   - do not merge switches (like -io)                             */
/*  Usage: Options:                                                 */
/*                                                                  */
/*     -e, -E, --escape <key> .... Escape character is              */
/*             cQ, cX, cs2, cA                                      */
/*             cB, cF, cG, cK                                       */
/*             cL, cR                                               */
/*     -n, -N, --num <num> .... Esc in numeric format               */
/*     -d, -D, --dtrflow .... DTR/DSR flow control                  */
/*     -r, -R, --rtsflow .... RTS/CTS flow control                  */
/*     --dtrstat <num> .... Set DTR state 0/1                       */
/*     --rtsstat <num> .... Set RTS state 0/1                       */
/*     -b, -B, --break .... Send break                              */
/*     -s, -S, --speed <key> .... Set the port speed to             */
/*             110, 300, 600, 1200                                  */
/*             2400, 4800, 9600, 14400                              */
/*             19200, 38400, 56000, 57600                           */
/*             115200, 128000, 256000                               */
/*     -p, -P, --parity <key> .... Set the parity to                */
/*             none, even, odd, mark                                */
/*     -t, -T, --stop <key> .... Set stop bits to                   */
/*             one, oneandhalf, two                                 */
/*     -c, -C <path> .... Capture to file                           */
/*     -w, -W <strg> .... Write string and exit                     */
/*     -m, -M, --marker . Time stamp markers on CR                  */
/*     --version .... Show version and exit                         */
/*     -h, -H, --help .... Help and exit                            */
/*                                                                  */
/*  Disclaimer   - (C) Gyorgy Horvaath - DTMI-BUTE - 2000-2009      */
/*..                                                                */
#ifdef   __MINGW32__
/* ================================================================= */
/* Win32 port ====================================================== */
/* ================================================================= */
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <fcntl.h>
#include <signal.h>
#include <windows.h>
#include "options.h"
/**/
int ttysh_help( int argc, char *argv[]  );
static int prversion( int argc, char *argv[]  )
{
printf( "%s\n", versionstring );
return 2;
}
/**/
static LOOKUP_T bdrtab[] =
{
{ CBR_110    , "110"        ,       },
{ CBR_300    , "300"        ,       },
{ CBR_600    , "600"        ,       },
{ CBR_1200   , "1200"       ,       },
{ CBR_2400   , "2400"       ,       },
{ CBR_4800   , "4800"       ,       },
{ CBR_9600   , "9600"       ,       },
{ CBR_14400  , "14400"      ,       },
{ CBR_19200  , "19200"      ,       },
{ CBR_38400  , "38400"      ,       },
{ CBR_56000  , "56000"      ,       },
{ CBR_57600  , "57600"      ,       },
{ CBR_115200 , "115200"     ,       },
{ CBR_128000 , "128000"     ,       },
{ CBR_256000 , "256000"     ,       },
{ 0x0000     , NULL,                },
};
static LOOKUP_T partab[] =
{
{ NOPARITY            , "none" , },
{ EVENPARITY          , "even" , },
{ ODDPARITY           , "odd"  , },
{ MARKPARITY          , "mark" , },
{ 0x0000              , NULL,    },
};
static LOOKUP_T stptab[] =
{
{ ONESTOPBIT          , "one"       , },
{ ONE5STOPBITS        , "oneandhalf", },
{ TWOSTOPBITS         , "two"       , },
{ 0x0000              , NULL        , },
};
static LOOKUP_T esctab[] =
{
{ 0x11    , "cQ"         ,       },
{ 0x18    , "cX"         ,       },
{ 0x00    , "cs2"        ,       },
{ 0x01    , "cA"         ,       },
{ 0x02    , "cB"         ,       },
{ 0x06    , "cF"         ,       },
{ 0x07    , "cG"         ,       },
{ 0x0b    , "cK"         ,       },
{ 0x0c    , "cL"         ,       },
{ 0x12    , "cR"         ,       },
{ 0x0000  , NULL,                },
};
static char devname[MAXSTRLEN] = "\\\\.\\com1";
static char cafname[MAXSTRLEN] = { 0, };
static char wstring[MAXSTRLEN] = { 0, };
static int  escchar  = 0x11;        /* Escape character is Ctrl+Q */
static int  dspeed   = CBR_115200;  /* COM settings: 115200,8,N,1 */
static int  dparity  = NOPARITY;
static int  dstop    = ONESTOPBIT;
static int  doption  = 0;           /* DTR/DSR handshake */
static int  roption  = 0;           /* RTS/CTS handshake */
static int  boption  = 0;           /* Send break        */
static int  rtsstat  = -1;          /* Set RTS state     */
static int  dtrstat  = -1;          /* Set DTR state     */
static int  moption  = 0;           /* Time stamp on CR  */
/**/
struct optentry ttyshoptions[] =
{
{ "escape" ,1, "?eE-escape", (char *)esctab, "Escape character is"  , (void *)(&escchar), 0,},
{ "numeric",1, "#nN-num"   ,         "%i"  , "Esc in numeric format", (void *)(&escchar), 0,},
{ "dtrflow",1, "+dD-dtrflow",        NULL  , "DTR/DSR flow control" , (void *)(&doption), 0,},
{ "rtsflow",1, "+rR-rtsflow",        NULL  , "RTS/CTS flow control" , (void *)(&roption), 0,},
{ "dtrstat",1, "#-dtrstat" ,         "%i"  , "Set DTR state 0/1"    , (void *)(&dtrstat), 0,},
{ "rtsstat",1, "#-rtsstat" ,         "%i"  , "Set RTS state 0/1"    , (void *)(&rtsstat), 0,},
{ "break"  ,1, "+bB-break" ,         NULL  , "Send break"           , (void *)(&boption), 0,},
{ "speed"  ,1, "?sS-speed" , (char *)bdrtab, "Set the port speed to", (void *)(&dspeed) , 0,},
{ "parity" ,1, "?pP-parity", (char *)partab, "Set the parity to"    , (void *)(&dparity), 0,},
{ "stop"   ,1, "?tT-stop"  , (char *)stptab, "Set stop bits to"     , (void *)(&dstop)  , 0,},
{ "capture",1, ".cC"       ,         "%s"  , "Capture to file"      , OPTSTRG(cafname)  ,   },
{ "write"  ,1, ":wW"       ,         "%s"  , "Write string and exit", OPTSTRG(wstring)  ,   },
{ "marker" ,1, "+mM-marker",         NULL  , "Timestamp markers CR" , (void *)(&moption), 0,},
{ "version",1, "!-version" ,         NULL  , "Show version and exit", (void *)prversion , 0,},
{ "help"   ,0, "!hH-help"  ,         NULL  , "Help and exit"        , (void *)ttysh_help, 0,},
{ "device" ,1, "@zZ"       ,         "%s"  , "Device name"          , OPTSTRG(devname)  ,   },
{ NULL     ,0, NULL        ,         NULL  , NULL                   , NULL              ,   },
};
/**/
HANDLE fdi = NULL;
HANDLE fdo = NULL;
HANDLE fds = NULL;
int    fdc = -1;
/**/
/**/
static OVERLAPPED statovl;
static HANDLE     statevt=NULL;
static int        ckfired=0;
/**/
static OVERLAPPED readovl;
static HANDLE     readevt=NULL;
/**/
static OVERLAPPED writeovl;
static HANDLE     writeevt=NULL;
/**/
DCB     dcb;
COMSTAT comstat;
DWORD   dwErr;
/**/
int w32_setcom( void )
/*  ~~~~~~~~~~                                                      */
{
COMMTIMEOUTS commtimeouts;
     dwErr = 0xfffffffflu;
     ClearCommError( fds, &dwErr, &comstat);
     ZeroMemory( &dcb, sizeof(dcb));
     SetupComm( fds, 2048, 2048 );
     SetCommMask( fds, EV_RXCHAR );
     PurgeComm( fds, PURGE_RXABORT | PURGE_TXABORT |
                            PURGE_RXCLEAR | PURGE_TXCLEAR);
     ZeroMemory(&commtimeouts, sizeof(commtimeouts));
     commtimeouts.ReadIntervalTimeout = MAXDWORD;
     commtimeouts.WriteTotalTimeoutConstant = 300;
     SetCommTimeouts( fds, &commtimeouts);
/**/
     GetCommState( fds, &dcb );
     dcb.BaudRate=dspeed;
     dcb.Parity  =dparity;
     dcb.StopBits=dstop;
     dcb.ByteSize=8;
     dcb.fBinary=1;
     dcb.fParity=0;
     dcb.fOutxCtsFlow=roption;
     dcb.fOutxDsrFlow=doption;
     dcb.fDtrControl=doption?DTR_CONTROL_ENABLE:DTR_CONTROL_DISABLE;
     dcb.fDsrSensitivity=0;
     dcb.fTXContinueOnXoff=0;
     dcb.fOutX=0;
     dcb.fInX=0;
     dcb.fErrorChar=0;
     dcb.fNull=0;
     dcb.fRtsControl=roption?RTS_CONTROL_ENABLE:RTS_CONTROL_DISABLE;
     dcb.fAbortOnError=0;
     dcb.XonLim=0;
     dcb.XoffLim=0;
     dcb.XonChar=0;
     dcb.XoffChar=0;
     dcb.ErrorChar=0;
     dcb.EofChar=0;
     dcb.EvtChar=0;
     SetCommState( fds, &dcb);
     dwErr = 0xfffffffflu;
     ClearCommError( fds, &dwErr, &comstat);
     if( rtsstat == 1 ) EscapeCommFunction( fds, SETRTS );
     if( rtsstat == 0 ) EscapeCommFunction( fds, CLRRTS );
     if( dtrstat == 1 ) EscapeCommFunction( fds, SETDTR );
     if( dtrstat == 0 ) EscapeCommFunction( fds, CLRDTR );
     if( boption      ) SetCommBreak( fds );
return 0;
}
/**/
int check_for_input( void )
/*  ~~~~~~~~~~~~~~~                                                 */
{
dwErr = 0xfffffffflu;
ClearCommError( fds, &dwErr, &comstat);
return comstat.cbInQue;
}
/**/
int w32_read( char *buff, int cnt )
/*  ~~~~~~~~                                                        */
{
DWORD err,cno;
if( !check_for_input( ) ) return 0;
if( ReadFile( fds, buff, cnt, &cno, &readovl ) )
  {
  return cno;
  }
err = GetLastError();
if( err != ERROR_IO_PENDING )
  {
  return -1;
  }
if( GetOverlappedResult( fds, &readovl, &cno, FALSE ) )
  {
  return cno;
  }
err = GetLastError();
return -1;
}
/**/
int w32_write( char *buff, int cnt )
/*  ~~~~~~~~~                                                       */
{
DWORD err,cno;
WriteFile( fds, buff, cnt, &cno, &writeovl );
if( GetOverlappedResult( fds, &writeovl, &cno, TRUE ) )
  {
  return cno;
  }
err = GetLastError();
return -1;
}
/**/
HANDLE w32_open( char *nam )
/*     ~~~~~~~~                                                     */
{
HANDLE rfd;

ckfired = 0;

if( !statevt )
  {
  statevt = CreateEvent( NULL,TRUE,FALSE,"statevt");
  if( !statevt ) return 0;
  }
ResetEvent( statevt );
ZeroMemory( &statovl, sizeof(statovl));
statovl.hEvent = statevt;

if( !readevt )
  {
  readevt = CreateEvent( NULL,TRUE,FALSE,"readevt");
  if( !readevt ) return 0;
  }
ResetEvent( readevt );
ZeroMemory( &readovl, sizeof(readovl));
readovl.hEvent = readevt;

if( !writeevt )
  {
  writeevt = CreateEvent( NULL,TRUE,FALSE,"writeevt");
  if( !writeevt ) return 0;
  }
ResetEvent( writeevt );
ZeroMemory( &writeovl, sizeof(writeovl));
writeovl.hEvent = writeevt;
/**/
rfd = CreateFile(    nam,
                     GENERIC_READ | GENERIC_WRITE,
                     0, (LPSECURITY_ATTRIBUTES)NULL,
                     OPEN_EXISTING, 
                     FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
                     (HANDLE)NULL  );

if ( (HANDLE)rfd == INVALID_HANDLE_VALUE ) return 0;
return rfd;
}
/**/
int ttysh_help( int argc, char *argv[]  )
/*  ~~~~~~~~~~                                                      */
{
printf( "\n"                                              );
printf( "Usage: ttysh device [options]\n"                 );
printf( "Notes:\n"                                        );
printf( " - device can be a COM port, e.g. \\\\.\\com1\n" );
printf( " - defaults for COMx are 115200,8,N,1\n"         );
printf( " - use Ctrl+Q (cQ) to escape by default\n"       );
printf( " - do not merge switches (like -io)\n"           );
prtusage( NULL,
          "Options:\n", ttyshoptions );
return 1;
}
char movie[16] = ".oOMOo.-\\|/-.*W*";
int  mf = 0;
static char sCR  [] = { 0x0d, 0x00, };
static char sNL  [] = { 0x0a, 0x00, };
static char sTAB [] = { 0x09, 0x00, };
/* ------------------------------ */
int main( int argc, char *argv[] )
/* ------------------------------ */
{
int report;
INPUT_RECORD ir[10];
DWORD cnt;
unsigned char uch1,uch2[1024];
char *p;
/* * * * * */
if( argc<2  ) { ttysh_help( 0, NULL ); return 0; }
report = getoptions( argc, argv, ttyshoptions  );
if( report == 2 )
  {
  return -1;
  }
if( !report  )
  {
  ttysh_help( 0, NULL );
  return  1;
  }
/* * * * * */
fdi = GetStdHandle( STD_INPUT_HANDLE );
fdo = GetStdHandle( STD_OUTPUT_HANDLE );
/* Open serial port */
fds = w32_open( argv[1] );
if( !fds )
  {
  perror("openserial" );
  return  2;
  }
w32_setcom( );
/* Open the caputre file - if any */
if( cafname[0] )
  {
  fdc = open( cafname, O_CREAT|O_TRUNC|O_RDWR );
  if( fdc < 0 ) perror( "capture file" );
  }
//w32_read( uch2, 1024 );
/* Check if a single turn string write is commencing. . .*/
if( wstring[0] )
  {
  p = wstring;
  while( *p )
     {
     if( *p == '\\' )
       {
       p++;
       if( !*p ) break;
       if( *p == 'r' ) w32_write(  sCR, 1  );
       if( *p == 'n' ) w32_write(  sNL, 1  );
       if( *p == 't' ) w32_write(  sTAB, 1 );
       }
     else if( *p == '\"' )
       {
       p++;
       continue;
       }
     else
       {
       w32_write(  p, 1 );
       }
     report = check_for_input();
     if( report )
       {
       w32_read( uch2, report ) ;                 /* Echo and reports */
       fwrite( uch2, report, 1, stdout );         /* to stdout        */
       if( fdc >= 0 ) write( fdc, uch2, report ); /* to capture file  */
       }
     p++;
     }
  }
else
/* See stdin, and serial device input if they have something */
do  {
    if( (report = w32_read( uch2, 1024 )) > 0 )
      {
      uch2[report] = 0;
      fwrite( uch2, report, 1, stdout );
      if( fdc >= 0 ) write( fdc, uch2, report );
      }
    if( GetNumberOfConsoleInputEvents( fdi, &cnt ) )
    if( cnt )
    if( ReadConsoleInput( fdi, ir, 1, &cnt ) )
    if( cnt )
    if( ir[0].EventType == KEY_EVENT && ir[0].Event.KeyEvent.bKeyDown )
    if( ir[0].Event.KeyEvent.uChar.AsciiChar )
      {
      if( boption )
        {
        ClearCommBreak( fds );
        boption=0;
        }
      uch1 = ir[0].Event.KeyEvent.uChar.AsciiChar;
      if( (uch1 == escchar) ) break;
      w32_write(  &uch1, 1 );
      }
    } while( 1 );
WaitForSingleObject( statevt, 56 );
report = check_for_input();
if( report )
  {
  w32_read( uch2, report ) ; 
  fwrite( uch2, report, 1, stdout );         /* to stdout        */
  if( fdc >= 0 ) write( fdc, uch2, report ); /* to capture file  */
  }
if( fds  )
  {
  CloseHandle( fds );
  }
if( fdc >= 0 ) close( fdc );
printf( "\n" );
return 0;
}
#else  /*__MINGW32__ */
/* ================================================================= */
/* Linux port ====================================================== */
/* ================================================================= */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <time.h>
#include <fcntl.h>
#include <signal.h>
#include <termios.h>
#include <unistd.h>
#include "options.h"
/**/
static int eat_this = 0;
struct timeval stamp;
char stbuff[256];
/**/
int ttysh_help( int argc, char *argv[]  );
static int prversion( int argc, char *argv[]  )
{
printf( "%s\n", versionstring );
return 2;
}
/**/
static LOOKUP_T bdrtab[] =
{
{ B50     , "50"         ,       },
{ B75     , "75"         ,       },
{ B110    , "110"        ,       },
{ B134    , "134"        ,       },
{ B150    , "150"        ,       },
{ B200    , "200"        ,       },
{ B300    , "300"        ,       },
{ B600    , "600"        ,       },
{ B1200   , "1200"       ,       },
{ B2400   , "2400"       ,       },
{ B4800   , "4800"       ,       },
{ B9600   , "9600"       ,       },
{ B19200  , "19200"      ,       },
{ B38400  , "38400"      ,       },
{ B57600  , "57600"      ,       },
{ B115200 , "115200"     ,       },
{ 0x0000  , NULL,                },
};
static LOOKUP_T partab[] =
{
{ 0                   , "none" , },
{ PARENB              , "even" , },
{ PARENB | PARODD     , "odd"  , },
{ 0x0000              , NULL,    },
};
static LOOKUP_T stptab[] =
{
{ 0                   , "one"  , },
{ CSTOPB              , "two"  , },
{ 0x0000              , NULL,    },
};
static LOOKUP_T esctab[] =
{
{ 0x11    , "cQ"         ,       },
{ 0x18    , "cX"         ,       },
{ 0x00    , "cs2"        ,       },
{ 0x01    , "cA"         ,       },
{ 0x02    , "cB"         ,       },
{ 0x06    , "cF"         ,       },
{ 0x07    , "cG"         ,       },
{ 0x0b    , "cK"         ,       },
{ 0x0c    , "cL"         ,       },
{ 0x12    , "cR"         ,       },
{ 0x0000  , NULL,                },
};
static char devname[MAXSTRLEN] = "/dev/ttyS0";
static char cafname[MAXSTRLEN] = { 0, };
static char wstring[MAXSTRLEN] = { 0, };
static int  escchar  = 0x11;        /* Escape character is Ctrl+Q */
static int  dspeed   = B115200;     /* COM settings: 115200,8,N,1 */
static int  dparity  =  0;
static int  dstop    =  0;
static int  ioption  =  0;
static int  ooption  =  0;
static int  yoption  =  0;
static int  doption  = 0;           /* DTR/DSR handshake */
static int  roption  = 0;           /* RTS/CTS handshake */
static int  boption  = 0;           /* Send break        */
static int  rtsstat  = -1;          /* Set RTS state     */
static int  dtrstat  = -1;          /* Set DTR state     */
static int  moption  = 0;           /* Time stamp on CR  */
struct optentry ttyshoptions[] =
{
{ "escape" ,1, "?eE-escape", (char *)esctab, "Escape character is"  , (void *)(&escchar), 0,},
{ "numeric",1, "#nN-num"   ,         "%i"  , "Esc in numeric format", (void *)(&escchar), 0,},
{ "speed"  ,1, "?sS-speed" , (char *)bdrtab, "Set the port speed to", (void *)(&dspeed) , 0,},
{ "parity" ,1, "?pP-parity", (char *)partab, "Set the parity to"    , (void *)(&dparity), 0,},
{ "stop"   ,1, "?tT-stop"  , (char *)stptab, "Set stop bits to"     , (void *)(&dstop)  , 0,},
{ "dtrflow",1, "+dD-dtrflow",        NULL  , "DTR/DSR flow control" , (void *)(&doption), 0,},
{ "rtsflow",1, "+rR-rtsflow",        NULL  , "RTS/CTS flow control" , (void *)(&roption), 0,},
{ "dtrstat",1, "#-dtrstat" ,         "%i"  , "Set DTR state 0/1"    , (void *)(&dtrstat), 0,},
{ "rtsstat",1, "#-rtsstat" ,         "%i"  , "Set RTS state 0/1"    , (void *)(&rtsstat), 0,},
{ "break"  ,1, "+bB-break" ,         NULL  , "Send break"           , (void *)(&boption), 0,},
{ "ipipe"  ,1, "+iI-ipipe" ,         NULL  , "Input is piped"       , (void *)(&ioption), 0,},
{ "opipe"  ,1, "+oO-opipe" ,         NULL  , "Output is piped"      , (void *)(&ooption), 0,},
{ "capture",1, ".cC"       ,         "%s"  , "Capture to file"      , OPTSTRG(cafname) ,    },
{ "write"  ,1, ":wW"       ,         "%s"  , "Write string and exit", OPTSTRG(wstring) ,    },
{ "bypass" ,1, "+-bypass"  ,         NULL  , "Bypass errors"        , (void *)(&yoption), 0,},
{ "marker" ,1, "+mM-marker",         NULL  , "Timestamp markers CR" , (void *)(&moption), 0,},
{ "version",1, "!-version" ,         NULL  , "Show version and exit", (void *)prversion , 0,},
{ "help"   ,0, "!hH-help"  ,         NULL  , "Help and exit"        , (void *)ttysh_help, 0,},
{ "device" ,1, "@zZ"       ,         "%s"  , "Device name"          , OPTSTRG(devname)  ,   },
{ NULL     ,0, NULL        ,         NULL  , NULL                   , NULL              ,   },
};
/**/
struct termios tisav;
struct termios tosav;
struct termios tssav;
int    fdi = STDIN_FILENO;
int    fdo = STDOUT_FILENO;
int    fds = -1;
int    fdc = -1;
/**/
void terminatorII( int sig )
/*  ~~~~~~~~~~                                                      */
{
if( !ioption ) tcsetattr(fdi, TCSANOW, &tisav);
if( !ooption ) tcsetattr(fdo, TCSANOW, &tosav);
if( fds != -1 )
  {
  tcsetattr(fds, TCSANOW, &tssav);
  close( fds );
  }
if( fdc >= 0 ) close( fdc );
exit(0);
}
/**/
int ttysh_help( int argc, char *argv[]  )
/*  ~~~~~~~~~~                                                      */
{
printf( "\n"                                              );
printf( "Usage: ttysh device [options]\n"                 );
printf( "Notes:\n"                                        );
printf( " - device can be a COM port, e.g. /dev/ttyS0\n"  );
printf( " - defaults for COMx are 115200,8,N,1\n"         );
printf( " - use Ctrl+Q (cQ) to escape by default\n"       );
printf( " - do not merge switches (like -io)\n"           );
prtusage( NULL,
          "Options:\n", ttyshoptions );
return 1;
}
/**/
static int sRTS = TIOCM_RTS;
static int sDTR = TIOCM_DTR;
/**/
static char sCR  [] = { 0x0d, 0x00, };
static char sNL  [] = { 0x0a, 0x00, };
static char sTAB [] = { 0x09, 0x00, };
static struct timeval to = { 0, 110000,};
/* ------------------------------ */
int main( int argc, char *argv[] )
/* ------------------------------ */
{
char *p;
struct sigaction termaction;
int retval;
int report;
struct termios t;
fd_set rfds;
unsigned char uch1,uch2;
/* * * * * */
if( argc<2  ) { ttysh_help( 0, NULL ); return 0; }
report = getoptions( argc, argv, ttyshoptions  );
if( report == 2 )
  {
  return -1;
  }
if( !report  )
  {
  ttysh_help( 0, NULL );
  return -1;
  }
/* * * * * */
if( !ioption )
 if( tcgetattr(fdi, &tisav) < 0)
   {
   perror("tcgetattr");
   if( !yoption ) return -1;
   }
if( !ooption )
 if( tcgetattr(fdo, &tosav) < 0)
   {
   perror("tcgetattr");
   if( !yoption ) return -1;
   }
/* Open serial port */
fds = open( argv[1], O_RDWR | O_NOCTTY /* | O_NDELAY */ );
if( fds == -1 )
  {
  perror("openserial" );
  return -1;
  }
if( tcgetattr(fds, &tssav) < 0)
   {
   perror("tcgetattr");
   if( !yoption ) return -1;
   }
/* Install the signal handler for SIGTERM */
memset( &termaction, 0, sizeof(termaction) );
sigemptyset(&termaction.sa_mask);
termaction.sa_handler = terminatorII;
sigaction(SIGTERM, &termaction, NULL);
sigaction(SIGQUIT, &termaction, NULL);
/* Demolish stdio */
t=tisav;
t.c_iflag &= ( ~IXON & ~IXOFF & ~INLCR & ~IGNCR & ~ICRNL );
t.c_lflag &= ( ~ICANON & ~ECHO & ~ISIG );
t.c_oflag &= ( ~OLCUC & ~ONLCR & ~OCRNL & ~ONLRET );
if( !ioption )
 if( tcsetattr(fdi, TCSANOW, &t) < 0)
  {
  perror("tcsetattr");
  if( !yoption ) return -1;
  }
t=tosav;
t.c_iflag &= ( ~IXON & ~IXOFF & ~INLCR & ~IGNCR & ~ICRNL );
t.c_lflag &= ( ~ICANON & ~ECHO & ~ISIG );
t.c_oflag &= ( ~OLCUC & ~ONLCR & ~OCRNL & ~ONLRET );
if( !ooption )
 if( tcsetattr(fdo, TCSANOW, &t) < 0)
  {
  perror("tcsetattr");
  if( !yoption ) return -1;
  }
/* Prep. the serial port */
t=tssav;
cfsetispeed( &t, dspeed );
cfsetospeed( &t, dspeed );
t.c_cflag &= ( ~CSIZE & ~CSTOPB & ~PARENB & ~HUPCL & ~CRTSCTS );
t.c_cflag |= ( CS8 | CLOCAL | CREAD | HUPCL | dstop | dparity);
if( doption ) t.c_cflag |= CRTSCTS;
if( roption ) t.c_cflag |= CRTSCTS;
t.c_lflag &= ( ~ICANON & ~ECHO & ~ISIG );
t.c_iflag &= ( ~IXON & ~IXOFF & ~INLCR & ~IGNCR & ~ICRNL );
t.c_oflag &= ( ~OLCUC & ~ONLCR & ~OCRNL & ~ONLRET );
if( tcsetattr(fds, TCSANOW, &t) < 0)
  {
  perror("tcsetattr");
  if( !yoption ) return -1;
  }
if( rtsstat == 1 ) ioctl( fds, TIOCMBIS, &sRTS );
if( rtsstat == 0 ) ioctl( fds, TIOCMBIC, &sRTS );
if( dtrstat == 1 ) ioctl( fds, TIOCMBIS, &sDTR );
if( dtrstat == 0 ) ioctl( fds, TIOCMBIC, &sDTR );
if( boption      ) ioctl( fds, TIOCSBRK );
/* Open the caputre file */
if( cafname[0] )
  {
  fdc = open( cafname, O_CREAT|O_TRUNC|O_RDWR, 0755 );
  if( fdc < 0 ) perror( "capture file" );
  }
/* Check if a single turn string write is commencing. . .*/
if( wstring[0] )
  {
  retval = 0;
  p = wstring;
  while( *p || (retval>0) )
     {
     if( *p )
       {
       if( *p == '\\' )
         {
         p++;
         if( *p )
           {
           if( *p == 'r' ) eat_this += write(  fds, sCR, 1  );
           if( *p == 'n' ) eat_this += write(  fds, sNL, 1  );
           if( *p == 't' ) eat_this += write(  fds, sTAB, 1 );
           if( *p == '"' ) eat_this += write(  fds, "\"", 1 );
           }
         p++;
         }
       else if( *p == '\"' )
         {
         p++;
         p++;
         }
       else
         {
         eat_this += write( fds, p, 1 );
         p++;
         }
       }
     FD_ZERO(&rfds);
     FD_SET( fds, &rfds);
     retval = select( fds+1, &rfds, NULL, NULL, &to );
     if( retval>0 )
       {
       if( FD_ISSET( fds, &rfds ) )
        if( read(  fds, &uch2, 1 ) > 0 )
         {
         eat_this += write( fdo, &uch2, 1 );
         if( fdc >= 0 ) eat_this += write( fdc, &uch2, 1 ); /* to capture file  */
         }
       }
     }
  }
else
/* See stdin, and serial device input if they have something */
 do {
    FD_ZERO(&rfds);
    FD_SET( fds, &rfds);
    FD_SET( STDIN_FILENO, &rfds);
    retval = select( fds+1, &rfds, NULL, NULL, &to );
    if( retval > 0 )
     {
     if( FD_ISSET( fds, &rfds ) )
      if( read(  fds, &uch2, 1 ) )
          {
          eat_this += write( fdo, &uch2, 1 );
          if( moption && (uch2 == '\r') && (fdc >= 0))
            {
            gettimeofday(&stamp,NULL);
            sprintf( stbuff, "\n%.8lX:%.6lu ", stamp.tv_sec, stamp.tv_usec );
            eat_this += write( fdc, stbuff, strlen(stbuff) );
            }
          else
            {
            if( fdc >= 0 ) eat_this += write( fdc, &uch2, 1 );
            }
          }
     if( FD_ISSET( STDIN_FILENO, &rfds ) )
      {
      if( read( fdi, &uch1, 1 ) )
        {
        if( boption )
          {
          ioctl( fds, TIOCCBRK );
          boption = 0;
          }
        if( (uch1 == escchar) && !ioption ) break;
        eat_this += write( fds, &uch1, 1 );
        }
      else
        {
        if( ioption ) break;
        }
      }
     }
    } while( 1 );
if( !ioption ) tcsetattr(fdi, TCSANOW, &tisav);
if( !ooption ) tcsetattr(fdo, TCSANOW, &tosav);
if( fds != -1 )
  {
  tcsetattr(fds, TCSAFLUSH, &tssav);
  close( fds );
  }
if( fdc >= 0 ) close( fdc );
printf( "\n" );
return 0;
}
#endif /*__MINGW32__ */
/*..................... END OF ................... ttysh.c ......  */
