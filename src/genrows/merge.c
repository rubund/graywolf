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

/* ----------------------------------------------------------------- 
FILE:	    merge.c                                       
DESCRIPTION:How to merge tiles together to make row configuration
	    better.
CONTENTS:   
DATE:	    Nov	29, 1990 
REVISIONS:  Fri Jan 25 17:50:54 PST 1991 - added mirror row feature.
	    Sat Sep 21 15:41:10 EDT 1991 - updated for memory.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) merge.c (Yale) version 3.5 5/14/92" ;
#endif

#include <stdio.h>
#include <yalecad/debug.h>
#include <yalecad/message.h>
#include <globals.h>



static check_max_length();
static merge_adjacent_tiles();



merge_tiles()
{
    TILE_BOX *tileptr ; /* current tile */

    merge_adjacent_tiles() ;
    /* now see if we can merge the start tile */
    if( start_tileG ){
	merge_upward( start_tileG ) ;
    }
} /* end merge_tiles */

merge_upward( begin_tile )
TILE_BOX *begin_tile ;
{
    INT left ;          /* left edge of merge tile */
    INT right ;         /* right edge of merge tile */
    TILE_BOX *tileptr ; /* current tile */
    TILE_BOX *temp ;    /* temp pointers to  tile */
    TILE_BOX *tptr ;    /* temp pointers to  tile */
    BOOL merge ;        /* TRUE if we merged at least one tile */

    /* we need to use the tile as a bound for left and right */
    left = begin_tile->llx ;
    right = begin_tile->urx ;
    merge = FALSE ;
    for( tileptr=tile_listG;tileptr;tileptr=tileptr->next ){
	/***********************************************************
	* See if we have any overlap with any tile in the tile set 
	* in the y direction. If so, we can use this as a new top
	* The tile's left edge must be <= left and the tile's right
	* edge must be >= to right.
	***********************************************************/
	if( tileptr == last_tileG || tileptr == begin_tile ){
	    continue ;
	}
	if( tileptr->llx > left ){
	    continue ;
	}
	if( tileptr->urx < right ){
	    continue ;
	}
	/* Does it touch the begin tile in the y direction ? */
	if( projectY( tileptr->lly,tileptr->ury,begin_tile->lly,begin_tile->ury)){
	    /* this tile may be merged with the bottom tile */
	    if( begin_tile->ury < tileptr->ury ){
		begin_tile->ury = tileptr->ury ;
		begin_tile->merged = MERGED_UP ;
		merge = TRUE ;
		/***************************************************
		* Now check the four cases:
		* Case I:

		+--------------------------------+
		+                                |
		+--------------------------------+
			     |                   |
			     |                   |
			     +-------------------+
		****************************************************/
		if( tileptr->llx < left && tileptr->urx == right ){
		    tileptr->urx = left ;
		    check_max_length( tileptr ) ;


		/***************************************************
		* Case II:

		+--------------------------------+
		+                                |
		+--------------------------------+
		|                   |
		|                   |
		+-------------------+
		****************************************************/
		} else if( tileptr->llx == left && tileptr->urx > right ){
		    tileptr->llx = right ;
		    check_max_length( tileptr ) ;

		/***************************************************
		* Case III:

		+--------------------------------+
		+                                |
		+--------------------------------+
		     |                   |
		     |                   |
		     +-------------------+
		****************************************************/
		} else if( tileptr->llx < left && tileptr->urx > right ){
		    /* in this case we need to add a new tile */
		    temp = tileptr->next ;
		    tptr = tileptr->next = (TILE_BOX *) 
			Ysafe_malloc( sizeof(TILE_BOX) ) ;
		    tptr->name = 0 ;
		    tptr->allocated = TRUE ;
		    tptr->llx = tileptr->llx ;
		    tptr->lly = tileptr->lly ;
		    tptr->urx = left ;
		    tptr->ury = tileptr->ury ;
		    tptr->merged = MERGED_NONE ;
		    tptr->actual_row_height = tileptr->actual_row_height ;
		    tptr->channel_separation = tileptr->channel_separation ;
		    tptr->min_length = tileptr->min_length ;
		    tptr->max_length = tptr->urx - tptr->llx - 2 * spacingG ;
		    tptr->row_start = 1 ;
		    tptr->numrows = 0 ;
		    tptr->illegal = FALSE ;
		    tptr->add_no_more_than = 0 ;
		    tptr->next = temp ;
		    tptr->prev = tileptr ;
		    temp->prev = tptr ;
		    tptr->class = tileptr->class ;
		    tptr->mirror = tileptr->mirror ;
		    /* see if we need to reset last_tileG */
		    if(!(tptr->next)){
			last_tileG = tptr ;
		    }
		    /* now reset tileptr values */
		    tileptr->llx = right ;
		    check_max_length( tileptr ) ;

		/***************************************************
		* Case IV:

		    +-------------------+
		    +                   |
		    +-------------------+
		    |                   |
		    |                   |
		    +-------------------+
		****************************************************/
		} else if( tileptr->llx == left && tileptr->urx == right ){
		    /* merge these adjacent tiles; delete tileptr2 */

		    if( tileptr == tile_listG ) {
			tile_listG = tile_listG->next ;
			tile_listG->prev = NULL ;
			/* Ysafe_free( tileptr ) ; */
			tileptr->allocated = FALSE ;
		    } else {
			tileptr->prev->next = tileptr->next ;
			tileptr->next->prev = tileptr->prev ;
			temp = tileptr ;
			tileptr = tileptr->prev ;
			/* Ysafe_free( temp ) ; */
			temp->allocated = FALSE ;
		    }
		} /* end case IV */

		if( limitMergeG ){
		    return ;
		}
	    }
	}
    }
    if( merge ){
	/* we merged tiles call merge right till we can't do no more */
	merge_upward( begin_tile ) ;
    }

} /* end merge_upward */

