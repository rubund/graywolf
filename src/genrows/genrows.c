/*
 *   Copyright (C) 1989-1992 Yale University
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
FILE:	    genrows.c                                       
DESCRIPTION:This file contains the main algorithm for generating the
	    standard cell rows.
CONTENTS:   
DATE:	    Aug  9, 1989 
REVISIONS:  Sep 18, 1989 - added row gridding to algorithm.
	    Dec 18, 1989 - handle special case of total row length
		less than minimum tile size.
	    Feb  8, 1990 - took total_row_length out of call by
		making it a static.
	    May  4, 1990 - added error checking for genrows.
	    Sep 25, 1990 - now create a .gen file when user
		changes macro positions. Fixed a pointer problem
		in vertex list.
	    Oct 21, 1990 - added snap core feature.
	    Dec  7, 1990 - added set parameter options and added
		force function.  Also fixed rounding of rows.
	    Dec 15, 1990 - fixed problem with stdcell only UNDO
		and install divide tile.
	    Dec 28, 1990 - added divide_tilelr.
	    Mon Jan  7 18:24:05 CST 1991 - removed two versions
		of core.  Check for compatible restart file.
		Turned off info about placed row length.
	    Sun Jan 20 21:22:37 PST 1991 - ported to AIX.
	    Fri Jan 25 17:50:54 PST 1991 - added mirror row feature.
	    Sat Jan 26 16:29:59 PST 1991 - added check_user_data
		and force_tiles. Also updated shortRow at top of
		core.
	    Sat Feb 23 00:33:08 EST 1991 - now make discard short
		row the default.
	    Thu Mar  7 03:10:48 EST 1991 - now allow user to set
		number of rows in the standard cell case.
	    Thu Mar  7 03:36:00 EST 1991 - added comments to the .blk
		file.
	    Wed Apr 17 22:35:57 EDT 1991 - now genrows can rotate
		macros.  Deleted snap_core.
	    Sun Apr 21 21:49:14 EDT 1991 - fixed problem with dividing
		tiles vertically.
	    Sat Sep 21 15:38:45 EDT 1991 - added memory function.
	    Tue Sep 24 00:47:00 EDT 1991 - output core into
		.gen file.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) genrows.c (Yale) version 3.24 5/14/92" ;
#endif

#include <stdio.h>
#include <yalecad/base.h>
#include <yalecad/file.h>
#include <yalecad/debug.h>
#include <yalecad/message.h>
#include <yalecad/buster.h>
#include <yalecad/rbtree.h>
#include <globals.h>

#define NOTOUCH    0
#define OVERLAP1   1
#define OVERLAP2   2
#define OVERLAP3   3
#define TOUCH      4

static ROW_BOX *set_of_rowsS , *last_rowptrS ;
static VERTEX_BOX *vertex_listS , *last_vertexS ;

static INT number_of_rows_addedS ;
static INT number_of_segs_addedS ;

/* core boundaries, as read in from the data */
static INT cx1S , cy1S , cx2S , cy2S ;/* current values */

static INT actual_row_heightS ;  /* row height in microns */
static INT channel_separationS ; /* channel separation in microns */
static INT min_lengthS ;         /* minimum valid row length */
static INT total_row_lengthS ;   /* total row length of standard cells and feeds */
static INT total_row_segmentS ;  /* number of row segments */
static INT stdcell_lengthS ;    /* total row length of standard cells */
static INT feed_lengthS = 0 ;       /* length of feeds */

static YTREEPTR tile_memoryG ;
static reset_tile_parameters();
static INT compare_tiles();

#if SIZEOF_VOID_P == 64
#define INTSCANSTR "%ld"
#else
#define INTSCANSTR "%d"
#endif

init_data_structures() 
{
    /*static INT compare_tiles() ;*/

    set_of_rowsS = NULL ;
    last_rowptrS = NULL ;
    tile_listG = NULL ;
    shortRowG = FALSE ;
    macroArrayG = NULL ;
    start_tileG = NULL ; /* current chosen tile */
    vertex_listS = NULL ;
    limitMergeG = FALSE;
    spacingG = 1 ;
    memoryG = TRUE ;
    tile_memoryG = Yrbtree_init( compare_tiles ) ;
} /* end init_data_structures */

set_feed_length( percent )
DOUBLE percent ;
{
    feed_lengthS = (INT) ( percent / 100.0 * (DOUBLE) stdcell_lengthS ) ;
} /* end set_feed_length */

process_tiles()
{
    TILE_BOX *tptr ;
    INT llx , lly , urx , ury ;
    INT counter = 0 ;

    last_tileG = NULL ;
    tile_listG = NULL ;
    while( find_tile(&llx , &lly , &urx , &ury ) ) {
	if(!(tile_listG)) {
	    tile_listG = (TILE_BOX *) Ysafe_malloc( sizeof(TILE_BOX) ) ;
	    tptr = last_tileG = tile_listG ;
	    tile_listG->prev = NULL ;
	} else {
	    tptr = last_tileG->next = 
		    (TILE_BOX *) Ysafe_malloc( sizeof(TILE_BOX) ) ;
	    tptr->prev = last_tileG ;
	    last_tileG = tptr ;
	}
	tptr->name = ++counter ;
	tptr->llx = llx ;
	tptr->lly = lly ;
	tptr->urx = urx ;
	tptr->ury = ury ;
	tptr->merged = MERGED_NONE ;
	tptr->actual_row_height = actual_row_heightS ;
	tptr->channel_separation = channel_separationS ;
	tptr->min_length = min_lengthS ;
	tptr->numrows = 0 ;
	tptr->illegal = FALSE ;
	tptr->class = 1 ;
	tptr->mirror = FALSE ;
	tptr->force = FALSE ;
	tptr->add_no_more_than = 0 ;
	tptr->allocated = TRUE ;
	tptr->next = NULL ;
    }

    /*  ADD EXTRA TILE ACROSS TOP OF THE CORE OF THE CHIP  	*/
    /*	(in case there isn't enough row space within the core)	*/
    tptr = last_tileG->next = 
	    (TILE_BOX *) Ysafe_malloc( sizeof(TILE_BOX) ) ;
    tptr->prev = last_tileG ;
    last_tileG = tptr ;
    tptr->llx = cx1S ;
    tptr->lly = cy2S ;
    tptr->urx = cx2S ;
    tptr->ury = cy2S + (cy2S - cy1S) / 2 ;  
    /*  Just some upper bound; I would expect that we would never
	have to extend the core upward by more than 1/2 */
    tptr->merged = MERGED_NONE ;
    tptr->actual_row_height = actual_row_heightS ;
    tptr->channel_separation = channel_separationS ;
    tptr->min_length = min_lengthS ;
    tptr->numrows = 0 ;
    tptr->illegal = FALSE ;
    tptr->force = FALSE ;
    tptr->class = 1 ;
    tptr->mirror = FALSE ;
    tptr->add_no_more_than = 0 ;
    tptr->allocated = TRUE ;
    tptr->next = NULL ;
    /* now set row start and max length fields */
    set_spacing() ;

} /* end process_tiles */

check_tiles()
{

    TILE_BOX *tile1 , *tile2 ;

    /*  CHECK FOR ILLEGAL TILE CONFIGURATIONS  */
    for( tile1 = tile_listG ; tile1 ; tile1 = tile1->next ) {
	for( tile2 = tile1->next ; tile2 ; tile2 = tile2->next ) {
	    if( !(tile1->lly >= tile2->ury || tile2->lly >= tile1->ury)){
		if( tile1->actual_row_height!=tile2->actual_row_height ||
		    tile1->channel_separation != tile2->channel_separation){
		    printf("\nERROR:\n") ;
		    printf("Tiles which overlap in the y-direction must have\n");
		    printf("the same row height and the same channel separation\n\n");
		}
	    }
	}
    }
} /* end check_tiles */



