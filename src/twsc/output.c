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
FILE:	    output.c                                       
DESCRIPTION:output the placement information.
CONTENTS:   output()
	    final_free_up()
	    create_cel_file()
DATE:	    Mar 27, 1989 
REVISIONS:  July 15, 1989
	    Oct   2, 1990 - removed elim_unused feeds and
		added check for clength == 0.
	    Wed Jan 16 14:25:22 PST 1991 - commented out free
		of pairArrayG.
	    Wed Jan 23 02:43:33 PST 1991 - output at density.
	    Sat May  4 13:47:58 EDT 1991 - now output row field
		similar to global routing definition.
	    Tue Aug 13 12:47:54 CDT 1991 - fixed create new
		cell file.
----------------------------------------------------------------- */
#ifndef VMS
#ifndef lint
static char SccsId[] = "@(#) output.c (Yale) version 4.14 9/23/91" ;
#endif
#endif

/* #define DEC  */
/* added on 06/01/90 Sury */
/* #define NSC */

#include "standard.h"
#include "groute.h"
#include "main.h"
#include "readpar.h"
#include "config.h"
#include "pads.h"
#include <string.h>
#include <yalecad/debug.h>
#include <yalecad/message.h>

/* external functions */
char *strtok(); /* added 06/01/90 sury */
INT comparex() ;

/* global variables */
extern INT spacer_widthG ;
extern INT actual_feed_thru_cells_addedG ;
extern BOOL output_at_densityG ;
extern BOOL create_new_cel_fileG ;
extern BOOL unused_feed_name_twspacerG ;
extern BOOL stand_cell_as_gate_arrayG ;

/* static definitions */
static char a_lineS[LRECL] ;

