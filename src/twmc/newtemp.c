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
FILE:	    newtemp.c                                       
DESCRIPTION:update temparature of the simulated annealing algorithm.
CONTENTS:  double calc_acceptance_ratio( iteration )
		double iteration ;
	    init_acceptance_rate()
	    int compute_attprcel() 
	    set_tw_speed( speed ) 
		double speed ;
DATE:	    Dec 19, 1988 
REVISIONS:  Feb 25, 1989 - allow negative iterations by setting 
		iteration to zero.
	    Feb 28, 1989 - updated schedule constants.
	    Mar 02, 1989 - moved schedule constants to temp.h
	    Mar 07, 1989 - added compute_attprcel.
	    Mar 13, 1989 - added set_tw_speed for attempts per cell
		modification.
	    Mon Feb  4 02:14:30 EST 1991 - reset the number of attempts
		and added quickroute function.
----------------------------------------------------------------- */
#include "allheaders.h"

double alphaS ;    /* exponential decay constant for high temp */
double betaS ;     /* exponential decay constant for low temp */
double speedS ;    /* multiply attempts per cell by this factor */

/* calculate static exponential time constants */
void init_acceptance_rate()
{
    /* determine alpha */
    alphaS =  - log( CRITRATIO ) / HIGHTEMP ;

    /* determine beta */
    betaS =  - log( ( LOWRATIO/CRITRATIO ) ) / (LOWTEMP - MEDTEMP ) ;
} /* end init_acceptance_rate */

/* given an iteration number return desired acceptance rate */
double calc_acceptance_ratio( double iteration )
{
    double desired_ratio ;

    if( iteration < 0.0 ){ /* allow negative iterations */
	iteration = 0.0 ;
    }
    if( iteration <= HIGHTEMP ){
	/* -------------------------------------------------------- 
	    desired_ratio = exp( - alphaS * iteration ) ;
	   -------------------------------------------------------- */
	desired_ratio = iteration ;
	desired_ratio *= - alphaS ;
	desired_ratio = exp( desired_ratio ) ;

    } else if( iteration <= MEDTEMP ){
	   desired_ratio = CRITRATIO ;

    } else if( iteration > LOWTEMP ){ /* last iterations */
	if( iteration < LASTTEMP ) {
	    desired_ratio = LOWRATIO - 
		    ( (LOWRATIO / (LASTTEMP - LOWTEMP)) *
					    (iteration - LOWTEMP) ) ;
	} else {
	    desired_ratio = 0.0 ;
	}

    } else {  /* low temp */
	/* -------------------------------------------------------- 
	    desired_ratio = 0.44 * exp( - betaS * ( iteration - MEDTEMP ))
	   -------------------------------------------------------- */
	desired_ratio = iteration - MEDTEMP ;
	desired_ratio *= - betaS ;
	desired_ratio = CRITRATIO * exp( desired_ratio ) ;
    }
    return( desired_ratio ) ;
} /* end calc_acceptance ratio */

int compute_attprcel() 
{
    double attempts ;

    /*  n to the 4/3 power  */
    attempts = 25.0 * pow( (double) endsuperG, 4.0 / 3.0 ) ;

    if( quickrouteG ){
	attempts /= 10.0 ;
    } else {

	/* modify the attempts per cell based on user input */
	attempts *= speedS ;
    }
    if( attempts < (double) MINTUPDATE ){
	/* guarantee at least one temperature update */
	attempts = (double) MINTUPDATE + 2.0 ;
    }

    return((int) attempts) ;

} /* end compute_attprcell */ 

void set_tw_speed( double speed ) 
{
    speedS = speed ;
} /* end set_tw_speed */