print_blk_file() 
{

    /*  output the rows; check for multiple row segments - if there
	are any, create the appropriate exceptions  */

    FILE *fpb ;     /* block file */
    FILE *fpg ;     /* gen file */
    ROW_BOX *rowptr , *segment , *save_segment ;
    MACROPTR mptr ;  /* the current macro */
    INT llx , urx , last_llx , last_urx , next_llx ;
    INT r ;
    INT i ;  /* counter */
    INT pt ;  /* counter */
    char filename[LRECL] ;
    VERTEXPTR *vptr ;       /* current vertex array */
    VERTEXPTR pt_ptr ;      /* current point */
    TILE_BOX *tileptr ;     /* current tile */

    if( no_outputG ){
	return ;
    }

    sprintf( filename, "%s.blk", cktNameG ) ;
    fpb = TWOPEN( filename, "w", ABORT ) ;
    fprintf(fpb,"rows %d\n", number_of_rows_addedS ) ;

    grid_rows() ;

    i = 1;
    for( rowptr = set_of_rowsS ; rowptr ; rowptr = rowptr->next_row ){

	/* Flip alternate rows, if requested */
	if (i == flip_alternateG) rowptr->mirror = !(rowptr->mirror);
	if (++i == 3) i = 1;

	fprintf(fpb,"row ") ;
	segment = rowptr->next_segment ;
	if( segment == NULL ) {
	    if( feeds_per_segG ){
		r = rowptr->urx - feeds_per_segG ;
		if( r > rowptr->llx ){
		    /* only output up to the feed to make space */
		    fprintf(fpb,"%d %d    %d %d", rowptr->llx, rowptr->lly,
						    r, rowptr->ury);
		    /* go on to the next row */
		    if( rowptr->class != 1 ){
			fprintf(fpb,"\n class %d", rowptr->class ) ;
		    }
		    if( rowptr->mirror ){
			fprintf(fpb,"\n mirror" ) ;
		    }
		    fprintf(fpb,"\n" ) ;
		    continue ;
		}
	    } 
	    /* no feeds to worry about */
	    fprintf(fpb,"%d %d    %d %d", rowptr->llx, rowptr->lly,
					    rowptr->urx, rowptr->ury);
	    if( rowptr->class != 1 ){
		fprintf(fpb," class %d", rowptr->class ) ;
	    }
	    if( rowptr->mirror ){
		fprintf(fpb," mirror" ) ;
	    }
	    fprintf(fpb,"\n" ) ;
	} else {
	    /* find x-extent of row segments */
	    llx = rowptr->llx ;
	    urx = rowptr->urx ;
	    last_urx = urx ; /* used below */
	    for( segment=rowptr->next_segment;segment;segment=segment->next_segment){
		if( segment->llx < llx ) {
		    llx = segment->llx ;
		    last_urx = segment->urx ; /* used below */
		    last_llx = segment->llx ; /* used below */
		}
		if( segment->urx > urx ) {
		    urx = segment->urx ;
		}
	    } /* end for loop */

	    r = rowptr->urx - feeds_per_segG ;
	    if( feeds_per_segG && r > last_llx ){
		    fprintf(fpb,"%d %d    %d %d ", llx, rowptr->lly,
						    r, rowptr->ury ) ;
	    } else {
		fprintf(fpb,"%d %d    %d %d ", llx, rowptr->lly,
						urx, rowptr->ury ) ;
	    }
	    /* now print out the exceptions */

	    last_llx = llx ;
	    next_llx = INT_MAX ;
	    save_segment = NULL ;
	    while( TRUE ) {
		if( rowptr->llx > last_llx ) {
		    next_llx = rowptr->llx ;
		    save_segment = rowptr ;
		}
		segment = rowptr->next_segment ;
		while( segment ) {
		    if( segment->llx > last_llx && 
					    segment->llx < next_llx ) {
			next_llx = segment->llx ;
			save_segment = segment ;
		    }
		    segment = segment->next_segment ;
		}
		if( save_segment ) {
		    r = next_llx - feeds_per_segG ;
		    if( feeds_per_segG && r > last_urx ){
			fprintf(fpb," except %d %d", last_urx ,
						     r ) ;
		    } else if( next_llx > last_urx ){
			fprintf(fpb," except %d %d", last_urx ,
						     next_llx ) ;
		    }
		    last_llx = next_llx ;
		    next_llx = INT_MAX ;
		    last_urx = save_segment->urx ;
		    save_segment = NULL ;
		} else {
		    break ;
		}
	    }
	    if( rowptr->class != 1 ){
		fprintf(fpb," class %d\n", rowptr->class ) ;
	    }
	    if( rowptr->mirror ){
		fprintf(fpb," mirror" ) ;
	    }
	    fprintf(fpb,"\n" ) ;
	}
    } /* end for loop */

    /* now output the tiles in C comment format */
    fprintf(fpb,"\n/* \n" ) ;
    fprintf(fpb,"   The Tile and Macro Information:\n" ) ;
    /* now count the number of tiles */
    i = 0 ;
    for( tileptr=tile_listG;tileptr;tileptr=tileptr->next ){
	if( tileptr->numrows > 0 ){
	    i++ ;
	}
    }
    fprintf(fpb,"   numtiles %d\n", i ) ;
    /* now give the positions of the tiles */
    for( tileptr=tile_listG;tileptr;tileptr=tileptr->next ){
	if( tileptr->numrows > 0 ){
	    fprintf(fpb,"   %d %d   %d %d\n", tileptr->llx, tileptr->lly,
		tileptr->urx, tileptr->ury ) ;
	}
    }

    fprintf(fpb,"\n" ) ;
    fprintf(fpb,"   nummacros %d", num_macrosG ) ;

    /* if macros exist write them in the .gen file */
    sprintf( filename, "%s.gen", cktNameG ) ;
    fpg = TWOPEN( filename, "w", ABORT ) ;
    fprintf( fpg, "core %d %d %d %d\n", cx1S, cy1S, cx2S, cy2S ) ;
    for( i = 1; i <= num_macrosG; i++ ){
	mptr = macroArrayG[i] ;
	fprintf( fpg, "%d %d %d\n", mptr->xcenter, mptr->ycenter, mptr->orient ) ;

	/* also write macro positions to block file */
	fprintf(fpb,"\n   macro orient %d ", mptr->orient ) ;
	vptr = mptr->vertices ;
	for( pt = 0 ; pt < mptr->num_vertices ; pt++ ) {
	    pt_ptr = vptr[pt] ;
	    fprintf(fpb,"%d %d  ", pt_ptr->x, pt_ptr->y ) ;
	}

    }

    fprintf(fpb,"\n\n*/\n" ) ; /* close comment */
    TWCLOSE( fpg ) ;
    TWCLOSE( fpb ) ;

    return ;
} /* end print_blk_file */

print_tiles() 
{

    INT i ;
    TILE_BOX *tileptr ;

    i = 0 ;
    for( tileptr=tile_listG;tileptr;tileptr=tileptr->next ){
	i++ ;
	printf("TILE:%4d\t(%4d %4d)   (%4d %4d)\n", i ,
					tileptr->llx , tileptr->lly , 
					tileptr->urx , tileptr->ury ) ;
    }
    printf("\n");
    return ;
} /* end print_tiles */

print_vertices()
{
    /* for debug only */

    VERTEX_BOX *vertex ;

    printf("VERTICES, where START=1, END=2, NOTHING=3, ");
    printf("MAYBE_START=12, MAYBE_END=13\n");

    for( vertex = vertex_listS ;vertex; vertex = vertex->next ){
	printf("x:%d    y:%d     class:%d     macro:%d\n",
		vertex->x , vertex->y , vertex->class , vertex->macro ) ;
    } /* end for loop */

    return ;
} /* end print_vertices */

BOOL find_tile( llx , lly , urx , ury )
INT *llx , *lly , *urx , *ury ;
{

    VERTEX_BOX *vertex , *lvertex , *rvertex , *tvertex ;
    INT lowest , left_edge , right_edge , top_edge ;
    BOOL found ; 

    /* find the leftmost of the lowest START vertices */
    /* this is where I start to write slow, but effective algs */

TOP:
    lowest = INT_MAX ;
    left_edge = INT_MAX ;
    lvertex = NULL ;
    for(vertex = vertex_listS ;vertex ; vertex = vertex->next ){
	if( vertex->class == START ) {
	    if( vertex->y < lowest ) {
		lowest = vertex->y ;
		lvertex = vertex ;
		left_edge = vertex->x ;
	    } else if( vertex->y == lowest ) {
		if( vertex->x < left_edge ) {
		    left_edge = vertex->x ;
		    lvertex = vertex ;
		}
	    }
	}
    } /* end for loop */
    if( lvertex == NULL ) {
	return(FALSE);  /* no more tiles exist */
    }

    /* find the END vertex whose y is lowest */
    /* and whose x is the nearest to the right of left_edge */
    right_edge = INT_MAX ;
    rvertex = NULL ;
    for( vertex = vertex_listS;vertex;vertex = vertex->next ){
	if( vertex->class == END ) {
	    if( vertex->y == lowest ) {
		if( vertex->x >= left_edge ) {
		    if( vertex->x < right_edge ) {
			right_edge = vertex->x ;
			rvertex = vertex ;
		    }
		}
	    }
	}
    } /* end for loop */
    if(!(rvertex)) {
	lvertex->class = NOTHING ;
	goto TOP ;
    }

    /* now find ANY vertex whose x is between left_edge and */
    /* right_edge and whose y is greater than lowest */
    top_edge = INT_MAX ;
    tvertex = NULL ;
    for(vertex = vertex_listS ;vertex ; vertex = vertex->next ){
	if( vertex->x >= left_edge && vertex->x <= right_edge ) {
	    if( vertex->y > lowest ) {
		if( vertex->y < top_edge ) {
		    top_edge = vertex->y ;
		    tvertex = vertex ;
		}
	    }
	}
    } /* end for loop */

    if( tvertex == NULL || left_edge == right_edge ||
					    lowest == top_edge ) {
	lvertex->class = NOTHING ;
	if( rvertex ) {
	    rvertex->class = NOTHING ;
	}
	goto TOP ;
    }


    /* change classes to NOTHING for the selected vertices */
    lvertex->class = NOTHING ;
    rvertex->class = NOTHING ;

    /* here is the tile */
    *llx = left_edge ;
    *lly = lowest ;
    *urx = right_edge ;
    *ury = top_edge ;


    /*  the generated tile has four vertices; if any of these vertices */
    /*  is not currently in the list (only the two top ones could be new) */
    /*  we need to add them */
    found = FALSE ;
    for(vertex = vertex_listS ;vertex ; vertex = vertex->next ){
	if( vertex->x == *llx && vertex->y == *ury ) {
	    found = TRUE ;
	    break ;
	}
    } /* end for loop */

    if( !found ) {
	last_vertexS = last_vertexS->next = 
			    (VERTEX_BOX *) Ysafe_malloc(sizeof(VERTEX_BOX));
	last_vertexS->x = *llx ;
	last_vertexS->y = *ury ;
	last_vertexS->next = NULL ;
	last_vertexS->macro = 0 ;
	last_vertexS->class = START ;
    }

    found = FALSE ;
    for( vertex = vertex_listS ;vertex ; vertex = vertex->next ){
	if( vertex->x == *urx && vertex->y == *ury ) {
	    found = TRUE ;
	    break ;
	}
    } /* end for loop */
    if( !found ) {
	last_vertexS = last_vertexS->next = 
			    (VERTEX_BOX *) Ysafe_malloc(sizeof(VERTEX_BOX));
	last_vertexS->x = *urx ;
	last_vertexS->y = *ury ;
	last_vertexS->next = NULL ;
	last_vertexS->macro = 0 ;
	last_vertexS->class = END ;
    }

    return(TRUE) ;
} /* end find_tile */