merge_downward( begin_tile )
TILE_BOX *begin_tile ;
{
    INT left ;          /* left edge of merge tile */
    INT right ;         /* right edge of merge tile */
    TILE_BOX *tileptr ; /* current tile */
    TILE_BOX *temp ;    /* temp pointers to  tile */
    TILE_BOX *tptr ;    /* temp pointers to  tile */
    BOOL merge ;        /* TRUE if we merged at least one tile */

    /* we need to use the tile as a bound for left and right */
    left = begin_tile->llx ;
    right = begin_tile->urx ;
    merge = FALSE ;
    for( tileptr=last_tileG->prev;tileptr;tileptr=tileptr->prev ){
	/***********************************************************
	* See if we have any overlap with any tile in the tile set 
	* in the y direction. If so, we can use this as a new top
	* The tile's left edge must be <= left and the tile's right
	* edge must be >= to right.
	***********************************************************/
	if( tileptr == last_tileG || tileptr == begin_tile ){
	    continue ;
	}
	if( tileptr->llx > left ){
	    continue ;
	}
	if( tileptr->urx < right ){
	    continue ;
	}
	/* Does it touch the begin tile in the y direction ? */
	if( projectY( tileptr->lly,tileptr->ury,begin_tile->lly,begin_tile->ury)){
	    /* this tile may be merged with the bottom tile */
	    if( begin_tile->lly > tileptr->lly ){
		begin_tile->lly = tileptr->lly ;
		begin_tile->merged = MERGED_DOWN ;
		merge = TRUE ;
		/***************************************************
		* Now check the four cases:
		* Case I:

			     +-------------------+
			     |                   |
			     |                   |
		+--------------------------------+
		+                                |
		+--------------------------------+
		****************************************************/
		if( tileptr->llx < left && tileptr->urx == right ){
		    tileptr->urx = left ;
		    check_max_length( tileptr ) ;


		/***************************************************
		* Case II:

		+-------------------+
		|                   |
		|                   |
		+--------------------------------+
		+                                |
		+--------------------------------+
		****************************************************/
		} else if( tileptr->llx == left && tileptr->urx > right ){
		    tileptr->llx = right ;
		    check_max_length( tileptr ) ;

		/***************************************************
		* Case III:

		     +-------------------+
		     |                   |
		     |                   |
		+--------------------------------+
		+                                |
		+--------------------------------+
		****************************************************/
		} else if( tileptr->llx < left && tileptr->urx > right ){
		    /* in this case we need to add a new tile */
		    temp = tileptr->next ;
		    tptr = tileptr->next = (TILE_BOX *) 
			Ysafe_malloc( sizeof(TILE_BOX) ) ;
		    tptr->name = 0 ;
		    tptr->allocated = TRUE ;
		    tptr->llx = tileptr->llx ;
		    tptr->lly = tileptr->lly ;
		    tptr->urx = left ;
		    tptr->ury = tileptr->ury ;
		    tptr->merged = MERGED_NONE ;
		    tptr->actual_row_height = tileptr->actual_row_height ;
		    tptr->channel_separation = tileptr->channel_separation ;
		    tptr->min_length = tileptr->min_length ;
		    tptr->max_length = tptr->urx - tptr->llx - 2 * spacingG ;
		    tptr->row_start = 1 ;
		    tptr->numrows = 0 ;
		    tptr->illegal = FALSE ;
		    tptr->add_no_more_than = 0 ;
		    tptr->class = tileptr->class ;
		    tptr->mirror = tileptr->mirror ;
		    tptr->next = temp ;
		    tptr->prev = tileptr ;
		    temp->prev = tptr ;
		    /* see if we need to reset last_tileG */
		    if(!(tptr->next)){
			last_tileG = tptr ;
		    }
		    /* now reset tileptr values */
		    tileptr->llx = right ;
		    check_max_length( tileptr ) ;

		/***************************************************
		* Case IV:

		    +-------------------+
		    |                   |
		    |                   |
		    +-------------------+
		    |                   |
		    +-------------------+
		****************************************************/
		} else if( tileptr->llx == left && tileptr->urx == right ){
		    /* merge these adjacent tiles; delete tileptr2 */

		    if( tileptr == tile_listG ) {
			tile_listG = tile_listG->next ;
			tile_listG->prev = NULL ;
			/* Ysafe_free( tileptr ) ; */
			tileptr->allocated = FALSE ;
		    } else {
			tileptr->prev->next = tileptr->next ;
			tileptr->next->prev = tileptr->prev ;
			temp = tileptr ;
			tileptr = tileptr->next ;
			/* Ysafe_free( temp ) ; */
			temp->allocated = FALSE ;
		    }
		} /* end case IV */

		if( limitMergeG ){
		    return ;
		}
	    }

	}

    }
    if( merge ){
	/* we merged tiles call merge right till we can't do no more */
	merge_downward( begin_tile ) ;
    }

} /* end merge_downward */

