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
FILE:	    draw.c                                       
DESCRIPTION:This file draw the data structures to the screen 
CONTENTS:   initgraphics()
DATE:	    Aug  9, 1989 
REVISIONS:  Feb  7, 1990 - took total row length out of procedure calls.
	    Feb 28, 1990 - added tile dialog box.
	    Sep 25, 1990 - added modify core area.
	    Oct 14, 1990 - updated for new graphics.
	    Oct 21, 1990 - fixed dialog boxes.
	    Dec  7, 1990 - rewrote and reorganized graphics functions.
		Added merge functions, align function, set parameter
		menus.  Also modified edit tile dialog box.
	    Dec 15, 1990 - added divide tile and reset tiles.
	    Dec 28, 1990 - added edit and align functions for macros.
		Also added divide tiles L/R.
	    Mon Jan  7 18:21:05 CST 1991 - now announce
		presence of restart file.  Also check
		for errors during read_vertices.
	    Fri Jan 25 17:50:54 PST 1991 - added mirror row feature.
	    Sat Jan 26 16:24:18 PST 1991 - made feeds a double
		and moved force rows to a call in main.c
	    Sat Feb 23 00:32:10 EST 1991 - disallow macro moves during
		partitioning.
	    Tue Mar 12 16:52:19 CST 1991 - added numrows function.
	    Thu Apr 18 00:57:25 EDT 1991 - now genrows can rotate
		macros.
	    Sun Apr 21 21:48:10 EDT 1991 - fixed problem with macro
		orientation and now follow control menu convention.
	    Sat Sep 21 15:37:36 EDT 1991 - added memory capability.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) draw.c (Yale) version 3.22 5/14/92" ;
#endif

#include <string.h>
#include <yalecad/base.h>
#include <yalecad/debug.h>
#include <globals.h>

#ifndef NOGRAPHICS

#define MENUP   "genrows_menu"

#include <yalecad/file.h>
#include <yalecad/draw.h>
#include <yalecad/colors.h>
#include <yalecad/message.h>
#include <yalecad/dialog.h>
#include <yalecad/relpos.h>

#define FCOLOR   TWYELLOW

static BOOL auto_drawS=TRUE ;/* whether to draw immediately after exp.*/
static BOOL drawLabelS=FALSE ; /* whether to draw labels */
static BOOL drawTileS=TRUE ; /* whether to draw tiles */
static BOOL drawMacroS=TRUE ; /* whether to draw macros */
static BOOL drawRowS=TRUE ;  /* whether to draw stdcell rows */
static BOOL drawOrientS=FALSE ;  /* whether to draw macro orientation label */
static BOOL selectMacroS = 0 ; /* current chosen macro */
static TILE_BOX *selected_tileS = NULL ;/* current chosen tile */
static ROW_BOX *selected_rowS = NULL ;/* current chosen row */


/* #define DEVELOPMENU */
/* During development use TWread_menus in place of menuS */
/* to create menu record, ie.  TWread_menus(MENUP) */
#ifdef DEVELOPMENU
#define MENU   TWread_menus(MENUP)
#else
#define MENU   menuS
#endif

/* #define DEVELOPDIALOG */

#ifndef DEVELOPDIALOG
#include <dialog.h>
#include <dialog2.h>
#include <dialog3.h>
#else
static TWDIALOGPTR dialogS ;
static TWDIALOGPTR row_dialogS ;
static TWDIALOGPTR macro_dialogS ;
#endif

#include <menus.h>

static draw_tile();
static draw_macro();
static draw_fs();
static last_chance();
static no_move_message();
static save_for_do();
static update_macro();
static graphics_dump();
static INT pick_macro();
static TILE_BOX *pick_tile();
static ROW_BOX *pick_row();
static BOOL edit_tiles();
static edit_macro();
static update_vertices();
static rotate_vertices();
static find_nearest_corner();
static highlight_corner();
static outm();

initgraphics( argc, argv, windowId )
INT argc, windowId ;
char *argv[] ;
{

    char *host ;
    char *Ygetenv() ;
    extern INT draw_the_data() ;


    if( !(graphicsG) ){
	return ;
    }
    /* we need to find host for display */
    if(!(TWcheckServer() )){
	M(MSG,NULL, "Aborting graphics...\n\n" ) ;
	graphicsG = FALSE ;
	return ;
    }
    if( windowId ){
	/* init windows as a parasite */
	if( !( TWinitParasite(argc,argv,TWnumcolors(),
	    TWstdcolors(),FALSE,MENU, draw_the_data, windowId ))){
	    M(ERRMSG,"initgraphics","Aborting graphics.");
	    graphicsG = FALSE ;
	    return ;
	}
    } else {
	/* init window as a master */
	if(!(TWinitGraphics(argc,argv,TWnumcolors(),TWstdcolors(),
	    FALSE,MENU, draw_the_data ))){
	    M(ERRMSG,"initgraphics","Aborting graphics.");
	    graphicsG = FALSE ;
	    return ;
	}
    }
    TWsetMode(0) ;
    TWsetwindow( 0, 0, 10, 10 ) ;
    TWdrawMenus() ;
    TWflushFrame() ;

} /* end initgraphics */


/* how to draw the data */
INT
draw_the_data()
{
    INT      i ;            /* counter */
    INT      l, b, r, t ;   /* core dimensions */
    INT      color ;        /* current color */
    INT      macro ;        /* current macro */
    char     label[LRECL] ; /* make a label buffer */
    char     *labelptr ;    /* pointer to current label */
    TILE_BOX *tileptr ;     /* current tile */
    ROW_BOX  *rowptr  ;     /* current row being output */
    ROW_BOX  *segment ;     /* current segment being output */
    ROW_BOX  *get_rowptr(); /* get data structure */

    if( !(graphicsG) ){
	return ;
    }
    /* initialize screen */
    TWstartFrame() ;
    /**** draw the MC calculated core *******/
    get_core( &l, &b, &r, &t, FALSE ) ;
    TWdrawCell( 0, l, b, r, t, TWWHITE, NULL ) ;
    /**** draw the tiles *******/
    if( drawTileS ){
	/* draw all but last tile */
	for( tileptr=tile_listG;tileptr->next;tileptr=tileptr->next ){
	    draw_tile( tileptr ) ;
	}
    }
    /**** draw the rows *******/
    if( drawRowS ){
	for( rowptr=get_rowptr();rowptr;rowptr=rowptr->next_row ){

	    if( rowptr->class ){
		sprintf( label, "SEGMENT:%d CLASS:%d", rowptr->seg, rowptr->class);
	    } else {
		sprintf( label, "SEGMENT:%d CLASS:1", rowptr->seg ) ;
	    }
	    if( rowptr->mirror ){
		strcat( label, " MIRROR" ) ;
	    }
	    if( drawLabelS ){
		labelptr = label ;
	    } else {
		labelptr = NULL ;
	    }

	    if( feeds_per_segG > 0 ){
		if( rowptr->next_segment ){
		    /* means we have more that one segment to row */
		    for( segment=rowptr;segment;segment=segment->next_segment){
			if( segment->class ){
			    sprintf( label, "SEGMENT:%d CLASS:%d",
			    segment->seg, segment->class);
			} else {
			    sprintf( label, "SEGMENT:%d CLASS:1", segment->seg ) ;
			}
			if( rowptr->mirror ){
			    strcat( label, " MIRROR" ) ;
			}
			if( drawLabelS ){
			    labelptr = label ;
			} else {
			    labelptr = NULL ;
			}
			r = segment->urx - feeds_per_segG ;
			if( r > segment->llx ){
			    /* make sure we have two valid segments */
			    TWdrawCell( segment->seg, segment->llx, segment->lly,
					r, segment->ury,
					TWBLUE, labelptr ) ;
			    TWdrawCell( segment->seg, r, segment->lly,
					segment->urx, segment->ury,
					TWYELLOW, NULL ) ;
			} else {
			    TWdrawCell( segment->seg, segment->llx, segment->lly,
					segment->urx, segment->ury,
					TWBLUE, labelptr ) ;
			}
			if( segment == selected_rowS ){
			    TWdrawCell( segment->seg, segment->llx, segment->lly,
					segment->urx, segment->ury,
					TWVIOLET, labelptr ) ;
			}

		    } /* end for loop */

		} else {
		    r = rowptr->urx - feeds_per_segG ;
		    if( r > rowptr->llx ){
			/* make sure we have two valid segments */
			TWdrawCell( rowptr->seg, rowptr->llx, rowptr->lly,
				    r, rowptr->ury,
				    TWBLUE, labelptr ) ;
			TWdrawCell( rowptr->seg, r, rowptr->lly,
				    rowptr->urx, rowptr->ury,
				    TWYELLOW, NULL ) ;
		    } else {
			TWdrawCell( rowptr->seg, rowptr->llx, rowptr->lly,
				    rowptr->urx, rowptr->ury,
				    TWBLUE, labelptr ) ;
		    }
		    if( rowptr == selected_rowS ){
			TWdrawCell( rowptr->seg, rowptr->llx, rowptr->lly,
				    rowptr->urx, rowptr->ury,
				    TWVIOLET, labelptr ) ;
		    }
		}
	    } else {
		if( rowptr->next_segment ){
		    /* means we have more that one segment to row */
		    for( segment=rowptr;segment;segment=segment->next_segment){
			if( segment->class ){
			    sprintf( label, "SEGMENT:%d CLASS:%d",
			    segment->seg, segment->class);
			} else {
			    sprintf( label, "SEGMENT:%d CLASS:1", segment->seg ) ;
			}
			if( rowptr->mirror ){
			    strcat( label, " MIRROR" ) ;
			}
			TWdrawCell( segment->seg, segment->llx, segment->lly,
				    segment->urx, segment->ury,
				    TWBLUE, labelptr ) ;
			if( segment == selected_rowS ){
			    TWdrawCell( segment->seg, segment->llx, segment->lly,
					segment->urx, segment->ury,
					TWVIOLET, labelptr ) ;
			}

		    } /* end for loop */

		} else {
		    TWdrawCell( rowptr->seg, rowptr->llx, rowptr->lly,
				rowptr->urx, rowptr->ury,
				TWBLUE, labelptr ) ;
		    if( rowptr == selected_rowS ){
			TWdrawCell( rowptr->seg, rowptr->llx, rowptr->lly,
				    rowptr->urx, rowptr->ury,
				    TWVIOLET, labelptr ) ;
		    }
		}
	    }
	} /* end for loop */
    } /* end drawRowS */

    if( drawMacroS ){
	for( macro = 1; macro <= num_macrosG; macro++ ){
	    draw_macro( macro, TWORANGE ) ;
	}
    }
    if( selectMacroS ){
	draw_macro( selectMacroS, TWVIOLET ) ;
    }
    
    /* FLUSH OUTPUT BUFFER */
    TWflushFrame() ;

} /* end draw_the_data */
/* ***************************************************************** */

