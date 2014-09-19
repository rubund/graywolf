/* ----------------------------------------------------------------- 
"@(#) temp.h (Yale) version 3.4 2/4/91"
FILE:	    temp.c                                       
DESCRIPTION:definitions for temperature control
CONTENTS:   
DATE:	    Mar 01, 1989 
REVISIONS:  Mon Feb  4 02:21:54 EST 1991 - added MINTUPDATE def.
----------------------------------------------------------------- */
#ifndef TEMP_H
#define TEMP_H

#define RATIOUPPERBOUND  0.15     /* control to find Tratio */
#define RATIOLOWERBOUND  0.05     /* control to find Tratio */
#define LOWTEMPRATIO     0.05     /* low temp anneal acceptance ratio */
#define EPSILON          0.000001 /* error tolerance */
#define DEFAULTTEMP      5.0      /* default temp for low temp anneal */
#define FINISHED         0.99999  /* round to 1.0 we are finished */

/* Definitions for heuristic temperature scheduler */

/* ----------------------------------------------------------------- 
    The temperature regions are defined as follows:
    Iterations                  Purpose
    -2 - 0                      Initialization
    1 - HIGHTEMP                High temperature regime exponential decay
    HIGHTEMP+1 - MEDTEMP        Critical temp. 0.44 - range limiter
    MEDTEMP+1 - LASTTEMP        Low temperature regime exponential decay.
----------------------------------------------------------------------- */
#define INIT_ITERATION    -2    /* initialization iteration */
#define HIGHTEMP         23.00  /* last iteration of high temp regime */
#define MEDTEMP          81.00  /* last iteration of range limiter */
#define LOWTEMP         125.0   /* temperature @ 6% acceptance rate */
#define TURNOFFT        125.0   /* iteration of neg. feddback turnoff */
#define LASTTEMP        155.00  /* last iteration */
#define CRITRATIO         0.44  /* critical ratio */
#define LOWRATIO          0.06  /* temperature for controler turn off */
#define ACCEPTDAMPFACTOR  0.025 /* damping factor for acceptance rate*/
#define ACCEPTDAMPFACTOR2 0.25  /* damping factor for low temp's */
#define NUMTUPDATES     400     /* max number of T updates per iteration*/
#define MINTUPDATE      14      /* mininum # moves before update */

#endif /* TEMP_H */
