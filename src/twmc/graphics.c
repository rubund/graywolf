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
FILE:	    graphics.c                                       
DESCRIPTION:graphics routines
CONTENTS:   
DATE:	    Feb 01, 1988 
REVISIONS:  Feb 26, 1989 - added moveCells so that cell bin structures
		could be updated.
			 - added determine_origin so that fixcells will
		be glued to the nearest corner of the bar.
			 - added drawLabel toggle and simple pick color
		procedure.
	    Mar 02, 1989 - changed argument to exitPgm.
			 - changed call to core adjustment routine.
	    Mar 11, 1989 - removed dummy routines and added graphics
		conditional compile switch.
	    Mar 16, 1989 - changed to new data structure.
	    Mar 27, 1989 - fixed problem with cell neighborhoods.
	    May  8, 1989 - added fit to data.
	    May 14, 1989 - attempted to add parasite mode for graphics.
	    May 18, 1989 - added drawBlocks code for partitioning.
	    Jun 21, 1989 - fixed problem with fixed cell swap classes.
	    Jun 23, 1989 - added reset cell, reconfig functions.
		modified setWindowSize to look at data.
	    Sep 25, 1989 - deleted menu items now in genrows.  Also
		now use TWstandard colors.
	    Apr 23, 1990 - added mst function, scaling of pins,
		and tell point functions.
	    Oct 14, 1990 - rewrote for new graphics routines and
		added arbitrary figures.
	    Sun Dec 16 00:37:44 EST 1990 - added analog header and
		fixed for new version of the dialog box.
	    Mon Feb  4 02:08:23 EST 1991 - fixed bug in edit cells and
		reworked fixCell calls.  Also added new wire estimator.
	    Thu Apr 18 01:34:10 EDT 1991 - made valid orientations
		functional.
	    Sat Apr 27 14:38:12 EDT 1991 - added draw_orient.
	    Sun May  5 14:28:47 EDT 1991 - now draw core at right
		coordinates.
	    Wed Jul 24 20:44:18 CDT 1991 - fixed problems with
		fixed cells.  Now there is an unfix mechanism.
	    Thu Aug 22 22:10:09 CDT 1991 - fixed problem with
		fixed cells moving during pairwise flips.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) graphics.c (Yale) version 3.17 10/18/91" ;
#endif

#ifndef NOGRAPHICS

#include <string.h>
#include <custom.h>
#include <dens.h>
#include <analog.h>
#include <yalecad/debug.h>
#include <yalecad/relpos.h>
#include <yalecad/colors.h>
#include <yalecad/draw.h>
#include <yalecad/dialog.h>
#include <yalecad/system.h>
#include <menus.h>


#define CELLEST     0
#define CELLBORDER  1
#define TRUECELL    2
#define INTRO       "Welcome to TimberWolfMC"


#define CELLCOLOR          TWGREEN
#define ORIGCOLOR          TWYELLOW
#define WIREESTCOLOR       TWORANGE
#define NETCOLOR           TWRED
#define PINCOLOR           TWBLACK
#define BINCOLOR           TWBLUE
#define BLACK              TWBLACK
#define NEIGHBORCOLOR      TWCYAN
#define FCOLOR             TWYELLOW

#define MODE         2
#define MENUP        "mc_menu"

/* #define DEVELOPMENU */
/* During development use TWread_menus in place of menuS */
/* to create menu record, ie.  TWread_menus(MENUP) */
#ifdef DEVELOPMENU
#define MENU   (TWMENUPTR ) TWread_menus(MENUP)
#else
#define MENU   menuS
#endif

/* #define DEVELOPDIALOG */
#ifndef DEVELOPDIALOG

#ifdef TI
#include <tidialog.h>
#else
#include <dialog.h>
#endif

#else /* DEVELOPDIALOG */
static TWDIALOGPTR dialogS ;
#endif /* DEVELOPDIALOG */

/* ***************************************************************** 
   DUMP CURRENT MC CONFIGURATION.  USER INTERFACE
*/
static BOOL avoidDump = FALSE ;
static BOOL drawPinS = FALSE ;   /* whether or not to draw pins */
static BOOL drawBinS = FALSE ;   /* whether or not to draw bins */
static BOOL drawLabelS = FALSE ;  /* whether to draw labels or not */
static BOOL ignoreStdMacroS=FALSE;/* normally draw the standard cell macros */
static INT  selectCellS ;        /* the current selected cell  */
static BOOL auto_drawS = TRUE ;/* whether to draw immediately after exp.*/
static BOOL drawNeighborS = TRUE; /* whether to draw neighborhoods */
static BOOL cleanFileS = FALSE ;   /* whether we need to delete old files */
static INT  drawNetS = 0 ; /* draw nets 0:none 1...n:net >numnets:all */
static BOOL drawBorderS = TRUE ; /* normal option - draw border otherwise cell tiles */
static BOOL drawGlobeS = TRUE ; /* turn off global routing tiles */
static BOOL drawGridS = TRUE ; /* draw the grid reference lines */
static BOOL drawWireEstS ; /* whether to draw wire estimation */
static INT  pinsizeS ;     /* size of the pin */
static BOOL movedCellS ;   /* lets us know whether we have to update cellbins */
static BOOL single_cell_moveS = FALSE ;
static BOOL drawFS = FALSE ;


/* Forward references */

INT draw_the_data() ;
static draw_fs();
static edit_cell();
static edit_field_string();
static edit_field_case();
static fix_the_cell();
static fix_the_cell2();

