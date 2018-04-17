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
FILE:	    urcost.c                                       
DESCRIPTION:update global routing cost.
CONTENTS:   urcost( segptr )
		SEGBOXPTR segptr ;
DATE:	    Mar 27, 1989 
REVISIONS:  Tue Mar 19 16:22:56 CST 1991 - fixed crash when
		there are no routing tracks in a channel.
----------------------------------------------------------------- */
#ifndef VMS
#ifndef lint
static char SccsId[] = "@(#) urcost.c (Yale) version 4.4 3/19/91" ;
#endif
#endif

#include "standard.h"
#include "groute.h"
#include <yalecad/debug.h>

urcost( segptr )
SEGBOXPTR segptr ;
{

CHANGRDPTR aptr1 , aptr2 , bptr1 , bptr2 , ptr ;
DENSITYPTR denptr , headptr ;
INT penalty , check ;
INT x , achannel , bchannel , aMaxVal , bMaxVal ;
INT maxaa , maxbb , pin1 , pin2 ;
INT aoutside , binside ;
INT ax1 , ax2 , bx1 , bx2 ;
INT track ;

penalty = 0 ;
pin1 = segptr->pin1ptr->terminal ;
pin2 = segptr->pin2ptr->terminal ;
if( segptr->switchvalue == swUP ) {
    aptr1 = TgridG[pin1]->up ;
    aptr2 = TgridG[pin2]->up ;
    bptr1 = TgridG[pin1]->down ;
    bptr2 = TgridG[pin2]->down ;
    bchannel = segptr->pin1ptr->row ;
    achannel = bchannel + 1 ;
} else {
    aptr1 = TgridG[pin1]->down ;
    aptr2 = TgridG[pin2]->down ;
    bptr1 = TgridG[pin1]->up ;
    bptr2 = TgridG[pin2]->up ;
    achannel = segptr->pin1ptr->row ;
    bchannel = achannel + 1 ;
}

aMaxVal = maxTrackG[achannel] ;
bMaxVal = maxTrackG[bchannel] ;
ax1 = aptr1->netptr->xpos ;
ax2 = aptr2->netptr->xpos ;
bx1 = bptr1->netptr->xpos ;
bx2 = bptr2->netptr->xpos ;

aoutside = 0 ;
denptr = DboxHeadG[ achannel ][ aMaxVal ]->next ;
for( ; denptr != DENSENULL ; denptr = denptr->next ) {
    x = denptr->grdptr->netptr->xpos ;
    if( !( ax1 <= x && ax2 >= x ) || denptr->grdptr->cross > 1 ) {
	aoutside = 1 ;
	break ;
    }
}
if( aoutside == 0 ) {
    penalty-- ;
}

binside  = 0 ;
denptr = DboxHeadG[ bchannel ][ bMaxVal ]->next ;
for( ; denptr != DENSENULL ; denptr = denptr->next ) {
    x = denptr->grdptr->netptr->xpos ;
    if( bx1 <= x && bx2 >= x && denptr->grdptr->cross == 0 ) {
	binside = 1 ;
	break ;
    }
}
if( binside == 1 ) {
    penalty++ ;
}

while( aptr1->prevgrd ) {
    if( aptr1->prevgrd->netptr->xpos == ax1 ) {
	aptr1 = aptr1->prevgrd ;
    } else {
	break ;
    }
}
while( aptr2->nextgrd ) {
    if( aptr2->nextgrd->netptr->xpos == ax2 ) {
	aptr2 = aptr2->nextgrd ;
    } else {
	break ;
    }
}
while( bptr1->prevgrd ) {
    if( bptr1->prevgrd->netptr->xpos == bx1 ) {
	bptr1 = bptr1->prevgrd ;
    } else {
	break ;
    }
}
while( bptr2->nextgrd ) {
    if( bptr2->nextgrd->netptr->xpos == bx2 ) {
	bptr2 = bptr2->nextgrd ;
    } else {
	break ;
    }
}
aptr2 = aptr2->nextgrd ;
bptr2 = bptr2->nextgrd ;
if( penalty == 0 ) {
    if( binside == 1 && aoutside == 0 ) {
	/* check = (aMaxVal - 1) - (bMaxVal + 1) ; */
	check = ABS(aMaxVal - bMaxVal - 2) - ABS(bMaxVal - aMaxVal) ;
    } else {
	maxaa = 0 ;
	maxbb = 0 ;
	for( ptr = aptr1 ; ptr != aptr2 ; ptr = ptr->nextgrd ) {
	    if( ptr->tracks > maxaa ) {
		maxaa = ptr->tracks ;
	    }
	}
	for( ptr = bptr1 ; ptr != bptr2 ; ptr = ptr->nextgrd ) {
	    if( ptr->tracks > maxbb ) {
		maxbb = ptr->tracks ;
	    }
	}
	/* maxaa = aMaxVal  - maxaa + 1 ; */
	/* maxbb = bMaxVal  - maxbb - 1 ; */
	/* check = maxaa - maxbb ; */
	check = ABS(aMaxVal - maxaa - bMaxVal + maxbb + 2) - 
			    ABS(aMaxVal - maxaa - bMaxVal + maxbb) ;
    }
} else {
    check = penalty ;
}

if( check <= 0 ) {
    for( ptr = aptr1 ; ptr != aptr2 ; ptr = ptr->nextgrd ) {
	denptr = ptr->dptr ;
	if( ptr->cross == 1 ) {
	    ptr->cross = 0 ;
	    if( denptr->next != DENSENULL ) {
		denptr->next->back = denptr->back ;
	    }
	    denptr->back->next = denptr->next ;
	    track = --(ptr->tracks) ;
	    if( track == -1 ){
		track = 0 ;
		D( "twsc/urcost",
		    fprintf( stderr, " track less than 1 reset to 0\n" ) ;
		) ;
	    }

	    headptr = DboxHeadG[ achannel ][ track ]->next ;
	    if( headptr != DENSENULL ) {
		DboxHeadG[ achannel ][ track ]->next = denptr ;
		denptr->next  = headptr ;
		headptr->back = denptr  ;
		denptr->back  = DboxHeadG[ achannel ][ track ] ;
	    } else {
		DboxHeadG[ achannel ][ track ]->next = denptr ;
		denptr->next = DENSENULL ;
		denptr->back = DboxHeadG[ achannel ][ track ];
	    }
	} else {
	    ptr->cross-- ;
	}
    }
    if( aoutside == 0 ) {
	maxTrackG[achannel]-- ;
    }
    for( ptr = bptr1 ; ptr != bptr2 ; ptr = ptr->nextgrd ) {
	denptr = ptr->dptr ;
	if( ptr->cross == 0 ) {
	    ptr->cross = 1 ;
	    if( denptr->next != DENSENULL ) {
		denptr->next->back = denptr->back ;
	    }
	    denptr->back->next = denptr->next ;
	    track = ++(ptr->tracks) ;

	    headptr = DboxHeadG[ bchannel ][ track ]->next ;
	    if( headptr != DENSENULL ) {
		DboxHeadG[ bchannel ][ track ]->next = denptr ;
		denptr->next  = headptr ;
		headptr->back = denptr  ;
		denptr->back  = DboxHeadG[ bchannel ][ track ] ;
	    } else {
		DboxHeadG[ bchannel ][ track ]->next = denptr ;
		denptr->next = DENSENULL ;
		denptr->back = DboxHeadG[ bchannel ][ track ];
	    }
	} else {
	    ptr->cross++ ;
	}
    }
    if( binside == 1 ) {
	maxTrackG[bchannel]++ ;
    }
    if( segptr->switchvalue == swUP ) {
	segptr->switchvalue = swDOWN ;
    } else {
	segptr->switchvalue = swUP ;
    }

    tracksG += penalty ;
    return (1) ;
} else {
    return (0) ;
}
}
