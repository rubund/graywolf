/*
 *   Copyright (C) 1989-1992 Yale University
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
FILE:	    ydebug.c                                       
DESCRIPTION:This file turns on and off debug testing.  Used by assert 
and D() routines to determine whether to perform debug testing.  At 
any time, debug can be turned on or off if the debug has been compiled
into the code.  This gives uses maximum flexibility for debug can be
loaded into code using -DDEBUG if asserts and D() statements are used.
They remain dormant in code except for Ydebug() call until debugFlagS
is turned on.  For production code, -DDEBUG should not be used.
Each routine is stored in a red-black tree during the execution.
CONTENTS:  Ydebug() 
	   YsetDebug( flag ) 
DATE:	   Apr 9, 1989 
REVISIONS: Apr 10, 1990 - rewrote debug routines so that each individual
    debug routine has its own name and can be turned off and on without
    recompiling.  At the end of a execution, it updates the dbg file.
    Also moved name to ydebug because of a dbx bug.
	    Apr 28, 1990 - added YdebugAssert so that assertions
		are not added to the tree.
	    Fri Jan 18 18:38:36 PST 1991 - fixed to run on AIX.
	    Fri Feb 22 23:40:32 EST 1991 - changed default to suit
		Richard.
	    Wed Apr 17 23:33:17 EDT 1991 - renamed treeS to debug_treeS
		so that debugging would be easier.  Stupid dbx.
	    Mon Aug 12 15:54:00 CDT 1991 - update for new
		Yrbtree_init.
	    Sun Nov  3 13:13:12 EST 1991 - fixed gcc complaints.
	    Tue Feb  4 15:31:28 EST 1992 - added return_code variable to
		Ydebug so you can switch it in the debugger if necessary.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) ydebug.c (Yale) version 3.15 2/7/92" ;
#endif

#include <yalecad/base.h>
#include <yalecad/rbtree.h>
#include <yalecad/file.h>
#include <yalecad/message.h>
#include <yalecad/string.h>

#define DBGFILE "dbg"

typedef struct {
    char *routine ;
    BOOL debugOn ;
} ROUTINE, *ROUTINEPTR ;

static BOOL debugFlagS = FALSE ;
static BOOL firstTimeS = FALSE ;      /* first time we ran program */
static YTREEPTR debug_treeS ;

/* ********* STATIC DEFINITIONS ************ */
static INT compare_routine( P2(ROUTINEPTR key1, ROUTINEPTR key2 ) ) ;
static ROUTINEPTR make_data_debug( P2(char *string, BOOL debugOn ) ) ;

BOOL Ydebug( routine ) 
char *routine ;
{   
    ROUTINEPTR data ;                 /* the matched data record */
    ROUTINE routine_key ;             /* used to test key */
    BOOL return_code ;		      /* return code so we can set in debugger */

    if( debugFlagS ){
	return_code = FALSE ;
	if( routine ){
	    routine_key.routine = routine ;
	    if( data = (ROUTINEPTR) 
		Yrbtree_search( debug_treeS, (char *) &(routine_key) ) ){
		if( data->debugOn ){
		    return_code = TRUE ;
		}
	    } else { /* add it to the tree */
		if( firstTimeS ){
		    /* the default is for it to be off */
		    data = make_data_debug( routine, FALSE ) ;
		    Yrbtree_insert( debug_treeS, (char *) data ) ;
		} else {
		    /* when adding new routines to an existing file */
		    /* the default is for it to be off */
		    data = make_data_debug( routine, FALSE ) ;
		    Yrbtree_insert( debug_treeS, (char *) data ) ;
		}
	    }
	} else {
	    fprintf( stderr, "No debug routine name specified here\n" ) ;
	}
    } else {
	return_code = FALSE ;
    }

    return( return_code ) ;

} /* end Ydebug */

/* ASSERTIONS are always on */
BOOL YdebugAssert() 
{   
    return( debugFlagS ) ;
} /* end YdebugAssert */

YdebugWrite()
{
    ROUTINEPTR data ;              /* the data in the tree */
    FILE *fp ;                   /* write to the debug file */

    if( debugFlagS ){
	
	if( YfileExists( DBGFILE ) ){
	    /* move to .bak to save a copy of the file */
	    sprintf( YmsgG, "/bin/cp %s %s.bak", DBGFILE,DBGFILE ) ;
	    system( YmsgG ) ;
	}
	/* now write a debug file */
	fp = TWOPEN( DBGFILE, "w", ABORT ) ;
	for( data = (ROUTINEPTR) Yrbtree_enumerate(debug_treeS,TRUE);data; 
	    data = (ROUTINEPTR) Yrbtree_enumerate(debug_treeS,FALSE)){

	    fprintf( fp, "%s %d\n", data->routine, data->debugOn ) ;
	}
	TWCLOSE( fp ) ;
    }
}

YsetDebug( flag )
BOOL flag ;
{

    char buffer[LRECL], *bufferptr ;
    char **tokens ;         /* for parsing menu file */
    INT  numtokens ;        /* number of tokens on the line */
    INT  line ;             /* count lines in input file */
    FILE *fp ;              /* open file pointer */
    ROUTINEPTR data ;       /* new data to be stored */

    if( flag ){
	/* initialize tree of routine name */
	YRBTREE_INIT( debug_treeS, compare_routine );
	line = 0 ;
	if( fp = TWOPEN( DBGFILE, "r", NOABORT ) ){
	    while( bufferptr=fgets(buffer,LRECL,fp )){
		/* parse file */
		line ++ ; /* increment line number */
		tokens = Ystrparser( bufferptr, " \t\n", &numtokens );

		if( numtokens == 2 ){
		    data = make_data_debug( tokens[0], atoi(tokens[1]) ) ;
		    Yrbtree_insert( debug_treeS, (char *) data ) ;
		} else {
		    sprintf( YmsgG, "Syntax error on line:%d\n", line ) ;
		    M(ERRMSG, "YsetDebug", YmsgG ) ;
		} 
	    }
	    TWCLOSE( fp ) ;
	} else {
	    firstTimeS = TRUE ;
	}
    }
    debugFlagS = flag ;
} /* end YsetDebug */

static INT compare_routine( key1, key2 )
ROUTINEPTR key1, key2 ;
{
    return( strcmp( key1->routine, key2->routine ) ) ;
} /* end compare */

static ROUTINEPTR make_data_debug( string, debugOn )
char *string ;
BOOL debugOn ;
{
    ROUTINEPTR data ;

    data = YMALLOC( 1, ROUTINE ) ;
    data->routine = Ystrclone( string ) ;
    data->debugOn = debugOn ;
    return( data ) ;
} /* end make_data_debug */

YfixDebug( ptr, type )
char *ptr ;
INT type ;
{
    switch( type ){
    case 0: /* integer */
	printf( "%d\n", (INT) ptr ) ;
	break ;
    case 1: /* string */
	printf( "%s\n", ptr ) ;
	break ;
    case 2: /* hexidecimal */
	printf( "%0x\n", (INT) ptr ) ;
	break ;
    }
}
