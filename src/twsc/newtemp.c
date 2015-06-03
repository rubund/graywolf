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
FILE:	    newtemp.c                                       
DESCRIPTION:update temparature of the simulated annealing algorithm.
CONTENTS:  DOUBLE calc_acceptance_ratio( iternum )
		DOUBLE iternum ;
	    init_acceptance_rate()
DATE:	    Dec 19, 1988 
REVISIONS:  
----------------------------------------------------------------- */
#ifndef VMS
#ifndef lint
static char SccsId[] = "@(#) newtemp.c (Yale) version 4.3 9/7/90" ;
#endif
#endif

#include "standard.h"
#include <yalecad/debug.h>

/* ----------------------------------------------------------------- 
    The temperature regions are defined as follows:
    Iterations                  Purpose
    -1 - 0                      Initialization
    1 - HIGHTEMP                High temperature regime exponential decay
    HIGHTEMP+1 - MEDTEMP        Critical temp. 0.44 - range limiter
    MEDTEMP+1 - LASTTEMP        Low temperature regime exponential decay.
    Note: If you change definitions here, check uloop.c
----------------------------------------------------------------------- */
#define HIGHTEMP        23.00 /* (15.0) last iteration of high temp regime */
#define MEDTEMP         81.00 /* (52.0) last iteration of range limiter */
#define LOWTEMP        125.00 /* (100.0) temperature @ 6% acceptance rate */
#define LASTTEMP       155.00 /* last iteration */
#define CRITRATIO        0.44 /* critical ratio */
#define LOWRATIO         0.06 /* temperature for controler turn off */

static DOUBLE alphaS ;    /* exponential decay constant for high temp */
static DOUBLE betaS ;     /* exponential decay constant for low temp */

/* calculate static exponential time constants */
init_acceptance_rate()
{
    /* determine alpha */
    alphaS =  - log( CRITRATIO ) / HIGHTEMP ;

    /* determine beta */
    betaS =  - log( ( LOWRATIO/CRITRATIO ) ) / (LOWTEMP - MEDTEMP ) ;
} /* end init_acceptance_rate */

/* given an iteration number return desired acceptance rate */
DOUBLE calc_acceptance_ratio( iternum )
DOUBLE iternum ;
{
    DOUBLE desired_ratio ;

    if( iternum <= HIGHTEMP ){
	/* -------------------------------------------------------- 
	    desired_ratio = exp( - alphaS * iternum ) ;
	   -------------------------------------------------------- */
	desired_ratio = iternum ;
	desired_ratio *= - alphaS ;
	desired_ratio = exp( desired_ratio ) ;

    } else if( iternum <= MEDTEMP ){
	desired_ratio = CRITRATIO ;

    } else if( iternum > LOWTEMP ){
	if( iternum < LASTTEMP ) {
	    desired_ratio = LOWRATIO - 
		    ( (LOWRATIO / (LASTTEMP - LOWTEMP)) *
					    (iternum - LOWTEMP) ) ;
	} else {
	    desired_ratio = 0.0 ;
	}
    } else {  /* low temp */
	/* -------------------------------------------------------- 
	    desired_ratio = 0.44 * exp( - betaS * ( iternum - MEDTEMP ))
	   -------------------------------------------------------- */
	desired_ratio = iternum - MEDTEMP ;
	desired_ratio *= - betaS ;
	desired_ratio = CRITRATIO * exp( desired_ratio ) ;
    }
    return( desired_ratio ) ;
} /* end calc_acceptance ratio */


/* #define TESTRATIO */
#ifdef TESTRATIO

/* test program for desired acceptance rate profile */
main( argc , argv )
INT argc ;
char *argv[] ;
{
    DOUBLE d_ratio, calc_acceptance_rate() ;
    INT i ;

    init_acceptance_rate() ;
    for( i= 0; i<= 150; i++ ){
	d_ratio = calc_acceptance_ratio( (DOUBLE) i ) ;
	printf( "%4.2le\n" , d_ratio ) ;
    }

} /* end main */
#endif /* TESTRATIO */
