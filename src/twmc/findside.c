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
FILE:	    findside.c
DESCRIPTION:This file contains the routines to find what side the
	    pins are located.
CONTENTS:   INT findside( pSideArray, cellptr , x , y )
		PSIDEBOX  *pSideArray ;
		CELLBOXPTR cellptr ;
		INT x , y ;
	    loadside( pSideArray, side , factor )
		PSIDEBOX  *pSideArray ;
		INT side ;
		DOUBLE factor ;
DATE:	    Aug  7, 1988 
REVISIONS:  Apr 23, 1990 - added error checking for soft pins and
		add them correctly to the PSIDE array.
	    Thu Jan 17 00:51:01 PST 1991 - fixed bug in the case
		of T shaped cells. Now finds correct side.
	    Wed Feb 13 23:35:07 EST 1991 - renamed set routines.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) findside.c version 3.5 2/13/91" ;
#endif

#include <custom.h>
#include <initialize.h>
#include <yalecad/debug.h>
#include <yalecad/set.h>

#define HOWMANY 0

/* returns which side a pin is located */
INT findside( pSideArray, cellptr , x , y )
PSIDEBOX  *pSideArray ;
CELLBOXPTR cellptr ;
INT x , y ;
{

INT k , min , kmin, dist, begin, end ;
INT xc, yc, b, e ;
INT *yvert, *xvert ;

min = INT_MAX ;
kmin = 1 ; /* default side */
xc = cellptr->xcenter ;
yc = cellptr->ycenter ;
xvert = cellptr->vertices->x ;
yvert = cellptr->vertices->y ;
for( k = 1 ; k <= cellptr->numsides ; k++ ) {
    if( pSideArray[k].vertical == 1 ) {
	dist = ABS(x - pSideArray[k].position) ;
	b = yvert[k] + yc ;
	if( k == cellptr->numsides ){
	    e = yvert[1] + yc ;
	} else {
	    e = yvert[k+1] + yc ;
	}
	begin = MIN( b, e ) ;
	end   = MAX( b, e ) ;

	if( begin <= y && y <= end && dist < min ){ 
	    min = ABS(x - pSideArray[k].position) ;
	    kmin = k ;
	}
    } else {
	dist = ABS(y - pSideArray[k].position) ;
	b = xvert[k] + xc ;
	if( k == cellptr->numsides ){
	    e = xvert[1] + xc ;
	} else {
	    e = xvert[k+1] + xc ;
	}
	begin = MIN( b, e ) ;
	end   = MAX( b, e ) ;

	if( begin <= x && x <= end && dist < min ){ 
	    min = ABS(y - pSideArray[k].position) ;
	    kmin = k ;
	}
    }
}
return( kmin ) ;

} /* end findside */


/* load the side with a pin count and add factor */
loadside( pSideArray, side , factor )
PSIDEBOX  *pSideArray ;
INT side ;
DOUBLE factor ;
{

pSideArray[side].pincount += factor ;

return ;
} /* end loadside */


load_soft_pins( ptr, pSideArray )
CELLBOXPTR ptr ;
PSIDEBOX *pSideArray ;
{

    INT i ;                        /* counter */
    INT j ;                        /* counter */
    INT howmany ;                  /* number of softpins for cell */
    INT restrict1 ;                 /* counter */
    INT match ;                    /* count number of matching restricts*/
    INT numsides ;                 /* number of side restrictions */
    INT numrestricts ;             /* current # restricts for child */
    INT parent_restricts ;         /* current # restricts for parent */
    BOOL abortPin ;                /* abort load if true */
    PINBOXPTR pin ;                /* current pin */
    PINBOXPTR parent ;             /* parent to current pin */
    SOFTBOXPTR spin ;              /* soft information for current pin */
    YSETPTR side_set ;             /* set of valid sides for pin */
    YSETLISTPTR curside ;          /* used to enumerate the set */

    if(!(ptr->softpins)){
	return ;
    }

    side_set = Yset_init( 1, ptr->numsides ) ;
    howmany = (INT) ptr->softpins[HOWMANY] ;
    for( i = 1 ; i <= howmany; i++ ){

	/**** DETERMINE THE VALID SIDES FOR A SOFTPIN ****/
	pin = ptr->softpins[i] ;
	spin = pin->softinfo ;

	if( pin->type == HARDPINTYPE || pin->type == PINGROUPTYPE ){
	    continue ;
	}

	/* now remove restrictions if present */
	numrestricts = spin->restrict1[HOWMANY] ;

	/* initialize set to the empty set */
	Yset_empty( side_set ) ;

	if( numrestricts == 0 ){
	    /* everybody is valid */
	    for( j = 1; j <= ptr->numsides; j++ ){
		Yset_add( side_set, j ) ;
	    }
	} else {
	    /* just validate the given list of sides */
	    for( restrict1 = 1; restrict1 <= numrestricts; restrict1++ ){
		Yset_add( side_set, spin->restrict1[restrict1] ) ;
	    }
	}
	/* at this point we have checked the leaf, check parents for */
	/* consistency if pin is not a softequiv type */
	parent = spin->parent ;
	abortPin = FALSE ;
	while( parent && pin->type != SOFTEQUIVTYPE ){
	    spin = parent->softinfo ;
	    parent_restricts = spin->restrict1[HOWMANY] ;

	    /* check the various cases of number of restrictions */
	    if( numrestricts != 0 && parent_restricts == 0 ){
		/* we have an inconsistency */
		sprintf( YmsgG, "Pin:%s has inconsistent restrictions\n",
		    pin->pinname ) ;
		M( ERRMSG, "load_soft_pins", YmsgG ) ;
		abortPin = TRUE ;
	    } else if( numrestricts != 0 && parent_restricts != 0 ){
		/* need to make sure that parent is more restrictive */
		/* than child */
		match = 0 ; /* initially no matching restrictions */
		for( restrict1 = 1;restrict1<=parent_restricts;restrict1++ ){
		    if( Yset_member( side_set, spin->restrict1[restrict1] ) ){
			match++ ;
		    } else {
		        Yset_add( side_set, spin->restrict1[restrict1] ) ;
		    }
		}
		if( match != numrestricts ){
		    sprintf( YmsgG, 
			"Pin:%s has inconsistent restrictions\n",
			pin->pinname ) ;
		    M( ERRMSG, "load_soft_pins", YmsgG ) ;
		    abortPin = TRUE ;
		}
	    } else if( numrestricts == 0 && parent_restricts != 0 ){
		Yset_comp( side_set ) ;
		for( restrict1 = 1;restrict1<=parent_restricts;restrict1++ ){
		    Yset_add( side_set, spin->restrict1[restrict1] ) ;
		}
	    } /* else if numrestricts == 0 && parent_restricts == 0 ) */
	    /* don't have to do anything for this case */

	    /* now prepare for possible next level of hierarchy */
	    numrestricts = parent_restricts ;
	    parent = spin->parent ;
	}

	if( abortPin ){
	    continue ;
	}

	/* now enumerate the set of valid sides for the pin */
	numsides = Yset_size( side_set ) ;
	if( numsides == 0 ){
	    sprintf( YmsgG, "Pin:%s has no valid side\n", pin->pinname ) ;
	    M( ERRMSG, "load_soft_pins", YmsgG ) ;
	    continue ;
	}

	for(curside=Yset_enumerate(side_set);curside;curside=curside->next ){
	    loadside( pSideArray, curside->node, 1.0 / numsides ) ;
	}
    }
    Yset_free( side_set ) ;
} /* end load_soft_pins */
