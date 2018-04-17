/*
 *   Copyright (C) 1990-1992 Yale University
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

/*----------------------------------------------------------------- 
FILE:	    rbtree.c                                       
DESCRIPTION:Red-black Binary tree routines.
CONTENTS:   
DATE:	    Mar 30, 1990 
REVISIONS:  Apr 19, 1990 - Added Yrbtree_dump()
	    Apr 28, 1990 - Added Yrbtree_free and Yrbtree_interval.
	    Oct  4, 1990 - rewrote to make Yrbtree_interval and
		enumerate work with more that one tree.
	    Oct 10, 1990 - added Yrbtree_size.
	    Oct 22, 1990 - added user_delete function to 
		rbtree_delete.
            Nov 02, 1990 - added Yrbtree_interval_free function -RAW
	    Tue Jan 15 02:13:35 EST 1991 - fixed problem with
		rbtree_interval.
	    Fri Feb  8 16:31:34 EST 1991 - fixed voidptr type casts
	    Sat Feb 23 04:59:18 EST 1991 - added debug.
            Apr 01, 1991 - maintain tree size instead of recounting -RAW
	    Thu Apr 18 00:41:37 EDT 1991 - fixed missing message.h
		and spelling error.
            Apr 19, 1991 - added Yrbtree_verify for debugging -RAW
	    Sun Apr 28 14:03:44 EDT 1991 - fixed problem with
		Yrbtree_interval using wrong comparison function.  
		Moved comparison function initialization to proper place.
	    Mon Aug 12 15:55:51 CDT 1991 - removed offset argument
		to initialization of tree in order to eliminate
		compiler problems.
	    Thu Oct 10 17:28:21 EDT 1991 - added rbtree_resort and copy.
            Thu Oct 17 11:19:18 EDT 1991 - added rbtree_pred and
               rbtree_rev_list ( reverse list )
	    10/18/91 - Changed startFlag from INT to BOOL for gcc -RAW 
	    Tue Oct 29 15:00:58 EST 1991 - added two search routines
		which keep track where you are in the tree.
	    Mon Dec  9 15:34:09 EST 1991 - now deck datastructures are
		associated with each tree so that memory can be freed
		correctly.
	    Sun Dec 15 02:39:26 EST 1991 - added example program for
		beginners.
	    Sun Jan 19 17:31:34 EST 1992 - added Yrbtree_deleteCurrentInterval,
		Yrbtree_deleteCurrentEnumerate, and fixed Yrbtree_search_closest.
	    Fri Feb  7 16:47:57 EST 1992 - added Yrbtree_interval_size.
	    Wed Feb 26 03:55:15 EST 1992 - fixed problem with successor routines
		when passed nilS.
 ----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) rbtree.c (Yale) version 3.38 4/18/92" ;
#endif

#define YTREE_H_DEFS

#include <yalecad/base.h>
#include <yalecad/debug.h>
#include <yalecad/deck.h>
#include <yalecad/message.h>

/* define macros for easier reading of the code */
#define  comp_f( a, b )  ( (*comp_func)(a,b) )

#define BLACK        0
#define RED          1

typedef struct bin_tree {
    struct bin_tree *left ;
    struct bin_tree *right ;
    struct bin_tree *parent ;
    VOIDPTR data ;
    BOOL color ;
} BINTREE, *BINTREEPTR ;

typedef struct tree {
    BINTREEPTR root ;                 /* the root of the tree */
    INT        (*compare_func)() ;    /* how to compare two keys */
    INT        size;                  /* current size of tree */
    BINTREEPTR intervalPtr;           /* current position in interval search */
    BINTREEPTR searchPtr;             /* current position in search */
    BINTREEPTR enumeratePtr;          /* current position in enumeration */
    YDECKPTR   enumerateDeck; 	      /* enumerate pointer deck */
    YDECKPTR   intervalDeck;          /* interval pointer deck */
} YTREEBOX, *YTREEPTR ;


#include <yalecad/rbtree.h>

/* ***************** STATIC VARIABLE DEFINITIONS ******************* */
static INT          treeSizeS ;        /* the size of a tree */
static BINTREE      sentinelS ;        /* build a sentinel */
static BINTREEPTR   nilS ;             /* pointer to sentinel */
static BINTREE      recalc_sentinelS ; /* build a recalculate flag */
static BINTREEPTR   recalcS ;          /* pointer to recalc flag */

/* ***************** STATIC FUNCTION DEFINITIONS ******************* */
static BINTREEPTR tree_search( P2(YTREEPTR tree, char *key) ) ;
static BINTREEPTR tree_suc( P1(BINTREEPTR ptr) ) ;
static BINTREEPTR tree_pred( P1(BINTREEPTR ptr) ) ;
static left_rotate( P2(YTREEPTR tree, BINTREEPTR x) ) ;
static right_rotate( P2(YTREEPTR tree, BINTREEPTR x) ) ;
static tree_free( P1(BINTREEPTR ptr) ) ;
static free_tree_and_data( P2(BINTREEPTR ptr, VOID (*userDelete)() ) ) ;
static tree_delete( P3(YTREEPTR tree, BINTREEPTR z, VOID (*userDelete)() ) ) ;
static tree_dump( P4(YTREEPTR tree,BINTREEPTR ptr,
    		    VOID (*print_key)(),INT printTab) ) ;

YTREEPTR Yrbtree_init( compare_func )
INT (*compare_func)() ;  /* user specifies key function */
{

    YTREEPTR tree ;      /* the current tree being built */

    /* initialize nilS */
    nilS = &sentinelS ;
    nilS->parent = NIL(BINTREEPTR) ;
    nilS->right = NIL(BINTREEPTR) ;
    nilS->left = NIL(BINTREEPTR) ;
    nilS->data = NIL(VOIDPTR) ;
    nilS->color = BLACK ;
    recalcS = &recalc_sentinelS ;

    tree = (YTREEPTR) YMALLOC( 1, YTREEBOX ) ;
    tree->root = nilS ;
    tree->compare_func = compare_func ;
    tree->enumerateDeck = NIL(YDECKPTR); 	     
    tree->intervalDeck = NIL(YDECKPTR) ; 
    tree->intervalPtr = NIL(BINTREEPTR) ;
    tree->searchPtr = NIL(BINTREEPTR) ;
    tree->enumeratePtr = NIL(BINTREEPTR) ;
    tree->size = 0;
    return( tree ) ;

} /* Yrbtree_init */

