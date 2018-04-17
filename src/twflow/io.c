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
FILE:	    io.c
DESCRIPTION:This file contains routines for handling I/O.  The routines
	    in this file build the graph.
CONTENTS:   init( numobj )
		INT numobj ;
	    add_object( pname, node )
		char *pname ;
		INT node ;
	    add_pdependency( fromNode ) 
		INT fromNode ;
	    add_path( pathname )
		char *pathname ;
	    set_file_type( type )
		BOOL type ;
	    add_fdependency( file ) 
		char *file ;
	    add_args( argument )
		char *argument ;
	    add_box( l, b, r, t )
		INT l, b, r, t ;
	    start_edge( fromNode )
		INT fromNode ;
	    add_line( x1, y1, x2, y2 )
		INT x1, y1, x2, y2 ;
	    ADJPTR findEdge( from, to, direction )
		INT from, to ;
		BOOL direction ;
	    process_arcs()
	    unmark_edges()
	    set_window()
DATE:	    May  7, 1989 
REVISIONS:  Jun 19, 1989 - added unmark_edges to insure auto_flow
		is initialized correctly.  Made list of nodes
		in same order as user gave so that program will
		be executed FIFO.
	    Sun Apr 21 22:35:40 EDT 1991 - now allow optional files.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) io.c version 2.2 4/21/91" ;
#endif

#include <string.h>
#include <globals.h>
#include <yalecad/debug.h>
#include <yalecad/file.h>

#define START     "start"
#define OPTIONAL  '*'

typedef struct ebox {
    INT from ;                           /* from node for edge */
    INT to ;                             /* to node for edge */
    char **argv ;                        /* argument vector */
    INT  argc ;                          /* argument count */
    FPTR ifiles ;                        /* list of files */
    FPTR ofiles ;                        /* list of files */
    DPTR dlist ;                         /* list of drawables */
    struct ebox *next ;                  /* list of drawables */
} EDGEBOX, *EDGEPTR ;

static INT curObjS ;                     /* counts the current object */
static INT boxLS, boxRS, boxTS, boxBS ;  /* bounding box of objects */
static BOOL errorFlagS = FALSE ;         /* records fault condition */
static BOOL inputNotOutputS ;            /* the current filetype */
static OBJECTPTR objS ;                  /* pointer to current obj */
static ADJPTR botEdgeS ;                 /* the current pointer */
static EDGEPTR edgeListS = NULL ;        /* list of drawable edges */

/* ***************** ERROR HANDLING ****************************** */
/* ERRORABORT is a macro which forces routines not to do any work */
/* when we find a syntax error in the input routine. */
#define ERRORABORT() \
{ \
    if( errorFlagS ){ \
	return ; /* don't do any work for errors */ \
    } \
} \

setErrorFlag()
{
    errorFlagS = TRUE ;
}
/* ***************** ERROR HANDLING ****************************** */

init( numobj )
INT numobj ;
{
    INT i ;
    OBJECTPTR o ;

    /* save number of objects in global for future use */
    numobjectsG = numobj ;
    numobj++ ; /* add 1 to cover 0 node */

    /* initialize bounding box of objects to find area for draw */
    boxLS = INT_MAX ;
    boxBS = INT_MAX ;
    boxRS = INT_MIN ;
    boxTS = INT_MIN ;

    /* allocate space for graph */
    proGraphG = (OBJECTPTR *) Ysafe_malloc( (numobj + 1) * sizeof(OBJECTPTR) );
    for( i = 0 ; i <= numobj ; i++ ){
	o = proGraphG[i] = (OBJECTPTR) Ysafe_malloc( sizeof(OBJECTBOX) );
	o->node = i ;
	o->adjF  = NULL ;
	o->adjB  = NULL ;
	o->numedges  = 0 ;
    }
    curObjS = -1 ; /* initialize object counter */
    add_object( START, 0 ) ;

} /* end init */

/* create a new object */
add_object( pname, node )
char *pname ;
INT node ;
{
    ERRORABORT() ;
    if( ++curObjS != node || node > numobjectsG ){
	setErrorFlag() ;
	M(ERRMSG, "add_object", "Problem with cell number\n" ) ;
	return ;
    }
    objS = proGraphG[curObjS] ;
    objS->node  = node ;
    objS->name  = pname ;
} /* end add_object */