initMCGraphics( argc, argv, windowId )
INT argc, windowId ;
char *argv[] ;
{

    char *host ;

#ifndef DEVELOPMENU
    set_graphics_wait_menu( MENU ) ;
#endif

    if( !(doGraphicsG) ){
	TWinitGraphics(argc,argv,TWnumcolors(),TWstdcolors(),TRUE,MENU,
	    draw_the_data ) ;
	return ;
    }
	
    /* we need to find host for display */
    if(!(host = Ygetenv("DISPLAY"))) {
	M(WARNMSG,"initMCGraphics","Can't get environment variable ");
	M(MSG,NULL, "for display.  Aborting graphics...\n\n" ) ;
	avoidDump = TRUE ;
	doGraphicsG = FALSE ;
	return ;
    }
    if( windowId ){
	/* init windows as a parasite */
	if( !( TWinitParasite(argc,argv,TWnumcolors(),TWstdcolors(),
	    FALSE, MENU, draw_the_data, windowId ))){
	    M(ERRMSG,"initMCGraphics","Aborting graphics.");
	    doGraphicsG = FALSE ;
	    avoidDump = TRUE ;
	    return ;
	}
    } else {
	/* init window as a master */
	if(!(TWinitGraphics(argc,argv,TWnumcolors(),TWstdcolors(),FALSE,MENU,
	    draw_the_data ))){
	    M(ERRMSG,"initMCGraphics","Aborting graphics.");
	    doGraphicsG = FALSE ;
	    avoidDump = TRUE ;
	    return ;
	}
    }
    TWsetwindow( 0, 0, 10, 10 ) ;
    TWsetFrame(0) ;
    TWdrawCell( 0, 0,0,11,11, BINCOLOR, INTRO ) ;
    TWmessage("To interupt program click on top menu window") ;
    TWflushFrame() ;
    selectCellS = 0 ; /* initialize to no selected cells */
    /* use TWinterupt to turn off window enter/leave lights */
    TWinterupt() ;
    /* set the default pin size */
    pinsizeS = (track_spacingXG + track_spacingYG) / 4 - 1 ;
    pinsizeS = MAX( 1, pinsizeS ) ;

} /* end initMCGraphics */

setGraphicWindow() 
{
    INT  expand ;
    INT  minx ;
    INT  maxx ;
    INT  miny ;
    INT  maxy ;
    INT  xc, yc ;      /* cell center */
    INT  i ;           /* counter */
    CELLBOXPTR cptr ;  /* cell record pointer */

    regenorient( 1, endsuperG ) ; /* update bounding boxes if necessary */

    minx = blocklG ;
    miny = blockbG ;
    maxx = blockrG ;
    maxy = blocktG ;

    /* look thru cells to find overall bounding box */
    for( i = 1; i <= endpadsG ; i++ ){
	cptr = cellarrayG[i] ;
	xc = cptr->xcenter ;
	yc = cptr->ycenter ;
	minx = MIN( minx, cptr->bounBox[cptr->orient]->l + xc ) ;
	miny = MIN( miny, cptr->bounBox[cptr->orient]->b + yc ) ;
	maxx = MAX( maxx, cptr->bounBox[cptr->orient]->r + xc ) ;
	maxy = MAX( maxy, cptr->bounBox[cptr->orient]->t + yc ) ;
    }

    expand = MAX( maxy - miny, maxx - minx ) ;
    expand = (INT) (0.1 * (DOUBLE) expand ) ;
    TWsetwindow( minx - expand, miny - expand, 
	maxx + expand, maxy + expand ) ;
} /* end setGraphicWindow */


/* set what we are going to draw on a dump to the screen */
/* placement data, compaction data , etc. are valid */
set_graphic_context( context )
INT context ;
{
    if( context == PARTITION_PLACEMENT ){
	/* after placement ignore drawing the standard macros */
	ignoreStdMacroS = TRUE ;
    }
    setGraphicWindow() ;
} /* end set_graphic_context */ 