output()
{

FILE *fpp1 , *fpp2 ;
INT locx , locy , height , width ;
INT xloc , i , cell , block , orient ;
INT num ;
INT xloc2 , yloc2 , yloc ;
INT *array , desire , k , limit ;
INT left , right , bottom , top , end ;
INT *deleted_feeds ;
INT eliminated_feeds ;
char filename[LRECL] , ctmp[32] ;
CBOXPTR cellptr ;
TIBOXPTR tptr ;
PADBOXPTR pptr ;
/* added on 06/01/90 Sury */
INT length;
INT row ;
char instance_name[LRECL], tmp_name[LRECL], *tmp_string;


deleted_feeds = (INT *) Ysafe_malloc( numcellsG * sizeof(INT) ) ;
eliminated_feeds = 0 ;

sprintf( filename , "%s.pl1" , cktNameG ) ;
fpp1 = TWOPEN( filename , "w", ABORT ) ;

sprintf( filename , "%s.pl2" , cktNameG ) ;
fpp2 = TWOPEN( filename , "w", ABORT ) ;

for( block = 1 ; block <= numRowsG ; block++ ) {

    left   = barrayG[block]->bxcenter + barrayG[block]->bleft   ;
    right  = barrayG[block]->bxcenter + barrayG[block]->bright  ;
    bottom = barrayG[block]->bycenter + barrayG[block]->bbottom ;
    top    = barrayG[block]->bycenter + barrayG[block]->btop    ;

    if( restartG == 0 && costonlyG == 1 ) {
	desire = barrayG[block]->desire ;
    } else {
	if( pairArrayG[block][0] > 0 ) {
	    cell = pairArrayG[block][ pairArrayG[block][0] ] ;
	    end  = carrayG[cell]->cxcenter + carrayG[cell]->tileptr
			    ->right ;
	    desire = end - left ;
	} else {
	    desire = 0 ;
	}
    }

    if( barrayG[block]->borient > 0 ) {
	fprintf(fpp2,"%d %d %d  %d %d  0 0\n", block ,
			    left, bottom, left + desire, top ) ;
    } else {
	fprintf(fpp2,"%d %d %d  %d %d  0 0\n", block ,
			    left, bottom, right, bottom + desire ) ;
    }

    num = pairArrayG[block][0] ;
    if( num == 0 ) {
	continue ;
    }
    array = pairArrayG[block] + 1 ;

    for( i = 0 ; i < num ; i++ ) {
	cell = array[ i ] ;
	cellptr = carrayG[ cell ] ;
	if( cellptr->clength == 0 ){
	    continue ;
	}
	if( strcmp( cellptr->cname , "TW_EXCEPT" ) == 0 ) {
	    continue ;
	}
	if( stand_cell_as_gate_arrayG ) {
	    if( strcmp( cellptr->cname , "GATE_ARRAY_SPACER" ) == 0 ) {
		continue ;
	    }
	}
	if( unused_feed_name_twspacerG ) {
	    if( strncmp(cellptr->cname,"twfeed",6) == STRINGEQ ) {
		if( tearrayG[cellptr->imptr->terminal] == NULL ) {
		    strcpy( ctmp , cellptr->cname ) ;
		    cellptr->cname = (char *) Ysafe_realloc(
			    cellptr->cname, (strlen(cellptr->cname)+3) *
			    sizeof(char) ) ;
		    sprintf( cellptr->cname , "%s%s" , "twspacer",
				strpbrk(ctmp, "0123456789") ) ;
		}
	    }
	}
	orient = cellptr->corient ;

	xloc = cellptr->cxcenter + cellptr->tileptr->left ;
	yloc = cellptr->cycenter + cellptr->tileptr->bottom ;

	xloc2 = cellptr->tileptr->right - 
		cellptr->tileptr->left ; 
	yloc2 = cellptr->tileptr->top - 
		cellptr->tileptr->bottom ; 

/* The following code was added on 06/01/90 Sury */
#ifdef NSC
	strcpy( tmp_name , cellptr->cname ) ;
	length = strcspn( tmp_name , ":" ) ;
	if( length < strlen( tmp_name ) ) {
	    tmp_string = strtok( tmp_name , ":" ) ;
	    tmp_string = strtok( NULL , ":" ) ;
	    sprintf( instance_name, "%s" , tmp_string ) ;
	} else {
	    sprintf( instance_name , "%s" , tmp_name ) ;
	}
	fprintf(fpp1,"%s %d %d  %d %d  %d %d\n",
			instance_name,
			xloc, yloc, xloc + xloc2,
			yloc + yloc2, orient, block ) ;
#else
	fprintf(fpp1,"%s %d %d  %d %d  %d %d\n",
			cellptr->cname ,
			xloc, yloc, xloc + xloc2,
			yloc + yloc2, orient, block ) ;
#endif
    }
}

if( deleted_feeds[0] > 0 ) {
    fprintf(fpoG,"Confirming number of eliminated feeds:%d\n",
					    eliminated_feeds ) ;
}

/* now output the pads and macros */
for( i = numcellsG + 1 ; i <= lastpadG ; i++ ) {
    cellptr = carrayG[ i ]  ;
    orient = cellptr->corient ;
    tptr = cellptr->tileptr ;
    left = tptr->left ;
    right = tptr->right ;
    bottom = tptr->bottom ;
    top = tptr->top ;
    YtranslateT( &left, &bottom, &right, &top, orient ) ;
    locx = cellptr->cxcenter + left ;
    locy = cellptr->cycenter + bottom ;
    height = top - bottom ;
    width =  right - left ;
    /* determine row */
    pptr = cellptr->padptr ;
    if( pptr->macroNotPad ){
	row = 0 ;
    } else {
	switch( pptr->padside ){
	    case L:
		row = - 1 ;
		break ;
	    case T:
		row = -4 ;
		break ;
	    case R:
		row = - 2 ;
		break ;
	    case B:
		row = - 3 ;
		break ;
	    default:
		M(ERRMSG,"output", "Unknown padside\n" ) ;
	}
    }


#ifndef DEC
/* The following code was added on 06/01/90 Sury */
#ifdef NSC
	strcpy( tmp_name , cellptr->cname ) ;
	length = strcspn( tmp_name , ":" ) ;
	if( length < strlen( tmp_name ) ) {
	    tmp_string = strtok( tmp_name , ":" ) ;
	    tmp_string = strtok( NULL , ":" ) ;
	    sprintf( instance_name, "%s" , tmp_string ) ;
	} else {
	sprintf( instance_name , "%s" , tmp_name ) ;
	}
    fprintf(fpp1,"%s %d %d  %d %d  %d %d\n", instance_name,
				locx, locy, locx + width,
				locy + height, orient, row ) ;
    fprintf(fpp2,"%s %d %d  %d %d  %d %d\n", instance_name,
				locx, locy, locx + width,
				locy + height, orient, row ) ;
#else
    /* normal case */
    fprintf(fpp1,"%s %d %d  %d %d  %d %d\n", cellptr->cname ,
				locx, locy, locx + width,
				locy + height, orient, row ) ;
    fprintf(fpp2,"%s %d %d  %d %d  %d %d\n", cellptr->cname ,
				locx, locy, locx + width,
				locy + height, orient, row ) ;
#endif
#else
    /* DEC case */
    fprintf(fpp1,"%s %d %d  %d %d  %d %d\n", cellptr->cname ,
		    locx, locy, locx + width,
		    locy + height, orient, -cellptr->padside ) ;
    fprintf(fpp2,"%s %d %d  %d %d  %d %d\n", cellptr->cname ,
		    locx, locy, locx + width,
		    locy + height, orient, -cellptr->padside ) ;
#endif

}
TWCLOSE( fpp1 ) ;
TWCLOSE( fpp2 ) ;

if( create_new_cel_fileG ) {
    create_cel_file() ;
}

Ysafe_free( deleted_feeds ) ;

return ;
}





