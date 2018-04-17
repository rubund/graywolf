/*
 *   Copyright (C) 1990-1992 Yale University
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
DESCRIPTION:graphics routines
CONTENTS:   
DATE:	    Jun 14, 1990 
REVISIONS:  Jun 21, 1990 - added graphics abort macro.
	    Oct 20, 1990 - fixed graphics update mode.
	    Nov 23, 1990 - now draw rectilinear cell properly.
	    Fri Feb 15 15:14:34 EST 1991 - added check to prevent
		drawing data before proper initialization.
	    Tue Mar 19 16:25:17 CST 1991 - now output cell name
		during drawLabel.
	    Thu Apr 18 01:47:00 EDT 1991 - now set graphics update
		to its correct state and fixed draw_single_cell_move.
	    Tue May  7 00:09:43 EDT 1991 - added TWsetFrame 
		initialization and draw orientation markers.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) graphics.c version 4.21 5/15/92" ;
#endif

#include "standard.h"
#include "main.h"
#include "readpar.h"
#include "pads.h"

#ifndef NOGRAPHICS

#include <yalecad/debug.h>
#include <yalecad/message.h>
#include <yalecad/colors.h>
#include <yalecad/draw.h>
#include <yalecad/relpos.h>


#define INTRO            "Welcome to TimberWolfSC"
#define GRAPHICSABORT    if(!(doGraphicsG))  return ;


#define NETCOLOR           TWRED
#define PINCOLOR           TWBLACK
#define BLACK              TWBLACK
#define FCOLOR             TWYELLOW

#define MODE         2
#define MENUP        "sc_menu"

/* #define DEVELOPMENU */
/* During development use TWread_menus in place of menuS */
/* to create menu record, ie.  TWread_menus(MENUP) */
#ifdef DEVELOPMENU
#define MENUREAD()   menuS = TWread_menus(MENUP) ;
#else
#define MENUREAD()
#endif

#include <menus.h>


extern INT actual_feed_thru_cells_addedG ;
/* ***************************************************************** */
static BOOL avoidDump = FALSE ;
static BOOL drawPinS = FALSE ;   /* whether or not to draw pins */
static BOOL drawLabelS = FALSE ; /* whether to draw labels or not */
static BOOL drawBlockS = TRUE ;  /* whether to draw blocks or not */
static BOOL drawCellS = TRUE ;   /* whether to draw stdcells or not */
static BOOL auto_drawS = TRUE ;  /* whether to draw immediately after exp.*/
static BOOL updateS = TRUE ;     /* normally in the update mode */
static BOOL single_cell_moveS = FALSE ; /* whether we have single cell moves */
static BOOL drawOrientS = FALSE ; /* whether to draw orientation mark */
static BOOL initS = FALSE ;    /* true if all initialization complete */
static INT  *heat_indexS ;     /* how much the cells are moving */
static INT  *heat_attemptS ;     /* how many attempts during the run */
static INT  heat_allocS ;     /* size of temperature index array */
static INT  drawNetS = 0 ; /* draw nets 0:none 1...n:net >numnets:all */
static INT  pinsizeS ;     /* size of the pin */


static draw_fs();
extern VOID draw_a_cell( INT );
extern INT draw_the_data() ;

