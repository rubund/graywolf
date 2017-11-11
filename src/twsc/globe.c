/*
 *   Copyright (C) 1989-1992 Yale University
 *   Copyright (C) 2015 Tim Edwards <tim@opencircuitdesign.com>
 *   Copyright (C) 2015 Staf Verhaegen <staf@stafverhaegen.be>
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
FILE:	    globe.c                                       
DESCRIPTION:global routing routines.
CONTENTS:   globe() 
	    preFeedAssgn()
	    free_static_in_globe()
	    FeedAssgn( row )
		int row ;
	    row_seg_intersect( ptr1 , ptr2 , segptr )
		PINBOXPTR ptr1 , ptr2 ;
		SEGBOXPTR segptr ;
	    copy_workerS_field( aptr, bptr )
		FEED_SEG_PTR aptr, bptr ;
	    assgn_impin( imptr , fsptr , row )
		IPBOXPTR imptr ;
		FEED_SEG_PTR fsptr ;
	    unequiv_pin_pre_processing()
	    relax_padPins_pinloc()
	    relax_unequiv_pinloc()
	    check_unequiv_connectivity()
DATE:	    Mar 27, 1989 
REVISIONS:  Sat Dec 15 22:08:21 EST 1990 - modified pinloc values
		so that it will always be positive.
	    Tue Jan 15 20:24:49 PST 1991 - added Carl's speedup
		to adding feeds and also fixed missing Ysafe_frees.
	    Fri Jan 25 23:50:09 PST 1991 - now add extra memory
		if needed to worker and L_jog arrays.
	    Tue Mar 12 17:10:47 CST 1991 - fixed DN10000 warnings.
	    Thu Aug 22 22:29:28 CDT 1991 - Carl made changes
		for rigidly fixed cells.
	    Wed Aug 28 14:37:40 EDT 1991 - Carl updated feed assignment
		code.
	    Thu Sep 19 14:15:51 EDT 1991 - added equal width cell
		capability.  Fixed initialization problem with
		rebuilding pins.
	    Thu Nov  7 22:56:29 EST 1991 - added timing cost to
		cell swaps.
	    Wed Dec 18 21:09:09 EST 1991 - handle special case of 
		large number of feeds and small number of crossings.
	    Tue May 12 22:23:31 EDT 1992 - fixed problem with orientation
		movement and added placement_improve switch.
----------------------------------------------------------------- */
#define GLOBE_VARS

#include <globals.h>
#include "allheaders.h"

#define CARL_NEW
#define PICK_int(l,u) (((l)<(u)) ? ((RAND % ((u)-(l)+1))+(l)) : (l))

/* global variables */
BOOL connectFlagG ;
int implicit_pins_usedG ;

static long int swap_cost(BOOL perim_flag) ;

/* static variables */
static int *wk_headS , max_feed_in_a_rowS ;
static int *L_jogS ;
static FEED_SEG_PTR *workerS ;
static int wkS ;
static long int global_wire_lengthS ;
static int swap_limitS ;

int globe()
{
	int row , net , cost , last_cost , swaps , found , total_final_cost ;
	int total_cost , total_final_global_wire , total_global_wire ;
	int last_global_wire , total_reduction , index , i , j , k , check ;
	int iterations, initial_time, last_index ;
	long int initial_wire ;
	PINBOXPTR netptr , cnetptr ;
	int ok = 1;

	implicit_pins_usedG = 0 ;
	decide_boundary() ;
	link_imptr() ;
	preFeedAssgn() ;
	printf(" doing feed-through pins assignment\n" ) ;

	for( row = 1 ; row <= numRowsG ; row++ ) {
		FeedAssgn(row) ;
	}
	free_static_in_globe() ;

	printf(" building the net-tree now !\n" ) ;
	postFeedAssgn() ;

	rebuild_nextpin() ;
	elim_unused_feedsSC() ;

	/* if( rigidly_fixed_cellsG ) { */
	if( refine_fixed_placement() == 0 ) {
		ok = 0;
		goto out;
	}
	/* } */

	fprintf(fpoG,"\nrow lengths after steiner trees:\n");
	findunlap2() ;


	if( placement_improveG ){
		iterations = 0 ;
		total_reduction = 0 ;
		j = 4 * numcellsG ;
		if( numcellsG <= 2000 ) {
			k = 3 ;
		} else if( numcellsG <= 4000 ) {
			k = 2 ;
		} else {
			k = 1 ;
		}

		rebuild_cell_paths() ;
		initial_wire = global_wire_lengthS = swap_cost( FALSE ) ;
		initial_time = timingcostG ;
		sprintf(YmsgG,"initial total global wire   :\t%d\n", global_wire_lengthS);
		M( MSG, NULL, YmsgG ) ;
		sprintf(YmsgG,"initial total timing penalty:\t%d\n\n\n", timingcostG);
		M( MSG, NULL, YmsgG ) ;

		M( MSG, NULL, "\nSteiner cell swap and rotation optimization\n" ) ;
		swap_limitS = - (int)( (double) global_wire_lengthS * 0.00005 ) ;
		sprintf(YmsgG,"swap_limit:%d\n", swap_limitS ) ;
		M( MSG, NULL, YmsgG ) ;

		for( ; ; ) {
			swaps = 0 ;
			check = 0 ;
			for( i = 1 ; i <= j ; i++ ) {
			row = PICK_int( 1 , numRowsG ) ;

			/* added by Carl 12/7/91 */
			if( pairArrayG[row][0] <= 1 ) {
				check++ ;
				continue ;
			} /* added by Carl 12/7/91 */

			last_index = pairArrayG[row][0] ;
			index = PICK_int( 1 , last_index ) ;
			if( index < last_index ){
				if( carrayG[pairArrayG[row][index]]->cclass < 0 || carrayG[pairArrayG[row][index+1]]->cclass < 0 ) {
					i-- ;
					if( ++check > j ) {
						break ;
					} else {
						continue ;
					}
				}
				swaps += improve_place_sequential( row , index ) ;
			} /* end if( index <... */

			swaps += cell_rotate( row , index ) ;

			} /* end for( i = 1... */
			iterations++ ;
			total_reduction += swaps ;
			if( iterations == 1 || iterations % 10 == 0 ) {
				sprintf(YmsgG,"reduction:\t%d\t\ttotal_red:%d\n", swaps, total_reduction ) ;
				M( MSG, NULL, YmsgG ) ;
			}
			if( swaps >= swap_limitS ) {
				if( --k == 0 ) {
					break ;
				}
			}
		}

		sprintf(YmsgG,"iterations              :\t%d\n", iterations ) ;
		M( MSG, NULL, YmsgG ) ;
		sprintf(YmsgG,"final total global wire :\t%d\n", global_wire_lengthS ) ;
		M( MSG, NULL, YmsgG ) ;
		sprintf(YmsgG,"final total time penalty:\t%d\n", timingcostG ) ;
		M( MSG, NULL, YmsgG ) ;
		sprintf(YmsgG,"\nTotal global wire reduced by:\t%5.3f%%\n",
		100.0 * (1.0 - (double) global_wire_lengthS / (double) initial_wire ) ) ;
		M( MSG, NULL, YmsgG ) ;
		if( initial_time ){
			sprintf(YmsgG,"Total time penalty reduced by:\t%5.3f%%\n",
			100.0 * (1.0 - (double) timingcostG / (double) initial_time ) ) ;
			M( MSG, NULL, YmsgG ) ;
		}

		M( MSG, NULL, "\nVERIFICATION\n" ) ;

		global_wire_lengthS = swap_cost( FALSE ) ;
		sprintf(YmsgG,"final total global wire :\t%d\n", global_wire_lengthS);
		M( MSG, NULL, YmsgG ) ;
		sprintf(YmsgG,"final total time penalty:\t%d\n\n\n", timingcostG);
		M( MSG, NULL, YmsgG ) ;
	} else {
		M( WARNMSG, "globe", "global routing placement improvement off\n" ) ;
	}

	postFeedAssgn_carl() ;

	/* end of added stuff */


	relax_padPins_pinloc() ;
	if( case_unequiv_pinG ) {
		relax_unequiv_pinloc() ;
	}
	switchable_or_not() ;


	printf(" set up the global routing grids\n" ) ;
	globroute() ;
	assgn_channel_to_seg() ;
	connectFlagG = TRUE ;
	printf(" removing redundant feed-through pins\n" ) ;
	for( net = 1 ; net <= numnetsG ; net++ ) {
		remove_overlap_segment( net ) ;
		remove_unnecessary_feed( net , 1 ) ;
	}
	if( connectFlagG ) {
		printf(" the connectivity of all the nets is verified\n" ) ;
	} else {
		printf(" the connectivity of some nets is lost\n" ) ;
		printf(" please contact with authors\n" ) ;
	}
	if( case_unequiv_pinG ) {
		if( check_unequiv_connectivity() ) {
			printf(" the usage of all unequivalent pins is legal\n" );
		} else {
			printf(" illegal usage of some unequivalent pins\n" ) ;
			printf(" please contact with the authors\n" ) ;
		}
	}

	out:
	free_chan_seg() ;
	free_z_memory() ;

	return(ok) ;
}

