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
FILE:	    plot.c
DESCRIPTION:Plot routines for Cricket Graph.  
CONTENTS:  
DATE:	    Feb 13, 1988 REVISIONS:  Jan 29, 1989 - changed msg to YmsgG.
	    Feb 01, 1989 - move all graphic routines to graphics.c
	    Mar 30, 1989 - changed tile datastructure.
	    Apr 29, 1990 - added message.h
	    Fri Jan 18 18:38:36 PST 1991 - fixed to run on AIX.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) plot.c version 1.6 12/9/91" ;
#endif

#include <stdarg.h>
#include <string.h>
#include <yalecad/base.h>
#include <yalecad/debug.h>
#include <yalecad/message.h>
#include <yalecad/file.h>
#include <yalecad/string.h>

#define MAXARGS 20
typedef struct {
    char  fileName[LRECL] ;
    FILE  *fp ;
    BOOL  headPrintedAlready ;
    BOOL  graphFlushed ;
} YPLOTTYPE, *YPLOTPTR ;

static YPLOTTYPE gfileS[MAXARGS] ;
static INT gfilenoS = 0 ;
static BOOL graphFilesS = TRUE ;


static INT findType();

Yplot_control( toggle )
BOOL toggle ;
{
    graphFilesS = toggle ;
} /* end YgraphControl */

/* graph init uses variable number of arguments */
Yplot_init( int dval, ... )
{

    va_list ap ;
    char *graphName;
    YPLOTPTR gptr ;

    va_start(ap, dval) ;

    if( !(graphFilesS) ){
	/* don't do anything if flag is not set */
	va_end(ap) ;
	return ;
    }

    while( graphName = va_arg( ap, char * ) ){
	/* save graph file name */
	gptr = &(gfileS[gfilenoS++]) ;
	sprintf( gptr->fileName,"%s",graphName ) ;
	gptr->fp = TWOPEN( graphName, "w", ABORT ) ;
	gptr->headPrintedAlready = FALSE ;
	gptr->graphFlushed = TRUE ;
    }
    va_end(ap) ;
}

/* graph init uses variable number of arguments */
Yplot_heading( int dval, ... )
{

    va_list ap ;
    char *gName, *varName ;
    YPLOTPTR gptr ;
    INT i ;
    FILE *fp ;

    va_start(ap, dval) ;

    if( !(graphFilesS) ){
	/* don't do anything if flag is not set */
	va_end(ap) ;
	return ;
    }

    /* first argument is always graph file name */
    if( !(gName = va_arg( ap, char * ) )){
	M(ERRMSG,"GRAPH","problem with first argument\n" ) ;
	va_end(ap) ;
	return ;
    } else {
	/* find matching file */
	for( i=0;i<gfilenoS;i++ ){
	    if( strcmp( gName, gfileS[i].fileName ) == STRINGEQ ){
		if( gfileS[i].headPrintedAlready ){
		    return ;
		}
		fp = gfileS[i].fp ;
		fprintf( fp, "*\n" ) ;
		break ;
	    }
	}
	if( i >= gfilenoS ){
	    sprintf( YmsgG,
		"couldn't find file %s name in opened file list\n" ) ;
	    M(ERRMSG,"GRAPH", YmsgG ) ;
	    return ;
	}
    }
    gfileS[i].headPrintedAlready = TRUE ;
    while( varName = va_arg( ap, char * ) ){
	fprintf( fp, "%s\t", varName ) ;
    }
    fprintf( fp, "\n" ) ;
    va_end(ap) ;
}

Yplot_close()
{
    INT i ;

    if( !(graphFilesS) ){
	/* don't do anything if flag is not set */
	return ;
    }

    for( i = 0; i<gfilenoS;i++ ){
	TWCLOSE( gfileS[i].fp ) ;
    }
}

#define NULL_TYPE 0
#define INT_TYPE  1
#define CHAR_TYPE 2
#define STRG_TYPE 3
#define DOUB_TYPE 4