/* used internally in this routine */
static BINTREEPTR tree_search( tree, key )
YTREEPTR tree ;
char *key ;
{
    INT (*comp_func)() ;      /* current compare function */
    BINTREEPTR ptr ;            /* current node in the tree */
    INT k ;                   /* test condition [-1,0,1] */

    ptr = tree->root ;
    comp_func = tree->compare_func ;

    while( ptr != nilS && (k = comp_f( ptr->data, key )) ){
	if( k > 0 ){
	    ptr = ptr->left ;
	} else {
	    ptr = ptr->right ;
	}
    }
    return( ptr ) ;
    
} /* end tree_search */


/* the global version */
VOIDPTR Yrbtree_search( tree, key )
YTREEPTR tree ;
VOIDPTR key ;
{
    INT (*comp_func)() ;      /* current compare function */
    BINTREEPTR ptr ;          /* current node in the tree */
    INT k ;

    ptr = tree->root ;
    comp_func = tree->compare_func ;

    while( ptr != nilS && (k = comp_f( ptr->data, key )) ){
	if( k > 0 ){
	    ptr = ptr->left ;
	} else {
	    ptr = ptr->right ;
	}
    }
    tree->searchPtr = ptr;	/* save new current position */
    if( ptr != nilS ){
	return( ptr->data ) ;
    }

    return( NIL(VOIDPTR) ) ;
    
} /* end Yrbtree_search */

VOIDPTR Yrbtree_search_closest( tree, key, func )
YTREEPTR tree ;
VOIDPTR key ;
INT func ;
{
    INT (*comp_func)() ;      /* current compare function */
    BINTREEPTR ptr ;          /* current node in the tree */
    BINTREEPTR closest_ptr ;  /* current closest match in the tree */
    BINTREEPTR suc, pred ;    /* successor and predecessor for closest_ptr */
    INT dist1, dist2 ;	      /* distance using predecessor functions */
    INT k ;

    ptr = tree->root ;
    comp_func = tree->compare_func ;
    closest_ptr = NIL(BINTREEPTR) ;

    while( ptr != nilS && (k = comp_f( ptr->data, key )) ){
	closest_ptr = ptr ; /* save the closest match so far */
	if( k > 0 ){
	    ptr = ptr->left ;
	} else {
	    ptr = ptr->right ;
	}

    }
    if( ptr == nilS ){
	/* in the case of no exact match return closest */
	if( closest_ptr ){
	    switch( func ){
	    case SEARCH_CLOSEST_NOP:
		break ;
	    case SEARCH_CLOSEST_BELOW:
		dist1 = comp_f( closest_ptr->data, key ) ;
		if( dist1 > 0 ){
		    pred = tree_pred( closest_ptr ) ;
		    if( pred != nilS ){
			/* take predecessor */
			closest_ptr = pred ;
		    }
		}
		break ;
	    case SEARCH_CLOSEST_ABOVE:
		dist1 = comp_f( closest_ptr->data, key ) ;
		if( dist1 < 0 ){
		    suc = tree_suc( closest_ptr ) ;
		    if( suc != nilS ){
			/* take successor */
			closest_ptr = suc ;
		    }
		}
		break ;
	    case SEARCH_CLOSEST:
		/* ----------------------------------------------------------
		*  See which one is closer - current pointer, predecessor
		*  or sucessor.
		---------------------------------------------------------- */
		dist1 = comp_f( closest_ptr->data, key ) ;
		pred = tree_pred( closest_ptr ) ;
		if( pred != nilS ){
		    dist2 = comp_f( pred->data, key ) ;
		    if( ABS(dist2) < ABS(dist1) ){
			/* predecessor is closer */
			closest_ptr = pred ;
			dist1 = dist2 ;
		    } 
		}
		suc = tree_suc( closest_ptr ) ;
		if( suc != nilS ){
		    dist2 = comp_f( suc->data, key ) ;
		    if( ABS(dist2) < ABS(dist1) ){
			/* take sucessor only if it is better */
			closest_ptr = suc ;
		    } 
		}
		break ;
	    } /* end switch( func )...  */
	    ptr = closest_ptr ;

	} else {
	    /* there is nothing in the tree */
	    return( NIL(VOIDPTR) ) ;
	}
    }
    tree->searchPtr = ptr;	/* save new current position */
    return( ptr->data ) ;
    
} /* end Yrbtree_search_closest */


VOIDPTR Yrbtree_min( tree )
YTREEPTR tree ;
{
    BINTREEPTR ptr ;          /* current node in the tree */
    
    if ( tree->root == nilS ) {
      return( NIL(VOIDPTR) );    
    }

    for( ptr = tree->root; ptr->left != nilS ; ptr = ptr->left ) ;
    tree->searchPtr = ptr;	/* save new current position */
    if( ptr != nilS ){
	return( ptr->data ) ;
    }
    return( NIL(VOIDPTR) ) ;
} /* end Yrbtree_min */

VOIDPTR Yrbtree_max( tree )
YTREEPTR tree ;
{
    BINTREEPTR ptr ;          /* current node in the tree */

    if ( tree->root == nilS ) {
      return( NIL(VOIDPTR) );    
    }

    for( ptr = tree->root; ptr->right != nilS ; ptr = ptr->right ) ;
    tree->searchPtr = ptr;	/* save new current position */
    if( ptr != nilS ){
	return( ptr->data ) ;
    }
    return( NIL(VOIDPTR) ) ;
} /* end Yrbtree_min */