BOOL read_vertices(fp,initial)
FILE *fp ;
BOOL initial ;
{
    /* format of input file: */
    /* stdcell_length [x]  */
    /* actual_row_height [y]        --->DOES NOT include channel sep */ 
    /* channel_separation [y] */ 
    /* min_length [x]	 */
    /* core [x] [y] [x] [y]  */
    /* macro [n] vertices [x][y] ... [x][y] */
    /* etc. */

    VERTEX_BOX *check_vertex , *vertex ;
    MACROPTR mptr ;
    TILE_BOX *tptr ;
    char string[LRECL] ;
    INT vert_count ;
    INT row_length ;
    INT x , y , i , next_x , next_y ;
    INT l, r, b, t ;
    INT last_direction , num_vertices , next_direction ;
    INT macro , check_macro ;
    INT xgrid, ygrid ;


    /* I will assume that the input data is syntactically correct */
    fscanf(fp,"%s", string ) ; /* "total_row_length" */
    fscanf(fp, INTSCANSTR, &row_length ) ;
    if( initial ){
	stdcell_lengthS = row_length ;
    } else {
	if( stdcell_lengthS != row_length ){
	    sprintf( YmsgG, 
	    "Restart file row_length does not match:input(%d) vs restart(%d)\n",
	    stdcell_lengthS, row_length ) ;
	    M( ERRMSG, "read_vertices", YmsgG ) ;
	    M( ERRMSG, NULL, "\tCircuit or grid has changed.  Must abort restore state.\n\n" ) ;
	    return( FALSE ) ;
	}
    }

    macro = 0 ;
    /* if an old macroArray exists blow it away */ 
    if( macroArrayG ){
	for( i = 1; i <= num_macrosG; i++ ){
	    Ysafe_free( macroArrayG[i] ) ;
	}
	Ysafe_free( macroArrayG ) ;
    }
    free_structures( TRUE ) ; /* free all points */

    fscanf(fp,"%s", string ) ; /* "actual_row_height" */
    fscanf(fp, INTSCANSTR, &actual_row_heightS ) ;
    fscanf(fp,"%s", string ) ; /* "channel_separation" */
    fscanf(fp, INTSCANSTR, &channel_separationS ) ;
    fscanf(fp,"%s", string ) ; /* "min_length" */
    fscanf(fp, INTSCANSTR, &min_lengthS ) ;
    fscanf(fp,"%s", string ) ; /* "core" */
    fscanf(fp, INTSCANSTR " " INTSCANSTR, &cx1S , &cy1S ) ;
    fscanf(fp, INTSCANSTR " " INTSCANSTR, &cx2S , &cy2S ) ;
    fscanf(fp,"%s", string ) ; /* "grid" */
    fscanf(fp, INTSCANSTR " " INTSCANSTR, &xgrid , &ygrid ) ;
    fscanf(fp,"%s", string ) ; /* "num_macros" */
    fscanf(fp, INTSCANSTR, &num_macrosG ) ;

    if( stdcell_lengthS <= 0 ){
	M( ERRMSG, "genrow", "No standard cells were found.\n" ) ;
	M( MSG, NULL, "Make sure Mincut was executed properly.\n" ) ;
	M( MSG, NULL, "Must exit.\n" ) ;
	if( graphicsG ){
	    G( TWcloseGraphics() ) ;
	}
	YexitPgm( PGMFAIL ) ;
    }

    if( min_lengthS > stdcell_lengthS ){
	M( WARNMSG, "read_vertices", 
	    "Total row length is smaller that minimum tile length\n" ) ;
	M( MSG, NULL,
	    "Minimum tile set to total row length / 2\n" ) ;
	min_lengthS = stdcell_lengthS / 2 ;
    }

    init_vertex_list( cx1S, cy1S, cx2S, cy2S ) ;

    macroArrayG = (MACROPTR *) Ysafe_malloc( (num_macrosG+1) * sizeof(MACROPTR) ) ;
    for( i = 1; i <= num_macrosG; i++ ){
	macroArrayG[i] = (MACROPTR) Ysafe_malloc( sizeof(MACROBOX) ) ;
	macroArrayG[i]->tiles = NULL ;
    }

    if( num_macrosG == 0 ){
	/* no work to do */
	return( TRUE ) ;
    }
    while( fscanf(fp,"%s", string ) == 1 ) {  /* "macro" */
	/* now initialize vertices array */
	mptr = macroArrayG[++macro] ;
	fscanf(fp,"%s", string ) ;  /* orient */
	fscanf(fp, INTSCANSTR, &(mptr->orient) ) ;
	fscanf(fp, INTSCANSTR, &num_vertices ) ;
	fscanf(fp,"%s", string ) ; /* "vertices" */
	mptr->vertices = (VERTEXPTR *) Ysafe_malloc( num_vertices * sizeof(VERTEXPTR));
	mptr->num_vertices = num_vertices ;
	vert_count = 0 ;
	l = INT_MAX ;
	r = INT_MIN ;
	b = INT_MAX ;
	t = INT_MIN ;
	for( i = 0 ; i < num_vertices ; i++ ) {
	    fscanf(fp, INTSCANSTR " " INTSCANSTR, &x , &y ) ;

	    if( initial && (x < cx1S || x > cx2S || y < cy1S || y > cy2S) ) {
		printf("a macro vertex lies outside the core\n");
		printf("this is not allowed\n");
		printf("macro:%d x:%d y:%d\n",macro,x,y ) ;
		printf("core: l:%d r:%d b:%d t:%d\n",cx1S,cx2S,cy1S,cy2S ) ;
		if( graphicsG ){
		    G( TWcloseGraphics() ) ;
		}
		YexitPgm(PGMFAIL);
	    }
	    /* now save the vertex */
	    vertex = (VERTEX_BOX *) Ysafe_malloc(sizeof(VERTEX_BOX));
	    vertex->x = x ;
	    vertex->y = y ;
	    vertex->macro = macro ;
	    vertex->next = NULL ;

	    /* save the bounding box of macro */
	    l = MIN( l, x ) ;
	    r = MAX( r, x ) ;
	    b = MIN( b, y ) ;
	    t = MAX( t, y ) ;

	    /* now save vertex in macro vertex array */
	    mptr->vertices[vert_count++] = vertex ;
	}
	/* now set the xcenter and ycenter fields */
	mptr->xcenter = (l + r) / 2 ;
	mptr->ycenter = (b + t) / 2 ;
	/* set the relative bounding box */
	mptr->left = l - mptr->xcenter ;
	mptr->right = r - mptr->xcenter ;
	mptr->bottom = b - mptr->ycenter ;
	mptr->top = t - mptr->ycenter ;

	if( macro == num_macrosG ){
	    break ;
	} 

    } /* end while loop */

    return( TRUE ) ;

} /* end read_vertices */

