/* ----------------------------------------------------------------- 
FILE:	    buster.h
DESCRIPTION:Include file for the utility routine to break a 
	    rectilinear cell up into tiles.
CONTENTS:   YBUSTBOXPTR Ybuster()
		BUSTBOXPTR A , R ;
	    INT Ybuster_init() ;
	    INT Ybuster_addpt( x, y ) ;
		INT x, y ;
DATE:	    Aug  7, 1988 - rewrote to match new parser.
REVISIONS:  May  1, 1990 - made sure we cannot match the 0 
		record in the redundancy check for points.
	    May 4, 1990  - moved to the library since it now occurs
		in many files.
	    Thu Apr 18 00:49:05 EDT 1991 - rewrote buster_verify.
----------------------------------------------------------------- */
#ifndef YBUSTER_H
#define YBUSTER_H

#ifndef lint
static char YbusterId[] = "@(#) buster.h version 1.5 4/18/91" ;
#endif

#include <yalecad/base.h>

typedef struct ybustbox {
    INT x ;
    INT y ;
} YBUSTBOX , *YBUSTBOXPTR ;


extern Ybuster_init() ;
/* 
Arguments:
    none
Function:
    Initialize the buster routines.  Must be called first before
    any points are added using Ybuster_addpt.
    Call this routine before the start of every object to be busted
    into tiles.
*/

extern Ybuster_addpt( P2( INT x, INT y ) ) ;
/* 
Arguments:
    INT x, y ;
Function:
    Add a point to the current arbitrary figure.
*/

extern YBUSTBOXPTR Ybuster() ;
/* 
Arguments:
    none
Function:
    Returns a tile of the arbitary figure.  Call this function until
    it returns null to receive all the tiles of an arbitrary rectilinear
    shaped figure.  The returning pointer list the four points of the
    tile starting at the point llx, lly and proceeding CW.  
    To access say the urx,ury point use 
	ptr[3].x, ptr[3].y if ptr is the returned pointer.
*/

extern Ybuster_free() ;
/* 
Arguments:
    none
Function:
    Free up the memory associated with buster.
*/

extern BOOL Ybuster_verify( P1(char *object_name) ) ;
/* 
Arguments:
    char *object_name
Function:
    Sanity checker.  Returns TRUE if things look reasonable.  It 
    returns FALSE otherwise and outputs and error message to the
    screen.  The object_name is a message string which is output
    on an error to describe the current object to be busted.
    This makes for better user messages.
*/

#endif /* YBUSTER_H */