merge_right( begin_tile )
TILE_BOX *begin_tile ;
{
    INT bottom ;        /* bottom edge of merge tile */
    INT top ;           /* top edge of merge tile */
    TILE_BOX *tileptr ; /* current tile */
    TILE_BOX *temp ;    /* temp pointers to  tile */
    TILE_BOX *tptr ;    /* temp pointers to  tile */
    BOOL merge ;        /* TRUE if we merged at least one tile */

    /* we need to use the tile as a bound for bottom and top */
    bottom = begin_tile->lly ;
    top = begin_tile->ury ;
    merge = FALSE ;
    for( tileptr=tile_listG;tileptr;tileptr=tileptr->next ){
	/***********************************************************
	* See if we have any overlap with any tile in the tile set 
	* in the x direction. If so, we can use this as a new right
	* The tile's bottom edge must be <= bottom and the tile's top
	* edge must be >= to top.
	***********************************************************/
	if( tileptr == last_tileG || tileptr == begin_tile ){
	    continue ;
	}
	if( tileptr->lly > bottom ){
	    continue ;
	}
	if( tileptr->ury < top ){
	    continue ;
	}
	/* Does it touch the begin tile in the y direction ? */
	if( projectX( tileptr->llx,tileptr->urx,begin_tile->llx,begin_tile->urx)){
	    /* this tile may be merged with the right tile */
	    if( begin_tile->urx < tileptr->urx ){
		begin_tile->urx = tileptr->urx ;
		begin_tile->merged = MERGED_RITE ;
		merge = TRUE ;
		check_max_length( begin_tile ) ;
		/***************************************************
		* Now check the four cases:
		* Case I:

		+-------------------+-------------------+
		+    begin          |    tileptr        |
		+-------------------+                   |
			            |                   |
			            +-------------------+
		****************************************************/
		if( tileptr->lly < bottom && tileptr->ury == top ){
		    tileptr->ury = bottom ;
		    check_max_length( tileptr ) ;


		/***************************************************
		* Case II:

			            +-------------------+
			            |                   |
		+-------------------+                   |
		+    begin          |    tileptr        |
		+-------------------+-------------------+
		****************************************************/
		} else if( tileptr->lly == bottom && tileptr->ury > top ){
		    tileptr->lly = top ;
		    check_max_length( tileptr ) ;

		/***************************************************
		* Case III:

			            +-------------------+
		+-------------------+                   |
		+    begin          |    tileptr        |
		+-------------------+                   |
				    +-------------------+ new tile here
		****************************************************/
		} else if( tileptr->lly < bottom && tileptr->ury > top ){
		    /* in this case we need to add a new tile */
		    temp = tileptr->next ;
		    tptr = tileptr->next = (TILE_BOX *) 
			Ysafe_malloc( sizeof(TILE_BOX) ) ;
		    tptr->name = 0 ;
		    tptr->allocated = TRUE ;
		    tptr->llx = tileptr->llx ;
		    tptr->lly = tileptr->lly ;
		    tptr->urx = tileptr->urx ;
		    tptr->ury = bottom ;
		    tptr->merged = MERGED_NONE ;
		    tptr->actual_row_height = tileptr->actual_row_height ;
		    tptr->channel_separation = tileptr->channel_separation ;
		    tptr->min_length = tileptr->min_length ;
		    tptr->max_length = tptr->urx - tptr->llx - 2 * spacingG ;
		    tptr->row_start = 1 ;
		    tptr->numrows = 0 ;
		    tptr->illegal = FALSE ;
		    tptr->add_no_more_than = 0 ;
		    tptr->next = temp ;
		    tptr->prev = tileptr ;
		    temp->prev = tptr ;
		    tptr->class = tileptr->class ;
		    tptr->mirror = tileptr->mirror ;
		    /* see if we need to reset last_tileG */
		    if(!(tptr->next)){
			last_tileG = tptr ;
		    }
		    /* now reset tileptr values */
		    tileptr->lly = top ;
		    check_max_length( tileptr ) ;

		/***************************************************
		* Case IV:
		+-------------------+-------------------+
		+    begin          |    tileptr        |
		+-------------------+-------------------+
		****************************************************/
		} else if( tileptr->lly == bottom && tileptr->ury==top ){
		    /* merge these adjacent tiles; delete tileptr2 */

		    if( tileptr == tile_listG ) {
			tile_listG = tile_listG->next ;
			tile_listG->prev = NULL ;
			/* Ysafe_free( tileptr ) ;*/
			tileptr->allocated = FALSE ;
			tileptr = tile_listG ;
		    } else {
			tileptr->prev->next = tileptr->next ;
			tileptr->next->prev = tileptr->prev ;
			temp = tileptr ;
			tileptr = tileptr->prev ;
			/* Ysafe_free( temp ) ; */
			temp->allocated = FALSE ;
		    }
		} /* end case IV */

		if( limitMergeG ){
		    return ;
		}
	    }

	}
    }
    if( merge ){
	/* we merged tiles call merge right till we can't do no more */
	merge_right( begin_tile ) ;
    }

} /* end merge_right */