/* heart of the graphic system processes user input */
process_graphics()
{

    INT x1, y1, x2, y2 ; /* coordinates for fixing cells and neighhds */
    INT x, y ;           /* coordinates from pointer */
    INT i ;             /* temp variable */
    INT selection ;     /* the users pick */
    INT pick_cell() ;   /* get cell from user */
    char *reply ;       /* user reply to a querry */
    BOOL ok ;           /* loop until this value is true */
    char leftNotRight[2] ; /* reference to left or right side of core */
    char bottomNotTop[2] ; /* reference to bottom or top of core */
    char leftNotRight2[2]; /* reference to left or right side of core */
    char bottomNotTop2[2]; /* reference to bottom or top of core */
    DOUBLE scale ;      /* scale wireestimation */
    DOUBLE eval_ratio() ;/* returns a double */
    DOUBLE calc_core_factor() ; /* calculate new core */

    if(!(doGraphicsG)){
	return ;
    }

    /* data might have changed so show user current config */
    /* any function other that the draw controls need to worry about */
    /* this concurrency problem -  show user current config */
    setGraphicWindow() ;
    draw_the_data() ;
    /* use TWcheckExposure to flush exposure events since we just */
    /* drew the data */
    TWcheckExposure() ;
    TWmessage( "TimberWolfMC is waiting for your response..." ) ;

    regenorient( 1, endsuperG ) ; /* update bounding boxes if necessary */

    selection  = CANCEL ;
    movedCellS = FALSE ;
    while( selection != CONTINUE_PROGRAM ){ /* loop until exit */
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
	    funccostG = findcost() ;
	    doGraphicsG = FALSE ;
	    return ;
	case COLORS:
	    TWtoggleColors() ;
	    break ;
	case CONTINUE_PROGRAM:
	    break ;
	case DUMP_GRAPHICS:
	    graphics_dump() ;
	    break ;
	case FULLVIEW:
	    TWfullView() ;
	    break ;
	case GRAPHICS_UPDATE_ON:
	    set_dump_ratio( 1 ) ;
	    break ;
	case GRAPHICS_UPDATE_OFF:
	    set_dump_ratio( 155 ) ;
	    /* break the loop */
	    selection = CONTINUE_PROGRAM ;
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
	/* ********* end menu1 ************ */
	case CELL_NEIGHBORHOOD:
	    if(!(selectCellS = pick_cell() )){
		break ;
	    }
	    /* fix cell in a neighborhood */
	    /* use initialize corner to set statics in initialize.c */
	    initializeCorner( selectCellS ) ;
	    TWmessage( "[FIX_CELL_IN_NEIGHBORHOOD]:Pick or enter first point for neighborhood:") ;
	    TWgetPt2( &x1, &y1 ) ;
	    TWmessage( "Pick or enter second point for neighborhood:");
	    TWgetPt2( &x2, &y2 ) ;
	    determine_origin( &x1, &y1, leftNotRight, bottomNotTop ) ; 
	    determine_origin( &x2, &y2, leftNotRight2, bottomNotTop2 ) ; 
	    /* cell is fixed within a fixed neighborhood */
	    fixCell( NEIGHBORHOODFLAG, x1, leftNotRight, y1, bottomNotTop,
		x2, leftNotRight2, y2, bottomNotTop2 ) ;
	    updateFixedCells( FALSE ) ;
	    /* fix swap classes - put cell in unique cell class */
	    addClass( --unique_classG ) ;
	    TWmessage( NIL(char *) ) ;
	    selectCellS = 0 ;
	    TWforceRedraw() ;
	    break ;
	case EDIT_CELL:
	    if( selectCellS = pick_cell() ){
		edit_cell( selectCellS ) ;
		TWforceRedraw() ;
		selectCellS = 0 ;
	    }
	    break ;
	case FIX_CELL:
	    if(!(selectCellS = pick_cell() )){
		break ;
	    }
	    fix_the_cell( selectCellS ) ;

	    /* in addition do below */
	case FIX_CELL_BUT_ROT:
	    if(!(selectCellS)){
		if(!(selectCellS = pick_cell() )){
		    break ;
		}
	    }
	    fix_the_cell2( selectCellS ) ;
	    TWforceRedraw() ;
	    break ;
	case GROUP_CELLS:
	    TWmessage( "Not implemented yet" ) ;
	    break ;
	case MOVE_CELL:
	    if(!(selectCellS = pick_cell() )){
		break ;
	    }
	    if( cellarrayG[selectCellS]->fixed ){
		TWmessage("ERROR:cannot move fixed cell" ) ;
		(void) sleep( (unsigned) 2 ) ;
		TWmessage("Use EDIT_CELL to free cell" ) ;
		(void) sleep( (unsigned) 2 ) ;
		break ;
	    }
	    TWmessage( "Pick or enter a reference point relative to cell center:" ) ;
	    if( TWgetPt2( &x, &y ) ){
		/* we know from keyboard */
		TWmessage( "Enter new position x, y<cr>:" ) ;
		TWgetPt( &x1, &y1 ) ;
		cellarrayG[selectCellS]->xcenter = x1 - x ;
		cellarrayG[selectCellS]->ycenter = y1 - y ;
	    } else {
		/* from mouse */
		TWmessage( "Pick new position with mouse" ) ;
		/* get old position of cell */
		get_global_pos( selectCellS, &x1, &y1, &x2, &y2 ) ;
		TWmoveRect( &x1, &y1, &x2, &y2, x, y ) ;
		/* calculate new position for cell */
		cellarrayG[selectCellS]->xcenter = (x1+x2)/2 ; 
		cellarrayG[selectCellS]->ycenter = (y1+y2)/2 ; 
	    }
	    movedCellS = TRUE ;
	    funccostG = findcost() ;
	    selectCellS = 0 ;
	    draw_the_data() ;
	    TWcheckExposure() ;
	    break ;
	/* ********* end menu2 ************ */
	case DRAW_BINS:
	    drawBinS = TRUE ;
	    TWmessage( "Area bins will be drawn" ) ;
	    TWforceRedraw() ;
	    break ;
	case IGNORE_BINS:
	    drawBinS = FALSE ;
	    TWmessage( "Area bins will be omitted from drawing" ) ;
	    TWforceRedraw() ;
	    break ;
	case DRAW_BORDER:
	    drawBorderS = TRUE ;
	    TWmessage( "Cell border will be drawn") ;
	    TWforceRedraw() ;
	    break ;
	case DRAW_TILES:
	    drawBorderS = FALSE ;
	    TWmessage( "The cells tiles will be draw" ) ;
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
	case DRAW_NEIGHBORHD:
	    drawNeighborS = TRUE ;
	    TWmessage( "Cell neighborhoods will be drawn" ) ;
	    TWforceRedraw() ;
	    break ;
	case IGNORE_NEIGHBORHOOD:
	    drawNeighborS = TRUE ;
	    TWmessage( "Cell neighborhoods will be omitted from drawing");
	    TWforceRedraw() ;
	    break ;
	case DRAW_NETS:
	    drawNetS = numnetsG + 1 ;
	    TWmessage( "Nets will be drawn" ) ;
	    TWforceRedraw() ;
	    break ;
	case IGNORE_NETS:
	    drawNetS = FALSE ;
	    TWmessage( "Nets will be omitted from drawing" ) ;
	    TWforceRedraw() ;
	    break ;
	case DRAW_ORIENT:
	    drawFS = TRUE ;
	    TWmessage( "Orientation symbol will be drawn" ) ;
	    TWforceRedraw() ;
	    break ;
	case IGNORE_ORIENT:
	    drawFS = FALSE ;
	    TWmessage( "Orientation symbol will be omitted" ) ;
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
	case DRAW_WIRING_EST:
	    drawWireEstS = TRUE ;
	    TWmessage( "Wiring area estimation for cells will be drawn") ;
	    TWforceRedraw() ;
	    break ;
	case IGNORE_WIRING_EST:
	    drawWireEstS = FALSE ;
	    TWmessage( "Wiring area estimation will be omitted from drawing" ) ;
	    TWforceRedraw() ;
	    break ;
	case DRAW_GLOBE_AREAS:
	    drawGlobeS = TRUE ;
	    TWforceRedraw() ;
	    break ;
	case IGNORE_GLOBE_AREAS:
	    drawGlobeS = FALSE ;
	    TWforceRedraw() ;
	    break ;
	/* ********* end menu3 ************ */
	case CHANGE_ASPECT_RATIO:
	    ok = FALSE ;
	    while(!(ok)){
		reply = TWgetString( "Enter new aspect ratio:" ) ;
		chipaspectG = atof( reply ) ;
		if( chipaspectG >= 0.01 ){
		    ok = TRUE ;
		}
	    }
	    /* now change aspect ratio - use eval_ratio to find time */
	    (void) calc_core_factor( eval_ratio( iterationG ) ) ;
	    /* update graphic window since aspect ratio has changed */
	    TWforceRedraw() ;
	    setGraphicWindow() ;
	    break ;
	case DRAW_SINGLE_NET:
	    /* get a net from the user */
	    ok = FALSE ;
	    do {
		/* get string from user */
		reply = TWgetString( "Enter net number:") ;
		drawNetS = atoi( reply ) ;
		if( drawNetS > 0 && drawNetS <= numnetsG ){
		    TWforceRedraw() ;
		    ok = TRUE ;
		} else {
		    TWmessage( "ERROR:invalid net number!" ) ;
		    (void) sleep( (unsigned) 2 ) ;
		}
	    } while ( !(ok) ) ;
	    break ;
	case GRAPHICS_WAIT:
	    wait_for_userG = TRUE ;
	    TWmessage( "TimberWolfMC will now stop at breakpoints\n" );
	    break ;
	case NO_GRAPHICS_WAIT:
	    wait_for_userG = FALSE ; 
	    TWmessage( "TimberWolfMC will NOT stop at graphic breakpoints\n" );
	    break ;
	case DRAW_SINGLE_CELL_MOVES:
	    single_cell_moveS = TRUE ;
	    TWcolorXOR( WIREESTCOLOR, TRUE ) ;
	    TWcolorXOR( CELLCOLOR, TRUE ) ;
	    TWcolorXOR( TWRED, TRUE ) ;
	    break ;
	case IGNORE_SINGLE_CELL:
	    single_cell_moveS = FALSE ;
	    TWcolorXOR( WIREESTCOLOR, FALSE ) ;
	    TWcolorXOR( CELLCOLOR, FALSE ) ;
	    TWcolorXOR( TWRED, FALSE ) ;
	    break ;

	} /*********************** end graphics SWITCH *****************/
	if( auto_drawS && TWcheckExposure() ){
	    draw_the_data() ;
	}
	if( movedCellS ){
	    loadbins( CELLEST ) ; /* wire estimation on */
	    funccostG = findcost() ;
	    movedCellS = FALSE ;
	}
    } 

    /* update all costs */
    funccostG = findcost() ;
    TWmessage("Continuing - to interupt program click on top menu window") ;
    /* use TWinterupt to turn off window enter/leave lights */
    TWinterupt() ;

} /* end process_graphics */


