/* ----------------------------------------------------------------- 
FILE:	    newtemp.c                                       
DESCRIPTION:update temparature of the simulated annealing algorithm.
CONTENTS:  DOUBLE calc_acceptance_ratio( iteration )
		DOUBLE iteration ;
	    init_acceptance_rate()
	    INT compute_attprcel() 
	    set_tw_speed( speed ) 
		DOUBLE speed ;
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
#ifndef lint
static char SccsId[] = "@(#) newtemp.c version 3.4 2/4/91" ;
#endif

#include <custom.h>
#include <temp.h>
#undef DEBUG
#include <yalecad/debug.h>


static DOUBLE alphaS ;    /* exponential decay constant for high temp */
static DOUBLE betaS ;     /* exponential decay constant for low temp */
static DOUBLE speedS ;    /* multiply attempts per cell by this factor */

/* calculate static exponential time constants */
init_acceptance_rate()
{
    /* determine alpha */
    alphaS =  - log( CRITRATIO ) / HIGHTEMP ;

    /* determine beta */
    betaS =  - log( ( LOWRATIO/CRITRATIO ) ) / (LOWTEMP - MEDTEMP ) ;
} /* end init_acceptance_rate */

/* given an iteration number return desired acceptance rate */
DOUBLE calc_acceptance_ratio( iteration )
DOUBLE iteration ;
{
    DOUBLE desired_ratio ;

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

INT compute_attprcel() 
{
    DOUBLE attempts ;

    /*  n to the 4/3 power  */
    attempts = 25.0 * pow( (DOUBLE) endsuperG, 4.0 / 3.0 ) ;

    if( quickrouteG ){
	attempts /= 10.0 ;
    } else {

	/* modify the attempts per cell based on user input */
	attempts *= speedS ;
    }
    if( attempts < (DOUBLE) MINTUPDATE ){
	/* guarantee at least one temperature update */
	attempts = (DOUBLE) MINTUPDATE + 2.0 ;
    }

    return((INT) attempts) ;

} /* end compute_attprcell */ 

set_tw_speed( speed ) 
DOUBLE speed ;
{
    speedS = speed ;
} /* end set_tw_speed */

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
    for( i= 0; i<= 155; i++ ){
	d_ratio = calc_acceptance_ratio( (DOUBLE) i ) ;
	printf( "%4.2le\n" , d_ratio ) ;
    }

} /* end main */
#endif /* TESTRATIO */