static draw_tile( tileptr )
TILE_BOX *tileptr ;     /* current tile */
{
    INT      color ;        /* current color */
    char     label[LRECL] ; /* make a label buffer */
    char     *labelptr ;    /* pointer to current label */

    if( drawLabelS ){
	sprintf( label, "TILE:%d", tileptr->name ) ;
	labelptr = label ;
    } else {
	labelptr = NULL ;
    }
    /* determine color base on legality */
    if( tileptr == selected_tileS ){
	color = TWVIOLET ;
    } else if( tileptr->illegal ){
	color = TWRED ;
    } else {
	color = TWGREEN ;
    }
    TWdrawCell( tileptr->name, tileptr->llx , tileptr->lly , 
		   tileptr->urx , tileptr->ury,
		   color, labelptr ) ;
} /* end draw_tile */

static draw_macro( macro, color )
INT macro ;
INT color ;
{
    INT i ;
    char *labelptr ;        /* name of cell */
    char label[LRECL] ;     /* buffer for name */
    VERTEXPTR *vptr ;       /* current vertex array */
    VERTEXPTR pt_ptr ;      /* current point */
    MACROPTR  mptr ;        /* current macro */

    mptr = macroArrayG[macro] ;
    vptr = mptr->vertices ;
    TWarb_init() ;
    for( i = 0 ; i < mptr->num_vertices ; i++ ) {
	pt_ptr = vptr[i] ;
	TWarb_addpt( pt_ptr->x, pt_ptr->y ) ;
	D( "genrows/macropts",
	    sprintf( label, "%d", i ) ;
	    TWdrawString( pt_ptr->x, pt_ptr->y, TWRED, label ) ;
	) ;
    }
    if( drawLabelS ){
	sprintf( label, "C:%d", macro ) ;
	labelptr = label ;
    } else {
	labelptr = NULL ;
    }
    TWdrawArb( macro, color, labelptr ) ;
    if( drawOrientS ){
	draw_fs( mptr ) ;
    }
} /* end draw_macro */

static draw_fs( mptr )
MACROPTR  mptr ;        /* current macro */
{
    INT i ;              /* counter */
    INT x[10], y[10] ;   /* only 10 points to an F */
    INT l, b, r, t ;     /* bounding box points */
    VERTEXPTR pt_ptr ;   /* current point */
    VERTEXPTR *vptr ;    /* current vertex array */
    INT xout, yout ;     /* rotated points */
    INT wid ;            /* with of the F */
    INT pt ;             /* point counter */
    INT xc, yc ;         /* center of cell */

    l = INT_MAX ;
    b = INT_MAX ;
    r = INT_MIN ;
    t = INT_MIN ;
    vptr = mptr->vertices ;
    xc = mptr->xcenter ;
    yc = mptr->ycenter ;
    for( i = 0 ; i < mptr->num_vertices ; i++ ) {
	pt_ptr = vptr[i] ;
	if( mptr->orient <= 3 ){
	    l = MIN( l, pt_ptr->x - xc ) ;
	    b = MIN( b, pt_ptr->y - yc ) ;
	    r = MAX( r, pt_ptr->x - xc ) ;
	    t = MAX( t, pt_ptr->y - yc ) ;
	} else {
	    /* opposite aspect ratio */
	    l = MIN( l, pt_ptr->y - yc ) ;
	    b = MIN( b, pt_ptr->x - xc ) ;
	    r = MAX( r, pt_ptr->y - yc ) ;
	    t = MAX( t, pt_ptr->x - xc ) ;
	}
    }
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
	REL_POS( mptr->orient, 
	    xout, yout,                              /* result */
	    x[pt], y[pt],                      /* cell relative */
	    xc, yc ) ;   /* cell center */

	TWarb_addpt( xout, yout ) ;
    }
    TWdrawArb( 0, FCOLOR, NULL ) ;
} /* end draw_fs */

/* set the size of the graphics window */
setGraphicWindow() 
{
    INT l, b, r, t ;
    INT expand ;

    get_core( &l, &b, &r, &t, TRUE ) ;
    expand = (int) (0.10 * (DOUBLE) (r - l) ) ;
    expand = MAX( expand,
    		  (int) (0.10 * (DOUBLE) (t - b) ) ) ;
    l -= expand ;
    r += expand ;
    b -= expand ;
    t += expand ;
    TWsetwindow( l, b, r, t ) ;
    
} /* end draw_changraph */

/* ***************************************************************** 
   USER INTERFACE
*/