initGraphics( argc, argv, windowId )
INT argc ;
char *argv[] ;
INT windowId ;
{

    char *host ;
    char *Ygetenv() ;
    extern INT horizontal_track_pitchG ;
    extern INT vertical_track_pitchG ;

    GRAPHICSABORT ;
	
    /* we need to find host for display */
    if(!(host = Ygetenv("DISPLAY"))) {
	M(WARNMSG,"initGraphics","Can't get environment variable ");
	M(MSG,NULL, "for display.  Aborting graphics...\n\n" ) ;
	avoidDump = TRUE ;
	doGraphicsG = FALSE ;
	return ;
    }

    MENUREAD() ;
    /* initialize the boolean menu entries according to current state */
    /* "Graphics Update On","Graphics Update Off",1,0,8,9, is the 8 line */
    menuS[7].bool_init = updateS ;

    if( windowId ){
	/* init windows as a parasite */
	if(!(TWinitParasite(argc,argv,TWnumcolors(),TWstdcolors(),
	    FALSE,menuS, draw_the_data, windowId ))){
	    M(ERRMSG,"initGraphics","Aborting graphics.");
	    doGraphicsG = FALSE ;
	    avoidDump = TRUE ;
	    return ;
	}
    } else {
	/* init window as a master */
	if(!(TWinitGraphics(argc,argv,TWnumcolors(),TWstdcolors(),FALSE,
	    menuS,draw_the_data ))){
	    M(ERRMSG,"initGraphics","Aborting graphics.");
	    doGraphicsG = FALSE ;
	    avoidDump = TRUE ;
	    return ;
	}
    }
    TWsetwindow( 0, 0, 10, 10 ) ;
    TWsetFrame(0) ;
    TWdrawCell( 0, 0,0,11,11, TWBLUE, INTRO ) ;
    setGraphicWindow() ;
    TWdrawMenus() ;
    TWmessage("To interupt program click on top menu window") ;
    TWflushFrame() ;
    /* use TWinterupt to turn off window enter/leave lights */
    TWinterupt() ;
    /* set the default pin size */
    pinsizeS = (vertical_track_pitchG + horizontal_track_pitchG) / 4 - 1 ;
    pinsizeS = MAX( 1, pinsizeS ) ;

} /* end initGraphics */

init_heat_index()
{
    GRAPHICSABORT ;

    heat_allocS = lastpadG + 1;
    heat_indexS = (INT *) Ysafe_malloc( heat_allocS * sizeof(INT) ) ;
    heat_attemptS = (INT *) Ysafe_malloc( heat_allocS * sizeof(INT) ) ;
    reset_heat_index() ;
    initS = TRUE ;
} /* end init_heat_index */

expand_heat_index()
{
    INT oldnum, i;

    GRAPHICSABORT ;

    oldnum = heat_allocS;
    heat_allocS = lastpadG + actual_feed_thru_cells_addedG + 1;
    if (oldnum < heat_allocS) {
       heat_indexS = (INT *) Ysafe_realloc( heat_indexS, heat_allocS * sizeof(INT) ) ;
       heat_attemptS = (INT *) Ysafe_realloc( heat_attemptS, heat_allocS * sizeof(INT) ) ;
       for( i = oldnum; i < heat_allocS ; i++ ){
	   heat_indexS[i] = 0 ;
	   heat_attemptS[i] = 0 ;
       }
    }
} /* end expand_heat_index */

setGraphicWindow() 
{
    INT  expand ;
    INT  minx ;
    INT  maxx ;
    INT  miny ;
    INT  maxy ;
    INT  xc, yc ;      /* cell center */
    INT  i ;           /* counter */
    CBOXPTR cptr ;  /* cell record pointer */

    minx = INT_MAX ;
    miny = INT_MAX ;
    maxx = INT_MIN ;
    maxy = INT_MIN ;

    /* look thru cells to find overall bounding box */
    for( i = 1; i <= lastpadG ; i++ ){
	cptr = carrayG[i] ;
	xc = cptr->cxcenter ;
	yc = cptr->cycenter ;
	if( yc == GATE_ARRAY_MAGIC_CONSTANT ){
	    /* don't draw these */
	    continue ;
	}
	minx = MIN( minx, cptr->tileptr->left + xc ) ;
	miny = MIN( miny, cptr->tileptr->bottom + yc ) ;
	maxx = MAX( maxx, cptr->tileptr->right + xc ) ;
	maxy = MAX( maxy, cptr->tileptr->top + yc ) ;
    }

    expand = MAX( maxy - miny, maxx - minx ) ;
    expand = (INT) (0.1 * (DOUBLE) expand ) ;
    TWsetwindow( minx - expand, miny - expand, 
	maxx + expand, maxy + expand ) ;
} /* end setGraphicWindow */

