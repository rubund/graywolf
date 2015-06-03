/*
 *   Copyright (C) 1988-1990 Yale University
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
static char SccsId[] = "@(#) acceptt.c version 3.3 9/5/90" ;
#endif

#include <custom.h>
#include <yalecad/debug.h>

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
INT acceptt( delta_cost )
INT delta_cost ;
{

INT truth ;
DOUBLE fred ;
register unsigned fract ;

d_costG = delta_cost ; /*** make it known to the world. JL ***/
fred =  ((DOUBLE) delta_cost ) / TG ; 

if( fred >= 0.0 ) {
    truth = 1 ;
} else if( fred < -80.0 ) {
    truth = 0 ;
} else if( fred > -0.0001 ) {
    if( 1.0 + fred > ( (DOUBLE) RAND / (DOUBLE)0x7fffffff ) ) { 
	truth = 1 ;
    } else {
	truth = 0 ;
    }
} else {
    fract = (INT)( -fred * 8388608.0 ) ;
    if( (table1S[ (fract >> 20) & MASK ] * 
		    table2S[ (fract >> 10) & MASK] * 
		    table3S[ fract & MASK ]) > 
		    ( (DOUBLE) RAND / (DOUBLE)0x7fffffff ) ) {
	truth = 1 ;
    } else {
	truth = 0 ;
    }
}

return(truth) ; 
}