/* heart of the graphic system processes user input */
void
process_graphics()
{

    INT x1, y1, x2, y2 ; /* coordinates for fixing cells and neighhds */
    INT x, y ;           /* coordinates from pointer */
    INT i ;             /* temp variable */
    INT rows ;          /* number of stdcell rows */
    INT net ;           /* net for minimum spanning tree */
    INT selection ;     /* the users pick */
    INT temp ;          /* temporary variable for atoi calc */
    INT minlength ;     /* minimum length of a tile */
    DOUBLE tempf ;      /* user reply transformed to float */
    /* static INT pick_macro(); */ /* get macro from user */
    INT pick_macro(); /* get macro from user */
    char *reply ;       /* user reply to a querry */
    char filename[LRECL];/* file name */
    BOOL ok ;           /* loop until this value is true */
    BOOL save_memory ;  /* save the memory state */
    FILE *fp ;          /* restore file */
    TILE_BOX *tile ;    /* traverse list of tiles */
    /* static TILE_BOX *pick_tile() ; */ /* used to pick a tile */
    TILE_BOX *pick_tile() ; /* used to pick a tile */
    TILE_BOX *cur_tile ; /* currently lit up tile */
    /* static ROW_BOX *pick_row() ; */ /* used to pick a row */
    ROW_BOX *pick_row() ; /* used to pick a row */

    sprintf( filename, "%s.gsav", cktNameG ) ;
    if( YfileExists( filename) ){
	sprintf( YmsgG, "Note: restore state file exists:%s\n\n",
	cktNameG ) ;
	M( MSG, NULL, YmsgG ) ;
    }

    /* data might have changed so show user current config */
    /* any function other that the draw controls need to worry about */
    /* this concurrency problem -  show user current config */
    setGraphicWindow() ;
    draw_the_data() ;
    /* use TWcheckExposure to flush exposure events since we just */
    /* drew the data */
    TWcheckExposure() ;
    if(!(wait_for_userG)){
	if (last_chanceG) {
	   wait_for_userG = TRUE ;
	   last_chance() ;
	}
	return; 
    }
    TWmessage( "Genrows is waiting for your response..." ) ;

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
	    if( invalidG ){
		TWmessage( "ERROR:you may not quit graphics since errors exist");
		selection = CANCEL ;
		break ;
	    } else {
		TWcloseGraphics() ;
		/* update all costs and reload cells */
		return ;
	    }
	case COLORS:
	    TWtoggleColors() ;
	    break ;
	case CONTINUE_PGM:
	    if( invalidG ){
		TWmessage( "ERROR:you may not continue since errors exist" ) ;
		selection = CANCEL ;
	    }
	    break ;
	case DUMP_GRAPHICS:
	    graphics_dump() ;
	    break ;
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
	case ALIGN_MACRO_IN_X:
	    if( noMacroMoveG ){
		no_move_message() ;
		break ;
	    }
	    if(!(selectMacroS = pick_macro( 
	    "Select the reference macro by clicking any mouse button in cell center"))){
		break ;
	    }
	    draw_macro( selectMacroS, TWVIOLET ) ;
	    TWmessage( "Now pick the reference corner|center for alignment" ) ;
	    TWgetPt2( &x, &y ) ;
	    find_nearest_corner( selectMacroS, x, y, &x1, &y1 ) ;
	    highlight_corner( selectMacroS, x1, y1 ) ;
	    if(!(selectMacroS = pick_macro(
	    "Select the macro to be aligned by clicking any mouse button in cell center"))){
		break ;
	    }
	    draw_macro( selectMacroS, TWVIOLET ) ;
	    TWmessage( "Now pick the corner|center you wish to align" ) ;
	    TWgetPt2( &x, &y ) ;
	    find_nearest_corner( selectMacroS, x, y, &x2, &y2 ) ;
	    highlight_corner( selectMacroS, x2, y2 ) ;
	    if( x1 == x2 ){
		/* no work to be done */
		draw_the_data() ;
		TWcheckExposure() ;
		break ;
	    }
	    save_for_do( UNDO ) ;
	    /* calculate the new center of the macro */
	    x = macroArrayG[selectMacroS]->xcenter + x1 - x2 ;
	    y = macroArrayG[selectMacroS]->ycenter ;
	    update_vertices( selectMacroS, x, y ) ;
	    update_macro() ;
	    break ;
	case ALIGN_MACRO_IN_Y:
	    if( noMacroMoveG ){
		no_move_message() ;
		break ;
	    }
	    if(!(selectMacroS = pick_macro(
	    "Select the reference macro by clicking any mouse button in cell center"))){
		break ;
	    }
	    draw_macro( selectMacroS, TWVIOLET ) ;
	    TWmessage( "Now pick the reference corner|center for alignment" ) ;
	    TWgetPt2( &x, &y ) ;
	    find_nearest_corner( selectMacroS, x, y, &x1, &y1 ) ;
	    highlight_corner( selectMacroS, x1, y1 ) ;
	    if(!(selectMacroS = pick_macro(
	    "Select the macro to be aligned by clicking any mouse button in cell center"))){
		break ;
	    }
	    draw_macro( selectMacroS, TWVIOLET ) ;
	    TWmessage( "Now pick the corner|center you wish to align" ) ;
	    TWgetPt2( &x, &y ) ;
	    find_nearest_corner( selectMacroS, x, y, &x2, &y2 ) ;
	    highlight_corner( selectMacroS, x2, y2 ) ;
	    if( y1 == y2 ){
		/* no work to be done */
		draw_the_data() ;
		TWcheckExposure() ;
		break ;
	    }
	    save_for_do( UNDO ) ;
	    /* calculate the new center of the macro */
	    x = macroArrayG[selectMacroS]->xcenter ;
	    y = macroArrayG[selectMacroS]->ycenter + y1 - y2 ;
	    update_vertices( selectMacroS, x, y ) ;
	    update_macro() ;
	    break ;
	case ALIGN_ROWS:
	    save_for_do( UNDO ) ;
	    selected_tileS = pick_tile(
	    "Pick reference tile whose row separation is desired") ;
	    if( selected_tileS == 0 ){
		break ;
	    }
	    temp = selected_tileS->channel_separation ;
	    draw_tile( selected_tileS ) ; /* show user which tile selected */
	    selected_tileS = pick_tile(
	    "Pick destination tile whose rows need to be aligned") ;
	    if( selected_tileS == 0 ){
		draw_the_data() ;
		TWcheckExposure() ;
		break ;
	    }
	    draw_tile( selected_tileS ) ; /* show user which tile selected */
	    selected_tileS->channel_separation = temp ;
	    selected_tileS = NULL ;
	    remakerows() ;
	    draw_the_data() ;
	    TWcheckExposure() ;
	    save_for_do( REDO ) ;
	    break ;
	case EDIT_ROW:
	    if( shortRowG ){
		TWmessage( "You cannot edit row in KEEP SHORT ROW mode") ;
		break ;
	    }
	    selected_rowS = pick_row() ;
	    if(!(selected_rowS)){
		break ;
	    }
	    save_for_do( UNDO ) ;
	    draw_the_data() ; /* show user which row is selected */
	    edit_row( selected_rowS ) ;
	    selected_rowS = NULL ;
	    draw_the_data() ;
	    TWcheckExposure() ;
	    save_for_do( REDO ) ;
	    break ;
	case EDIT_MACRO:
	    if( noMacroMoveG ){
		no_move_message() ;
		break ;
	    }
	    if(!(selectMacroS = pick_macro(NULL) )){
		break ;
	    }
	    draw_macro( selectMacroS, TWVIOLET ) ;
	    TWmessage( "Now pick the reference corner|center" ) ;
	    TWgetPt2( &x, &y ) ;
	    find_nearest_corner( selectMacroS, x, y, &x1, &y1 ) ;
	    highlight_corner( selectMacroS, x1, y1 ) ;
	    save_for_do( UNDO ) ;
	    edit_macro( selectMacroS, x1, y1 ) ;
	    update_macro() ;
	    break ;

	case EDIT_TILE:
	    selected_tileS = pick_tile(NULL) ;
	    if( selected_tileS == 0 ){
		break ;
	    }
	    save_for_do( UNDO ) ;
	    draw_tile( selected_tileS ) ; /* show user which tile selected */
	    if(!(edit_tiles( selected_tileS))){
		/* nothing changed so don't do any work */
		cur_tile = selected_tileS ;
		selected_tileS = NULL ;/* turn off tile */
		draw_tile( cur_tile ) ;
		break ;
	    }
	    selected_tileS = NULL ;
	    remakerows() ;
	    draw_the_data() ;
	    TWcheckExposure() ;
	    save_for_do( REDO ) ;
	    break ;
	case KEEP_SHORT_ROW:
	    shortRowG = TRUE ;
	    TWmessage( "Genrows will keep short row" ) ;
	    remakerows() ;
	    draw_the_data() ;
	    TWcheckExposure() ;
	    break ;
	case DISCARD_SHORT_ROW:
	    TWmessage( "Genrows will discard short row and round up if possible" ) ;
	    shortRowG = FALSE ;
	    remakerows() ;
	    draw_the_data() ;
	    TWcheckExposure() ;
	    break ;
	case MODIFY_CORE_AREA:
	    if( noMacroMoveG ){
		no_move_message() ;
		break ;
	    }
	    TWmessage( "Pick or enter bottom left corner of core area:" ) ;
	    TWgetPt2( &x, &y ) ;
	    TWmessage( "Pick or enter upper right corner of core area:" ) ;
	    TWgetPt2( &x1, &y1 ) ;
	    if( x1 <= x ){
		TWmessage("ERROR:x coordinates invalid - ignored" ) ; 
		break ;
	    }
	    if( y1 <= y ){
		TWmessage("ERROR:y coordinates invalid - ignored" ) ; 
		break ;
	    }
	    save_for_do( UNDO ) ;
	    set_core( x, x1, y, y1 ) ;
	    /* now we need to redo all the work we have done before */ 
	    start_tileG = NULL;
	    recalculate(TRUE) ; /* free data */
	    remakerows() ;
	    draw_the_data() ;
	    get_core( &x, &y, &x1, &y1, FALSE ) ;
	    sprintf( YmsgG, "Core is now llx:(%d,%d) urx:(%d,%d)", 
		x, y, x1, y1 ) ;
	    TWmessage( YmsgG ) ;
	    TWcheckExposure() ;
	    save_for_do( REDO ) ;
	    break ;
	case MOVE_MACRO:
	    if( noMacroMoveG ){
		no_move_message() ;
		break ;
	    }
	    if(!(selectMacroS = pick_macro(NULL) )){
		break ;
	    }
	    save_for_do( UNDO ) ;
	    TWmessage( "Pick or enter a reference point relative to cell center:" ) ;
	    if( TWgetPt2( &x, &y ) ){
		/* we know from keyboard */
		TWmessage( "Enter new position x, y<cr>:" ) ;
		TWgetPt( &x1, &y1 ) ;
		update_vertices( selectMacroS, x1 - x, y1 - y ) ;
	    } else {
		/* from mouse */
		TWmessage( "Pick new position with mouse" ) ;
		/* get old position of cell */
		get_global_pos( selectMacroS, &x1, &y1, &x2, &y2 ) ;
		TWmoveRect( &x1, &y1, &x2, &y2, x, y ) ;
		/* calculate new position for cell */
		update_vertices( selectMacroS, (x1+x2)/2,(y1+y2)/2 );
	    }
	    update_macro() ;
	    break ;
	case NUMROWS:
	    if( noMacroMoveG ){
		no_move_message() ;
		break ;
	    }
	    if( num_macrosG != 0 ){
		TWmessage( "Number of rows may not be set since macros exist") ;
		break ;
	    }
	    save_for_do( UNDO ) ;
	    do {
		if( reply = TWgetString("Enter the number of rows: ")){ 
		    num_rowsG = atoi(reply) ;
		    if( num_rowsG <= 0 ){
			TWmessage( "Number of row may not be <= 0" ) ;
			(void) sleep( (unsigned) 2) ;
		    }
		}
	    } while( num_rowsG <= 0 ) ;
	    save_memory = memoryG ;
	    memoryG = FALSE ;
	    calculate_numrows() ;
	    setGraphicWindow() ;
	    draw_the_data() ;
	    TWcheckExposure() ;
	    save_for_do( REDO ) ;
	    memoryG = save_memory ;
	    break ;
	case REDO:
	    sprintf( filename, "%s.redo", cktNameG ) ;
	    fp = TWOPEN( filename, "r", NOABORT ) ;
	    if(!fp){
		outm(ERRMSG,"save_state", "could not redo command" ) ;
		break ;
	    }
	    if(!(read_vertices(fp,FALSE))){
		break ;
	    }
	    start_tileG = NULL;
	    recalculate(FALSE) ; /* don't free data */
	    if( restore_state(fp) ){
		remakerows() ;
		draw_the_data() ;
		TWcheckExposure() ;
	    }
	    TWCLOSE( fp ) ;
	    break ;
	case RESTORE_STATE:
	    do {
		reply = TWgetString("Enter restore file name: ") ;
	    } while(!(reply)) ;
	    sprintf( filename, "%s.gsav", reply ) ;
	    fp = TWOPEN( filename, "r", NOABORT ) ;
	    if(!fp){
		sprintf( YmsgG, "ERROR:could not open file:%s",
		    filename ) ;
		outm(ERRMSG,"save_state", YmsgG ) ;
		break ;
	    }
	    if(!(read_vertices(fp,FALSE))){
		break ;
	    }
	    start_tileG = NULL;
	    recalculate(FALSE) ; /* don't free data */
	    if( restore_state(fp) ){
		remakerows() ;
		setGraphicWindow() ;
		draw_the_data() ;
		TWcheckExposure() ;
	    }
	    TWCLOSE( fp ) ;
	    break ;
	case SAVE_STATE:
	    do {
		reply = TWgetString("Enter file name for save file: ") ;
	    } while(!(reply)) ;
	    sprintf( filename, "%s.gsav", reply ) ;
	    fp = TWOPEN( filename, "w", NOABORT ) ;
	    if(!fp){
		sprintf( YmsgG, "ERROR:could not open file:%s",
		    filename ) ;
		outm(ERRMSG,"save_state", YmsgG ) ;
		break ;
	    }
	    save_state(fp) ;
	    TWCLOSE( fp ) ;
	    sprintf( YmsgG, "Genrows saved state:%s", reply ) ;
	    TWmessage( YmsgG ) ;
	    break ;
	case UNDO:
	    sprintf( filename, "%s.undo", cktNameG ) ;
	    fp = TWOPEN( filename, "r", NOABORT ) ;
	    if(!fp){
		outm(ERRMSG,"save_state", "could not undo command" ) ;
		break ;
	    }
	    if(!(read_vertices(fp,FALSE))){
		break ;
	    }
	    start_tileG = NULL;
	    recalculate(FALSE) ; /* don't free data */
	    if( restore_state(fp) ){
		remakerows() ;
		draw_the_data() ;
		TWcheckExposure() ;
	    }
	    TWCLOSE( fp ) ;
	    break ;
	case DIVIDE_TILE_LEFT_RIGHT:
	    selected_tileS = pick_tile(NULL) ;
	    if( selected_tileS == 0 ){
		break ;
	    }
	    save_for_do( UNDO ) ;
	    draw_tile( selected_tileS ) ; /* show user which tile selected */
	    TWmessage( "Now pick the division point for the tile");
	    do {
		ok = FALSE ;
		TWgetPt2( &x, &y ) ;
		if( x<=selected_tileS->llx || x>=selected_tileS->urx ){
		    TWmessage("Division line outside tile - pick again" ) ;
		} else {
		    ok = TRUE ;
		}
		
	    } while(!(ok)) ;
	    divide_tilelr( selected_tileS , x ) ; 
	    renumber_tiles() ;
	    remakerows() ;
	    selected_tileS = NULL;
	    draw_the_data() ;
	    TWcheckExposure() ;
	    save_for_do( REDO ) ;
	    break ;
	case DIVIDE_TILE_UP_DOWN:
	    selected_tileS = pick_tile(NULL) ;
	    if( selected_tileS == 0 ){
		break ;
	    }
	    save_for_do( UNDO ) ;
	    draw_tile( selected_tileS ) ; /* show user which tile selected */
	    TWmessage( "Now pick the division point for the tile");
	    do {
		ok = FALSE ;
		TWgetPt2( &x, &y ) ;
		if( y<=selected_tileS->lly || y>=selected_tileS->ury ){
		    TWmessage("Division line outside tile pick again" ) ;
		} else {
		    ok = TRUE ;
		}
		
	    } while(!(ok)) ;
	    divide_tile( selected_tileS , y ) ; 
	    renumber_tiles() ;
	    remakerows() ;
	    selected_tileS = NULL;
	    draw_the_data() ;
	    TWcheckExposure() ;
	    save_for_do( REDO ) ;
	    break ;
	case LIMIT_MERGES:
	    limitMergeG = TRUE ;
	    TWmessage( "Merges will be limited to immediate neighbors");
	    break ;
	case UNLIMIT_MERGES:
	    limitMergeG = FALSE ;
	    TWmessage( "Maximum possible merges from a tile will be performed" );
	    break ;
	case MERGE_DOWNWARD:
	    selected_tileS = pick_tile(NULL) ;
	    if( selected_tileS == 0 ){
		break ;
	    }
	    save_for_do( UNDO ) ;
	    draw_tile( selected_tileS ) ; /* show user which tile selected */
	    TWflushFrame() ;
	    (void) sleep( (unsigned) 2) ;
	    merge_downward( selected_tileS ) ;
	    renumber_tiles() ;
	    selected_tileS = NULL ;
	    remakerows() ;
	    draw_the_data() ;
	    TWcheckExposure() ;
	    save_for_do( REDO ) ;
	    break ;
	case MERGE_LEFT:
	    selected_tileS = pick_tile(NULL) ;
	    if( selected_tileS == 0 ){
		break ;
	    }
	    save_for_do( UNDO ) ;
	    draw_tile( selected_tileS ) ; /* show user which tile selected */
	    TWflushFrame() ;
	    (void) sleep( (unsigned) 2) ;
	    merge_left( selected_tileS ) ;
	    renumber_tiles() ;
	    selected_tileS = NULL ;
	    remakerows() ;
	    draw_the_data() ;
	    TWcheckExposure() ;
	    save_for_do( REDO ) ;
	    break ;
	case MERGE_RIGHT:
	    selected_tileS = pick_tile(NULL) ;
	    if( selected_tileS == 0 ){
		break ;
	    }
	    save_for_do( UNDO ) ;
	    draw_tile( selected_tileS ) ; /* show user which tile selected */
	    TWflushFrame() ;
	    (void) sleep( (unsigned) 2) ;
	    merge_right( selected_tileS ) ;
	    renumber_tiles() ;
	    selected_tileS = NULL ;
	    remakerows() ;
	    draw_the_data() ;
	    TWcheckExposure() ;
	    save_for_do( REDO ) ;
	    break ;
	case MERGE_UPWARD:
	    selected_tileS = pick_tile(NULL) ;
	    if( selected_tileS == 0 ){
		break ;
	    }
	    save_for_do( UNDO ) ;
	    draw_tile( selected_tileS ) ; /* show user which tile selected */
	    TWflushFrame() ;
	    (void) sleep( (unsigned) 2) ;
	    merge_upward( selected_tileS ) ;
	    renumber_tiles() ;
	    selected_tileS = NULL ;
	    remakerows() ;
	    draw_the_data() ;
	    TWcheckExposure() ;
	    save_for_do( REDO ) ;
	    break ;
	case RESET_TILES:
	    save_for_do( UNDO ) ;
	    save_memory = memoryG ;
	    memoryG = FALSE ;
	    /* redo all the work we have done before */ 
	    recalculate(TRUE) ; /* free data */
	    remakerows() ;
	    draw_the_data() ;
	    TWcheckExposure() ;
	    save_for_do( REDO ) ;
	    memoryG = save_memory ;
	    break ;
	case DRAW_LABELS:
	    drawLabelS = TRUE ;
	    TWforceRedraw() ;
	    break ;
	case IGNORE_LABELS:
	    drawLabelS = FALSE ;
	    TWforceRedraw() ;
	    break ;
	case DRAW_ORIENT:
	    drawOrientS = TRUE ;
	    TWforceRedraw() ;
	    break ;
	case IGNORE_ORIENT:
	    drawOrientS = FALSE ;
	    TWforceRedraw() ;
	    break ;
	case DRAW_MACROS:
	    drawMacroS = TRUE ;
	    TWforceRedraw() ;
	    break ;
	case IGNORE_MACROS:
	    drawMacroS = FALSE ;
	    TWforceRedraw() ;
	    break ;
	case DRAW_ROWS:
	    drawRowS = TRUE ;
	    TWforceRedraw() ;
	    break ;
	case IGNORE_ROWS:
	    drawRowS = FALSE ;
	    TWforceRedraw() ;
	    break ;
	case DRAW_TILES:
	    drawTileS = TRUE ;
	    TWforceRedraw() ;
	    break ;
	case IGNORE_TILES:
	    drawTileS = FALSE ;
	    TWforceRedraw() ;
	    break ;
	case FEED_PERCENTAGE:
	    save_for_do( UNDO ) ;
	    ok = FALSE ;
	    while(!(ok)){
		if( reply = TWgetString( "Enter feed ratio in percentage [0-100]: " )){
		    tempf = atof( reply ) ;
		    if( tempf >= 0.0 ){
			ok = TRUE ;
		    }
		}
	    }
	    set_feed_length( tempf ) ;
	    remakerows() ;
	    draw_the_data() ;
	    TWcheckExposure() ;
	    save_for_do( REDO ) ;
	    break ;
	case MIN_ROW_LENGTH:
	    save_for_do( UNDO ) ;
	    ok = FALSE ;
	    while(!(ok)){
		if( reply = TWgetString( "Enter minimum valid row length: " )){ 
		    temp = atoi( reply ) ;
		    if( temp >= 0 ){
			ok = TRUE ;
			set_minimum_length( temp ) ;
		    } else {
			outm( ERRMSG, "reset_min_row_length", 
			    "Invalid minimum row length. Must be >= 0" ) ;
		    }
		}
	    }
	    remakerows() ;
	    draw_the_data() ;
	    TWcheckExposure() ;
	    save_for_do( REDO ) ;
	    break ;
	case ROW_SEPARATION:
	    save_for_do( UNDO ) ;
	    ok = FALSE ;
	    while(!(ok)){
		if( reply = TWgetString( "Enter row separation [1.0 nominal]:" )){
		    tempf = atof( reply ) ;
		    if( tempf > 0.0 ){
			ok = TRUE ;
			set_row_separation( tempf, 0.0 ) ;
		    } else {
			outm( ERRMSG, "reset_row_separation", 
			    "Invalid row separation. Must be > 0.0" ) ;
		    }
		}
	    }
	    remakerows() ;
	    draw_the_data() ;
	    TWcheckExposure() ;
	    save_for_do( REDO ) ;
	    break ;
	case SET_SPACING:
	    save_for_do( UNDO ) ;
	    ok = FALSE ;
	    while(!(ok)){
		if( reply = TWgetString( "Enter spacing between row and tile edge: " )){ 
		    temp = atoi( reply ) ;
		    if( temp >= 0 ){
			ok = TRUE ;
			spacingG = temp ;
			set_spacing() ;
		    } else {
			outm( ERRMSG, "set_spacing", "Invalid space. Must be >= 0" ) ;
		    }
		}
	    }
	    remakerows() ;
	    draw_the_data() ;
	    TWcheckExposure() ;
	    save_for_do( REDO ) ;
	    break ;
	case MEMORY_ON:
	    memoryG = TRUE ;
	    TWmessage("Genrows will try to remember the last state" ) ;
	    break ;
	case MEMORY_OFF:
	    memoryG = FALSE ;
	    TWmessage("Genrow's memory has been turned off" ) ;
	    break ;
	} /*********************** end graphics SWITCH *****************/
	if( auto_drawS && TWcheckExposure() ){
	    draw_the_data() ;
	}

    } 

    if( shortRowG ){
	TWmessage( "Now forcing removal of short row") ;
	shortRowG = FALSE ;
	remakerows() ;
	draw_the_data() ;
	if (last_chanceG) (VOID)last_chance() ;

    }

} /* end process_graphics */

