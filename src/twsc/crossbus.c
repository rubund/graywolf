/*
 *   Copyright (C) 1989-1990 Yale University
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
FILE:	    crossbus.c                                       
DESCRIPTION:cross bus code.
CONTENTS:  handle_crossbuses()
	    check_violations()
	    reduce_violations()
DATE:	    Mar 27, 1989 
REVISIONS:  
----------------------------------------------------------------- */
#ifndef VMS
#ifndef lint
static char SccsId[] = "@(#) crossbus.c (Yale) version 4.5 10/14/90" ;
#endif
#endif

#include "standard.h"
#include "main.h"
#include "groute.h"
#include "pads.h"

/* global definitions */
extern INT left_row_boundaryG ;
extern INT row_extentG ;
extern BOOL exclude_noncrossbus_padsG ;

handle_crossbuses()
{

PINBOXPTR netptr ;
FENCEBOXPTR fence , fence2, save_fence , prev_fence ;
INT cell , net , process , row_area , area ;
INT length , max_x , min_x , max_y , min_y , count , cross_bus ;
INT block , x , y , avg_length , left , right ;
INT large_pos , large_neg , ro_height , tmp , i ;
INT distance , j , save_j , side ;
INT *left_pins , *rite_pins , *top_pins , *bot_pins , bound ;

left_pins = (INT *) Ysafe_malloc( (1 + numtermsG) * sizeof( INT ) ) ;
rite_pins = (INT *) Ysafe_malloc( (1 + numtermsG) * sizeof( INT ) ) ;
top_pins  = (INT *) Ysafe_malloc( (1 + numtermsG) * sizeof( INT ) ) ;
bot_pins  = (INT *) Ysafe_malloc( (1 + numtermsG) * sizeof( INT ) ) ;

large_pos = 10000000 ;
large_neg = -10000000 ;
if( rowHeightG % 2 == 1 ) {
    ro_height = rowHeightG + 1 ;
} else {
    ro_height = rowHeightG ;
}

for( net = 1 ; net <= numnetsG ; net++ ) {
    length = 0 ;
    count = 0 ;
    process = 0 ;
    left_pins[0] = 0 ;
    rite_pins[0] = 0 ;
    top_pins[0]  = 0 ;
    bot_pins[0]  = 0 ;
    min_x = large_pos ;
    min_y = large_pos ;
    max_x = large_neg ;
    max_y = large_neg ;
    for( netptr = netarrayG[net]->pins;netptr ; netptr = netptr->next ) {
	if( netptr->cell > numcellsG ) {
	    process = 1 ;
	    x = carrayG[ netptr->cell ]->cxcenter ;
	    y = carrayG[ netptr->cell ]->cycenter ;
	    if( carrayG[ netptr->cell ]->padptr ){
		side = carrayG[ netptr->cell ]->padptr->padside ;
	    } else {
		side = 0 ;
	    }
	    if( side == 1 /* L */ ) {
		left_pins[ ++left_pins[0] ] = netptr->cell ;
	    } else if( side == 2 /* T */ ) {
		top_pins[ ++top_pins[0] ] = netptr->cell ;
	    } else if( side == 3 /* R */ ) {
		rite_pins[ ++rite_pins[0] ] = netptr->cell ;
	    } else if( side == 4 /* B */ ) {
		bot_pins[ ++bot_pins[0] ] = netptr->cell ;
	    }
	    if( x < min_x ) {
		min_x = x ;
	    }
	    if( x > max_x ) {
		max_x = x ;
	    }
	    if( y < min_y ) {
		min_y = y ;
	    }
	    if( y > max_y ) {
		max_y = y ;
	    }
	} else {
	    count++ ;
	    length += carrayG[netptr->cell]->clength ;
	}
    } /* close of first pass thru all pins for this net */
    if( !process ) {
	continue ;
    }

    if( count == 0 ) {
	continue ;
    }
    avg_length = length / count ;
    if( avg_length < (INT) mean_widthG ) {
	avg_length = (INT) mean_widthG ;
    }
    if( avg_length % 2 == 1 ) {
	avg_length++ ;
    }

    if( left_pins[0] > 0 && rite_pins[0] > 0 ) {
	for( i = 1 ; i <= left_pins[0] ; i++ ) {
	    distance = 10000000 ;
	    for( j = 1 ; j <= rite_pins[0] ; j++ ) {
		if( ABS( carrayG[ left_pins[i] ]->cycenter -
			carrayG[ rite_pins[j] ]->cycenter) < distance ) {
		    distance = ABS( carrayG[ left_pins[i] ]->cycenter -
				    carrayG[ rite_pins[j] ]->cycenter) ;
		    save_j = j ;
		}
	    }
	    if( distance <= 5 * ro_height ) {
		min_y = carrayG[ left_pins[i] ]->cycenter ;
		max_y = carrayG[ rite_pins[save_j] ]->cycenter ;
		if( min_y > max_y ) {
		    tmp = min_y ;
		    min_y = max_y ;
		    max_y = tmp ;
		}
		bound = 3 * (1 + (count - 1)/2) * ro_height ;
		/* while( max_y - min_y < 9 * ro_height ) */
		while( max_y - min_y < bound ) {
		    min_y -= ro_height / 2 ;
		    max_y += ro_height / 2 ;
		}
		min_x = carrayG[ left_pins[i] ]->cxcenter ;
		max_x = carrayG[ rite_pins[save_j] ]->cxcenter ;

		for( netptr = netarrayG[net]->pins ;netptr;
			netptr = netptr->next ) {
		    if( netptr->cell > numcellsG ) {
			continue ;
		    }
		    fence = carrayG[netptr->cell]->fence ;
		    carrayG[netptr->cell]->fence = (FENCEBOXPTR)
				    Ysafe_malloc( sizeof(FENCEBOX) ) ;
		    carrayG[netptr->cell]->fence->next_fence = fence ;
		    carrayG[netptr->cell]->fence->min_block  = min_y ;
		    carrayG[netptr->cell]->fence->max_block  = max_y ;
		    carrayG[netptr->cell]->fence->min_xpos = min_x ;
		    carrayG[netptr->cell]->fence->max_xpos = max_x ;
		}
	    }
	}
    } else if( top_pins[0] > 0 && bot_pins[0] > 0 ) {
	for( i = 1 ; i <= bot_pins[0] ; i++ ) {
	    distance = 10000000 ;
	    for( j = 1 ; j <= top_pins[0] ; j++ ) {
		if( ABS( carrayG[ bot_pins[i] ]->cxcenter -
			carrayG[ top_pins[j] ]->cxcenter) < distance ) {
		    distance = ABS( carrayG[ bot_pins[i] ]->cxcenter -
				    carrayG[ top_pins[j] ]->cxcenter) ;
		    save_j = j ;
		}
	    }
	    if( distance <= 4 * avg_length ) {
		min_x = carrayG[ bot_pins[i] ]->cxcenter ;
		max_x = carrayG[ top_pins[save_j] ]->cxcenter ;
		if( min_x > max_x ) {
		    tmp = min_x ;
		    min_x = max_x ;
		    max_x = tmp ;
		}
		bound = 2 * (2 + (count - 1)/2) * avg_length ;
		/* while( max_x - min_x < 6 * avg_length ) */
		while( max_x - min_x < bound ) {
		    min_x -= avg_length / 2 ;
		    max_x += avg_length / 2 ;
		}
		min_y = carrayG[ bot_pins[i] ]->cycenter ;
		max_y = carrayG[ top_pins[save_j] ]->cycenter ;

		netptr = netarrayG[net]->pins ;
		for( ; netptr ; netptr = netptr->next ) {
		    if( netptr->cell > numcellsG ) {
			continue ;
		    }
		    fence = carrayG[netptr->cell]->fence ;
		    carrayG[netptr->cell]->fence = (FENCEBOXPTR)
				    Ysafe_malloc( sizeof(FENCEBOX) ) ;
		    carrayG[netptr->cell]->fence->next_fence = fence ;
		    carrayG[netptr->cell]->fence->min_block  = min_y ;
		    carrayG[netptr->cell]->fence->max_block  = max_y ;
		    carrayG[netptr->cell]->fence->min_xpos = min_x ;
		    carrayG[netptr->cell]->fence->max_xpos = max_x ;
		}
	    }
	}
    } else {
	if( min_x < left_row_boundaryG ) {
	    min_x = left_row_boundaryG ;
	}
	if( min_x > left_row_boundaryG + row_extentG ) {
	    min_x = left_row_boundaryG + row_extentG ;
	}
	if( max_x < left_row_boundaryG ) {
	    max_x = left_row_boundaryG ;
	}
	if( max_x > left_row_boundaryG + row_extentG ) {
	    max_x = left_row_boundaryG + row_extentG ;
	}
	if( min_y < barrayG[1]->bycenter ) {
	    min_y = barrayG[1]->bycenter ;
	}
	if( min_y > barrayG[numRowsG]->bycenter ) {
	    min_y = barrayG[numRowsG]->bycenter ;
	}
	if( max_y < barrayG[1]->bycenter ) {
	    max_y = barrayG[1]->bycenter ;
	}
	if( max_y > barrayG[numRowsG]->bycenter ) {
	    max_y = barrayG[numRowsG]->bycenter ;
	}
	min_x -= (3 + (count * 3)) * avg_length ;
	max_x += (3 + (count * 3)) * avg_length ;
	min_y -= (3 + (count * 3)) * rowHeightG ;
	max_y += (3 + (count * 3)) * rowHeightG ;

	netptr = netarrayG[net]->pins ;
	for( ; netptr ; netptr = netptr->next ) {
	    if( netptr->cell > numcellsG ) {
		continue ;
	    }
	    fence = carrayG[netptr->cell]->fence ;
	    carrayG[netptr->cell]->fence = (FENCEBOXPTR)
			    Ysafe_malloc( sizeof(FENCEBOX) ) ;
	    carrayG[netptr->cell]->fence->next_fence = fence ;
	    carrayG[netptr->cell]->fence->min_block  = min_y ;
	    carrayG[netptr->cell]->fence->max_block  = max_y ;
	    carrayG[netptr->cell]->fence->min_xpos = min_x ;
	    carrayG[netptr->cell]->fence->max_xpos = max_x ;
	}
    }
}

for( cell = 1 ; cell <= numcellsG ; cell++ ) {
    fence = carrayG[cell]->fence ;
    if( !fence ) {
	continue ;
    }
    for( ; fence ; fence = fence->next_fence ) {
	for( block = 1 ; block <= numRowsG ; block++ ) {
	    if( barrayG[block]->bycenter >= fence->min_block ) {
		fence->min_block = block ;
		break ;
	    }
	}
	for( block = numRowsG ; block >= 1 ; block-- ) {
	    if( barrayG[block]->bycenter <= fence->max_block ) {
		fence->max_block = block ;
		break ;
	    }
	}
    }
    fence = carrayG[cell]->fence ;
    for( ; fence ; fence = fence->next_fence ) {
	if( fence->min_xpos < left_row_boundaryG ) {
	    fence->min_xpos = left_row_boundaryG ;
	}
	if( fence->max_xpos > left_row_boundaryG + row_extentG ) {
	    fence->max_xpos = left_row_boundaryG + row_extentG ;
	}
    }
    fence = carrayG[cell]->fence ;
    cross_bus = 0 ;
    for( ; fence ; fence = fence->next_fence ) {
	if( (fence->min_xpos <= left_row_boundaryG && 
		fence->max_xpos >= left_row_boundaryG + row_extentG) ||
		      (fence->min_block <= 1 && 
		      fence->max_block >= numRowsG)){
	    cross_bus = 1 ;
	    break ;
	}
    }
    if( cross_bus ) {
	fence = carrayG[cell]->fence ;
	prev_fence = NULL ;
	for( ; fence ; ) { 
	    if( (fence->min_xpos <= left_row_boundaryG && 
			fence->max_xpos >= left_row_boundaryG + 
			row_extentG) || (fence->min_block <= 1 &&
				fence->max_block >= numRowsG )){
		prev_fence = fence ; 
		fence = fence->next_fence ; 
		continue ;
	    }
	    if( prev_fence == NULL ) {
		carrayG[cell]->fence = fence->next_fence ;
		Ysafe_free( fence ) ;
		fence = carrayG[cell]->fence ;
	    } else {
		prev_fence->next_fence = fence->next_fence ;
		Ysafe_free( fence ) ;
		fence = prev_fence->next_fence ;
	    }
	}

	/* intersecting crossbuses of the same orienation and merge'm */
	fence = carrayG[cell]->fence ;
	for( ; fence ; fence = fence->next_fence ) {
	    if( fence->min_block <= 1 && fence->max_block >= numRowsG ){
		fence2 = fence->next_fence ;
		prev_fence = fence ;
		for( ; fence2 ; ) { 
		    if( fence2->min_block <= 1 && 
					fence2->max_block >= numRowsG ){
			if( fence->min_xpos > fence2->max_xpos ||
				fence2->min_xpos > fence->max_xpos ) {
			    prev_fence = fence2 ;
			    fence2 = fence2->next_fence ;
			    continue ;
			}
			if( fence->min_xpos > fence2->min_xpos ) {
			    fence->min_xpos = fence2->min_xpos ;
			}
			if( fence->max_xpos < fence2->max_xpos ) {
			    fence->max_xpos = fence2->max_xpos ;
			}
			prev_fence->next_fence = fence2->next_fence ;
			Ysafe_free( fence2 ) ;
			fence2 = prev_fence->next_fence ;
		    } else {
			prev_fence = fence2 ;
			fence2 = fence2->next_fence ;
		    }
		}
	    }
	}
	fence = carrayG[cell]->fence ;
	for( ; fence ; fence = fence->next_fence ) {
	    if( fence->min_xpos <= left_row_boundaryG && 
		    fence->max_xpos >= left_row_boundaryG + row_extentG ){
		fence2 = fence->next_fence ;
		prev_fence = fence ;
		for( ; fence2 ; ) { 
		    if( fence2->min_xpos <= left_row_boundaryG && 
				    fence2->max_xpos >= 
				    left_row_boundaryG + row_extentG ){
			if( fence->min_block > fence2->max_block ||
				fence2->min_block > fence->max_block ) {
			    prev_fence = fence2 ;
			    fence2 = fence2->next_fence ;
			    continue ;
			}
			if( fence->min_block > fence2->min_block ) {
			    fence->min_block = fence2->min_block ;
			}
			if( fence->max_block < fence2->max_block ) {
			    fence->max_block = fence2->max_block ;
			}
			prev_fence->next_fence = fence2->next_fence ;
			Ysafe_free( fence2 ) ;
			fence2 = prev_fence->next_fence ;
		    } else {
			prev_fence = fence2 ;
			fence2 = fence2->next_fence ;
		    }
		}
	    }
	}
    } else { /* keep only largest fence */
	fence = carrayG[cell]->fence ;
	row_area = 0 ;
	for( ; fence ; fence = fence->next_fence ) { 
	    left = fence->min_xpos ;
	    if( left < left_row_boundaryG ) {
		left = left_row_boundaryG ;
	    }
	    right = fence->max_xpos ;
	    if( right > left_row_boundaryG + row_extentG ) {
		right = left_row_boundaryG + row_extentG ;
	    }
	    area = (right - left) * (fence->max_block - 
						fence->min_block) ;
	    if( area > row_area ) {
		row_area = area ;
		save_fence = fence ;
	    }
	}
	fence = carrayG[cell]->fence ;
	prev_fence = NULL ;
	for( ; fence ; ) { 
	    if( fence == save_fence ) {
		prev_fence = fence ; 
		fence = fence->next_fence ; 
		continue ;
	    }
	    if( prev_fence == NULL ) {
		carrayG[cell]->fence = fence->next_fence ;
		Ysafe_free( fence ) ;
		fence = carrayG[cell]->fence ;
	    } else {
		prev_fence->next_fence = fence->next_fence ;
		Ysafe_free( fence ) ;
		fence = prev_fence->next_fence ;
	    }
	}
	if( exclude_noncrossbus_padsG ) {
	    Ysafe_free( carrayG[cell]->fence ) ;
	    carrayG[cell]->fence = NULL ;
	}
    }
}

count = 0 ;
for( cell = 1 ; cell <= numcellsG ; cell++ ) {
    fence = carrayG[cell]->fence ;
    if( fence ) {
	count++ ;
    } else {
	continue ;
    }
    /*
    printf("Constrained Cell #%d:\n ", cell ) ;
    for( ; fence ; fence = fence->next_fence ) {
	printf("    X: %5d, %5d   R:%3d, %3d\n",  fence->min_xpos , 
						  fence->max_xpos ,
						  fence->min_block ,
						  fence->max_block ) ;
    }
    printf("\n");
    */
}
fprintf(fpoG,"Percentage of constrained cells:%f\n", 
			(DOUBLE) count / (DOUBLE) numcellsG ) ;
fflush(stdout);
Ysafe_free( left_pins ) ;
Ysafe_free( rite_pins ) ;
Ysafe_free( top_pins  ) ;
Ysafe_free( bot_pins  ) ;

return ;
}