add_pdependency( fromNode ) 
INT fromNode ;
{
    OBJECTPTR from, to ;
    ADJPTR    temp, newE ; /* edge temporary pointers */

    ERRORABORT() ;
    if( fromNode < 0 || fromNode > numobjectsG ){
	M( ERRMSG, "add_pdependency", "Node out of bounds\n" ) ;
	setErrorFlag() ;
	return ;
    }

    from = proGraphG[fromNode] ;
    to   = objS ;

    /* create backward edge since we are given edges in dependency form */
    if( temp = to->adjB ){
	/* add at end of the list */
	for( ; temp->next ; temp = temp->next ) ;
	newE = temp->next = (ADJPTR) Ysafe_malloc(sizeof(ADJBOX)) ;
    } else {
	/* start adjacency list */
	newE = to->adjB = (ADJPTR) Ysafe_malloc(sizeof(ADJBOX)) ;
    }
    newE->next = NULL ;
    newE->node = fromNode ;
    newE->ifiles = NULL ;
    newE->ofiles = NULL ;
    newE->marked = FALSE ;

    /* now create forward edge */
    if( temp = from->adjF ){
	/* add at end of the list */
	for( ; temp->next ; temp = temp->next ) ;
	newE = temp->next = (ADJPTR) Ysafe_malloc(sizeof(ADJBOX)) ;
    } else {
	/* start adjacency list */
	newE = from->adjF = (ADJPTR) Ysafe_malloc(sizeof(ADJBOX)) ;
    }
    newE->next = NULL ;
    newE->node = objS->node ;
    newE->ifiles = NULL ;
    newE->ofiles = NULL ;
    newE->marked = FALSE ;

} /* end add_pdependency */

add_path( pathname )
char *pathname ;
{
    ERRORABORT() ;
    objS->path = pathname ;
} /* end add_path */

/* set file type */
set_file_type( type )
BOOL type ;
{
    inputNotOutputS = type ;
} /* end set_file_type */

add_fdependency( file ) 
char *file ;
{
    INT len ;
    FPTR temp, newF ;

    ERRORABORT() ;
    if( inputNotOutputS ){
	/* now create list of files program depends on */
	if( temp = edgeListS->ifiles ){
	    newF = edgeListS->ifiles = (FPTR) Ysafe_malloc(sizeof(FBOX)) ;
	    /* hook to rest of list */
	    newF->next = temp ;
	} else {
	    /* start adjacency list */
	    newF = edgeListS->ifiles = (FPTR) Ysafe_malloc(sizeof(FBOX)) ;
	    newF->next = NULL ;
	}
    } else {
	/* now create list of files program depends on */
	if( temp = edgeListS->ofiles ){
	    newF = edgeListS->ofiles = (FPTR) Ysafe_malloc(sizeof(FBOX)) ;
	    /* hook to rest of list */
	    newF->next = temp ;
	} else {
	    /* start adjacency list */
	    newF = edgeListS->ofiles = (FPTR) Ysafe_malloc(sizeof(FBOX)) ;
	    newF->next = NULL ;
	}
    }
    newF->optional = FALSE ;
    len = strlen( file ) - 1 ;
    if( len >= 0 ){
	if( file[len] == OPTIONAL ){
	    /* remove the OPTIONAL character */
	    newF->optional = TRUE ;
	    file[len] = EOS ;
	}
    }
    newF->fname = file ;
} /* end add_fdependency */

add_args( argument )
char *argument ;
{
    ERRORABORT() ;
    ASSERT( edgeListS->argc < MAXARGS, "add_args",
	"Max num of args exceeded\n");
    edgeListS->argv[edgeListS->argc++] = argument ;
} /* end add_args */

add_box( l, b, r, t )
INT l, b, r, t ;
{
    ERRORABORT() ;

    /* save bounding box of all tiles */
    boxLS = MIN( l, boxLS ) ;
    boxBS = MIN( b, boxBS ) ;
    boxRS = MAX( r, boxRS ) ;
    boxTS = MAX( t, boxTS ) ;
    objS->l = l ;
    objS->r = r ;
    objS->b = b ;
    objS->t = t ;
} /* end add_box */

/* - - - - - - - - - - - -- - - - - - - - - - - - - -- - - - - - - -
    Need to store drawn lines temporarily because graph has not been
    completed at this point.  We could force user to topologically sort
    data but we want to make it easy for the user.  It's not much trouble
    anyway.
- - - - - - -- - - - - - - - - - - - - - - - - - - - - - - - - - - */
start_edge( fromNode )
INT fromNode ;
{
    EDGEPTR temp ;

    ERRORABORT() ;

    if( fromNode < 0 || fromNode > numobjectsG ){
	M( ERRMSG, "start_edge", "Node out of bounds\n" ) ;
	setErrorFlag() ;
	return ;
    }

    if( temp = edgeListS ){
	edgeListS = (EDGEPTR) Ysafe_malloc(sizeof(EDGEBOX)) ;
	/* hook to rest of list */
	edgeListS->next = temp ;
    } else {
	/* start adjacency list */
	edgeListS = (EDGEPTR) Ysafe_malloc(sizeof(EDGEBOX)) ;
	edgeListS->next = NULL ;
    }
    edgeListS->from = fromNode ;
    edgeListS->to = objS->node ;
    edgeListS->dlist  = NULL ;
    edgeListS->ifiles = NULL ;
    edgeListS->ofiles = NULL ;
    edgeListS->argv = (char **) Ysafe_malloc( MAXARGS * sizeof(char *) ) ;
    edgeListS->argc = 0 ;

    /* update edge count for from node */
    proGraphG[fromNode]->numedges++ ;
    
} /* end start_edge */