void globe_free_up()
{
	netgraph_free_up();
}

void preFeedAssgn()
{

SEGBOXPTR segptr , nextptr ;
int i , net ;

max_feed_in_a_rowS = 3 * TotRegPinsG / numRowsG ;
wk_headS = (int *)Ysafe_malloc( max_feed_in_a_rowS * sizeof(int) ) ;
L_jogS = (int *)Ysafe_malloc( max_feed_in_a_rowS * sizeof(int) ) ;
workerS = (FEED_SEG_PTR *)Ysafe_malloc(
	    ( max_feed_in_a_rowS + 1 ) * sizeof(FEED_SEG_PTR) ) ;
for( i = 1 ; i <= max_feed_in_a_rowS ; i++ ) {
    workerS[i] = ( FEED_SEG_PTR )Ysafe_malloc( sizeof(FEED_SEG) ) ;
}
for( net = 1 ; net <= numnetsG ; net++ ) {
    segptr = netsegHeadG[net]->next ;
    if( segptr == NULL ) {
	continue ;
    }
    for( ; segptr ; segptr = nextptr ) {
	nextptr = segptr->next ;
	if( segptr->pin1ptr->row == segptr->pin2ptr->row &&
					    segptr->flag ) {
	    segptr->prev->next = nextptr ;
	    if( nextptr != NULL ) {
		nextptr->prev = segptr->prev ;
	    }
	    Ysafe_free( segptr ) ;
	}
    }
}
}

void free_static_in_globe()
{

int i ;

Ysafe_free( L_jogS ) ;
for( i = 1 ; i <= max_feed_in_a_rowS ; i++ ) {
    Ysafe_free( workerS[i] ) ;
}
Ysafe_free( workerS ) ;
Ysafe_free( wk_headS ) ;
Ysafe_free( total_feed_in_the_rowG ) ;
}

