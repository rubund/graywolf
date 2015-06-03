/*
 *   Copyright (C) 1989-1990 Yale University
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
FILE:	    findrcost.c                                       
DESCRIPTION:find routing cost.
CONTENTS:   findrcost()
	    initial_tracks( segptr )
		SEGBOXPTR segptr ;
	    set_cedgebin()
	    reset_track()
	    facing_cellheight( pin , row , pinloc , status )
		INT pin, row , pinloc , status ;
DATE:	    Mar 27, 1989 
REVISIONS:  Sat Dec 15 22:08:21 EST 1990 - modified pinloc values
		so that it will always be positive.
----------------------------------------------------------------- */
#ifndef VMS
#ifndef lint
static char SccsId[] = "@(#) findrcost.c (Yale) version 4.6 12/15/90" ;
#endif
#endif

#include "standard.h"
#include "groute.h"

static INT **cedgebinS ;
static INT cedge_binwidthS ;
static INT num_edgebinS ;

findrcost()
{

SEGBOXPTR segptr ;
CHANGRDPTR gdptr ;
DENSITYPTR *hdptr , headptr , dptr ;
INT chan ;
INT trackG , max_trk , k ;
INT net ;


for( net = 1 ; net <= numnetsG ; net++ ) {
    for( segptr = netsegHeadG[net]->next ;
	segptr ; segptr = segptr->next ){
	initial_tracks( segptr ) ;
    }
    for( segptr = netsegHeadG[net]->next ;
	segptr ; segptr = segptr->next ){
	process_cross( segptr , 0 ) ;
    }
}
DboxHeadG  = ( DENSITYPTR ** )Ysafe_calloc( numChansG + 1,
				sizeof( DENSITYPTR * ) ) ;
maxTrackG  = (INT *)Ysafe_malloc( ( numChansG + 1 ) * sizeof( INT ) ) ;
nmaxTrackG = (INT *)Ysafe_malloc( ( numChansG + 1 ) * sizeof( INT ) ) ;
max_tdensityG = 0 ;
tracksG   = 0 ;
if( uneven_cell_heightG ) {
    set_cedgebin( ) ;
    reset_track( ) ;
}
for( chan = 1 ; chan <= numChansG ; chan++ ) {
    max_trk = 0 ;
    for( gdptr = BeginG[ chan ] ; gdptr != GRDNULL ;
					gdptr = gdptr->nextgrd ) {
	if( gdptr->tracks > max_trk ) {
	    max_trk = gdptr->tracks ;
	}
    }
    maxTrackG[ chan ]   = max_trk ;
    if( max_trk > max_tdensityG ) {
	max_tdensityG = max_trk ;
    }
    tracksG += max_trk ;
}
printf(" the starting value of tracks = %4d\n" , tracksG ) ;
k = max_tdensityG + 100 ;
for( chan = 1 ; chan <= numChansG ; chan++ ) {
    DboxHeadG[ chan ]  = hdptr = ( DENSITYPTR *)Ysafe_calloc( k + 1,
			    sizeof( DENSITYPTR) ) ;
    for( trackG = 0 ; trackG <= k ; trackG++ ) {
	hdptr[trackG] = ( DENSITYPTR )Ysafe_calloc( 1, sizeof(DENSITYBOX));
    }
    for( gdptr = BeginG[ chan ] ; gdptr ; gdptr = gdptr->nextgrd ) {
	trackG = gdptr->tracks ;
	gdptr->dptr = dptr = 
	    ( DENSITYPTR )Ysafe_calloc( 1,sizeof(DENSITYBOX) ) ;
	headptr = hdptr[trackG] ;
	if( headptr->next ) {
	    dptr->next = headptr->next ;
	    dptr->next->back = dptr ;
	    headptr->next = dptr ;
	    dptr->back = headptr;
	} else {
	    headptr->next = dptr ;
	    dptr->back = headptr ;
	}
	dptr->grdptr = gdptr ;
    }
}
}

initial_tracks( segptr )
SEGBOXPTR segptr ;
{

INT x1 , x2 , pin1 , pin2 ;
PINBOXPTR netptr1 , netptr2 ;
CHANGRDPTR ptr1 , ptr2 , ptr ;

x1 = segptr->pin1ptr->xpos ;
x2 = segptr->pin2ptr->xpos ;
if( x1 == x2 ) {
    return ;
}
netptr1 = segptr->pin1ptr ;
netptr2 = segptr->pin2ptr ;
pin1 = netptr1->terminal ;
pin2 = netptr2->terminal ;
if( netptr1->row < netptr2->row ) {
    ptr1 = TgridG[pin1]->up ;
    ptr2 = TgridG[pin2]->down ;
} else if( netptr1->row > netptr2->row ) {
    ptr1 = TgridG[pin1]->down ;
    ptr2 = TgridG[pin2]->up ;
} else if( segptr->switchvalue == swUP ) {
    ptr1 = TgridG[pin1]->up ;
    ptr2 = TgridG[pin2]->up ;
} else if( segptr->switchvalue == swDOWN ) {
    ptr1 = TgridG[pin1]->down ;
    ptr2 = TgridG[pin2]->down ;
} else if( netptr1->pinloc >= NEITHER && netptr2->pinloc >= NEITHER ) {
    ptr1 = TgridG[pin1]->up ;
    ptr2 = TgridG[pin2]->up ;
} else {
    ptr1 = TgridG[pin1]->down ;
    ptr2 = TgridG[pin2]->down ;
}
while( ptr1->prevgrd ) {
    if( ptr1->prevgrd->netptr->xpos == x1 ) {
	ptr1 = ptr1->prevgrd ;
    } else {
	break ;
    }
}
while( ptr2->nextgrd ) {
    if( ptr2->nextgrd->netptr->xpos == x2 ) {
	ptr2 = ptr2->nextgrd ;
    } else {
	break ;
    }
}

ptr2 = ptr2->nextgrd ;

for( ptr = ptr1 ; ptr != ptr2 ; ptr = ptr->nextgrd ) {
    if( ptr->cross == 0 ) {
	ptr->tracks++ ;
	ptr->cross = 1 ;
    }
}
}