BOOL restore_state( fp ) 
FILE *fp ;
{
    INT i ;
    INT numtiles ;
    INT cur_numtiles ;
    INT start ;
    INT diff ;
    char string[LRECL] ;
    TILE_BOX *tptr ;
    TILE_BOX *newtile ;
    TILE_BOX *last_tile ;

    /* feed_lengthS */
    fscanf(fp,"%s " INTSCANSTR, string , &feed_lengthS ) ;
    /* spacing */
    fscanf(fp,"%s " INTSCANSTR, string , &spacingG ) ;
    /* numtiles */
    fscanf(fp,"%s " INTSCANSTR, string , &numtiles ) ;
    cur_numtiles = 0 ;
    for( tptr=tile_listG;tptr;tptr=tptr->next ){
	/* count number of tiles for maintaining memory req. */
	cur_numtiles++ ;
    }
    if( numtiles > cur_numtiles ){
	/* add to the number of tiles */
	diff = numtiles - cur_numtiles ;
	tptr = last_tileG ;
	for( i =1; i <= diff; i++ ){
	    newtile = (TILE_BOX *) Ysafe_malloc( sizeof(TILE_BOX) ) ;
	    newtile->allocated = TRUE ;
	    tptr->next = newtile ;
	    newtile->prev = tptr ;
	    tptr = newtile ;
	}
	newtile->next = NULL ;
	last_tileG = newtile ;

    } else if( numtiles < cur_numtiles ){
	/* free up some tiles */
	cur_numtiles = 0 ;
	for( tptr=tile_listG;tptr;tptr=tptr->next ){
	    if( ++cur_numtiles == numtiles ){
		break ;
	    }
	}
	last_tileG = tptr ;
	/* free the tile structure */
	for( tptr=tptr->next;tptr; ){
	    last_tile = tptr ;
	    tptr = tptr->next ;
	    Ysafe_free( last_tile ) ;
	}
	last_tileG->next = NULL ;
	update_tile_memory( FALSE ) ;

    }
    /* now read in tiles */
    for( tptr=tile_listG;tptr;tptr=tptr->next ){
	/* 1 if this is the start tile */
	fscanf( fp, INTSCANSTR, &start ) ;
	if( start ){
	    start_tileG = tptr ;
	}
	/* llx, lly, urx, ury, force, class mirror */
	fscanf(fp, INTSCANSTR " " INTSCANSTR " " INTSCANSTR " " INTSCANSTR
		" " INTSCANSTR " " INTSCANSTR " " INTSCANSTR,
	    &(tptr->llx), &(tptr->lly), &(tptr->urx), &(tptr->ury),
	    &(tptr->force), &(tptr->class), &(tptr->mirror) ) ;

	/* numrows, actual_row_height, add_no_more_than, chansep */
	fscanf(fp, INTSCANSTR " " INTSCANSTR " " INTSCANSTR " " INTSCANSTR,
	&(tptr->numrows), &(tptr->actual_row_height),
	&(tptr->add_no_more_than), &(tptr->channel_separation) ) ;
	/* min_length, row_start, max_length, illegal */
	fscanf(fp, INTSCANSTR " " INTSCANSTR " " INTSCANSTR " " INTSCANSTR,
	    &(tptr->min_length), &(tptr->row_start),
	    &(tptr->max_length), &(tptr->illegal) ) ;
    } /* end tile loop */

    return( TRUE ) ;
}

save_state(fp)
FILE *fp ;
{
    INT i ;
    INT vert ;
    INT cur_numtiles ;
    TILE_BOX *tptr ;
    MACROPTR mptr ;
    VERTEXPTR pt ;

    fprintf( fp, "total_row_length %d\n", stdcell_lengthS ) ;
    fprintf( fp, "actual_row_height %d\n", actual_row_heightS ) ;
    fprintf( fp, "channel_separation %d\n", channel_separationS ) ;
    fprintf( fp, "min_length %d\n", min_lengthS ) ;
    fprintf( fp, "core %d %d %d %d\n", cx1S, cy1S, cx2S, cy2S ) ;
    fprintf( fp, "grid 0 0\n" ) ;
    fprintf( fp, "num_macro %d\n", num_macrosG ) ;
    for( i = 1; i <= num_macrosG; i++ ){
	mptr = macroArrayG[i] ;
	fprintf( fp, "macro orient %d %d vertices ", 
	    mptr->orient, mptr->num_vertices ) ;
	for( vert = 0; vert < mptr->num_vertices; vert++ ){
	    pt = mptr->vertices[vert] ;
	    fprintf( fp, "%d %d ", pt->x, pt->y ) ;
	}
	fprintf( fp, "\n" ) ;
    }
    fprintf( fp, "feed_length %d\n", feed_lengthS ) ;
    fprintf( fp, "spacing %d\n", spacingG ) ;
    cur_numtiles = 0 ;
    for( tptr=tile_listG;tptr;tptr=tptr->next ){
	/* count number of tiles for error checking */
	cur_numtiles++ ;
    }
    fprintf( fp, "numtiles %d\n", cur_numtiles ) ;
    for( tptr=tile_listG;tptr;tptr=tptr->next ){
	/* 1 if this is the start tile */
	if( tptr == start_tileG ){
	    fprintf( fp, "1 " ) ;
	} else {
	    fprintf( fp, "0 " ) ;
	}
	/* llx, lly, urx, ury, force, class mirror */
	fprintf( fp,"%d %d %d %d %d %d %d ", 
	    tptr->llx, tptr->lly, tptr->urx, tptr->ury, tptr->force,
	    tptr->class, tptr->mirror ) ;
	/* numrows, actual_row_height, add_no_more_than, chansep*/
	fprintf(fp,"%d %d %d %d ", 
	    tptr->numrows, tptr->actual_row_height,
	    tptr->add_no_more_than, tptr->channel_separation ) ;
	/* min_length, row_start, max_length, illegal */
	fprintf(fp,"%d %d %d %d\n", 
	    tptr->min_length, tptr->row_start, tptr->max_length, tptr->illegal ) ;
    } /* end tile loop */

} /* end save_state */

process_vertices()
{

    VERTEXPTR check_vertex , vertex ;
    VERTEXPTR pt_ptr ;
    VERTEXPTR *vptr ;
    MACROPTR mptr ;
    char string[LRECL] ;
    INT vert_count ;
    INT x , y , i , next_x , next_y ;
    INT last_direction , num_vertices , next_direction ;
    INT macro , check_macro ;
    INT xgrid, ygrid ;


    for( macro = 1; macro <= num_macrosG; macro++ ){

	mptr = macroArrayG[macro] ;
	vptr = mptr->vertices ;
	num_vertices = mptr->num_vertices ;
	last_direction = FROM_RIGHT ;
	x = vptr[0]->x ;
	y = vptr[0]->y ;
	for( i = 1 ; i <= num_vertices ; i++ ) {
	    pt_ptr = vptr[i-1] ;
	    /* link all the vertices */
	    last_vertexS->next = pt_ptr ;
	    last_vertexS = pt_ptr ;
	    if( i < num_vertices ) {
		next_x = vptr[i]->x ;
		next_y = vptr[i]->y ;
		if( next_x == x ) {
		    if( next_y > y ) {
			next_direction = TO_TOP ;
		    } else {
			next_direction = TO_BOTTOM ;
		    }
		} else {
		    if( next_x > x ) {
			next_direction = TO_RIGHT ; 
		    } else {
			next_direction = TO_LEFT ;
		    }
		}
	    } else {
		next_direction = TO_LEFT ;
	    }

	    if( last_direction == FROM_BOTTOM ) {
		if( next_direction == TO_RIGHT ) {
		    pt_ptr->class = MAYBE_START ;
		} else { /* TO_LEFT */
		    pt_ptr->class = NOTHING ;
		}
	    } else if( last_direction == FROM_TOP ) {
		if( next_direction == TO_RIGHT ) {
		    pt_ptr->class = START ;
		} else { /* TO_LEFT */
		    pt_ptr->class = START ;
		}
	    } else if( last_direction == FROM_LEFT ) {
		if( next_direction == TO_TOP ) {
		    pt_ptr->class = END ;
		} else { /* TO_BOTTOM */
		    pt_ptr->class = MAYBE_END ;
		}
	    } else { /* last_direction == FROM_RIGHT */
		if( next_direction == TO_TOP ) {
		    pt_ptr->class = END ;
		} else { /* TO_BOTTOM */
		    pt_ptr->class = NOTHING ;
		}
	    }
	    if( next_direction == TO_TOP ) {
		last_direction = FROM_BOTTOM ;
	    } else if( next_direction == TO_BOTTOM ) {
		last_direction = FROM_TOP ;
	    } else if( next_direction == TO_RIGHT ) {
		last_direction = FROM_LEFT ;
	    } else {  /* next_direction == TO_LEFT  */
		last_direction = FROM_RIGHT ;
	    }

	    x = next_x ; /* set for the next time thru the loop */
	    y = next_y ;
	}
    }

    /*  see if the MAYBE_START vertices should really be START  */
    /*  this transformation happens when macros abut or when a  */
    /*  macro touches the left or right core boundary */
    for( check_vertex=vertex_listS;check_vertex;check_vertex=check_vertex->next ){
	if( check_vertex->class == MAYBE_START ) {
	    /* perform the check */
	    check_macro = check_vertex->macro ;
	    for( vertex = vertex_listS ;vertex ; vertex = vertex->next ){
		if( vertex->macro != check_macro && 
			    vertex->next &&
			    vertex->macro == vertex->next->macro && 
			    vertex->x == vertex->next->x &&
			    vertex->x == check_vertex->x ) {
		    if( vertex->y > vertex->next->y ) {
			if( check_vertex->y > vertex->next->y &&
				    check_vertex->y < vertex->y ) {
			    check_vertex->class = START ;
			    break ;
			}
		    } else {
			if( check_vertex->y < vertex->next->y &&
				    check_vertex->y > vertex->y ) {
			    check_vertex->class = START ;
			    break ;
			}
		    }
		}
	    } /* end for loop */
	} /* end if */
    } /* end for loop */

    for( check_vertex=vertex_listS;check_vertex;check_vertex=check_vertex->next ){
	if( check_vertex->class == MAYBE_END ) {
	    /* perform the check */
	    check_macro = check_vertex->macro ;
	    for( vertex = vertex_listS ;vertex ; vertex = vertex->next ){
		if( vertex->macro != check_macro && 
			    vertex->next &&
			    vertex->macro == vertex->next->macro && 
			    vertex->x == vertex->next->x &&
			    vertex->x == check_vertex->x ) {
		    if( vertex->y > vertex->next->y ) {
			if( check_vertex->y > vertex->next->y &&
				    check_vertex->y < vertex->y ) {
			    check_vertex->class = END ;
			    break ;
			}
		    } else {
			if( check_vertex->y < vertex->next->y &&
				    check_vertex->y > vertex->y ) {
			    check_vertex->class = END ;
			    break ;
			}
		    }
		}
	    } /* end inner for loop */
	} /* end if */
    } /* end outer for loop */

    return ;
} /* end process_vertices */