void FeedAssgn( int row )
{

PINBOXPTR netptr , ptr1 , ptr2 ;
SEGBOXPTR segptr , nextptr ;
IPBOXPTR imptr , iptr , ipinptr[40] , i_imptr , f_imptr ;
int net , impcount , firstnode , spacing ;
int i , j , k , last_i , last_j ;
int min_i , min_x , x , jog_num ;
int comparenptr() ;

/* added by Carl for the wild attempt to optimally align feeds */
int **cost , *assign_to_track , feed , feed_xpos , track , track_xpos ;
int v_tracks , assigned_track , *assigned_to_track ;
int num_parts , bound , assign_iter ;
int i_bound , f_bound , num_feeds , start_feed_xpos , end_feed_xpos ;
int count , i_count , f_count ;
int req_remaining_fds , remaining_fds , tot_imp_fds , num_assigned ;


wkS = 0 ;
jog_num = 0 ;
for( net = 1 ; net <= numnetsG ; net++ ) {
    for( netptr = steinerHeadG[net]->next ;
	netptr ; netptr = netptr->next ) {
	if( netptr->flag && netptr->row == row ) {
	    row_seg_intersect( netptr , NULL , NULL ) ;
	}
    }
    for( segptr = netsegHeadG[net]->next ; segptr ; segptr = nextptr ) {
	nextptr = segptr->next ;
	ptr1 = segptr->pin1ptr ;
	ptr2 = segptr->pin2ptr ;
	if( ptr1->row < row && ptr2->row > row ) {
	    if( segptr->switchvalue == swL_down ) {
		row_seg_intersect( ptr2 , ptr1 , segptr ) ;
	    } else {
		row_seg_intersect( ptr1 , ptr2 , segptr ) ;
	    }
	} else if( ptr1->row == row && ptr2->row == row ) {
	    if( segptr->switchvalue == swL_down ) {
		row_seg_intersect( ptr2 , ptr1 , segptr ) ;
	    } else if( segptr->switchvalue == swL_up ) {
		row_seg_intersect( ptr1 , ptr2 , segptr ) ;
	    } else if( ABS( segptr->pin1ptr->pinloc - 
			segptr->pin2ptr->pinloc ) > 1 ) {
		row_seg_intersect( ptr1 , ptr2 , segptr ) ;
	    }
	} else if( ptr1->row == row ) {
	    if( segptr->switchvalue == swL_down && !segptr->flag ) {
		row_seg_intersect( ptr2 , ptr1 , segptr ) ;
		if( ptr1->pinloc >= NEITHER ) {
		    L_jogS[ ++jog_num ] = wkS ;
		}
	    } else if( (int) ptr1->pinloc == BOTCELL ) {
		if( segptr->switchvalue == swL_down ) {
		    row_seg_intersect( ptr2 , ptr1 , segptr ) ;
		} else {
		    row_seg_intersect( ptr1 , ptr2 , segptr ) ;
		}
	    }
	} else if( ptr2->row == row ) {
	    if( segptr->switchvalue == swL_up && !segptr->flag ) {
		row_seg_intersect( ptr1 , ptr2 , segptr ) ;
		if( ptr2->pinloc <= NEITHER ) {
		    L_jogS[ ++jog_num ] = wkS ;
		}
	    } else if( ptr2->pinloc == TOPCELL ) {
		if( segptr->switchvalue == swL_down ) {
		    row_seg_intersect( ptr2 , ptr1 , segptr ) ;
		} else {
		    row_seg_intersect( ptr1 , ptr2 , segptr ) ;
		}
	    }
	} else if( ptr2->row < row ) {
	    segptr->prev->next = nextptr ;
	    if( nextptr != NULL ) {
		nextptr->prev = segptr->prev ;
	    }
	    Ysafe_free( segptr ) ;
	}
    }
}
if( wkS == 0 ) {
    return ;
}

while( wkS > total_feed_in_the_rowG[row] ) {
    segptr = workerS[ L_jogS[1] ]->segptr ;
    min_i = 1 ;
    min_x = ABS( segptr->pin1ptr->xpos - segptr->pin2ptr->xpos ) ;
    for( i = 2 ; i <= jog_num ; i++ ) {
	segptr = workerS[ L_jogS[i] ]->segptr ;
	x = ABS( segptr->pin1ptr->xpos - segptr->pin2ptr->xpos ) ;
	if( x < min_x ) {
	    min_i = i ;
	    min_x = x ;
	}
    }
    if( min_i == jog_num ) {
	if( L_jogS[jog_num] < wkS ) {
	    copy_workerS_field( workerS[ L_jogS[jog_num] ], workerS[wkS] ) ;
	}
    } else {
	copy_workerS_field( workerS[ L_jogS[min_i] ], 
			   workerS[ L_jogS[jog_num] ] ) ;
	copy_workerS_field( workerS[ L_jogS[jog_num] ], workerS[wkS] ) ;
    }
    wkS-- ;
    jog_num-- ;
}



/* Carl's attempt at improving the feed position assignment 	*/
/* using, god forbid, a nonheuristic, nonhack algorithm for	*/
/* once (linear assignment)					*/


Yquicksort( (char *)(workerS+1), wkS, sizeof(FEED_SEG_PTR), comparenptr);

if( impFeedsG[row]->next == NULL ) {
    return ;
}

/* find out the number of available feeds			*/
v_tracks = 0 ;
for( imptr = impFeedsG[row]->next ; imptr ; imptr = imptr->next ) {
    v_tracks++ ;
}

#ifdef CARL_NEW

assigned_to_track = (int *) Ysafe_calloc( (1+v_tracks) , sizeof(int) ) ;

num_parts = v_tracks / 400 ;
/* if remainder >= 100 or num_parts == 0 ) */
if( v_tracks - 400 * num_parts >= 100 || num_parts == 0 ) {
    num_parts++ ;
}

if( row % 5 == 0 ) {
    fprintf(fpoG,"Divide-and-Conquer Subdivisions in LA:%d at row:%d\n", 
						num_parts, row ) ;
    fflush(fpoG);
}

/* READER: don't expect to understand anything until the end of the  */
/* the "for" loop */


tot_imp_fds = 0 ;
imptr = impFeedsG[row]->next ;
for( ; ; ) {
    tot_imp_fds++ ;
    if( imptr->next != NULL ) {
	imptr = imptr->next ;
    } else {
	break ;
    }
}


bound = wkS / num_parts ;
/* special case of large number of feeds and small number of crossings */
if( bound < num_parts ){
    bound = wkS ;
    num_parts = 1 ;
}
f_count = 0 ;
for( assign_iter = 1 ; assign_iter <= num_parts ; assign_iter++ ) {

    req_remaining_fds = wkS - bound * num_parts ;
    req_remaining_fds += bound * (num_parts - assign_iter + 1) ;

    i_bound = (assign_iter - 1) * bound + 1 ;
    if( assign_iter < num_parts ) {
	f_bound = assign_iter * bound ;
    } else {
	f_bound = wkS ;
    }
    num_feeds = f_bound - i_bound + 1 ;
    /* determines bounds */
    start_feed_xpos = workerS[i_bound]->netptr->xpos ;
    end_feed_xpos = workerS[f_bound]->netptr->xpos ;


    remaining_fds = tot_imp_fds ;
    i_count = 0 ;
    imptr = impFeedsG[row]->next ;
    for( ; ; ) {
	i_count++ ;
	remaining_fds-- ;
	if( imptr->xpos >= start_feed_xpos && i_count > f_count ) {
	    break ;
	}
	if( imptr->next != NULL ) {
	    if( remaining_fds >= req_remaining_fds ) {
		imptr = imptr->next ;
	    } else {
		break ;
	    }
	} else {
	    break ;
	}
    }

    count = 0 ; /* count is the number of unused feeds in the interval */

    if( assign_iter < num_parts ) {
	req_remaining_fds = wkS - bound * num_parts ;
	req_remaining_fds += bound * (num_parts - assign_iter) ;
    } else {
	req_remaining_fds = 0 ;
    }

    i_imptr = imptr ;
    f_count = i_count - 1 ;
    remaining_fds++ ;
    for( ; ; ) {
	f_imptr = imptr ;
	f_count++ ;
	count++ ;
	remaining_fds-- ;
	if( imptr->xpos >= end_feed_xpos && count >= num_feeds ) {
	    break ;
	}
	if( imptr->next != NULL ) {
	    if( remaining_fds - 1 >= req_remaining_fds ) {
		imptr = imptr->next ;
	    } else {
		break ;
	    }
	} else {
	    break ;
	}
    }
    if( count < num_feeds ) {
	fprintf(fpoG,"GR fails at line 501\n");
	fflush(fpoG);
    }


    cost = Yassign_init( count , count ) ;

    for( feed = i_bound ; feed <= f_bound ; feed++ ) {
	feed_xpos = workerS[feed]->netptr->xpos ;
	track = 0 ;
	imptr = i_imptr ;
	for( i = i_count ; i <= f_count ; i++ ) {
	    track++ ;
	    track_xpos = imptr->xpos ;
	    cost[feed-i_bound+1][track] = ABS( feed_xpos - track_xpos ) ;
	    imptr = imptr->next ;
	}
    }
    for( feed = num_feeds + 1 ; feed <= count ; feed++ ) {
	track = 0 ;
	for( i = i_count ; i <= f_count ; i++ ) {
	    track++ ;
	    cost[feed][track] = 1000000 ;
	}
    }

    assign_to_track = Yassign( cost , count , count ) ;

    num_assigned = 0 ;
    for( feed = i_bound ; feed <= f_bound ; feed++ ) {
	assigned_track = assign_to_track[feed-i_bound+1] ;
	track = 0 ;
	imptr = i_imptr ;
	for( i = i_count ; i <= f_count ; i++ ) {
	    if( ++track == assigned_track ) {
		break ;
	    }
	    imptr = imptr->next ;
	}
	if( assigned_to_track[i] != 0 ) {
	    printf("total screw up\n");
	    fflush(stdout);
	} else {
	    assigned_to_track[i] = feed ;
	}

	assgn_impin( imptr , workerS[feed] , row ) ;
	num_assigned++ ;
    }
    /*
    fprintf(fpoG,"row:%d\tnum_feeds:%d\tnum_assigned:%d\t\tcount:%d\n",
				row, num_feeds, num_assigned , count ) ;
    printf("row:%d\tnum_feeds:%d\tnum_assigned:%d\t\tcount:%d\n",
				row, num_feeds, num_assigned , count ) ;
    fflush(fpoG);
    fflush(stdout);
    */
    Yassign_free( cost , count , count ) ;

    /* end of this fly-by-night attempt	*/
}

Ysafe_free( assigned_to_track ) ;



#else
cost = Yassign_init( v_tracks , v_tracks ) ;

for( feed = 1 ; feed <= wkS ; feed++ ) {
    feed_xpos = workerS[feed]->netptr->xpos ;
    track = 0 ;
    for( imptr = impFeeds[row]->next ; imptr ; imptr = imptr->next ) {
	track++ ;
	track_xpos = imptr->xpos ;
	cost[feed][track] = ABS( feed_xpos - track_xpos ) ;
    }
}
for( ; feed <= v_tracks ; feed++ ) {
    track = 0 ;
    for( imptr = impFeeds[row]->next ; imptr ; imptr = imptr->next ) {
	track++ ;
	cost[feed][track] = 1000000 ;
    }
}

assign_to_track = Yassign( cost , v_tracks , v_tracks ) ;


for( feed = 1 ; feed <= wkS ; feed++ ) {
    assigned_track = assign_to_track[feed] ;
    track = 0 ;
    for( imptr = impFeeds[row]->next ; imptr ; imptr = imptr->next ) {
	if( ++track == assigned_track ) {
	    break ;
	}
    }
    assgn_impin( imptr , workerS[feed] , row ) ;
}


Yassign_free( cost , v_tracks , v_tracks ) ;

Ysafe_free( assigned_to_track ) ;

/* end of this fly-by-night attempt				*/
#endif

return ;
}

