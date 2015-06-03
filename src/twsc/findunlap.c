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
FILE:	    findunlap.c                                       
DESCRIPTION:remove overlap from cells.
CONTENTS:   findunlap(flag)
		INT flag ;
	    even_the_rows(flag)
		INT flag ;
	    even_the_rows_2( iteration )
		INT iteration ;
	    check_row_length()
	    find_longest_row() ;
	    find_shortest_row() ;
	    find_last_6_moveable_cells( cell1, cell2, cell3,
					cell4, cell5, cell6 )
		INT *cell1, *cell2, *cell3, *cell4, *cell5, *cell6 ;
	    row_cost( long_row , short_row , width )
		INT long_row , short_row , width ;
DATE:	    Mar 27, 1989 
REVISIONS:  Thu Dec 20 00:23:46 EST 1990 - removed += operator.
	    Wed Sep 11 11:19:25 CDT 1991 - modified
		for new global router.
----------------------------------------------------------------- */
#ifndef VMS
#ifndef lint
static char SccsId[] = "@(#) findunlap.c (Yale) version 4.14 4/2/92" ;
#endif
#endif


#include "standard.h"
#include "main.h"
#include "groute.h"
#include "feeds.h"
#include <yalecad/debug.h>

#define PICK_INT(l,u) (((l)<(u)) ? ((RAND % ((u)-(l)+1))+(l)) : (l))

/* globals */
INT largest_delta_row_lenG ;
extern INT **pairArrayG ;
extern INT exact_feedsG ;
extern INT extra_cellsG ;
extern INT actual_feed_thru_cells_addedG ;
extern BOOL gate_arrayG ;
extern BOOL no_row_lengthsG ;
extern DOUBLE mean_widthG ;
extern INT *feeds_in_rowG ;

static INT *row_lengthS ;
static BOOL first_passS = TRUE ;