build_macros()
{
    INT i ;
    INT j ;
    MACROPTR mptr ;
    VERTEXPTR vptr ;
    MACRO_TILE *tptr ;
    MACRO_TILE *last_tile ;
    YBUSTBOXPTR bustptr ;

    if( num_macrosG <= 0 ){
	return ;
    }

    /* otherwise we have macro tiles to build */
    for( i = 1 ; i <= num_macrosG ; i++ ){
	mptr = macroArrayG[i] ;
	/* first remove old tiles if they exist */
	for( tptr = mptr->tiles;tptr; ){
	    last_tile = tptr ;
	    tptr = tptr->next ;
	    Ysafe_free( last_tile ) ;
	}
	mptr->tiles = NULL ;

	Ybuster_init() ;
	for( j = 0; j < mptr->num_vertices; j++ ){
	    vptr = mptr->vertices[j] ;
	    Ybuster_addpt( vptr->x, vptr->y ) ;
	}
	while( bustptr = Ybuster() ){
	    /* l = bustptr[1].x ; */
	    /* r = bustptr[4].x ; */
	    /* b = bustptr[1].y ; */
	    /* t = bustptr[2].y ; */
	    last_tile = mptr->tiles  ;
	    tptr = mptr->tiles = (MACRO_TILE *) Ysafe_malloc( sizeof(MACRO_TILE) ) ;
	    tptr->next = last_tile ;
	    /* now fill in the data */
	    tptr->l = bustptr[1].x ;
	    tptr->r = bustptr[4].x ;
	    tptr->b = bustptr[1].y ;
	    tptr->t = bustptr[2].y ;
	}
    }
    Ybuster_free() ;

    check_overlap() ;

} /* end build_macros */


makerows()
{

    TILE_BOX *tileptr, *get_starting_tile() ;
    ROW_BOX *rowptr , *segment ;
    DOUBLE amount , total ;
    INT row_length_so_far , count , actually_added , add_no_more_than ;
    INT num_segments, llx, width ;
    BOOL top_rows ;


    row_length_so_far = 0 ;
    number_of_rows_addedS = 0 ;
    number_of_segs_addedS = 0 ;

    /* traverse thru tile list */
    if( !(tileptr = start_tileG )){
        tileptr = tile_listG ;
    }
    total_row_lengthS = stdcell_lengthS + feed_lengthS ;
    total_row_segmentS = 0 ;
    for( ; tileptr ; tileptr = tileptr->next ){

	add_no_more_than = total_row_lengthS - row_length_so_far ;
	row_length_so_far +=
	    convert_tile_to_rows( tileptr, add_no_more_than ) ;
    }

    /*  Check if we can get row_length_so_far closer to total_row_length
	by adding a row across the top of the core  */
    if( last_tileG->numrows >= 1 ){
	top_rows = TRUE ;
    } else {
	top_rows = FALSE ;
    }
    width = last_tileG->max_length ;
    llx = last_tileG->llx + last_tileG->row_start ;
    /* initiallly set width of row to max length of tile */
    while( cx2S - cx1S + row_length_so_far <= total_row_lengthS ) {
	if( last_rowptrS == NULL ) {
	    set_of_rowsS = (ROW_BOX *) Ysafe_malloc( sizeof( ROW_BOX ) ) ;
	    rowptr = last_rowptrS = set_of_rowsS ;
	} else {
	    rowptr = last_rowptrS->next_row = 
			(ROW_BOX *) Ysafe_malloc( sizeof( ROW_BOX ) ) ;
	    last_rowptrS = rowptr ;
	}
	top_rows = TRUE ;
	rowptr->next_row = NULL ;
	rowptr->next_segment = NULL ;
	rowptr->class = last_tileG->class ;
	rowptr->mirror = last_tileG->mirror ;
	rowptr->seg = ++number_of_segs_addedS ;
	rowptr->llx = cx1S ;
	rowptr->lly = cy2S ;
	rowptr->urx = cx2S ;
	rowptr->ury = cy2S + actual_row_heightS + channel_separationS ;

	cy2S += actual_row_heightS + channel_separationS ;
	row_length_so_far += cx2S - cx1S ;
	number_of_rows_addedS++ ;
    }
    /* make sure the last row is a full row if short */
    if(!(shortRowG)){
	if( top_rows ){
	    last_rowptrS->llx = llx ;
	    last_rowptrS->urx = llx + width ;
	}
    }
    D( "genrows/placed_amount",
	if( total_row_lengthS - row_length_so_far ){
	    fprintf( stderr, 
	    "Warning:Total row length requested exceeds placed row length by:%d\n",
	    total_row_lengthS - row_length_so_far ) ;
	} else {
	    fprintf( stderr, 
	    "Total row length requested equals placed row length\n") ;
	}
    ) ;

    /* now divide up feedthru length among segments */
    /* feed length is only an approximation so don't worry about remainder */
    if( total_row_segmentS > 0 ){
	feeds_per_segG = feed_lengthS / total_row_segmentS ;
    } else {
	feeds_per_segG = 0 ;
    }

    return ;
} /* end makerows */


BOOL convert_tile_to_rows( tileptr, add_no_more_than )
/*  Purpose:  given a rectangular tile region, generate a set of
	      rows which covers this region */
/* returns the total length of rows generated for this tile */
TILE_BOX *tileptr ;
INT add_no_more_than ;
{

    INT llx , lly ;  /* lower left  x and y coordinates */
    INT urx , ury ;  /* upper right x and y coordinates */
    INT width ;
    INT totalw ;
    INT shortage ;
    INT actual_row_height ;
    INT channel_separation ;
    ROW_BOX *rowptr , *saveptr ;
    INT horiz_line , mismatch , overlap ;
    INT row_segments_generated ;
    INT chan_sep_below_row , row_height ;
    INT half_chan_sep_above_row ;
    INT chan_sep_above_row ;
    INT limit ;
    INT numrows ;
    INT proposed_length ;

    /* form local variables for speed */

    actual_row_height = tileptr->actual_row_height ;
    channel_separation = tileptr->channel_separation ;
    llx = tileptr->llx ;
    lly = tileptr->lly ;
    urx = tileptr->urx ;
    ury = tileptr->ury ;

    /* reset add_no_more_than parameter if uses has set it for this tile*/
    if( tileptr->add_no_more_than ){
	add_no_more_than = 
	    MIN( add_no_more_than, tileptr->add_no_more_than ) ;
    }

    row_height = actual_row_height + channel_separation ;
    chan_sep_below_row = channel_separation / 2 ;
    chan_sep_above_row = channel_separation - channel_separation / 2 ;
    half_chan_sep_above_row = chan_sep_above_row / 2 ;
    width = tileptr->max_length ;  /* initiallly set width of row to max length of tile */

    if( urx - llx < tileptr->min_length || tileptr->illegal ||
	width < tileptr->min_length ) {
	return(FALSE) ;
    }

    row_segments_generated = 0 ;

    totalw = 0 ;
    horiz_line = lly ;
    /* now calculate the new llx base on the given offset from beginning of tile */
    llx += tileptr->row_start ;  /* the offset from beginning of tile normally 1 */
    if( tileptr->force && tileptr->numrows > 0 ){
	/* see if we need to modify the start and stop of row to accommodate */
	/* exactly  n rows */
	proposed_length = tileptr->numrows * width ;
	numrows = tileptr->numrows ;
	if( proposed_length > add_no_more_than ){
	    shortage = proposed_length - add_no_more_than ;
	    /* calculate shortage per row */
	    shortage /= tileptr->numrows ;
	    tileptr->max_length = width -= shortage ;
	    /* break up the shortage evenly between start and stop */
	    /*
	    shortage /= 2 ;
	    llx += shortage ;
	    tileptr->row_start += shortage ;
	    */
	}
    } else {
	numrows = 0 ;
    }
    /* now reset the number of rows */
    tileptr->numrows = 0 ;

    while( horiz_line + chan_sep_below_row + actual_row_height +
			    half_chan_sep_above_row <= ury ){
	if( totalw >= add_no_more_than ){
	    /* we are done */
	    break ;
	}
	if( tileptr->force && tileptr->numrows > 0 && tileptr->numrows >= numrows){
	    break ;
	}
	/* see if we will get closer if we add another segment */
	proposed_length = (row_segments_generated+1) * tileptr->max_length ;
	if( proposed_length > add_no_more_than ) {
	    /* try setting width to the shortage if we allow a short row  */
	    if( shortRowG || 
		( !(shortRowG) && number_of_rows_addedS < 1) ){
		/* second clause is special case when only very */
		/* small number of standard cells */
		width = add_no_more_than - totalw ;
	    } else {
		/* we will round up or down to the closest row */
		limit = (INT) ( (DOUBLE) 0.50 * (DOUBLE) width ) ;
		if( proposed_length - add_no_more_than > limit ){
		    /* we will be over limit if we add another row */
		    break ;
		}
	    }
	}
	/* ***********************************************************
	Propose a new row :
	proposed row coord's:
	    (llx, horiz_line + chan_sep_below_row),
	     (llx+width,horiz_line + chan_sep_below_row + actual_row_height )
	check if proposed row has a partial overlap - in the y
	   direction - with any other row segment.  if so, we
	   merge the two and create an exception 
        *************************************************************/
	for( rowptr = set_of_rowsS ;rowptr; rowptr = rowptr->next_row ) {
	    if( !(rowptr->lly - chan_sep_below_row >= horiz_line + 
		    channel_separation + actual_row_height ||
		    rowptr->ury + chan_sep_above_row <= horiz_line) ) {
		/* try to line up these row segments */
		/* try to shift up proposed row by an amount equal
		    to their mismatch in the y direction */
		mismatch = rowptr->lly - 
		    (horiz_line + chan_sep_below_row) ;
		if( mismatch < 0 ) {
		    horiz_line += channel_separation + 
			actual_row_height + mismatch ;
		} else {
		    horiz_line += mismatch ;
		}
		if( horiz_line + chan_sep_below_row + 
		    half_chan_sep_above_row + actual_row_height > ury ) {
		    /* row won't fit anymore */
		    return(FALSE) ;
		}
		break ;
	    } 
	} /* end for loop */

	/* check if we need to add a row segment to an existing row */
	overlap = FALSE ;
	for( rowptr = set_of_rowsS ;rowptr; rowptr = rowptr->next_row ) {
	    if( !(rowptr->lly - chan_sep_below_row >= horiz_line + 
		channel_separation + actual_row_height ||
		rowptr->ury + chan_sep_above_row <= horiz_line) ) {
		/* the proposed row is thus part of a larger row */
		tileptr->numrows++ ;
		saveptr = rowptr->next_segment ;
		rowptr->next_segment = 
			(ROW_BOX *) Ysafe_malloc( sizeof( ROW_BOX ) ) ;
		rowptr->next_segment->next_segment = saveptr ;
		rowptr = rowptr->next_segment ;
		rowptr->class = tileptr->class ;
		rowptr->mirror = tileptr->mirror ;
		rowptr->seg = ++number_of_segs_addedS ;
		rowptr->llx = llx ;
		rowptr->lly = horiz_line + chan_sep_below_row ;
		rowptr->urx = llx + width ;
		rowptr->ury = horiz_line + chan_sep_below_row + 
					    actual_row_height ;
		overlap = TRUE ;
		break ;
	    }
	} /* end for loop */

	if( !overlap ) {
	    /* start a new row */
	    number_of_rows_addedS++ ;
	    tileptr->numrows++ ;
	    if(!(last_rowptrS)) {
		set_of_rowsS = (ROW_BOX *) 
		    Ysafe_malloc( sizeof( ROW_BOX ) ) ;
		set_of_rowsS->next_row = NULL ;
		set_of_rowsS->next_segment = NULL ;
		set_of_rowsS->class = tileptr->class ;
		set_of_rowsS->mirror = tileptr->mirror ;
		set_of_rowsS->seg = ++number_of_segs_addedS ;
		set_of_rowsS->llx = llx ;
		set_of_rowsS->lly = horiz_line + chan_sep_below_row ;
		set_of_rowsS->urx = llx + width ;
		set_of_rowsS->ury = horiz_line + chan_sep_below_row + 
						    actual_row_height ;
		last_rowptrS = set_of_rowsS ;
	    } else {
		last_rowptrS->next_row = (ROW_BOX *) 
		    Ysafe_malloc( sizeof( ROW_BOX ) ) ;
		last_rowptrS = last_rowptrS->next_row ;
		last_rowptrS->next_row = NULL ;
		last_rowptrS->next_segment = NULL ;
		last_rowptrS->class = tileptr->class ;
		last_rowptrS->mirror = tileptr->mirror ;
		last_rowptrS->seg = ++number_of_segs_addedS ;
		last_rowptrS->llx = llx ;
		last_rowptrS->lly = horiz_line + chan_sep_below_row ; 
		last_rowptrS->urx = llx + width ;
		last_rowptrS->ury = horiz_line + chan_sep_below_row + 
						    actual_row_height ;
	    }
	}
	row_segments_generated++ ;
	horiz_line += row_height ;
	totalw += width ;
    }
    total_row_segmentS += row_segments_generated ;
    return( totalw ) ;
} /* end convert_tile_to_rows */

