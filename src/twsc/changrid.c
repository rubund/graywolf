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
FILE:	    changrid.c                                       
DESCRIPTION:channel gridding code.
CONTENTS:   changrid( )
	    pre_findrcost()
DATE:	    Mar 27, 1989 
REVISIONS:  Sat Dec 15 22:08:21 EST 1990 - modified pinloc values
		so that it will always be positive.
	    Wed Aug 28 14:27:04 EDT 1991 - added more debug.
----------------------------------------------------------------- */
#ifndef VMS
#ifndef lint
static char SccsId[] = "@(#) changrid.c (Yale) version 4.5 8/29/91" ;
#endif
#endif

#include "standard.h"
#include "groute.h"

changrid( )
{

CHANGRDPTR **gdptr , grdptr , cgdptr , ngdptr ;
PINBOXPTR netptr ;
INT row , net , channel , terminal ;
INT *numPins , *PinInChan ;
INT count , i, k, comparegdx() ;


numPins   = (INT *)Ysafe_calloc( numChansG+1, sizeof(INT) ) ;
PinInChan = (INT *)Ysafe_calloc( numChansG+1, sizeof(INT) ) ;
for( net = 1 ; net <= numnetsG ; net++ ) {
    for( netptr = netarrayG[net]->pins ; netptr ; netptr = netptr->next ) {
	if( (INT) netptr->pinloc == BOTCELL ) {
	    numPins[ netptr->row ]++ ;
	} else if( (INT) netptr->pinloc == TOPCELL ) {
	    numPins[ netptr->row + 1 ]++ ;
	} else {  /* pinloc == 0  pin on pads or macros */
	    if( netptr->row > 0 ) {
		numPins[ netptr->row ]++ ;
	    }
	    if( netptr->row < numChansG ) {
		numPins[ netptr->row + 1 ]++ ;
	    }
	}
    }
}

gdptr = (CHANGRDPTR **)Ysafe_malloc( (numChansG+1) *
				sizeof(CHANGRDPTR *) ) ;
for( channel = 1 ; channel <= numChansG ; channel++ ) {
    gdptr[channel] = ( CHANGRDPTR * )Ysafe_malloc(
	( numPins[channel] + 1 ) * sizeof( CHANGRDPTR ) ) ;
}

k = maxtermG + 2 * numChansG ;
TgridG = ( TGRIDPTR * )Ysafe_malloc( ( k+1 ) * sizeof( TGRIDPTR ) ) ;
for( terminal = 1 ; terminal <= k ; terminal++ ) {
    TgridG[ terminal ] = ( TGRIDPTR )Ysafe_malloc( sizeof(TGRIDBOX) ) ;
}
for( net = 1 ; net <= numnetsG ; net++ ) {
    for( netptr = netarrayG[net]->pins ;netptr;netptr=netptr->next ) {
	row = netptr->row ;
	terminal = netptr->terminal ;
#ifdef DEBUG
    if( tearrayG[ terminal ] == PINNULL ) {
	printf(" netptr for pin = %8d is NULL; net:%d row:%d\n" , 
			terminal , net , row ) ;
	fflush(stdout) ;
    }
#endif
	grdptr = ( CHANGRDPTR )Ysafe_calloc( 1, sizeof(CHANGRDBOX)) ;
	if( (INT) netptr->pinloc == BOTCELL ) {
	    channel = row ;
	    count = ++(PinInChan[ channel ]) ;
	    gdptr[ channel ][ count ] = grdptr ;
	    TgridG[ terminal ]->up = grdptr ;
	    TgridG[ terminal ]->down = grdptr ;
	    grdptr->netptr  = tearrayG[ terminal ] ;
	} else if( (INT) netptr->pinloc == TOPCELL ) {
	    channel = row + 1 ;
	    count = ++(PinInChan[ channel ]) ;
	    gdptr[ channel ][ count ] = grdptr ;
	    TgridG[ terminal ]->up = grdptr ;
	    TgridG[ terminal ]->down = grdptr ;
	    grdptr->netptr  = tearrayG[ terminal ] ;
	} else {  /* pinloc == NEITHER  pin on pads or macros */
	    if( 1 <= row && row <= numChansG - 1 ) {
		channel = row ;
		count = ++(PinInChan[ channel ]) ;
		gdptr[ channel ][ count ] = grdptr ;
		TgridG[ terminal ]->down   = grdptr ;
		grdptr->netptr  = tearrayG[ terminal ] ;

		channel = row + 1 ;
		count = ++(PinInChan[ channel ]) ;
		grdptr = gdptr[ channel ][ count ] =
		 ( CHANGRDPTR )Ysafe_calloc( 1,sizeof(CHANGRDBOX)) ;
		TgridG[ terminal ]->up = grdptr ;
		grdptr->netptr  = tearrayG[ terminal ] ;
	    } else if( row == 0 ) {
		count = ++(PinInChan[ 1 ]) ;
		gdptr[ 1 ][ count ] = grdptr ;
		TgridG[ terminal ]->up   = grdptr ;
		TgridG[ terminal ]->down = grdptr ;
		grdptr->netptr  = tearrayG[ terminal ] ;
	    } else {            /* row == numChansG */
		count = ++(PinInChan[ numChansG ]) ;
		gdptr[ numChansG ][ count ] = grdptr ;
		TgridG[ terminal ]->up   = grdptr ;
		TgridG[ terminal ]->down = grdptr ;
		grdptr->netptr  = tearrayG[ terminal ] ;
	    }
	}
    }
}

for( channel = 1 ; channel <= numChansG ; channel++ ) {
    Yquicksort( (char *) ( gdptr[channel] + 1 ) , numPins[channel] ,
	sizeof( CHANGRDPTR ), comparegdx ) ;
}

BeginG = ( CHANGRDPTR * )Ysafe_calloc( numChansG + 1, 
		    sizeof(CHANGRDPTR ) ) ;
EndG   = ( CHANGRDPTR * )Ysafe_calloc( numChansG + 1,
		    sizeof(CHANGRDPTR ) ) ;
for( channel = 1 ; channel <= numChansG ; channel++ ) {
    BeginG[channel] = ( CHANGRDPTR )Ysafe_calloc( 1,
		    sizeof(CHANGRDBOX) ) ;
    BeginG[channel]->netptr = ( PINBOXPTR )Ysafe_calloc(1,sizeof(PINBOX));
    BeginG[channel]->netptr->xpos = gxstartG - 1 ;
    BeginG[channel]->netptr->terminal = maxtermG + channel ;
    BeginG[channel]->netptr->row = channel ;
    BeginG[channel]->netptr->pinloc = BOTCELL ;
    tearrayG[maxtermG + channel] = BeginG[channel]->netptr ;

    EndG[channel] = ( CHANGRDPTR )Ysafe_calloc( 1,sizeof(CHANGRDBOX)) ;
    EndG[channel]->netptr = ( PINBOXPTR )Ysafe_calloc( 1,sizeof(PINBOX)) ;
    EndG[channel]->netptr->xpos = gxstopG + 1 ;
    EndG[channel]->netptr->terminal = maxtermG + numChansG + channel ;
    EndG[channel]->netptr->row = channel ;
    EndG[channel]->netptr->pinloc = BOTCELL ;
    tearrayG[maxtermG + numChansG + channel] = EndG[channel]->netptr ;
}
for( channel = 1 ; channel <= numChansG ; channel++ ) {
    TgridG[ maxtermG + channel ]->up   = BeginG[ channel ] ;
    TgridG[ maxtermG + channel ]->down = BeginG[ channel ] ;
    TgridG[ maxtermG + numChansG + channel ]->up   = EndG[ channel ] ;
    TgridG[ maxtermG + numChansG + channel ]->down = EndG[ channel ] ;
}

for( channel = 1 ; channel <= numChansG ; channel++ ) {
    if( numPins[channel] == 0 ) {
	BeginG[channel]->nextgrd = NULL ;
	EndG[channel]->prevgrd   = NULL ;
    } else if( numPins[channel] == 1 ) {
	BeginG[channel]->nextgrd = gdptr[channel][1] ;
	gdptr[channel][1]->prevgrd  = BeginG[channel] ;
	EndG[channel]->prevgrd   = gdptr[channel][1] ;
	gdptr[channel][1]->nextgrd  = EndG[channel] ;
    } else {
	BeginG[channel]->nextgrd     = gdptr[channel][1] ;
	gdptr[channel][1]->prevgrd  = BeginG[channel] ;
	for( i = 1 ; i <= numPins[channel]-1 ; i++ ) {
	    cgdptr = gdptr[channel][i]   ;
	    ngdptr = gdptr[channel][i+1] ;
	    cgdptr->nextgrd   = ngdptr ;
	    ngdptr->prevgrd   = cgdptr ;
	}
	ngdptr->nextgrd  = EndG[channel] ;
	EndG[channel]->prevgrd  = ngdptr ;
    }
}
for( channel = 1 ; channel <= numChansG ; channel++ ) {
    Ysafe_free( gdptr[ channel ] ) ;
}
Ysafe_free( gdptr ) ;

Ysafe_free( numPins ) ;
Ysafe_free( PinInChan ) ;
}