findunlap(flag)
INT flag ;
{

CBOXPTR cellptr , ptr ;
TIBOXPTR tileptr ;
DBOXPTR dimptr ;
PINBOXPTR termptr , netptr ;
INT cost , corient , last_cell , total_penalty ;
INT block , cell , bin ;
INT i , n , xwire, ywire ;
INT bigblkx ;
INT net , x , y , *blklen , k ;
INT pathcount ;
INT delta_row_len ;
INT length , tmp ;
PATHPTR path ;
GLISTPTR net_of_path ;

blklen = (INT *) Ysafe_malloc( (numRowsG + 1) * sizeof( INT ) ) ;
for( i = 0 ; i <= numRowsG ; i++ ) {
    blklen[i]  = 0 ;
}

if( pairArrayG != NULL ) {
    for( block = 1 ; block <= numRowsG ; block++ ) {
	Ysafe_free( pairArrayG[block] ) ;
    }
    Ysafe_free( pairArrayG ) ;
    pairArrayG = NULL ;
}
if( flag == 2 ) {
    unlap(2) ;
} else if( flag == 0 ) {
    unlap(0) ;
} else {
    unlap(1) ;
}

if( binptrG != NULL ) {
    for( block = 1 ; block <= numRowsG ; block++ ) {
	for( bin = 0 ; bin <= numBinsG ; bin++ ) {
	    Ysafe_free( binptrG[ block ][ bin ]->cell ) ;
	    Ysafe_free( binptrG[ block ][ bin ] ) ;
	}
	Ysafe_free( binptrG[ block ] ) ;
    }
    Ysafe_free( binptrG ) ;
    binptrG = NULL ;
}

for( cell = 1 ; cell <= numcellsG ; cell++ ) {
    ptr = carrayG[ cell ] ;
    corient = ptr->corient ;
    for( termptr = ptr->pins ; termptr; termptr = termptr->nextpin ){
        x = termptr->txpos[ corient/2 ] + ptr->cxcenter;
	if( flag == 2 && x != termptr->xpos ) {
	    if( cell <= numcellsG - extra_cellsG ) {
		printf("actual cell %d x error\n", cell );
	    } else {
		printf("extra cell %d y error\n", cell );
	    }
	}
        termptr->xpos = termptr->txpos[ corient/2 ] + ptr->cxcenter;
        y = termptr->typos[ corient%2 ] + ptr->cycenter;
	if( flag == 2 && y != termptr->ypos ) {
	    if( cell <= numcellsG - extra_cellsG ) {
		printf("actual cell %d y error\n", cell );
	    } else {
		printf("extra cell %d y error\n", cell );
	    }
	}
        termptr->ypos = termptr->typos[ corient%2 ] + ptr->cycenter;
        termptr->newx = 0 ;
        termptr->newy = 0 ;
	termptr->row  = ptr->cblock ; 
    }
}
for( cell = numcellsG + 1 ; cell <= lastpadG ; cell++ ) {
    ptr = carrayG[ cell ] ;
    for( termptr = ptr->pins ; termptr; termptr = termptr->nextpin ){
        x = termptr->txpos[1] + ptr->cxcenter;
	if( flag == 2 && x != termptr->xpos ) {
	    printf("pad x error\n");
	}
        termptr->xpos = termptr->txpos[1] + ptr->cxcenter;
        y = termptr->typos[1] + ptr->cycenter;
	if( flag == 2 && y != termptr->ypos ) {
	    printf("pad y error\n");
	}
        termptr->ypos = termptr->typos[1] + ptr->cycenter;
        termptr->newx = 0 ;
        termptr->newy = 0 ;
    }
}

k = numcellsG + numtermsG + actual_feed_thru_cells_addedG ;
for( cell = lastpadG + 1 ; cell <= k ; cell++ ) {
    ptr = carrayG[ cell ] ;
    corient = ptr->corient ;
    for( termptr = ptr->pins ; termptr; termptr = termptr->nextpin ){
        x = termptr->txpos[ corient/2 ] + ptr->cxcenter;
	if( flag == 2 && x != termptr->xpos ) {
	    printf("feed %d x error\n", cell );
	}
        termptr->xpos = termptr->txpos[ corient/2 ] + ptr->cxcenter;
        y = termptr->typos[ corient%2 ] + ptr->cycenter;
	if( flag == 2 && y != termptr->ypos ) {
	    printf("feed %d y error\n", cell );
	}
        termptr->ypos = termptr->typos[ corient%2 ] + ptr->cycenter;
        termptr->newx = 0 ;
        termptr->newy = 0 ;
	termptr->row  = ptr->cblock ; 
    }
}

cost  = 0 ;
xwire = 0 ;
ywire = 0 ;
for( net = 1 ; net <= numnetsG ; net++ ) {
    dimptr =  netarrayG[ net ] ;
    if(!(netptr = dimptr->pins)) {
	continue ;
    }
    dimptr->xmin = dimptr->xmax = netptr->xpos ;
    dimptr->ymin = dimptr->ymax = netptr->ypos ;
    dimptr->Lnum = dimptr->Rnum = 1 ;
    dimptr->Bnum = dimptr->Tnum = 1 ;
    netptr = netptr->next ;
    
    n = 1 ;
    for( ; netptr ; netptr = netptr->next ) {
	x = netptr->xpos ;
	y = netptr->ypos ;
	if( x < dimptr->xmin ) {
	    dimptr->xmin = x ;
	    dimptr->Lnum = 1 ;
	} else if( x == dimptr->xmin ) {
	    dimptr->Lnum++ ;
	    if( x == dimptr->xmax ) {
		dimptr->Rnum++ ;
	    }
	} else if( x > dimptr->xmax ) {
	    dimptr->xmax = x ;
	    dimptr->Rnum = 1 ;
	} else if( x == dimptr->xmax ) {
	    dimptr->Rnum++ ;
	}
	if( y < dimptr->ymin ) {
	    dimptr->ymin = y ;
	    dimptr->Bnum = 1  ;
	} else if( y == dimptr->ymin ) { 
	    dimptr->Bnum++ ;
	    if( y == dimptr->ymax ) {
		dimptr->Tnum++ ;
	    }
	} else if( y > dimptr->ymax ) {
	    dimptr->ymax = y ;
	    dimptr->Tnum = 1  ;
	} else if( y == dimptr->ymax ) {
	    dimptr->Tnum++ ;
	}
	n++ ;
    }
    dimptr->numpins = n ;
    xwire += dimptr->xmax - dimptr->xmin ;
    ywire += dimptr->ymax - dimptr->ymin ;

    cost += dimptr->halfPx = dimptr->newhalfPx = 
					 dimptr->xmax - dimptr->xmin ;
    dimptr->halfPy = dimptr->newhalfPy = dimptr->ymax - dimptr->ymin ;
    cost = cost + (INT)( vertical_wire_weightG * (DOUBLE) dimptr->halfPy ) ;
}
funccostG = cost ;

/* ************* now calculate the timing penalty ************** */
timingcostG = 0 ;
for( pathcount = 1 ; pathcount <= numpathsG ; pathcount++ ) {

    path = patharrayG[pathcount] ;
    length = 0 ;
    ASSERTNCONT( path, "findcost", "pointer to path is NULL" ) ;
    /* for all nets k of a path i */
    /* -----------------------------------------------------------------
        For all nets k of a path i:
	    We use the minimum strength driver for each net to calculate
	    the lower bound on the length and the maximum strength driver
	    for the upper bound on the length.  The user must take false
	    paths into account when specifying the driver strengths.
    ------------------------------------------------------------------ */
    for( net_of_path=path->nets;net_of_path;
	net_of_path=net_of_path->next ){
	net = net_of_path->p.net ;
	dimptr = netarrayG[net] ;
	/* accumulate length of path */
	length = length + (INT)
	    (horizontal_path_weightG * (DOUBLE) dimptr->halfPx) ;
	length = length + (INT)
	    (vertical_path_weightG * (DOUBLE) dimptr->halfPy) ;

    }
    /* save result */
    path->path_len = path->new_path_len = length ;

    /* calculate penalty */
    /* no penalty if within target window */
    /* lowerbound <= length <= upperbound */
    if( length > path->upper_bound ){
	timingcostG += length - path->upper_bound ; 
    } else if( length < path->lower_bound ){
	timingcostG += path->lower_bound - length ;
    }
}

fprintf(fpoG,"Total Wire Length - X-Component:%d\n", xwire ) ;
fprintf(fpoG,"Total Wire Length - Y-Component:%d\n", ywire ) ;
fprintf(fpoG,"Time Penalty:%d\n", timingcostG ) ;
fflush(fpoG);


for( block = 1 ; block <= numRowsG ; block++ ) {
    last_cell = pairArrayG[block][0] ;
    for( i = 1 ; i <= last_cell ; i++ ) {
	cellptr = carrayG[ pairArrayG[block][i] ] ;
	tileptr = cellptr->tileptr ;
	blklen[block] += tileptr->right - tileptr->left  ;
    }
}


if( exact_feedsG ) {
    for( i = 1 ; i <= numRowsG ; i++ ) {
	blklen[i] += fdcel_addedG[i] * fdWidthG ;
    }
    exact_feedsG = 0 ;
}

bigblkx = 0 ;
largest_delta_row_lenG = -1000000 ;
if( !no_row_lengthsG ) {
    if( first_passS ) {
	for( i = 1 ; i <= numRowsG ; i++ ) {
	    tmp = feeds_in_rowG[i] * fdWidthG ;
	    if( tmp > barrayG[i]->orig_desire ) {
		blklen[i] += barrayG[i]->orig_desire ;
	    } else {
		blklen[i] += tmp ;
	    }
	    total_penalty = 0 ;
	}
    }
    fprintf(fpoG,"BLOCK    TOTAL CELL LENGTHS    OVER/UNDER TARGET      DESIRE\n");
    for( i = 1 ; i <= numRowsG ; i++ ) {
	if( blklen[i] > blklen[ bigblkx ] ) {
	    bigblkx = i ;
	}
	delta_row_len = blklen[i] - barrayG[i]->desire ;
	if( delta_row_len > largest_delta_row_lenG ) {
	    largest_delta_row_lenG = delta_row_len ;
	}
	if( first_passS ) {
	    total_penalty += ABS( delta_row_len ) ;
	}
	fprintf( fpoG, "%5d    %18d    %17d   %9d\n", i,
			blklen[i] ,  delta_row_len , barrayG[i]->desire );
    }

    fprintf( fpoG, "\nLONGEST Block is:%d   Its length is:%d\n",
				bigblkx , blklen[ bigblkx ] ) ;
    if( first_passS ) {
	fprintf( fpoG,"total penalty is %d\n", total_penalty ) ;
    }
    first_passS = FALSE ;
}
Ysafe_free( blklen ) ;

return ;
}