final_free_up()
{
INT i, j, k, row, pin, net, cell, chan, track ;
CBOXPTR cellptr ;
DBOXPTR dimptr ;
PINBOXPTR ptr, nextptr ;
ADJASEGPTR adj, nextadj ;
SEGBOXPTR segptr, nextseg ;
CHANGRDPTR gdptr, nextgrd ;
DENSITYPTR *hdptr ;
IPBOXPTR imptr ;
FEED_DATA *feedptr ;

Ysafe_free( FeedInRowG ) ;
for( row = 1 ; row <= numRowsG ; row++ ) {
    Ysafe_free( impFeedsG[row] ) ;
}
Ysafe_free( impFeedsG ) ;

for( i = 1 ; i <= numRowsG ; i++ ) {
    feedptr = feedpptrG[i] ;
    for( j = 1 ; j <= chan_node_noG ; j++ ) {
	Ysafe_free( feedptr[j] ) ;
    }
    Ysafe_free( feedpptrG[i] ) ;
}
Ysafe_free( feedpptrG ) ;

k = max_tdensityG + 100 ;
for( chan = 1 ; chan <= numChansG ; chan++ ) {
    Ysafe_free( BeginG[chan]->netptr ) ;
    Ysafe_free( EndG[chan]->netptr ) ;
    for( gdptr = BeginG[chan] ; gdptr ; gdptr = nextgrd ) {
	nextgrd = gdptr->nextgrd ;
	Ysafe_free( gdptr->dptr ) ;
	Ysafe_free( gdptr ) ;
    }

    hdptr = DboxHeadG[chan] ;
    for( track = 0 ; track <= k ; track++ ) {
	Ysafe_free( hdptr[track] ) ;
    }
    Ysafe_free( DboxHeadG[chan] ) ;
}
Ysafe_free( BeginG ) ;
Ysafe_free( EndG ) ;
Ysafe_free( DboxHeadG ) ;
Ysafe_free( maxTrackG ) ;
Ysafe_free( nmaxTrackG ) ;

k = maxtermG + 2 * numChansG ;
for( pin = 1 ; pin <= k ; pin++ ) {
    Ysafe_free( TgridG[pin] ) ;
}
Ysafe_free( TgridG ) ;

/*
Not needed as this is done in findunlap.c
for( i = 1 ; i <= numRowsG ; i++ ) {
    Ysafe_free( pairArrayG[i] ) ;
}
Ysafe_free( pairArrayG ) ;
pairArrayG = NULL ;
*/

for( cell = 1 ; cell <= numcellsG ; cell++ ) {
    cellptr = carrayG[cell] ;
    imptr = cellptr->imptr ;
    if( imptr ) {
	for( ; imptr->next; imptr = imptr->next ) {
	    if( imptr->next->cell != cell ) break ;
	}
	imptr->next = NULL ;
    }
}
fprintf( fpoG,"Actual # of Feed Cells Added:\t%d\n\n\n",
    actual_feed_thru_cells_addedG ) ;
k = numcellsG + numtermsG + actual_feed_thru_cells_addedG ;
for( cell = numcellsG + numtermsG + 1 ; cell <= k ; cell++ ) {
    cellptr = carrayG[cell] ;
    cellptr->imptr->next = NULL ;
    Ysafe_free( cellptr->cname ) ;
    Ysafe_free( cellptr->tileptr ) ;
    Ysafe_free( cellptr->imptr->pinname ) ;
    Ysafe_free( cellptr->imptr->eqpinname ) ;
    Ysafe_free( cellptr->imptr ) ;
    Ysafe_free( carrayG[cell] ) ;
    carrayG[cell] = NULL ;
}
actual_feed_thru_cells_addedG = 0 ;

for( net = 1 ; net <= numnetsG ; net++ ) {
    for( segptr = netsegHeadG[net] ; segptr ; segptr = nextseg ) {
	nextseg = segptr->next ;
	Ysafe_free( segptr ) ;
    }
    dimptr = netarrayG[net] ;
    if( !dimptr->pins ) {
	continue ;
    }
    if( dimptr->pins->terminal > TotRegPinsG ) {
	for( ptr = dimptr->pins ; ptr ; ptr = nextptr ) {
	    nextptr = ptr->next ;
	    for( adj = ptr->adjptr ; adj ; adj = nextadj ) {
		nextadj = adj->next ;
		Ysafe_free( adj ) ;
	    }
	    Ysafe_free( ptr->eqptr ) ;
	    if( nextptr->terminal <= TotRegPinsG ) break ;
	}
	dimptr->pins = nextptr ;
    }
    for( ptr = dimptr->pins ; ptr ; ptr = ptr->next ) {
	for( adj = ptr->adjptr->next ; adj ; adj = nextadj ) {
	    nextadj = adj->next ;
	    Ysafe_free( adj ) ;
	}
	ptr->adjptr->next = NULL ;
    }
}
Ysafe_free( netsegHeadG ) ;

}




