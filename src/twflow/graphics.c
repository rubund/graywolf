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
FILE:	    graphics.c 
DESCRIPTION:This file contains graphic routines for twflow program.
CONTENTS:   init_graphics()
	    void draw_the_data()
	    process_graphics()
	    ADJPTR make_decision( obj, direction )
		OBJECTPTR obj ;
		BOOL direction ;
	    ADJPTR get_edge_from_user( obj, direction )
		OBJECTPTR obj ;
		BOOL direction ;
	    graphics_set_object( object )
		INT find_obj()
DATE:	    May  7, 1989 - original coding.
REVISIONS:  Jun 19, 1989 - added return for no graphics case.
	    Jun 19, 1989 - added prompt toggle.  Added get_edge_from_user
		and modified make_decision so that multiple edge
		case is correct
	    Aug  1, 1990 - modified more than one selection mechanism
		to make it easier for the user.
	    Sat Feb 23 00:15:36 EST 1991 - added dump.
	    Sat Feb 23 04:51:29 EST 1991 - added parasite mode.
	    Sun Apr 21 22:35:09 EDT 1991 - updated to the convention.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) graphics.c version 2.6 4/21/91" ;
#endif

#include <string.h>
#include <yalecad/base.h>
#include <yalecad/message.h>
#include <yalecad/draw.h>
#include <yalecad/string.h>
#include <yalecad/colors.h>
#include <yalecad/debug.h>
#include <globals.h>

#define SLEEPTIME     (unsigned) 2
#define DATADIR       "./DATA"
#define MENUP         "twflow_menu"
#define WAITMSG       "graywolf is waiting for your response..."

static BOOL auto_drawS  = TRUE ;
static BOOL promptS     = FALSE ;        /* whether to prompt for path */
static INT selectedObjS = 0 ;            /* current selected object */
static ADJPTR selectedEdgeS = NULL ;     /* current selected edge */

#ifndef NOGRAPHICS 


/* #define DEVELOPMENU */
/* During development use TWread_menus in place of menuS */
/* to create menu record, ie.  TWread_menus(MENUP) */
#ifdef DEVELOPMENU
#define MENU   TWread_menus(MENUP)
#else
#define MENU   menuS
#endif

#include <menus.h>

init_graphics(argc,argv,windowId)
INT argc ;
char *argv[] ;
INT windowId ;
{
    INT draw_the_data() ;

    if( !(graphicsG) ){
	return ;
    } 
    if( windowId ){
	/* init windows as a parasite */
	fprintf( stderr,"windowId:%d\n", windowId ) ;
	if( !( TWinitParasite(argc,argv,TWnumcolors(),TWstdcolors(),
	    FALSE, MENU, draw_the_data, windowId ))){
	    M(ERRMSG,"initgraphics","Aborting graphics.");
	    graphicsG = FALSE ;
	    return ;
	}
    } else {
	if(!(TWinitGraphics(argc,argv,TWnumcolors(),TWstdcolors(),
	    FALSE, MENU, draw_the_data ))){
	    M(ERRMSG,"init_graphics","Aborting graphics.");
	    graphicsG = FALSE ;
	    return ;
	}
    }
    TWdrawMenus() ;
    TWflushFrame() ;
} /* end init_graphics */

/* draw_the_data routine draws compaction graph */
INT draw_the_data()
{

    INT  i ;
    INT  color ;
    OBJECTPTR o ;
    DPTR   dptr ;
    ADJPTR eptr ;

    /* graphics is turned off */
    if( !graphicsG ){
	return ;
    }

    TWstartFrame() ;
    /* first draw the objects */
    for( i=1; i<= numobjectsG;i++){
	o = proGraphG[i] ;
	if( i == selectedObjS ){
	    color = TWRED ;
	} else {
	    color = TWGREEN ;
	}
	TWdrawCell( i, o->l,o->b,o->r,o->t, color, o->name ) ;
    } /* end drawing the objects */

    /* next draw the edges between objects */
    /* go thru all the objects */
    for( i=1;i<= numobjectsG;i++){
	/* go thru all the edges of an object */
	for( eptr = proGraphG[i]->adjF ;eptr; eptr=eptr->next){
	    if( eptr == selectedEdgeS ){
		color = TWRED ;
	    } else {
		color = TWBLUE ;
	    }
	    for( dptr = eptr->geometry; dptr ; dptr = dptr->next ){
		TWdrawNet( i,dptr->x1,dptr->y1,
		    dptr->x2,dptr->y2, color, NULL);
	    }
	}
    }
    TWflushFrame() ;
    if( YdebugAssert() ){
	sleep( SLEEPTIME ) ;
    }

} /* end draw_the_data */


