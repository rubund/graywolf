/*
 *   Copyright (C) 1989-1991 Yale University
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
FILE:	    set.c                                       
DESCRIPTION:This file contains the set utility routines.  The set
members of the set are integers in a range [lowerLimit..upperLimit]. 
The routines add & subtract members from the set efficiently.  
Multiple sets may be handled simultaneously.  The complexity of the
routines are as follows.
    Yset_init      : O(n)
    Yset_free      : O(n)
    Yset_member    : O(1)
    Yset_add       : O(1)
    Yset_enumerate : O(1)
    Yset_empty     : O(1)
    Yset_size      : O(1)
    Yset_comp      : O(n)
The Yset_empty gets its speed by incrementing the value of in_set.  Since
all tests for membership check a match of this value, all members of set 
can be deleted at once. 
NOTE: These set routines are best suited for applications where you need
to enumerate the members of the set and the set is bounded by an integer.
In other applications, they may not be as suitable.
CONTENTS:
DATE:	    Apr 10, 1989 - original coding. 
REVISIONS:  Dec  4, 1989 added YsetSize and made Yenumerate a macro.
	    Apr 18, 1990 - now Yset_init allocates the memory for a set.
	    Apr 19, 1990 - added YcompSet 
	    May  8, 1990 - fixed error messages.
	    Fri Feb 15 15:36:27 EST 1991 - renamed the set functions.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) set.c version 3.8 12/15/91" ;
#endif

#include <yalecad/base.h>
#include <yalecad/debug.h>
#include <yalecad/message.h>
#include <yalecad/set.h>


/* initialize set */
YSETPTR Yset_init( lowerLimit, upperLimit ) 
INT lowerLimit, upperLimit ;
{   
    INT i ;
    INT sizeSet ;
    YSETPTR set ;

    set = YMALLOC( 1, YSET ) ;

    ASSERTNRETURN( set, "Yset_init", "Ran out of memory\n" ) ;

    /* make sure set limits are correct */
    if( lowerLimit > upperLimit ){
	M( ERRMSG, "Yset_init", "Set limits are in error\n" ) ;
	return ;
    }
    sizeSet = upperLimit - lowerLimit + 1 ;
    set->set = YMALLOC( sizeSet, YSETLISTPTR ) ; 
    for( i=0;i<sizeSet;i++ ){
	set->set[i] = YCALLOC( 1, YSETLISTBOX );
    }
    /* subtract off lower limit to make all */
    /* accesses of array easier */
    set->set -= lowerLimit ;
    set->lowerLimit = lowerLimit ;
    set->upperLimit = upperLimit ;
    set->size = sizeSet ;
    set->in_set = 1 ;
    set->cardinality = 0 ;
    set->list = NULL ;
    return( set ) ;
} /* end Yset_init */

Yset_free( set ) 
YSETPTR set ;
{   
    INT i ;
    INT sizeSet ;

    /* add back offset to free array */
    set->set += set->lowerLimit ;
    sizeSet = set->size ;
    for( i=0;i<sizeSet;i++ ){
	YFREE( set->set[i] ) ;
    }
    YFREE( set->set ) ;
    YFREE( set ) ;
} /* end Yset_free */

/* test whether a node is member of a set */
BOOL Yset_member( set, node ) 
YSETPTR set ;
INT  node ;
{
    if( node >= set->lowerLimit && node <= set->upperLimit ){
	if( set->set[node]->member == set->in_set ){
	    return( TRUE ) ;
	} else {
	    return( FALSE ) ;
	}
    } else {
	M( ERRMSG, "MEMBER", "value of node is out of bounds of set\n" ) ;
	return( FALSE ) ;
    }
} /* end Yset_member */

/* add a node to the set */
/* returns TRUE if this a new member of set FALSE if already a member */
BOOL Yset_add( set, node ) 
YSETPTR set ;
INT  node ;
{  
    YSETLISTPTR temp ;

    if( node >= set->lowerLimit && node <= set->upperLimit ){
	if( set->set[node]->member == set->in_set ){
	    return( FALSE ) ;
	}
	set->set[node]->member = set->in_set ;
	set->cardinality++ ;
	/* connect the doubly linked list */
	if( temp = set->list ){
	    /* hook to old list */
	    set->list = set->set[node] ;
	    set->list->next = temp ;
	    /* fix back link of temp */
	    temp->prev = set->list ;
	} else {
	    set->list = set->set[node] ;
	    /* terminate new list */
	    set->list->next = NULL ;
	}
	set->list->node = node ; /* store data */
	set->list->prev = NULL ; /* terminate backward list */

    } else {
	M( ERRMSG, "ADD2SET", "value of node is out of bounds of set\n") ;
	return( FALSE ) ;
    }
    return( TRUE ) ;
} /* end Yset_add */

/* delete a node from the set */
Yset_delete( set, node )
YSETPTR set ;
INT node ;
{
    YSETLISTPTR delptr ;

    ASSERTNRETURN( node >= set->lowerLimit && node <= set->upperLimit, 
	"Yset_delete", "node out of range\n") ;
    delptr = set->set[node] ;
    ASSERTNRETURN( delptr->member == set->in_set, "Yset_delete",
	"node has already been deleted\n" ) ;

    if( delptr->next ){
	if( delptr->prev ){ 
	    /* node to remove is in middle */
	    delptr->prev->next = delptr->next ;
	    delptr->next->prev = delptr->prev ;

	} else {
	    /* node to remove is first in the list */
	    set->list = delptr->next ;
	    delptr->next->prev = SETNULL ;
	}

    } else { /* node is at end of list */
	if( delptr->prev ){ 
	    /* node is at end and more than one member exists */
	    delptr->prev->next = SETNULL ;

	} else {
	    /* saveNode is only member in the list */
	    set->list = SETNULL ;
	}
    }
    /* now update boolean flag */
    delptr->member = FALSE ;
    /* update count */
    if( -- (set->cardinality) < 0 ){
	set->cardinality = 0 ;
    }

} /* end Yset_delete */
	
/* To clear set we only need to update in_set number and to null list */
Yset_empty( set ) 
YSETPTR set ;
{
    set->in_set++ ;
    set->list = NULL ;
    set->cardinality = 0 ;
} /* end Yset_empty */


/* Set complementation */
Yset_comp( set ) 
YSETPTR set ;
{

    INT i ;                       /* counter */
    INT lower ;                   /* lower bound on set */
    INT upper ;                   /* upper bound on set */
    INT member ;                  /* member match */
    YSETLISTPTR *setarray ;       /* set is an array of YLISTBOXes */


    lower = set->lowerLimit ;
    upper = set->upperLimit ;
    member = set->in_set ;
    setarray = set->set ;

    for( i = lower ; i <= upper; i++ ){
	/* perform complementation on the set */
	if( setarray[i]->member == member ){
	    setarray[i]->member = 0 ;
	} else {
	    setarray[i]->member = 1 ;
	}
    }
    /* now fix the linked list by emptying the set and adding to set */
    set->in_set = 1 ;
    set->list = NULL ;
    set->cardinality = 0 ;
    for( i = lower ; i <= upper; i++ ){
	if( setarray[i]->member ){
	    Yset_add( set, i ) ; 
	}
    }
} /* end Yset_comp */