static last_chance()
{
    INT i ; /* counter */

    (void) sleep( (unsigned) 1 ) ;
    TWmessage( "If you wish to modify the rows, you have 10 secs. to click on top menu") ;
    for( i = 1; i <= 10; i++ ){
	(void) sleep( (unsigned) 1 ) ;
	if( TWinterupt() ){
	    TWmessage( "Please reconfigure the rows" ) ;
	    (void) sleep( (unsigned) 1 ) ;
	    process_graphics() ;
	    break ;
	}
    }
} /* end last_chance */

static no_move_message()
{
    TWmessage("Macro moves/core changes not allowed in partitioning");
}

static save_for_do( save )
INT save ;
{
    char filename[LRECL] ;
    FILE *fp ;

    if( save == UNDO ){
	sprintf( filename, "%s.undo", cktNameG ) ;
    } else if( save == REDO ){
	sprintf( filename, "%s.redo", cktNameG ) ;
    }
    fp = TWOPEN( filename, "w", ABORT ) ;
    save_state(fp) ;
    TWCLOSE( fp ) ;
} /* end undo */

static update_macro()
{
    char filename[LRECL] ;
    FILE *fp ;

    /* this is the easiest way to redo entire algorithm from scratch */
    sprintf( filename, "%s.tmp", cktNameG ) ;
    fp = TWOPEN( filename, "w", ABORT ) ;
    save_state(fp) ;
    TWCLOSE( fp ) ;

    fp = TWOPEN( filename, "r", ABORT ) ;
    if(!(read_vertices(fp,FALSE))){
	return ;
    }
    start_tileG = NULL;
    selectMacroS = 0 ;
    recalculate(FALSE) ; /* don't free data */
    remakerows() ;
    draw_the_data() ;
    TWcheckExposure() ;
    save_for_do( REDO ) ;
} /* update_macro */