void row_seg_intersect( PINBOXPTR ptr1 , PINBOXPTR ptr2 , SEGBOXPTR segptr )
{

int i ;

if( ++wkS > max_feed_in_a_rowS ) {
    max_feed_in_a_rowS = max_feed_in_a_rowS + 100 ;

    wk_headS = (int *) Ysafe_realloc( wk_headS , 
	max_feed_in_a_rowS * sizeof(int) ) ;
    L_jogS = (int *) Ysafe_realloc( L_jogS , 
	max_feed_in_a_rowS * sizeof(int) ) ;
    workerS = (FEED_SEG_PTR *)Ysafe_realloc( workerS ,
		( max_feed_in_a_rowS + 1 ) * sizeof(FEED_SEG_PTR) ) ;
    for( i = wkS ; i <= max_feed_in_a_rowS ; i++ ) {
	workerS[i] = ( FEED_SEG_PTR )Ysafe_malloc( sizeof(FEED_SEG) ) ;
    }
}

workerS[ wkS ]->netptr = ptr1 ;
workerS[ wkS ]->refer  = ptr2 ;
workerS[ wkS ]->segptr = segptr ;
}

void copy_workerS_field( FEED_SEG_PTR aptr, FEED_SEG_PTR bptr )
{
aptr->netptr = bptr->netptr ;
aptr->refer  = bptr->refer ;
aptr->segptr = bptr->segptr ;
}

void assgn_impin( IPBOXPTR imptr , FEED_SEG_PTR fsptr , int row )
{

int net ;
PINBOXPTR netptr , ptr1 , ptr2 ;
SEGBOXPTR segptr ;

++implicit_pins_usedG ;
if( fsptr->segptr ) {
    netptr = ( PINBOXPTR )Ysafe_calloc( 1, sizeof( PINBOX ) ) ;
    netptr->adjptr = (ADJASEGPTR)Ysafe_calloc(
			    1, sizeof(ADJASEG) ) ;
    netptr->net  = net = fsptr->segptr->pin1ptr->net ;
    netptr->row  = row ;

    netptr->next = netarrayG[net]->pins ;
    netarrayG[net]->pins = netptr ;

    segptr = fsptr->segptr ;
    netptr->xpos = imptr->xpos ;
    /* FIXME: the following line leaks memory but just freeing it gives freed memory access */
    segptr->pin1ptr = netptr ;
    ptr1 = segptr->pin1ptr ;
    ptr2 = segptr->pin2ptr ;

#define TWF
#ifdef TWF
    if( ptr2->row == ptr1->row ) {
	segptr->switchvalue == nswLINE ;
	segptr->flag = TRUE ;
    } else if( add_Lcorner_feedG &&
	ABS( ptr1->xpos - ptr2->xpos ) >= average_feed_sepG ) {
	segptr->flag = FALSE ;
    } else {
	segptr->flag = TRUE ;
    }
#else
    if( ptr2->row == ptr1->row ) {
	segptr->switchvalue == nswLINE ;
	segptr->flag = TRUE ;
	/*  else if( add_Lcorner_feed &&			 */
	/*  ABS( ptr1->xpos - ptr2->xpos ) >= average_feed_sep ) */
    } else {
	segptr->flag = FALSE ;
	/* else  		 */
	/* segptr->flag = TRUE ; */
    }
#endif

} else { /* a steiner point */
    netptr = fsptr->netptr ;
    netptr->xpos = imptr->xpos ;
}
netptr->ypos = barrayG[row]->bycenter ;
netptr->cell = imptr->cell ;
netptr->pinloc = NEITHER   ;
netptr->terminal = imptr->terminal ;
netptr->pinname  = imptr->pinname  ;
tearrayG[ netptr->terminal ] = netptr ;
netptr->eqptr = ( EQ_NBOXPTR )Ysafe_calloc( 1, sizeof(EQ_NBOX) ) ;
netptr->eqptr->typos = carrayG[ netptr->cell ]->tileptr->bottom ;
netptr->eqptr->pinname = imptr->eqpinname ;
/* now add timing to feed thru */
if( strncmp( carrayG[netptr->cell]->cname, "twfeed", 6 ) == STRINGEQ ){
    carrayG[netptr->cell]->paths = netarrayG[netptr->net]->paths ;
}
}