/* This is what argument list looks like - use it to pass any type */
/* of variable to graph */
/* GRAPH( graphFileName, xVarformat, xVar, yVarformat, yVars... ) */ 
Yplot( int dval, ... ) 
{
    va_list ap ;
    char *gName ;
    char *control ;
    char *format ;
    char gchar ;
    char *gstr ;
    char **tokenBuf ;
    INT gint ;
    INT i , type, numtokens ;
    DOUBLE gdoub ;
    FILE *fp ;
    static char copyformatS[LRECL] ;
    /* static INT findType();*/

    va_start(ap, dval) ;
    if( !(graphFilesS) ){
	/* don't do anything if flag is not set */
	va_end(ap) ;
	return ;
    }

    /* first argument is always graph file name */
    if( !(gName = va_arg( ap, char * ) )){
	M(ERRMSG,"GRAPH","problem with first argument\n" ) ;
	va_end(ap) ;
	return ;
    } else {
	/* find matching file */
	for( i=0;i<gfilenoS;i++ ){
	    if( strcmp( gName, gfileS[i].fileName ) == STRINGEQ ){
		fp = gfileS[i].fp ;
		break ;
	    }
	}
	if( i >= gfilenoS ){
	    sprintf( YmsgG,
		"couldn't find file %s name in opened file list\n" ) ;
	    M(ERRMSG,"GRAPH", YmsgG ) ;
	    return ;
	}
    }

    /* GRAPH( graphFileName, xVarformat, xVar, yVarformat, yVars... ) */ 
    /* second argument is control for xvariable */
    if( !(control = va_arg( ap, char * ) )){
	M(ERRMSG,"GRAPH","problem with third argument\n" ) ;
	va_end(ap) ;
	return ;
    }
    /* need to make copy of format since scanner is destructive */
    sprintf( copyformatS,"%s",control ) ;
    tokenBuf = Ystrparser( copyformatS," \n\t\\", &numtokens ) ;
    if( !(type = findType(tokenBuf,0)) ){
	M(ERRMSG,"GRAPH","Unknown control type.\n" ) ;
	va_end(ap) ;
	return ;
    }
    /* only print if graph has previously been flushed */
    /* now that we have type we can get third element */
    switch( type ){
    case INT_TYPE:
	gint = va_arg( ap, INT ) ;
	if( gfileS[i].graphFlushed ){  
	    fprintf( fp, tokenBuf[0], gint ) ;
	    fprintf( fp, "\t" ) ;
	}
	break ;
    case CHAR_TYPE:
#ifdef linux
	gchar = (char) va_arg( ap, int ) ;
#else
	gchar = va_arg( ap, char ) ;
#endif
	if( gfileS[i].graphFlushed ){  
	    fprintf( fp, tokenBuf[0], gchar ) ;
	    fprintf( fp, "\t" ) ;
	}
	break ;
    case STRG_TYPE:
	gstr = va_arg( ap, char * ) ;
	if( gfileS[i].graphFlushed ){  
	    fprintf( fp, tokenBuf[0], gstr ) ;
	    fprintf( fp, "\t" ) ;
	}
	break ;
    case DOUB_TYPE:
	gdoub = va_arg( ap, DOUBLE ) ;
	if( gfileS[i].graphFlushed ){  
	    fprintf( fp, tokenBuf[0], gdoub ) ;
	    fprintf( fp, "\t" ) ;
	}
	break ;
    }
    gfileS[i].graphFlushed = FALSE ;
	
	    
    /* fourth argument is control for yvariable */
    if( !(format = va_arg( ap, char * ) )){
	M(ERRMSG,"GRAPH","problem with third argument\n" ) ;
	va_end(ap) ;
	return ;
    }
    /* need to make copy of format since scanner is destructive */
    sprintf( copyformatS,"%s",format ) ;
    tokenBuf = Ystrparser( copyformatS," \n\t\\", &numtokens ) ;
    for( i = 0; i< numtokens; i++ ){
	if( !(type = findType(tokenBuf,i)) ){
	    M(ERRMSG,"GRAPH","Unknown control type.\n" ) ;
	    va_end(ap) ;
	    return ;
	}
	switch( type ){
	    case INT_TYPE:
		gint = va_arg( ap, INT ) ;
		fprintf( fp, tokenBuf[i], gint ) ;
		break ;
	    case CHAR_TYPE:
#ifdef linux
		gchar = (char) va_arg( ap, int ) ;
#else
		gchar = va_arg( ap, char ) ;
#endif
		fprintf( fp, tokenBuf[i], gchar ) ;
		break ;
	    case STRG_TYPE:
		gstr = va_arg( ap, char * ) ;
		fprintf( fp, tokenBuf[i], gstr ) ;
		break ;
	    case DOUB_TYPE:
		gdoub = va_arg( ap, DOUBLE ) ;
		fprintf( fp, tokenBuf[i], gdoub ) ;
		break ;
	}
	fprintf( fp, "\t" ) ;
    }
    va_end(ap) ;

}

