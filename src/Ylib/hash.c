/*
 *   Copyright (C) 1988-1991 Yale University
 *
 *   This work is distributed in the hope that it will be useful; you can
 *   redistribute it and/or modify it under the terms of the
 *   GNU General Public License as published by the Free Software Foundation;
 *   either version 2 of the License,
 *   or any later version, on the following conditions:
 *
 *   (a) YALE MAKES NO, AND EXPRESSLY DISCLAIMS
 *   ALL, REPRESENTATIONS OR WARRANTIES THAT THE MANUFACTURE, USE, PRACTICE,
 *   SALE OR
 *   OTHER DISPOSAL OF THE SOFTWARE DOES NOT OR WILL NOT INFRINGE UPON ANY
 *   PATENT OR
 *   OTHER RIGHTS NOT VESTED IN YALE.
 *
 *   (b) YALE MAKES NO, AND EXPRESSLY DISCLAIMS ALL, REPRESENTATIONS AND
 *   WARRANTIES
 *   WHATSOEVER WITH RESPECT TO THE SOFTWARE, EITHER EXPRESS OR IMPLIED,
 *   INCLUDING,
 *   BUT NOT LIMITED TO, WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A
 *   PARTICULAR
 *   PURPOSE.
 *
 *   (c) LICENSEE SHALL MAKE NO STATEMENTS, REPRESENTATION OR WARRANTIES
 *   WHATSOEVER TO
 *   ANY THIRD PARTIES THAT ARE INCONSISTENT WITH THE DISCLAIMERS BY YALE IN
 *   ARTICLE
 *   (a) AND (b) above.
 *
 *   (d) IN NO EVENT SHALL YALE, OR ITS TRUSTEES, DIRECTORS, OFFICERS,
 *   EMPLOYEES AND
 *   AFFILIATES BE LIABLE FOR DAMAGES OF ANY KIND, INCLUDING ECONOMIC DAMAGE OR
 *   INJURY TO PROPERTY AND LOST PROFITS, REGARDLESS OF WHETHER YALE SHALL BE
 *   ADVISED, SHALL HAVE OTHER REASON TO KNOW, OR IN FACT SHALL KNOW OF THE
 *   POSSIBILITY OF THE FOREGOING.
 *
 */

/* ----------------------------------------------------------------- 
FILE:	    hash.c                                       
DESCRIPTION:This file contains the routines for building and 
	    maintaining a hash table.
CONTENTS:   
DATE:	    Jul  7, 1988 
REVISIONS:  Nov  6, 1988 - added user defined hash delete function.
	    Jan 18, 1989 - added delete operation albeit inefficient
		to hash_search routine.
	    Apr 27, 1989 - changed to Y prefix.
	    Apr 29, 1990 - added message.h
	    Aug  3, 1990 - added hash_add as a convenience function.
	    Oct  8, 1990 - made consistent with prototypes.
	    Dec  8, 1990 - fixed Yhash_table_size.
	    Fri Jan 18 18:38:36 PST 1991 - fixed to run on AIX.
	    Thu Apr 18 00:40:49 EDT 1991 - renamed functions for
		consistency.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) hash.c version 3.11 12/15/91" ;
#endif

#include <stdio.h>
#include <yalecad/base.h>
#include <yalecad/hash.h>
#include <yalecad/debug.h>

extern char *Ystrclone(char *);

/* definitions local to this file only */

#define  PRIMECOUNT   200
#define  MINPRIMESIZE 23

/* static global to this file only */
static INT tablesize ;
static YTABLEPTR *table ;


YHASHPTR Yhash_table_create( numentries )
INT numentries ;
{
    YHASHPTR  hashtable ;
    INT Yhash_table_size() ;

    hashtable = YMALLOC( 1, YHASHBOX ) ;
    hashtable->size = tablesize = Yhash_table_size( numentries ) ;

    table = YCALLOC( tablesize, YTABLEPTR ) ;
    hashtable->hash_table = table ;
    hashtable->thread = NULL ; /* initialize list */
    return(hashtable) ;
} /* end Yhash_create */

INT Yhash_table_get( hashtable )
YHASHPTR  hashtable ;
{
    return(hashtable->size) ;
}