static BINTREEPTR tree_suc( ptr )
BINTREEPTR ptr ;
{
    BINTREEPTR y ;          /* a temporary pointer */

    if( ptr == nilS ){
	return( nilS ) ;
    }
    if( ptr->right != nilS ){
	for( ptr = ptr->right; ptr->left != nilS ; ptr = ptr->left ) ;
	return( ptr ) ;
    }
    for( y = ptr->parent; y != nilS && ptr == y->right; y = y->parent ){
	ptr = y ;
    }
    return( y ) ;
} /* end tree_suc */

static BINTREEPTR tree_pred( ptr )
BINTREEPTR ptr ;
{
    BINTREEPTR y ;          /* a temporary pointer */

    if( ptr == nilS ){
	return( nilS ) ;
    }
    if( ptr->left != nilS ){
	/* find the rightmost of the left pointer */
	for( ptr = ptr->left; ptr->right != nilS ; ptr = ptr->right ) ;
	return( ptr ) ;
    }
    /* otherwise we have to back up to the parent */
    for( y = ptr->parent; y != nilS && ptr == y->left; y = y->parent ){
	ptr = y ;
    }
    return( y ) ;
} /* end tree_pred */


VOIDPTR Yrbtree_suc( tree, data )
YTREEPTR tree ;
VOIDPTR data ;
{
    BINTREEPTR ptr ;          /* a temporary pointer */
    BINTREEPTR suc ;          /* a temporary pointer */

    ptr = tree_search( tree, data ) ;
    suc = tree_suc( ptr ) ;
    if( suc != nilS ){
	return( suc->data ) ;
    }
    return( NIL(VOIDPTR) ) ;
} /* end Yrbtree_suc */

VOIDPTR Yrbtree_search_suc( tree )
YTREEPTR tree ;
{
    BINTREEPTR ptr ;		/* current node in the tree */

    /* find successor to current position in tree set by Yrbtree_search */
    ptr = tree->searchPtr;	/* get old current position */
    ptr = tree_suc( ptr ) ;
    tree->searchPtr = ptr;	/* save new current position */

    if( ptr != nilS ){
	return( ptr->data ) ;
    }
    return( NIL(VOIDPTR) ) ;
    
} /* end Yrbtree_search_suc */

VOIDPTR Yrbtree_search_pred( tree )
YTREEPTR tree ;
{
    BINTREEPTR ptr ;		/* current node in the tree */

    /* 
	Find predecessor to current position in tree set by 
	Yrbtree_search or Yrbtree_interval 
    */
    ptr = tree->searchPtr;	/* get old current position */
    ptr = tree_pred( ptr ) ;
    tree->searchPtr = ptr;	/* save new current position */

    if( ptr != nilS ){
	return( ptr->data ) ;
    }
    return( NIL(VOIDPTR) ) ;
    
} /* end Yrbtree_search_pred */

static left_rotate( tree, x )
YTREEPTR tree ;
BINTREEPTR x ;
{
    BINTREEPTR y ;          /* a temporary pointer */

    y = x->right ;             /* set y */
    x->right = y->left ;       /* Turn y's left subtreee in x's right */
    if( y->left != nilS ){
	y->left->parent = x ;
    }
    y->parent = x->parent ;   /* link x's parent to y */
    if( x->parent == nilS ){
	tree->root = y ;
    } else if( x == x->parent->left ){
	x->parent->left = y ;
    } else {
	x->parent->right = y ;
    }
    y->left = x ;             /* put x on y's left */
    x->parent = y ;       
} /* left_rotate */

static right_rotate( tree, x )
YTREEPTR tree ;
BINTREEPTR x ;
{
    BINTREEPTR y ;          /* a temporary pointer */

    y = x->left ;             /* set y */
    x->left = y->right ;      /* Turn y's right subtreee in x's left */
    if( y->right != nilS ){
	y->right->parent = x ;
    }
    y->parent = x->parent ;   /* link x's parent to y */
    if( x->parent == nilS ){
	tree->root = y ;
    } else if( x == x->parent->right ){
	x->parent->right = y ;
    } else {
	x->parent->left = y ;
    }
    y->right = x ;            /* put x on y's right */
    x->parent = y ;       
} /* right_rotate */

/* -----------------------------------------------------------------
* Delete a node in the tree by using actual pointer.  Also frees
* user data if necessary.
----------------------------------------------------------------- */
static tree_delete( tree, z, userDelete )
YTREEPTR tree ;
BINTREEPTR z ;
VOID (*userDelete)();
{
    BINTREEPTR w ;            /* a temporary pointer */
    BINTREEPTR x ;            /* a temporary pointer */
    BINTREEPTR y ;            /* a temporary pointer */

    /* we must delete the user data first so to avoid copy problem later */
    if (userDelete) {
      (*userDelete)( z->data ) ;
    }
    /* now begin deletion process */
    /* determine which node to splice out */
    if( z->left == nilS || z->right == nilS ){
	y = z ;
    } else {
	y = tree_suc( z ) ;
    }
    /* let x be the non-nil child of y or nil if y has no children */
    if( y->left != nilS ){
	x = y->left ;
    } else {
	x = y->right ;
    }
    /* splice out y */
    x->parent = y->parent ;
    if( y->parent == nilS ){
	/* y is the root - x becomes new root */
	tree->root = x ;
    } else if( y == y->parent->left ){
	y->parent->left = x ;
    } else {
	y->parent->right = x ;
    }
    /* if the successor of z was spliced out copy content */
    if( y != z ){
	z->data = y->data ;
    }

    /* now take care of the red black attributes */
    if( y->color == BLACK ){
	while( x != tree->root && x->color == BLACK ){
	    if( x == x->parent->left ){
		w = x->parent->right ;
		if( w->color == RED ){
		    w->color = BLACK ;
		    x->parent->color = RED ;
		    left_rotate( tree, x->parent ) ;
		    w = x->parent->right ;
		}
		if( w->left->color == BLACK && w->right->color == BLACK ){
		    w->color = RED ;
		    x = x->parent ;
		} else {
		    if( w->right->color == BLACK ){
			w->left->color = BLACK ;
			w->color = RED ;
			right_rotate( tree, w ) ;
			w = x->parent->right ;
		    }
		    w->color = x->parent->color ;
		    x->parent->color = BLACK ;
		    w->right->color = BLACK ;
		    left_rotate( tree, x->parent ) ;
		    x = tree->root ;
		}
	    } else { /* right side case */
		w = x->parent->left ;
		if( w->color == RED ){
		    w->color = BLACK ;
		    x->parent->color = RED ;
		    right_rotate( tree, x->parent ) ;
		    w = x->parent->left ;
		}
		if( w->right->color == BLACK && w->left->color == BLACK ){
		    w->color = RED ;
		    x = x->parent ;
		} else {
		    if( w->left->color == BLACK ){
			w->right->color = BLACK ;
			w->color = RED ;
			left_rotate( tree, w ) ;
			w = x->parent->left ;
		    }
		    w->color = x->parent->color ;
		    x->parent->color = BLACK ;
		    w->left->color = BLACK ;
		    right_rotate( tree, x->parent ) ;
		    x = tree->root ;
		}
	    } /* end right side case */
	} /* end while loop */
	x->color = BLACK ;
    } /* end test on y's color */

    /* now safe to free y */
    YFREE(y);

    tree->size --;

} /* end tree_delete */