check_violations()
{

FENCEBOXPTR fence ;
INT cell , total_r , total_l , x , error , row_error ;
INT max_x , min_x , row , min_block , max_block ;
INT r_error ;

total_r = 0 ;
total_l = 0 ;
for( cell = 1 ; cell <= numcellsG ; cell++ ) {
    fence = carrayG[cell]->fence ;
    if( !fence ) {
	continue ;
    }
    x = carrayG[cell]->cxcenter ;
    row = carrayG[cell]->cblock ;
    error = 10000000 ;
    row_error = 10000000 ;
    for( ; fence && (!(error == 0 && row_error == 0)) ; 
					fence = fence->next_fence ) {
	max_x = fence->max_xpos ;
	min_x = fence->min_xpos ;
	min_block = fence->min_block ;
	max_block = fence->max_block ;
	if( x > max_x ) {
	    if( x - max_x < error ) {
		error = x - max_x ;
		r_error = 1 ;
		if( row > max_block ) {
		    if( row - max_block < row_error ) {
			row_error = row - max_block ;
		    }
		} else if( row < min_block ) {
		    if( min_block - row < row_error ) {
			row_error = min_block - row;
		    }
		} else {
		    row_error = 0 ;
		}
	    }
	} else if( x < min_x ) {
	    if( min_x - x < error ) {
		error = min_x - x ;
		r_error = 0 ;
		if( row > max_block ) {
		    if( row - max_block < row_error ) {
			row_error = row - max_block ;
		    }
		} else if( row < min_block ) {
		    if( min_block - row < row_error ) {
			row_error = min_block - row;
		    }
		} else {
		    row_error = 0 ;
		}
	    }
	} else {
	    error = 0 ;
	    if( row > max_block ) {
		if( row - max_block < row_error ) {
		    row_error = row - max_block ;
		}
	    } else if( row < min_block ) {
		if( min_block - row < row_error ) {
		    row_error = min_block - row;
		}
	    } else {
		row_error = 0 ;
	    }
	}
    }
    if( !( error == 0 && row_error == 0) ) {
	if( error > (INT) mean_widthG ) {
	    if( r_error == 1 ) {
		total_r += error ;
		/*
		fprintf(fpoG,"Right Violation for cell #%d ", cell );
		fprintf(fpoG,"X ERROR by %d\n", error ) ;
		*/
	    } else {
		total_l += error ;
		/*
		fprintf(fpoG,"Left  Violation for cell #%d ", cell );
		fprintf(fpoG,"X ERROR by %d\n", error ) ;
		*/
	    }
	    /*
	    fprintf(fpoG,"Row Error by %d rows\n", row_error ) ;
	    */
	}
    }
}
fprintf(fpoG,"Total placement violations to the left:%d\n", total_l );
fprintf(fpoG,"Total placement violations to the rite:%d\n", total_r );

return ;
}




