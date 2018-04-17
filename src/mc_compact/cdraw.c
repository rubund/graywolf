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
FILE:	    cdraw.c
DESCRIPTION:This file contains graphic routines for compaction program.
CONTENTS:   
DATE:	    Apr 17, 1989 
REVISIONS:  Apr 25, 1989 - added graphics loop to program.
	    May  5, 1989 - added NOGRAPHICS conditional compile.
	    Sun Nov  4 12:50:58 EST 1990 - set drawLabel back to
		correct initialization. Also added draw critical
		tiles.
	    Sun Feb 17 19:37:33 EST 1991 - added 3D graphics.
	    Sat Feb 23 00:47:48 EST 1991 - now draw the changraph.
	    Fri Mar 29 14:13:22 EST 1991 - temp fix for 2D graphics
		avoids arbitrary edge explosion.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) cdraw.c version 7.5 3/29/91" ;
#endif

#ifndef NOGRAPHICS 

#include <yalecad/base.h>
#include <yalecad/message.h>
#include <yalecad/draw.h>
#include <yalecad/colors.h>
#include <yalecad/debug.h>
#include <compact.h>

#define SLEEPTIME     (unsigned) 2
#define DATADIR       "./DATA"
#define COMPACTMENU   "compact_menu"

/* #define DEVELOPMENU */
/* During development use TWread_menus in place of menuS */
/* to create menu record, ie.  TWread_menus(COMPACTMENU) */
#ifdef DEVELOPMENU
#define MENU   TWread_menus(COMPACTMENU)
#else
#define MENU   menuS
#endif

#include <menus.h>

static BOOL drawLabelS = FALSE ;
static BOOL drawEdgeS  = FALSE ;
static BOOL auto_drawS  = TRUE ;
static BOOL drawForwardS = TRUE ;
static BOOL selectCellS = 0 ;
static BOOL drawMovedS = TRUE ;
static BOOL draw3DS = FALSE ;
static BOOL drawCriticalS = FALSE ;
static BOOL drawEdgeLabelS = FALSE ;
static BOOL drawChanGraphS = TRUE ;
static INT  zspanS ;

init_graphics( argc, argv, windowId )
INT argc, windowId ;
char *argv[] ;
{
    char *host, *Ygetenv() ;
    int  xpandx, xpandy ;
    INT draw_the_data() ;

    /* we need to find host for display */
    if(!(host = Ygetenv("DISPLAY"))) {
	M(WARNMSG,"init_graphics","Can't get environment variable ");
	M(MSG,NULL, "for display.  Aborting graphics...\n\n" ) ;
	graphicsG = FALSE ;
	return ;
    }
    if( windowId ){
	/* init windows as a parasite */
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
	xpandx = blockrG - blocklG ;
	xpandy = blocktG - blockbG ;
	zspanS = MIN( xpandx, xpandy ) ;
	xpandx /= 2 ;
	xpandx /= 2 ;
	TWsetwindow( blocklG-xpandx, blockbG-xpandy, 
	    blockrG+xpandx, blocktG+xpandy ) ;
	TWdrawMenus() ;
	TWflushFrame() ;
    }
}

set_draw_critical( flag ) 
BOOL flag ;
{

    if( YdebugAssert() ){
	/* in debug mode, I always want to look at moved cells */
	drawCriticalS = FALSE ;
    } else {
	drawCriticalS = flag ;
    }
} /* end set_draw_critical */

