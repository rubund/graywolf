/*
 *   Copyright (C) 1989-1991 Yale University
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
FILE:	    autoflow.c 
DESCRIPTION:This file contains routines for following the flow.
CONTENTS:   auto_flow()
	    exec_single_prog()
	    report_problem( adjptr )
		ADJPTR adjptr ;
	    BOOL check_dependencies( adjptr )
		ADJPTR adjptr ;
	    autoflow_set_object( object )
		INT object ;
DATE:	    May  7, 1989 - original coding.
REVISIONS:  May 25, 1989 - added needtoExecute to handle case of no
		input file but an output file.
	    Jun 19, 1989 - added $ to be substitute for designName.
	    Jun 21, 1989 - fixed bug in auto flow code concerning
		multiple edges on a single program execution.
	    Nov 23, 1990 - now exit status is correct if a program
		experiences trouble.
	    Fri Feb 22 23:55:29 EST 1991 - fixed new library rename.
	    Sun Apr 21 22:33:53 EDT 1991 - now allow optional files.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) autoflow.c version 2.4 4/21/91" ;
#endif

#include <string.h>
#include <yalecad/base.h>
#include <yalecad/message.h>
#include <yalecad/debug.h>
#include <globals.h>

#define STARTOBJECT  0          /* start of the graph */
#define ERROR        -1         /* error from YgetFileTime() */
static INT objectS ;            /* the last program that was run */

auto_flow()
{

    ADJPTR     adjptr ;         /* current edge in graph */
    ADJPTR     make_decision(); /* decides which way to travel */
    OBJECTPTR  o ;              /* current object */

    objectS = STARTOBJECT ;
    problemsG = FALSE ;

    unmark_edges() ; /* set all program edges to be unmarked */

    while( autoflowG ){ /* loop until done */


	o = proGraphG[objectS] ;
	if( o->numedges > 1 ){
	    adjptr = make_decision( o, FORWARD ) ;
	} else if( o->numedges == 1 ){
	    ASSERT( o->adjF, "auto_flow", "Null edge pointer\n" ) ;
	    adjptr = o->adjF ;
	} else {
	    /* now edges we are done */
	    break ;
	}
	/* new object to be executed */
	objectS = adjptr->node ;

	/* tell graphics selected object and draw it */
	G( graphics_set_object( objectS ) ) ;
	G( draw_the_data() ) ;

	if( check_dependencies( adjptr ) ){
	    /* program files are out of date execute program */
	    if( executePgm( adjptr ) ){
		/* we received a non zero return code break loop */
		/* report problem */
		report_problem( adjptr ) ;
		break ;
	    }
	}

        /* allow user to change things */
	/* G( ) is NOGRAPHICS conditional compile */
	G( if( graphicsG && TWinterupt() ) ){
	    G( process_graphics() ) ;
	}
	
    } /* end autoflow loop */
    G( draw_the_data() ) ;

} /* end autoflow */

exec_single_prog()
{
    ADJPTR     adjptr ;         /* current edge in graph */
    ADJPTR     get_edge_from_user(); /* decides which way to travel */
    ADJPTR     findEdge() ;     /* find the edge for the given nodes */
    OBJECTPTR  o ;              /* current object */
    char       filename[LRECL] ;/* buffer for filename */
    FPTR       fdepend ;        /* current file in question */
    INT        numedges ;       /* count backward edges */

    o = proGraphG[objectS] ;
    /* get backwards edge count */
    numedges = 0 ;
    for( adjptr = o->adjB; adjptr; adjptr = adjptr->next ){
	numedges++ ;
    }
    if( numedges > 1 ){
	adjptr = get_edge_from_user( o, BACKWARD ) ;
    } else if( numedges == 1 ){
	ASSERT( o->adjB, "auto_flow", "Null edge pointer\n" ) ;
	adjptr = o->adjB ;
    } else {
	/* now edges we are done */
	return ;
    }

    /* tell graphics selected object and draw it */
    G( graphics_set_object( objectS ) ) ;
    G( draw_the_data() ) ;

    /* now find forward edge */
    adjptr = findEdge( adjptr->node, objectS, FORWARD ) ;

    /* check to see all input files exist */
    for( fdepend = adjptr->ifiles;fdepend; fdepend = fdepend->next ){
	ASSERTNCONT( fdepend->fname, "auto_flow","Null file name\n");
	if( *fdepend->fname == '$' ){
	    /* suffix keyword */
	    sprintf( filename, "%s%s", cktNameG, fdepend->fname+1 ) ;
	} else {
	    strcpy( filename, fdepend->fname ) ;
	}
	if(!(YfileExists( filename,TRUE )) && !(fdepend->optional) ){
	    sprintf( YmsgG, "ERROR:input file %s does not exist",
		filename ) ;
	    G( TWmessage( YmsgG ) ) ;
	    M( ERRMSG, NULL, YmsgG ) ;
	    M( MSG, NULL, "\n" ) ;
	    return ;
	}
    } /* end check of all input files */

    if( executePgm( adjptr ) ){
	/* we received a non zero return code break loop */
	report_problem( adjptr ) ;
    }
    G( draw_the_data() ) ;
} /* end exec_single_prog */