/* find the cell in question */
INT pick_cell()
{

    INT i ;
    INT match_count ;        /* keep track of all cells that match */
    INT x, y ;               /* coordinates picked by user */
    INT cell ;               /* selected cell */
    INT l, r, b, t ;         /* cell sides */

    cell = 0 ;
    TWmessage("Pick cell by clicking any mouse button at center of cell");
    TWgetPt( &x, &y ) ;
    /* look thru all cells O(numcells) algorithm */
    match_count = 0 ;
    for( i = 1 ; i <= endpadsG; i++ ){
	get_global_pos( i, &l, &b, &r, &t ) ;
	/* see if cell boundary contains this point */
	if( x >= l && x <= r ){
	    if( y >= b && y <= t ){
		selectCellS = i ;
		match_count++ ;
	    }
	}
    } /* end loop */

    if( match_count == 0 ){
	TWmessage( "No cell selected" ) ;
	if( selectCellS ){  /* user didn't like any options */
	    selectCellS = 0 ;
	    draw_the_data() ; /* draw the data with highlight off */
	    TWcheckExposure() ;
	}
	return( 0 ) ;
    } else if( match_count == 1 ){
	draw_the_data() ; /* draw the data with highlight on */
	TWcheckExposure() ;
	sprintf( YmsgG, "Selected cell:%d - %s", cell,
	    cellarrayG[selectCellS]->cname ) ;
	TWmessage( YmsgG ) ;
	return( selectCellS ) ;
    } else {
	/* more than one match */
	TWmessage( "More than one match.  Choose correct cell" ) ;  
	(void) sleep( (unsigned) 2 ) ;
	cell = 0 ;
	while( TRUE ){
	    for( i = 1 ; i <= endpadsG; i++ ){
		get_global_pos( i, &l, &b, &r, &t ) ;
		/* see if cell boundary contains this point */
		if( x >= l && x <= r ){
		    if( y >= b && y <= t ){
			selectCellS = i ;
			draw_the_data() ;
			TWcheckExposure() ;

			/* give directions */
			sprintf( YmsgG, "Selected cell:%d - %s", cell,
			    cellarrayG[selectCellS]->cname ) ;
			TWmessage( YmsgG ) ;
			(void) sleep( (unsigned) 2 ) ;
			sprintf( YmsgG,"%s","If correct, enter <cr>. Otherwise ") ;
			strcat( YmsgG,
			    "enter n<cr> for next cell:") ;
			/* look for empty string - means we are satisfied */
			if(!(TWgetString(YmsgG))){
			    return( selectCellS ) ;
			}
		    }
		}
	    } /* end loop */
	}
    }

} /* end find_cell */