pre_findrcost()
{

SEGBOXPTR segptr ;
PINBOXPTR ptr1 , ptr2 ;
INT net ;

for( net = 1 ; net <= numnetsG ; net++ ) {
    for( segptr = netsegHeadG[net]->next ;
	segptr ; segptr = segptr->next ) {
	ptr1 = segptr->pin1ptr ;
	ptr2 = segptr->pin2ptr ;
	if( ABS( ptr1->row - ptr2->row ) > 1 ) {
	    segptr->switchvalue = nswLINE ;
	    if( ptr1->xpos < blk_most_leftG ) {
		if( ptr1->row > ptr2->row ) {
		    segptr->pin1ptr =
			tearrayG[ maxtermG + ptr2->row + 1 ] ;
		} else {
		    segptr->pin1ptr =
			tearrayG[ maxtermG + ptr2->row ] ;
		}
	    } else if( ptr2->xpos > blk_most_riteG ) {
		if( ptr2->row > ptr1->row ) {
		    segptr->pin2ptr =
			tearrayG[ maxtermG + numChansG + ptr1->row + 1 ] ;
		} else {
		    segptr->pin2ptr =
			tearrayG[ maxtermG + numChansG + ptr1->row ] ;
		}
	    } else {
		printf("segment other than connecting pin has row") ;
		printf(" difference greater than 1\n" ) ;
	    }
	}
    }
}
}
