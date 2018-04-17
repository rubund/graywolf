/*
 *   Copyright (C) 1990 Yale University
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
FILE:	    system.c                                       
DESCRIPTION:system routines
DATE:	    Apr 26, 1990 
REVISIONS:  May 12, 1990 - added move file and getenv.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) system.c version 3.4 8/28/90" ;
#endif

#include <yalecad/file.h>
#include <yalecad/message.h>

INT Ysystem( program, abortFlag, exec_statement, abort_func )
char *program ;
BOOL abortFlag ;
char *exec_statement ;
INT  (*abort_func)() ;
{
    INT status ;        /* return status from program */

    if( status = system( exec_statement ) ){
	/* get status from exit routine */
	status = (status & 0x0000FF00) >> 8 ;/* return code in 2nd byte */
	/* now determine the program */

	sprintf( YmsgG, "Program %s returned with exit code:%d\n",program,
	    status );
	M( ERRMSG, NULL, YmsgG ) ;
	if( abort_func ){
	    (*abort_func)() ;
	}
	if( abortFlag ){
	    YexitPgm( PGMFAIL ) ; /* exit the program */
	}
	return( status ) ;
    } 
    return( 0 ) ;
} /* end Ysystem */

YcopyFile( sourcefile, destfile )
char *sourcefile, *destfile ;
{
    sprintf( YmsgG, "/bin/cp %s %s", sourcefile, destfile ) ;
    Ysystem( "Ylib/YcopyFile", ABORT, YmsgG, NULL ) ;
} /* end Ycopyfile */

YmoveFile( sourcefile, destfile )
char *sourcefile, *destfile ;
{
    sprintf( YmsgG, "/bin/mv %s %s", sourcefile, destfile ) ;
    Ysystem( "Ylib/YmoveFile", ABORT, YmsgG, NULL ) ;
} /* end Ycopyfile */

Yrm_files( files )
char *files ;
{
    sprintf( YmsgG, "/bin/rm -rf %s", files ) ;
    Ysystem( "Ylib/Yrm_files", NOABORT, YmsgG, NULL ) ;
} /* end Ycopyfile */

char *Ygetenv( env_var )
char *env_var ;
{
    char *getenv() ;

    return( (char *) getenv( env_var ) ) ;

} /* end Ygetenv */