void unequiv_pin_pre_processing()
{
DBOXPTR dimptr ;
PINBOXPTR ptr ;
int net, row, hi, lo, k, xmin, xmax ;

for( net = 1 ; net <= numnetsG ; net++ ) {
    dimptr = netarrayG[net] ;
    if( dimptr->numpins <= 1 ) continue ;
    ptr = dimptr->pins ;
    lo = hi = ptr->row ;
    for( ptr = ptr->next ; ptr ; ptr = ptr->next ) {
	if( lo > ptr->row ) lo = ptr->row ;
	if( hi < ptr->row ) hi = ptr->row ;
    }
    if( lo == hi ) {
	if( dimptr->numpins == 2 ) continue ;
	ptr = dimptr->pins ;
	xmin = xmax = ptr->xpos ;
	for( ptr = ptr->next ; ptr ; ptr = ptr->next ) {
	    if( ptr->xpos < xmin ) {
		xmin = ptr->xpos ;
	    } else if( ptr->xpos > xmax ) {
		xmax = ptr->xpos ;
	    }
	}
	for( ptr = dimptr->pins ; ptr ; ptr = ptr->next ) {
	    if( ptr->eqptr == NULL || ptr->eqptr->unequiv == 0 ) {
		continue ;
	    }
	    if( ptr->xpos != xmin && ptr->xpos != xmax ) {
		ptr->pinloc = BOTCELL ;
	    }
	}
    }
    for( ptr = dimptr->pins ; ptr ; ptr = ptr->next ) {
	if( ptr->eqptr == NULL || ptr->eqptr->unequiv == 0 ) {
	    continue ;
	}
	row = ptr->row ;
	if( row == hi ) {
	    ptr->pinloc = BOTCELL ;
	} else if( row == lo ) {
	    ptr->pinloc = TOPCELL ;
	} else {
	    k = ( row - lo ) % 2 ;
	    if( k == 1 ) {
		ptr->pinloc = BOTCELL ;
	    } else {
		ptr->pinloc = TOPCELL ;
	    }
	}
    }
}
}

void relax_padPins_pinloc()
{
int i ;
PINBOXPTR pinptr ;

for( i = lastpadG ; i > numcellsG ; i-- ) {
    for( pinptr = carrayG[i]->pins ;pinptr ; pinptr = pinptr->nextpin ) {
	if( pinptr->adjptr->next == NULL ) {
	    pinptr->pinloc = NEITHER ;
	} else if( pinptr->adjptr->next->next == NULL ) {
	    pinptr->pinloc = NEITHER ;
	}
    }
}
}

void relax_unequiv_pinloc()
{
DBOXPTR dimptr ;
PINBOXPTR ptr ;
int net ;

for( net = 1 ; net <= numnetsG ; net++ ) {
    dimptr = netarrayG[net] ;
    if( dimptr->numpins <= 2 ) continue ;
    for( ptr = dimptr->pins ; ptr ; ptr = ptr->next ) {
	if( ptr->pinloc == NEITHER || ptr->eqptr == NULL ||
		    ptr->eqptr->unequiv == 0 ) continue ;
	if( ptr->adjptr->next == NULL ) {
	    ptr->pinloc = NEITHER ;
	} else if( ptr->adjptr->next->next == NULL ) {
	    ptr->pinloc = NEITHER ;
	}
    }
}
}

int check_unequiv_connectivity()
{
int net, channel, correctness ;
ADJASEG *adj ;
PINBOXPTR pinptr ;
DBOXPTR dimptr ;

correctness = 1 ;
for( net = 1 ; net <= numnetsG ; net++ ) {
    dimptr = netarrayG[net] ;
    if( dimptr->numpins <= 1 ) {
	continue ;
    }
    for( pinptr = dimptr->pins ; pinptr ; pinptr = pinptr->next ) {
	if( pinptr->eqptr == NULL || pinptr->eqptr->unequiv == 0 ) {
	    continue ;
	}
	adj = pinptr->adjptr->next ;
	channel = adj->segptr->flag ;
	for( adj = adj->next ; adj ; adj = adj->next ) {
	    if( channel != adj->segptr->flag ) {
		correctness = 0 ;
		printf(" unequivalent pin connection violation" ) ;
		printf(" for net %d pin %d\n", net, pinptr->terminal ) ;
	    }
	}
    }
}
return( correctness ) ;
}


#define	UNVISITED_SEG	0
#define	VISITED_SEG	1
#define	REVISITED_SEG	1

static long int swap_cost( perim_flag )
BOOL perim_flag ;
{

    PINBOXPTR pin1ptr , pin2ptr ;
    SEGBOXPTR segptr ;
    DBOXPTR net_p ;
    int xwire, ywire ;
    int net ;
    long int global_wire_length ;
    int newtimepenal ;
    FILE *fp ;
    static BOOL outputL = TRUE ;
    int p_path, s_path ;
    int Px, Sx, Py, Sy, Ppath, Spath ; 	/* used to cummulate info */

    /* -----------------------------------------------------------------
	We are going to use the half perimeter fields to calculate the
	timing costs.  We find the global wire cost by adding up the
	length from the segments.
    ----------------------------------------------------------------- */
    global_wire_length = 0 ;
    D( "twsc/swap_cost",
	if( outputL == TRUE ){
	    fp = TWOPEN( "wire", "w", ABORT ) ;
	    fprintf( fp, "net\tnumpins\tPx\tSx\tPy\tSy\tPtime\tStime\n" ) ;
	    Px = 0 ;
	    Py = 0 ;
	    Sx = 0 ;
	    Sy = 0 ;
	    Spath = 0 ;
	    Ppath = 0 ;
	}
    ) ;

    for( net = 1; net <= numnetsG; net++ ){
	xwire = 0 ;
	ywire = 0 ;
	for( segptr = netsegHeadG[net]->next;segptr;segptr = segptr->next ){
	    pin1ptr = segptr->pin1ptr ;
	    pin2ptr = segptr->pin2ptr ;
	    segptr->swap_flag = UNVISITED_SEG ;
	    xwire += ABS(pin1ptr->xpos - pin2ptr->xpos) ;
	    ywire += ABS(pin1ptr->ypos - pin2ptr->ypos) ;
	} /* end for( segptr = netsegHeadG[net]... */

	global_wire_length += (long int)xwire ;

	/* now save result in netarray */
	net_p = netarrayG[net] ;
	if( perim_flag ){
	    if( net_p->newhalfPx != xwire ){
		printf( "new x half perim:%d doesn't match calc.:%d\n",
		    net_p->newhalfPx, xwire ) ;
		net_p->newhalfPx = xwire ;
	    }
	    if( net_p->newhalfPy != ywire ){
		printf( "new y half perim:%d doesn't match calc.:%d\n",
		    net_p->newhalfPy, ywire ) ;
		net_p->newhalfPy = ywire ;
	    }
	    
	} else {
	    D( "twsc/swap_cost",
		if( outputL == TRUE ){
		    net_p->newhalfPx = xwire ;
		    net_p->newhalfPy = ywire ;
		    p_path = dpath_len( net, TRUE ) ;
		    s_path = dpath_len( net, FALSE ) ;
		    fprintf( fp, "%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n", net, 
		    net_p->numpins, net_p->halfPx, xwire, net_p->halfPy, ywire,
		    p_path, s_path ) ;
		    Px += net_p->halfPx ;
		    Py += net_p->halfPy ;
		    Sx += xwire ;
		    Sy += ywire ;
		    Spath += s_path ;
		    Ppath += p_path ;
		}
	    ) ;
	    net_p->halfPx = net_p->newhalfPx = xwire ;
	    net_p->halfPy = net_p->newhalfPy = ywire ;
	}
    } /* end for( net = 1... */

    D( "twsc/swap_cost",
	if( outputL == TRUE ){
	    outputL = FALSE ;
	    fprintf( fp, 
	    "----------------------------------------------------------------\n");
	    fprintf( fp, " \t \t%d\t%d\t%d\t%d\t%d\t%d\n", 
		Px,Sx,Py,Sy,Ppath,Spath ) ;
	    TWCLOSE(fp) ;
	}
    ) ;

    /* now recompute the timing penalty */
    newtimepenal = recompute_timecost() ;

    D( "twsc/swap_cost",
	if( timingcostG != newtimepenal ){
	    printf( "Timing penalty mismatch %d vs %d\n",
		timingcostG, newtimepenal ) ;
	}
    ) ;

    timingcostG = newtimepenal ;


    return( global_wire_length ) ;
} /* end swap_cost() */

