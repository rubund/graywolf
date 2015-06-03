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
FILE:	    setpwates.c                                       
DESCRIPTION:Adde pin weights to all the tiles for wirelength estimation.
CONTENTS:   setpwates()
DATE:	    Apr  2, 1989 - added header and instance code.
REVISIONS:  May 14, 1989 - set weights to zero for stdcell case.
	    Mon Feb  4 02:20:39 EST 1991 - updated for new wire estimator
	    NOTE that standard cell weights are -2 as a flag.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) setpwates.c version 3.4 2/4/91" ;
#endif

#include <custom.h>
#include <yalecad/debug.h>

setpwates()
{

TILEBOXPTR tileptr ;
CELLBOXPTR cptr ;
INSTBOXPTR instptr ;
INT cell ;
INT i ;
INT numinstances ;

for( cell = 1 ; cell <= numcellsG ; cell++ ) {
    cptr = cellarrayG[cell] ;
    if( instptr = cptr->instptr ){
	numinstances = instptr->numinstances ;
    } else {
	numinstances = 1 ;
    }
    for( i=0; i < numinstances; i++ ){
	if( instptr ){
	    tileptr=instptr->tile_inst[i] ;
	} else {
	    tileptr=cptr->tiles ;
	}
	if( cptr->celltype == STDCELLTYPE ){
	    if( new_wire_estG ){
		for( ;tileptr;tileptr=tileptr->next ){
		    /* no area estimation for stdcells */
		    /* make the weight negative as a flag in wireestxy2 */
		    tileptr->lweight = -2.0 ;
		    tileptr->rweight = -2.0 ;
		    tileptr->bweight = -2.0 ;
		    tileptr->tweight = -2.0 ;
		} /* end for loop */
	    } else {
		for( ;tileptr;tileptr=tileptr->next ){
		    /* no area estimation for stdcells */
		    tileptr->lweight = 0.0 ;
		    tileptr->rweight = 0.0 ;
		    tileptr->bweight = 0.0 ;
		    tileptr->tweight = 0.0 ;
		} /* end for loop */
	    }

	} else if( new_wire_estG ){
	    /* in this case these weights have been set in watesides */
	    continue ;
	} else {
	    for( ;tileptr;tileptr=tileptr->next ){
		if( tileptr->lweight > pinsPerLenG ) {
		    tileptr->lweight /= pinsPerLenG ;
		} else {
		    tileptr->lweight = 1.0 ;
		}
		if( tileptr->rweight > pinsPerLenG ) {
		    tileptr->rweight /= pinsPerLenG ;
		} else {
		    tileptr->rweight = 1.0 ;
		}
		if( tileptr->bweight > pinsPerLenG ) {
		    tileptr->bweight /= pinsPerLenG ;
		} else {
		    tileptr->bweight = 1.0 ;
		}
		if( tileptr->tweight > pinsPerLenG ) {
		    tileptr->tweight /= pinsPerLenG ;
		} else {
		    tileptr->tweight = 1.0 ;
		}
	    }  /* end for loop */
	} /* end test of celltype */
    } /* end instance loop */
}
return ;

} /* end setpwates */