Yhash_table_delete(hashtable, userdelete )
YHASHPTR  hashtable ;
INT  (*userdelete)() ;
{
    INT i ;
    YTABLEPTR hptr , zapptr ;

    table = hashtable->hash_table ;
    tablesize = hashtable->size ;
    for( i = 0 ; i < tablesize ; i++ ) {
	for( hptr=table[i];hptr; ){
	    zapptr = hptr ;
	    hptr = hptr->next ;
	    /* execute user define delete function if requested */
	    if( userdelete ){
		(*userdelete)(zapptr->data) ;
	    }
	    YFREE( zapptr ) ;
	}
    }
    YFREE( table ) ;
    YFREE( hashtable ) ;

}

/* returns true if conflict occured */
char *Yhash_search(hashtable, key, data, operation )
YHASHPTR  hashtable ;
char *key ;
VOIDPTR data ;
INT operation ;
{

#ifdef HASHFUNC1
    INT     i ,
	    len ;
#else
    INT     shift ;
    char    *name ;
#endif
    UNSIGNED_INT hsum = 0 ;
    YTABLEPTR   curPtr, temptr, curTable, tempThread ;

    /* initialization */
    table = hashtable->hash_table ;
    tablesize = hashtable->size ;

#ifdef HASHFUNC1
    len = strlen(key) ;
    for( i = 0 ;i < len; i++ ) {
	hsum += ( UNSIGNED_INT ) key[i] ;
    }
#else
    /*  FUNCTION hash_key */
    name = key ;
    for (shift=1 ;*name; name++){
	hsum = hsum + *name<<shift;
	shift = (shift + 1) & 0x0007;
    }
#endif
    hsum %= tablesize ;

    /* insert into table only if distinct number */
    if( temptr = table[hsum] ){
	/* list started at this hash */
	for(curPtr=temptr;curPtr;curPtr=curPtr->next ) {
	    if( strcmp(curPtr->key, key ) == STRINGEQ ){
		if( operation == DELETE ){
		    /* delete item in table by making data NULL */
		    /* this is only a quick fix and should be */
		    /* modified in the future to remove the space */
		    curPtr->data = NULL ;
		    /* operation a success so return -1 */
		    return( (char *) -1 ) ;
		} else {
		    /* operation find or enter - return item */
		    return( curPtr->data ) ;
		}
	    }
	}
	if( operation == ENTER ){
	    /* now save data */
	    table[hsum] = curTable = YMALLOC( 1, YTABLEBOX ) ;
	    curTable->data = (char *) data ;
	    curTable->key = (char *) Ystrclone( key ) ;
	    curTable->next = temptr ;
	    /* now fix thread which goes through hash table */
	    tempThread = hashtable->thread ;
	    hashtable->thread = curTable ;
	    curTable->threadNext = tempThread ;
	}

    } else {
	/* no list started at this hash */
	if( operation == ENTER ){ 
	    /* enter into the table on an enter command */
	    curTable = table[hsum] = YMALLOC( 1, YTABLEBOX ) ;
	    curTable->data = (char *) data ;
	    curTable->key = (char *) Ystrclone( key ) ;
	    curTable->next = NULL ;
	    /* now fix thread which goes through hash table */
	    if( tempThread = hashtable->thread ){
		hashtable->thread = curTable ;
		curTable->threadNext = tempThread ;
	    } else {
		/* first entry into hash table */
		hashtable->thread = curTable ;
		curTable->threadNext = NULL ;
	    } /* thread fixed */

	} else {
	    /* cant find anything on a find operation */
	    return( NULL ) ;
	}
    }
    return( NULL ) ; /* no conflict on a enter */

} /* end hash_search */