/* the graphics program can draw the results at each desired */
/* timestep. */
INT draw_the_data()
{

    INT  i ;
    INT  x ;
    INT  y ;
    INT  pt ;
    INT  max ;
    INT  grid ;
    INT  area ;
    INT  offset ;
    char *labelptr ;
    char label[LRECL] ;
    INT  x0, x1, y0, y1 ;
    PINBOXPTR  curPin ;
    BINBOXPTR  bp ;
    ANALOGPTR aptr ;
    CELLBOXPTR cellptr ;
    DOUBLE percent ;

    if( avoidDump || !(doGraphicsG) ){
	return( -1 ) ;
    }
    if( cleanFileS == TRUE ){
	Yrm_files( "DATA/*" ) ;
	cleanFileS = FALSE ;
    }
    TWstartFrame() ;
    TWmessage( "Drawing the data...Please wait" ) ;

    /* draw the cells */
    for( i = 1; i <= totalcellsG ; i++ ){
	twmc_draw_a_cell( i ) ;
    }

    if( selectCellS ){
	get_global_pos( selectCellS, &x0, &y0, &x1, &y1 ) ;
	TWhighLightRect( x0, y0, x1, y1 ) ;
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
    /* draw core region for reference */
    TWdrawLine( ++i, blocklG,blockbG,blocklG,blocktG,BLACK,NULL ) ;
    TWdrawLine( ++i, blocklG,blocktG,blockrG,blocktG, BLACK,NULL ) ;
    TWdrawLine( ++i, blockrG,blocktG,blockrG,blockbG, BLACK, NULL ) ;
    TWdrawLine( ++i, blockrG,blockbG,blocklG,blockbG, BLACK, NULL ) ;

    if( drawBinS ){
	/* draw bins for reference */
	for( x=1;x<maxBinXG;x++ ){
	    for( y=1;y<maxBinYG;y++ ){
		bp = binptrG[x][y] ;
		/* left edge */
		TWdrawLine( ++i,
		bp->left, bp->bottom,
		bp->left, bp->top, BINCOLOR, NULL ) ;

		/* top edge */
		TWdrawLine(i,
		bp->left, bp->top,
		bp->right, bp->top , BINCOLOR, NULL ) ;

		/* right edge */
		TWdrawLine(i,
		bp->right, bp->top,
		bp->right, bp->bottom, BINCOLOR, NULL ) ;

		/* bottom edge */
		TWdrawLine(i,
		bp->right, bp->bottom,
		bp->left, bp->bottom, BINCOLOR, NULL ) ;
	    }
	}
	for( x = 0 ; x <= maxBinXG ; x++ ) {
	    for( y = 0 ; y <= maxBinYG ; y++ ) {
		bp = binptrG[x][y] ;
		if( drawLabelS ){
		    /* name the cell */
		    sprintf(label,"%d", bp->penalty ) ; 
		    labelptr = label ;
		} else {
		    labelptr = NULL ;
		}
		area = (bp->right - bp->left) * (bp->top - bp->bottom) ;
		percent = (DOUBLE) bp->penalty / (DOUBLE) area ;
		if( percent < -0.75 ){
		    TWdrawRect( x, bp->left, bp->bottom, bp->right,
			bp->top, TWRED, labelptr ) ;
		} else if( percent < -0.50 ){
		    TWdrawRect( x, bp->left, bp->bottom, bp->right,
			bp->top, TWYELLOW, labelptr ) ;
		} else if( percent < -0.25 ){
		    TWdrawRect( x, bp->left, bp->bottom, bp->right,
			bp->top, TWBLUE, labelptr ) ;
		} else if( percent < 0.0 ){
		    TWdrawRect( x, bp->left, bp->bottom, bp->right,
			bp->top, TWORANGE, labelptr ) ;
		} else {
		    TWdrawRect( x, bp->left, bp->bottom, bp->right,
			bp->top, TWCYAN, labelptr ) ;
		}
	    }
	}
		
    } /* end test of binsOnS */

    if( drawGridS && gridGivenG ){
	Ygrid_getx( &grid, &offset ) ;
	/* draw gridding lines for reference */
	max = blockrG ;
	Ygridx ( &max ) ;
	for( i=1; i <= max; i++ ){
	    /* vertical edge */
	    x = i * grid ;
	    Ygridx( &x ) ;
	    TWdrawLine( i, x, 0, x, blockrG, BINCOLOR, NULL ) ;
	}
	Ygrid_gety( &grid, &offset ) ;
	max = blocktG ;
	Ygridy ( &max ) ;
	for( i=1; i <= max; i++ ){
	    /* horizontal edge */
	    y = i * grid ;
	    Ygridy( &y ) ;
	    TWdrawLine( i, 0, y, blocktG, y, BINCOLOR, NULL ) ;
	}
    } /* end test of drawGridS */

    /* draw neighborhoods if desired */
    if( drawNeighborS ){
	for( i=1; i<= endsuperG; i++ ){
	    draw_neighbors( i ) ;
	}
    }

    /* now output pins */
    /* pins are the terminal points on a net */
    if( drawPinS ){
	for( i = 1 ; i <= numpinsG ; i++ ) {
	    curPin = termarrayG[i] ;
	    if( drawLabelS ){
		labelptr = curPin->pinname ;
	    } else {
		labelptr = NULL ;
	    }
	    if( aptr = curPin->analog ){
		TWarb_init() ;
		cellptr = cellarrayG[curPin->cell] ;
		ASSERTNCONT( cellptr, "draw_the_data","cellptr NULL\n" ) ;

		for( pt = 0; pt < aptr->num_corners; pt++ ){
		    /* rel position is a macro which calculates */
		    /* absolute pin loc - defined in relpos.h */
		    REL_POS( cellptr->orient, 
			x, y,                               /* result */
			aptr->x_contour[pt],
			aptr->y_contour[pt],              /* cell relative */
			cellptr->xcenter, cellptr->ycenter ) ;  /* cell center */

		    TWarb_addpt( x, y ) ;
		}
		TWdrawArb( i, PINCOLOR, labelptr ) ;
	    } else {
		x =  curPin->xpos ;
		y =  curPin->ypos ;
		TWdrawPin( curPin->net,x-pinsizeS,y-pinsizeS,
		    x+pinsizeS,y+pinsizeS, PINCOLOR, labelptr ) ;
	    }
	}
    }

    /* clear wait message and FLUSH OUTPUT BUFFER */
    TWmessage( NULL ) ;
    TWflushFrame() ;
    return( 0 ) ;

} /* end draw_the_data */

twmc_draw_a_cell( cell )
{
    INT  pt ;
    INT  xc, yc ;
    INT  x, y ;
    INT  type ;
    INT  x0, x1, y0, y1 ;
    INT  l, r, b, t ;
    INT  *xvert ;
    INT  *yvert ;
    char label[LRECL] ;
    char *labelptr ;
    CELLBOXPTR cptr ;
    TILEBOXPTR tileptr ;
    MOVEBOXPTR pos ;
    MOVEBOX pos_buffer ;
    RTILEBOXPTR rtptr ;

    pos = &pos_buffer ;
    cptr = cellarrayG[cell] ;
    type = cptr->celltype ;
    if( type != CUSTOMCELLTYPE && type != SOFTCELLTYPE &&
	type != PADCELLTYPE && type != STDCELLTYPE ){
	return ;
    }
    if( ignoreStdMacroS && type == STDCELLTYPE ){
	return ;
    }
    xc =  cptr->xcenter ;
    yc =  cptr->ycenter ;
    if( drawLabelS ){
	/* name the cell */
	sprintf(label,"C%d:%s",cell, cptr->cname ) ; 
	labelptr = label ;
    } else {
	labelptr = NULL ;
    }
    if( !(drawBorderS) || drawWireEstS ){
	for( tileptr = cptr->tiles;tileptr;tileptr=tileptr->next ) {

	    l = tileptr->left ;
	    r = tileptr->right ;
	    b = tileptr->bottom ;
	    t = tileptr->top ;

	    /* first get orientation from  orientation */
	    Ytranslate( &l, &b, &r, &t, cptr->orient ) ;
	    pos->l = l ;
	    pos->r = r ;
	    pos->b = b ;
	    pos->t = t ;

	    l += xc ;
	    r += xc ;
	    b += yc ;
	    t += yc ;

	    /* wire estimation draw */
	    if( drawWireEstS && cell <= numcellsG ){
		pos->lw = tileptr->lweight ;
		pos->rw = tileptr->rweight ;
		pos->bw = tileptr->bweight ;
		pos->tw = tileptr->tweight ;
		Ytranslatef( &(pos->lw),&(pos->bw),&(pos->rw),&(pos->tw),
		    cptr->orient ) ;
		if( new_wire_estG ){
		    wireestxy2( pos,xc,yc ) ;
		} else {
		    wireestxy( pos,xc,yc ) ;
		} 
		x0 = pos->l ;
		x1 = pos->r ;
		y0 = pos->b ;
		y1 = pos->t ;
		TWdrawCell( cell, x0,y0,x1,y1, WIREESTCOLOR, NULL ) ;
	    }

	    if(!(drawBorderS)){
		/* normal cell draw */
		TWdrawCell( cell,l,b,r,t, CELLCOLOR,labelptr ) ;
	    }


	}
    } /* end tile case */

    if( cell <= numcellsG && routingTilesG && drawGlobeS ){
	for( rtptr = routingTilesG[cell];rtptr;rtptr=rtptr->next ){
	    TWdrawCell( cell, rtptr->x1 + xc, rtptr->y1 + yc,
		rtptr->x2 + xc, rtptr->y2 + yc, TWRED,labelptr ) ;
	}
    }

    if( drawBorderS ){
	/* normal way to draw the cells */
	TWarb_init() ;

	xvert = cptr->vertices->x ;
	yvert = cptr->vertices->y ;
	for( pt = 1; pt <= cptr->numsides; pt++ ){
	    /* rel position is a macro which calculates */
	    /* absolute pin loc - defined in relpos.h */
	    REL_POS( cptr->orient, 
		x, y,                               /* result */
		xvert[pt],
		yvert[pt],                        /* cell relative */
		xc, yc ) ;                        /* cell center */

	    TWarb_addpt( x, y ) ;
	}
	TWdrawArb( cell, CELLCOLOR, labelptr ) ;
    }
    if( drawFS ){
	draw_fs( cptr ) ;
    }

} /* end TWdrawCell */

static draw_fs( cptr )
CELLBOXPTR cptr ;
{
    INT x[10], y[10] ;   /* only 10 points to an F */
    INT l, b, r, t ;     /* bounding box points */
    INT xout, yout ;     /* rotated points */
    INT wid ;            /* with of the F */
    INT pt ;             /* point counter */
    BOUNBOXPTR bounptr ; /* cell's boundary */

    bounptr = cptr->bounBox[0] ;
    l = bounptr->l ;
    b = bounptr->b ;
    r = bounptr->r ;
    t = bounptr->t ;
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
	REL_POS( cptr->orient, 
	    xout, yout,                              /* result */
	    x[pt], y[pt],                      /* cell relative */
	    cptr->xcenter, cptr->ycenter ) ;   /* cell center */

	TWarb_addpt( xout, yout ) ;
    }
    TWdrawArb( 0, FCOLOR, NIL(char *) ) ;
} /* end draw_fs */