Yplot_flush( gName ) 
char *gName ;
{
    INT i ;

    if( !(graphFilesS) ){
	/* don't do anything if flag is not set */
	return ;
    }
    if( gName ){ 
	for( i=0;i<gfilenoS;i++ ){
	    if( strcmp( gName, gfileS[i].fileName ) == STRINGEQ ){
		if(!(gfileS[i].graphFlushed)){
		    /* only flush if we have to */
		    fprintf( gfileS[i].fp, "\n" ) ;
		    fflush( gfileS[i].fp ) ;
		    gfileS[i].graphFlushed = TRUE ;
		}
		return ;
	    }
	}
	if( i >= gfilenoS ){
	    sprintf( YmsgG,
		"couldn't find file %s name in opened file list\n" ) ;
	    M(ERRMSG,"GRAPH", YmsgG ) ;
	    return ;
	}
    } else { /* if null flush all files */
	for( i=0; i< gfilenoS ; i++ ){
	    if(!(gfileS[i].graphFlushed)){
		/* only flush if we have to */
		fprintf( gfileS[i].fp, "\n" ) ;
		fflush( gfileS[i].fp ) ;
		gfileS[i].graphFlushed = TRUE ;
	    }
	}
    }

} /* end GRAPHFLUSH */

static INT findType( control, number )
char **control ;
INT number ;
{
    char *formatChar ;
    
    formatChar = control[number] ;
    if( formatChar ){

	if( strchr(formatChar,'d') ){
	    return( INT_TYPE ) ;
	} else if( strchr(formatChar,'f') ){
	    return( DOUB_TYPE ) ;
	} else if( strchr(formatChar,'e') ){
	    return( DOUB_TYPE ) ;
	} else if( strchr(formatChar,'s') ){
	    return( STRG_TYPE ) ;
	} else if( strchr(formatChar,'c') ){
	    return( CHAR_TYPE ) ;
	}
    }
    return( NULL_TYPE ) ;
} /* end findType */


#ifdef TEST

main()
{
    INT i ;       /* counter */
    DOUBLE f ;    /* function value */
    INT y ;       /* integer function value */

    /* first initialize two graphs */
    Yplot_init( 0, "graph1", "graph_kroy", NULL ) ;

    /* -------------------------------------------------------------
       Now set the heading for the graph.
       YgraphHeading can be call multiple times but first time will
       be what is output to the graph file.  Remember the Nulls at end.
    ---------------------------------------------------------------- */
    for( i = 1; i <= 10; i++ ){
	/* set the first graph heading. It is good to keep heading with */
	/* output so it is easy to read the arguments to Ygraph */
	Yplot_heading( 0, "graph1", "x_axis", "f(x)", "y2", NULL ) ;
	/* -------------------------------------------------------------
	   Ygraph has the following format:
	   Ygraph( filename, x format, x varible, y format, y variables...
	------------------------------------------------------------- */
	f = (DOUBLE) i ;
	y = i * i ;
	Yplot( 0, "graph1", "%d", i, "%4.2le %d", f, y ) ;
	/* now after each graph has been finished flush data */
	Yplot_flush( "graph1" ) ;

	/* now output another graph */
	Yplot_heading( 0, "graph_kroy", "iter", "Temperature", NULL ) ;
	Yplot( 0, "graph_kroy", "%4.2le", (DOUBLE) i, "%d", 3 * i ) ;
	Yplot_flush( "graph_kroy" ) ;
    }

    /* When we are done with the graph close them */
    Yplot_close() ;

    exit(0) ;
}

#endif /* TEST */
    