create_cel_file()
{


FILE *fpoG2 , *fp ;
char *token , fixed_string[32] , filename[256] ;
char cell_name[32] ;
INT ignore_line , block , offset , test , carrayG_index , is_a_cell ;


if( rowsG > 0 ) {
    sprintf( filename , "%s.scel" , cktNameG ) ;
    fp = TWOPEN( filename, "r" , ABORT ) ;
} else {
    sprintf( filename , "%s.cel" , cktNameG ) ;
    fp = TWOPEN( filename, "r" , ABORT ) ;
}
sprintf( filename , "%s.ncel" , cktNameG ) ;
fpoG2 = TWOPEN( filename, "w" , ABORT ) ;

strcpy( fixed_string , "" ) ;
carrayG_index = 0 ;
while(  load_a_lineS(fp)  ) {
    ignore_line = 0 ;
    if( (token = strtok( a_lineS , " 	" )) != NULL ) {
	if( strcmp( token , "cell" ) == 0 ) {
	    is_a_cell = 1 ;
	    fprintf(fpoG2, "%s ", token ) ;
	    token = strtok( NULL , " 	" ) ;
	    fprintf(fpoG2, "%s ", token ) ;
	    strcpy( cell_name , strtok( NULL , " 	" ) ) ;
	    fprintf(fpoG2, "%s ", cell_name ) ;
	    do {
		test = strcmp( carrayG[ ++carrayG_index ]->cname,
					cell_name ) ;
	    } while( test != 0 ) ;
	 } else if( strcmp( token , "left" ) == 0 ) {
	    if( is_a_cell ) {
		block  = carrayG[carrayG_index]->cblock ;
		offset = carrayG[carrayG_index]->cxcenter +
			 carrayG[carrayG_index]->tileptr->left -
			(barrayG[block]->bxcenter +
			 barrayG[block]->bleft) ;

		if( strcmp( fixed_string , "" ) != 0 ) {
		    add_new_line( offset , block , fixed_string , fpoG2 ) ;
		    strcpy( fixed_string , "" ) ;
		} else {
		    add_new_line( offset , block , "nonfixed" , fpoG2 ) ;
		}
	    }
	    fprintf(fpoG2, "%s ", token ) ;
	    is_a_cell = 0 ;
	} else if( strcmp( token , "initially" ) == 0 ) {
	    ignore_line = 1 ;
	    strcpy( fixed_string , strtok( NULL , " 	" ) ) ;
	} else if( strcmp( token , "\n" ) != 0 ) {
	    fprintf(fpoG2, "%s ", token ) ;
	}
	while( (token = strtok( NULL , " 	" )) != NULL ) {
	    if( !ignore_line ) {
		fprintf(fpoG2, "%s ", token ) ;
	    }
	}
    }
    if( !ignore_line ) {
	fprintf(fpoG2, "\n");
    }
}
TWCLOSE(fp) ;
TWCLOSE(fpoG2) ;
}