/* heart of the graphic system processes user input */
process_graphics()
{

    INT selection ;     /* the users pick */
    INT find_obj() ;    /* find the users pick */
    INT x, y ;          /* the picked point */

    /* data might have changed so show user current config */
    /* any function other that the draw controls need to worry about */
    /* this concurrency problem -  show user current config */
    draw_the_data() ;
    /* use TWcheckExposure to flush exposure events since we just */
    /* drew the data */
    TWcheckExposure() ;
    TWmessage( WAITMSG ) ;
    M( MSG, NULL, WAITMSG ) ;
    M( MSG, NULL, "\n" ) ;

    selection  = CANCEL ;
    while( selection != CONTINUE_PGM ){ /* loop until exit */
	selection = TWcheckMouse() ;
	switch( selection ){
	case CANCEL:
	    /* do nothing */
	    break ;
	case AUTO_REDRAW_ON:
	    auto_drawS = TRUE ;
	    break ;
	case AUTO_REDRAW_OFF:
	    auto_drawS = FALSE ;
	    break ;
	case CLOSE_GRAPHICS:
	    TWcloseGraphics() ;
	    /* update all costs and reload cells */
	    graphicsG = FALSE ;
	    return ;
	case COLORS:
	    TWtoggleColors() ;
	    break ;
	case CONTINUE_PGM:
	    break ;
	case DUMP_GRAPHICS:
	    TWsetFrame(0) ; /* update the frame count */
	    /* now change mode to dump to file */
	    TWsetMode(1) ;
	    /* dump the data to a file now instead of screen */
	    draw_the_data() ;
	    /* restore the state to previous condition */
	    /* and set draw to screen */
	    TWsetMode(0) ;
	    break ;
	case EXIT_PROGRAM:
	    TWcloseGraphics() ;
	    YexitPgm( PGMOK ) ;
	    return ;
	case FULLVIEW:
	    TWfullView() ;
	    break ;
	case REDRAW:
	    draw_the_data() ;
	    /* use TWcheckExposure to flush exposure events since */
	    /* we just drew the data */
	    TWcheckExposure() ;
	    break ;
	case TELL_POINT:
	    TWmessage( "Pick a point" ) ;
	    TWgetPt( &x, &y ) ;
	    sprintf( YmsgG,"The point is (%d,%d)",x,y ) ;
	    TWmessage( YmsgG ) ;
	    break ;
	case TRANSLATE:
	    TWtranslate() ;
	    break ;
	case ZOOM:
	    TWzoom() ;
	    break ;
	case AUTOFLOW:
	    autoflowG = TRUE ; /* used to interupt auto flow */
	    auto_flow() ;
	    break ;
	case PICK_PGM:
	    selectedObjS = find_obj() ;
	    autoflow_set_object( selectedObjS ) ;
	    break ;
	case EXECUTE_PGM:
	    exec_single_prog() ;
	    break ;
	case PROMPT_ON:
	    promptS = TRUE ;
	    TWmessage( "Prompt on" ) ;
	    break ;
	case PROMPT_OFF:
	    promptS = FALSE ;
	    TWmessage( "Prompt off" ) ;
	    break ;
	} /* end switch */

	if( auto_drawS && TWcheckExposure() ){
	    draw_the_data() ;
	}
    } 
    TWmessage("Continuing - to interupt program click on top menu window") ;

} /* end process_graphics */

/* get edge loops until it gets answer from user */
ADJPTR get_edge_from_user( obj, direction )
OBJECTPTR obj ;
BOOL direction ;
{
    ADJPTR adjptr ;
    ADJPTR start_edge ;
    ADJPTR findEdge() ;
    INT edge_count ;
    char *answer ;

    while( TRUE ){ /* loop until user makes a selection */
	if( direction == FORWARD ){
	    start_edge = obj->adjF ;
	} else {
	    start_edge = obj->adjB ;
	}
	/* count the number of edges that meet the criteria */
	edge_count = 0 ;
	for( adjptr = start_edge ; adjptr ; adjptr = adjptr->next ){
	    edge_count++ ;
	}
	for( adjptr = start_edge ; adjptr ; adjptr = adjptr->next ){
	    if( direction == FORWARD ){
	        selectedEdgeS = adjptr ;
	    } else {
		/* need to reverse edge so selectedEdge will match */
		selectedEdgeS = findEdge( adjptr->node, obj->node, 
		    FORWARD ) ;
	    }
	    /* show user the edge */
	    draw_the_data() ;
	    TWcheckExposure() ;

	    if( edge_count > 1 ){
		/* give directions */
		sprintf( YmsgG,"%s","If edge is not correct, enter n<cr> ") ;
		strcat( YmsgG, 
		    "for next edge. If satisfied, enter non-null string:") ;
		/* look for empty string - means we are satisfied */
		answer = TWgetString(YmsgG) ;
		if( answer ){
		    if( *answer != 'n' ){
			return( adjptr ) ;
		    }
		} else {
		    return( adjptr ) ;
		}
	    } else {
		/* simple case the correct edge */
		return( adjptr ) ;
	    }
	} /* end for loop */
    } /* end while loop */

} /* end get_edge_from_user */


