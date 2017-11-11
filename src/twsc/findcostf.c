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
FILE:	    findcostf.c                                       
DESCRIPTION:find total cost with feeds.
CONTENTS:   findcostf()
	    installf()
	    install_clusters()
	    place_clusters()
DATE:	    Mar 27, 1989 
REVISIONS:  Apr  1, 1990 - added missing ignore test for wire penalty.
	    Thu Dec 20 00:23:46 EST 1990 - removed += operator.
	    Thu Jan 31 15:56:05 EST 1991 - added recompute_timecost
		for reconfig problems.
	    Wed Sep 11 11:18:19 CDT 1991 - modified
		for new global routing algorith.
----------------------------------------------------------------- */
#include <globals.h>
#include "allheaders.h"

#define PICK_INT(l,u) (((l)<(u)) ? ((RAND % ((u)-(l)+1))+(l)) : (l))

int blkleftG , blkriteG ;

/* static variables */
static int old_numBinS ;
static int cluster_norm_offsetS = 0 ;
static int **cluster_configS ;

int findcostf()
{
TIBOXPTR tileptr1 ;
CBOXPTR cellptr1 ;
BINPTR bptr ;
int left , right ;
int bin , LoBin , HiBin ;
int block , cell , blk ;
int startx , endx ;
int cost ;
int k , cbin , row ;

blkleftG = INT_MAX ;
blkriteG = INT_MIN ;
for( block = 1 ; block <= numRowsG ; block++ ) {
    if( barrayG[ block ]->bxcenter + barrayG[ block ]->bleft <
						blkleftG ) {
	blkleftG = barrayG[ block ]->bxcenter +
					barrayG[ block ]->bleft ;
    } 
    if( barrayG[ block ]->bxcenter + 
			barrayG[ block ]->bright > blkriteG ) {
	blkriteG = barrayG[ block ]->bxcenter +
				    barrayG[ block ]->bright ;
    }
}
binOffstG = blkleftG ;
max_blklengthG = blkriteG - blkleftG ;

old_numBinS = numBinsG ;

numBinsG = (int)( ( blkriteG - binOffstG ) / binWidthG ) ;
if( ( blkriteG - binOffstG ) > ( numBinsG * binWidthG ) ) {
    numBinsG++ ;
}

if( numBinsG > old_numBinS ) {
    for( row = 1 ; row <= numRowsG ; row++ ) {
	bin_configG[row] = (int *) Ysafe_realloc( bin_configG[row] ,
				(1 + numBinsG) * sizeof(int) ) ;
	for( bin = old_numBinS + 1 ; bin <= numBinsG ; bin++ ) {
	    bin_configG[row][bin] = 0 ;
	}
    }
}

cost = recompute_wirecost() ;

binpenalG = 0 ;
rowpenalG = 0 ;
penaltyG = 0 ;

for( block = 1 ; block <= numRowsG ; block++ ) {
    for( bin = 0 ; bin <= old_numBinS ; bin++ ) {
	Ysafe_free( binptrG[block][bin]->cell ) ;
	Ysafe_free( binptrG[block][bin] ) ;
    }
    Ysafe_free( binptrG[block] ) ;
}

for( block = 1 ; block <= numRowsG ; block++ ) {

    binptrG[block] = (BINPTR * ) Ysafe_malloc( (numBinsG + 1) *
				      sizeof( BINPTR ) ) ;
    left  = barrayG[ block ]->bleft + barrayG[ block ]->bxcenter ;
    right = barrayG[ block ]->bleft + barrayG[ block ]->bxcenter 
				   + barrayG[ block ]->desire   ;
    /* set barray->oldsize to zero for upcoming calculation */
    barrayG[ block ]->oldsize = 0 ;
    LoBin = SetBin( left ) ;
    HiBin = SetBin( right ) ;
    
    for( bin = 0 ; bin <= numBinsG ; bin++ ) {
	binptrG[block][bin] = (BINBOX *) Ysafe_malloc( 
					    sizeof(BINBOX) ) ; 
	binptrG[block][bin]->cell = (int *)Ysafe_malloc( 
					    10 * sizeof(int) );
	bptr = binptrG[block][bin] ;
	bptr->cell[0] = 0 ;
	bptr->right = binOffstG + bin * binWidthG ;
	bptr->left  = bptr->right - binWidthG ;
	if( bin == LoBin ) {
	    bptr->penalty = left - bptr->right ;
	} else if( bin == HiBin ) {
	    bptr->penalty = bptr->left - right ;
	} else if( bin > HiBin || bin < LoBin ) {
	    bptr->penalty = 0 ;
	} else {
	    bptr->penalty = - binWidthG ;
	}
    }
}

installf() ;

for( cell = 1 ; cell <= numcellsG - extra_cellsG ; cell++ ) {

    cellptr1 = carrayG[ cell ] ;
    tileptr1 = cellptr1->tileptr ;
    block = cellptr1->cblock ;

    startx = cellptr1->cxcenter + tileptr1->left  ;
    endx   = cellptr1->cxcenter + tileptr1->right ;

    barrayG[block]->oldsize += endx - startx ;

    cbin  = SetBin( cellptr1->cxcenter ) ;
    LoBin = SetBin( startx ) ;
    HiBin = SetBin( endx ) ;

    k = ++(binptrG[block][cbin]->cell[0]) ;
    if( k % 10 == 0 ) {
	binptrG[block][cbin]->cell = (int *) Ysafe_realloc(
	      binptrG[block][cbin]->cell, (k + 10) * sizeof( int ) ) ;
    }
    binptrG[block][cbin]->cell[k] = cell ;
    if( LoBin == HiBin ) {
	binptrG[block][LoBin]->penalty +=  ( endx - startx ) ;
    } else {
	bptr = binptrG[block][LoBin] ;
	bptr->penalty += ( bptr->right - startx ) ;

	bptr = binptrG[block][HiBin] ;
	bptr->penalty +=  ( endx - bptr->left ) ;

	if( LoBin + 1 < HiBin ) {
	    for( bin = LoBin + 1 ; bin <= HiBin - 1 ; bin++ ) {
		binptrG[block][bin]->penalty += binWidthG ;
	    }
	}
    }
}

for( block = 1 ; block <= numRowsG ; block++ ) {
    for( bin = 0 ; bin <= numBinsG ; bin++ ) {
	binpenalG += ABS( binptrG[block][bin]->penalty ) ;
    }
}

for( blk = 1 ; blk <= numRowsG ; blk++ ) {
    rowpenalG += ABS(barrayG[blk]->oldsize - barrayG[blk]->desire) ;
}

penaltyG = (int)( binpenConG * (double) binpenalG + 
				roLenConG * (double) rowpenalG ) ;

timingcostG = recompute_timecost() ;

return( cost ) ;
}