add_line( x1, y1, x2, y2 )
INT x1, y1, x2, y2 ;
{

    DPTR temp, dptr ;

    ERRORABORT() ;

    if( temp = edgeListS->dlist ){
	dptr = edgeListS->dlist = (DPTR) Ysafe_malloc(sizeof(DBOX)) ;
	/* hook to rest of list */
	dptr->next = temp ;
    } else {
	/* start a new draw list */
	dptr = edgeListS->dlist = (DPTR) Ysafe_malloc(sizeof(DBOX)) ;
	dptr->next = NULL ;
    }
    dptr->x1 = x1 ;
    dptr->y1 = y1 ;
    dptr->x2 = x2 ;
    dptr->y2 = y2 ;
    boxLS = MIN( x1, boxLS ) ;
    boxBS = MIN( y1, boxBS ) ;
    boxRS = MAX( x2, boxRS ) ;
    boxTS = MAX( y2, boxTS ) ;

} /* end add_line */

ADJPTR findEdge( from, to, direction )
INT from, to ;
BOOL direction ;
{
    ADJPTR adjptr ;

    ASSERT( from >= 0 && from <= numobjectsG,"findEdge",
	"Node out of bounds\n" ) ;

    if( direction == FORWARD ){
	adjptr = proGraphG[from]->adjF;
    } else {
	adjptr = proGraphG[from]->adjB;
    }
    for( ; adjptr; adjptr = adjptr->next ){
	if( adjptr->node == to ){
	    return( adjptr ) ;
	}
    }
    return( NULL ) ;
} /* end findEdge */

/* process lines */
/* Now add the drawn lines to the graph data structure */
process_arcs()
{
    ADJPTR adjptr ;
    EDGEPTR edge ;

    if( errorFlagS ){
	M( ERRMSG, NULL, "Problem reading flow file.  Aborting...\n\n" ) ;
	YexitPgm( PGMFAIL ) ;
    }

    /* cross reference edge to correct place */
    for( edge = edgeListS; edge; edge=edge->next ){
	/* find forward edge */
	adjptr = findEdge( edge->from, edge->to, FORWARD ) ;
	if( !(adjptr) ){
	    sprintf( YmsgG, "Inconsistently defined edge %d -> %d \n",
		edge->from, edge->to ) ;
	    M( ERRMSG, "process_arcs", YmsgG ) ;
	    setErrorFlag() ;
	    continue ;
	}
	adjptr->geometry = edge->dlist ;
	adjptr->ifiles = edge->ifiles ;
	adjptr->ofiles = edge->ofiles ;
	adjptr->argv = edge->argv ;
	adjptr->argc = edge->argc ;
	/* find backward edge */
	adjptr = findEdge( edge->to, edge->from, BACKWARD ) ;
	if( !(adjptr) ){
	    sprintf( YmsgG, "Inconsistently defined edge %d -> %d \n",
		edge->to, edge->from ) ;
	    M( ERRMSG, "process_arcs", YmsgG ) ;
	    setErrorFlag() ;
	    continue ;
	}
	adjptr->geometry = edge->dlist ;
	adjptr->ifiles = edge->ifiles ;
	adjptr->ofiles = edge->ofiles ;
	adjptr->argv = edge->argv ;
	adjptr->argc = edge->argc ;
    } 

    /* exit if we get error in graph */
    if( errorFlagS ){
	YexitPgm( PGMFAIL ) ;
    }

    /* now free edgeList */
    for( ; edgeListS; ){
	edge = edgeListS ;
	edgeListS = edgeListS->next ;
	Ysafe_free( edge ) ;
    }
    G( set_window() ) ;


} /* end process_arcs */

/* clean edges so everything must be checked */
unmark_edges()
{
    INT i ;                   /* counter */
    OBJECTPTR o ;             /* object pointer */
    ADJPTR    adjptr ;        /* pointer to edge */

    for( i = 0 ; i <= numobjectsG ; i++ ){
	o = proGraphG[i] ;
	for( adjptr = o->adjF; adjptr ; adjptr = adjptr->next ){
	    adjptr->marked = FALSE ;
	}
	for( adjptr = o->adjB; adjptr ; adjptr = adjptr->next ){
	    adjptr->marked = FALSE ;
	}
    }
} /* end unmark_edges */

#ifndef NOGRAPHICS

#include <yalecad/draw.h>

set_window()
{
    INT xpand ;   /* make output look nice */
    INT min, max ; /* make into square */

    /* also set the drawing window */
    /* take min max to make window centered */
    min = MIN( boxLS, boxBS ) ;
    max = MAX( boxRS, boxTS ) ;
    xpand = ( max - min ) / 10 ;
    min -= xpand ;
    max += xpand ;
    TWsetwindow( min , min, max, max ) ;
} /* end set_window */
#endif /* NOGRAPHICS */