int improve_place_sequential( int row , int index )
{

int cell1 , cell2 , shift1 , shift2 , swap ;
long int global_wire , new_global_wire ;
int cell , pin , xpos , other_cell ;
long int ic , fc ;
int net, xwire ;
int newtimepenal ;
DBOXPTR net_p ;
CBOXPTR ptr ;
PINBOXPTR pin1ptr , pin2ptr , termptr ;
ADJASEGPTR adjptr ;
SEGBOXPTR segptr ;

/* -----------------------------------------------------------------
    In order to avoid counting segments twice, use the swap_flag field.
    Initially all segments are marked unvisited.  When traversing
    segments of first cell mark them as visited.  When traversing
    second cell we do not update any segments that have been visited.
    Everything is fine at this point but we need to set the flags
    back.  We do this when traversing the cells a second time to
    find the new cost.  We then reverse the roles of the flags.
    It is important to note that VISITED_SEG == REVISITED_SEG.
----------------------------------------------------------------- */
newtimepenal = timingcostG ;
global_wire = 0 ;
clear_net_set() ;

D( "twsc/cell_swap_opt",
    ic = swap_cost( FALSE ) ;
) ;

/* find current global wire lengths */
other_cell = cell = pairArrayG[row][index] ;
ptr = carrayG[cell] ;
for( termptr = ptr->pins; termptr; termptr = termptr->nextpin ) {
    pin = termptr->terminal ;
    net = termptr->net ;
    net_p = netarrayG[net] ;
    /* check to see if this net has been initialized */
    if(!(member_net_set(net) )){
	add2net_set( net ) ;
	net_p->newhalfPx = net_p->halfPx ;
    }
    xwire = 0 ;
    /* look at all the segments that this pin connects */
    for( adjptr = termptr->adjptr->next;adjptr;adjptr = adjptr->next ) {
	segptr = adjptr->segptr ;
	if( segptr->swap_flag == UNVISITED_SEG ){
	    segptr->swap_flag = VISITED_SEG ; /* set to membership */
	    pin1ptr = segptr->pin1ptr ;
	    pin2ptr = segptr->pin2ptr ;
	    xwire += ABS(pin1ptr->xpos - pin2ptr->xpos) ;
	}
    }
    net_p->newhalfPx -= xwire ;
    global_wire += (long int)xwire ;
}
cell = pairArrayG[row][index+1] ;
ptr = carrayG[cell] ;
for( termptr = ptr->pins; termptr; termptr = termptr->nextpin ) {
    pin = termptr->terminal ;
    net = termptr->net ;
    net_p = netarrayG[net] ;
    /* check to see if this net has been initialized */
    if(!(member_net_set(net) )){
	add2net_set( net ) ;
	net_p->newhalfPx = net_p->halfPx ;
    }
    xwire = 0 ;
    for( adjptr = termptr->adjptr->next;adjptr;adjptr = adjptr->next ) {
	segptr = adjptr->segptr ;
	if( segptr->swap_flag == UNVISITED_SEG ){
	    segptr->swap_flag = VISITED_SEG ;
	    pin1ptr = segptr->pin1ptr ;
	    pin2ptr = segptr->pin2ptr ;
	    xwire += ABS(pin1ptr->xpos - pin2ptr->xpos) ;
	}
    }
    net_p->newhalfPx -= xwire ;
    global_wire += (long int)xwire ;
}

/* swap cells */
cell1 = pairArrayG[row][index] ;
cell2 = pairArrayG[row][index+1] ;
shift1 = carrayG[cell2]->clength ;
shift2 = -carrayG[cell1]->clength ;
carrayG[cell1]->cxcenter += shift1 ;
carrayG[cell2]->cxcenter += shift2 ;
pairArrayG[row][index] = cell2 ;
pairArrayG[row][index+1] = cell1 ;
/* ...update pin positions now..... */
for( termptr = carrayG[cell1]->pins; termptr; 
				termptr = termptr->nextpin ) {
    termptr->xpos += shift1 ;
}
for( termptr = carrayG[cell2]->pins; termptr; 
				termptr = termptr->nextpin ) {
    termptr->xpos += shift2 ;
}


/* find new global wire lengths */
new_global_wire = 0 ;
other_cell = cell = pairArrayG[row][index] ;
ptr = carrayG[cell] ;
for( termptr = ptr->pins; termptr; termptr = termptr->nextpin ) {
    pin = termptr->terminal ;
    net_p = netarrayG[termptr->net] ;
    xwire = 0 ;
    for( adjptr = termptr->adjptr->next;adjptr;adjptr = adjptr->next ) {
	segptr = adjptr->segptr ;
	if( segptr->swap_flag == REVISITED_SEG ){
	    segptr->swap_flag = UNVISITED_SEG ;
	    pin1ptr = segptr->pin1ptr ;
	    pin2ptr = segptr->pin2ptr ;
	    xwire += ABS(pin1ptr->xpos - pin2ptr->xpos) ;
	}
    }
    net_p->newhalfPx += xwire ;
    new_global_wire += (long int)xwire ;
}
cell = pairArrayG[row][index+1] ;
ptr = carrayG[cell] ;
for( termptr = ptr->pins; termptr; termptr = termptr->nextpin ) {
    pin = termptr->terminal ;
    net_p = netarrayG[termptr->net] ;
    xwire = 0 ;
    for( adjptr = termptr->adjptr->next;adjptr;adjptr = adjptr->next ) {
	segptr = adjptr->segptr ;
	if( segptr->swap_flag == REVISITED_SEG ){
	    segptr->swap_flag = UNVISITED_SEG ;
	    pin1ptr = segptr->pin1ptr ;
	    pin2ptr = segptr->pin2ptr ;
	    xwire += ABS(pin1ptr->xpos - pin2ptr->xpos) ;
	}
    }
    net_p->newhalfPx += xwire ;
    new_global_wire += (long int)xwire ;
}

/* -----------------------------------------------------------------
    Calculate the timing penalty for swap.
----------------------------------------------------------------- */
newtimepenal += calc_incr_time2( cell1, cell2 ) ;
D( "twsc/check_timing/globe",
    if( dcalc_full_penalty(newtimepenal) == FALSE ){
	printf( "Problem with timing penalty\n" ) ;
    }
) ;

D( "twsc/cell_swap_opt",
    fc = swap_cost( TRUE ) ;

    if( fc - ic != new_global_wire - global_wire ) {
	printf("error in improve_place_sequential()\n");
    }
) ;


// (global_wire - new_global_wire) is assumed to not exceed size int
// although each termin is size long int

if( accept_greedy( (int)(global_wire-new_global_wire), timingcostG-newtimepenal, 0 )){

    swap = (int)(new_global_wire - global_wire) ;
    global_wire_lengthS += (long int)swap ;
    swap += newtimepenal - timingcostG ;
    if( numpathsG ){
	update_time2() ;
	timingcostG = newtimepenal ;
	/* -----------------------------------------------------------------
	    There might be some redundancy here but the checking for it
	    has overhead.  Do simple method for now.
	----------------------------------------------------------------- */
	ptr = carrayG[cell1] ;
	for( termptr = ptr->pins;termptr;termptr=termptr->nextpin ) {
	    net_p = netarrayG[termptr->net] ;
	    net_p->halfPx = net_p->newhalfPx ;
	}
	ptr = carrayG[cell2] ;
	for( termptr = ptr->pins;termptr;termptr=termptr->nextpin ) {
	    net_p = netarrayG[termptr->net] ;
	    net_p->halfPx = net_p->newhalfPx ;
	}
    }
    return( swap ) ;
} else {
    /* swap cells back to original position */
    cell1 = pairArrayG[row][index] ;
    cell2 = pairArrayG[row][index+1] ;
    shift1 = carrayG[cell2]->clength ;
    shift2 = -carrayG[cell1]->clength ;
    carrayG[cell1]->cxcenter += shift1 ;
    carrayG[cell2]->cxcenter += shift2 ;
    pairArrayG[row][index] = cell2 ;
    pairArrayG[row][index+1] = cell1 ;
    /* ...update pin positions now..... */
    for( termptr = carrayG[cell1]->pins; termptr; 
				    termptr = termptr->nextpin ) {
	termptr->xpos += shift1 ;
    }
    for( termptr = carrayG[cell2]->pins; termptr; 
				    termptr = termptr->nextpin ) {
	termptr->xpos += shift2 ;
    }
    return(0);
}
}

