/*
 *   Copyright (C) 1988-1992 Yale University
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
FILE:	    paths.c
DESCRIPTION:output length of paths
CONTENTS:   print_paths.
	    INT calc_incr_time( cell ) 
		INT cell ;
	    update_time( cell ) 
		INT cell ;
	    INT calc_incr_time2( cella, cellb ) 
		INT cella, cellb ;
	    update_time2() 
	    init_path_set() 
	    add2path_set( path ) 
		INT  path ;
	    PSETPTR enum_path_set()
	    clear_path_set() 
	    init_net_set() 
	    add2net_set( net ) 
		INT  net ;
	    BOOL member_net_set( net )
	    clear_net_set() 
	    INT dcalc_full_penalty(newtimepenal)
	    INT dcalc_path_len(path_num)
		INT path_num ;
DATE:	    Oct	22, 1988 
REVISIONS:  Dec  3, 1988 - completed timing driven code.
	    Jan 29, 1989 - added \n's for pretty output.
	    Mar 11, 1989 - added statistics to print_paths.
			 - added wirelength to print_paths.
	    Mar 13, 1989 - now output penalty in print_paths.
	    Thu Dec 20 00:23:46 EST 1990 - removed += operator.
	    Fri Jan 18 18:33:25 PST 1991 - updated output format.
	    Thu Apr 18 01:52:21 EDT 1991 - added OOS for out of spec
		for easier searching.
	    Sun Apr 21 21:25:12 EDT 1991 - added ignore keyword for
		analyze nets.  Renamed for library stat function.
	    Mon Aug 12 17:01:03 CDT 1991 - changed timing ASSERTIONS
		to D( ) constructs to speed execution time during
		debug mode.
	    Fri Nov  8 01:08:41 EST 1991 - rewrote pad output file
		for easier understanding.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) paths.c version 4.12 4/2/92" ;
#endif

/* #define ZERO_CHECK */

#include "standard.h"
#include "main.h"
#include <yalecad/debug.h>
#include "readnets.h"
#include <yalecad/message.h>
#include <yalecad/stat.h>

/* global variables */
extern  DOUBLE avg_timeG ;/* average random time penalty */
extern  DOUBLE avg_funcG ;/* average random wirelength penalty */

/* forward declarations */
INT dcalc_min_path_len() ;
INT dcalc_max_path_len() ;
INT dcalc_path_len(INT, INT);

static INT errorboundS = 0 ;