void installf()
{

int row , n , i , bin , diff , extra , stop , tmp ;
int c_limit , num_trys , cell_num ;
int *c_ptr ;

for( row = 1 ; row <= numRowsG ; row++ ) {
    tmp = feeds_in_rowG[row] * fdWidthG ;
    if( tmp > barrayG[row]->orig_desire ) {
	barrayG[row]->oldsize = barrayG[row]->orig_desire ;
    } else {
	barrayG[row]->oldsize = tmp ;
    }
}


for( row = 1 ; row <= numRowsG ; row++ ) {
    n = feeds_in_rowG[row] ;
    if( n < 1 ) {
	for( bin = 0 ; bin <= numBinsG ; bin++ ) {
	    bin_configG[row][bin] = 0 ;
	}
	for( ; bin <= old_numBinS ; bin++ ) {
	    bin_configG[row][bin] = 0 ;
	}
	continue ;
    }
    extra = 0 ;
    if( old_numBinS > numBinsG ) {
	for( bin = numBinsG + 1 ; bin <= old_numBinS ; bin++ ) {
	    if( bin_configG[row][bin] == 1 ) {
		extra++ ;
		bin_configG[row][bin] = 0 ;
	    }
	}
    }
    if( n < bin_configG[row][0] - extra ) {
	diff = bin_configG[row][0] - extra - n ;
        for( i = 0 ; i < diff ; ) {
	    bin = PICK_INT( 1 , numBinsG ) ;
	    if( bin_configG[row][bin] == 0 ) {
	        continue ;
	    }
	    i++ ;
	    bin_configG[row][bin] = 0 ;
	}
	bin_configG[row][0] = n ;
    } else if( n > bin_configG[row][0] - extra ) {
	diff = n - (bin_configG[row][0] - extra) ;
        for( i = 0, num_trys = 0 ; i < diff ; ) {
	    if( ++num_trys > 30 * diff ) {
		break ;
	    }
	    bin = PICK_INT( 1 , numBinsG ) ;
	    if( bin_configG[row][bin] == 1 ) {
	        continue ;
	    }
	    c_ptr = binptrG[row][bin]->cell ;
	    c_limit = c_ptr[0] ;
	    stop = 0 ;
	    for( cell_num = 1 ; cell_num <= c_limit ; cell_num++ ) {
		if( carrayG[ c_ptr[cell_num] ]->cclass < 0 ) {
		    stop = 1 ;
		    break ;
		}
	    }
	    if( stop ) {
		continue ;
	    }
	    i++ ;
	    bin_configG[row][bin] = 1 ;
	}
	bin_configG[row][0] = n - (diff - i) ;
    } else {
	bin_configG[row][0] = n ;
    }

    for( bin = 1 ; bin <= numBinsG ; bin++ ) {
	if( bin_configG[row][bin] == 1 ) {
	    binptrG[row][bin]->penalty += binWidthG ;
	}
    }
}

return;
}