even_the_rows(flag,even_the_rows_max)
INT flag ;
BOOL even_the_rows_max ;
{

INT cell1 , cell2 , cell3 , cell4 , cell5 , cell6 ;
INT long_row , short_row , row , bound , j , i , desire ;
INT select , cost , num_trys, index, shift ;
unsigned bit_class ;


row_lengthS = (INT *) Ysafe_calloc(  (1+numRowsG), sizeof(INT) ) ;

for( row = 1 ; row <= numRowsG ; row++ ) {
    bound = 0 ;
    j = pairArrayG[row][0] ;
    for( i = 1 ; i <= j ; i++ ) {
	bound += carrayG[ pairArrayG[row][i] ]->clength ;
    }
    if( flag ) {
	bound += fdcel_addedG[row] * fdWidthG ;
    }
    desire = barrayG[row]->desire ;
    row_lengthS[row] = bound - desire ;
}

num_trys = 0 ;
while( 1 ) {

    if( even_the_rows_max ){
	determine_unequal_rows( &short_row, &long_row ) ;
    } else {
	determine_unequal_rows( &short_row, &long_row ) ;
	short_row = find_shortest_row( long_row ) ;
    }
    if( pairArrayG[long_row][0] == 1 ) {
	continue ;
    }

    /* long_row = find_longest_row() ; */
    /*
    if( long_row == 0 ) {
	break ;
    }
    */
    find_last_6_moveable_cells( long_row, &cell1 , &cell2 , &cell3 ,
					  &cell4 , &cell5 , &cell6 ) ;
    select = 0 ;
    if( cell1 > 0 ) {
	cost = row_cost(long_row, short_row, carrayG[cell1]->clength);
	if( cost < 0 ) {
	    select = cell1 ;

	    if( carrayG[cell1]->cclass > 0 ) {
		bit_class = 1 ;
		index = (barrayG[short_row]->bclass - 1) / 32 ;
		shift = barrayG[short_row]->bclass - 32 * index ;
		bit_class <<= (shift - 1) ;
		if( bit_class & carrayG[cell1]->cbclass[index] ) {
		    /* a move to this row is allowed */
		} else {
		    select = 0 ;
		}
	    }
	}
    }
    if( select == 0 && cell2 > 0 ) {
	cost = row_cost(long_row , short_row , carrayG[cell2]->clength) ;
	if( cost < 0 ) {
	    select = cell2 ;

	    if( carrayG[cell2]->cclass > 0 ) {
		bit_class = 1 ;
		index = (barrayG[short_row]->bclass - 1) / 32 ;
		shift = barrayG[short_row]->bclass - 32 * index ;
		bit_class <<= (shift - 1) ;
		if( bit_class & carrayG[cell2]->cbclass[index] ) {
		    /* a move to this row is allowed */
		} else {
		    select = 0 ;
		}
	    }
	}
    }
    if( select == 0 && cell3 > 0 ) {
	cost = row_cost(long_row , short_row , carrayG[cell3]->clength) ;
	if( cost < 0 ) {
	    select = cell3 ;

	    if( carrayG[cell3]->cclass > 0 ) {
		bit_class = 1 ;
		index = (barrayG[short_row]->bclass - 1) / 32 ;
		shift = barrayG[short_row]->bclass - 32 * index ;
		bit_class <<= (shift - 1) ;
		if( bit_class & carrayG[cell3]->cbclass[index] ) {
		    /* a move to this row is allowed */
		} else {
		    select = 0 ;
		}
	    }
	}
    }
    if( select == 0 && cell4 > 0 ) {
	cost = row_cost(long_row , short_row , carrayG[cell4]->clength) ;
	if( cost < 0 ) {
	    select = cell4 ;

	    if( carrayG[cell4]->cclass > 0 ) {
		bit_class = 1 ;
		index = (barrayG[short_row]->bclass - 1) / 32 ;
		shift = barrayG[short_row]->bclass - 32 * index ;
		bit_class <<= (shift - 1) ;
		if( bit_class & carrayG[cell4]->cbclass[index] ) {
		    /* a move to this row is allowed */
		} else {
		    select = 0 ;
		}
	    }
	}
    }
    if( select == 0 && cell5 > 0 ) {
	cost = row_cost(long_row , short_row , carrayG[cell5]->clength) ;
	if( cost < 0 ) {
	    select = cell5 ;

	    if( carrayG[cell5]->cclass > 0 ) {
		bit_class = 1 ;
		index = (barrayG[short_row]->bclass - 1) / 32 ;
		shift = barrayG[short_row]->bclass - 32 * index ;
		bit_class <<= (shift - 1) ;
		if( bit_class & carrayG[cell5]->cbclass[index] ) {
		    /* a move to this row is allowed */
		} else {
		    select = 0 ;
		}
	    }
	}
    }
    if( select == 0 && cell6 > 0 ) {
	cost = row_cost(long_row , short_row , carrayG[cell6]->clength) ;
	if( cost < 0 ) {
	    select = cell6 ;

	    if( carrayG[cell6]->cclass > 0 ) {
		bit_class = 1 ;
		index = (barrayG[short_row]->bclass - 1) / 32 ;
		shift = barrayG[short_row]->bclass - 32 * index ;
		bit_class <<= (shift - 1) ;
		if( bit_class & carrayG[cell6]->cbclass[index] ) {
		    /* a move to this row is allowed */
		} else {
		    select = 0 ;
		}
	    }
	}
    }
    if( select == 0 ) {
	if( ++num_trys > 2 * numRowsG ) {
	    break ;
	}
	continue ;
    }
    num_trys = 0 ;
    /*
    fprintf(fpoG,"cell:%d  moved from row:%d  to  row:%d\n",
	select, long_row , short_row ) ;
    fflush(fpoG) ;
    */
    row_lengthS[ long_row ] -= carrayG[select]->clength ;
    row_lengthS[ short_row ] += carrayG[select]->clength ;
    pairArrayG[ short_row ][ ++pairArrayG[short_row][0] ] = select ;
    for( i = 1 ; i <= pairArrayG[long_row][0] ; i++ ) {
        if( pairArrayG[long_row][i] == select ) {
	    for( i++ ; i <= pairArrayG[long_row][0] ; i++ ) {
		pairArrayG[long_row][i-1] =  pairArrayG[long_row][i] ;
	    }
	    pairArrayG[long_row][0]-- ;
	    break ;
	}
    }
    carrayG[select]->cblock = short_row ;
    carrayG[select]->cycenter = barrayG[short_row]->bycenter ;
    if( barrayG[short_row]->borient != barrayG[long_row]->borient ) {
	if( carrayG[select]->corient == 0 ||
				    carrayG[select]->corient == 2 ) {
	    if( carrayG[select]->corient == 0 ) {
		carrayG[select]->corient = 1 ;
	    } else {
		carrayG[select]->corient = 3 ;
	    }
	} else {  /* its 1 or 3  */
	    if( carrayG[select]->corient == 1 ) {
		carrayG[select]->corient = 0 ;
	    } else {
		carrayG[select]->corient = 2 ;
	    }
	}
    }
}

Ysafe_free( row_lengthS ) ;
return ;
}