/* heart of the graphic syskem processes user input */
process_graphics()
{

    INT x, y ;           /* coordinates from pointer */
    INT selection ;     /* the users pick */
    char *reply ;       /* user reply to a querry */
    BOOL ok ;           /* loop until this value is true */

    /* data might have changed so show user current config */
    /* any function other that the draw controls need to worry about */
    /* this concurrency problem -  show user current config */
    GRAPHICSABORT ;
    setGraphicWindow() ;
    draw_the_data() ;
    /* use TWcheckExposure to flush exposure events since we just */
    /* drew the data */
    TWcheckExposure() ;
    TWmessage( "TimberWolfSC is waiting for your response..." ) ;

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
	case COLORS:
	    TWtoggleColors() ;
	    break ;
	case CONTINUE_PGM:
	    break ;
	case DUMP_GRAPHICS:
	    graphics_dump() ;
	    break ;
	case FULLVIEW:
	    TWfullView() ;
	    break ;
	case GRAPHICS_UPDATE_ON:
	    updateS = TRUE ;
	    break ;
	case GRAPHICS_UPDATE_OFF:
	    updateS = FALSE ;
	    selection = CONTINUE_PGM ; /* exit loop */
	    break ;
	case CLOSE_GRAPHICS:
	    closegraphics() ;
	    return ;
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
	case DRAW_BLOCKS:
	    drawBlockS = TRUE ;
	    TWmessage( "Standard cell rows will be drawn" ) ;
	    TWforceRedraw() ;
	    break ;
	case IGNORE_BLOCKS:
	    drawBlockS = FALSE ;
	    TWmessage( "Standard cell rows will be omitted from drawing");
	    TWforceRedraw() ;
	    break ;
	case DRAW_STDCELLS:
	    drawCellS = TRUE ;
	    TWmessage( "Standard cells will be drawn" ) ;
	    TWforceRedraw() ;
	    break ;
	case IGNORE_STDCELLS:
	    drawCellS = FALSE ;
	    TWmessage( "Standard cells will be omitted from drawing");
	    TWforceRedraw() ;
	    break ;
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
	case DRAW_NETS:
	    drawNetS = INT_MAX ;
	    TWmessage( "Nets will be drawn" ) ;
	    TWforceRedraw() ;
	    break ;
	case IGNORE_NETS:
	    drawNetS = FALSE ;
	    TWmessage( "Nets will be omitted from drawing" ) ;
	    TWforceRedraw() ;
	    break ;
	case DRAW_PINS:
	    drawPinS = TRUE ;
	    TWmessage( "Pins will be drawn" ) ;
	    TWforceRedraw() ;
	    break ;
	case IGNORE_PINS:
	    drawPinS = FALSE ;
	    TWmessage( "Pins will be omitted from drawing" ) ;
	    TWforceRedraw() ;
	    break ;
	case DRAW_SINGLE_NET:
	    /* get a net from the user */
	    ok = FALSE ;
	    do {
		/* get string from user */
		reply = TWgetString( "Enter net number:") ;
		if(!(reply)){
		    continue ;
		}
		drawNetS = atoi( reply ) ;
		if( drawNetS > 0 && drawNetS <= numnetsG ){
		    ok = TRUE ;
		    TWforceRedraw() ;
		} else {
		    TWmessage( "ERROR:invalid net number!" ) ;
		    (void) sleep( (unsigned) 2 ) ;
		}
	    } while ( !(ok) ) ;
	    break ;
	case DRAW_SINGLE_CELL_MOVES:
	    single_cell_moveS = TRUE ;
	    break ;
	case IGNORE_SINGLE_MOVES:
	    single_cell_moveS = FALSE ;
	    break ;
	case DRAW_ORIENT:
	    drawOrientS = TRUE ;
	    TWmessage( "Orientation markers will be drawn" ) ;
	    TWforceRedraw() ;
	    break ;
	case IGNORE_ORIENT:
	    drawOrientS = FALSE ;
	    TWmessage("Orientation markers will be omitted from drawing");
	    TWforceRedraw() ;
	    break ;

	} /*********************** end graphics SWITCH *****************/
	if( auto_drawS && TWcheckExposure() ){
	    draw_the_data() ;
	}

    } 
    TWmessage("Continuing - to interupt program click on top menu window") ;
    /* use TWinterupt to turn off window enter/leave lights */
    TWinterupt() ;

} /* end process_graphics */