int cell_rotate( int row , int index )
{

int cell, swap ;
long int global_wire , new_global_wire ;
int pin , xpos ;
long int ic, fc ;
int net, xwire, xc, left, right, dist_l, dist_r ;
int newtimepenal ;
DBOXPTR net_p ;
CBOXPTR ptr ;
PINBOXPTR pin1ptr , pin2ptr , termptr ;
ADJASEGPTR adjptr ;
SEGBOXPTR segptr ;

/* -----------------------------------------------------------------
    In order to avoid counting segments twice, use the swap_flag field.
----------------------------------------------------------------- */
D( "twsc/cell_swap_opt",
    ic = swap_cost( FALSE ) ;
) ;

global_wire = 0 ;
clear_net_set() ;
/* find current global wire lengths */
cell = pairArrayG[row][index] ;
ptr = carrayG[cell] ;
if( ptr->orflag == 0){
    return(0) ;
}
for( termptr = ptr->pins; termptr; termptr = termptr->nextpin ) {
    pin = termptr->terminal ;
    net = termptr->net ;
    net_p = netarrayG[net] ;
    /* check to see if this net has been initialized */
    if(!(member_net_set(net) )){
	add2net_set( net ) ;
	net_p->newhalfPx = net_p->halfPx ;
    }
    xwire = 0 ;
    /* look at all the segments that this pin connects */
    for( adjptr = termptr->adjptr->next;adjptr;adjptr = adjptr->next ) {
	segptr = adjptr->segptr ;
	if( segptr->swap_flag == UNVISITED_SEG ){
	    segptr->swap_flag = VISITED_SEG ; /* set to membership */
	    pin1ptr = segptr->pin1ptr ;
	    pin2ptr = segptr->pin2ptr ;
	    xwire += ABS(pin1ptr->xpos - pin2ptr->xpos) ;
	}
    }
    net_p->newhalfPx -= xwire ;
    global_wire += (long int)xwire ;
}

/* rotate the cell */
xc = ptr->cxcenter ;
left = xc + ptr->tileptr->left ;
right = xc + ptr->tileptr->right ;

/* ...update pin positions now..... */
for( termptr = carrayG[cell]->pins; termptr; termptr = termptr->nextpin ) {
    dist_l = termptr->xpos - left ;
    dist_r = right - termptr->xpos ;
    if( dist_l < dist_r ){
	/* pin closer to left edge move to right edge */
	termptr->xpos = right - dist_l ;
    } else {
	/* pin closer to right edge move to left edge */
	termptr->xpos = left + dist_r ;
    }
}

/* find new global wire lengths */
new_global_wire = 0 ;
for( termptr = ptr->pins; termptr; termptr = termptr->nextpin ) {
    pin = termptr->terminal ;
    net_p = netarrayG[termptr->net] ;
    xwire = 0 ;
    for( adjptr = termptr->adjptr->next;adjptr;adjptr = adjptr->next ) {
	segptr = adjptr->segptr ;
	if( segptr->swap_flag == REVISITED_SEG ){
	    segptr->swap_flag = UNVISITED_SEG ;
	    pin1ptr = segptr->pin1ptr ;
	    pin2ptr = segptr->pin2ptr ;
	    xwire += ABS(pin1ptr->xpos - pin2ptr->xpos) ;
	}
    }
    net_p->newhalfPx += xwire ;
    new_global_wire += (long int)xwire ;
}

/* -----------------------------------------------------------------
    Calculate the timing penalty for swap.
----------------------------------------------------------------- */
newtimepenal = timingcostG + calc_incr_time( cell ) ;
D( "twsc/check_timing",
    ASSERT( dcalc_full_penalty(newtimepenal), NULL, "time problem\n" ) ;
) ;

D( "twsc/cell_swap_opt",
    fc = swap_cost( TRUE ) ;

    if( fc - ic != new_global_wire - global_wire ) {
	printf("error in cell_rotate()\n");
    }
) ;

if( accept_greedy( (int)(global_wire-new_global_wire), timingcostG-newtimepenal, 0 )){
    swap = (int)(new_global_wire - global_wire );
    global_wire_lengthS += (long int)swap ;
    swap += newtimepenal - timingcostG ;
    if( numpathsG ){
	update_time(cell) ;
	timingcostG = newtimepenal ;
	/* -----------------------------------------------------------------
	    There might be some redundancy here but the checking for it
	    has overhead.  Do simple method for now.
	----------------------------------------------------------------- */
	ptr = carrayG[cell] ;
	for( termptr = ptr->pins;termptr;termptr=termptr->nextpin ) {
	    net_p = netarrayG[termptr->net] ;
	    net_p->halfPx = net_p->newhalfPx ;
	}
    }
    /* make sure to update cell orient field */
    if( ptr->corient == 0 ){
	ptr->corient = 2 ;
    } else if( ptr->corient == 2 ){
	ptr->corient = 0 ;
    } else if( ptr->corient == 1 ){
	ptr->corient = 3;
    } else if( ptr->corient == 3 ){
	ptr->corient = 1;
    } else {
	sprintf( YmsgG, "Invalid rotation for cell:%s\n", ptr->cname ) ;
	M( ERRMSG, "cell_rotate", YmsgG ) ;
    }
    return( swap ) ;
} else {
    /* rotate cells back to original position */
    for( termptr = carrayG[cell]->pins; termptr; termptr = termptr->nextpin ) {
	dist_l = termptr->xpos - left ;
	dist_r = right - termptr->xpos ;
	if( dist_l < dist_r ){
	    /* pin closer to left edge move to right edge */
	    termptr->xpos = right - dist_l ;
	} else {
	    /* pin closer to right edge move to left edge */
	    termptr->xpos = left + dist_r ;
	}
    }
    return(0);
}
} /* end cell_rotate() */

