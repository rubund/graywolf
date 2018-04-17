/* ----------------------------------------------------------------- 
FILE:	    set.h                                       
DESCRIPTION:Insert file for the set utility routines.  The set
members of the set are integers in a range [lowerLimitt..upperLimit]. 
The routines add & subtract members from the set efficiently.  
Multiple sets may be handled simultaneously.  The complexity of the 
routines are as follows.
    Yset_init      : O(n)
    Yset_free      : O(n)
    Yset_member    : O(1)
    Yset_add   t   : O(1)
    Yset_enumerate : O(1)
    Yset_empty     : O(1)
    Yset_size      : O(1)
The Yset_empty gets its speed by incrementing the value of in_set.  Since
all tests for membership check a match of this value, all members of set 
can be deleted at once. 
NOTE: These set routines are best suited for applications where you need
to enumerate the members of the set and the set is bounded by an integer.
In other applications, they may not be as suitable.
CONTENTS:   Typedefs and definitions for set routines.
DATE:	    Apr 10, 1989 
REVISIONS:  Dec  4, 1989 added Yset_size and made Yenumerate a macro.
	    Apr 18, 1990 - now Yset_init allocate the set.
	    Apr 19, 1990 - added YcompSet 
	    Fri Feb 15 15:39:18 EST 1991 - renamed set functions.
----------------------------------------------------------------- */
#ifndef SET_H
#define SET_H

#ifndef lint
static char YsetId[] = "@(#) set.h (Yale) version 1.7 2/15/91" ;
#endif

#include <yalecad/base.h>

typedef struct ysetlist {
    INT              member ;/* Determining membership by finding match */
    INT              node ;  /* name of node */
    struct ysetlist  *next ; /* set is a threaded array */
    struct ysetlist  *prev ;
} YSETLISTBOX, *YSETLISTPTR ;

typedef struct {
    INT  size ;              /* size of set */
    INT  cardinality ;       /* number of members in the set */
    INT  in_set ;            /* match for determining membership */
    INT  lowerLimit ;        /* lower bound for set */
    INT  upperLimit ;        /* upper bound for set */
    YSETLISTPTR list ;       /* a list enumerating members of set */
    YSETLISTPTR *set ;       /* set is an array of YLISTBOXes */
} YSET, *YSETPTR ;

#define SETNULL (YSETLISTPTR) 0

/* set macro for finding size of the set */
#define Yset_size(x_xz)    (x_xz->cardinality)
/* 
Arguments:
    YSETPTR set ;
Function:
    returns the cardinality of the set at current time.
*/

/* set macro for enumeration return the first item in the list of set */
#define Yset_enumerate(x_xz)  (x_xz->list )
/* 
Arguments:
    YSETPTR set ;
Function:
    returns the head of a list of nodes in the set.  Use node record
    field as an indication of the element.
*/

extern YSETPTR Yset_init( P2(INT lowerLimit, INT upperLimit ) ) ;
/* 
Arguments:
    INT lowerLimit, upperLimit ;
Function:
    initialize set.  lowerLimit and upperLimit specify set boundaries.
    returns a pointer to the set data structure.
*/

extern Yset_free( P1(YSETPTR set ) ) ; 
/* 
Arguments:
    YSETPTR set ;
Function:
    free set previously allocated - set is pointer to a set record
    allocated by YinitSet.
*/

extern BOOL Yset_member( P2(YSETPTR set, INT node ) ) ;
/* 
Arguments:
    YSETPTR set ;
    INT node ;
Function:
    tests whether node is member of set.  Returns true if member;otherwise
    it returns false.
*/

extern BOOL Yset_add( P2(YSETPTR set, INT node ) ) ;
/* 
Arguments:
    YSETPTR set ;
    INT node ;
Function:
    add node as member of set.  
    returns TRUE if this a new member of set FALSE if already a member.
*/

extern Yset_delete( P2(YSETPTR set, INT node ) ) ;
/* 
Arguments:
    YSETPTR set ;
    INT node ;
Function:
    removes node from set.  
*/


extern Yset_empty( P1(YSETPTR set ) ) ; 
/* 
Arguments:
    YSETPTR set ;
Function:
    makes the set an empty set.
*/

extern Yset_comp( P1(YSETPTR set ) ) ;
/* 
Arguments:
    YSETPTR set ;
Function:
    takes the complement of the given set.
*/

#endif /* SET_H */
