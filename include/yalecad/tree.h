/* ----------------------------------------------------------------- 
FILE:	    tree.h                                       
DESCRIPTION:Tree include file for binary and red-black trees.
CONTENTS:   
DATE:	    Mar 30, 1990 
REVISIONS:  
----------------------------------------------------------------- */
#ifndef TREE_H
#define TREE_H

#ifndef lint
static char treeId[] = "@(#) tree.h version 1.3 10/9/90" ;
#endif

#ifndef RBTREE_H
typedef struct tree {
    char       *root ;                  /* the root of the tree */
    INT        (*compare_func)() ;     /* how to compare two keys */
    INT        offset ;                /* how to get the key */
} YTREEBOX, *YTREEPTR ;
#endif

/* macro definition for tree structure see Ytree_init */
#define YTREE_INIT( tree_xz, compare_xz, name_xz, key_name_xz ) \
{ \
    name_xz *p_xz ; \
    p_xz = (name_xz *) Ysafe_malloc( sizeof(name_xz) ) ; \
    tree_xz = Ytree_init( compare_xz, \
	(int) &(p_xz->key_name_xz)-(int) (p_xz) ) ;\
    Ysafe_free( p_xz ) ; \
}

/* ******************* BINARY TREE INCLUDE FILES ****************** */
extern YTREEPTR Ytree_init( P2(INT compare_func(), INT offset ) ) ;
/* 
Arguments:
    INT compare_func() ;
    INT offset ;
Function:
    Initializes a binary tree.  The user should supply a compare
    comparison function similar to the one used by the UNIX 
    quicksort routine.  The function compare_func is assumed to have
    two arguments which are pointers to the arguments being compared.
    The function should return an integer less than, equal to, or
    greater than 0 according as the first argument is to be considered
    less than, equal to , or greater than the second.  Offset is
    the distance in bytes that the key is offset from the beginning
    of the data record.  The function returns a pointer to the
    tree data structure. Use the tree pointer returned by Ytree_init 
    for all routines to work on this tree.  The macro YTREE_INIT 
    has been designed to make calling this routine easier. 
    The macro has the following syntax: 
    YTREE_INIT( tree, compare_func, datarecordname, keyname )
    where tree is the name of the returned tree.  Compare_func is
    the name of the compare function.  Data record name is the name
    of the record to be stored, and key name is the field to be
    used as the key to the record.
*/


extern char *Ytree_search( P2(YTREEPTR tree, char *key ) ) ;
/* 
Arguments:
    YTREEPTR tree ;
    char *key ;
Function:
    Given a binary tree data structure, it return the a pointer
    to the stored data object that matches the given key. It 
    returns NULL if no match is found.
*/

extern char *Ytree_min( P1(YTREEPTR tree ) ) ;
/* 
Arguments:
    YTREEPTR tree ;
Function:
    Given a binary tree data structure, it return the a pointer
    to the minimum data object stored in the tree.  It returns 
    NULL if nothing is in the tree.
*/

extern char *Ytree_max( P1(YTREEPTR tree ) ) ;
/* 
Arguments:
    YTREEPTR tree ;
Function:
    Given a binary tree data structure, it return the a pointer
    to the maximum data object stored in the tree.  It returns 
    NULL if nothing is in the tree.
*/

extern char *Ytree_suc( P2(YTREEPTR tree, char *key ) ) ;
/* 
Arguments:
    YTREEPTR tree ;
    char *key ;
Function:
    Given a binary tree data structure, it return the a pointer
    to the successor to the given key stored in the tree.  It returns 
    NULL if nothing is in the tree or if no match to the key is found.
*/

extern Ytree_insert( P2(YTREEPTR tree, char *data ) ) ;
/* 
Arguments:
    YTREEPTR tree ;
    char *data ;
Function:
    Insert an element into the tree.  Data is a pointer to the users
    record to be store in the tree.  Each record must contain a key
    to sort upon.
*/

extern char *Ytree_enumerate( P1(YTREEPTR tree ) ) ;
/* 
Arguments:
    YTREEPTR tree ;
Function:
    Enumerate all the data in a tree.  First time call with tree 
    as argument to get first element in tree starting at the minimum
    element.  For all subsequent calls, pass a NULL argument to 
    get all the remaining members of the tree.  Returns a pointer
    to the user record.  Returns NULL when all elements have been
    requested or when no match can be found. Restrictions:can
    only be performed on 1 tree at a time.  Use rbtree in this case.
*/

extern char *Ytree_interval( P3(YTREEPTR tree, char *low_key, char *high_key ) ) ;
/* 
Arguments:
    YTREEPTR tree ;
    char *low_key, *high_key ;
Function:
    Enumerate all the data in a tree between low_key and high_key. 
    First time call with tree as argument to get first element in 
    tree >= the low_key.  For all subsequent calls, pass a 
    NULL argument to get all the remaining members of the tree. 
    Returns NULL when element > high_key or no match can be found.
    Restrictions:can only be performed on 1 tree at a time.
    Use rbtree in this case.
*/

extern BOOL Ytree_delete( P2(YTREEPTR tree, char *key ) ) ;
/* 
Arguments:
    YTREEPTR tree ;
    char *key ;
Function:
    Delete a node in the tree by using the key.  Returns 1 if
    successful, 0 otherwise.
*/

#endif /* TREE_H */