/* the graphics program can draw the results at each desired */
/* timestep. */
INT draw_the_data()
{

    INT  i ;
    INT  x ;
    INT  y ;
    INT  l, r, b, t ;
    INT  layer ;
    INT last_cell ;     /* end of cells to be drawn */
    PINBOXPTR  curPin ;
    char *pinname, *find_layer( /* pinname, layer */ ) ;

    if( avoidDump || !(doGraphicsG) || !(initS) ){
	return ;
    }
    TWstartFrame() ;
    TWmessage( "Drawing the data...Please wait" ) ;

    if( drawBlockS ){
	/* draw the desired standard cell rows if necessary */
	for( i = 1 ; i <= numRowsG ; i++ ) {
	    x = barrayG[i]->bxcenter ;
	    y = barrayG[i]->bycenter ;
	    l = x + barrayG[i]->bleft;
	    r = x + barrayG[i]->bright;
	    b = y + barrayG[i]->bbottom;
	    t = y + barrayG[i]->btop;
	    TWdrawCell( i, l,b,r,t, TWWHITE, NULL ) ;
	}
    }

    /* DRAW the CELLS */
    if( drawCellS ){
	i = 1 ; /* start with standard cells */
    } else {
	i = numcellsG + 1 ; /* start with pads and macros */
    }
    last_cell = lastpadG + actual_feed_thru_cells_addedG ;

    for( ; i <= last_cell ; i++ ) {
	draw_a_cell( i ) ;
    }

    /* now build net file */
    /* nets are the interconnections between the cells */
    if( drawNetS ){
	Ymst_init( get_max_pin() ) ;
	for( i=1;i<=numnetsG;i++){
	    /* this is the single net case */
	    if( drawNetS <= numnetsG && i != drawNetS ){
		continue ;
	    }
	    Ymst_clear() ;
	    for(curPin=netarrayG[i]->pins;curPin;curPin=curPin->next){
		Ymst_addpt( curPin->xpos, curPin->ypos ) ;
	    }
	    Ymst_draw() ;
	}
	Ymst_free() ;
    }

    /* now output pins */
    /* pins are the terminal points on a net */
    if( drawPinS ){
	for( i = 1 ; i <= maxtermG ; i++ ) {
	    curPin = tearrayG[i] ;
	    if( !(curPin)){
		continue ;
	    }
	    x =  curPin->xpos ;
	    y =  curPin->ypos ;
	    if( drawLabelS ){
		pinname = find_layer( curPin->pinname, &layer ) ;
		TWdrawPin( curPin->net,x-pinsizeS,y-pinsizeS,
		    x+pinsizeS,y+pinsizeS, PINCOLOR, pinname ) ;
	    } else {
		TWdrawPin( curPin->net,x-pinsizeS,y-pinsizeS,
		    x+pinsizeS,y+pinsizeS, PINCOLOR, NULL ) ;
	    }
	}
    }

    /* clear wait message and FLUSH OUTPUT BUFFER */
    TWmessage( NULL ) ;
    TWflushFrame() ;

} /* end draw_the_data */

