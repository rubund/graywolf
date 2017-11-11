/* ----------------------------------------------------------------- 
FILE:	    tech.h                                       
DESCRIPTION:include file for technology lookup.
DATE:	    Oct 13, 1990 
REVISIONS:  
----------------------------------------------------------------- */
#ifndef YTECH_H
#define YTECH_H

#define TECH_CHARACTERISTIC	(DOUBLE) 1000.0

#define LAYER1		"layer1"
#define LAYER2		"layer2"
#define LAYER3		"layer3"
#define VIA1_2		"via1/2"
#define VIA2_3		"via2/3"


void Ytech_init( P1(char *designName ) ) ;
/*
Function:
    Read a technology file for a given design.  Must be called 
    first before using other technology functions.
*/


int Ytech_spacing( P2(char *object1, char *object2 ) ) ;
/*
Function:
    Returns the spacing between the two given objects.
    If it does not exist it defaults to zero.
*/

int Ytech_width( P1(char *object) ) ;
/*
Function:
    Returns the width of the given object.
    If it does not exist it defaults to zero.
*/

int Ytech_pitch( P1(char *object) ) ;
/*
Function:
    Convenience function that returns the pitch of a given object.
    It returns the sum of width and spacing for that object.
*/

#endif /* YTECH_H */