merge_left( begin_tile )
TILE_BOX *begin_tile ;
{
    INT bottom ;        /* bottom edge of merge tile */
    INT top ;           /* top edge of merge tile */
    TILE_BOX *tileptr ; /* current tile */
    TILE_BOX *temp ;    /* temp pointers to  tile */
    TILE_BOX *tptr ;    /* temp pointers to  tile */
    BOOL merge ;        /* TRUE if we merged at least one tile */

    /* we need to use the tile as a bound for bottom and top */
    bottom = begin_tile->lly ;
    top = begin_tile->ury ;
    merge = FALSE ;
    for( tileptr=tile_listG;tileptr;tileptr=tileptr->next ){
	/***********************************************************
	* See if we have any overlap with any tile in the tile set 
	* in the x direction. If so, we can use this as a new right
	* The tile's bottom edge must be <= bottom and the tile's top
	* edge must be >= to top.
	***********************************************************/
	if( tileptr == last_tileG || tileptr == begin_tile ){
	    continue ;
	}
	if( tileptr->lly > bottom ){
	    continue ;
	}
	if( tileptr->ury < top ){
	    continue ;
	}
	/* Does it touch the begin tile in the y direction ? */
	if( projectX( tileptr->llx,tileptr->urx,begin_tile->llx,begin_tile->urx)){
	    /* this tile may be merged with the right tile */
	    if( begin_tile->llx > tileptr->llx ){
		begin_tile->llx = tileptr->llx ;
		begin_tile->merged = MERGED_LEFT ;
		merge = TRUE ;
		check_max_length( begin_tile ) ;
		/***************************************************
		* Now check the four cases:
		* Case I:
		+-------------------+-------------------+ 
		|    tileptr        |    begin          |
	        |                   +-------------------+
		+-------------------+
		****************************************************/
		if( tileptr->lly < bottom && tileptr->ury == top ){
		    tileptr->ury = bottom ;
		    check_max_length( tileptr ) ;


		/***************************************************
		* Case II:
		+-------------------+ 
		|                   +-------------------+
		|    tileptr        |    begin          |
		+-------------------+-------------------+

		****************************************************/
		} else if( tileptr->lly == bottom && tileptr->ury > top ){
		    tileptr->lly = top ;
		    check_max_length( tileptr ) ;

		/***************************************************
		* Case III:

		+-------------------+
		|                   +-------------------+ 
		|    tileptr        |    begin          |
	        |                   +-------------------+
		+-------------------+ new tile here
		****************************************************/
		} else if( tileptr->lly < bottom && tileptr->ury > top ){
		    /* in this case we need to add a new tile */
		    temp = tileptr->next ;
		    tptr = tileptr->next = (TILE_BOX *) 
			Ysafe_malloc( sizeof(TILE_BOX) ) ;
		    tptr->name = 0 ;
		    tptr->allocated = TRUE ;
		    tptr->llx = tileptr->llx ;
		    tptr->lly = tileptr->lly ;
		    tptr->urx = tileptr->urx ;
		    tptr->ury = bottom ;
		    tptr->merged = MERGED_NONE ;
		    tptr->actual_row_height = tileptr->actual_row_height ;
		    tptr->channel_separation = tileptr->channel_separation ;
		    tptr->min_length = tileptr->min_length ;
		    tptr->max_length = tptr->urx - tptr->llx - 2 * spacingG ;
		    tptr->row_start = 1 ;
		    tptr->numrows = 0 ;
		    tptr->illegal = FALSE ;
		    tptr->add_no_more_than = 0 ;
		    tptr->next = temp ;
		    tptr->prev = tileptr ;
		    temp->prev = tptr ;
		    tptr->class = tileptr->class ;
		    tptr->mirror = tileptr->mirror ;
		    /* see if we need to reset last_tileG */
		    if(!(tptr->next)){
			last_tileG = tptr ;
		    }
		    /* now reset tileptr values */
		    tileptr->lly = top ;
		    check_max_length( tileptr ) ;

		/***************************************************
		* Case IV:
		+-------------------+-------------------+
		+    tileptr        |    begin          |
		+-------------------+-------------------+
		****************************************************/
		} else if( tileptr->lly == bottom && tileptr->ury==top ){
		    /* merge these adjacent tiles; delete tileptr2 */

		    if( tileptr == tile_listG ) {
			tile_listG = tile_listG->next ;
			tile_listG->prev = NULL ;
			/* Ysafe_free( tileptr ) ; */
			tileptr->allocated = FALSE ;
			tileptr = tile_listG ;
		    } else {
			tileptr->prev->next = tileptr->next ;
			tileptr->next->prev = tileptr->prev ;
			temp = tileptr ;
			tileptr = tileptr->prev ;
			/* Ysafe_free( temp ) ; */
			temp->allocated = FALSE ;
		    }
		} /* end case IV */

		if( limitMergeG ){
		    return ;
		}
	    }

	}
    }
    if( merge ){
	/* we merged tiles call merge left till we can't do no more */
	merge_left( begin_tile ) ;
    }

} /* end merge_left */