VOID draw_a_cell( cell )
INT cell ;
{
    INT  x ;
    INT  y ;
    INT  l, r, b, t ;
    INT  pt ;
    INT  color ;
    INT  cell_temp ; /* cell temperature */
    INT  attempts ;
    CBOXPTR cptr ;
    PADBOXPTR  padptr ;
    char label[LRECL] ;
    char *labelptr ;
    FLOAT acc_rate ;

    cptr = carrayG[cell] ;
    x =  cptr->cxcenter ;
    y =  cptr->cycenter ;
    if( y == GATE_ARRAY_MAGIC_CONSTANT ){
	/* don't draw these */
	return ;
    }
    /* name the cell */
    /* draw cell labels if requested */
    if( drawLabelS ){
	sprintf(label,"C%d:%s",cell, carrayG[cell]->cname ) ; 
	labelptr = label ;
    } else {
	labelptr = NIL(char *) ;
    }
    cell_temp = heat_indexS[cell] ;
    attempts = heat_attemptS[cell] ;
    if( attempts <= 0 ){
	if( strncmp(carrayG[cell]->cname,"twfeed",6) == STRINGEQ ) {
	    color = TWYELLOW ;
	} else {
	    color = TWGREEN ;
	}
    } else {
	/* don't need to be very accurate here */
	acc_rate = (FLOAT) cell_temp / (FLOAT) attempts ;
	/* now pick the color base on acceptance rate */
	/* 44% + 10% error margin */
	if( acc_rate > (FLOAT) 0.48 ){
	    color = TWRED ;
	} else if( acc_rate < (FLOAT) 0.40 ){
	    /* 44% - 10% error margin */
	    color = TWBLUE ;
	} else {
	    /* in the 44% region */
	    color = TWORANGE ;
	}
    }
    padptr = cptr->padptr ;
    if( padptr && padptr->numcorners > 4 ){
	/* way to draw a rectilinear cell */
	TWarb_init() ;

	for( pt = 0; pt < padptr->numcorners; pt++ ){
	    /* rel position is a macro which calculates */
	    /* absolute pin loc - defined in relpos.h */
	    REL_POS( (INT) cptr->corient, 
		l, b,                               /* result */
		padptr->xpoints[pt],
		padptr->ypoints[pt],             /* cell relative */
		x, y ) ;                        /* cell center */

	    TWarb_addpt( l, b ) ;
	}
	TWdrawArb( cell, color, labelptr ) ;

    } else {
	l = cptr->tileptr->left ;
	r = cptr->tileptr->right ;
	b = cptr->tileptr->bottom ;
	t = cptr->tileptr->top ;
	YtranslateT( &l, &b, &r, &t, (INT) cptr->corient ) ;
	l += x ;
	r += x ;
	b += y ;
	t += y ;
	TWdrawCell( cell, l,b,r,t, color, labelptr ) ;
    }
    if( drawOrientS && padptr ){
	draw_fs( cptr ) ;
    }
} /* end draw a cell */


static draw_fs( cptr )
CBOXPTR cptr ;
{
    INT x[10], y[10] ;   /* only 10 points to an F */
    INT l, b, r, t ;     /* bounding box points */
    INT xout, yout ;     /* rotated points */
    INT wid ;            /* with of the F */
    INT pt ;             /* point counter */
    TIBOXPTR bounptr ;   /* cell's boundary */

    bounptr = cptr->tileptr ;
    l = bounptr->left ;
    b = bounptr->bottom ;
    r = bounptr->right ;
    t = bounptr->top ;
    wid = (INT) (0.25 * (DOUBLE)( t - b ) ) ;
    /* now set the points */
    x[0] = l ;         y[0] = b ;
    x[1] = l ;         y[1] = t ;
    x[2] = r ;         y[2] = t ;
    x[3] = r ;         y[3] = t - wid ;
    x[4] = l + wid ;   y[4] = y[3] ;
    x[5] = x[4] ;      y[5] = y[4] - wid ;
    x[6] = l + 2*wid ; y[6] = y[5] ;
    x[7] = x[6] ;      y[7] = y[6] - wid ;
    x[8] = x[5] ;      y[8] = y[7] ;
    x[9] = x[4] ;      y[9] = b ;
    TWarb_init() ;
    for( pt = 0; pt <= 9; pt++ ){
	/* rel position is a macro which calculates */
	/* absolute pin loc - defined in relpos.h */
	REL_POS( (INT) cptr->corient, 
	    xout, yout,                              /* result */
	    x[pt], y[pt],                        /* cell relative */
	    cptr->cxcenter, cptr->cycenter ) ;   /* cell center */

	TWarb_addpt( xout, yout ) ;
    }
    TWdrawArb( 0, FCOLOR, NIL(char *) ) ;
} /* end draw_fs */