print_paths( ) 
{

    char filename[LRECL] ;
    INT i, pathLength ;
    INT length ;
    INT above, really_above, really_below, below, in ;
    INT check, way_above, way_below ; 
    INT really_way_above, really_way_below ;
    INT *stat ;
    INT xspan, yspan ;
    PATHPTR pptr, get_path_list() ;
    GLISTPTR net ;
    DBOXPTR nptr ;
    PINBOXPTR netptr ;
    FILE *fp ;
    DOUBLE mean ;
    DOUBLE paths ;
    INT num_paths ;
    INT penaltyS ;

    i = 0 ;
    above = 0 ;
    really_above = 0 ;
    way_above = 0 ;
    really_way_above = 0 ;
    below = 0 ;
    really_below = 0 ;
    way_below = 0 ;
    really_way_below = 0 ;
    in = 0 ;
    penaltyS = 0 ;
    /* get total number of paths in timing analysis */
    num_paths = get_total_paths() ;
    /* allocate the an array for calculating statistics */
    if( num_paths ){
	stat = (INT *) Ysafe_malloc( num_paths * sizeof(INT) ) ;
    }
    sprintf(filename, "%s.pth" , cktNameG ) ;
    fp = TWOPEN( filename , "w", ABORT ) ;
    /* first the lengths of the nets */
    fprintf( fp, "The paths:\n" ) ;
    fprintf( fp, "##############################################\n" ) ;
    for( pptr=get_path_list() ;pptr;pptr=pptr->next ){
	pathLength = 0 ;
	xspan = 0 ;
	yspan = 0 ;
	/* get pathLength for path */
	fprintf( fp, "\npath %3d:\n",++i ) ;
	for( net = pptr->nets ; net ; net = net->next ){
	    nptr = netarrayG[net->p.net] ;
	    fprintf( fp, "\t\t%s\n", nptr->name ) ;
	    length = (INT)(horizontal_path_weightG * (DOUBLE) nptr->halfPx) +
		 (INT)(vertical_path_weightG * (DOUBLE) nptr->halfPy) ;
	    pathLength += length ;
	    xspan += nptr->halfPx ;
	    yspan += nptr->halfPy ;
	}
	fprintf( fp, "\tpriority:%d lower_bound:%d upper_bound:%d\n",
	    pptr->priority, pptr->lower_bound, pptr->upper_bound ) ;
	fprintf( fp,"\tweighted length:%d\n", pathLength ) ;
	fprintf( fp, "\txspan:%d yspan:%d length:%d ", xspan, yspan,
	    xspan+yspan ) ;
	if( pptr->priority ){
	    ASSERT( pathLength == pptr->path_len,"print_paths",
		"pathLength does not equal incremental value " ) ;
	}

	check = FALSE ;
	for( net = pptr->nets ; net ; net = net->next ){
	    netptr = netarrayG[net->p.net]->pins ;
	    for( ; netptr ; netptr = netptr->next ) {
		if( netptr->cell > numcellsG ) {
		    check = TRUE ;
		    break ;
		}
	    }
	}
	if( pathLength < pptr->lower_bound ){
	    penaltyS += pptr->lower_bound - pathLength ;
	    fprintf( fp, "*" ) ;
	    if( check ) {
		fprintf( fp, " pad" ) ;
	    }
	    if( pathLength < (INT)( 0.90 * (DOUBLE) pptr->lower_bound) ){
		below++ ;

		if( !check ) {
		    really_below++ ;
		}
	    }
	    if( pathLength < (INT)( 0.80 * (DOUBLE) pptr->lower_bound) ){
		way_below++ ;
		fprintf( fp, " OOS" ) ;
		if( !check ) {
		    really_way_below++ ;
		}
	    }
	} else if( pathLength > pptr->upper_bound ){
	    penaltyS += pathLength - pptr->upper_bound ;
	    fprintf( fp, "*" ) ;
	    check = FALSE ;
	    for( net = pptr->nets ; net ; net = net->next ){
		netptr = netarrayG[net->p.net]->pins ;
		for( ; netptr ; netptr = netptr->next ) {
		    if( netptr->cell > numcellsG ) {
			check = TRUE ;
			break ;
		    }
		}
	    }
	    if( check ) {
		fprintf( fp, " pad" ) ;
	    }
	    if( pathLength > (INT)( 1.10 * (DOUBLE) pptr->upper_bound) ){
		above++ ;

		if( !check ) {
		    really_above++ ;
		}
	    }
	    if( pathLength > (INT)( 1.20 * (DOUBLE) pptr->upper_bound) ){
		way_above++ ;
		fprintf( fp, " OOS" ) ;
		if( !check ) {
		    really_way_above++ ;
		}
	    }
	} else {
	    in++ ;
	}
	fprintf( fp, "\n" ) ;
	/* add pathlength to stat array */
	stat[i-1] = pathLength ;
	
    }
    /* first the lengths of the nets */
    fprintf( fp, "\nThe nets:\n" ) ;
    fprintf( fp, "##############################################\n" ) ;
    for( i=1;i<=numnetsG;i++ ){
	nptr = netarrayG[i] ;
	fprintf( fp, "net %3d:%s xspan:%d yspan:%d length:%d numpins:%d",
	    i, nptr->name, nptr->halfPx, nptr->halfPy, 
	    (INT)(horizontal_path_weightG * (DOUBLE) nptr->halfPx) +
		 (INT)(vertical_path_weightG * (DOUBLE) nptr->halfPy),
	    nptr->numpins ) ;
	if( nptr->ignore ){
	    fprintf( fp, " ignored\n" ) ;
	} else {
	    fprintf( fp, "\n" ) ;
	}
    }
    /* avoid a divide by zero */
    if( num_paths ){
	paths = (DOUBLE) num_paths ;
	fprintf( fp, "\nSummary:\n" ) ;
	fprintf( fp, "Total wirelength               :%5d\n", funccostG ) ;
	fprintf( fp, "Total time penalty             :%5d\n", penaltyS ) ;
	fprintf( fp, "Number of paths                :%5d\n", num_paths ) ;
	fprintf( fp, "Number of active paths         :%5d\n", numpathsG ) ;
	fprintf( fp, "Number of paths 10%% below spec :%5d - %4.2f%%\n",
	    below, 100.0 * (DOUBLE) below / paths ) ;
	fprintf( fp, "Number of paths 10%% above spec :%5d - %4.2f%%\n",
	    above, 100.0 * (DOUBLE) above / paths ) ;
	fprintf( fp, "Number of paths within  spec   :%5d - %4.2f%%\n",
	    in, 100.0 * (DOUBLE) in / paths ) ;
	fprintf( fp, "# of non-pad paths out of spec :%5d\n", really_above +
						really_below ) ;
	fprintf( fp, "Number of paths 20%% below spec :%5d - %4.2f%%\n",
	    way_below, 100.0 * (DOUBLE) way_below / paths ) ;
	fprintf( fp, "Number of paths 20%% above spec :%5d - %4.2f%%\n",
	    way_above, 100.0 * (DOUBLE) way_above / paths ) ;
	fprintf( fp, "# of non-pad paths out of spec :%5d\n", 
				really_way_above + really_way_below ) ;
	fprintf( fp, "Min  length                    :%4.2le\n",
	    Ystat_min( stat, num_paths, sizeof(INT) ) ) ;
	fprintf( fp, "Max  length                    :%4.2le\n",
	    Ystat_max( stat, num_paths, sizeof(INT) ) ) ;
	mean = Ystat_mean( stat, num_paths, sizeof(INT) ) ;
	fprintf( fp, "Mean length                    :%4.2le\n", mean ) ;
	fprintf( fp, "Standard Dev. length           :%4.2le\n",
	    sqrt( Ystat_var( stat, num_paths, sizeof(INT),mean) ) ) ;
	Ysafe_free( stat ) ;
    }
    TWCLOSE( fp ) ;

} /* end print_paths */

