/* ----------------------------------------------------------------- 
FILE:	    message.h                                       
DESCRIPTION:This file contains include file for message screen output
	    routines.  Various macros are defined in order that the
	    user may substitute their own input/output routines.
CONTENTS:   MACRO DEFINITIONS
DATE:	    Feb  2, 1988 
	    static char SccsId[] = "@(#) message.h version 1.8 9/16/91" ;
REVISIONS:  Jan 27, 1989 - changed msg to YmsgG to denote that it is
		global variable.
	    Apr 10, 1990 - now automatically add base.h for user.
	    Fri Feb 15 15:40:07 EST 1991 - renamed message functions.
	    Mon Sep 16 22:18:40 EDT 1991 - added missing extern.
----------------------------------------------------------------- */
/* message.h definitions for message system */

#ifndef MESSAGE_H
#define MESSAGE_H
  
#include <yalecad/base.h>

#define MSG        0
#define ERRMSG     1
#define WARNMSG    2
#define DBGMSG     3

#define M_NORMAL   0
#define M_VERBOSE  1
#define M_SILENT   2

/* global definitions in message.c */
extern char *YmsgG ;

/* use printMessage for the message system at this time */
#define M(x,y,z)   (Ymessage_print( x,(char *)y,(char *)z))
#define FLUSHOUT() (Ymessage_flush())

/* define the output functions */
/* ----------------------------------------------------------------- 
    Although seems pain in the neck to have a specific output macro
    for each number of file output,  compiler will warn us of argument
    mismatch.  This could also be done with vargs function but it is
    unknown if this is portable over all machines and compilers.
*/
#define OUT9(a,b,c,d,e,f,g,h,i)  { sprintf(YmsgG,a,b,c,d,e,f,g,h,i) ; \
                                   Ymessage_output(YmsgG) ;                \
				 }
#define OUT8(a,b,c,d,e,f,g,h)    { sprintf(YmsgG,a,b,c,d,e,f,g,h)   ; \
                                   Ymessage_output(YmsgG) ;                \
				 }
#define OUT7(a,b,c,d,e,f,g)      { sprintf(YmsgG,a,b,c,d,e,f,g)     ; \
                                   Ymessage_output(YmsgG) ;                \
				 }
#define OUT6(a,b,c,d,e,f)        { sprintf(YmsgG,a,b,c,d,e,f)       ; \
                                   Ymessage_output(YmsgG) ;                \
				 }
#define OUT5(a,b,c,d,e)          { sprintf(YmsgG,a,b,c,d,e)         ; \
                                   Ymessage_output(YmsgG) ;                \
				 }
#define OUT4(a,b,c,d)            { sprintf(YmsgG,a,b,c,d)           ; \
                                   Ymessage_output(YmsgG) ;                \
				 }
#define OUT3(a,b,c)              { sprintf(YmsgG,a,b,c)             ; \
                                   Ymessage_output(YmsgG) ;                \
				 }
#define OUT2(a,b)                { sprintf(YmsgG,a,b)               ; \
                                   Ymessage_output(YmsgG) ;                \
				 }
#define OUT1(a)                  { sprintf(YmsgG,a)                 ; \
                                   Ymessage_output(YmsgG) ;                \
				 }
/* catch errors with this one */
#define OUT()                    { sprintf(YmsgG,a)                 ; \
                                   Ymessage_output(YmsgG) ;                \
				 }

/* message routines */
extern Ymessage_print( P3(INT messageType, char *routine, char *message) ) ;
/* 
Function:
    Print a message to screen and/or to a file.  There are the following
    types of messages:
    TYPE  MODE                      ACTION
    ----  ----                      ------
    MSG   Normal                    Written to stdout.
	  File Redirect             Written to file.
          File Redirect & verbose   Written to stdout and to given file.
          Silent                    Ignored.
          Silent && File Redirect   Written to file.
 ERRMSG   Normal                    Written to stderr. Error count incremented.
	  File Redirect             Written to file & stderr. Error count inc.
          File Redirect & verbose   Written to file & stderr. Error count inc.
          Silent                    Written to stderr. Error count incremented.
          Silent && File Redirect   Written to file & stderr. Error count inc.
 DBGMSG   Normal                    Written to stdout.
	  File Redirect             Written to file.
          File Redirect & verbose   Written to stdout and to given file.
          Silent                    Ignored.
          Silent && File Redirect   Written to file.
WARNMSG   Normal                    Written to stderr.  Warning count inc.
	  File Redirect             Written to file.    Warning count inc.
          File Redirect & verbose   Written to stderr and to given file. WC inc.
          Silent                    Written to stderr. WC incremented.
          Silent && File Redirect   Written to file & stderr. WC inc.
    YmsgG is a global pointer to a character buffer workspace.
*/

extern Ymessage_warn_count() ;
/*
Function:
    Increment the warning count.
*/

extern Ymessage_error_count() ;
/*
Function:
    Increment the error count.
*/

extern INT Ymessage_get_warncount() ;
/*
Function:
    Returns the warning count.
*/

extern INT Ymessage_get_errorcount() ;
/*
Function:
    Returns the error count.
*/

extern Ymessage_init( P1(FILE *fileptr) ) ;
/*
Function:
    Redirects the messages to the given file.
*/

extern Ymessage_output( P1(char *messageString ) ) ;
/* 
Function:
    Output message to screen if verbose mode has been set or
    output message to a file if message have been redirected. Otherwise
    it does nothing.  Used in the OUT macros.
*/

extern Ymessage_mode( P1(INT mode) ) ;
/* 
Function:
    Set the message mode.  It may be one of M_VERBOSE, M_NORMAL, or
    M_SILENT.
*/

extern BOOL Ymessage_get_mode() ;
/* 
Function:
    Returns the state of the message mode switch.
*/

extern Ymessage_flush() ;
/* 
Function:
    Flush the buffered output.
*/

extern Ymessage_close() ;
/* 
Function:
    Close the output streams.
*/

#endif /* MESSAGE_H */