erase_a_cell( cell, x, y )
INT cell ;
INT x, y ;
{
    INT  i, j ;
    INT  l, r, b, t ;
    INT  block ;
    INT  lobin, hibin ;
    INT  *cells_in_bins ;
    CBOXPTR cptr ;

    GRAPHICSABORT ;
    if( !(binptrG) || !(single_cell_moveS) || !(updateS) ){
	/* won't work if binptr is not valid */
	return ;
    }

    cptr = carrayG[cell] ;
    l = x + cptr->tileptr->left ;
    r = x + cptr->tileptr->right ;
    b = y + cptr->tileptr->bottom ;
    t = y + cptr->tileptr->top ;
    TWdrawCell( cell, l,b,r,t, TWWHITE, NULL ) ;
    /* now find the bins that it overlaps and redraw all the cells */
    /* in these bins */
    lobin = SetBin( l ) ; 
    hibin = SetBin( r ) ; 
    block = cptr->cblock ;
    for( i = lobin; i <= hibin ; i++ ){
	cells_in_bins = binptrG[block][i]->cell ;
	for( j = 1 ; j <= *cells_in_bins ; j++ ) {
	    draw_a_cell( cells_in_bins[j] ) ;
	}
    }
} /* end draw a cell */


/* dumps the data to a file for future study */
graphics_dump() 
{
    /* now change mode to dump to file */
    TWsetMode(1) ;
    /* dump the data to a file now instead of screen */
    draw_the_data() ;
    /* restore the state to previous condition and set draw to screen */
    TWsetMode(0) ;
}

/* see if uses wishes an interupt otherwise just draw the data */
check_graphics( drawFlag )
BOOL drawFlag ;
{
    if( doGraphicsG ){
	if( TWinterupt() ){
	    process_graphics() ;
	} else if( drawFlag && updateS ){
	    setGraphicWindow() ;
	    draw_the_data() ;
	}
    }
} /* end check_graphics */

graphics_cell_update( cell )
INT cell ;
{
    GRAPHICSABORT ;
    if( !(single_cell_moveS) || !(updateS) ){
	return ;
    }
    ASSERTNRETURN( cell > 0 && cell < heat_allocS, "graphics_cell_update",
	"cell out of bounds\n" ) ;
    heat_indexS[cell]++ ;
    heat_attemptS[cell]++ ;
    draw_a_cell( cell ) ;
} /* end graphics_cell_update */

graphics_cell_attempt( cell )
INT cell ;
{
    GRAPHICSABORT ;
    ASSERTNRETURN( cell > 0 && cell < heat_allocS, "graphics_cell_update",
	"cell out of bounds\n" ) ;
    heat_attemptS[cell]++ ;
} /* end graphics_cell_attempt */

reset_heat_index()
{
    INT i ; /* counter */

    GRAPHICSABORT ;
    for( i = 0; i < heat_allocS ; i++ ){
	heat_indexS[i] = 0 ;
	heat_attemptS[i] = 0 ;
    }
} /* end reset_heat_index */

set_update( flag )
BOOL flag ;
{
    updateS = flag ;
} /* end set_update */

#endif /* NOGRAPHICS */

/* close graphics window on fault */
closegraphics( )
{
    if( doGraphicsG ){
	G( TWcloseGraphics() ) ;
	doGraphicsG = FALSE ;
    }
} /* end closegraphics */