add_new_line( x_rel , block , fixed_ptr , fp )
INT x_rel , block ;
char *fixed_ptr ;
FILE *fp ;
{

fprintf(fp, "initially %s %d from left of block %d\n",
				fixed_ptr , x_rel , block ) ;
return ;
}



load_a_lineS(fp)
FILE *fp ;
{

INT i ;
char tmp ;

if( (int) (tmp = fgetc(fp)) != EOF ) {
    a_lineS[0] = tmp ;
    i = 1 ;
    if( tmp != '\n' ) {
	while( (a_lineS[i] = fgetc(fp)) != '\n' ) {
	    i++ ;
	}
    }
    a_lineS[i] = '\0' ;
    return(1) ;
} else {
    return(0) ;
}
}

/* ******************************************************************** */
density()
{
    /* set all the cells at density */
    INT row ;
    INT cell ;
    INT block ;
    INT rowtop ;
    INT corient ;
    INT rowcenter ;
    CBOXPTR cellptr ;
    PINBOXPTR pin ;

    /* first update all the y coordinates of rows */
    if (maxTrackG){   /* only perform if global routing is performed */
	for( row = 1 ; row <= numRowsG ; row++ ) {
	    if( row == 1 ){
		rowcenter = barrayG[1]->bheight / 2 + 
			maxTrackG[1]*track_pitchG + route2actG ;
	    } else {
		rowcenter = rowtop + barrayG[row]->bheight / 2 +
		    maxTrackG[row]*track_pitchG + route2actG ;
	    }
	    rowtop = rowcenter + barrayG[row]->bheight - 
		        barrayG[row]->bheight / 2;
	    D( "twsc/buildDensityArray",
		fprintf( stderr, "row:%d oldy:%d newy:%d tracks:%d\n",
		    row, barrayG[row]->bycenter, rowcenter, 
		    maxTrackG[row] ) ;
	    ) ;
	    barrayG[row]->bycenter = rowcenter ;
	}
    } else {
	return ;
    }

    for( cell = 1; cell <= numcellsG; cell++ ){
	cellptr = carrayG[cell] ;
	block = cellptr->cblock ;
	corient = cellptr->corient ;
	if( block != 0 ){
	    cellptr->cycenter = barrayG[block]->bycenter ;
	    /* update the y positions on this cell */
	    for( pin = cellptr->pins;pin;pin=pin->nextpin ) {
		pin->ypos = pin->typos[corient%2] + cellptr->cycenter;
	    }
	}
    }

} /* end density */
