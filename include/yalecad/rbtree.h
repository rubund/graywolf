/* ----------------------------------------------------------------- 
FILE:	    rbtree.h                                       
DESCRIPTION:Tree include file for binary and red-black trees.
    The same functionality as binary trees but better performance
    for trees > 100 elements.
CONTENTS:   
DATE:	    Mar 30, 1990 
REVISIONS:  Oct  8, 1990 - added prototypes and tree size.
	    Oct 22, 1990 - changed prototypes for rbtree_free.
            Apr 19, 1991 - added Yrbtree_verify for debugging -RAW
	    Mon Aug 12 15:49:34 CDT 1991 - got rid of offset
		argument to rbtree.
	    Sun Nov  3 12:55:27 EST 1991 - added missing function
		definitions: Yrbtree_search_...
	    Fri Feb  7 16:29:58 EST 1992 - added Yrbtree_interval_size.
----------------------------------------------------------------- */
#ifndef RBTREE_H
#define RBTREE_H

#ifndef lint
static char YrbtreeId[] = "@(#) rbtree.h version 1.20 5/22/92" ;
#endif

#include <yalecad/base.h>

#ifndef YTREE_H_DEFS
typedef struct tree YTREEBOX ;
typedef struct tree *YTREEPTR ;
#endif

/* macro definition for tree structure see Yrbtree_init */
#define YRBTREE_INIT( tree_xz, compare_xz ) \
{ \
    tree_xz = Yrbtree_init( compare_xz ) ; \
}

/* THE DEFINITIONS FOR SEARCH CLOSEST FUNCTIONALITY */
#define SEARCH_CLOSEST_NOP	0
#define	SEARCH_CLOSEST_BELOW	1
#define	SEARCH_CLOSEST_ABOVE	2
#define	SEARCH_CLOSEST		3

/* ******************* BINARY TREE INCLUDE FILES ****************** */
extern YTREEPTR Yrbtree_init( P1(INT (*compare_func)() ) ) ;
/* 
Arguments:
    INT compare_func() ;
Function:
    Initializes a binary tree.  The user should supply a compare
    comparison function similar to the one used by the UNIX 
    quicksort routine.  The function compare_func is assumed to have
    two arguments which are pointers to the arguments being compared.
    The function should return an integer less than, equal to, or
    greater than 0 according as the first argument is to be considered
    less than, equal to , or greater than the second.  
    the distance in bytes that the key is offset from the beginning
    of the data record.  The function returns a pointer to the
    tree data structure. Use the tree pointer returned by Yrbtree_init 
    for all routines to work on this tree.  The comparison function
    no longer needs to be unique.  See Yrbtree_deleteCurrentEnumerate
    and Yrbtree_deleteCurrentInterval for more details.
*/


extern VOIDPTR Yrbtree_search( P2(YTREEPTR tree, VOIDPTR key ) ) ;
/* 
Arguments:
    YTREEPTR tree ;
    VOIDPTR key ;
Function:
    Given a binary tree data structure, it return the a pointer
    to the stored data object that matches the given key. It 
    returns NULL if no match is found.  Sets the search marker
    for subsequent search_suc and search_pred calls.
*/

extern VOIDPTR Yrbtree_min( P1(YTREEPTR tree) ) ;
/* 
Arguments:
    YTREEPTR tree ;
Function:
    Given a binary tree data structure, it return the a pointer
    to the minimum data object stored in the tree.  It returns 
    NULL if nothing is in the tree. Sets the search marker for
    subsequent search_suc and search_pred calls.
*/

extern VOIDPTR Yrbtree_max( P1(YTREEPTR tree ) ) ;
/* 
Arguments:
    YTREEPTR tree ;
Function:
    Given a binary tree data structure, it return the a pointer
    to the maximum data object stored in the tree.  It returns 
    NULL if nothing is in the tree.  Sets the search marker for
    subsequent search_suc and search_pred calls.
*/

extern VOIDPTR Yrbtree_suc( P2(YTREEPTR tree, VOIDPTR key ) ) ;
/* 
Arguments:
    YTREEPTR tree ;
    VOIDPTR key ;
Function:
    Given a binary tree data structure, it return the a pointer
    to the successor to the given key stored in the tree.  It returns 
    NULL if nothing is in the tree or if no match to the key is found.
*/

extern VOID Yrbtree_insert( P2(YTREEPTR tree, VOIDPTR data ) ) ;
/* 
Arguments:
    YTREEPTR tree ;
    VOIDPTR data ;
Function:
    Insert an element into the tree.  Data is a pointer to the users
    record to be store in the tree.  Each record must contain a key
    to sort upon.
*/