/* hash add adds to table if it doesn't already exist - new flag
   will be set to true.  If key already exists in table then
   hash add will not add it to the hash table but will notify the
   user by setting new flag to false.
*/
char *Yhash_add( hashtable, key, add_function, new_flag ) 
YHASHPTR  hashtable ;
char *key ;
char *(*add_function)() ;
BOOL *new_flag ;
{

#ifdef HASHFUNC1
    INT     i ,
	    len ;
#else
    INT     shift ;
    char    *name ;
#endif
    UNSIGNED_INT hsum = 0 ;
    YTABLEPTR   curPtr, temptr, curTable, tempThread ;

    /* initialization */
    table = hashtable->hash_table ;
    tablesize = hashtable->size ;

#ifdef HASHFUNC1
    len = strlen(key) ;
    for( i = 0 ;i < len; i++ ) {
	hsum += ( UNSIGNED_INT ) key[i] ;
    }
#else
    /*  FUNCTION hash_key */
    name = key ;
    for (shift=1 ;*name; name++){
	hsum = hsum + *name<<shift;
	shift = (shift + 1) & 0x0007;
    }
#endif
    hsum %= tablesize ;

    /* insert into table only if distinct number */
    if( temptr = table[hsum] ){
	/* list started at this hash */
	for(curPtr=temptr;curPtr;curPtr=curPtr->next ) {
	    if( strcmp(curPtr->key, key ) == STRINGEQ ){
		/* item is currently in the table set */
		/* new flag to false */
		*new_flag = FALSE ;
		return( curPtr->data ) ;
	    }
	}
	/* otherwise add to the table */
	/* now save data */
	table[hsum] = curTable = YMALLOC( 1, YTABLEBOX ) ;
	curTable->data = (*add_function)() ;
	curTable->key = (char *) Ystrclone( key ) ;
	curTable->next = temptr ;
	/* now fix thread which goes through hash table */
	tempThread = hashtable->thread ;
	hashtable->thread = curTable ;
	curTable->threadNext = tempThread ;

    } else {
	/* no list started at this hash */
	/* enter into the table on an enter command */
	curTable = table[hsum] = YMALLOC( 1, YTABLEBOX ) ;
	curTable->data = (*add_function)() ;
	curTable->key = (char *) Ystrclone( key ) ;
	curTable->next = NULL ;
	/* now fix thread which goes through hash table */
	if( tempThread = hashtable->thread ){
	    hashtable->thread = curTable ;
	    curTable->threadNext = tempThread ;
	} else {
	    /* first entry into hash table */
	    hashtable->thread = curTable ;
	    curTable->threadNext = NULL ;
	} /* thread fixed */
    }
    *new_flag = TRUE ;
    return( curTable->data ) ; /* no conflict on a enter */

} /* end hash_add */

INT Yhash_set_size(hashtable)
YHASHPTR  hashtable ;
{
    INT count = 0 ;
    YTABLEPTR thread ;

    for( thread = hashtable->thread;thread;thread=thread->threadNext ){
	count++;
    }
    return(count) ;
}

/*---------------------------- hash_table_size -------------------------*/
INT Yhash_table_size( minEntries )
INT minEntries;
{
  INT   i;
  BOOL  isPrime;
  INT   prime;
  INT   testPrime;
  static INT   primes[PRIMECOUNT] =
              {  3,   5,   7,  11,  13,  17,  19,  23,  29,  31,
                37,  41,  43,  47,  53,  59,  61,  67,  71,  73,
                79,  83,  89,  97, 101, 103, 107, 109, 113, 127,
                131, 137, 139, 149, 151, 157, 163, 167, 173, 179,
                181, 191, 193, 197, 199, 211, 223, 227, 229, 233,
                239, 241, 251, 257, 263, 269, 271, 277, 281, 283,
                293, 307, 311, 313, 317, 331, 337, 347, 349, 353,
                359, 367, 373, 379, 383, 389, 397, 401, 409, 419,
                421, 431, 433, 439, 443, 449, 457, 461, 463, 467,
                479, 487, 491, 499, 503, 509, 521, 523, 541, 547,
                557, 563, 569, 571, 577, 587, 593, 599, 601, 607,
                613, 617, 619, 631, 641, 643, 647, 653, 659, 661,
                673, 677, 683, 691, 701, 709, 719, 727, 733, 739,
                743, 751, 757, 761, 769, 773, 787, 797, 809, 811,
                821, 823, 827, 829, 839, 853, 857, 859, 863, 877,
                881, 883, 887, 907, 991, 919, 929, 937, 941, 947,
                953, 967, 971, 977, 983, 991, 997,1009,1013,1019,
               1021,1031,1033,1039,1049,1051,1061,1063,1069,1087,
               1091,1093,1097,1103,1109,1117,1123,1129,1151,1153,
               1163,1171,1181,1187,1193,1201,1213,1217,1223,1229 };

    if (minEntries <= MINPRIMESIZE){
	return(MINPRIMESIZE);
    } else {
	testPrime = minEntries;
	/* test to see if even */
	if ((testPrime % 2) == 0){
	    testPrime = testPrime + 1;
	}
	do {
	    testPrime = testPrime + 2;
	    isPrime = TRUE;
	    for (i=0;i < PRIMECOUNT;i++){
		prime = primes[i];
		if (testPrime < prime*prime){
		    break;
		}
		if ((testPrime % prime) == 0){
		    isPrime = FALSE;
		    break;
		}
	    }
	} while (!(isPrime));

	return(testPrime);
    }

}  /*  FUNCTION Yhash_table_size */
