/*
 *   Copyright (C) 1990-1991 Yale University
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
FILE:	    seagate.c                                       
DESCRIPTION:Sea of gate global routing translator function.
CONTENTS:   
DATE:	    Apr  7, 1990 
REVISIONS:  
----------------------------------------------------------------- */
#ifndef VMS
#ifndef lint
static char SccsId[] = "@(#) seagate.c (Yale) version 4.7 3/7/91" ;
#endif
#endif


#include "standard.h"
#include "groute.h"
#include "main.h"
#include "readpar.h"
#include "pads.h"


extern INT vertical_track_pitchG ;
extern INT horizontal_track_pitchG ;
extern INT global_routing_iterationsG ;
extern BOOL new_row_formatG ;
extern BOOL min_peak_densityG ;
extern BOOL min_total_densityG ;
extern BOOL stand_cell_as_gate_arrayG ;

seagate_input()
{
FILE *fp ;
CBOXPTR cellptr ;
IPBOXPTR imptr ;
PINBOXPTR pinptr ;
char filename[128], *pin_name, *eqpin_name ;
INT left, rite, *Aray, left_most_pitch ;
INT coreLeft, coreRite, coreBot, coreTop, cxcenter ;
INT x, y, n, m, net, row, cell, padside, templateHeight ;
INT left_edge , rite_edge ;
INT top_edge , bot_edge , first_pin ;

coreBot  = INT_MIN ;
coreTop  = INT_MAX ;

m = pairArrayG[1][0] ;

cellptr = carrayG[ pairArrayG[1][1] ] ;
coreLeft = cellptr->cxcenter + cellptr->tileptr->left ;

cellptr = carrayG[ pairArrayG[1][m] ] ;
coreRite = cellptr->cxcenter + cellptr->tileptr->right ;

for( row = 2 ; row <= numRowsG ; row++ ) {
    Aray = pairArrayG[row] ;
    m = Aray[0] ;

    cellptr = carrayG[ Aray[1] ] ;
    left = cellptr->cxcenter + cellptr->tileptr->left ;

    cellptr = carrayG[ Aray[m] ] ;
    rite = cellptr->cxcenter + cellptr->tileptr->right ;

    if( left < coreLeft ) coreLeft = left ;
    if( rite > coreRite ) coreRite = rite ;
}

for( cell = lastpadG ; cell > numcellsG ; cell-- ) {
    cellptr = carrayG[cell] ;
    if( cellptr->padptr ){
	padside = cellptr->padptr->padside ;
    } else {
	continue ;
    }
    switch( padside ) {
    case T :
    case MT :
    case MTT :
	y = cellptr->cycenter + cellptr->tileptr->bottom ;
	if( y < coreTop ) coreTop = y ;
	break ;
    case B :
    case MB :
    case MBB :
	y = cellptr->cycenter + cellptr->tileptr->top ;
	if( y > coreBot ) coreBot = y ;
	break ;
    default :
	break ;
    }
}
if( coreBot == INT_MIN ) {
    cellptr = carrayG[ pairArrayG[1][1] ] ;
    coreBot = cellptr->cycenter + cellptr->tileptr->bottom ;
}
if( coreTop == INT_MAX ) {
    cellptr = carrayG[ pairArrayG[numRowsG][1] ] ;
    coreTop = cellptr->cycenter + cellptr->tileptr->top ;
}
templateHeight = barrayG[2]->bycenter - barrayG[1]->bycenter ;

left_most_pitch = coreRite ;
for( cell = 1 ; cell <= numcellsG ; cell++ ) {
    cellptr = carrayG[cell] ;
    cxcenter = cellptr->cxcenter ;
    for( imptr = cellptr->imptr ; imptr ; imptr = imptr->next ) {
	if( cellptr->corient <= 1 ) {
	    imptr->xpos = cxcenter + imptr->txpos ;
	    if( imptr->xpos < left_most_pitch )
		left_most_pitch = imptr->xpos ;
	} else {
	    if( cellptr->clength % 2 == 0 ) {
		imptr->xpos = cxcenter - imptr->txpos ;
	    } else {
		imptr->xpos = cxcenter - imptr->txpos - 1 ;
	    }
	    if( imptr->xpos < left_most_pitch )
		left_most_pitch = imptr->xpos ;
	}
    }
}
sprintf( filename, "%s.sgpar", cktNameG ) ;
fp = TWOPEN( filename, "w", ABORT ) ;
if( min_peak_densityG ) {
    /* do nothing */
} else if( min_total_densityG ) {
    fprintf( fp, "standard_cell\n") ;
} else {
    if( !gate_arrayG || stand_cell_as_gate_arrayG ) {
	fprintf( fp, "standard_cell\n") ;
    }
}
if( global_routing_iterationsG == 0 ) {
    global_routing_iterationsG = 5 ;
}
fprintf( fp, "global_routing_iterations %d\n", global_routing_iterationsG);
fprintf( fp, "chipLeft %d\n", coreLeft ) ;
fprintf( fp, "chipRite %d\n", coreRite ) ;
fprintf( fp, "chipTop %d\n", coreTop ) ;
fprintf( fp, "chipBot %d\n", coreBot ) ;
fprintf( fp, "left_most_pitch %d\n", left_most_pitch );
fprintf( fp, "x_pitch %d\n", vertical_track_pitchG ) ;
fprintf( fp, "y_pitch %d\n", horizontal_track_pitchG ) ;
fprintf( fp, "channel_capacity 1000\n");
fprintf( fp, "numblocks %d\n", numRowsG + 1 ) ;
fprintf( fp, "templateHeight %d\n", templateHeight ) ;
fprintf( fp, "vt_offset %d\n", barrayG[1]->bycenter - coreBot ) ;

if( pin_layers_givenG ) {
    fprintf(fp,"pin_layers_given\n");
    fprintf(fp,"feedLayer %d\n", feedLayerG );
}
if( rowsG > 0 || new_row_formatG ) {
    fprintf(fp,"new_row_format\n");
}
fprintf( fp, "top_of_rows %d\n", barrayG[numRowsG]->bycenter +
				 barrayG[numRowsG]->btop ) ;
fprintf( fp, "bot_of_rows %d\n", barrayG[1]->bycenter +
				 barrayG[1]->bbottom ) ;
for( row = 1 ; row <= numRowsG ; row++ ) {
    left_edge = barrayG[row]->bxcenter + barrayG[row]->bleft ;
    cellptr = carrayG[ pairArrayG[row][ pairArrayG[row][0] ] ] ;
    rite_edge = cellptr->cxcenter + cellptr->tileptr->right ;
    bot_edge = barrayG[row]->bycenter + barrayG[row]->bbottom ;
    top_edge = barrayG[row]->bycenter + barrayG[row]->btop ;
    fprintf(fp, "%d %d %d %d\n", left_edge , rite_edge ,
				 bot_edge , top_edge  ) ;
}
TWCLOSE( fp ) ;

sprintf( filename, "%s.sgnet", cktNameG ) ;
fp = TWOPEN( filename, "w", ABORT ) ;
for( net = 1 ; net <= numnetsG ; net++ ) {
    first_pin = 1 ;
    for( pinptr = netarrayG[net]->pins ;pinptr ; pinptr = pinptr->next ) {
	cellptr = carrayG[pinptr->cell] ;
	if( cellptr->padptr ){
	    padside = cellptr->padptr->padside ;
	} else {
	    padside = 0 ;
	}
	if( !padside ) {
	    x = pinptr->xpos ;
	    if( pinptr->eqptr ) {
		y = barrayG[pinptr->row]->bycenter ;
		switch( cellptr->corient ) {
		case 0:
		case 2:
		    if( pinptr->eqptr->typos > 0 ) {
			  pin_name = pinptr->eqptr->pinname;
			eqpin_name = pinptr->pinname ;
		    } else {
			  pin_name = pinptr->pinname ;
			eqpin_name = pinptr->eqptr->pinname;
		    }
		    break ;
		case 1:
		case 3:
		    if( pinptr->eqptr->typos > 0 ) {
			  pin_name = pinptr->pinname ;
			eqpin_name = pinptr->eqptr->pinname;
		    } else {
			  pin_name = pinptr->eqptr->pinname;
			eqpin_name = pinptr->pinname ;
		    }
		    break ;
		}
	    } else {
		y = pinptr->ypos ;
		pin_name = pinptr->pinname;
	    }
	} else {
	    x = pinptr->xpos ;
	    y = pinptr->ypos ;
	    pin_name = pinptr->pinname ;
	}
	if( first_pin ) {
	    fprintf( fp, "signal %s\n", netarrayG[net]->name ) ;
	    first_pin = 0 ;
	}
	if( pinptr->eqptr ) {
	    fprintf(fp, " %s %s %d %d %s\n",
		pin_name, eqpin_name, x, y, cellptr->cname ) ;
	} else {
	    fprintf(fp, " %s DUMMY %d %d %s\n",
		pin_name, x, y, cellptr->cname ) ;
	}
    }
}
TWCLOSE( fp ) ;

sprintf( filename, "%s.sgpass", cktNameG ) ;
fp = TWOPEN( filename, "w", ABORT ) ;
for( cell = 1 ; cell <= numcellsG ; cell++ ) {
    cellptr = carrayG[cell] ;
    cxcenter = cellptr->cxcenter ;
    for( imptr = cellptr->imptr ; imptr ; imptr = imptr->next ) {
	if( cellptr->corient <= 1 ) {
	    imptr->xpos = cxcenter + imptr->txpos ;
	} else {
	    if( cellptr->clength % 2 == 0 ) {
		imptr->xpos = cxcenter - imptr->txpos ;
	    } else {
		imptr->xpos = cxcenter - imptr->txpos - 1 ;
	    }
	}
    }
}
link_imptr() ;
for( row = 1 ; row <= numRowsG ; row++ ) {
    n = 0;
    for( imptr = impFeedsG[row]->next ; imptr ; imptr= imptr->next ) n++;
    fprintf(fp, " blk %d numFeeds %d\n", row+1, n ) ;
    for( imptr = impFeedsG[row]->next ; imptr ; imptr= imptr->next ) {
	cellptr = carrayG[imptr->cell] ;
	if( cellptr->corient == 0 || cellptr->corient == 2 ) {
	    fprintf(fp, "%d %s %s %s\n", imptr->xpos, imptr->pinname,
		imptr->eqpinname, cellptr->cname );
	} else {
	    fprintf(fp, "%d %s %s %s\n", imptr->xpos, imptr->eqpinname,
		imptr->pinname, cellptr->cname );
	}
    }
    fprintf(fp, "\n" );
}
for( cell = 1 ; cell <= numcellsG ; cell++ ) {
    cellptr = carrayG[cell] ;
    imptr = cellptr->imptr ;
    if( imptr ) {
	for( ; imptr->next ; imptr = imptr->next ) {
	    if( imptr->next->cell != cell ) break ;
	}
	imptr->next = NULL ;
    }
}
TWCLOSE( fp ) ;
}