gate_arrayG_even_the_rows(flag)
INT flag ;
{

CBOXPTR cellptr , cellptr1 , cellptr2 , cellptr3 , cellptr4 ; 
INT shortest , *set , i , row , tmp , w1 , w2 , w3 , w4 , error ;
INT width1, width2, width3, width4, desire, mark, bound, new_bound ;
INT gap , j , k , l , m, shift, index ;
unsigned bit_class ;

if( numRowsG <= 6 ) {
    return ;
}
set = (INT *) Ysafe_malloc( (1 + numcellsG) * sizeof( INT ) ) ;
set[0] = 0 ;

shortest = 0 ;
for( row = 1 ; row <= numRowsG ; row++ ) {
    bound = 0 ;
    j = pairArrayG[row][0] ;
    for( i = 1 ; i <= j ; i++ ) {
	bound += carrayG[ pairArrayG[row][i] ]->clength ;
    }
    if( flag ) {
	bound += fdcel_addedG[row] * fdWidthG ;
    }
    desire = barrayG[row]->desire ;
    if( desire - bound > shortest ) {
	shortest = desire - bound ;
    }
}
for( row = 1 ; row <= numRowsG ; row++ ) {
    bound = 0 ;
    j = pairArrayG[row][0] ;
    for( i = 1 ; i <= j ; i++ ) {
	bound += carrayG[ pairArrayG[row][i] ]->clength ;
    }
    if( flag ) {
	bound += fdcel_addedG[row] * fdWidthG ;
    }
    desire = barrayG[row]->desire ;
    gap = desire - bound ;

    while( gap < shortest ) {
	i = pairArrayG[row][0] + 1 ;
	do { 
	    if( --i > 0 ) {
		cellptr1 = carrayG[ pairArrayG[row][i] ] ;
	    }
	} while( i > 0 && cellptr1->cclass < 0 ) ;
	if( i == 0 ) {
	    break ;
	}
	j = i ;
	do { 
	    if( --j > 0 ) {
		cellptr2 = carrayG[ pairArrayG[row][j] ] ;
	    }
	} while( j > 0 && cellptr2->cclass < 0 ) ;
	if( j == 0 ) {
	    break ;
	}
	k = j ;
	do { 
	    if( --k > 0 ) {
		cellptr3 = carrayG[ pairArrayG[row][k] ] ;
	    }
	} while( k > 0 && cellptr3->cclass < 0 ) ;
	if( k == 0 ) {
	    break ;
	}
	l = k ;
	do { 
	    if( --l > 0 ) {
		cellptr4 = carrayG[ pairArrayG[row][l] ] ;
	    }
	} while( l > 0 && cellptr4->cclass < 0 ) ;
	if( l == 0 ) {
	    break ;
	}
	w1 = cellptr1->clength ;
	w2 = cellptr2->clength ;
	w3 = cellptr3->clength ;
	w4 = cellptr4->clength ;
	if( w1 <= w2 && w1 <= w3 && w1 <= w4 ) {
	    gap += w1 ;
	    set[ ++set[0] ] = pairArrayG[row][i] ;
	    for( m = i + 1 ; m <= pairArrayG[row][0] ; m++ ) {
		pairArrayG[row][m-1] = pairArrayG[row][m] ;
	    }
	    pairArrayG[row][0]-- ;
	} else if( w2 <= w1 && w2 <= w3 && w2 <= w4 ) {
	    gap += w2 ;
	    set[ ++set[0] ] = pairArrayG[row][j] ;
	    for( m = j + 1 ; m <= pairArrayG[row][0] ; m++ ) {
		pairArrayG[row][m-1] = pairArrayG[row][m] ;
	    }
	    pairArrayG[row][0]-- ;
	} else if( w3 <= w1 && w3 <= w2 && w3 <= w4 ) {
	    gap += w3 ;
	    set[ ++set[0] ] = pairArrayG[row][k] ;
	    for( m = k + 1 ; m <= pairArrayG[row][0] ; m++ ) {
		pairArrayG[row][m-1] = pairArrayG[row][m] ;
	    }
	    pairArrayG[row][0]-- ;
	} else {
	    gap += w4 ;
	    set[ ++set[0] ] = pairArrayG[row][l] ;
	    for( m = l + 1 ; m <= pairArrayG[row][0] ; m++ ) {
		pairArrayG[row][m-1] = pairArrayG[row][m] ;
	    }
	    pairArrayG[row][0]-- ;
	}
    }
}
/*
for( i = 1 ; i <= set[0] ; i++ ) {
    printf("cell: %d    length: %d\n", set[i], carrayG[set[i]]->clength);
}
*/

mark = 0 ;
error = 0 ;
for( row = 1 ; row <= numRowsG ; row++ ) {
    bound = 0 ;
    for( i = 1 ; i <= pairArrayG[row][0] ; i++ ) {
	bound += carrayG[ pairArrayG[row][i] ]->clength ;
    }
    if( flag ) {
	bound += fdcel_addedG[row] * fdWidthG ;
    }
    desire = barrayG[row]->desire ;
/*
    printf("row: %d    bound: %d   desire: %d\n", row, bound, desire);
*/

    while( 1 ) {
	if( mark+1 <= set[0] ) {
	    width1 = carrayG[ set[mark+1] ]->clength ;
	} else {
	    break ;
	}
	if( mark+2 <= set[0] ) {
	    width2 = carrayG[ set[mark+2] ]->clength ;
	} else {
	    width2 = 10000000 ;
	}
	if( mark+3 <= set[0] ) {
	    width3 = carrayG[ set[mark+3] ]->clength ;
	} else {
	    width3 = 10000000 ;
	}
	if( mark+4 <= set[0] ) {
	    width4 = carrayG[ set[mark+4] ]->clength ;
	} else {
	    width4 = 10000000 ;
	}
	w1 = ABS( bound + width1 + error - desire ) ;
	w2 = ABS( bound + width2 + error - desire ) ;
	w3 = ABS( bound + width3 + error - desire ) ;
	w4 = ABS( bound + width4 + error - desire ) ;
	if( w1 <= w2 && w1 <= w3 && w1 <= w4 ){
	    new_bound = bound + width1 ;
	} else if( w2 <= w1 && w2 <= w3 && w2 <= w4 ) {
	    tmp = set[mark+1] ;
	    set[mark+1] = set[mark+2] ;
	    set[mark+2] = tmp ;
	    new_bound = bound + width2 ;
	} else if( w3 <= w1 && w3 <= w2 && w3 <= w4 ) {
	    tmp = set[mark+1] ;
	    set[mark+1] = set[mark+3] ;
	    set[mark+3] = tmp ;
	    new_bound = bound + width3 ;
	} else {
	    tmp = set[mark+1] ;
	    set[mark+1] = set[mark+4] ;
	    set[mark+4] = tmp ;
	    new_bound = bound + width4 ;
	}
	if( ABS(new_bound + error - desire) <= 
					ABS(bound + error - desire) ) {
	    bit_class = 1 ;
	    index = (barrayG[row]->bclass - 1) / 32 ;
	    shift = barrayG[row]->bclass - 32 * index ;
	    bit_class <<= (shift - 1) ;
	    if( row == carrayG[set[mark+1]]->cblock ||
			carrayG[set[mark+1]]->cclass == 0 ||
			bit_class & carrayG[set[mark+1]]->cbclass[index]) {
		/* this cell can go into "row" */
		bound = new_bound ;
		pairArrayG[row][ ++pairArrayG[row][0] ] = set[ ++mark ] ;
		cellptr = carrayG[ pairArrayG[row][ pairArrayG[row][0] ] ] ;

		if( barrayG[row]->borient != 
				barrayG[cellptr->cblock]->borient ) {
		    if( cellptr->corient == 0 ||
					cellptr->corient == 2 ) {
			if( cellptr->corient == 0 ) {
			    cellptr->corient = 1 ;
			} else {
			    cellptr->corient = 3 ;
			}
		    } else {  /* its 1 or 3  */
			if( cellptr->corient == 1 ) {
			    cellptr->corient = 0 ;
			} else {
			    cellptr->corient = 2 ;
			}
		    }
		}
		cellptr->cblock = row ;
		cellptr->cycenter = barrayG[row]->bycenter ;
	    } else {
		mark++ ;
	    }
	} else {
	    error += bound - desire ;
	    break ;
	}
    }
}

Ysafe_free( set ) ;
return ;
}




