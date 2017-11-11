/*
 *   Copyright (C) 1988-1991 Yale University
 *
 *   This work is distributed in the hope that it will be useful; you can
 *   redistribute it and/or modify it under the terms of the
 *   GNU General Public License as published by the Free Software Foundation;
 *   either version 2 of the License,
 *   or any later version, on the following conditions:
 *
 *   (a) YALE MAKES NO, AND EXPRESSLY DISCLAIMS
 *   ALL, REPRESENTATIONS OR WARRANTIES THAT THE MANUFACTURE, USE, PRACTICE,
 *   SALE OR
 *   OTHER DISPOSAL OF THE SOFTWARE DOES NOT OR WILL NOT INFRINGE UPON ANY
 *   PATENT OR
 *   OTHER RIGHTS NOT VESTED IN YALE.
 *
 *   (b) YALE MAKES NO, AND EXPRESSLY DISCLAIMS ALL, REPRESENTATIONS AND
 *   WARRANTIES
 *   WHATSOEVER WITH RESPECT TO THE SOFTWARE, EITHER EXPRESS OR IMPLIED,
 *   INCLUDING,
 *   BUT NOT LIMITED TO, WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A
 *   PARTICULAR
 *   PURPOSE.
 *
 *   (c) LICENSEE SHALL MAKE NO STATEMENTS, REPRESENTATION OR WARRANTIES
 *   WHATSOEVER TO
 *   ANY THIRD PARTIES THAT ARE INCONSISTENT WITH THE DISCLAIMERS BY YALE IN
 *   ARTICLE
 *   (a) AND (b) above.
 *
 *   (d) IN NO EVENT SHALL YALE, OR ITS TRUSTEES, DIRECTORS, OFFICERS,
 *   EMPLOYEES AND
 *   AFFILIATES BE LIABLE FOR DAMAGES OF ANY KIND, INCLUDING ECONOMIC DAMAGE OR
 *   INJURY TO PROPERTY AND LOST PROFITS, REGARDLESS OF WHETHER YALE SHALL BE
 *   ADVISED, SHALL HAVE OTHER REASON TO KNOW, OR IN FACT SHALL KNOW OF THE
 *   POSSIBILITY OF THE FOREGOING.
 *
 */

/* ----------------------------------------------------------------- 
FILE:	    message.c                                       
DESCRIPTION:message processing routines
	    The object of these routines is to allow the user flexibility
	    in redirecting the output of messages to the screen as well
	    as keeping track of program errors and warnings.
CONTENTS:   Ymessage_print( messageType, routine, messageString )
		int messageType ;
		char *routine, *messageString ;
	    int Ymessage_get_warncount()
	    int Ymessage_get_errorcount()
	    Ymessage_warncount()
	    Ymessage_errorcount()
	    Ymessage_output( messageString )
		char *messageString ;
	    Ymessage_init( fileptr )
		FILE *fileptr ;
	    Ymessage_mode( mode )
		int mode ;
	    Ymessage_flush()
	    Ymessage_close()

DATE:	    Jan 29, 1988 
REVISIONS:  Sep 30, 1988 - added incWarningCount & incErrorCount
	    Jan 27, 1989 - changed msg to msgG so that use can tell
		it is global variable.
	    Jan 29, 1989 - took \n out of printf statement 
	    Apr 27, 1989 - changed to Y prefix.
	    May  3, 1989 - changed msgG to YmsgG to be consistent.
	    Aug  8, 1989 - fixed problem with double write to 
		to output file.
	    Wed Jan 30 14:10:52 EST 1991 - now output warning messages
		to the screen also.
	    Fri Feb 15 15:21:57 EST 1991 - added modes to message
		system.  Modified names to make it easier.
	    Thu Mar  7 01:27:48 EST 1991 - now always flush stdout.
----------------------------------------------------------------- */
#include <globals.h>

int errorCountS = 0;
int warningCountS = 0;
FILE *foutS = NULL ;         /* open output file */
FILE *outS = NULL ;          /* redirected out file */
BOOL verboseS = FALSE ;
BOOL modeS = M_NORMAL ;
char message_bufS[LRECL] ;
char typeS[8] ;

/* global variable definition */
char *YmsgG = message_bufS ;

void Ymessage_print( int messageType, char *routine, char *messageString )
{

	FILE *out ; /* where to output result if verbose */

	/* determine message type */
	switch (messageType){
		case MSG:typeS[0] = EOS;  /* no need for an explanation */
		break ;
		case ERRMSG:sprintf(typeS,"ERROR");
			if( routine ){
				/* error message should always have routine */
				/* continuation of a message doesn't count */
				errorCountS++;
			}
		break ;
		case WARNMSG:sprintf(typeS,"WARNING");
			if( routine ){
				/* warning message should always have routine */
				/* continuation of a message doesn't count as new */
				/* warning message */
				warningCountS++;
			}
		break ;
		case DBGMSG:sprintf(typeS,"DEBUG");
		break ;
	}

	if( modeS == M_SILENT ){
		if( messageType != ERRMSG && messageType != WARNMSG ){
		/* eat the message in this case */
		return ;
		}
	}

	/* no file installed send to screen */
	if( messageString && routine ){
		printf( "%s[%s]:%s",typeS,routine,messageString);
	} else if( messageString ){
		printf( "%s",messageString );
	}

} /* end message_print */

/* increment the static variable */
void Ymessage_warn_count()
{
    warningCountS++ ;
} /* end message_warn_count */

/* increment the static variable */
void Ymessage_error_count()
{
    errorCountS++ ;
}/* end Ymessage_error_count */

int Ymessage_get_warncount()
{
    return(warningCountS);
} /*end Ymessage_get_warncount */

int Ymessage_get_errorcount()
{
    return(errorCountS);
} /*end Ymessage_get_errorcount */

void Ymessage_output(char *messageString)
{
    if( verboseS ){
	fprintf( outS, "%s", messageString ) ;
    }
    if( foutS ){
	fprintf( foutS, "%s", messageString ) ;
    }

} /* end message_output */

void Ymessage_init( FILE *fileptr )
{
     foutS = fileptr ;
} /* end Ymessage_init */

void Ymessage_mode( int mode )
{
    if( mode == M_VERBOSE ){
	verboseS = TRUE ;
        outS = stdout ;
	modeS = M_VERBOSE ;
    } else if( mode == M_NORMAL ){
	verboseS = FALSE ;
        outS = NULL ;
	modeS = M_NORMAL ;
    } else if( mode == M_SILENT ){
	verboseS = FALSE ;
        outS = NULL ;
	modeS = M_SILENT ;
    }
} /* end YverboseMessage */

BOOL Ymessage_get_mode()
{
    return( modeS ) ;
} /* end Ymessage_get_mode */

void Ymessage_flush()
{
	if( outS ){
		fflush(outS) ;
	}
	if( foutS ){
		fflush(foutS) ;
	}
	fflush( stdout ) ;
} /* end Ymessage_flush */

void Ymessage_close()
{
	if( foutS ){
		TWCLOSE(foutS) ;
		foutS = (FILE *)NULL;
	}
	if( outS ){
		TWCLOSE(outS) ;
		outS = (FILE *)NULL;
	}
} /* end Ymessage_close */