install_clusters()
{

int row , n , i , bin , i_error , delta_bin , length_in_row , cell ;
int total ;
int total_actual_clusters ;
double error , n_double , cluster_norm ;

fprintf(fpoG,"total number of clusters which should be added in: %d\n",
			num_clustersG ) ;
total_actual_clusters = 0 ;

cluster_configS = (int **) Ysafe_malloc((1 + numRowsG) * sizeof(int *)) ;
for( row = 1 ; row <= numRowsG ; row++ ) {
    cluster_configS[row] = (int *) Ysafe_malloc((1+numBinsG)*sizeof(int));
    for( bin = 0 ; bin <= numBinsG ; bin++ ) {
	cluster_configS[row][bin] = 0 ;
    }
}

cluster_norm = (double) num_clustersG / (double) numRowsG + cluster_norm_offsetS ;

if( num_clustersG == 0 ) {
    return ;
}

error = 0.0 ;
total = 0 ;

for( row = 1 ; total < num_clustersG && row <= numRowsG ; row++ ) {
    length_in_row = 0 ;
    for( cell = 1 ; cell <= numcellsG - extra_cellsG ; cell++ ) {
	if( carrayG[cell]->cblock == row ) {
	    if( carrayG[cell]->cclass < 0 ) {
		length_in_row += carrayG[cell]->clength ;
	    }
	}
    }
    n_double = cluster_norm + error ;

    n = (int) n_double ;
    if( n_double - (double) n >= 0.5 ) {
	n++ ;
    }
    error = n_double - (double) n ;

    while( total + n > num_clustersG ) {
	n-- ;
    }

    if( length_in_row + n * cluster_widthG > barrayG[row]->desire ) {
	if( row == numRowsG ) {
	    /*  can't fit all the required clusters; increase cluster_norm */
	    ++cluster_norm_offsetS ;
	    for( row = 1 ; row <= numRowsG ; row++ ) {
		Ysafe_free( cluster_configS[row] ) ;
	    }
	    Ysafe_free( cluster_configS ) ;
	    install_clusters() ;
	    return ;
	}
	n = (barrayG[row]->desire - length_in_row) / cluster_widthG ;
	if( n < 0 ) {
	    n = 0 ;
	}
	error += (double)( (int) n_double - n ) ; 
    }

    if( n == 0 ) {
	continue ;
    }
    total += n ;

    i_error = 0 ;
    bin = 0 ;
    for( i = 0 ; i < n ; i++ ) {
	delta_bin = (numBinsG + i_error) / n ;
	bin += delta_bin ;
	i_error += numBinsG - delta_bin * n ;
	if( bin < 1 ) bin = 1 ;
	if( bin > numBinsG ) bin = numBinsG ;
	cluster_configS[row][bin]++ ;
    }

    fprintf(fpoG,"Number of clusters added to row:%d was:%d\n", row,i);
    total_actual_clusters += i ;
    for( bin = 1 ; bin <= numBinsG ; bin++ ) {
	if( cluster_configS[row][bin] >= 1 ) {
	    barrayG[row]->oldsize += cluster_configS[row][bin] *
						    cluster_widthG ;
	}
    }
}
fprintf(fpoG,"actual total number of clusters which were added in: %d\n",
			total_actual_clusters ) ;
return;
} /* end install_clusters */