/* ----------------------------------------------------------------- 
    The update path routines
    The incremental routines speed execution by looking at only those
    paths connected to the cells that are to be moved.  In addition,
    only those nets which are modified by the move are updated.  This
    gives us an incremental length which we add to old half perimeter
    to give us total lenght of this net.  The penalty is based on this
    length.  We calculate an incremental penalty based on the change
    in the penalty.
----------------------------------------------------------------- */
/* calculate the timing cost incrementally */
INT calc_incr_time( cell ) 
INT cell ;
{
    INT newpenal ;        /* proposed move's timing penalty delta */
    INT oldpenal ;        /* the old penalty of the nets that move */
    INT path_num ;        /* name of path */
    INT net ;             /* net of path */
    INT length ;          /* path length incremental */
    GLISTPTR pptr ;       /* pointer to paths of a cell */
    GLISTPTR net_of_path ;
    PATHPTR path ;
    DBOXPTR dimptr ;

    newpenal = 0 ;
    oldpenal = 0 ;
    /* for all paths of the cell that has moved */
    for( pptr = carrayG[cell]->paths; pptr ; pptr = pptr->next ) {

	length = 0 ;
	path_num = pptr->p.path ;
	ASSERTNCONT( path_num > 0 && path_num <= numpathsG,
	    "calc_incr_time", "path is out of bounds" ) ;
	path = patharrayG[path_num] ;
	/* for all nets k of a path i */
	for( net_of_path=path->nets;net_of_path;
	    net_of_path=net_of_path->next ){
	    net = net_of_path->p.net ;

	    dimptr = netarrayG[net] ;

	    /* accumulate length of path */
	    if( member_net_set( net ) ){
		/* this half - perimeter has changed use update */
		/* calculate total change on path */
		length = length + (INT) 
		    (horizontal_path_weightG * (DOUBLE) dimptr->newhalfPx ) ;
		length = length - (INT) 
		    (horizontal_path_weightG * (DOUBLE) dimptr->halfPx ) ;
		length = length + (INT) 
		    (vertical_path_weightG * (DOUBLE) dimptr->newhalfPy ) ;
		length = length - (INT) 
		    (vertical_path_weightG * (DOUBLE) dimptr->halfPy ) ;
	    } /* else this half - perimeter has not changed use old */
	}
	/* save total result - change to total length */
	path->new_path_len = length += path->path_len ;

        D( "twsc/check_timing",
	    ASSERT( dcalc_path_len(path_num,length),"update_time",
		"length mismatch" ) ;
	) ;

	/* calculate change in timing penalty */
	/* no penalty if within target window */
	/* lowerbound <= length <= upperbound */
	if( length > path->upper_bound ){
	    newpenal += length - path->upper_bound ; 
	} else if( length < path->lower_bound ){
	    newpenal += path->lower_bound - length ;
	}
	/* now the old penalty */
	if( path->path_len > path->upper_bound ){
	    oldpenal += path->path_len - path->upper_bound ; 
	} else if( path->path_len < path->lower_bound ){
	    oldpenal += path->lower_bound - path->path_len ;
	}
    } /* end loop on the paths of a cell */
    /* return the change in penalty */
    return( newpenal - oldpenal ) ;

} /* end function calc_incr_time */