/* dumps the data to a file for future study */
static graphics_dump() 
{
    /* now change mode to dump to file */
    TWsetMode(1) ;
    TWsetFrame(0) ;
    /* dump the data to a file now instead of screen */
    draw_the_data() ;
    /* restore the state to previous condition and set draw to screen */
    TWsetMode(0) ;
}


/* find the macro in question */
static INT pick_macro( twmsg )
char *twmsg ;
{

    INT i ;
    INT match_count ;        /* keep track of all cells that match */
    INT x, y ;               /* coordinates picked by user */
    INT cell ;               /* selected cell */
    INT l, r, b, t ;         /* cell sides */

    if( twmsg ){
	TWmessage( twmsg ) ;
    } else {
	TWmessage("Pick cell by clicking any mouse button at center of cell");
    }
    TWgetPt( &x, &y ) ;
    /* look thru all cells O(numcells) algorithm */
    match_count = 0 ;
    for( i = 1 ; i <= num_macrosG; i++ ){
	get_global_pos( i, &l, &b, &r, &t ) ;
	/* see if cell boundary contains this point */
	if( x >= l && x <= r ){
	    if( y >= b && y <= t ){
		selectMacroS = i ;
		match_count++ ;
	    }
	}
    } /* end loop */

    if( match_count == 0 ){
	TWmessage( "No cell selected" ) ;
	if( selectMacroS ){  /* user didn't like any options */
	    selectMacroS = 0 ;
	    /* draw the data with highlight off
		draw_the_data() ;
		TWcheckExposure() ;
	    */
	}
	return( 0 ) ;
    } else if( match_count == 1 ){
	/* draw the data with highlight on */
	draw_macro( selectMacroS, TWVIOLET ) ;
	TWcheckExposure() ;
	sprintf( YmsgG, "Selected macro:%d", selectMacroS ) ;
	TWmessage( YmsgG ) ;
	return( selectMacroS ) ;
    } else {
	/* more than one match */
	TWmessage( "More than one match.  Choose correct cell" ) ;  
	(void) sleep( (unsigned) 2 ) ;
	while( TRUE ){
	    for( i = 1 ; i <= num_macrosG; i++ ){
		get_global_pos( i, &l, &b, &r, &t ) ;
		/* see if cell boundary contains this point */
		if( x >= l && x <= r ){
		    if( y >= b && y <= t ){
			selectMacroS = i ;
			draw_the_data() ;
			TWcheckExposure() ;

			/* give directions */
			sprintf( YmsgG, "Selected cell:%d",
			    selectMacroS ) ;
			TWmessage( YmsgG ) ;
			(void) sleep( (unsigned) 2 ) ;
			sprintf( YmsgG,"%s","If correct, enter <cr>. Otherwise ") ;
			strcat( YmsgG,
			    "enter n<cr> for next cell: ") ;
			/* look for empty string - means we are satisfied */
			if(!(TWgetString(YmsgG))){
			    return( selectMacroS ) ;
			}
		    }
		}
	    } /* end loop */
	}
    }

} /* end pick_macro */

static TILE_BOX *pick_tile( pmsg )
char *pmsg ;
{
    INT x, y ;            /* the user's pick points */
    TILE_BOX *tile ;      /* the current tile */

    if( pmsg ){
	TWmessage( pmsg ) ;
    } else {
	TWmessage( "Select tile to edit by using mouse") ;
    }
    TWgetPt( &x, &y ) ;
    for( tile=tile_listG;tile;tile=tile->next ){
	if( tile->llx <= x && x <= tile->urx &&
	    tile->lly <= y && y <= tile->ury ){
	    /* we found a tile return */
	    return(tile) ;
	}
    }
    /* if we get here we didn't find a tile */
    TWmessage( "No tile selected") ;
    return( NULL ) ;

} /* end pick_tile */

static ROW_BOX *pick_row()
{
    INT x, y ;            /* the user's pick points */
    ROW_BOX  *rowptr  ;     /* current row being output */
    ROW_BOX  *segment ;     /* current segment being output */
    ROW_BOX  *get_rowptr(); /* get data structure */

    TWmessage( "Select row to edit by using mouse") ;
    TWgetPt( &x, &y ) ;
    for( rowptr=get_rowptr();rowptr;rowptr=rowptr->next_row ){

	if( rowptr->next_segment ){
	    /* means we have more that one segment to row */
	    for( segment=rowptr;segment;segment=segment->next_segment){
		if( segment->llx <= x && x <= segment->urx &&
		    segment->lly <= y && y <= segment->ury ){
		    /* we found a tile return */
		    return(segment) ;
		}

	    } /* end for loop */

	} else {
	    if( rowptr->llx <= x && x <= rowptr->urx &&
		rowptr->lly <= y && y <= rowptr->ury ){
		/* we found a tile return */
		return(rowptr) ;
	    }
	}
    } /* end for loop */
    /* if we get here we didn't find a row */
    TWmessage( "No row selected") ;
    return( NULL ) ;
} /* end pick row */


#define TILEF        3
#define LEFTF        4
#define BOTF         LEFTF+1
#define RITEF        LEFTF+2
#define TOPF         LEFTF+3
#define LEGALF       8
#define LEGALCASE    LEGALF+1
#define ILLEGALCASE  LEGALF+2
#define ROWF         12
#define MAXROWF      14
#define NUMROWF      16
#define MINF         21
#define STARTF       23
#define MAXF         25
#define SEPF         27
#define CLASSF       29
#define FORCEF       17
#define FORCECASE    FORCEF+1
#define NOFORCECASE  FORCEF+2
#define MIRRORF      30
#define MIRRORCASE   MIRRORF+1 
#define NOMIRRORCASE MIRRORF+2 
#define FWIDTH       15   /* field width */

static INT get_row_height( answer )
TWDRETURNPTR answer ;  /* return from user */
{
    INT height ;

    height = atoi( answer[ROWF].string ) ;
    if( height <= 0 ){
	outm( ERRMSG, "edit_tile",
	"Invalid row height.  Must be greater than zero" ) ;
	return( 1 ) ; ;
    }
    return( height ) ;
} /* end get_row_height */