divide_tile( tile , horiz_line ) 
TILE_BOX *tile ;
INT horiz_line ;
{

    TILE_BOX *new_tile ;
    INT orig_top ;

    new_tile = (TILE_BOX *) Ysafe_malloc( sizeof(TILE_BOX) ) ;
    new_tile->next = tile->next ;
    tile->next = new_tile ;

    /* now fix the backlinks */
    new_tile->prev = tile ;
    new_tile->next->prev = new_tile ;

    orig_top = tile->ury ;
    tile->ury = horiz_line ; /* the orig. tile stops at horiz_line */

    new_tile->lly = horiz_line ;/* the new tile begins at horiz_line */
    new_tile->ury = orig_top ;/* the new tile top is the old one's top */
    new_tile->llx = tile->llx ;
    new_tile->urx = tile->urx ;

    new_tile->merged = MERGED_NONE ;
    new_tile->actual_row_height = tile->actual_row_height ;
    new_tile->channel_separation = tile->channel_separation ;
    new_tile->min_length = tile->min_length ;
    new_tile->max_length = tile->max_length ;
    new_tile->row_start = tile->row_start ;
    new_tile->numrows = 0 ;
    new_tile->illegal = tile->illegal ;
    new_tile->force = tile->force ;
    new_tile->add_no_more_than = tile->add_no_more_than ;
    new_tile->class = tile->class ;
    new_tile->mirror = tile->mirror ;
    new_tile->allocated = TRUE ;

    return ;

} /* end divide_tile */

divide_tilelr( tile , vert_line ) 
TILE_BOX *tile ;
INT vert_line ;
{

    TILE_BOX *new_tile ;
    INT orig_right ;

    new_tile = (TILE_BOX *) Ysafe_malloc( sizeof(TILE_BOX) ) ;
    new_tile->next = tile->next ;
    tile->next = new_tile ;

    /* now fix the backlinks */
    new_tile->prev = tile ;
    new_tile->next->prev = new_tile ;

    orig_right = tile->urx ;
    tile->urx = vert_line ; /* the orig. tile stops at vert_line */

    new_tile->llx = vert_line ;/* the new tile begins at vert_line */
    new_tile->urx = orig_right ;/* the new tile top is the old one's rite */
    new_tile->lly = tile->lly ;
    new_tile->ury = tile->ury ;

    new_tile->merged = MERGED_NONE ;
    new_tile->actual_row_height = tile->actual_row_height ;
    new_tile->channel_separation = tile->channel_separation ;
    new_tile->min_length = tile->min_length ;
    new_tile->row_start = tile->row_start ;
    new_tile->numrows = 0 ;
    new_tile->illegal = tile->illegal ;
    new_tile->force = tile->force ;
    new_tile->add_no_more_than = tile->add_no_more_than ;
    new_tile->class = tile->class ;
    new_tile->mirror = tile->mirror ;
    new_tile->max_length = new_tile->urx - new_tile->llx - 2 * spacingG ;
    tile->max_length = tile->urx - tile->llx - 2 * spacingG ;
    new_tile->allocated = TRUE ;

    return ;

} /* end divide_tilelr */

get_core( left, bottom, right, top, tileFlag )
INT *left, *bottom, *right, *top ;
BOOL tileFlag ;  /* if true include the tiles in core */
{

    INT l, r, b, t ;
    TILE_BOX *tileptr ;

    /* expanded core is max over given core and tiles */

    l = cx1S ;
    b = cy1S ;
    r = cx2S ;
    t = cy2S ;

    if( tileFlag ){

	for( tileptr=tile_listG;tileptr;tileptr=tileptr->next ){
	    l = MIN( l, tileptr->llx ) ;
	    b = MIN( b, tileptr->lly ) ; 
	    r = MAX( r, tileptr->urx ) ;
	    t = MAX( t, tileptr->ury ) ;
	}
    }
    D( "genrows/get_core",printf( "l:%d r:%d b:%d t:%d\n", l,r,b,t ));
    *left = l ;
    *right = r ;
    *bottom = b ;
    *top = t ;
    return ;
} /* end get_core */

ROW_BOX  *get_rowptr()
{
    return( set_of_rowsS ) ;
} /* end get_rowptr */

grid_rows()
{
    ROW_BOX *rowptr , *segment ;
    INT oldX, oldY ;

    for( rowptr = set_of_rowsS ; rowptr ; rowptr = rowptr->next_row ){
	oldX = rowptr->llx ;
	oldY = rowptr->lly ;
	YforceGrid( &(rowptr->llx), &(rowptr->lly) ) ;
	rowptr->urx += rowptr->llx - oldX ;
	rowptr->ury += rowptr->lly - oldY ;
	for( segment=rowptr->next_segment;segment;segment=segment->next_segment){
	    oldX = segment->llx ;
	    oldY = segment->lly ;
	    YforceGrid( &(segment->llx), &(segment->lly) ) ;
	    segment->urx += segment->llx - oldX ;
	    segment->ury += segment->lly - oldY ;
	}
    }
} /* end grid_rows */

set_minimum_length( length )
INT length ;
{
    TILE_BOX *tileptr ;      /* current tile */

    min_lengthS = length ;
    for( tileptr=tile_listG;tileptr;tileptr=tileptr->next ){
	tileptr->min_length = min_lengthS ;
    }
} /* end set_minimum_length */