update_time( cell ) 
INT cell ;
{

    INT path_num ;        /* name of path */
    GLISTPTR pptr ;       /* pointer to paths of a cell */
    PATHPTR path ;

    /* for all paths of the cell that has moved */
    for( pptr = carrayG[cell]->paths; pptr ; pptr = pptr->next ) {

	path_num = pptr->p.path ;
	ASSERTNCONT( path_num > 0 && path_num <= numpathsG,
	    "update_time", "path is out of bounds" ) ;
	path = patharrayG[path_num] ;
	/* keep result */
	path->path_len = path->new_path_len ;
    }

} /* end function update_time */

/* calculate the timing cost incrementally for two cells */
INT calc_incr_time2( cella, cellb ) 
INT cella ;
INT cellb ;
{
    INT newpenal ;        /* proposed move's timing penalty delta */
    INT oldpenal ;        /* the old penalty of the nets that move */
    INT path_num ;        /* name of path */
    INT net ;             /* net of path */
    INT length ;          /* path length incremental */
    GLISTPTR pptr ;       /* pointer to paths of a cell */
    GLISTPTR net_of_path ;
    PATHPTR path ;
    PSETPTR pathlist, enum_path_set() ;
    DBOXPTR dimptr ;

    newpenal = 0 ;
    oldpenal = 0 ;
    clear_path_set() ;

    /* use add2path_set to get union of paths from cella and cellb */
    for( pptr = carrayG[cella]->paths; pptr ; pptr = pptr->next ) {
	add2path_set( pptr->p.path ) ;
    }
    for( pptr = carrayG[cellb]->paths; pptr ; pptr = pptr->next ) {
	add2path_set( pptr->p.path ) ;
    }


    /* now use UNIQUE list of the union of the two cell's paths */
    for( pathlist=enum_path_set(); pathlist; pathlist=pathlist->next){

	length = 0 ;
	path_num = pathlist->path ;
	ASSERTNCONT( path_num > 0 && path_num <= numpathsG,
	    "calc_incr_time2", "path is out of bounds" ) ;
	path = patharrayG[path_num] ;

	/* for all nets k of a path i */
	for( net_of_path=path->nets;net_of_path;
	    net_of_path=net_of_path->next ){
	    net = net_of_path->p.net ;

	    dimptr = netarrayG[net] ;

	    /* accumulate length of path */
	    if( member_net_set( net ) ){
		/* this half - perimeter has changed use update */
		/* calculate total change on path */
		length = length + (INT) 
		    (horizontal_path_weightG * (DOUBLE) dimptr->newhalfPx ) ;
		length = length - (INT) 
		    (horizontal_path_weightG * (DOUBLE) dimptr->halfPx ) ;
		length = length + (INT) 
		    (vertical_path_weightG * (DOUBLE) dimptr->newhalfPy ) ;
		length = length - (INT) 
		    (vertical_path_weightG * (DOUBLE) dimptr->halfPy ) ;
	    } /* else this half - perimeter has not changed use old */
	}
	/* save total result - change to total length */
	path->new_path_len = length += path->path_len ;

        D( "twsc/check_timing",
	    ASSERT( dcalc_path_len(path_num,length),"update_time",
		"length mismatch" ) ;
	) ;

	/* calculate change in timing penalty */
	/* no penalty if within target window */
	/* lowerbound <= length <= upperbound */
	if( length > path->upper_bound ){
	    newpenal += length - path->upper_bound ; 
	} else if( length < path->lower_bound ){
	    newpenal += path->lower_bound - length ;
	}
	/* now the old penalty */
	if( path->path_len > path->upper_bound ){
	    oldpenal += path->path_len - path->upper_bound ; 
	} else if( path->path_len < path->lower_bound ){
	    oldpenal += path->lower_bound - path->path_len ;
	}
    } /* end loop on paths */
    /* return the change in penalty */
    return( newpenal - oldpenal ) ;

} /* end function calc_incr_time2 */

