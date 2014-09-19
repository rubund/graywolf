/* ----------------------------------------------------------------- 
FILE:	    dset.h                                       
DESCRIPTION:include file for disjoint set utility functions 
CONTENTS:   
DATE:	    Apr 26, 1991 
REVISIONS:  Sun Dec  8 23:30:16 EST 1991 - removed indirection 
		from the comparison functions.
	    Sun Dec 15 02:44:47 EST 1991 - now store sub/superset
		tree with the set itself.
----------------------------------------------------------------- */
#ifndef YDSET_H
#define YDSET_H

#ifndef lint
static char Ydset_h_SccsId[] = "@(#) dset.h version 1.6 3/28/92";
#endif

#include <yalecad/base.h>
#include <yalecad/rbtree.h>

typedef struct ydsetrec {
    YTREEPTR dtree ;            /* rbtree holding sets */
    INT (*compare_func)() ;	/* how to compare functions in rbtree */
    VOID (*user_delete)() ;	/* how to delete functions in rbtree */
    YTREEPTR superset_tree ;	/* tree to store the superset */
    YTREEPTR subset_tree ;	/* tree to store the subsets */
    YTREEPTR parent_tree ;	/* tree to store the parents */
} YDSETBOX, *YDSETPTR ;

/*---------------------------------------
  Ydset_init()
  NOTE that the users comparison function will be similar to 
  comparison function used in the rbtree package.
  ---------------------------------------*/
extern YDSETPTR Ydset_init( P1(INT (*compare_func)() ) ) ;
/*
Function:
    Initialize the union/find routines.  Returns back a set.
    A set of sets is implemented.  Therefore, many union find
    data structures may be used at once.  A comparison function
    must be supplied to sort the data.
    See rbtree.h for more details about comparison functions.
*/

/*---------------------------------------
  Ydset_empty
  Free all elements in the set but leaves the set intact
  This may be used recursively.
  ---------------------------------------*/
extern VOID Ydset_empty( P2(YDSETPTR set,VOID (*userDelete)() ) );
/*
  free all elements of a superset.  Function userDelete is applied to user data.
*/

/*---------------------------------------
  ---------------------------------------*/
extern VOIDPTR Ydset_enumerate( P2(YDSETPTR set, BOOL startFlag));
/*
Function:
    Enumerate all of the elements of the super set
*/
     
/*---------------------------------------
  ---------------------------------------*/
extern VOIDPTR Ydset_enumerate_superset( P2(YDSETPTR set, BOOL startFlag));
/*
Function:
    Enumerate all of the elements of the super set
*/

extern VOIDPTR Ydset_enumerate_parents( P2(YDSETPTR dset, BOOL startFlag) ) ;
/*
Function:
    Enumerate the parents of the super set
*/
     
/*---------------------------------------
  ---------------------------------------*/
extern VOIDPTR Ydset_enumerate_subset( P3 (YDSETPTR set,VOIDPTR subsetData,
					BOOL startFlag));
/*
Function:
  Find the set which subsetData is an element of.  Enumerate all
  the elements of that set.
*/

/*---------------------------------------
  ---------------------------------------*/
extern VOIDPTR Ydset_find( P2(YDSETPTR dset, VOIDPTR data ) ) ;
/*
Function:
    Returns subset name for the given data of the given set.  If
    data does not exist, it is created and put in set.
*/

/*-------------------------------------------------
  Ydset_find_set()
  Searches for the set an item belongs to.
  This routine avoids makeset of the item
  which Ydset_find does by default
  -------------------------------------------------*/
VOIDPTR Ydset_find_set( P2( YDSETPTR dset, VOIDPTR data ));

/*---------------------------------------
  Ydset_free
  Free all elements in the set and the set.
  This can be used recursively.
  ---------------------------------------*/
extern VOID Ydset_free( P2(YDSETPTR set,VOID (*userDelete)() ) );
/*
  free the entire superset.  Function userDelete is applied to user data.
*/

/*---------------------------------------
  ---------------------------------------*/
extern VOIDPTR Ydset_union( P3(YDSETPTR set, VOIDPTR x, VOIDPTR y ) ) ;
/*
Function:
    Perform union operation on two data items for the given set.
    If either data item does not exist, the item is created 
    and put in set.
    Returns the subset name.
*/

/*---------------------------------------
  ---------------------------------------*/
extern VOIDPTR Ydset_search( P2(YDSETPTR set,VOIDPTR data));
/*
    Search for an elment in the super set
*/

/*---------------------------------------
  ---------------------------------------*/
INT Ydset_subset_size( P2(YDSETPTR set, VOIDPTR data));
/*
    returns the size of the subset data is an element of.
*/

/*---------------------------------------
  ---------------------------------------*/
extern INT Ydset_superset_size(P1(YDSETPTR set));
/*
  returns the size of the entire superset
*/

/*-----------------------
  Ydset_verify
  -----------------------*/
extern INT Ydset_verify( P1(YDSETPTR set));

/*------------------------
  Ydset_dump
  ------------------------*/
extern Ydset_dump(P2( YDSETPTR set, VOID (*printFunc)() ) );

#endif /* YDSET_H */