VOID Yrbtree_insert( tree, data )
YTREEPTR tree ;
VOIDPTR data ;
{
    BINTREEPTR x ;          /* a temporary pointer */
    BINTREEPTR y ;          /* a temporary pointer */
    BINTREEPTR z ;          /* a temporary pointer */
    BINTREEPTR nil ;          /* a temporary pointer */
    BINTREEPTR ptr ;          /* a temporary pointer */
    INT (*comp_func)() ;      /* current compare function */

    if(!(data)){
	M( ERRMSG, "Yrbtree_insert","No data given\n" ) ;
	return ;
    }

    /* first perform an normal insertion into the binary tree */
    comp_func = tree->compare_func ;
    y = nilS ;
    x = YMALLOC( 1, BINTREE ) ;
    x->data = data ;
    x->left = nilS ;
    x->right = nilS ;
    /* find the place where we want to deposit the data */
    for( ptr = tree->root; ptr != nilS ; ){
	y = ptr ;  /* save the place for the insert */
	if( comp_f( data, ptr->data ) < 0 ){
	    ptr = ptr->left ;
	} else {
	    ptr = ptr->right ;
	}
    }
    /* now set the parent field of x */
    x->parent = y ;
    if( y == nilS ){
	tree->root = x ;
    } else if( comp_f( data, y->data ) < 0 ){
	y->left = x ;
    } else {
	y->right = x ;
    }

    /* now fix up the tree so it is a red black tree */
    x->color = RED ;
    while( x != tree->root && x->parent->color == RED ){
	if( x->parent == x->parent->parent->left ){
	    y = x->parent->parent->right ;
	    if( y->color == RED ){
		x->parent->color = BLACK ;
		y->color = BLACK ;
		x->parent->parent->color = RED ;
		x = x->parent->parent ;
	    } else {
		if( x == x->parent->right ){
		    x = x->parent ;
		    left_rotate( tree, x ) ;
		}
		x->parent->color = BLACK ;
		x->parent->parent->color = RED ;
		right_rotate( tree, x->parent->parent ) ;
	    }
	} else {
	    y = x->parent->parent->left ;
	    if( y->color == RED ){
		x->parent->color = BLACK ;
		y->color = BLACK ;
		x->parent->parent->color = RED ;
		x = x->parent->parent ;
	    } else {
		if( x == x->parent->left ){
		    x = x->parent ;
		    right_rotate( tree, x ) ;
		}
		x->parent->color = BLACK ;
		x->parent->parent->color = RED ;
		left_rotate( tree, x->parent->parent ) ;
	    }
	}
    }
    tree->root->color = BLACK ;
    tree->size ++;
} /* Yrbtree_insert */

/* enumerate all the data.  First time call with startFlag=TRUE */
/* next times call with startFlag=FALSE. */
VOIDPTR Yrbtree_enumerate( tree, startFlag )
YTREEPTR tree ;
BOOL startFlag;
{
  BINTREEPTR ptr ;          /* remember where we are */
  
  if( startFlag ){
    for( ptr = tree->root; ptr != nilS && ptr->left != nilS;
	ptr = ptr->left ) ;
  } else {
    /* find successor to current ptr */
    ptr = tree->enumeratePtr;         /* get old current position */
    /* find successor to current ptr */
    if( ptr == recalcS ){
      /* calculate from the beginning */
      return( Yrbtree_enumerate( tree, TRUE )) ;
    } else {
      ptr = tree_suc( ptr ) ;
    }
  }
  tree->enumeratePtr = ptr;         /* save new current position */
  if( ptr != nilS ){
    return( ptr->data ) ;
  }
  return( NIL(VOIDPTR) ) ;
} /* end Yrbtree_enumerate */

/* inverse of enumerate.  Start at max of tree and work backwords */
/* next times call with startFlag=FALSE. */
VOIDPTR Yrbtree_revlist( tree, startFlag )
YTREEPTR tree ;
BOOL startFlag;
{
  BINTREEPTR ptr ;          /* remember where we are */
  
  if( startFlag ){
    /* find the maximum in the tree */
    for( ptr = tree->root;ptr != nilS && ptr->right != nilS;
	ptr = ptr->right ) ;
  } else {
    ptr = tree->enumeratePtr;         /* get old current position */
    /* find successor to current ptr */
    if( ptr == recalcS ){
      /* calculate from the beginning */
      return( Yrbtree_revlist( tree, TRUE )) ;
    } else {
      ptr = tree_pred( ptr ) ;
    }
  }
  tree->enumeratePtr = ptr;         /* save new current position */
  if( ptr != nilS ){
    return( ptr->data ) ;
  }
  return( NIL(VOIDPTR) ) ;
} /* end Yrbtree_revlist */