static check_max_length( tileptr )
TILE_BOX *tileptr ;
{
    INT length ;              /* length of tile */

    length = tileptr->urx - tileptr->llx - 2 * spacingG ;
    tileptr->max_length = length ;

}/* end check_max_length */

renumber_tiles()
{
    INT count ;              /* count the tiles */
    TILE_BOX *tileptr ;      /* current tile */
    count = 0 ;
    for( tileptr=tile_listG;tileptr;tileptr=tileptr->next ){
	tileptr->name = ++count ;
    }
} /* end renumber_tiles() */

static merge_adjacent_tiles()
{

    TILE_BOX *tileptr1 , *tileptr2 , *tileptr ;

REDO: 
    for( tileptr1 = tile_listG;tileptr1->next;tileptr1 = tileptr1->next ){
	// for( tileptr2=tile_listG;tileptr2->next;tileptr2=tileptr2->next ){
	for( tileptr2=tile_listG;tileptr2;tileptr2=tileptr2->next ){
	    if( tileptr2 == tileptr1 ) {
		continue ;
	    }
	    if( tileptr1->llx == tileptr2->llx &&
			    tileptr1->urx == tileptr2->urx &&
			    tileptr1->ury == tileptr2->lly ) {
		/* merge these adjacent tiles; delete tileptr2 */
		tileptr1->ury = tileptr2->ury ;

		if( tileptr2 == tile_listG ) {
		    tile_listG = tile_listG->next ;
		    tile_listG->prev = NULL ;
		    /* Ysafe_free( tileptr2 ) ; */
		    tileptr2->allocated = FALSE ;
		    goto REDO ;
		} else {
		    tileptr = tile_listG ;
		    while( tileptr->next != tileptr2 ) {
			tileptr = tileptr->next ;
		    }
		    tileptr->next = tileptr2->next ;
		    if( tileptr2->next ){
		        tileptr2->next->prev = tileptr ;
		    }
		    /* Ysafe_free( tileptr2 ) ; */
		    tileptr2->allocated = FALSE ;
		    goto REDO ;
		}
	    }

	} /* end inner loop */
    } /* end outer loop */
    return ;
}/* end merge_adjacent_tiles */

dtiles()
{
    TILE_BOX *tptr ;      /* current tile */

    fprintf( stderr, "The forward tiles\n" ) ;
    for( tptr=tile_listG;tptr;tptr=tptr->next ){
	fprintf( stderr, "\tTile:%d l:%5d b:%5d r:%5d t:%5d\n", 
	    tptr->name, tptr->llx, tptr->lly, tptr->urx, tptr->ury ) ;
    }
    fprintf( stderr, "The backward tiles\n" ) ;
    for( tptr=last_tileG;tptr;tptr=tptr->prev ){
	fprintf( stderr, "\tTile:%d l:%5d b:%5d r:%5d t:%5d\n", 
	    tptr->name, tptr->llx, tptr->lly, tptr->urx, tptr->ury ) ;
    }
    
}