static INT get_row_sep( answer )
TWDRETURNPTR answer ;  /* return from user */
{
    INT sep ;

    sep = atoi( answer[SEPF].string ) ;
    if( sep <= 0 ){
	outm( ERRMSG, "edit_tile",
	"Invalid row separation.  Must be greater than zero" ) ;
	return(1) ;
    }
    if( sep > selected_tileS->ury - selected_tileS->lly ){
	outm( ERRMSG, "edit_tile",
	"Invalid row separation.  Must be less than tile y dimension" ) ;
	return(1) ;
    }
    return( sep ) ;
} /* end get_row_sep */

static INT get_maxrows( answer )
TWDRETURNPTR answer ;  /* return from user */
{
    INT maxrows ;

    /* means the user change the field */
    maxrows = atoi( answer[MAXROWF].string ) ;
    if( maxrows < 0 ){ 
	outm( ERRMSG, "edit_tile",
	"Invalid number of rows.  Must be non-negative" ) ;
	return( 0 ) ;
    }
    return( maxrows ) ;
} /* end get_maxrows */

static INT update_tile_data( answer, field )
TWDRETURNPTR answer ;  /* return from user */
INT field ;
{
    INT sep ;
    INT rows ;
    INT maxrows ;
    INT height ;
    INT row_height ;

    switch( field ){
    case FORCECASE:
	break ;
    case NOFORCECASE:
	sprintf( answer[STARTF].string, "%d", selected_tileS->llx + spacingG ) ;
	sprintf( answer[MAXF].string, "%d", selected_tileS->urx - spacingG ) ;
	answer[STARTF].bool = TRUE ;
	answer[MAXF].bool = TRUE ;
	break ;
    case ROWF:
	row_height = get_row_height( answer ) ;
	sep = get_row_sep( answer ) ;
	height = selected_tileS->ury - selected_tileS->lly ;
	maxrows = ( height ) / (row_height + sep ) ;
	sprintf( answer[MAXROWF].string, "%d", maxrows ) ;
	rows = atoi( answer[NUMROWF].string ) ;
	if( rows < 0 ){ 
	    outm( ERRMSG, "edit_tile",
	    "Invalid number of rows.  Must be non-negative" ) ;
	    return ;
	}
	if( rows > maxrows ){
	    sprintf( answer[NUMROWF].string, "%d", maxrows ) ;
	}
	break ;
    case MAXROWF:
	/* means the user change the field */
	maxrows = get_maxrows( answer ) ;
	row_height = get_row_height( answer ) ;
	/* now calculate the channel separation for this tile */
	height = selected_tileS->ury - selected_tileS->lly ;
	sep =  (height - maxrows * row_height) / (maxrows) ;
	if( sep <= 0 ){
	    /* calculate the maximum amount of rows if sep = 1 */
	    sep = 1 ;
	    maxrows = ( height ) / (row_height + sep ) ;
	    outm( WARNMSG, "edit_tile",
	    "Exceeded maximum number of rows.  Set to maximum." ) ;
	}
	sprintf( answer[NUMROWF].string, "%d", maxrows ) ;
	sprintf( answer[SEPF].string, "%d", sep ) ;
	break ;
    case NUMROWF:
	maxrows = get_maxrows( answer ) ;
	/* means the user changed the field */
	rows = atoi( answer[NUMROWF].string ) ;
	if( rows < 0 ){ 
	    outm( ERRMSG, "edit_tile",
	    "Invalid number of rows.  Must be non-negative" ) ;
	    return ;
	}
	if( rows > maxrows ){
	    outm( WARNMSG, "edit_tile",
	    "Exceeded maximum number of rows.  Set to maximum." ) ;
	    sprintf( answer[NUMROWF].string, "%d", maxrows ) ;
	}
	break ;
    case MINF:
	break ;
    case SEPF:
	sep = get_row_sep( answer ) ;
	row_height = get_row_height( answer ) ;
	height = selected_tileS->ury - selected_tileS->lly ;
	rows = ( height ) / (row_height + sep ) ;
	sprintf( answer[MAXROWF].string, "%d", rows ) ;
	if( rows > selected_tileS->numrows ){
	    sprintf( answer[NUMROWF].string, "%d", rows ) ;
	}
	break ;
    } /* end switch */
}

static BOOL edit_tiles( tile )
TILE_BOX *tile ;
 {
    INT  temp ;            /* temporary answer from user */
    INT  sep ;             /* channel separation */
    INT  rows ;            /* number of rows */
    INT  height ;          /* tile height */
    INT  max_rows ;        /* maximum number of rows */
    char tileName[FWIDTH]; /* a scratch buffer for tile name */
    char left[FWIDTH];     /* a scratch buffer for left tile side */
    char right[FWIDTH];    /* a scratch buffer for right tile side */
    char class[FWIDTH];    /* a scratch buffer for the class */
    char top[FWIDTH];      /* a scratch buffer for top tile side */
    char bottom[FWIDTH];   /* a scratch buffer for bottom tile side */
    char numrows[FWIDTH];  /* a scratch buffer for number of rows */
    char maxrows[FWIDTH];  /* a scratch buffer for number of rows */
    char rowHeight[FWIDTH];/* a scratch buffer for row height */
    char minlength[FWIDTH];/* a scratch buffer for minimum length of row */
    char maxlength[FWIDTH];/* a scratch buffer for maximum length of row */
    char separation[FWIDTH];/* a scratch buffer for row separation */
    char startrow[FWIDTH]; /* a scratch buffer for row separation */
    BOOL old_force ;       /* keep track of change */
    TWDRETURNPTR answer ;  /* return from user */

    TWmessage( "Edit Tile" ) ;

#ifdef DEVELOPDIALOG
    dialogS = TWread_dialog( "genrows.dialog" ) ;
    if( !(dialogS) ){
	return ; /* avoid crashes */
    }
#endif


    /* now initialize the fields to user data */
    sprintf( tileName,  "Tile  : %d", tile->name ) ;
    dialogS[TILEF].string = tileName ;
    dialogS[TILEF].len = strlen( tileName ) ;
    sprintf( left,  "Left  : %d", tile->llx ) ;
    dialogS[LEFTF].string = left ;
    dialogS[LEFTF].len = strlen( left ) ;
    sprintf( bottom,"Bottom: %d", tile->lly ) ;
    dialogS[BOTF].string = bottom ;
    dialogS[BOTF].len = strlen( bottom ) ;
    sprintf( right, "Right : %d", tile->urx ) ;
    dialogS[RITEF].string = right ;
    dialogS[RITEF].len = strlen( right ) ;
    sprintf( top,   "Top   : %d", tile->ury ) ;
    dialogS[TOPF].string = top ;
    dialogS[TOPF].len = strlen( top ) ;
    /* set legality of tile */
    if( tile->illegal ){
	dialogS[LEGALF].group = ILLEGALCASE ;
    } else {
	dialogS[LEGALF].group = LEGALCASE ;
    }
    /* Now set the row height fields */
    sprintf( rowHeight,   "%d", tile->actual_row_height ) ;
    dialogS[ROWF].string = rowHeight ;
    /* Now set the number of rows */
    height = tile->ury - tile->lly ;
    sep = tile->channel_separation ;
    max_rows = ( height ) / (tile->actual_row_height + sep ) ;
    if( tile->numrows > max_rows ){
	max_rows = tile->numrows ;
    }
    sprintf( maxrows, "%d", max_rows ) ;
    dialogS[MAXROWF].string = maxrows ;
    sprintf( numrows,   "%d", tile->numrows ) ;
    dialogS[NUMROWF].string = numrows ;
    /* Now set the min length of row */
    sprintf( minlength,   "%d", tile->min_length ) ;
    dialogS[MINF].string = minlength ;
    /* Now set the start of the row */
    sprintf( startrow,   "%d", tile->llx + tile->row_start ) ;
    dialogS[STARTF].string = startrow ;
    /* Now set the max length of row */
    sprintf( maxlength,   "%d", 
	tile->llx + tile->row_start + tile->max_length ) ;
    dialogS[MAXF].string = maxlength ;
    /* Now set the chan separation of row */
    sprintf( separation,   "%d", tile->channel_separation ) ;
    dialogS[SEPF].string = separation ;
    sprintf( class,  "%d", tile->class ) ;
    dialogS[CLASSF].string = class ;
    if( tile->force ){
	dialogS[FORCEF].group = FORCECASE ;
    } else {
	dialogS[FORCEF].group = NOFORCECASE ;
    }
    old_force = tile->force ;
    /* set mirror option of tile */
    if( tile->mirror ){
	dialogS[MIRRORF].group = MIRRORCASE ;
    } else {
	dialogS[MIRRORF].group = NOMIRRORCASE ;
    }

    /* initialization complete */

    if( answer = TWdialog( dialogS, "genrows", update_tile_data ) ){
	/* must be the number of the case field */
	if( answer[LEGALCASE].bool ){
	    /* the tile is legal */
	    tile->illegal = FALSE ;
	} else if( answer[ILLEGALCASE].bool ){
	    tile->illegal = TRUE ;
	}
	if( answer[MIRRORCASE].bool ){
	    tile->mirror = TRUE ;
	} else if( answer[NOMIRRORCASE].bool ){
	    tile->mirror = FALSE ;
	}
	if( answer[FORCECASE].bool ){
	    tile->force = TRUE ;
	} else if( answer[NOFORCECASE].bool ){
	    tile->force = FALSE ;
	    if( old_force != tile->force ){
		/* only do this on a change */
		tile->row_start = spacingG ;
		tile->max_length = tile->urx - tile->llx - 2 * spacingG ;
	    }
	}
	if( answer[ROWF].bool ){
	    /* means the user change the field */
	    temp = get_row_height( answer ) ;
	    if( temp <= 0 ){
		return ;
	    }
	    tile->actual_row_height = temp ;
	}
	if( answer[MAXROWF].bool ){
	    /* means the user change the field */
	    rows = atoi( answer[MAXROWF].string ) ;
	    if( rows < 0 ){ 
		outm( ERRMSG, "edit_tile",
		"Invalid number of rows.  Must be non-negative" ) ;
		return ;
	    }
	    /* now calculate the channel separation for this tile */
	    height = tile->ury - tile->lly ;
	    sep =  (height - rows * tile->actual_row_height) / (rows) ;
	    if( sep <= 0 ){
		/* calculate the maximum amount of rows if sep = 1 */
		sep = 1 ;
		rows = ( height ) / (tile->actual_row_height + sep ) ;
		outm( WARNMSG, "edit_tile",
		"Exceeded maximum number of rows.  Set to maximum." ) ;
	    }
	    tile->numrows = rows ;
	    tile->channel_separation = sep ;
	}
	if( answer[NUMROWF].bool ){
	    /* means the user change the field */
	    rows = atoi( answer[NUMROWF].string ) ;
	    if( rows < 0 ){ 
		outm( ERRMSG, "edit_tile",
		"Invalid number of rows.  Must be non-negative" ) ;
		return ;
	    }
	    /* now calculate the channel separation for this tile */
	    height = tile->ury - tile->lly ;
	    sep = tile->channel_separation ;
	    max_rows = ( height ) / (tile->actual_row_height + sep ) ;

	    if( rows <= max_rows ){
		tile->numrows = rows ;
	    } else {
		outm( WARNMSG, "edit_tile",
		"Exceeded maximum number of rows.  Set to maximum." ) ;
		tile->numrows = max_rows ;
	    }
	}
	if( answer[MINF].bool ){
	    /* means the user change the field */
	    temp = atoi( answer[MINF].string ) ;
	    if( temp <= 0 ){
		outm( ERRMSG, "edit_tile",
		"Invalid minimum length.  Must be greater than zero" ) ;
		return ;
	    }
	    tile->min_length = temp ;
	}
	if( answer[MAXF].bool ){
	    /* means the user change the field */
	    temp = atoi( answer[MAXF].string ) ;
	    if( temp < tile->llx  ){
		outm( ERRMSG, "edit_tile",
		"Invalid end of row.  Must be greater than tile left" );
		return ;
	    }
	    if( temp > tile->urx ){
		outm( ERRMSG, "edit_tile",
		"Invalid end of row.  Must be less than tile right" ) ;
		return ;
	    }
	    tile->max_length = temp - tile->llx - tile->row_start ;
	}
	if( answer[STARTF].bool ){
	    /* means the user change the field */
	    temp = atoi( answer[STARTF].string ) ;
	    if( temp < tile->llx ){
		outm( ERRMSG, "edit_tile",
		"Invalid start of row.  Must be greater than tile left") ;
		return ;
	    }
	    if( temp > tile->urx ){
		outm( ERRMSG, "edit_tile",
		"Invalid start of row.  Row must start before end of tile" ) ;
		return ;
	    }
	    tile->row_start = temp - tile->llx ;
	    /* now modify the width of the tile accordingly */
	    if(  tile->llx + tile->row_start + tile->max_length > 
						    tile->urx - 2*spacingG ){
		tile->max_length = 
		    tile->urx - tile->llx - tile->row_start - 2*spacingG ;
		outm( MSG, "edit_tile",
		"Note:resetting maximum length of row." ) ;
	    }
	}
	if( answer[SEPF].bool ){
	    /* means the user change the field */
	    tile->channel_separation = get_row_sep( answer ) ;
	}
	if( answer[CLASSF].bool ){
	    /* means the user change the field */
	    temp = atoi( answer[CLASSF].string ) ;
	    if( temp <= 0 ){
		outm( ERRMSG, "edit_tile",
		    "ERROR:Invalid class.  Must be greater than zero" ) ;
		return ;
	    }
	    tile->class = temp ;
	}
	return( TRUE ) ;
    } else {
	return( FALSE ) ;
    }
} /* end edit_tiles */