/* Push the current enumeration pointer.  This is useful */
/* for recursive enumeration                             */
VOID Yrbtree_enumeratePush ( tree )
YTREEPTR tree;
{
   if (!(tree->enumerateDeck)) {
     tree->enumerateDeck = Ydeck_init();
   }

   Ydeck_push( tree->enumerateDeck,tree->enumeratePtr);
} 

/* pops the current enumeration pointer.  This is useful */
/* for recursive enumeration                             */
VOID Yrbtree_enumeratePop ( tree )
YTREEPTR tree;
{
   if (!(tree->enumerateDeck)) {
     M(ERRMSG,"Yrbtree_enumeratePop","There has been no push\n");
   }
   
   if ( Ydeck_size(tree->enumerateDeck) <= 0 ) {
     M(ERRMSG,"Yrbtree_enumeratePop","Imbalance of push/pop\n");
   }
 
    tree->enumeratePtr = (BINTREEPTR) Ydeck_pop(tree->enumerateDeck);
} /* end Yrbtree_enumeratePop() */

/* delete a node in the tree by using the key */
BOOL Yrbtree_delete( tree, key, userDelete )
YTREEPTR tree ;
VOIDPTR key ;
VOID (*userDelete)();
{
    BINTREEPTR z ;            /* a temporary pointer */

    z = tree_search( tree, key ) ;
    if( z == nilS ){
	/* didn't find data in the tree */
	return( FALSE ) ;
    }
    tree_delete( tree, z, userDelete ) ;
    return( TRUE ) ;

} /* end Yrbtree_delete() */

BOOL Yrbtree_deleteCurrentInterval( tree, userDelete )
YTREEPTR tree ;
VOID (*userDelete)();
{
    BINTREEPTR z ;            /* a temporary pointer */
    BINTREEPTR w ;            /* a temporary pointer */

    if( (z = tree->intervalPtr) && z != nilS ){
	/* -----------------------------------------------------------------
	*  Perform operations to insure that interval enumerate works 
	*  after delete.  Lucky since predecessor does not get mangled
	*  in a tree delete operation.
	----------------------------------------------------------------- */
	w = tree_pred( z ) ;
	if( w != nilS ){
	    tree->intervalPtr = w ; /* ok to use predecessor */
	} else {
	    /* this is the case where we delete min in tree */
	    /* need to recalculate */
	    tree->intervalPtr = recalcS ;
	}
	/* -----------------------------------------------------------------
	*  Now we can delete the rbtree pointer.
	----------------------------------------------------------------- */
	tree_delete( tree, z, userDelete ) ;
	return( TRUE ) ;
    } else {
	return( FALSE ) ;
    }
} /* end Yrbtree_deleteCurrentInterval() */

BOOL Yrbtree_deleteCurrentEnumerate( tree, userDelete )
YTREEPTR tree ;
VOID (*userDelete)();
{
    BINTREEPTR z ;            /* a temporary pointer */
    BINTREEPTR w ;            /* a temporary pointer */

    if( (z = tree->enumeratePtr) && z != nilS ){
	/* -----------------------------------------------------------------
	*  Perform operations to insure that interval enumerate works 
	*  after delete.  Lucky since predecessor does not get mangled
	*  in a tree delete operation.
	----------------------------------------------------------------- */
	w = tree_pred( z ) ;
	if( w != nilS ){
	    tree->enumeratePtr = w ; /* ok to use predecessor */
	} else {
	    /* this is the case where we delete min in tree */
	    /* need to recalculate */
	    tree->enumeratePtr = recalcS ;
	}
	/* -----------------------------------------------------------------
	*  Now we can delete the rbtree pointer.
	----------------------------------------------------------------- */
	tree_delete( tree, z, userDelete ) ;
	return( TRUE ) ;
    } else {
	return( FALSE ) ;
    }
} /* end Yrbtree_deleteCurrentEnumerate() */

static tree_dump( tree, ptr, print_key, printTab )
YTREEPTR tree ;
BINTREEPTR ptr ;
VOID (*print_key)() ;
INT printTab ;
{
    INT  i ;

    if( ptr != nilS ){
	printTab+=4;

	tree_dump( tree, ptr->right, print_key, printTab ) ;

	/* print level */
	fprintf( stderr, " " ) ;
	for( i = 1; i < printTab; i++ ){
	    fprintf( stderr, " " ) ;
	}
	(*print_key)( ptr->data ) ;
	if( ptr->color == BLACK ){
	    fprintf( stderr, ":black\n" ) ;
	} else {
	    fprintf( stderr, ":red\n" ) ;
	}

	tree_dump( tree, ptr->left, print_key, printTab ) ;

    }
} /* end Ytree_dump */

VOID Yrbtree_dump( tree, print_key )
YTREEPTR tree ;
VOID (*print_key)() ;
{
    if( tree ){
        Yrbtree_verify(tree);
	tree_dump( tree, tree->root, print_key, 1 ) ;
    }
} /* end Ytree_dump */