set_row_separation( channel_sep_relative, channel_sep_absolute )
DOUBLE channel_sep_relative;
INT channel_sep_absolute;
{
    TILE_BOX *tileptr ;      /* current tile */

    channel_separationS =
	(INT) (channel_sep_relative *(DOUBLE) actual_row_heightS)
	+ channel_sep_absolute;
    for( tileptr=tile_listG;tileptr;tileptr=tileptr->next ){
	tileptr->channel_separation = channel_separationS ;
    }
} /* end set_row_separation */

set_spacing()
{
    TILE_BOX *tileptr ;      /* current tile */

    for( tileptr=tile_listG;tileptr;tileptr=tileptr->next ){
	tileptr->row_start = spacingG ;
	tileptr->max_length = tileptr->urx - tileptr->llx - 2 * spacingG ;
	if( tileptr->max_length < 0 ){
	    tileptr->max_length = 0 ;
	}
    }
} /* end set_spacing */

BOOL force_tiles()
{
    TILE_BOX *tile ;      /* current tile */

    for( tile=last_tileG; tile; tile = tile->prev ){
	if( tile == last_tileG ){
	    /* don't do anything in this case */
	    break ;
	}
	if( tile->numrows > 0 ){
	    tile->force = TRUE ;
	    return( TRUE ) ;
	}
    }
    return( FALSE ) ;
} /* end force_tiles */

check_user_data()
{
    if( min_lengthS < spacingG ){
	M( ERRMSG, "check_user_errors", 
	"minimum row length < tile_to_row_spacing. Reset\n" ) ;
	set_minimum_length( spacingG + 2 ) ;
    }
} /* end check_user_errors */

remakerows()
{
    ROW_BOX *freerow, *freeseg ;      /* used to free data */
    ROW_BOX *segptr ;                 /* traverse segments freeing them */

    /* free memory if allocated before calling make rows again */
    for( ; set_of_rowsS ; ){

	/* keep memory of where row was */
	/* first free segments of the row */
	for( segptr = set_of_rowsS->next_segment;segptr; ){
	    freeseg = segptr ;
	    segptr = segptr->next_segment ;
	    Ysafe_free( freeseg ) ;
	}
	freerow = set_of_rowsS ;
	set_of_rowsS = set_of_rowsS->next_row ;
	Ysafe_free( freerow ) ;
    }
    last_rowptrS = NULL ;

    /* now everything has been freed - ok to make rows */
    makerows() ;

    D( "genrows/numrows",
	fprintf( stderr, "Number of rows = %d\n", number_of_rows_addedS ) ;
    ) ;

} /* end remakerows */

init_vertex_list( left, bottom, right, top )
INT left, bottom, right, top ;
{

    /* start the vertex list */
    last_vertexS = vertex_listS = 
	    (VERTEX_BOX *) Ysafe_malloc( sizeof( VERTEX_BOX ) ) ;
    last_vertexS->x = right ;
    last_vertexS->y = bottom ;
    last_vertexS->macro = 0 ;
    last_vertexS->class = END ;
    last_vertexS = last_vertexS->next = 
	    (VERTEX_BOX *) Ysafe_malloc( sizeof( VERTEX_BOX ) ) ;
    last_vertexS->x = right ;
    last_vertexS->y = top ;
    last_vertexS->macro = 0 ;
    last_vertexS->class = NOTHING ;
    last_vertexS = last_vertexS->next = 
	    (VERTEX_BOX *) Ysafe_malloc( sizeof( VERTEX_BOX ) ) ;
    last_vertexS->x = left ;
    last_vertexS->y = top ;
    last_vertexS->macro = 0 ;
    last_vertexS->class = NOTHING ;
    last_vertexS = last_vertexS->next = 
	    (VERTEX_BOX *) Ysafe_malloc( sizeof( VERTEX_BOX ) ) ;
    last_vertexS->x = left ;
    last_vertexS->y = bottom ;
    last_vertexS->macro = 0 ;
    last_vertexS->class = START ;
    last_vertexS->next = NULL ;
} /* end init_vertex_list */

static VOID free_tile( tile )
TILE_BOX *tile ;
{
    Ysafe_free( tile ) ;
} /* end free_tile */

free_structures( allpts )
BOOL allpts ;
{

    TILE_BOX *last_tile ;
    VERTEXPTR vertex ;  /* current point */
    VERTEXPTR free_vertex ; /* used to free point */

    /* first initialize vertex array */
    for( vertex = vertex_listS ;vertex; ){
	free_vertex = vertex ;
	vertex = vertex->next ;
	if( allpts || free_vertex->macro == 0 ){
	    /* free only the non macro points */
	    Ysafe_free( free_vertex ) ;
	} else {
	    /* reset the link field */
	    free_vertex->next = NULL ;
	}
    }
}

update_tile_memory( free_flag )
BOOL free_flag ;
{
    TILE_BOX *tile ;

    /* free the tile structure */
    if( free_flag ){
	Yrbtree_empty( tile_memoryG, free_tile ) ;
    } else {
	Yrbtree_empty( tile_memoryG, NULL ) ;
    }
    /* put the new tiles in the tree */
    for( tile=tile_listG ;tile;tile = tile->next ){
	Yrbtree_insert( tile_memoryG, tile ) ;
    }
} /* end update_tile_memory */

recalculate( freepts )
BOOL freepts ;
{
    BOOL previous_invalid_state ;

    previous_invalid_state = invalidG ;
    update_tile_memory( TRUE ) ;
    if( freepts ){
	/* if we are asked to free points this means recalculation */
	/* is not from a reread of the points. Therefore keep macros*/
	free_structures( FALSE ) ;
    }
    find_core() ;
    init_vertex_list( cx1S, cy1S, cx2S, cy2S ) ;
    build_macros() ;
    process_vertices() ;
    process_tiles() ;
    merge_tiles() ;
    check_tiles() ;
    if( memoryG && !(previous_invalid_state) ){
	reset_tile_parameters() ;
    }
} /* end recalculate */

find_core()
{
    INT macro ;
    INT left, rite, bot, top ;
    INT i ;
    INT num_vertices ;
    MACROPTR mptr ;
    VERTEXPTR *vptr ;
    VERTEXPTR pt_ptr ;


    for( macro = 1; macro <= num_macrosG; macro++ ){

	mptr = macroArrayG[macro] ;
	vptr = mptr->vertices ;
	num_vertices = mptr->num_vertices ;
	for( i = 0 ; i < num_vertices ; i++ ) {
	    pt_ptr = vptr[i] ;
	    /* find the limits over all vertices */
	    cx1S = MIN( cx1S, pt_ptr->x ) ;
	    cx2S = MAX( cx2S, pt_ptr->x ) ;
	    cy1S = MIN( cy1S, pt_ptr->y ) ;
	    cy2S = MAX( cy2S, pt_ptr->y ) ;
	}
    }
} /* end find_core */

BOOL snap_core( l, r, b, t )
INT *l, *r, *b, *t ;
{
    INT width ;   /* new width of core */
    INT height ;   /* new width of core */
    INT numtiles ; /* number of tiles with rows */
    INT total_sep ; /* total row separation */
    TILE_BOX *tileptr ; /* current tile */

    if( num_macrosG > 0 ){
	return( FALSE ) ;
    }
    /* need to determine the current values of channel_separation */
    /* take the average of the tiles */
    total_sep = 0 ;
    numtiles = 0 ;
    for( tileptr=tile_listG;tileptr;tileptr=tileptr->next ){
	if( tileptr->numrows > 0 ){
	    total_sep += tileptr->channel_separation ;
	    numtiles++ ;
	}
    }
    /* set channel_separation to average channel separation */
    channel_separationS = total_sep / numtiles ;
    height = number_of_rows_addedS * actual_row_heightS +
	number_of_rows_addedS * channel_separationS ;
    /* take into account the start and end of the row thru spacingG */
    width = (total_row_lengthS+2*number_of_rows_addedS+
	2*spacingG*number_of_rows_addedS) / number_of_rows_addedS ;
    if( total_row_lengthS % number_of_rows_addedS ){
	width++ ;
    }
    *l = cx1S ;
    *r = cx1S + width ;
    *b = cy1S ;
    *t = cy1S + height ;
    return( TRUE ) ;
} /* end snap_core */

set_core( left, right, bottom, top )
INT left, right, bottom, top ;
{
    cx1S = left ;
    cx2S = right ;
    cy1S = bottom ;
    cy2S = top ;
} /* end set_core */

check_overlap()
{
    INT macro ;
    INT match ;
    MACRO_TILE *tilea ;
    MACRO_TILE *tileb ;
    INT projectX() ;
    INT projectY() ;

    invalidG = FALSE ;
    for( macro = 1; macro <= num_macrosG; macro++ ){
	tilea = macroArrayG[macro]->tiles ;
	if(!(tilea)){
	    continue ;
	}
	for( match = macro+1; match <= num_macrosG; match++ ){
	    tileb = macroArrayG[match]->tiles ;
	    if(!(tileb)){
		continue ;
	    }
	    /* now check for overlap */
	    if( projectY( tilea->b, tilea->t, tileb->b, tileb->t) ){

		if( projectX( tilea->l,tilea->r,tileb->l,tileb->r )){

		    sprintf( YmsgG, "Macro %d overlaps macro %d.\n",
			macro, match ) ;
		    M(ERRMSG, "check_overlap", YmsgG ) ;
		    invalidG = TRUE ;

		}
	    }
	}
    }

    if( invalidG ){
	M(ERRMSG, NULL, 
	"Results are invalid. Turn on draw macros to locate overlap\n\n");
    }

} /********* end check of tiles ***********/