edit_row( rowptr )
ROW_BOX *rowptr ;
{

#define LEFT_F         3
#define BOT_F          LEFT_F+1
#define RITE_F         LEFT_F+2
#define TOP_F          LEFT_F+3
#define CLASS_F        8 
#define MIRROR_F       9
#define MIRRORCASE_F   MIRROR_F+1 
#define NOMIRRORCASE_F MIRROR_F+2 


    char left[FWIDTH];     /* a scratch buffer for left tile side */
    char right[FWIDTH];    /* a scratch buffer for right tile side */
    char top[FWIDTH];      /* a scratch buffer for top tile side */
    char bottom[FWIDTH];   /* a scratch buffer for bottom tile side */
    char class[FWIDTH];    /* a scratch buffer for the class */
    INT  temp ;            /* calculate the class */
    TWDRETURNPTR answer ;  /* return from user */

    TWmessage( "Edit Row" ) ;

#ifdef DEVELOPDIALOG
    row_dialogS = TWread_dialog( "row.dialog" ) ;
    if( !(row_dialogS) ){
	return ; /* avoid crashes */
    }
#endif
    /* now initialize the fields to user data */
    sprintf( left,  "Left  : %d", rowptr->llx ) ;
    row_dialogS[LEFT_F].string = left ;
    row_dialogS[LEFT_F].len = strlen(left) ;
    sprintf( bottom,"Bottom: %d", rowptr->lly ) ;
    row_dialogS[BOT_F].string = bottom ;
    row_dialogS[BOT_F].len = strlen(bottom) ;
    sprintf( right, "Right : %d", rowptr->urx ) ;
    row_dialogS[RITE_F].string = right ;
    row_dialogS[RITE_F].len = strlen(right) ;
    sprintf( top,   "Top   : %d", rowptr->ury ) ;
    row_dialogS[TOP_F].string = top ;
    row_dialogS[TOP_F].len = strlen(top) ;
    /* Now set the class field */
    if( rowptr->class ){
	sprintf( class, "%d", rowptr->class ) ;
    } else {
	sprintf( class, "1" ) ;
    }
    row_dialogS[CLASS_F].string = class ;
    /* set mirror option of tile */
    if( rowptr->mirror ){
	row_dialogS[MIRROR_F].group = MIRRORCASE_F ;
    } else {
	row_dialogS[MIRROR_F].group = NOMIRRORCASE_F ;
    }

    /* Now call the dialog box */
    if( answer = TWdialog( row_dialogS, "row", NULL ) ){
	if( answer[CLASS_F].bool ){
	    /* means the user change the field */
	    temp = atoi( answer[CLASS_F].string ) ;
	    if( temp <= 0 ){
		TWmessage( "ERROR:Invalid class.  Must be greater than zero" ) ;
		return ;
	    }
	    rowptr->class = temp ;
	}
	if( answer[MIRRORCASE_F].bool ){
	    rowptr->mirror = TRUE ;
	} else if( answer[NOMIRRORCASE_F].bool ){
	    rowptr->mirror = FALSE ;
	}
    }
} /* end edit_row */

/* the MACRO DIALOG definitions */
#define XPOSF        6
#define YPOSF        8
#define DELTAXF      10
#define DELTAYF      12
#define ORIENTF      13
#define ORIENTBASE   14

static INT update_macro_data( answer, field )
TWDRETURNPTR answer ;  /* return from user */
INT field ;
{
    INT pos ;
    INT deltax, deltay ;
    INT orient ;

    deltax = 0 ;
    deltay = 0 ;
    switch( field ){
    case DELTAXF:
	deltax = atoi( answer[DELTAXF].string ) ;
	break ;
    case DELTAYF:
	deltay = atoi( answer[DELTAYF].string ) ;
	break ;
    case ORIENTBASE:
    case ORIENTBASE+1:
    case ORIENTBASE+2:
    case ORIENTBASE+3:
    case ORIENTBASE+4:
    case ORIENTBASE+5:
    case ORIENTBASE+6:
    case ORIENTBASE+7:
#ifdef TI
	orient = ice2tw(field-ORIENTBASE+1) ;
#else 
	orient = field - ORIENTBASE ;
#endif
	/* order is important here */
	rotate_vertices( macroArrayG[selectMacroS], orient ) ;
	macroArrayG[selectMacroS]->orient = orient ;
	TWforceRedraw() ;
	break ;
    } /* end switch */

    if( deltax == 0 && deltay == 0 ){
	return ; /* no work to do */
    }
    /* else update the cooridates positions */
    if( deltax != 0 ){
	pos = atoi( answer[XPOSF].string ) ;
	pos += deltax ;
	sprintf( answer[XPOSF].string, "%d", pos ) ;
	answer[XPOSF].bool = TRUE ;
	sprintf( answer[DELTAXF].string, "0" ) ;
    }
    if( deltay != 0 ){
	pos = atoi( answer[YPOSF].string ) ;
	pos += deltay ;
	sprintf( answer[YPOSF].string, "%d", pos ) ;
	answer[YPOSF].bool = TRUE ;
	sprintf( answer[DELTAYF].string, "0" ) ;
    }
} /* end update_macro_data */