/* the corresponding handshake to set the highlighted drawing object */
graphics_set_object( object )
INT object ;
{
    selectedObjS = object ;     /* set the current selected object */
} /* graphics_set_object */

/* find the object in question */
INT find_obj()
{

    INT i ;
    INT x, y ;               /* coordinates picked by user */
    INT obj ;               /* selected cell */
    OBJECTPTR o ;           /* pointer to object */

    obj = 0 ;
    TWmessage("Pick program by clicking any mouse button at center of object");

#ifdef NEEDED
    (void) sleep(SLEEPTIME) ;
#endif

    TWgetPt( &x, &y ) ;
    /* look thru all programs O(programs) algorithm */
    for( i=1; i<= numobjectsG;i++){
	o = proGraphG[i] ;
	/* see if cell boundary contains this point */
	if( x >= o->l && x <= o->r ){
	    if( y >= o->b && y <= o->t ){
		selectedObjS = i ;
		draw_the_data() ;
		TWcheckExposure() ;

		obj = selectedObjS ;
		break ;
	    }
	}
    } /* end loop */

    if( obj == 0 ){
	TWmessage( "No cell selected" ) ;
	if( selectedObjS ){  /* user didn't like any options */
	    selectedObjS = 0 ;
	    draw_the_data() ; /* draw the data with highlight off */
	    TWcheckExposure() ;
	}
    } else {
	sprintf( YmsgG, "Selected program:%s", proGraphG[obj]->name ) ;
	TWmessage( YmsgG ) ;
    }
    return( obj ) ;
} /* end find_obj */

#else /* NOGRAPHICS case */

/* get edge loops until it gets answer from user */
ADJPTR get_edge_from_user( obj, direction )
OBJECTPTR obj ;
BOOL direction ;
{
    ADJPTR adjptr ;
    ADJPTR findEdge() ;
    INT node1, node2 ;
    char reply[LRECL] ;

    while( TRUE ){ /* loop until user makes a selection */
	if( direction == FORWARD ){
	    adjptr = obj->adjF ;
	} else {
	    adjptr = obj->adjB ;
	}
	for( ; adjptr ; adjptr = adjptr->next ){
	    if( direction == FORWARD ){
		node1 = obj->node ;
		node2 = adjptr->node ;
	    } else {
		node1 = adjptr->node ;
		node2 = obj->node ;
	    }
	    /* tell the user the edge */

	    /* give directions */
	    fprintf( stdout,"%s-->%s", proGraphG[node1]->name, 
		proGraphG[node2]->name ) ;
	    fprintf( stdout,"If execution path is correct, enter y[es]<cr>.\n") ;
	    fprintf( stdout,"Otherwise enter n<cr> for next edge.\n") ;
	    fscanf( stdout, "%s", reply ) ;
	    if( reply ){
		if( reply[0] == 'y' || reply[0] == 'Y' ){
		    return( adjptr ) ;
		}
	    }
	} /* end for loop */
    } /* end while loop */
} /* end get_edge_from_user */


#endif /* NOGRAPHICS */

/* make_decision asks the user for the next program to run */
ADJPTR make_decision( obj, direction )
OBJECTPTR obj ;
BOOL direction ;
{
    ADJPTR adjptr ;
    ADJPTR get_edge_from_user() ;

    if( promptS ){
	adjptr = get_edge_from_user( obj, direction ) ;
	adjptr->marked = TRUE ;
	return( adjptr ) ;
    } else { 
	/* ********************************************************
	*  Avoid asking user - use ordering information given by 
	*  the user.  Take first unexecuted edge as next edge to be
	*  executed.
	*********************************************************** */
	if( direction == FORWARD ){
	    adjptr = obj->adjF ;
	} else {
	    adjptr = obj->adjB ;
	}
	for( ; adjptr ; adjptr = adjptr->next ){
	    if( !(adjptr->marked) ){
		adjptr->marked = TRUE ;
		return( adjptr ) ;
	    }
	}
	/* if we didn't find any unmarked nodes we need to ask user */
	adjptr =  get_edge_from_user( obj, direction ) ;
	return( adjptr ) ;
	
    }

} /* end make decision */