VOIDPTR Yrbtree_interval( tree, low_key, high_key, startFlag )
YTREEPTR tree ;
VOIDPTR low_key, high_key ;
BOOL startFlag;
{
    BINTREEPTR ptr ;                  /* remember where we are */
    static INT (*comp_func)() ;       /* current compare function */
    BINTREEPTR y ;                    /* remember where we are */

    comp_func = tree->compare_func ;
    if( startFlag ){

	/* find where we would put low_key if we were to put */
	/* it in the tree */
	y = nilS ;
	for( ptr = tree->root; ptr != nilS; ){
	    y = ptr ;  /* save the place for the proposed insert */
	    if( comp_f( low_key, ptr->data ) <= 0 ){
		ptr = ptr->left ;
	    } else {
		ptr = ptr->right ;
	    }
	}
	/* now see if we need to find successor */
	if( y != nilS ){
	    ptr = y ;
	    if( comp_f( low_key, ptr->data ) > 0 ){
		/* find successor to current ptr */
		ptr = tree_suc( ptr ) ;
	    }
	}

    } else {
      /* find successor to current ptr */
      ptr = tree->intervalPtr;        /* get old current pointer */
      if( ptr == recalcS ){
	/* calculate from the beginning */
	return( Yrbtree_interval( tree, low_key, high_key, TRUE )) ;
      } else {
	ptr = tree_suc( ptr ) ;
      }
    }
    /* is pointer in interval? */
    if( ptr != nilS &&
       comp_f( ptr->data, high_key ) > 0 ){
      ptr = nilS ;
    }
    tree->intervalPtr = ptr;         /* save new current position */
    if( ptr != nilS ){
	return( ptr->data ) ;
    }
    return( NIL(VOIDPTR) ) ;
} /* end Ytree_interval */


INT Yrbtree_interval_size( tree, low_key, high_key )
YTREEPTR tree ;
VOIDPTR low_key, high_key ;
{
    static INT (*comp_func)() ;       /* current compare function */
    INT size ;			      /* number of element in interval */
    BINTREEPTR ptr ;                  /* remember where we are */
    BINTREEPTR y ;                    /* remember where we are */

    comp_func = tree->compare_func ;
    size = 0 ;
    /* -----------------------------------------------------------------
    *  Find where we would put low_key if we were to put it in the tree.
    ----------------------------------------------------------------- */
    for( y = nilS, ptr = tree->root; ptr != nilS; ){
	y = ptr ;  /* save the place for the proposed insert */
	if( comp_f( low_key, ptr->data ) <= 0 ){
	    ptr = ptr->left ;
	} else {
	    ptr = ptr->right ;
	}
    }
    /* now see if we need to find successor */
    if( y != nilS ){
	ptr = y ;
	if( comp_f( low_key, ptr->data ) > 0 ){
	    /* find successor to current ptr */
	    ptr = tree_suc( ptr ) ;
	}
    }
    /* is pointer in interval? */
    if( ptr != nilS && comp_f( ptr->data, high_key ) > 0 ){
	ptr = nilS ;
    }
    while( ptr && ptr != nilS ){
	size++ ;
	ptr = tree_suc( ptr ) ;
	/* is pointer in interval? */
	if( ptr != nilS && comp_f( ptr->data, high_key ) > 0 ){
	    ptr = nilS ;
	}
    }
    return( size ) ;
} /* end Ytree_interval_size() */


/* Push the current interval pointer.  This is useful  */
/* for recursive intervals                             */
VOID Yrbtree_intervalPush ( tree )
YTREEPTR tree;
{
   if (!(tree->intervalDeck)) {
     tree->intervalDeck = Ydeck_init();
   }

   Ydeck_push(tree->intervalDeck,tree->intervalPtr);
} 

/* Pop the current interval pointer.  This is useful  */
/* for recursive intervals                             */
VOID Yrbtree_intervalPop ( tree )
YTREEPTR tree;
{
   if (!(tree->intervalDeck)) {
     M(ERRMSG,"Yrbtree_intervalPop","There has been no push\n");
   }
   
   if ( Ydeck_size(tree->intervalDeck) <= 0 ) {
     M(ERRMSG,"Yrbtree_intervalPop","Imbalance of push/pop\n");
   }
 
    tree->intervalPtr = (BINTREEPTR) Ydeck_pop(tree->intervalDeck);
} /* end Yrbtree_intervalPop() */

VOID Yrbtree_interval_free( tree, low_key, high_key, userDelete )
YTREEPTR tree ;
VOIDPTR low_key, high_key ;
VOID (*userDelete)();
{

  VOIDPTR data ;

  for (data = Yrbtree_interval( tree, low_key, high_key, TRUE); data;
       data = Yrbtree_interval( tree, low_key, high_key, FALSE) ) {
    Yrbtree_deleteCurrentInterval( tree, userDelete ) ;
  }

} /* end Yrbtree_interval_free() */

static tree_free( ptr )
BINTREEPTR ptr ;
{

    if( ptr->left != nilS ){
	tree_free( ptr->left ) ;
    } 
    if( ptr->right != nilS ){
	tree_free( ptr->right ) ;
    } 
    if( ptr != nilS ){
	YFREE( ptr ) ;
    }
} /* end tree_free */

static free_tree_and_data( ptr, userDelete )
BINTREEPTR ptr ;
VOID (*userDelete)();
{

    if( ptr->left != nilS ){
	free_tree_and_data( ptr->left,userDelete ) ;
    } 
    if( ptr->right != nilS ){
	free_tree_and_data( ptr->right,userDelete ) ;
    } 
    if( ptr != nilS ){
        (*userDelete)( ptr->data );  /* free tree nodes and data */
	YFREE( ptr ) ;
    }
} /* end tree_free */

VOID Yrbtree_free( tree, userDelete )
YTREEPTR tree ;
VOID (*userDelete)();
{
    if( tree ){
	Yrbtree_empty( tree,userDelete ) ;
	YFREE( tree );                           
    }
} /* end Yrbtree_free */