update_time2() 
{
    PATHPTR path ;
    PSETPTR pathlist, enum_path_set() ;

    /* now use UNIQUE list of the union of the two cell's paths */
    for( pathlist=enum_path_set(); pathlist; pathlist=pathlist->next){

	ASSERTNCONT( pathlist->path > 0 && pathlist->path <= numpathsG,
	    "update_time2", "path is out of bounds" ) ;
	path = patharrayG[pathlist->path] ;
	/* keep result */
	path->path_len = path->new_path_len ;
    }

} /* end function update_time */
/* ----------------------------------------------------------------- 
   The set required in the update path routines needs the following
   operations done efficiently :
       membership - O(1)
       add to set if unique - O(1)
       clear set  - O(1)
       set enumeration - O( paths in set )
Below is a space hungry O( numpaths ) but time efficient implementation
We combine two data structures to accomplish this goal. We use array 
for testing membership and uniqueness but use list for enumeration.
By incrementing count we can reinitialize easily.
Note: PSETBOX, PSETPTR definitions are in custom.h
----------------------------------------------------------------- */
static PSETPTR path_set_listS ;   /* list is beginning of set as a list */
static PSETPTR *path_set_arrayS ; /* set is an array of path set boxes */
static INT path_set_countS ;      /* current set count */

/* initialize set */
init_path_set() 
{   
    INT i ;

    path_set_arrayS=(PSETPTR *)Ysafe_malloc((numpathsG+1)*sizeof(PSETPTR));
    for( i=0;i<=numpathsG;i++ ){
	path_set_arrayS[i] = (PSETPTR) Ysafe_calloc( 1, sizeof(PSETBOX) ) ;
    }
    path_set_countS = 1 ;
    path_set_listS = NULL ;
} /* end initset */