row_cost( long_row , short_row , width )
INT long_row , short_row , width ;
{

INT cost ;

cost = row_lengthS[short_row] + width - row_lengthS[long_row] ;
return( cost ) ;
}




find_last_6_moveable_cells( row, cell1, cell2, cell3, cell4, cell5, cell6) 
INT row ;
INT *cell1 , *cell2 , *cell3 , *cell4 , *cell5 , *cell6 ;
{

INT i ;
CBOXPTR cellptr ;

i = pairArrayG[row][0] + 1 ;
do {
    if( --i > 0 ) {
	cellptr = carrayG[ pairArrayG[row][i] ] ;
    }
} while( i > 0 && (cellptr->paths != NULL || cellptr->cclass < 0 )) ;
if( i <= 0 ) {
    *cell1 = 0 ;
    *cell2 = 0 ;
    *cell3 = 0 ;
    *cell4 = 0 ;
    *cell5 = 0 ;
    *cell6 = 0 ;
    return ;
} else {
    *cell1 = pairArrayG[row][i] ;
}

do {
    if( --i > 0 ) {
	cellptr = carrayG[ pairArrayG[row][i] ] ;
    }
} while( i > 0 && (cellptr->paths != NULL || cellptr->cclass < 0 )) ;
if( i <= 0 ) {
    *cell2 = 0 ;
    *cell3 = 0 ;
    *cell4 = 0 ;
    *cell5 = 0 ;
    *cell6 = 0 ;
    return ;
} else {
    *cell2 = pairArrayG[row][i] ;
}

do {
    if( --i > 0 ) {
	cellptr = carrayG[ pairArrayG[row][i] ] ;
    }
} while( i > 0 && (cellptr->paths != NULL || cellptr->cclass < 0 )) ;
if( i <= 0 ) {
    *cell3 = 0 ;
    *cell4 = 0 ;
    *cell5 = 0 ;
    *cell6 = 0 ;
    return ;
} else {
    *cell3 = pairArrayG[row][i] ;
}

do {
    if( --i > 0 ) {
	cellptr = carrayG[ pairArrayG[row][i] ] ;
    }
} while( i > 0 && (cellptr->paths != NULL || cellptr->cclass < 0 )) ;
if( i <= 0 ) {
    *cell4 = 0 ;
    *cell5 = 0 ;
    *cell6 = 0 ;
    return ;
} else {
    *cell4 = pairArrayG[row][i] ;
}

do {
    if( --i > 0 ) {
	cellptr = carrayG[ pairArrayG[row][i] ] ;
    }
} while( i > 0 && (cellptr->paths != NULL || cellptr->cclass < 0 )) ;
if( i <= 0 ) {
    *cell5 = 0 ;
    *cell6 = 0 ;
    return ;
} else {
    *cell5 = pairArrayG[row][i] ;
}

do {
    if( --i > 0 ) {
	cellptr = carrayG[ pairArrayG[row][i] ] ;
    }
} while( i > 0 && (cellptr->paths != NULL || cellptr->cclass < 0 )) ;
if( i <= 0 ) {
    *cell6 = 0 ;
    return ;
} else {
    *cell6 = pairArrayG[row][i] ;
}

return ;
}