place_clusters()
{

int bin , cell , row , c , count ;

cell = numcellsG - extra_cellsG ;
for( row = 1 ; row <= numRowsG ; row++ ) {
    for( bin = 1 ; bin <= numBinsG ; bin++ ) {
	if( cluster_configS[row][bin] == 0 ) {
	    continue ;
	}
	count = cluster_configS[row][bin] ;
	for( c = 1 ; c <= count ; c++ ) {
	    carrayG[++cell]->cblock = row ;
	    carrayG[cell]->cycenter = barrayG[row]->bycenter ;
	    carrayG[cell]->cxcenter = (((double) bin / (double) numBinsG) *
			(barrayG[row]->bright - barrayG[row]->bleft)) +
			barrayG[row]->bleft + barrayG[row]->bxcenter ;
	}
    }
}

return ;
}

/* **************************************************************** */
int recompute_wirecost()
{
    int n ;
    int x, y ;
    int net ;
    int cell ;
    int cost ;
    int corient ;
    int block ;
    CBOXPTR ptr ;
    DBOXPTR dimptr ;
    PINBOXPTR netptr , termptr ;

    for( cell = 1 ; cell <= lastpadG ; cell++ ) {
	ptr = carrayG[ cell ] ;
	corient = ptr->corient ;
	block   = ptr->cblock  ;
	if( cell <= numcellsG - extra_cellsG ) {
	    ptr->cycenter = barrayG[block]->bycenter ;
	    for( termptr = ptr->pins ; termptr ; termptr=termptr->nextpin ) {
		termptr->xpos = termptr->txpos[corient/2] + ptr->cxcenter;
		termptr->ypos = termptr->typos[corient%2] + ptr->cycenter;
	    }
	} else if( cell > numcellsG ) {
	    for( termptr = ptr->pins ; termptr ; termptr=termptr->nextpin ) {
		termptr->xpos = termptr->txpos[1] + ptr->cxcenter;
		termptr->ypos = termptr->typos[1] + ptr->cycenter;
	    }
	}
    }

    cost = 0 ;
    for( net = 1 ; net <= numnetsG ; net++ ) {
	dimptr =  netarrayG[ net ] ;
	if( dimptr->ignore == 1 ){
	    continue ;
	}
	if( netptr = dimptr->pins ) {
	    dimptr->xmin = dimptr->xmax = netptr->xpos ;
	    dimptr->ymin = dimptr->ymax = netptr->ypos ;
	    dimptr->Lnum = dimptr->Rnum = 1 ;
	    dimptr->Bnum = dimptr->Tnum = 1 ;
	    netptr = netptr->next ;
	}
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
	cost += dimptr->halfPx = dimptr->newhalfPx = 
					     dimptr->xmax - dimptr->xmin ;
	dimptr->halfPy = dimptr->newhalfPy = dimptr->ymax - dimptr->ymin ;
	cost = cost + (int)( vertical_wire_weightG * (double) dimptr->halfPy ) ;
    } /* end for loop */
    return( cost ) ;

} /* end recompute_wirecost */


int recompute_timecost()
{

int pathcount ;
int net ;
int timingpenal ;
int length ;
PATHPTR path ;
DBOXPTR dimptr ;
GLISTPTR net_of_path ;

/* ************* now calculate the timing penalty ************** */
timingpenal = 0 ;
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

	length = length + (int) 
	    (horizontal_path_weightG * (double) dimptr->halfPx) ;
	length = length + (int) 
		     (vertical_path_weightG * (double) dimptr->halfPy) ;
    }
    /* save result */
    path->path_len = path->new_path_len = length ;

    /* calculate penalty */
    /* no penalty if within target window */
    /* lowerbound <= length <= upperbound */
    if( length > path->upper_bound ){
	timingpenal += length - path->upper_bound ; 
    } else if( length < path->lower_bound ){
	timingpenal += path->lower_bound - length ;
    }
}
return( timingpenal ) ;

} /* end recompute_timecost */