extern VOIDPTR Yrbtree_enumerate( P2(YTREEPTR tree, BOOL startFlag) ) ;
/* 
Arguments:
    YTREEPTR tree ;
Function:
    Enumerate all the data in a tree.  First time call with startFlag 
    set to TRUE to get first element in tree starting at the minimum
    element.  For all subsequent calls, pass a FALSE argument to 
    get all the remaining members of the tree.  Returns a pointer
    to the user record.  Returns NULL when all elements have been
    requested or when no match can be found.
*/

extern VOID Yrbtree_enumeratePush( P1(YTREEPTR tree) ) ;
/* 
Arguments:
    YTREEPTR tree ;
Function:
    Push the current enumeratation posistion pointer on to a 
    stack.  This is useful if the user wishes to recursively
    enumerate trees.
*/

extern VOID Yrbtree_enumeratePop( P1(YTREEPTR tree) ) ;
/* 
Arguments:
    YTREEPTR tree ;
Function:
    Pop the current enumeratation posistion pointer off of the
    stack.  This is useful if the user wishes to recursively
    enumerate trees.
*/

extern VOIDPTR Yrbtree_interval( P4(YTREEPTR tree, VOIDPTR low_key,
				VOIDPTR high_key, BOOL startFlag ) ) ;
/* 
Arguments:
    YTREEPTR tree ;
    VOIDPTR low_key, *high_key ;
    BOOL startFlag;
Function:
    Enumerate all the data in a tree between low_key and high_key. 
    First time call with startFlag=TRUE to get first element in 
    tree >= the low_key.  For all subsequent calls, pass
    startFlag=FALSE to get all the remaining members of the tree. 
    Returns NULL when element > high_key or no match can be found.
*/

extern INT Yrbtree_interval_size( P3(YTREEPTR tree,VOIDPTR low_key,
				    VOIDPTR high_key) ) ;
/* 
Arguments:
    YTREEPTR tree ;
    VOIDPTR low_key, *high_key ;
Function:
    Given an interval describe by the low and high keys, it returns the
    number of elements in that interval.
*/

extern VOID Yrbtree_intervalPush( P1(YTREEPTR tree) ) ;
/* 
Arguments:
    YTREEPTR tree ;
Function:
    Push the current interval posistion pointer on to a 
    stack.  This is useful if the user wishes to recursively
    enumerate trees.
*/

extern VOID Yrbtree_intervalPop( P1(YTREEPTR tree) ) ;
/* 
Arguments:
    YTREEPTR tree ;
Function:
    Pop the current interval posistion pointer on to a 
    stack.  This is useful if the user wishes to recursively
    enumerate trees.
*/

extern VOID Yrbtree_interval_free( P4(YTREEPTR tree, VOIDPTR low_key,
				VOIDPTR high_key, VOID (*userDelete)()) );
/* 
Arguments:
    YTREEPTR tree ;
    VOIDPTR low_key, *high_key ;
    BOOL startFlag;
Function:
    Enumerate all the data in a tree between low_key and high_key. 
    All nodes between low_key and high_key are deleted from the tree.
    The data can be freed by specifying the userDelete parameter.    
    See below for an example.
*/

extern BOOL Yrbtree_delete( P3(YTREEPTR tree,  VOIDPTR key, VOID (*userDelete)() ) ) ;
/* 
Arguments:
    YTREEPTR tree ;
    VOIDPTR key ;
Function:
    Delete a node in the tree by using the key. If userDelete !=0,
    the user delete function supplied is executed with the pointer of 
    the data stored at the tree node as its argument.  For example,
    when we need to delete the tree node we also need to free a field
    of the data stored in addition to the data.  If no user delete
    function is supplied, only memory corresponding to the tree structure
    is freed.   .
		.
		.
	    Yrbtree_delete( my_tree, key, my_data_delete ) ;
		.
		.
		.
	    VOID my_data_delete( data )
	    MY_DATA_TYPE *data ;
	    {
		Ysafe_free( data->my_allocated_field ) ;
		Ysafe_free( data ) ;
	    } 

    Returns 1 if successful, 0 otherwise.
*/

extern BOOL Yrbtree_deleteCurrentInterval(P2(YTREEPTR tree,VOID (*userDelete)()));
/*
Function:
   While in an interval loop, deletes the current element.  This allows
   one to delete individual items that are not distinct with respect
   to the comparison function.  See the rbtree test program to see
   how it is used.
*/