find_longest_row()
{
INT row ;
INT num_trys ;

num_trys = 0 ;
do {
    row = PICK_INT( 1 , numRowsG ) ;
} while( row_lengthS[row] <= 0 && ++num_trys <= 2*numRowsG ) ;

return( row ) ;
}



/*
find_shortest_row( long_row )
INT long_row ;
{
INT row ;

if( long_row + 1 <= numRowsG && long_row - 1 >= 1 ) {
    if( row_lengthS[long_row + 1] <= row_lengthS[long_row - 1] ) {
	row = long_row + 1 ;
    } else {
	row = long_row - 1 ;
    }
} else if( long_row + 1 <= numRowsG ) {
    row = long_row + 1 ;
} else {
    row = long_row - 1 ;
}

return(row) ;
}
*/


find_shortest_row( long_row )
INT long_row ;
{
INT row ;
INT short_row_length ;
INT slack , short_row , min_row , max_row ;

if( long_row > 3 ) {
    min_row = long_row - 3 ;
} else {
    min_row = 1 ;
}
if( long_row <= numRowsG - 3 ) {
    max_row = long_row + 3 ;
} else {
    max_row = numRowsG ;
}
short_row_length = INT_MAX ;
for( row = min_row ; row <= max_row ; row ++ ){
    slack = row_lengthS[row] ;
    if( slack < short_row_length ){
	short_row = row ;
	short_row_length = slack ;
    }
}
return(short_row) ;
}


determine_unequal_rows( short_row, long_row )
INT *short_row, *long_row ;
{
    INT row ;
    INT short_row_length, long_row_length ;
    INT slack ;

    short_row_length = INT_MAX ;
    long_row_length = INT_MIN ;
    for( row = 1 ; row <= numRowsG ; row ++ ){
	slack = row_lengthS[row] ;
	if( slack > long_row_length ){
	    *long_row = row ;
	    long_row_length = slack ;
	}
	if( slack < short_row_length ){
	    *short_row = row ;
	    short_row_length = slack ;
	}
    }
    
} /* end determine_unequal_rows() */



