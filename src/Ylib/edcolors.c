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
FILE:	    edcolors.c                                       
DESCRIPTION:These routines allow user to toggle colors of the 
    display.
CONTENTS:   TWtoggleColors()
DATE:	    Sep 23, 1989 - added TWtoggleColors()
REVISIONS:  Dec  7, 1990 - updated for new dialog procedure.
	    Wed May  1 18:56:14 EDT 1991 - added toggle for arb fill.
	    Sun Nov  3 12:52:21 EST 1991 - fixed gcc complaints.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) edcolors.c (Yale) version 1.8 12/15/91" ;
#endif

#ifndef NOGRAPHICS

#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>

#include <yalecad/base.h>
#include <yalecad/dialog.h>
#include <yalecad/colors.h>
#include <yalecad/draw.h>
#include <yalecad/string.h>
#include "info.h"

static TWDIALOGPTR fieldS ;
static init_field( P8(INT field, INT row, INT column, INT str_len, char *string,
		    INT type, INT color, INT group ) ) ;

/* be able to turn off individual colors */
TWtoggleColors()
{
    char **colors ;      /* the standard color array */
    INT  i ;             /* counter */
    INT  numfields ;     /* number of dialog fields */
    INT  numcolors ;     /* the number of colors in color array */
    INT  row ;           /* current row of dialog */
    INT  group ;         /* current case group of dialog */
    INT  color ;         /* current color processed */
    INT  field ;         /* current field of dialog */
    BOOL *colorOn ;      /* whether each color is on */
    TWDIALOGPTR fptr;    /* current field of dialog */
    TWDRETURNPTR answer ; /* return from user */
    TWINFOPTR info ;     /* get information from main draw routine */

    /* get colors for dialog window */
    info = TWgetDrawInfo() ;
    colors = info->colors ;
    numcolors = info->numColors ;
    colorOn = info->colorOn ;

    numfields = 3 * numcolors + 7 ;
    fieldS = YCALLOC( numfields, TWDIALOGBOX ) ;

    /* set the accept and reject fields */
    /* field, row, column, strlen, string, type, color, group  */
    init_field( 0, 1, 5, 8, " ACCEPT ", BUTTONTYPE, TWGREEN, 0 ) ;
    init_field( 1, 1,30, 8, " REJECT ", BUTTONTYPE, TWRED, 0 ) ;

    if( TWget_arb_fill() ){
	init_field( 2, 3, 5, 9, "Arb Fill:" , CASETYPE,  TWBLACK, 3 ) ;
    } else {
	init_field( 2, 3, 5, 9, "Arb Fill:" , CASETYPE,  TWBLACK, 4 ) ;
    }
    init_field( 3, 3, 30, 3, "on", BUTTONTYPE, TWGREEN, 1 ) ;
    init_field( 4, 3, 35, 3, "off", BUTTONTYPE, TWRED, 1 ) ;

    init_field( 5, 4, 1, 7, "Colors:" , LABELTYPE,  TWBLACK, 0 ) ;
    row = 5 ;
    group = 1 ;
    for( i = 1, field = 5; i <= numcolors; i++ ){
	field++ ;
	if( i == 1 ){ /* white - change to black 2 */
	    color = 2 ;
	} else {
	    color = i ;
	}
	/* set current initial switch position */
	if( colorOn[i] ){
	    init_field( field, ++row, 5, 5, colors[i], CASETYPE,
		color, field+1 ) ;
	} else {
	    init_field( field, ++row, 5, 5, colors[i], CASETYPE,
		color, field+2 ) ;
	}
	init_field( ++field, row, 30, 3, "on", BUTTONTYPE,
	    color, ++group ) ;
	init_field( ++field, row, 35, 3, "off", BUTTONTYPE,
	    color, group ) ;
    }
    /* terminate list with zeros */
    init_field( ++field, 0, 0, 0, 0, 0, 0, 0 ) ;

    /* initialization complete */

    if( answer = TWdialog( fieldS, "colors", NULL ) ){
	if( answer[3].bool ){
	    TWarb_fill( TRUE ) ;
	} else {
	    TWarb_fill( FALSE ) ;
	}
	for( i = 7, color = 0; i < numfields; i +=3 ){
	    if( answer[i].bool ){
    		colorOn[++color] = TRUE ;
	    } else {
    		colorOn[++color] = FALSE ;
	    }
	}
    }
    /* cleanup memory */
    for( i = 0; i < numfields; i++ ){
	if( fieldS[i].string ){
	    YFREE( fieldS[i].string ) ;
	}
    }
    YFREE( fieldS ) ;
} /* end TWtoggleColors */


static init_field( field, row, column, str_len, string, 
type, color, group )
INT field, row, column, str_len, type, color, group ;
char *string ;
{
    TWDIALOGPTR fptr;    /* current field of dialog */

    fptr = &(fieldS[field]) ;
    fptr->row = row ;
    fptr->column = column ;
    fptr->len = str_len ;
    fptr->type = type ;
    fptr->color = color ;
    fptr->group = group ;
    fptr->string = Ystrclone( string ) ;
} /* end init_field */

#endif /* NOGRAPHICS */