VOID Yrbtree_empty( tree, userDelete )
YTREEPTR tree ;
VOID (*userDelete)();
{
  if( tree ){
    if (tree->root != nilS) {             /* does tree need to be emptied? */
      if (userDelete) {
	free_tree_and_data(tree->root,userDelete);   /* free tree and data */
      } else {
	tree_free(tree->root);            /* free tree nodes only */
      }
      tree->root = nilS ;                 /* tree is now empty */
    }                       /* end if tree->root */
  }                         /* end if tree       */
  tree->size = 0;
  /* free decks associated with tree if they exist */
  if( tree->enumerateDeck ){
    Ydeck_free( tree->enumerateDeck, NULL ) ;
    tree->enumerateDeck = NIL(YDECKPTR) ; 
  }
  if( tree->intervalDeck ){
    Ydeck_free( tree->intervalDeck, NULL ) ;
    tree->intervalDeck = NIL(YDECKPTR) ; 
  }
} /* end Yrbtree_empty() */


INT Yrbtree_size( tree )
YTREEPTR tree ;
{
    INT size;

    if( tree ){
      if ( tree->size >= 0 ) {
	return( tree->size ) ;
      }
      M(ERRMSG,"Yrbtree_size","Bogus rbTree size\n");
    } else {
      M(ERRMSG,"Yrbtree_size","NULL rbTree pointer\n");
    }
    return (-1);
} /* end Yrbtree_size */

INT (*Yrbtree_get_compare( tree ))()
YTREEPTR tree;
{
  return ( tree->compare_func );
}

INT Yrbtree_verify(tree)
     YTREEPTR tree ;
{
  INT count;
  INT rc;
  BINTREEPTR ptr;

  /* exercise tree pointers */
  
  rc = TRUE;
  count = 0;
 
  if ( YcheckDebug(tree) < sizeof(YTREEBOX) ) {
      M(ERRMSG,"Yrbtree_verify","tree memory is corrupt\n");
      rc = FALSE;
  }

  /* get first item in tree */
  for( ptr = tree->root; ptr != nilS && ptr->left != nilS; ptr = ptr->left ) ;
  /* walk tree */
  for ( ; ptr != nilS; ptr = tree_suc( ptr ) ) {
    if ( YcheckDebug(ptr) < sizeof(BINTREE) ) {
      M(ERRMSG,"Yrbtree_verify","tree node memory is corrupt\n");
      rc = FALSE;
    }
    count ++;
  }
  
  /* if tree is corrupt, we may still get out of loop */
  /* check the size field */
  if ( count != Yrbtree_size(tree) ) {
      M(ERRMSG,"Yrbtree_verify","Bogus rbTree. Data structure is corrupt\n");
      rc = FALSE;
  } 

  return ( rc );
}

VOID Yrbtree_resort( tree, compare_func )
YTREEPTR tree ;
INT (*compare_func)() ;  /* user specifies key function */
{
    BINTREEPTR ptr ;          /* remember where we are */
    YTREEPTR new_tree ;       /* the new tree being built */

    new_tree = Yrbtree_init( compare_func ) ;
  
    /* first find the leftmost pointer */
    for( ptr=tree->root;ptr!=nilS && ptr->left != nilS; ptr=ptr->left ) ;

    /* now enumerate all the members of the tree in order */
    for( ; ptr && ptr != nilS ; ptr = tree_suc( ptr ) ) {
	if( ptr != nilS ){
	    /* build new tree */
	    Yrbtree_insert( new_tree, ptr->data ) ;
	}
    }

    /* free the data in the old tree */
    Yrbtree_empty( tree, NULL ) ;

    /* now copy over tree data */
    tree->root = new_tree->root ;
    tree->size = new_tree->size ;
    tree->intervalPtr = NIL(BINTREEPTR) ;
    tree->enumeratePtr = NIL(BINTREEPTR) ;
    tree->searchPtr = NIL(BINTREEPTR) ;
    tree->compare_func = compare_func ;
    /* now free new tree */
    YFREE( new_tree ) ;

} /* end Yrbtree_resort */

YTREEPTR Yrbtree_copy( tree, compare_func )
YTREEPTR tree ;
INT (*compare_func)() ;  /* user specifies key function */
{
    BINTREEPTR ptr ;          /* remember where we are */
    YTREEPTR new_tree ;       /* the new tree being built */

    new_tree = Yrbtree_init( compare_func ) ;
  
    /* first find the leftmost pointer */
    for( ptr=tree->root;ptr!=nilS && ptr->left != nilS; ptr=ptr->left ) ;

    /* now enumerate all the members of the tree in order */
    for( ; ptr && ptr != nilS ; ptr = tree_suc( ptr ) ) {
	if( ptr != nilS ){
	    /* build new tree */
	    Yrbtree_insert( new_tree, ptr->data ) ;
	}
    }
    return( new_tree ) ;

} /* end Yrbtree_resort */


#ifdef TEST
/* ##################################################################
                       TEST OF TREE ROUTINES 
   ################################################################## */
typedef struct {
    INT  str_len ;
    char *name ;
} DATA, *DATAPTR ;

static compare_string( key1, key2 )
DATAPTR key1, key2 ;
{
    return( strcmp( key1->name, key2->name ) ) ;
} /* end compare */

static compare_len( key1, key2 )
DATAPTR key1, key2 ;
{
    return( key1->str_len - key2->str_len ) ;
} /* end compare_len */

static VOID print_data( data )
DATAPTR data ;
{
    fprintf( stderr, "%s:%d ", data->name, data->str_len ) ;
} /* end print_data() */

static VOID free_data( data )
DATAPTR data ;
{
    YFREE( data->name ) ;
    YFREE( data ) ;
} /* end print_data() */

static char *make_data( string )
char *string ;
{
    DATAPTR data ;

    data = YMALLOC( 1, DATA ) ;
    data->str_len = strlen(string) + 1 ;
    data->name = YMALLOC( data->str_len, char ) ;
    strcpy( data->name, string ) ;
    return( (char *) data ) ;
} /* end make_data */