/* add a path to the set if not already in set */
add2path_set( path ) 
INT  path ;
{  
    PSETPTR temp, cpath ;

    if( path >= 1 && path <= numpathsG ){
	cpath = path_set_arrayS[path] ;
	/* something is a member in set is counts match */
	if( cpath->member != path_set_countS ){
	    /* new path to be added */
	    /* connect to the single linked list */
	    if( temp = path_set_listS ){
		/* hook to old list */
		path_set_listS = cpath ;
		cpath->next = temp ;
	    } else {
		path_set_listS = cpath ;
		/* terminate new list */
		cpath->next = NULL ;
	    }
	    cpath->path = path ; /* store data */
	    cpath->member = path_set_countS ; /* store membership */
	}

    } else {
	M( ERRMSG, "ADD2SET","value of path is out of bounds of set\n" ) ;
    }
} /* end add2path_set */

PSETPTR enum_path_set()
{
    return( path_set_listS ) ;
}

clear_path_set() 
{
    path_set_countS ++ ;
    path_set_listS = NULL ;
} /* end clear_path_set */

/* ----------------------------------------------------------------- 
   Below is a second set of set routines required in the update path
   routines which operate on nets instead of paths.  The following
   operations need to be done efficiently :
       membership - O(1)
       add to set if unique - O(1)
       clear set  - O(1)
   Since we don't need to enumerate set we only need array to implement
   this set.  
----------------------------------------------------------------- */
static INT *net_set_array ; /* set is an array of net set boxes */
static INT net_set_count ;      /* current set count */

/* initialize set */
init_net_set() 
{   
    net_set_array = (INT *) Ysafe_calloc((numnetsG+1), sizeof(INT) );
    net_set_count = 1 ;
} /* end initset */

/* add a net to the set if not already in set */
add2net_set( net ) 
INT  net ;
{  
    if( net >= 1 || net <= numnetsG ){
	/* current count make array member a valid member of set */
	net_set_array[net] = net_set_count ;

    } else {
	M( ERRMSG, "ADD2SET", "value of path is out of bounds of set" ) ;
    }
} /* end add2net_set */

BOOL member_net_set( net )
/* test for membership */
{
    if( net_set_array[net] == net_set_count ){
	return( TRUE ) ;
    } else {
	return( FALSE ) ;
    }
} /* end member_net_set */

clear_net_set() 
{
    /* to clear set we only need to increment net_set_count */
    /* we can use this set up to 2 Gig times without any problem */
    net_set_count ++ ;
} /* end clear_path_set */

#ifdef DEBUG
/* *************** DEBUG FUNCTIONS *************************** */
/* debug function to make sure calculation is correct */
INT dcalc_full_penalty( newtimepenal )
INT newtimepenal ;
{
    INT timingpenal ;
    INT pathcount ;
    INT length ;          /* path length incremental */
    INT net ;             /* net of path */
    GLISTPTR net_of_path ;
    PATHPTR path ;
    DBOXPTR dimptr ;

    /* now calculate the penalties */
    /* first the timing penalty */
    timingpenal = 0 ;
    for( pathcount = 1 ; pathcount <= numpathsG ; pathcount++ ) {

	path = patharrayG[pathcount] ;
	ASSERTNCONT( path, "dcalc_full_penalty", 
	    "pointer to path is NULL" ) ;
	/* for all nets k of a path i */
	length = 0 ;
	for( net_of_path=path->nets;net_of_path;
	    net_of_path=net_of_path->next ){
	    net = net_of_path->p.net ;
	    dimptr = netarrayG[net] ;

	    /* accumulate length of path */
	    if( member_net_set( net ) ){
		/* this half - perimeter has changed use update */
		/* calculate total change on path */
		length = length + (INT) 
		    (horizontal_path_weightG * (DOUBLE) dimptr->newhalfPx ) ;
		length = length + (INT) 
			 (vertical_path_weightG * (DOUBLE) dimptr->newhalfPy) ;
	    } else {
		/* old length */
		length = length + (INT) 
		    (horizontal_path_weightG * (DOUBLE) dimptr->halfPx) ;
		length = length + (INT) 
		    (vertical_path_weightG * (DOUBLE) dimptr->halfPy ) ;
	    }
	}
	/* calculate penalty */
	/* no penalty if within target window */
	/* lowerbound <= length <= upperbound */
	if( length > path->upper_bound ){
	    timingpenal += length - path->upper_bound ; 
	} else if( length < path->lower_bound ){
	    timingpenal += path->lower_bound - length ;
	}
    }
    if( ABS( timingpenal - newtimepenal ) > errorboundS ){
	errorboundS = ABS( timingpenal - newtimepenal ) ;
	return( FALSE ) ;
    } else {
	return( TRUE ) ;
    }
}