extern BOOL Yrbtree_deleteCurrentEnumerate(P2(YTREEPTR tree,VOID (*userDel)()));
/*
Function:
   Like Yrbtree_deleteCurrentInterval, this routine is call from inside an
   Yrbtree_enumerate loop.  It will delete the current element and yet
   allow the user to continue enumeration.  This allows one to delete
   individual items that are not distinct with respect to the comparison
   function.  See the rbtree test program to see how it is used.
*/

extern VOID Yrbtree_empty( P2(YTREEPTR tree, VOID (*userDelete)() ) ) ;
/*
Arguments:
   YTREEPTR tree;
   BOOL freeDataflag;
Function:
   Removes all nodes in the tree.  If userDelete != 0, node data
   unallocated using userDelete function
*/

extern VOID Yrbtree_free( P2(YTREEPTR tree, VOID (*userDelete)() ) ) ;
/*
Arguments:
   YTREEPTR tree;
   BOOL freeDataflag;
Function:
   Removes tree and all nodes in the tree.  If userDelete != 0, node data
   unallocated using userDelete function
*/

extern INT Yrbtree_size( P1(YTREEPTR tree) ) ;
/*
Function:
   Find the total elements in the tree.
*/

extern INT(*Yrbtree_get_compare( P1(YTREEPTR tree) ))() ;
/*
Function:
   Returns a pointer to the tree's comparison function.
*/

extern INT Yrbtree_verify( P1(YTREEPTR tree) ) ;
/*
Function:
   Exercise tree pointers by walking through the tree.
   Useful for dubugging.
   The routine will fault or return a 0 if verification fails.
   The routine will return a 1 if verification is successful.
*/


extern VOIDPTR Yrbtree_search_closest( P3(YTREEPTR tree,VOIDPTR key,INT func) ) ;
/*
Function:
   Finds the closest match for a given key.  Will return the exact
   match if it exists.  Returns NULL if no items exist in tree.
   Sets the search marker for subsequent search_suc and search_pred
   calls.  Func is used to control the meaning of closest relative
   to the comparison function.  Ithas four values:
	SEARCH_CLOSEST_NOP
	SEARCH_CLOSEST
	SEARCH_CLOSEST_BELOW
	SEARCH_CLOSEST_ABOVE
   SEARCH_CLOSEST_NOP should be specified when the comparison function
   does not give an integer measure of the distance from a perfect match.
   SEARCH_CLOSEST is the normal mode. It returns the closest element
   to the key value based on the comparison functions return value.
   SEARCH_CLOSEST_BELOW returns the element closest to the key which is 
   less than or equal to the given key.
   SEARCH_CLOSEST_ABOVE returns the element closest to the key which is 
   greater than or equal to the given key.
*/

extern VOIDPTR Yrbtree_search_suc( P1(YTREEPTR tree) ) ;
/*
Function:
   Returns the sucessor to current search object (set by the search 
   marker).  Returns NULL if no such item exists.  This routine 
   should only be called after the search marker has been set
   by one of: Yrbtree_search_closest, Yrbtree_min, Yrbtree_max, or
   Yrbtree_search.
*/

extern VOIDPTR Yrbtree_search_pred( P1(YTREEPTR tree) ) ;
/*
Function:
   Returns the predecessor to current search object (set by the search 
   marker).  Returns NULL if no such item exists.  This routine 
   should only be called after the search marker has been set
   by one of: Yrbtree_search_closest, Yrbtree_min, Yrbtree_max, or
   Yrbtree_search.
*/

extern VOIDPTR Yrbtree_revlist( P2(YTREEPTR tree, BOOL startFlag) ) ;
/*
Function:
   Enumerate the tree in reverse order.
*/

extern VOID Yrbtree_dump( P2(YTREEPTR tree, VOID (*print_key)() )) ;
/*
Function:
   Dump the contents of a tree.  Print keys takes one argument,
   a key.
*/

extern VOID Yrbtree_resort( P2(YTREEPTR tree, INT (*compare_func)() )) ;
/*
Function:
   Takes a tree and resorts the tree with a new comparison function.
   All search markers are reset to NIL.
*/

extern YTREEPTR Yrbtree_copy( P2(YTREEPTR tree,INT (*compare_func)() ) );
/*
Function:
   Make a copy of a tree sorted with the given comparison function.
   Old tree remains allocated and all markers remain undisturbed.
*/

extern BOOL Yrbtree_insertIfUnique( P2(YTREEPTR tree, VOIDPTR data ) ) ;
/* 
Arguments:
    YTREEPTR tree ;
    VOIDPTR data ;
Function:
    Insert an element into the tree if it does exist in the tree.
    Returns TRUE if added to tree; otherwise returns FALSE.
*/

#endif /* RBTREE_H */