main()
{

    YTREEPTR tree1 ;
    DATAPTR data ;
    static char buffer1[LRECL] ;
    static char buffer2[LRECL] ;
    INT count ;
    DATA key1, key2 ;

    key1.name = buffer1 ;
    key2.name = buffer2 ;

    fprintf( stderr, "Initial memory:%d\n", YgetCurMemUse() ) ;

    tree1 = Yrbtree_init( compare_string ) ;
    Yrbtree_insert( tree1, make_data("the" ) ) ;
    Yrbtree_insert( tree1, make_data("silver" ) ) ;
    Yrbtree_insert( tree1, make_data("fox" ) ) ;
    Yrbtree_insert( tree1, make_data("jumped" ) ) ;
    Yrbtree_insert( tree1, make_data("over" ) ) ;
    Yrbtree_insert( tree1, make_data("the" ) ) ;
    Yrbtree_insert( tree1, make_data("moon" ) ) ;
    Yrbtree_verify(tree1) ;
    fprintf( stderr, "Output the initial tree\n" ) ;
    for( data=(DATAPTR)Yrbtree_enumerate(tree1,TRUE); data; 
	data=(DATAPTR)Yrbtree_enumerate(tree1,FALSE)){
	fprintf( stderr, "key:%s\n", data->name ) ;
    }
    fprintf( stderr, "\nLook for jumped...\n" ) ;
    strcpy( key1.name, "jumped" ) ;
    data = (DATAPTR) Yrbtree_search( tree1, &key1 ) ;
    Yrbtree_verify(tree1) ;
    if( data ){
	fprintf( stderr, "key:%s\n", data->name ) ;
    }
    Yrbtree_dump( tree1, print_data ) ;
    fprintf( stderr, "\nDelete jumped...\n" ) ;
    strcpy( key2.name, "jumped" ) ;
    Yrbtree_delete( tree1, &key2, free_data ) ;
    Yrbtree_verify(tree1) ;
    Yrbtree_dump( tree1, print_data ) ;

    for( data=(DATAPTR)Yrbtree_enumerate(tree1,TRUE); 
	data;
	data=(DATAPTR)Yrbtree_enumerate(tree1,FALSE)){
	fprintf( stderr, "key:%s\n", data->name ) ;
    }
    fprintf( stderr, "\nInsert takeover and try to delete history...\n" );
    Yrbtree_insert( tree1, make_data("takeover" ) ) ;
    strcpy( key1.name, "history" ) ;
    if( Yrbtree_delete( tree1, &key1, free_data ) ){
	fprintf( stderr, "found history\n" ) ;
    } else {
	fprintf( stderr, "didn't find history\n" ) ;
    }
    strcpy( key1.name, "fox" ) ;
    Yrbtree_delete( tree1, &key1, free_data ) ;
    fprintf( stderr, "\nNow show result...\n" );
    for( data=Yrbtree_enumerate(tree1,TRUE); data; 
	data=Yrbtree_enumerate(tree1,FALSE)){
	fprintf( stderr, "key:%s\n", data->name ) ;
    }

    fprintf( stderr, "\nNow find interval 'over' to 'the'\n" );
    strcpy( key1.name, "over" ) ;
    strcpy( key2.name, "the" ) ;
    for( data=Yrbtree_interval(tree1,&key1,&key2,TRUE); data; 
	data=Yrbtree_interval(tree1,&key1,&key2,FALSE)){
	fprintf( stderr, "key:%s\n", data->name ) ;
    }
    fprintf( stderr, "The interval size is:%d\n",
	Yrbtree_interval_size( tree1, &key1, &key2 ) ) ;

    fprintf( stderr, "\nInsert zoo and delete the two the's...\n" );
    Yrbtree_insert( tree1, make_data("zoo" ) ) ;
    strcpy( key1.name, "the" ) ;
    strcpy( key2.name, "the" ) ;
    for( data=Yrbtree_interval(tree1,&key1,&key2,TRUE); data; 
	data=Yrbtree_interval(tree1,&key1,&key2,FALSE)){
	fprintf( stderr, "key:%s\n", data->name ) ;
	Yrbtree_deleteCurrentInterval( tree1, free_data ) ;
    }

    fprintf( stderr, "\nNow dump the tree\n" );
    Yrbtree_dump( tree1, print_data ) ;

    fprintf( stderr, "\nInsert two airplanes and then delete...\n" );
    Yrbtree_insert( tree1, make_data("airplane" ) ) ;
    Yrbtree_insert( tree1, make_data("airplane" ) ) ;
    fprintf( stderr, "\nLook for airplanes\n" );
    Yrbtree_dump( tree1, print_data ) ;

    /* now delete them */
    strcpy( key1.name, "airplane" ) ;
    strcpy( key2.name, "airplane" ) ;
    for( data=Yrbtree_interval(tree1,&key1,&key2,TRUE); data; 
	data=Yrbtree_interval(tree1,&key1,&key2,FALSE)){
	fprintf( stderr, "key:%s\n", data->name ) ;
	Yrbtree_deleteCurrentInterval( tree1, free_data ) ;
    }
    Yrbtree_dump( tree1, print_data ) ;
    fprintf( stderr, "The interval size is:%d\n",
	Yrbtree_interval_size( tree1, &key1, &key2 ) ) ;

    fprintf( stderr, "\nDelete third sorted element in the tree\n" );
    for( count = 0, data=Yrbtree_enumerate(tree1,TRUE); data; 
	data=Yrbtree_enumerate(tree1,FALSE)){
	if( ++count == 3 ){
	    Yrbtree_deleteCurrentEnumerate( tree1, free_data ) ;
	}
    }
    Yrbtree_dump( tree1, print_data ) ;

    fprintf( stderr, "\nUse a new comparison function sort by length\n" );
    Yrbtree_resort( tree1, compare_len ) ;

    fprintf( stderr, "\nAgain dump the tree\n" );
    Yrbtree_dump( tree1, print_data ) ;

    fprintf( stderr, "\nNow free tree\n" );
    Yrbtree_free( tree1, free_data ) ;
    fprintf( stderr, "Final memory:%d\n", YgetCurMemUse() ) ;

    exit(0) ;
}

#endif /* TEST */
    