even_the_rows_2( iteration )
INT iteration ;
{

CBOXPTR cellptr , cellptr1 , cellptr2 , cellptr3 , cellptr4 ; 
INT shortest , *set , i , row , tmp , w1 , w2 , w3 , w4 , error ;
INT width1, width2, width3, width4, desire, mark, bound, new_bound ;
INT gap , j , k , l , m, shift, index ;
unsigned bit_class ;

if( numRowsG <= 1 || numcellsG <= 6 ) {
    return ;
}
set = (INT *) Ysafe_malloc( (1 + numcellsG) * sizeof( INT ) ) ;
set[0] = 0 ;

shortest = 0 ;
for( row = 1 ; row <= numRowsG ; row++ ) {
    bound = 0 ;
    j = pairArrayG[row][0] ;
    for( i = 1 ; i <= j ; i++ ) {
	bound += carrayG[ pairArrayG[row][i] ]->clength ;
    }
    desire = barrayG[row]->desire ;
    if( desire - bound > shortest ) {
	shortest = desire - bound ;
    }
}
shortest += iteration * (INT) mean_widthG ;

for( row = 1 ; row <= numRowsG ; row++ ) {
    bound = 0 ;
    j = pairArrayG[row][0] ;
    for( i = 1 ; i <= j ; i++ ) {
	bound += carrayG[ pairArrayG[row][i] ]->clength ;
    }
    desire = barrayG[row]->desire ;
    gap = desire - bound ;

    while( gap < shortest ) {
	i = pairArrayG[row][0] + 1 ;
	do { 
	    if( --i > 0 ) {
		cellptr1 = carrayG[ pairArrayG[row][i] ] ;
	    }
	} while( i > 0 && cellptr1->cclass < 0 ) ;
	if( i == 0 ) {
	    break ;
	}
	j = i ;
	do { 
	    if( --j > 0 ) {
		cellptr2 = carrayG[ pairArrayG[row][j] ] ;
	    }
	} while( j > 0 && cellptr2->cclass < 0 ) ;
	if( j == 0 ) {
	    break ;
	}
	k = j ;
	do { 
	    if( --k > 0 ) {
		cellptr3 = carrayG[ pairArrayG[row][k] ] ;
	    }
	} while( k > 0 && cellptr3->cclass < 0 ) ;
	if( k == 0 ) {
	    break ;
	}
	l = k ;
	do { 
	    if( --l > 0 ) {
		cellptr4 = carrayG[ pairArrayG[row][l] ] ;
	    }
	} while( l > 0 && cellptr4->cclass < 0 ) ;
	if( l == 0 ) {
	    break ;
	}
	w1 = cellptr1->clength ;
	w2 = cellptr2->clength ;
	w3 = cellptr3->clength ;
	w4 = cellptr4->clength ;
	if( w1 <= w2 && w1 <= w3 && w1 <= w4 ) {
	    gap += w1 ;
	    set[ ++set[0] ] = pairArrayG[row][i] ;
	    for( m = i + 1 ; m <= pairArrayG[row][0] ; m++ ) {
		pairArrayG[row][m-1] = pairArrayG[row][m] ;
	    }
	    pairArrayG[row][0]-- ;
	} else if( w2 <= w1 && w2 <= w3 && w2 <= w4 ) {
	    gap += w2 ;
	    set[ ++set[0] ] = pairArrayG[row][j] ;
	    for( m = j + 1 ; m <= pairArrayG[row][0] ; m++ ) {
		pairArrayG[row][m-1] = pairArrayG[row][m] ;
	    }
	    pairArrayG[row][0]-- ;
	} else if( w3 <= w1 && w3 <= w2 && w3 <= w4 ) {
	    gap += w3 ;
	    set[ ++set[0] ] = pairArrayG[row][k] ;
	    for( m = k + 1 ; m <= pairArrayG[row][0] ; m++ ) {
		pairArrayG[row][m-1] = pairArrayG[row][m] ;
	    }
	    pairArrayG[row][0]-- ;
	} else {
	    gap += w4 ;
	    set[ ++set[0] ] = pairArrayG[row][l] ;
	    for( m = l + 1 ; m <= pairArrayG[row][0] ; m++ ) {
		pairArrayG[row][m-1] = pairArrayG[row][m] ;
	    }
	    pairArrayG[row][0]-- ;
	}
    }
}
/*
for( row = 1 ; row <= numRowsG ; row++ ) {
    gap = 0 ;
    i = pairArrayG[row][0] + 1 ;
    while( gap < shortest ) {
	do { 
	    if( --i > 0 ) {
		cellptr1 = carrayG[ pairArrayG[row][i] ] ;
	    }
	} while( i > 0 && 
			strcmp("GATE_ARRAY_SPACER", cellptr1->cname) != 0 ) ;
	if( i == 0 ) {
	    break ;
	}
	w1 = cellptr1->clength ;
        gap += w1 ;
        set[ ++set[0] ] = pairArrayG[row][i] ;
        for( m = i + 1 ; m <= pairArrayG[row][0] ; m++ ) {
	    pairArrayG[row][m-1] = pairArrayG[row][m] ;
        }
        pairArrayG[row][0]-- ;
    }
}
*/

mark = 0 ;
error = 0 ;
for( row = 1 ; row <= numRowsG ; row++ ) {
    bound = 0 ;
    for( i = 1 ; i <= pairArrayG[row][0] ; i++ ) {
	bound += carrayG[ pairArrayG[row][i] ]->clength ;
    }
    desire = barrayG[row]->desire ;

    while( 1 ) {
	if( mark+1 <= set[0] ) {
	    width1 = carrayG[ set[mark+1] ]->clength ;
	} else {
	    break ;
	}
	if( mark+2 <= set[0] ) {
	    width2 = carrayG[ set[mark+2] ]->clength ;
	} else {
	    width2 = 10000000 ;
	}
	if( mark+3 <= set[0] ) {
	    width3 = carrayG[ set[mark+3] ]->clength ;
	} else {
	    width3 = 10000000 ;
	}
	if( mark+4 <= set[0] ) {
	    width4 = carrayG[ set[mark+4] ]->clength ;
	} else {
	    width4 = 10000000 ;
	}
	w1 = ABS( bound + width1 + error - desire ) ;
	w2 = ABS( bound + width2 + error - desire ) ;
	w3 = ABS( bound + width3 + error - desire ) ;
	w4 = ABS( bound + width4 + error - desire ) ;
	if( w1 <= w2 && w1 <= w3 && w1 <= w4 ){
	    new_bound = bound + width1 ;
	} else if( w2 <= w1 && w2 <= w3 && w2 <= w4 ) {
	    tmp = set[mark+1] ;
	    set[mark+1] = set[mark+2] ;
	    set[mark+2] = tmp ;
	    new_bound = bound + width2 ;
	} else if( w3 <= w1 && w3 <= w2 && w3 <= w4 ) {
	    tmp = set[mark+1] ;
	    set[mark+1] = set[mark+3] ;
	    set[mark+3] = tmp ;
	    new_bound = bound + width3 ;
	} else {
	    tmp = set[mark+1] ;
	    set[mark+1] = set[mark+4] ;
	    set[mark+4] = tmp ;
	    new_bound = bound + width4 ;
	}
	if( ABS(new_bound + error - desire) <= 
					ABS(bound + error - desire) ) {
	    bit_class = 1 ;
	    index = (barrayG[row]->bclass - 1) / 32 ;
	    shift = barrayG[row]->bclass - 32 * index ;
	    bit_class <<= (shift - 1) ;
	    if( row == carrayG[set[mark+1]]->cblock ||
			carrayG[set[mark+1]]->cclass == 0 ||
			bit_class & carrayG[set[mark+1]]->cbclass[index]) {
		/* this cell can go into "row" */
		bound = new_bound ;
		pairArrayG[row][ ++pairArrayG[row][0] ] = set[ ++mark ] ;
		cellptr = carrayG[ pairArrayG[row][ pairArrayG[row][0] ] ] ;
		if( barrayG[row]->borient != 
				barrayG[cellptr->cblock]->borient ) {
		    if( cellptr->corient == 0 ||
					cellptr->corient == 2 ) {
			if( cellptr->corient == 0 ) {
			    cellptr->corient = 1 ;
			} else {
			    cellptr->corient = 3 ;
			}
		    } else {  /* its 1 or 3  */
			if( cellptr->corient == 1 ) {
			    cellptr->corient = 0 ;
			} else {
			    cellptr->corient = 2 ;
			}
		    }
		}
		cellptr->cblock = row ;
		cellptr->cycenter = barrayG[row]->bycenter ;
	    } else {
		mark++ ;
	    }
	} else {
	    error += bound - desire ;
	    break ;
	}
    }
}

Ysafe_free( set ) ;
return ;
}