static edit_macro( macro, xoff, yoff )
{
    TWDRETURNPTR answer ;  /* return from user */
    MACROPTR mptr ;        /* current macro information */
    char xpos[FWIDTH];     /* a scratch buffer for xpos */
    char ypos[FWIDTH];     /* a scratch buffer for ypos */
    char deltax[FWIDTH];   /* a scratch buffer for deltax */
    char deltay[FWIDTH];   /* a scratch buffer for deltay */
    INT x, y ;             /* new coordinates */
    INT pos ;              /* user's input */
    INT old_orient ;       /* put orientation back if necessary */

#ifdef DEVELOPDIALOG
    macro_dialogS = TWread_dialog( "macro.dialog" ) ;
    if( !(macro_dialogS) ){
	return ; /* avoid crashes */
    }
#endif
    mptr = macroArrayG[macro] ;
    /* now initialize the fields to user data */
    sprintf( xpos,  "%d", xoff ) ;
    macro_dialogS[XPOSF].string = xpos ;
    sprintf( ypos, "%d", yoff ) ;
    macro_dialogS[YPOSF].string = ypos ;
    sprintf( deltax,"0" ) ;
    macro_dialogS[DELTAXF].string = deltax ;
    sprintf( deltay,"0" ) ;
    macro_dialogS[DELTAYF].string = deltay ;

    /* set orientation */
#ifdef TI
    macro_dialogS[ORIENTF].group = tw2ice(mptr->orient) + ORIENTBASE - 1 ;
#else
    macro_dialogS[ORIENTF].group = mptr->orient + ORIENTBASE ;
#endif /* TI */

    old_orient = macroArrayG[selectMacroS]->orient ;

    if( answer = TWdialog( macro_dialogS, "macro", update_macro_data )){
	if( answer[XPOSF].bool ){
	    /* means the user change the field */
	    pos = atoi( answer[XPOSF].string ) ;
	    /* calculate the new center of the macro */
	    x = mptr->xcenter + pos - xoff ;
	} else {
	    x = mptr->xcenter ;
	}
	if( answer[YPOSF].bool ){
	    /* means the user change the field */
	    pos = atoi( answer[YPOSF].string ) ;
	    /* calculate the new center of the macro */
	    y = mptr->ycenter + pos - yoff ;
	} else {
	    y = mptr->ycenter ;
	}
	update_vertices( selectMacroS, x, y ) ;
    } else if( macroArrayG[selectMacroS]->orient != old_orient ){
	/* order is important here */
	rotate_vertices( macroArrayG[selectMacroS], old_orient ) ;
	macroArrayG[selectMacroS]->orient = old_orient ;
	TWforceRedraw() ;
    }

} /* end edit_macro */


get_global_pos( macro, l, b, r, t )
INT macro ; 
INT *l, *r, *b, *t ;
{

    MACROPTR mptr ;

    mptr = macroArrayG[macro] ;
    *l = mptr->left + mptr->xcenter ;
    *r = mptr->right + mptr->xcenter ;
    *b = mptr->bottom + mptr->ycenter ;
    *t = mptr->top + mptr->ycenter ;
} /* end get_global_pos */

static update_vertices( macro, newxcenter, newycenter )
INT macro, newxcenter, newycenter ;
{
    INT j ;
    INT deltaX ;
    INT deltaY ;
    MACROPTR mptr ;
    VERTEXPTR vptr ;

    mptr = macroArrayG[macro] ;
    deltaX = newxcenter - mptr->xcenter ;
    deltaY = newycenter - mptr->ycenter ;
    for( j = 0; j < mptr->num_vertices; j++ ){
	vptr = mptr->vertices[j] ;
	vptr->x += deltaX ;
	vptr->y += deltaY ;
    }
    mptr->xcenter = newxcenter ;
    mptr->ycenter = newycenter ;
} /* end update_vertices */

static rotate_vertices( mptr, orient )
MACROPTR mptr ;
INT orient ;
{
    INT j, p ;
    INT x, y ;
    INT l, b ;
    INT this_pt ;
    INT inverse ;
    INT old_orient ;
    INT lowestp ;
    INT limit ;
    INT nextpos, nextneg ;
    INT *temp_x ;
    INT *temp_y ;
    INT numpts ;
    VERTEXPTR vptr ;

    old_orient = mptr->orient ;
    numpts = mptr->num_vertices ;
    /* allocate the space for reoordering the points of the macro */
    temp_x = (INT *) Ysafe_malloc( numpts * sizeof(INT) ) ;
    temp_y = (INT *) Ysafe_malloc( numpts * sizeof(INT) ) ;
    /* first subtract off the center */
    for( j = 0; j < mptr->num_vertices; j++ ){
	vptr = mptr->vertices[j] ;
	vptr->x -= mptr->xcenter ;
	vptr->y -= mptr->ycenter ;
    }
    if( old_orient ){
	/* we need to perform the inverse first */
	/* if we have any orientation other than zero */
	inverse = Ytrans_inv_orient( old_orient ) ;
	for( j = 0; j < numpts; j++ ){
	    vptr = mptr->vertices[j] ;
	    REL_POST( inverse, 
		x, y,                               /* result */
		vptr->x, vptr->y,                   /* cell relative */
		0, 0 ) ;  /* cell center */
	    vptr->x = x ;
	    vptr->y = y ;
	}
    }
    for( j = 0; j < numpts; j++ ){
	vptr = mptr->vertices[j] ;
	REL_POST( orient, 
	    temp_x[j], temp_y[j],                 /* result */
	    vptr->x, vptr->y,                   /* cell relative */
	    mptr->xcenter, mptr->ycenter ) ;  /* cell center */
    }
    /* now we need to reorder the vertices */
    /* all the points have been rotated into their proper view and */
    /* stored in the vertices array.  Now find lowest of the left pts. */
    l = INT_MAX ;
    b = INT_MAX ;
    for( j = 0; j < numpts; j++ ){
	if( temp_x[j] <= l ){
	    if( temp_y[j] <= b ){
		l = temp_x[j] ;
		b = temp_y[j] ;
		lowestp = j ;
	    }
	}
    }
    /* now determine CW direction */
    nextpos = (lowestp + 1 ) % numpts; ;
    if( lowestp == 0 ){
	nextneg = numpts - 1 ;
    } else {
	nextneg = lowestp - 1 ;
    }
    j = 0 ;
    if( temp_x[nextpos] == l && temp_y[nextpos] > b ){
	/* clockwise is positive */
	limit = lowestp + numpts; 
	for( p = lowestp; p < limit; p++ ) {
	    this_pt = p % numpts ;
	    vptr = mptr->vertices[j++] ;
	    vptr->x = temp_x[this_pt] ; 
	    vptr->y = temp_y[this_pt] ; 
	}

    } else if( temp_x[nextneg] == l && temp_y[nextneg] > b ){
	/* clockwise is negative */
	limit = lowestp - numpts ;
	for( p = lowestp; p > limit; p-- ) {
	    if( p < 0 ){
		this_pt = numpts + p ;
	    } else {
		this_pt = p ;
	    }
	    vptr = mptr->vertices[j++] ;
	    vptr->x = temp_x[this_pt] ; 
	    vptr->y = temp_y[this_pt] ; 
	}
    } else {
	M( ERRMSG, "rotate_vertices",
	"couldn't find clockwise direction for boundary\n" ) ;
    }
    Ysafe_free( temp_x ) ;
    Ysafe_free( temp_y ) ;

} /* end rotate_vertices */

static find_nearest_corner( macro, x, y, x_ret, y_ret )
INT macro, x, y, *x_ret, *y_ret ;
{
    INT j ;
    INT dist ;
    INT cdist ;
    INT closest_pt ;
    MACROPTR mptr ;
    VERTEXPTR vptr ;

    mptr = macroArrayG[macro] ;
    dist = INT_MAX ;
    for( j = 0; j < mptr->num_vertices; j++ ){
	vptr = mptr->vertices[j] ;
	cdist = ABS(vptr->x - x) + ABS(vptr->y - y) ;
	if( cdist < dist ){
	    closest_pt = j ;
	    dist = cdist ;
	}
    }
    /* now check the center point */
    cdist = ABS(mptr->xcenter-x) + ABS(mptr->ycenter-y) ;
    if( cdist < dist ){
	/* the center is the closest point */
	*x_ret = mptr->xcenter ;
	*y_ret = mptr->ycenter ;
    } else {
	/* one of the corner points */
	vptr = mptr->vertices[closest_pt] ;
	*x_ret = vptr->x ;
	*y_ret = vptr->y ;
    }
} /* end find_nearest_corner */


static highlight_corner( macro, x, y )
INT macro, x, y ;
{
    INT l, b, r, t ;   /* the core */
    INT xpand ;        /* blow up corner by this amount */

    get_core( &l, &b, &r, &t, TRUE ) ;
    xpand = MIN( r - l, t - b ) ;
    xpand = (INT) (0.025 * (DOUBLE) xpand ) ;
    xpand = MAX( xpand, 1 ) ; /* don't let it go to zero */
    TWdrawCell( 0, x-xpand,y-xpand,x+xpand,y+xpand,TWBLACK, NULL ) ;
    TWflushFrame() ;
    /*
    (void) sleep( (unsigned) 2 ) ;
    */

} /* end highlight_corner */

static outm( errtype, routine, string )
INT errtype ;
char *routine ;
char *string ;
{
    char buffer[LRECL] ;

    switch( errtype ){
    case ERRMSG:
	if( routine ){
	    sprintf( buffer, "ERROR[%s]:", routine ) ;
	} else {
	    sprintf( buffer, "ERROR:" ) ;
	}
	break ;
    case WARNMSG:
	if( routine ){
	    sprintf( buffer, "WARNING[%s]:", routine ) ;
	} else {
	    sprintf( buffer, "WARNING:" ) ;
	}
	break ;
    default :
	buffer[0] = EOS ;
    }
    strcat( buffer, string ) ;
    TWmessage( buffer ) ;
    sprintf( buffer, "%s\n", string ) ;
    M( errtype, routine, buffer ) ;
} /* end outm */

#else /* NOGRAPHICS */
/* dummy routines to resolve globals */
#endif
