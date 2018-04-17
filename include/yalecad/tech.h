/* ----------------------------------------------------------------- 
FILE:	    tech.h                                       
DESCRIPTION:include file for technology lookup.
DATE:	    Oct 13, 1990 
REVISIONS:  
----------------------------------------------------------------- */
#ifndef YTECH_H
#define YTECH_H

#ifndef lint
static char YassignId[] = "@(#) tech.h (Yale) version 1.1 10/22/90" ;
#endif

#include <yalecad/base.h>

#define TECH_CHARACTERISTIC	(DOUBLE) 1000.0

#define LAYER1		"layer1"
#define LAYER2		"layer2"
#define LAYER3		"layer3"
#define VIA1_2		"via1/2"
#define VIA2_3		"via2/3"


extern Ytech_init( P1(char *designName ) ) ;
/*
Function:
    Read a technology file for a given design.  Must be called 
    first before using other technology functions.
*/


extern INT Ytech_spacing( P2(char *object1, char *object2 ) ) ;
/*
Function:
    Returns the spacing between the two given objects.
    If it does not exist it defaults to zero.
*/

extern INT Ytech_width( P1(char *object) ) ;
/*
Function:
    Returns the width of the given object.
    If it does not exist it defaults to zero.
*/

extern INT Ytech_pitch( P1(char *object) ) ;
/*
Function:
    Convenience function that returns the pitch of a given object.
    It returns the sum of width and spacing for that object.
*/

#endif /* YTECH_H */