report_problem( adjptr )
ADJPTR adjptr ;
{
    sprintf( YmsgG, "Trouble executing %s", 
	proGraphG[adjptr->node]->name)  ;
    G( TWmessage( YmsgG ) ) ;
    problemsG = TRUE ;
} /* end report_problem */

/* returns true if files are out of date - false otherwise */
BOOL check_dependencies( adjptr )
ADJPTR adjptr ;
{
    INT        input_time ;     /* last input file which was modified */
    INT        output_time ;    /* first output file */
    INT        ftime ;          /* time file was modified */
    INT        YgetFileTime() ; /* returns last time file was modified */
    char       filename[LRECL] ;/* buffer for filename */
    FPTR       fdepend ;        /* current file in question */
    BOOL       needtoExecute ;  /* TRUE if an output file doesnt exist */

    /* check dependencies */
    /* find the input file which was modified last */
    input_time = INT_MIN ;
    for( fdepend = adjptr->ifiles;fdepend; fdepend = fdepend->next ){
	ASSERTNCONT( fdepend->fname, "auto_flow","Null file name\n");
	if( *fdepend->fname == '$' ){
	    /* suffix keyword */
	    sprintf( filename, "%s%s", cktNameG, fdepend->fname+1 ) ;
	} else {
	    strcpy( filename, fdepend->fname ) ;
	}
	if(!(YfileExists( filename,TRUE ))){
	    continue ;
	}
	ftime = YgetFileTime( filename ) ;
	if( ftime == ERROR ){
	    Ymessage_error_count() ;
	}
	input_time = MAX( ftime, input_time ) ;
    }

    /* find the output file which was modified the earliest */
    output_time = INT_MAX ;
    /* ***************************************************************
    *  Conditions for program execution:
    *  1. Input files have later date than output files.
    *  2. An output file does not exist.
    *  3. No given output files program is assumed to always be executed.
    *  ************************************************************* */
    /* first make check if no output files exist program is executed */
    if( fdepend = adjptr->ofiles ){
	/* program may not need to be executed */
	needtoExecute = FALSE ;
    } else {
	/* always needs to be executed */
	needtoExecute = TRUE ;
    }
    for( ; fdepend; fdepend = fdepend->next ){
	ASSERTNCONT( fdepend->fname,"auto_flow","Null file name\n" );
	if( *fdepend->fname == '$' ){
	    sprintf( filename, "%s%s", cktNameG, fdepend->fname+1 ) ;
	} else {
	    strcpy( filename, fdepend->fname ) ;
	}
	if(!(YfileExists( filename,TRUE ))){
	    /* one of the output files doesn't exist */
	    needtoExecute = TRUE ;
	    continue ;
	}
	ftime = YgetFileTime( filename ) ;
	if( ftime == ERROR ){
	    Ymessage_error_count() ;
	}
	output_time = MIN( ftime, output_time ) ;
    }

    if( output_time < input_time || needtoExecute ){
	/* files are out of date - program needs to be executed */
	return( TRUE ) ;
    }
    return( FALSE ) ;

} /* end BOOL check_dependencies */

/* allow graphics loop to change the object */
autoflow_set_object( object )
INT object ;
{
    objectS = object ;
} /* end autoflow_set_object */