/* draw the neighborhood of a cell if it exists */
draw_neighbors( cell )
INT cell ;
{

    CELLBOXPTR ptr ;
    FIXEDBOXPTR fptr ;

    ptr = cellarrayG[cell] ;
    ASSERTNRETURN( ptr, "draw_neighbor", "cell ptr is null" ) ;
    /* for core cells check fixed record */
    /* for group cells check group record */
    if( ptr->celltype == GROUPCELLTYPE ){
	ASSERT( ptr->group, "draw_neighbors", "pointer null");
	fptr = ptr->group->fixed ;
    } else {
	fptr = ptr->fixed ;
    }
    if( fptr ){  /* means we have a fixed cell */
	if( fptr->fixedType != POINTFLAG ){
	    /* draw neighborhood */
	    /* left edge */
	    TWdrawLine( cell, fptr->x1, fptr->y1,
		fptr->x1, fptr->y2, NEIGHBORCOLOR, NULL ) ;

	    /* top edge */
	    TWdrawLine( cell, fptr->x1, fptr->y2,
		fptr->x2, fptr->y2, NEIGHBORCOLOR, NULL ) ;

	    /* right edge */
	    TWdrawLine( cell, fptr->x2, fptr->y2,
		fptr->x2, fptr->y1, NEIGHBORCOLOR, NULL ) ;

	    /* right edge */
	    TWdrawLine( cell, fptr->x2, fptr->y1,
		fptr->x1, fptr->y1, NEIGHBORCOLOR, NULL ) ;
	}

    } /* end test of fix cell */
} /* end draw_neighbors */

