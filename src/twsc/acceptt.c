/*
 *   Copyright (C) 1988-1992 Yale University
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
FILE:	    acceptt.c                                       
DESCRIPTION:accept routine used in simulated annealing
CONTENTS:   acceptt( INT  )
DATE:	    Jan 30, 1988 
REVISIONS:
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) acceptt.c version 4.7 4/2/92" ;
#endif

#include <yalecad/base.h>
#include <yalecad/debug.h>
#include "main.h"
#include "standard.h"

#define MASK 0x3ff

static DOUBLE table1S[1024] , table2S[1024] , table3S[1024] ;
 
init_table()
{
    INT i2 ;
    table1S[0] = 1.0 ;
    table2S[0] = 1.0 ;
    table3S[0] = 1.0 ;
    for( i2 = 1 ; i2 <= 1023 ; i2++ ) {
	table1S[ i2 ] = exp( -(DOUBLE) i2 / 8.0 ) ;
	table2S[ i2 ] = exp( -(DOUBLE) i2 / 8192.0 ) ;
	table3S[ i2 ] = exp( -(DOUBLE) i2 / 8388608.0 ) ;
    }
}

#ifdef DEBUG_CODE
static FILE *fpS = NULL ;
BOOL acceptt( d_wire, d_time, d_penal )
INT d_wire, d_time, d_penal ;
{
    BOOL truth ;
    INT time ;
    extern INT aG ;

    time = d_time ;
    truth = acceptt2( d_wire, d_time, d_penal ) ;
    if(!fpS){
	fpS = fopen( "newcost.dat", "w" ) ;
    }
    fprintf( fpS, "delta_cost:%10d w:%10d t:%10d p:%10d accept:%d cell:%d\n", 
	d_costG, d_wire, time, d_penal, truth, aG ) ;
    fflush( fpS ) ;
    return( truth ) ;
    
}
#endif /* DEBUG_CODE */

/* -----------------------------------------------------------------
    Acceptance function for the annealing algorithm.  We expect all
    deltas to be of the form (old_cost - new_cost). If new_cost is
    less than the old_cost, the quantity will be positive and always
    acceptted.  The variable d_costG lets the statistic accumulation
    code know of the change.
----------------------------------------------------------------- */
BOOL acceptt( d_wire, d_time, d_penal )
INT d_wire, d_time, d_penal ;
{

    DOUBLE fred ;
    register unsigned fract ;

    /* d_time = (INT) ( 10.0 * timeFactorG * (DOUBLE) d_time ) ; */
    d_time = (INT) ( timeFactorG * (DOUBLE) d_time ) ;
    d_costG = d_wire + d_time + d_penal ; 
    if( d_costG >= 0 ){
	return( TRUE ) ;
    } else {
	fred = ((DOUBLE) d_costG ) / TG ; 
    }

    /* Now we are left with the uphill moves */
    if( fred < -80.0 ) {
	return( FALSE ) ;
    } else if( fred > -0.0001 ) {
	if( 1.0 + fred > ( (DOUBLE) RAND / (DOUBLE)0x7fffffff ) ) { 
	    return( TRUE ) ;
	} else {
	    return( FALSE ) ;
	}
    } else {
	fract = (INT)( -fred * 8388608.0 ) ;
	if( (table1S[ (fract >> 20) & MASK ] * 
			table2S[ (fract >> 10) & MASK] * 
			table3S[ fract & MASK ]) > 
			( (DOUBLE) RAND / (DOUBLE)0x7fffffff ) ) {
	    return( TRUE ) ;
	} else {
	    return( FALSE ) ;
	}
    }
} /* end acceptt() */


/* -----------------------------------------------------------------
    Greedy acceptance function for the annealing algorithm.  We expect
    all deltas to be of the form (old_cost - new_cost). If new_cost is
    less than the old_cost, the quantity will be positive and always
    acceptted.  
----------------------------------------------------------------- */
BOOL accept_greedy( d_wire, d_time, d_penal )
INT d_wire, d_time, d_penal ;
{

    if( d_time == 0 ){
	/* in the case the delta time is zero, use wirelength */
	d_costG = d_wire + d_penal ; 
	if( d_costG >= 0 ){
	    return( TRUE ) ;
	} else {
	    return( FALSE ) ;
	}
    } else {
	/* If we have a timing constraint, use it */
	d_costG = d_time + d_penal ; 
	if( d_costG >= 0 ){
	    return( TRUE ) ;
	} else {
	    return( FALSE ) ;
	}
    }
} /* end accept_greedy() */
