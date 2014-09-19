/* ----------------------------------------------------------------- 
"@(#) hash.h version 1.7 9/22/91"
FILE:	    hash.h                                       
DESCRIPTION:Insert file for threaded hash routines.
CONTENTS:   
DATE:	    Jul 17, 1988 
REVISIONS:  Nov  6, 1988 - added user delete function to delTable.
	    Jan 18, 1989 - added DELETE operation to hash_search.
	    Apr 27, 1989 - added Y prefix.
	    Thu Apr 18 00:52:26 EDT 1991 - renamed functions.
----------------------------------------------------------------- */
#ifndef HASH_H
#define HASH_H

#include <yalecad/base.h>

typedef struct ytable_rec {
    char *key ;
    char *data ;
    struct ytable_rec *next ;
    struct ytable_rec *threadNext ; /* thread thru entire table */
} YTABLEBOX, *YTABLEPTR ;

typedef struct {
    YTABLEPTR *hash_table ;
    YTABLEPTR thread ;
    INT size ;
} YHASHBOX, *YHASHPTR ;

#define ENTER   (INT) 1
#define FIND    (INT) 0
#define DELETE  (INT) 2

extern YHASHPTR Yhash_table_create( P1(INT numentries) ) ;
/*
Function:
    Returns a hash table with the given number of entries.
    More that one hash table can coexist at the same time.
*/

extern INT Yhash_table_get( P1( YHASHPTR  hashtable ) ) ;
/*
Function:
    Returns the current size of hash table set by Yhash_table_create 
*/

extern char *Yhash_search( P4(YHASHPTR  hashtable, char *key, VOIDPTR data,
	INT operation ) ) ;
/*
Function:
    Hash table search routine.  Given a hashtable and a key, perform
    the following operations:
	ENTER:if key is in table it, returns a pointer to the item.
	      if key is not in table, add it to the table. returns NULL.
	FIND:if key is in table, it returns a pointer to the item.
	      if key is not in the table, it returns NULL.
	DELETE:if key is in table, it returns -1
	       if key is not in table, it return NULL.
	Memory is not freed in the delete case, but marked dirty.
    Data is a pointer to the information to be store with the given key.
*/
		
extern char *Yhash_add( P4( YHASHPTR  hashtable, char *key,
	char *(*add_function)(), BOOL *new_flag ) ) ;
/*
Function:
    Hash table search convenience routine. It avoids two calls to hash_search
    in the case of always adding to the table it the item does not exist.
    Hash_add adds to table if it doesn't already exist - new flag
    will be set to true.  If key already exists in table then
    hash add will not add it to the hash table but will notify the
    user by setting new flag to false.  The add_function is used
    to associate the information with the key.  It should return a
    pointer to the memory where the information is stored.  This 
    function always returns a pointer to the data.
*/


extern Yhash_table_delete( P2(YHASHPTR  hashtable,INT (*userdelete)() ) ) ;
/*
Function:
    Frees the memory associated with a hash table. The user
    make supply a function which deletes the memory associated 
    with the data field.  This function must have the data
    pointer supplied by the hash add routines as an argument,ie.
    Yhash_table_delete( my_hash_table, my_free_func ) ;
    my_free_func( data )
    char *data ;
    {
    }
*/

extern INT Yhash_set_size( P1( YHASHPTR  hashtable ) ) ;
/*
Function:
    Since this is a threaded hash table we can find the size of
    all the valid entries in the table in O(n) where n is the
    number of valid entries. Returns the number of valid entries.
    One may enumerate the hash table by writing the following loop.
    TABLEPTR ptr ;
    for( ptr = mytable->thread; ptr ; ptr= ptr->threadNext ){
	...
    }
*/
#endif /* HASH_H */