/* avoid dump when we don't want it */
dsetDump( flag )
BOOL flag ;
{
    avoidDump = flag ;
} /* end dsetDump */

/* get dump flag */
BOOL dgetDump()
{
    return( avoidDump ) ;
}

/* dumps the data to a file for future study */
graphics_dump() 
{
    /* now change mode to dump to file */
    TWsetMode(1) ;
    /* dump the data to a file now instead of screen */
    draw_the_data() ;
    /* restore the state to previous condition and set draw to screen */
    TWsetMode(0) ;
} /* end graphics_dump() */

static edit_cell( cell )
INT cell ;
{

#define NAMEF         3
#define CELLNUMF      4
#define XCENTERF      6
#define YCENTERF      8
#define ORIENTF       9
#define VALIDF        19
#define FIXEDF        43
#define VALIDSPACE    3
#define ORIENTBASE    ORIENTF+1
#define DIALOGNAME    "editcell"

    INT i ;              /* counter */
    INT offset ;         /* offset from base */
    INT status ;         /* status from check_valid_orient */
    char name[LRECL];    /* a scratch buffer for cellname */
    char cellnum[LRECL]; /* a scratch buffer for cellnum */
    char xcenter[LRECL]; /* a scratch buffer for xcenter */
    char ycenter[LRECL]; /* a scratch buffer for ycenter */
    CELLBOXPTR ptr ;     /* current cell record */
    TWDRETURNPTR answer ; /* return from user */

    TWmessage( "Edit cell" ) ;

#ifdef DEVELOPDIALOG
    dialogS = TWread_dialog( "mc.dialog" ) ;
    if( !(dialogS) ){
	return ; /* avoid crashes */
    }
#endif

    ptr = cellarrayG[ cell ] ;


    /* now set the cell information fields */
    /* the cell name */
    sprintf( name, "Name:    %s", ptr->cname ) ;
    edit_field_string( dialogS, NAMEF, name ) ;
    sprintf( cellnum, "Number:  %d", ptr->cellnum ) ;
    edit_field_string( dialogS, CELLNUMF, cellnum ) ;

    /* now set the x y centers */
    sprintf( xcenter, "%d", ptr->xcenter ) ;
    edit_field_string( dialogS, XCENTERF, xcenter ) ;

    sprintf( ycenter, "%d", ptr->ycenter ) ;
    edit_field_string( dialogS, YCENTERF, ycenter ) ;

    /* set orientation */
#ifdef TI
    edit_field_case( dialogS, ORIENTF, tw2ice(ptr->orient) + ORIENTBASE -1 ) ; 
#else
    edit_field_case( dialogS, ORIENTF, ptr->orient + ORIENTBASE ) ; 
#endif /* TI */
    for( i = 0; i <= 7; i++ ){
	offset = VALIDSPACE * i ;
	if( ptr->orientList[i] ){
	    dialogS[VALIDF+offset].group = VALIDF + offset + 1 ;
	} else {
	    dialogS[VALIDF+offset].group = VALIDF + offset + 2 ;
	}
    }

    if( ptr->fixed ){
	/* this means fixed is on */
	dialogS[FIXEDF].group = FIXEDF + 1 ;
    } else {
	dialogS[FIXEDF].group = FIXEDF + 2 ;
    } 

    /* initialization complete */

    if( answer = TWdialog( dialogS, DIALOGNAME, NULL) ) {

	/* if answer field has changed update position */
	if( answer[XCENTERF].bool ){
	    /* the xcenter has changed */
	    ptr->xcenter = atoi( answer[XCENTERF].string ) ;
	    movedCellS = TRUE ;
	} 
	if( answer[YCENTERF].bool ){
	    /* the ycenter has changed */
	    ptr->ycenter = atoi( answer[YCENTERF].string ) ;
	    movedCellS = TRUE ;
	} 
#ifdef TI
	/* check orientation fields */
	for( i = 0; i <= 7; i++ ){
	    if( answer[ORIENTBASE+i].bool ){
		ptr->orient = ice2tw(i+1) ;
		break ;
	    }
	}
#else 
	/* check orientation fields */
	for( i = 0; i <= 7; i++ ){
	    if( answer[ORIENTBASE+i].bool ){
		ptr->orient = i ;
		break ;
	    }
	}
#endif
	/* check for valid orientation fields */
	for( i = 0; i <= 7; i++ ){
	    offset = VALIDSPACE * i ;
	    if( answer[VALIDF+offset+1].bool ){
		ptr->orientList[i] = TRUE ;
	    } else {
		ptr->orientList[i] = FALSE ;
	    }
	}
	status = check_valid_orient( ptr ) ;
	if( status == -1 ){
	    TWmessage( 
	    "ERROR:No valid orientations. Setting 0 valid." ) ;
	    ptr->orientList[0] = TRUE ;
	    ptr->orient = 0 ;
	    movedCellS = TRUE ;
	    (void) sleep( (unsigned) 3 ) ;
	} else if( status == 1 ){
	    sprintf( YmsgG, 
	    "Orientation set to valid orientation:%d",
		ptr->orient ) ;
	    TWmessage( YmsgG ) ;
	    (void) sleep( (unsigned) 3 ) ;
	    movedCellS = TRUE ;
	}

	if( answer[FIXEDF+1].bool ){
	    /* this means fixed has been requested */
	    if(!(ptr->fixed)){
		fix_the_cell( cell ) ;
		fix_the_cell2( cell ) ;
	    }
	} else if( answer[FIXEDF+2].bool ){
	    /* this means fixed wants to be off */
	    if( ptr->fixed ){
		/* it is on turn it off */
		delete_fix_constraint( cell ) ;
	    }
	}
    }

} /* end edit_tiles */