INT dcalc_path_len(path_num,verify_length)
INT path_num ;
INT verify_length ;
{

    INT net ;             /* net of path */
    INT length ;          /* path length incremental */
    GLISTPTR net_of_path ;
    PATHPTR path ;
    DBOXPTR dimptr ;

    path = patharrayG[path_num] ;
    /* for all nets k of a path i */
    length = 0 ;
    for( net_of_path=path->nets;net_of_path;
	net_of_path=net_of_path->next ){
	net = net_of_path->p.net ;
	dimptr = netarrayG[net] ;

	/* accumulate length of path */
	if( member_net_set( net ) ){
	    /* this half - perimeter has changed use update */
	    /* calculate total change on path */
	    length = length + (INT) 
		(horizontal_path_weightG * (DOUBLE) dimptr->newhalfPx);
	    length = length + (INT) 
		(vertical_path_weightG * (DOUBLE) dimptr->newhalfPy) ;
	} else {
	    /* old length */
	    length = length + (INT) 
		(horizontal_path_weightG * (DOUBLE) dimptr->halfPx ) ;
	    length = length + (INT) 
		(vertical_path_weightG * (DOUBLE) dimptr->halfPy ) ;
	}
    }
    if( ABS( length - verify_length ) > errorboundS ){
	errorboundS = ABS( length - verify_length ) ;
	return( FALSE ) ;
    } else {
	return( TRUE ) ;
    }
}


INT dpath_len( net_num, old_not_new )
INT net_num ;
BOOL old_not_new ;
{

    INT net ;             /* net of path */
    INT length ;          /* path length incremental */
    INT timingpenal ;
    GLISTPTR net_of_path ;
    PATHPTR path ;
    GLISTPTR pptr ;       /* pointer to paths of a cell */
    DBOXPTR dimptr ;

    timingpenal = 0 ;
    for( pptr = netarrayG[net_num]->paths; pptr ; pptr = pptr->next ) {

	path = patharrayG[pptr->p.path] ;
	/* for all nets k of a path i */
	length = 0 ;
	for( net_of_path=path->nets;net_of_path;
	    net_of_path=net_of_path->next ){
	    net = net_of_path->p.net ;
	    dimptr = netarrayG[net] ;

	    /* accumulate length of path */
	    if( old_not_new ){
		/* old length */
		length = length + (INT) 
		    (horizontal_path_weightG * (DOUBLE) dimptr->halfPx ) ;
		length = length + (INT) 
		    (vertical_path_weightG * (DOUBLE) dimptr->halfPy ) ;
	    } else {
		/* this half - perimeter has changed use update */
		/* calculate total change on path */
		length = length + (INT) 
		    (horizontal_path_weightG * (DOUBLE) dimptr->newhalfPx);
		length = length + (INT) 
		    (vertical_path_weightG * (DOUBLE) dimptr->newhalfPy) ;

	    }
	}
	/* calculate penalty */
	/* no penalty if within target window */
	/* lowerbound <= length <= upperbound */
	if( length > path->upper_bound ){
	    timingpenal += length - path->upper_bound ; 
	} else if( length < path->lower_bound ){
	    timingpenal += path->lower_bound - length ;
	}
    } /* end for( pptr = netarrayyG[net_num]->paths... */
    return( timingpenal ) ;

} /* end INT dpath_len() */