/* the set_cedgebin() , reset_track , facing_cellheight() function
    would be used only when the cells are in uneven height */

set_cedgebin()
{

CBOXPTR cellptr ;
INT channel , row , bin , bin_rite_edge , crite ;
INT most_left , most_rite , rite , i ;

cedgebinS = (INT **)Ysafe_malloc( ( numChansG + 1 ) * sizeof(INT *) ) ;
cedge_binwidthS = (INT)( 2.0 * mean_widthG ) ;
cellptr = carrayG[ pairArrayG[1][ pairArrayG[1][0] ] ] ;
most_rite = cellptr->cxcenter + cellptr->tileptr->right ;
for( row = 2 ; row <= numRowsG ; row++ ) {
    cellptr = carrayG[ pairArrayG[row][ pairArrayG[row][0] ] ] ;
    rite = cellptr->cxcenter + cellptr->tileptr->right ;
    if( rite > most_rite ) {
	most_rite = rite ;
    }
}
num_edgebinS = ( most_rite - blkleftG ) / cedge_binwidthS + 1 ;
for( channel = 1 ; channel <= numChansG ; channel++ ) {
    cedgebinS[ channel ] = (INT *)Ysafe_malloc(
		( num_edgebinS + 1 ) * sizeof( INT ) ) ;
}


for( row = 1 ; row <= numRowsG ; row++ ) {
    cellptr = carrayG[ pairArrayG[row][1] ] ;
    most_left = cellptr->cxcenter + cellptr->tileptr->left ;
    bin_rite_edge = most_left + cedge_binwidthS ;
    cedgebinS[row][1] = 1 ;
    bin = 1 ;
    for( i = 1 ; i <= pairArrayG[row][0] ; i++ ) {
	cellptr = carrayG[ pairArrayG[row][i] ] ; 
	crite = cellptr->cxcenter + cellptr->tileptr->right ;
	if( bin_rite_edge < crite ) {
	    while( bin_rite_edge <= crite ) {
		cedgebinS[row][++bin] = i ;
		bin_rite_edge += cedge_binwidthS ;
	    }
	}
    }
    for( ++bin ; bin <= num_edgebinS ; bin++ ) {
	cedgebinS[row][bin] = 0 ;
    }
}
}


reset_track()
{

CBOXPTR cellptr ;
TIBOXPTR tileptr ;
PINBOXPTR termptr ;
INT cell , pin , pinloc , row , top , bottom ;

for( cell = 1 ; cell <= numcellsG ; cell++ ) {
    cellptr = carrayG[cell] ;
    tileptr = cellptr->tileptr ;
    row =  cellptr->cblock ;
    top =  tileptr->top ;
    bottom = -tileptr->bottom ;
    for( termptr = cellptr->pins ;termptr; termptr = termptr->nextpin ) {

	pin = termptr->terminal ;
	pinloc = tearrayG[pin]->pinloc ;
	if( pinloc == TOPCELL ) {
	    TgridG[ pin ]->up->tracks += ( top +
	    facing_cellheight( pin , row , 1 , NEW ) ) / track_pitchG ;
	} else if( pinloc == BOTCELL ) {
	    TgridG[ pin ]->up->tracks += ( bottom +
	    facing_cellheight( pin , row , -1 , NEW ) ) / track_pitchG;
	} else {  /* pinloc == 0 */
	    TgridG[ pin ]->up->tracks += ( top +
	    facing_cellheight( pin , row , 1 , NEW ) ) / track_pitchG ;
	    TgridG[ pin ]->down->tracks += ( bottom +
	    facing_cellheight( pin , row , -1 , NEW ) ) / track_pitchG;
	}
    }
}
for( ; cell <= lastpadG ; cell++ ) {
    cellptr = carrayG[cell] ;
    for( termptr = cellptr->pins ;termptr; termptr = termptr->nextpin ) {
	pin = termptr->terminal ;
	row = tearrayG[ pin ]->row ;
	TgridG[ pin ]->up->tracks += facing_cellheight(
		pin , row , 0 , NEW ) / track_pitchG ;
    }
}
}