check_row_length()
{

INT longest , row , shortest ;
INT desire , bound , i , j , long_row ;

if( numRowsG <= 1 || numcellsG <= 6 ) {
    return(0) ;
}

longest = -1 ;
long_row = 1 ;
shortest = 10000000 ;
for( row = 1 ; row <= numRowsG ; row++ ) {
    bound = 0 ;
    j = pairArrayG[row][0] ;
    for( i = 1 ; i <= j ; i++ ) {
	bound += carrayG[ pairArrayG[row][i] ]->clength ;
    }
    desire = barrayG[row]->desire ;
    if( bound - desire > longest ) {
	longest = bound - desire ;
	long_row = row ;
    }
    if( bound - desire < shortest ) {
	shortest = bound - desire ;
    }
}
if( gate_arrayG ) {
    if( longest != shortest ) {
	return(1) ;
    } else {
	return(0) ;
    }
} else {
    if( (DOUBLE) longest / (DOUBLE)(barrayG[long_row]->desire) > 0.01 &&
				    longest >= (INT) mean_widthG ) {
	return(1) ;
    } else {
	return(0) ;
    }
}
}




findunlap2()
{

CBOXPTR cellptr ;
TIBOXPTR tileptr ;
INT last_cell ;
INT block , i , bigblkx , *blklen , delta_row_len ;

blklen = (INT *) Ysafe_malloc( (numRowsG + 1) * sizeof( INT ) ) ;
for( i = 0 ; i <= numRowsG ; i++ ) {
    blklen[i]  = 0 ;
}

for( block = 1 ; block <= numRowsG ; block++ ) {
    last_cell = pairArrayG[block][0] ;
    for( i = 1 ; i <= last_cell ; i++ ) {
	cellptr = carrayG[ pairArrayG[block][i] ] ;
	tileptr = cellptr->tileptr ;
	blklen[block] += tileptr->right - tileptr->left  ;
    }
}

bigblkx = 0 ;
largest_delta_row_lenG = -1000000 ;

fprintf(fpoG,"BLOCK    TOTAL CELL LENGTHS    OVER/UNDER TARGET\n");
for( i = 1 ; i <= numRowsG ; i++ ) {
    if( blklen[i] > blklen[ bigblkx ] ) {
	bigblkx = i ;
    }
    delta_row_len = blklen[i] - barrayG[i]->desire ;
    if( delta_row_len > largest_delta_row_lenG ) {
	largest_delta_row_lenG = delta_row_len ;
    }
    fprintf( fpoG, "%5d    %18d    %17d\n", i,
		    blklen[i] ,  delta_row_len );
}

fprintf( fpoG, "\nLONGEST Block is:%d   Its length is:%d\n",
			    bigblkx , blklen[ bigblkx ] ) ;

Ysafe_free( blklen ) ;

return ;
}