static edit_field_string( dialog, field, string )
TWDIALOGPTR dialog;    /* dialog record */
INT field ;
char *string ;
{
    dialog[field].string = string ;

} /* end edit_field_string */

static edit_field_case( dialog, field, initcase )
TWDIALOGPTR dialog;    /* dialog record */
INT field ;
INT initcase ;
{
    TWDIALOGPTR fptr;    /* current dialog record */

    dialog[field].group = initcase ;

} /* end edit_field_case */

set_graphics_wait_menu( menus )
TWMENUBOX menus[] ;
{
    INT i ;   /* counter */
    for( i = 0; i < TWNUMMENUS; i++ ){
	/* look for the graphics wait menu */
	if( strcmp( menus[i].item, "Graphics Wait") ==
	    STRINGEQ ){
	    /* we found the graphics box */
	    /* change to current state */
	    if( wait_for_userG ){
		menus[i].bool_init = TRUE ;
	    } else {
		menus[i].bool_init = FALSE ;
	    }
	    break ;
	}
    }
}

static fix_the_cell( cell )
INT cell ;
{
    INT i ; /* counter */

    /* turn off all rotational possibibilites */
    for( i = 0 ; i < 8 ; i++ ) {
	if( i != cellarrayG[cell]->orient ){
	    cellarrayG[cell]->orientList[i] = FALSE ;
	} else {
	    cellarrayG[cell]->orientList[i] = TRUE ;
	}
    }
    cellarrayG[cell]->orientList[HOWMANYORIENT] = 1 ;
} /* end fix_the_cell */

static fix_the_cell2( cell )
INT cell ;
{
    INT x1, y1 ;
    char leftNotRight[2] ; /* reference to left or right side of core */
    char bottomNotTop[2] ; /* reference to bottom or top of core */

    /* use initialize corner to set statics in initialize.c */
    initializeCorner( cell ) ;
    /* now fixcell */
    x1 = cellarrayG[cell]->xcenter ;
    y1 = cellarrayG[cell]->ycenter ;
    sprintf( YmsgG, "Fixing cell:%d - %s @ (%d,%d)",
	cell, cellarrayG[cell]->cname, x1, y1 ) ;
    TWmessage( YmsgG ) ;
    determine_origin( &x1, &y1, leftNotRight, bottomNotTop ) ; 
    /* fix cell at x1, y1 using orign from determine origin */
    fixCell( POINTFLAG, x1, leftNotRight, y1, bottomNotTop, 
	0, "L", 0, "B") ;
    updateFixedCells( FALSE ) ;
    /* fix swap classes - put cell in unique cell class */
    addClass( --unique_classG ) ;
    TWmessage( NIL(char *) ) ;
    selectCellS = 0 ;
} /* end fix_the_cell2 */

#endif /* NOGRAPHICS */