reduce_violations()
{

FENCEBOXPTR fence , save_fence ;
INT cell , x , error , work , l_error , r_error , row ;
INT max_x , min_x , i ;

work = 0 ;
for( row = 1 ; row <= numRowsG ; row++ ) {
    for( i = 1 ; i <= pairArrayG[row][0] ; i++ ) {
	cell = pairArrayG[row][i] ;
	fence = carrayG[cell]->fence ;
	if( !fence ) {
	    continue ;
	}
	x = carrayG[cell]->cxcenter ;
	l_error = 10000000 ;
	error = 10000000 ;
	r_error = 10000000 ;
	for( ; fence && error != 0 ; fence = fence->next_fence ) {
	    max_x = fence->max_xpos ;
	    min_x = fence->min_xpos ;
	    if( error == 0 ) {
	    } else if( x > max_x || x < min_x ) {
		if( x > max_x ) {
		    if( x - max_x < error ) {
			r_error = x - max_x ;
			save_fence = fence ;
		    }
		} else {
		    if( min_x - x < error ) {
			l_error = min_x - x ;
			save_fence = fence ;
		    }
		}
	    } else {
		error = 0 ;
	    }
	}
	if( error != 0 ) {
	    work++ ;
	    if( l_error < r_error ) {
		/* we should shift the cell to the right */
		carrayG[ pairArrayG[row][i] ]->cxcenter =
			  save_fence->min_xpos + 3 * (INT)(mean_widthG) ;
	    } else {
		/* we should shift the cell to the left */
		carrayG[ pairArrayG[row][i] ]->cxcenter =
			  save_fence->max_xpos - 3 * (INT)(mean_widthG) ;
	    }
	}
    }
}
return( work ) ;
}