/* draw_the_data routine draws compaction graph */
INT draw_the_data()
{

    INT  i ;
    INT  toX, toY, fromX, fromY, sinkTile ;
    INT  color ;
    static BOOL firstL = TRUE ;
    unsigned sleep() ;
    char label[LRECL] ;
    char *labelptr ;
    COMPACTPTR tptr, t ;
    ECOMPBOXPTR eptr ;

    /* graphics is turned off */
    if( !graphicsG ){
	return ;
    }
    if( firstL && numcellsG ){
	zspanS = ROUND( (DOUBLE) zspanS / (DOUBLE) numcellsG / 10.0 ) ;
	firstL = FALSE ;
    }

    TWstartFrame() ;

    if( drawChanGraphS ){
	draw_changraph() ;
    }

    /* first draw the tiles */
    for( i=1;i<=numtilesG;i++){
	t = tileNodeG[i] ;
	if( t->type || t->cell == selectCellS ){
	    /* stdcells or selected cells */
	    color = TWRED ;
	} else if( drawCriticalS && (t->criticalX || t->criticalY) ){
	    /* criticalS is turned on by entry into compaction cycle.*/ 
	    color = TWVIOLET ;
	} else if( !drawCriticalS && drawMovedS && t->moved ){
	    /* in debug mode we look at moved tiles */
	    color = TWVIOLET ;
	} else {
	    color = TWGREEN ;
	}
	if( drawLabelS ){
	    sprintf( label, "C%d:T%d", t->cell, i ) ;
	    labelptr = label ;
	} else {
	    labelptr = NIL(char *) ;
	}
	if( draw3DS ){
	    TW3Darb_init() ;
	    TW3Darb_addpt( t->l, t->b, t->cell * zspanS ) ;
	    TW3Darb_addpt( t->l, t->t, t->cell * zspanS ) ;
	    TW3Darb_addpt( t->r, t->t, t->cell * zspanS ) ;
	    TW3Darb_addpt( t->r, t->b, t->cell * zspanS ) ;
	    TW3DdrawArb( i, color, labelptr ) ;
	} else {
	    TWarb_init() ;
	    TWarb_addpt( t->l, t->b ) ;
	    TWarb_addpt( t->l, t->t ) ;
	    TWarb_addpt( t->r, t->t ) ;
	    TWarb_addpt( t->r, t->b ) ;
	    TWdrawArb( i, color, labelptr ) ;
	}

    } /* end drawing the tiles */

    /* nets are the edges between the cells */
    /* -------------------------------------------------------------- 
        space requirements also include 2 sources and 2 sinks: 
        xSource = 0, xsink = numtilesG+1, ysource = numtilesG + 2,
	and ysink = numtilesG + 3 positions in the tileNode array.
    */
    if( drawEdgeS && drawForwardS ){
	/****************** XGRAPH *******************************/
	sinkTile = numtilesG+1;
	for( eptr = xGraphG[0]->xadjF ;eptr; eptr=eptr->next){
	    fromX = xGraphG[0]->l ;
	    fromY = (blockbG + blocktG) / 2 ;
	    tptr = tileNodeG[eptr->node] ;
	    toX = tptr->l ;
	    toY = (tptr->b + tptr->t) / 2 ;
	    if( drawEdgeLabelS ){
		sprintf( label, "%d", eptr->constraint );
		labelptr = label ;
	    } else {
		labelptr = NIL(char *) ;
	    }
	    TWdrawNet( 0, fromX,fromY,toX,toY, TWRED, labelptr ) ;
	}
	for( i=1;i<=numtilesG;i++){
	    for( eptr = xGraphG[i]->xadjF ;eptr; eptr=eptr->next){
		fromX = xGraphG[i]->l ;
		fromY = (xGraphG[i]->b + xGraphG[i]->t) / 2 ;
		tptr = tileNodeG[eptr->node] ;
		toX = tptr->l ;
		if( eptr->node != sinkTile ){
		    toY = (tptr->b + tptr->t) / 2 ;
		} else {
		    toY = (blockbG + blocktG) / 2 ;
		}
		sprintf( label, "%d", eptr->constraint );
		if( drawEdgeLabelS ){
		    sprintf( label, "%d", eptr->constraint );
		    labelptr = label ;
		} else {
		    labelptr = NIL(char *) ;
		}
		TWdrawNet( i,fromX,fromY,toX,toY, TWRED, labelptr ) ;
	    }
	}

	/****************** YGRAPH *******************************/
	sinkTile = numtilesG + 3 ;
	for( eptr = yGraphG[0]->yadjF ;eptr; eptr=eptr->next){
	    fromY = yGraphG[0]->b ;
	    fromX = (blocklG + blockrG) / 2 ;
	    tptr = tileNodeG[eptr->node] ;
	    toY = tptr->b ;
	    toX = (tptr->l + tptr->r) / 2 ;
	    if( drawEdgeLabelS ){
		sprintf( label, "%d", eptr->constraint );
		labelptr = label ;
	    } else {
		labelptr = NIL(char *) ;
	    }
	    TWdrawNet( 0,fromX,fromY,toX,toY, TWBLUE, labelptr ) ;
	}
	for( i=1;i<=numtilesG;i++){
	    for( eptr = yGraphG[i]->yadjF ;eptr; eptr=eptr->next){
		fromY = yGraphG[i]->b ;
		fromX = (yGraphG[i]->l + yGraphG[i]->r) / 2 ;
		tptr = tileNodeG[eptr->node] ;
		toY = tptr->b ;
		if( eptr->node != sinkTile ){
		    toX = (tptr->l + tptr->r) / 2 ;
		} else {
		    toX = (blocklG + blockrG) / 2 ;
		}
		if( drawEdgeLabelS ){
		    sprintf( label, "%d", eptr->constraint );
		    labelptr = label ;
		} else {
		    labelptr = NIL(char *) ;
		}
		TWdrawNet( i,fromX,fromY,toX,toY, TWBLUE, labelptr ) ;
	    }
	}
    } else if( drawEdgeS && !drawForwardS ){
	/****************** XGRAPH *******************************/
	sinkTile = numtilesG + 1 ;
	for( eptr = xGraphG[sinkTile]->xadjB ;eptr; eptr=eptr->next){
	    fromX = xGraphG[sinkTile]->l ;
	    fromY = (blockbG + blocktG) / 2 ;
	    tptr = tileNodeG[eptr->node] ;
	    toX = tptr->l ;
	    toY = (tptr->b + tptr->t) / 2 ;
	    sprintf( label, "%d", eptr->constraint );
	    if( drawEdgeLabelS ){
		sprintf( label, "%d", eptr->constraint );
		labelptr = label ;
	    } else {
		labelptr = NIL(char *) ;
	    }
	    TWdrawNet( 0, fromX,fromY,toX,toY, TWRED, labelptr ) ;
	}
	for( i=1;i<=numtilesG;i++){
	    for( eptr = xGraphG[i]->xadjB ;eptr; eptr=eptr->next){
		fromX = xGraphG[i]->l ;
		fromY = (xGraphG[i]->b + xGraphG[i]->t) / 2 ;
		tptr = tileNodeG[eptr->node] ;
		toX = tptr->l ;
		if( eptr->node != sinkTile ){
		    toY = (tptr->b + tptr->t) / 2 ;
		} else {
		    toY = (blockbG + blocktG) / 2 ;
		}
		if( drawEdgeLabelS ){
		    sprintf( label, "%d", eptr->constraint );
		    labelptr = label ;
		} else {
		    labelptr = NIL(char *) ;
		}
		TWdrawNet( i,fromX,fromY,toX,toY, TWRED, labelptr ) ;
	    }
	}

	/****************** YGRAPH *******************************/
	sinkTile = numtilesG + 3 ;
	for( eptr = tileNodeG[sinkTile]->yadjB ;eptr; eptr=eptr->next){
	    fromY = tileNodeG[sinkTile]->t ;
	    fromX = (blocklG + blockrG) / 2 ;
	    tptr = tileNodeG[eptr->node] ;
	    toY = tptr->t ;
	    toX = (tptr->l + tptr->r) / 2 ;
	    if( drawEdgeLabelS ){
		sprintf( label, "%d", eptr->constraint );
		labelptr = label ;
	    } else {
		labelptr = NIL(char *) ;
	    }
	    TWdrawNet( 0,fromX,fromY,toX,toY, TWBLUE, labelptr ) ;
	}
	for( i=1;i<=numtilesG;i++){
	    for( eptr = yGraphG[i]->yadjB ;eptr; eptr=eptr->next){
		fromY = yGraphG[i]->t ;
		fromX = (yGraphG[i]->l + yGraphG[i]->r) / 2 ;
		tptr = tileNodeG[eptr->node] ;
		toY = tptr->t ;
		if( eptr->node != sinkTile ){
		    toX = (tptr->l + tptr->r) / 2 ;
		} else {
		    toX = (blocklG + blockrG) / 2 ;
		}
		if( drawEdgeLabelS ){
		    sprintf( label, "%d", eptr->constraint );
		    labelptr = label ;
		} else {
		    labelptr = NIL(char *) ;
		}
		TWdrawNet( i,fromX,fromY,toX,toY, TWBLUE, labelptr ) ;

	    }
	}
    } /* end draw backward case */ 

    if( draw3DS ){
	TW3DdrawAxis() ;
    }

    TWflushFrame() ;
    D( "draw_the_data", sleep( SLEEPTIME ) ) ;

} /* end draw_the_data */

