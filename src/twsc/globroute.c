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
FILE:	    globroute.c                                       
DESCRIPTION:global routing functions.
CONTENTS:   globroute()
	    process_cross( segptr , status )
		INT status ;
		SEGBOXPTR segptr ;
DATE:	    Mar 27, 1989 
REVISIONS:  Sat Dec 15 22:08:21 EST 1990 - modified pinloc values
		so that it will always be positive.
----------------------------------------------------------------- */
#ifndef VMS
#ifndef lint
static char SccsId[] = "@(#) globroute.c (Yale) version 4.5 12/15/90" ;
#endif
#endif

#include "standard.h"
#include "groute.h"
#include "main.h"

/* global variables */
extern INT Max_numPinsG ;

#define TOP 1
#define BOT 0

globroute()
{

INT flips , attempts , net ;
INT pick , number , attlimit ;
INT found , trys , maxtrys ;
INT x , x1 , x2 , channel ;
SEGBOXPTR segptr , *segment ;
DENSITYPTR denptr ;
PINBOXPTR netptr1 , netptr2 ;


changrid( ) ;

pre_findrcost( ) ;

findrcost() ;

fprintf(fpoG,"\n\nTHIS IS THE ORIGINAL NUMBER OF TRACKS: %d\n\n\n" , 
							tracksG ) ;
fflush(fpoG);

flips    =  0 ;
attempts =  0 ;
attlimit = 25 * numnetsG ;
segment  = ( SEGBOXPTR *)Ysafe_malloc(
	( Max_numPinsG + 1 ) * sizeof( SEGBOXPTR ) ) ;

while( ++attempts < attlimit ) {
    if( attempts % 1000 == 0 ) {
	printf(" tracks = %3d at attempts = %5d\n" ,tracksG ,attempts ) ;
    }
    do {
	net = (INT) ( (DOUBLE) numnetsG * ( (DOUBLE) RAND / 
				      (DOUBLE) 0x7fffffff ) ) + 1 ;
    } while( net == numnetsG + 1 ) ;
    number = 0 ;
    for( segptr = netsegHeadG[net]->next ; segptr ;
			    segptr = segptr->next ){
	process_cross( segptr , 1 ) ;
	if( segptr->switchvalue != nswLINE ) {
	    segment[ ++number ] = segptr ;
	}
    }
    maxtrys = 4 * number ;
    trys = 0 ;
    while( ++trys <= maxtrys ) {
	do {
	    pick = (INT) ( (DOUBLE) number * ( (DOUBLE) RAND / 
					  (DOUBLE) 0x7fffffff ) ) + 1 ;
	} while( pick == number + 1 ) ;
	segptr = segment[ pick ] ;
	netptr1 = segptr->pin1ptr ;
	netptr2 = segptr->pin2ptr ;
	if( segptr->switchvalue == swUP ) {
	    channel = netptr1->row + 1 ;
	} else {
	    channel = netptr2->row ;
	}
	x1 = netptr1->xpos ;
	x2 = netptr2->xpos ;

	found = NO ;
	for( denptr = DboxHeadG[ channel ][ maxTrackG[channel] ]->next
		    ; denptr != DENSENULL ; denptr = denptr->next ) {
	    x = denptr->grdptr->netptr->xpos ;
	    if( x1 <= x && x2 >= x ) {
		found = YES ;
		break ;
	    }
	}
	if( !found ) {
	    continue ;
	}

	if( urcost( segptr ) ) {
	    flips++    ;
	}
    }
    for( segptr = netsegHeadG[net]->next ; segptr ;
			    segptr = segptr->next ){
	process_cross( segptr , 0 ) ;
    }
}
fprintf(fpoG,"no. of accepted flips: %d\n", flips ) ;
fprintf(fpoG,"no. of attempted flips: %d\n", attempts ) ;
fprintf(fpoG,"THIS IS THE NUMBER OF TRACKS: %d\n\n\n" , tracksG ) ;
fflush(fpoG);

return ;
}

process_cross( segptr , status )
INT status ;
SEGBOXPTR segptr ;
{

INT x1 , x2 ;
PINBOXPTR pin1ptr , pin2ptr ;
CHANGRDPTR ptr1 , ptr2 , ptr ;

pin1ptr = segptr->pin1ptr ;
pin2ptr = segptr->pin2ptr ;
x1 = pin1ptr->xpos ;
x2 = pin2ptr->xpos ;
if( x1 == x2 ) {
    return ;
}

if( pin1ptr->row < pin2ptr->row ) {
    ptr1 = TgridG[ pin1ptr->terminal ]->up ;
    ptr2 = TgridG[ pin2ptr->terminal ]->down ;
} else if( pin1ptr->row > pin2ptr->row ) {
    ptr1 = TgridG[ pin1ptr->terminal ]->down ;
    ptr2 = TgridG[ pin2ptr->terminal ]->up ;
} else if( segptr->switchvalue == swUP ) {
    ptr1 = TgridG[ pin1ptr->terminal ]->up ;
    ptr2 = TgridG[ pin2ptr->terminal ]->up ;
} else if( segptr->switchvalue == swDOWN ) {
    ptr1 = TgridG[ pin1ptr->terminal ]->down ;
    ptr2 = TgridG[ pin2ptr->terminal ]->down ;
} else if( pin1ptr->pinloc >= NEITHER && pin2ptr->pinloc >= NEITHER ) {
    ptr1 = TgridG[ pin1ptr->terminal ]->up ;
    ptr2 = TgridG[ pin2ptr->terminal ]->up ;
} else {
    ptr1 = TgridG[ pin1ptr->terminal ]->down ;
    ptr2 = TgridG[ pin2ptr->terminal ]->down ;
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
if( status == 1 ) {
    for( ptr = ptr1 ; ptr != ptr2 ; ptr = ptr->nextgrd ) {
	ptr->cross++ ;
    }
} else {
    for( ptr = ptr1 ; ptr != ptr2 ; ptr = ptr->nextgrd ) {
	ptr->cross = 0 ;
    }
}
}