facing_cellheight( pin , row , pinloc , status )
INT pin, row , pinloc , status ;
{

CBOXPTR cellptr ;
INT bin , i , *Aray , most_left , most_rite ;
INT nrow , pin_x , crite ;

if( 1 <= row  && row <= numRowsG ) {
    nrow = row + pinloc - PINLOC_OFFSET ;
    if( nrow == 0 || nrow == numChansG || pinloc == NEITHER ) {
	return(0) ;
    }
} else if( row == 0 ) {
    nrow = 1 ;
} else {
    nrow = numRowsG ;
}

Aray  = pairArrayG[nrow] ;
if( status == OLD ) {
    pin_x = tearrayG[pin]->newx ;
} else {
    pin_x = tearrayG[pin]->xpos ;
}
cellptr   = carrayG[ Aray[1] ] ;
most_left = cellptr->cxcenter + cellptr->tileptr->left ;
cellptr   = carrayG[ Aray[Aray[0]] ] ;
most_rite = cellptr->cxcenter + cellptr->tileptr->right ;
if( pin_x < most_left || pin_x > most_rite ) {
    return( 0 ) ;
}
bin = ( pin_x - most_left ) / cedge_binwidthS + 1 ;
if( ( i = cedgebinS[nrow][bin] ) == 0 ) {
    return( 0 ) ;
}
cellptr =  carrayG[ Aray[i] ] ;
if( ( ( cellptr->tileptr->left + cellptr->cxcenter ) == pin_x )
					    && ( bin >= 2 ) ) {
    if( carrayG[ Aray[i-1] ]->cheight > cellptr->cheight ) {
	cellptr = carrayG[ Aray[--i] ] ;
    }
} else {
    crite = cellptr->tileptr->right + cellptr->cxcenter ;
    while(  crite < pin_x && i < Aray[0] ) {
	cellptr = carrayG[ Aray[++i] ] ;
	crite += cellptr->clength ;
    }
    if( crite == pin_x && i != Aray[0] ) {
	if( carrayG[ Aray[i+1] ]->cheight > cellptr->cheight ) {
	    cellptr = carrayG[ Aray[i+1] ] ;
	}
    }
}

if( 1 <= row  && row <= numRowsG ) {
    if( pinloc == TOPCELL ) {
	return( -cellptr->tileptr->bottom ) ;
    } else {
	return( cellptr->tileptr->top ) ;
    }
} else if( row == 0 ) {
    return( -cellptr->tileptr->bottom ) ;
} else {
    return( cellptr->tileptr->top ) ;
}
}

fcellheight( pin , fcell , status )
INT pin , *fcell , status ;
{

CBOXPTR cellptr ;
INT bin , i , *Aray , most_left , most_rite ;
INT nrow , pin_x , crite , row , pinloc ;

row = tearrayG[pin]->row ;
pinloc = tearrayG[pin]->pinloc ;
if( 1 <= row  && row <= numRowsG ) {
    nrow = row + pinloc - PINLOC_OFFSET ;
    if( nrow == 0 || nrow == numChansG || pinloc == NEITHER ) {
	*fcell = 0 ;
	return(0) ;
    }
} else if( row == 0 ) {
    nrow = 1 ;
} else {
    nrow = numRowsG ;
}

Aray  = pairArrayG[nrow] ;
if( status == OLD ) {
    pin_x = tearrayG[pin]->newx ;
} else {
    pin_x = tearrayG[pin]->xpos ;
}
cellptr   = carrayG[ Aray[1] ] ;
most_left = cellptr->cxcenter + cellptr->tileptr->left ;
cellptr   = carrayG[ Aray[Aray[0]] ] ;
most_rite = cellptr->cxcenter + cellptr->tileptr->right ;
if( pin_x < most_left || pin_x > most_rite ) {
    *fcell = 0 ;
    return( 0 ) ;
}
bin = ( pin_x - most_left ) / cedge_binwidthS + 1 ;
if( ( i = cedgebinS[nrow][bin] ) == 0 ) {
    *fcell = 0 ;
    return( 0 ) ;
}
*fcell = Aray[i] ;
cellptr =  carrayG[ *fcell ] ;
crite = cellptr->tileptr->right + cellptr->cxcenter ;
while(  crite < pin_x && i < Aray[0] ) {
    *fcell = Aray[++i] ;
    cellptr = carrayG[ *fcell ] ;
    crite += cellptr->clength ;
}
if( crite == pin_x && i != Aray[0] ) {
    if( carrayG[ Aray[i+1] ]->cheight > cellptr->cheight ) {
	*fcell = Aray[i+1] ;
	cellptr = carrayG[ *fcell ] ;
    }
}

if( 1 <= row  && row <= numRowsG ) {
    if( pinloc == TOPCELL ) {
	return( -cellptr->tileptr->bottom ) ;
    } else {
	return( cellptr->tileptr->top ) ;
    }
} else if( row == 0 ) {
    return( -cellptr->tileptr->bottom ) ;
} else {
    return( cellptr->tileptr->top ) ;
}
}