/* heart of the graphic system processes user input */
process_graphics()
{

    int x1, y1, x2, y2 ; /* coordinates for fixing cells and neighhds */
    INT x, y ;           /* coordinates from pointer */
    int i ;            /* temp variable */
    int selection ;     /* the users pick */
    char *reply ;       /* user reply to a querry */
    BOOL ok ;           /* loop until this value is true */
    BOOL movedCell ; /* lets us know whether we have to update cellbins */
    INFOPTR stretch_edge ; /* the edge to be ref. for stretch */
    INFOPTR get_closest_edge() ;

    /* data might have changed so show user current config */
    /* any function other that the draw controls need to worry about */
    /* this concurrency problem -  show user current config */
    draw_the_data() ;
    /* use TWcheckExposure to flush exposure events since we just */
    /* drew the data */
    TWcheckExposure() ;
    TWmessage( "TimberWolfMC is waiting for your response..." ) ;

    selection  = CANCEL ;
    while( selection != CONTINUE_PROG ){ /* loop until exit */
	selection = TWcheckMouse() ;
	switch( selection ){
	case CANCEL:
	    /* do nothing */
	    break ;
	case REDRAW:
	    draw_the_data() ;
	    /* use TWcheckExposure to flush exposure events since */
	    /* we just drew the data */
	    TWcheckExposure() ;
	    break ;
	case ZOOM:
	    TWzoom() ;
	    break ;
	case FULLVIEW:
	    TWfullView() ;
	    break ;
	case CONTINUE_PROG:
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
	case DRAW_LABELS:
	    drawLabelS = TRUE ;
	    TWmessage( "Labels will be drawn" ) ;
	    TWforceRedraw() ;
	    break ;
	case IGNORE_LABELS:
	    drawLabelS = FALSE ;
	    TWmessage( "Labels will be omitted from drawing" ) ;
	    TWforceRedraw() ;
	    break ;
	case DRAW_EDGES:
	    drawEdgeS = TRUE ;
	    TWmessage( "Edges will be drawn" ) ;
	    TWforceRedraw() ;
	    break ;
	case IGNORE_EDGES:
	    drawEdgeS = FALSE ;
	    TWmessage( "Edges will be omitted from drawing" ) ;
	    TWforceRedraw() ;
	    break ;
	case COLORS:
	    TWtoggleColors() ;
	    break ;
	case DRAW_EDGE_LABELS:
	    drawEdgeLabelS = TRUE ;
	    TWmessage( "Edge constraint labels will be drawn" ) ;
	    TWforceRedraw() ;
	    break ;
	case IGNORE_EDGE_LABELS:
	    drawEdgeLabelS = FALSE ;
	    TWmessage( "Edge constraint labels will be ignored" ) ;
	    TWforceRedraw() ;
	    break ;
	case DRAW_FORWARD:
	    drawForwardS = TRUE ;
	    TWmessage( "Forward edges will be drawn" ) ;
	    TWforceRedraw() ;
	    break ;
	case DRAW_BACKWARD:
	    drawForwardS = FALSE ;
	    TWmessage( "Backward edges will be drawn" ) ;
	    TWforceRedraw() ;
	    break ;
	case TRANSLATE:
	    TWtranslate() ;
	    break ;
	case DUMP_GRAPHICS:
	    /* now change mode to dump to file */
	    TWsetMode(1) ;
	    /* dump the data to a file now instead of screen */
	    draw_the_data() ;
	    /* restore the state to draw to screen */
	    TWsetMode(0) ;
	    break ;
	case TELL_POINT:
	    TWmessage( "Pick a point" ) ;
	    TWgetPt( &x, &y ) ;
	    sprintf( YmsgG,"The point is (%d,%d)",x,y ) ;
	    TWmessage( YmsgG ) ;
	    break ;
	case DUMP_STATE:
	    output() ;
	    break ;
	case HIGHLITE_CELL:
	    reply = TWgetString( "Enter cell to highlite:" ) ;
	    selectCellS = atof( reply ) ;
	    if( selectCellS > 0 && selectCellS <= numcellsG ){
		sprintf( YmsgG, "Selected cell is :%d", selectCellS ) ;
		TWmessage( YmsgG ) ;
	    } else {
		TWmessage( "No valid cell selected" ) ;
		selectCellS = 0 ;
	    }
	    TWforceRedraw() ;
	    break ;
	case DRAW_MOVED_TILES:
	    drawMovedS = TRUE ;
	    TWmessage( "Moved tiles will be highlighted" ) ;
	    TWforceRedraw() ;
	    break ;
	case IGNORE_MOVED_TILES:
	    TWmessage( "Moved tiles will NOT be highlighted" ) ;
	    drawMovedS = FALSE ;
	    TWforceRedraw() ;
	    break ;
	case SET__3D_CAMERA:
	    TW3DsetCamera() ;
	    draw3DS = TRUE ;
	    break ;
	case NORMAL_VIEW:
	    TW3Dnormal_view() ;
	    draw3DS = FALSE ;
	    break ;
	case FIND_EDGE:
	    TWmessage( "Pick an edge" ) ;
	    TWgetPt( &x, &y ) ;
	    stretch_edge = get_closest_edge( x, y ) ;
	    draw_the_data() ;
	    TWmessage( "Pick move point" ) ;
	    TWgetPt( &x, &y ) ;
	    stretch_graph( stretch_edge, x, y ) ;
	    stretch_edge->marked = FALSE ;
	    TWforceRedraw() ;
	    break ;
	}
	if( auto_drawS && TWcheckExposure() ){
	    draw_the_data() ;
	}

    } 

    TWmessage("Continuing - to interupt program click on top menu window") ;

} /* end process_graphics */