static INT xspaceS = 0 ;
static INT yspaceS = 0 ;

/* function returns whether one cell projects onto another */
INT projectX( tile1_left, tile1_right, tile2_left, tile2_right )
INT tile1_left, tile1_right, tile2_left, tile2_right ;
{
    /* -----------------------------------------------------
       First check case 2nd tile larger than first 
	complete overlap
    */
    if( tile2_left <= tile1_left && tile1_right <= tile2_right ){
	return( OVERLAP1 ) ;

    /* -----------------------------------------------------
       Check if an edge of tile two is encompassed by tile 1 
       Second check left edge of tile2 :
	tile1_left <= tile2_left < tile1_right + xspaceG
    */
    } else if( tile1_left<=tile2_left&&tile2_left<tile1_right+xspaceS ){
	return( OVERLAP2 ) ;
    /* -----------------------------------------------------
       Third check right edge of tile2 :
	tile1_left - xspaceS < tile2_right < tile1_right 
    */
    } else if( tile1_left-xspaceS<tile2_right&&tile2_right<=tile1_right){
	return( OVERLAP3 ) ;

    /* -----------------------------------------------------
       Fourth case tiles touching.
    */
    } else if( tile2_left == tile1_right + xspaceS || 
	       tile1_left - xspaceS == tile2_right ){
	return( TOUCH ) ;

    } else {
	return( NOTOUCH ) ;
    }
} /* end projectX */

/* function returns whether one cell projects onto another */
INT projectY( tile1_bot, tile1_top, tile2_bot, tile2_top )
INT tile1_bot, tile1_top, tile2_bot, tile2_top ;
{
    /* -----------------------------------------------------
       First check to see if 2nd tile larger than first 
    */
    if( tile2_bot <= tile1_bot && tile1_top <= tile2_top ){
	return( OVERLAP1 ) ;

    /* -----------------------------------------------------
       Check if an edge of tile two is encompassed by tile 1 
       Second check bottom edge of tile2 :
	tile1_bot <= tile2_bot < tile1_top 
    */
    } else if( tile1_bot <= tile2_bot && tile2_bot < tile1_top+yspaceS){

	return( OVERLAP2 ) ;
    /* -----------------------------------------------------
       Third check top edge of tile2 :
	tile1_bot < tile2_top <= tile1_top 
    */
    } else if( tile1_bot-yspaceS < tile2_top && tile2_top <= tile1_top ){
	return( OVERLAP3 ) ;

    /* -----------------------------------------------------
       Fourth case tiles touching.
    */
    } else if( tile2_bot == tile1_top + yspaceS || 
	       tile1_bot - yspaceS == tile2_top ){
	return( TOUCH ) ;

    } else {
	return( NOTOUCH ) ; /* no touch or overlap */ 
    }
}/* end projectY */

#ifdef NEEDED
static INT count_rows()
{
    INT count ;           /* row counter */
    ROW_BOX *rowptr ;     /* current row */

    for( count=0,rowptr=set_of_rowsS;rowptr;rowptr=rowptr->next_row ){
	count++ ;
    }
    return( count ) ;
} /* end count_rows */
#endif /* NEEDED */

calculate_numrows()
{
    INT l, r, b, t ;  /* dimensions of new core */

    if( num_macrosG <= 0 ){
	/* first count rows and set bounds */
	number_of_rows_addedS = num_rowsG ;
	snap_core( &l, &r, &b, &t ) ;
	set_core( l, r, b, t ) ;
	start_tileG = NULL ;
	/* now we need to redo all the work we have done before */ 
	recalculate(TRUE) ; /* free data */
	remakerows() ;
    } else {
	/* binary search to find correct row separation */
#ifdef LATER
	while( low <= high ){
	    mid = low + (high-low) / 2 ;
	    switch (c = SIGN(strcmp(mid->rw_name, yytext))) {
	    case -1:
	      low = mid + 1;
	      break;
	    case 0:
	      return(mid->rw_yylex);
	      break;
	    case 1:
	      high = mid - 1;
	      break;
	    }  /* switch ... */
	}
#endif

    }
} /* calculate_numrows */

static INT compare_tiles( tile1, tile2 )
TILE_BOX *tile1, *tile2 ;
{
    /* sort by x then y */
    if( tile1->llx < tile2->llx ){
	return( -1 ) ;
    } else if( tile1->llx > tile2->lly ){
	return( 1 ) ;
    } else {
	if( tile1->lly < tile2->lly ){
	    return( -1 ) ;
	} else {
	    return( 1 ) ;
	}
    }
} /* end compare_tiles */


static reset_tile_parameters()
{

    TILE_BOX lo ;     /* low sentinel */
    TILE_BOX hi ;     /* hi sentinel */
    TILE_BOX *ntile ; /* the new tile */
    TILE_BOX *otile ; /* the old tile */
    TILE_BOX *match_tile ; /* the old tile with the most area in common*/
    INT overlapx ;    /* type of x overlap */
    INT overlapy ;    /* type of y overlap */
    FLOAT area ;        /* area of overlap between new and old tiles */
    FLOAT deltay ;       /* area of overlap in y direction */
    FLOAT largest_area ; /* old tile with largest amount of overlap */

    for( ntile=tile_listG;ntile;ntile = ntile->next ){
	lo.llx   = INT_MIN ;
	lo.lly   = INT_MIN ;
	hi.llx   = ntile->urx ;
	hi.lly   = INT_MIN ;
	largest_area = 0.0 ;
	area = 0.0 ;
	match_tile = NULL ;
	for(otile=(TILE_BOX *)Yrbtree_interval(tile_memoryG,&lo,&hi,TRUE);
	    otile ;
	    otile=(TILE_BOX *)Yrbtree_interval(tile_memoryG,&lo,&hi,FALSE)){
	    if(!(otile->allocated)){
		/* it has already been deleted */
		continue ;
	    }
	    if( overlapy = projectY( ntile->lly,ntile->ury,otile->lly-1,otile->ury+1)){
		if( overlapy == TOUCH ){
		    continue ;
		}
		if( overlapx = projectX( ntile->llx,ntile->urx,otile->llx-1,otile->urx+1 )){
		    if( overlapx == TOUCH ){
			continue ;
		    }
		    switch( overlapy ){
		    case OVERLAP1: /* complete overlap in y direction */
			deltay = (ntile->ury - ntile->lly ) ;
			switch( overlapx ){
			case OVERLAP1:
			    area = (ntile->urx - ntile->llx) * deltay ;
			    break ;
			case OVERLAP2: /* old tile to the right */
			    area = (ntile->urx - otile->llx) * deltay ;
			    break ;
			case OVERLAP3: /* old tile to the left */
			    area = (otile->urx - ntile->llx) * deltay ;
			    break ;
			} /* end switch( overlapx ... */
			break ;

		    case OVERLAP2: /* old tile above new tile */
			deltay = (ntile->ury - otile->lly ) ;
			switch( overlapx ){
			case OVERLAP1:
			    area = (ntile->urx - ntile->llx) * deltay ;
			    break ;
			case OVERLAP2: /* old tile to the right */
			    area = (ntile->urx - otile->llx) * deltay ;
			    break ;
			case OVERLAP3: /* old tile to the left */
			    area = (otile->urx - ntile->llx) * deltay ;
			    break ;
			} /* end switch( overlapx ... */
			break ;
			
		    case OVERLAP3:
			deltay = (otile->ury - ntile->lly ) ;
			switch( overlapx ){
			case OVERLAP1:
			    area = (ntile->urx - ntile->llx) * deltay ;
			    break ;
			case OVERLAP2: /* old tile to the right */
			    area = (ntile->urx - otile->llx) * deltay ;
			    break ;
			case OVERLAP3: /* old tile to the left */
			    area = (otile->urx - ntile->llx) * deltay ;
			    break ;
			} /* end switch( overlapx ... */
			break ;
		    } /* end switch( overlapy... */
		} /* end if( overlapx = projectX... */
	    } /* end if( overlapy = projectY... */

	    if( area > largest_area ){
		largest_area = area ;
		match_tile = otile ;
	    }
	} /* end of Yrbtree_interval search */

	if( match_tile ){
	    /* then it is safe to inherit properties */
	    ntile->numrows = match_tile->numrows ;
	    ntile->class = match_tile->class ;
	    ntile->actual_row_height  = match_tile-> actual_row_height ;
	    ntile->add_no_more_than = match_tile->add_no_more_than ;
	    ntile->channel_separation  = match_tile->channel_separation ;
	    ntile->min_length  = match_tile->min_length ;
	    ntile->row_start  = match_tile->row_start ;
	    ntile->illegal = match_tile->illegal ;
	    ntile->force  = match_tile->force ;
	    ntile->mirror  = match_tile->mirror ;
	    /* the tricky one max length */
	    if( match_tile->max_length >
		ntile->urx - ntile->llx - ntile->row_start - spacingG ){
		/* we need to reset the maximum length */
		ntile->max_length = 
		    ntile->urx - ntile->llx - ntile->row_start - 2*spacingG ;
	    } else {
		ntile->max_length  = match_tile->max_length ;
	    }
	}

    } /* end loop on ntiles */
} /* end reset_tile_parameters */
