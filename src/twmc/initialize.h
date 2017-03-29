/* ----------------------------------------------------------------- 
FILE:	    initialize.h
DESCRIPTION:Header file for initialize.c
CONTENTS:   
DATE:	    March 15, 1990 
REVISIONS:  Wed Dec 19 19:38:46 EST 1990 - added analog pin type.
----------------------------------------------------------------- */
/* *****************************************************************
   static char SccsId[] = "@(#) initialize.h version 3.4 12/19/90" ;
***************************************************************** */
#ifndef INITIALIZE_H
#define INITIALIZE_H

#define PINGROUPTYPE     1
#define HARDPINTYPE      2
#define SOFTPINTYPE      3
#define SOFTEQUIVTYPE    4
#define HARDEQUIVTYPE    5
#define ADDEQUIVTYPE     6
#define ANALOGPINTYPE    7

/* below is what we expect to be a large floorplanning input */
/* user may change parameters if they wish. Subject to change */
#define EXPECTEDNUMCELLS   500
#define EXPECTEDNUMNETS   1000
#define EXPECTEDNUMUNCON  100
#define EXPECTEDNUMPADS   10
#define EXPECTEDCORNERS   8
#define EXPECTEDINSTANCES 1
#define EXPECTEDPINGRP    1

#endif /* INITIALIZE_H */