void elim_unused_feedsSC()
{

CBOXPTR ptr , cellptr , first_cptr , last_cptr ;
PINBOXPTR termptr ;
int row, feed_count, i , last , cell_left , length , max_length ;
int j , k , elim , cell , limit , left_edge , corient ;
int *Aray , longest_row , shift , *row_len , total_elim ;

row_len = (int *) Ysafe_calloc( (1+numRowsG) , sizeof(int) ) ;
for( i = 1 ; i <= numRowsG ; i++ ) {
    Aray = pairArrayG[i] ;
    first_cptr = carrayG[ Aray[1] ] ;
    last_cptr  = carrayG[ Aray[ Aray[0] ] ] ;
    length = last_cptr->cxcenter + last_cptr->tileptr->right -
	     first_cptr->cxcenter - first_cptr->tileptr->left ;
    row_len[i] = length ;
}

total_elim = 0 ;
for( ; ; ) {
    /* find longest row */
    max_length = 0 ;
    for( i = 1 ; i <= numRowsG ; i++ ) {
	if( row_len[i] > max_length ) {
	    max_length = row_len[i] ;
	    row = i ;
	}
    }

    i = 0 ;
    limit = pairArrayG[row][0] ;
    for( k = limit ; k >= 1 ; k-- ) {
	cell = pairArrayG[row][k] ;
	if( strncmp(carrayG[cell]->cname,"twfeed",6) == STRINGEQ ) {
	    if( tearrayG[carrayG[cell]->imptr->terminal] == NULL ) {
		if( carrayG[cell]->clength != 0 ) {		    
		    /* we have here an unused feed cell */
		    row_len[row] -= carrayG[cell]->clength ;
		    carrayG[cell]->clength = 0 ;		    
		    carrayG[cell]->tileptr->left = 0 ;
		    carrayG[cell]->tileptr->right = 0 ;
		    i++ ;
		    break ;
		}
	    }
	}
    }

    if( i == 0 ) {
	fprintf(fpoG,"Eliminated %d unused feeds in the longest row\n",
							total_elim ) ;
	break ;
    } else {
	total_elim++ ;
	/* replace, i.e. repack, the cells for this row */
	left_edge  = barrayG[row]->bxcenter + barrayG[row]->bleft ;
	for( i = 1 ; i <= limit ; i++ ) {
	    cellptr = carrayG[ pairArrayG[row][i] ] ;
	    cell_left = cellptr->tileptr->left ;
	    shift = (left_edge - cell_left) - cellptr->cxcenter ;
		/* new minus old */
	    cellptr->cxcenter += shift ;
	    for( termptr = cellptr->pins; termptr; 
					    termptr = termptr->nextpin ){
		termptr->xpos += shift ;
	    }
	    left_edge += cellptr->tileptr->right - cell_left ;
	}
    }
}


/* now print out the new row lengths */
fprintf(fpoG,"BLOCK      TOTAL CELL LENGTHS      OVER/UNDER TARGET\n");
max_length = 0 ;
for( i = 1 ; i <= numRowsG ; i++ ) {
    Aray = pairArrayG[i] ;
    first_cptr = carrayG[ Aray[1] ] ;
    last_cptr  = carrayG[ Aray[ Aray[0] ] ] ;
    length = last_cptr->cxcenter + last_cptr->tileptr->right -
	     first_cptr->cxcenter - first_cptr->tileptr->left ;
    fprintf( fpoG, "%3d            %7d \n", i, length );
    if( max_length < length ) {
	longest_row = i ;
	max_length = length ;
    }
}
fprintf( fpoG, "\nlong intEST Row is:%d   Its length is:%d\n",
			    longest_row , max_length ) ;


Ysafe_free( row_len );
return ;
}

void rebuild_nextpin()
{
    int net, cell ;
    PINBOXPTR netptr , cnetptr ;

    /* added by Carl 22 July 1990 */
    /* blow off the old nextpin fields as they may be corrupted anyway */
    for( net = 1 ; net <= numnetsG ; net++ ) {
	for( netptr = netarrayG[net]->pins ; netptr ; netptr = netptr->next ) {
	    netptr->nextpin = NULL ;
	}
    }

    for( cell = 1 ; cell <= lastpadG ; cell++ ) {
	carrayG[cell]->pins = NULL ;
    }

    /* now rebuild the nextpin fields */
    for( net = 1 ; net <= numnetsG ; net++ ) {
	for( netptr = netarrayG[net]->pins ; netptr ; netptr = netptr->next ) {

	    if( netptr->net <= 0 ){
		printf( "We have an zero net\n" ) ;
		continue ;
	    }
	    /* add it into this cell's pin list */
	    cnetptr = carrayG[netptr->cell]->pins ;
	    carrayG[netptr->cell]->pins = netptr ;
	    carrayG[netptr->cell]->pins->nextpin = cnetptr ;
	}
    }
} /* end rebuild_nextpin */

void rebuild_cell_paths()
{
    int i ;
    CBOXPTR ptr ;
    GLISTPTR path_p, freepath_p ;

    for( i=1;i<=lastpadG; i++ ){
	
	ptr = carrayG[i] ;
	for( path_p = ptr->paths;path_p; ){
	    freepath_p = path_p ;
	    path_p = path_p->next ;
	    Ysafe_free( freepath_p ) ;
	}
	ptr->paths = NULL ;
    }
    add_paths_to_cells() ;
} /* end rebuild_cell_paths() */