INT dprint_error()
{
    sprintf( YmsgG, 
	"\n\nWe found that the timing error was bound by :%d\n\n", 
	errorboundS ) ;
    M( MSG, "dprint_error", YmsgG ) ;
    return( 0 ) ;
} /* end dprint_error() */

dverify_nets()
{

    INT net ;             /* net of path */
    INT per ;
    DBOXPTR dimptr ;

    for( net=1; net <= numnetsG; net++ ){
	dimptr = netarrayG[net] ;
	per = dimptr->xmax - dimptr->xmin ;
	ASSERTNCONT( per == dimptr->halfPx, "dverify_nets", "problem") ;
	per = dimptr->ymax - dimptr->ymin ;
	ASSERTNCONT( per == dimptr->halfPy, "dverify_nets", "problem") ;
    }
} /* end dverify_nets */

dprint_paths( cell )
{
    INT path_num ;        /* name of path */
    INT net ;             /* net of path */
    GLISTPTR pptr ;       /* pointer to paths of a cell */
    GLISTPTR net_of_path ;
    PATHPTR path ;
    DBOXPTR dimptr ;

    /* for all paths of the cell that has moved */
    for( pptr = carrayG[cell]->paths; pptr ; pptr = pptr->next ) {

	path_num = pptr->p.path ;
	path = patharrayG[path_num] ;
	fprintf( stderr, "path:%d, cur_len:%d new_len:%d\n",
	    path_num, path->path_len, path->new_path_len ) ;
	/* for all nets k of a path i */
	for( net_of_path=path->nets;net_of_path;net_of_path=net_of_path->next ){
	    net = net_of_path->p.net ;

	    dimptr = netarrayG[net] ;
	    fprintf( stderr, "  net:%4d Px:%6d nPx:%6d Py:%6d nPy:%6d ",
		net, dimptr->halfPx, dimptr->newhalfPx, dimptr->halfPy,
		dimptr->newhalfPy ) ;

	    /* accumulate length of path */
	    if( member_net_set( net ) ){
		fprintf( stderr, "*" ) ;
	    }
	    fprintf( stderr, "\n" ) ;
	} /* end for( net_of_path... */
    } /* end for( pptr... */
} /* end dprint_paths() */

dprint_net_set()
{
    INT net ;

    fprintf( stderr, "Current net set:\n" ) ;
    for( net = 1; net <= numnetsG; net++ ){
	if( member_net_set( net ) ){
	    fprintf( stderr, "%d ", net ) ;
	}
    }
    fprintf( stderr, "\n" ) ;
} /* end dprint_net_set() */

#endif /* DEBUG */



#define TIMEDAMPFACTOR   1.0     /* damping factor on time penalty */

DOUBLE calc_time_factor() 
{
    /* **** timing penalty controller **** */

#ifdef ZERO_CHECK
    timeFactorG = 0.0 ;
#else
    if( avg_timeG == 0.0 ) {
	timeFactorG = 3.0 ;
    } else {
	timeFactorG = 3.0 * (DOUBLE) avg_funcG / (DOUBLE) avg_timeG ;
    }
#endif
    return( timeFactorG ) ;

} /* end function calc_time_penalty */



calc_init_timeFactor() 
{

#ifdef ZERO_CHECK
    timeFactorG = 0.0 ;
#else
    timeFactorG = 4.0 ;
#endif
    fprintf(fpoG,"\n\nTIMING FACTOR (COMPUTED) : %f\n", timeFactorG ) ;

} /* end calc_init_timeFactor */