/* how to draw the channel graph */
draw_changraph()
{
    INT i ;                        /* temp counter */
    INT color ;                    /* color of edge */
    CHANBOXPTR nptr ;              /* pointer to channel graph */
    CHANBOXPTR node1 ;             /* pointer to channel graph */
    CHANBOXPTR node2 ;             /* pointer to channel graph */
    INFOPTR eptr ;                 /* current edge */
    char label_buf[LRECL] ;        /* buffer for labels */
    char *label ;                  /* label pointer */
    INT x1, x2, y1, y2 ;           /* points of trace line */

    /* first draw the nodes */
    for( i = 1; i <= numnodesG; i++ ){
	nptr = changraphG[i] ;
	if( drawLabelS ){
	    label = label_buf ;
	    sprintf( label, "N%d", i ) ;
	} else {
	    label = NULL ;
	}
	TWdrawCell( i, nptr->xc-1, nptr->yc-1, 
	    nptr->xc+1, nptr->yc+1, TWRED, label ) ;
    } /* end draw of nodes */

    /* now draw the edges */
    for( i = 1; i <= numedgesG ; i++ ){
	eptr = edgeArrayG[i] ;
	node1 = changraphG[eptr->node1] ;
	node2 = changraphG[eptr->node2] ;
	if( eptr->marked ){
	    color = TWYELLOW ;
	} else {
	    color = TWMEDAQUA ;
	}
	TWdrawLine( i, node1->xc, node1->yc, 
	    node2->xc, node2->yc, color, NULL ) ;

    } /* end draw of edges */
    
} /* end draw_changraph */

#endif /* NOGRAPHICS */

