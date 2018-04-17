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
FILE:	    utemp.c                                       
DESCRIPTION:outer loop of simulated annealing algorithm.
CONTENTS:   utemp()
	    from_middle()
	    from_beginning()
	    INT compute_attprcel(flag)
		INT flag;
	    rm_overlapping_feeds()
	    refine_placement()
	    simple_refine_placement()
	    parametric_refine( k_max , k_limit )
		INT k_max, k_limit ;
	    route_only_critical_nets() 
	    elim_nets()
DATE:	    Mar 27, 1989 
REVISIONS:  Fri Jan 25 23:45:25 PST 1991 - added user meter.
	    Mon Aug 12 17:01:03 CDT 1991 - changed timing ASSERTIONS
		to D( ) constructs to speed execution time during
		debug mode.
	    Thu Aug 22 22:27:18 CDT 1991 - Carl made changes
		for rigidly fixed cells.
----------------------------------------------------------------- */
#ifndef VMS
#ifndef lint
static char SccsId[] = "@(#) utemp.c (Yale) version 4.19 4/5/92" ;
#endif
#endif


#include "standard.h"
#include "main.h"
#include "groute.h"
#include "ucxxglb.h"
#include "readpar.h"
#include "parser.h"

#include <yalecad/message.h>
#include <yalecad/debug.h>

/* global variables */
INT moveable_cellsG ;
BOOL pairtestG;
BOOL no_row_lengthsG ;

/* global variable references */
extern INT attprcelG ;
extern INT spacer_widthG ;
extern INT orig_max_row_lengthG ;
extern BOOL noPairsG ;
extern BOOL good_initial_placementG ;
extern BOOL orientation_optimizationG ;
extern BOOL stand_cell_as_gate_arrayG ;
extern BOOL prop_rigid_cellsG ;
extern BOOL rigidly_fixed_cellsG ;
extern DOUBLE finalRowControlG ;
extern DOUBLE initialRowControlG ;
extern DOUBLE ratioG;

INT comparex() ;
INT compute_attprcel() ;

utemp()
{

INT check ;
unsigned i2 ;
INT i , freeze ;


if( orientation_optimizationG ) {
    pairtestG = TRUE ;
}

fraction_doneG = 0.0 ;

check = 0 ;
freeze = 10000000 ;
init_table() ;


attprcelG = compute_attprcel(1);

if( pairtestG == 0 ) {
    attmaxG = attprcelG * moveable_cellsG ;

    if( iterationG < 1 ) {
	resume_runG = 0 ;
    }
    if( !resume_runG && !good_initial_placementG ) {
	from_beginning() ;
    } else if( resume_runG ) {
	from_middle() ;
    }
}

for( ; ; ) {
    if( pairtestG == FALSE && TG >= 0.01 ) {
	uloop() ;
	savewolf(0) ; 
    } else {
	D( "twsc/after_annealing", 
	    G( process_graphics() ) ;
	) ;
	pairtestG = TRUE ;
	if( check == 0 ) {
	    check = 1 ;

	    savewolf(0) ;
	    freeze = iterationG ;

	    if( connection_machineG == TRUE ) {
		findunlap(0) ;
		outcm() ;
	    }

	    findunlap(0) ;
#ifndef MITLL
	    if( SGGRG || (!doglobalG) ) {
		even_the_rows(0,FALSE) ;
		M( MSG, NULL,"evening the row lengths\n") ;
		findunlap(0) ;
	    }
#else
	    if( check_row_lengths() ) {
		gate_array_even_the_rows(0) ;
		M( MSG, NULL,"evening the row lengths\n") ;
		findunlap(0) ;
	
		for( i = 1 ; i <= 5 ; i++ ) {
		    if( check_row_lengths() ) {
			sprintf(YmsgG,"evening the row lengths %d\n",
							    i+1) ;
			M( MSG, NULL, YmsgG ) ;
			even_the_rows_2( i ) ;
			findunlap(0) ;
		    }
		}
	    }
#endif
	    penaltyG = 0 ;
	    /*
	     *  This computes new wire costs for the compacted
	     *  placement (and feed insertion).  It calls unlap()
	     *  which sorts and places the cells end-to-end
	     */
	    M( MSG, NULL,"Removed the cell overlaps --- ");
	    M( MSG, NULL,"Will do neighbor interchanges only now\n");
	    sprintf( YmsgG, "\nTOTAL INTERCONNECT LENGTH: %d\n",funccostG);
	    M( MSG, NULL, YmsgG ) ;
	    sprintf(YmsgG,"initialRowControl:%8.3f\n", initialRowControlG);
	    M( MSG, NULL, YmsgG ) ;
	    sprintf(YmsgG,"finalRowControl:%8.3f\n", finalRowControlG);
	    M( MSG, NULL, YmsgG ) ;
	    fflush(fpoG);

	    attmaxG = 5 * moveable_cellsG ;
	    if( noPairsG == 0 ) {
		TG = 0.001 ;
		M(  MSG, NULL, "iter      T      Wire accept Time\n" ) ;
		upair() ;
		savewolf(1) ;
	    }
	} else {
	    if( noPairsG == 0 ) {
		TG = 0.001 ;
		upair() ;
		savewolf(1) ;
	    }
	}
	print_paths() ;
    }

    if(!(Ymessage_get_mode() )){
	/* if we aren't dumping everything to the screen */
	/* show iteration number */
	USER_INCR_METER() ;
	printf("%3d ", iterationG );
	if( iterationG % 15 == 0 ) {
	    printf("\n");
	}
    }
    ++iterationG;
    fflush( stdout ) ;
    G( check_graphics(TRUE) ) ;

    if( iterationG >= freeze + 3 ) {

	ASSERT( dprint_error(), NULL, NULL ) ;
	if( doglobalG ) {
	    execute_global_router() ;
	} else {
	    findunlap(0) ;
	    output() ; 
	}
	sprintf(YmsgG,"FINAL TOTAL INTERCONNECT LENGTH: %d\n",funccostG);
	M( MSG, NULL, YmsgG ) ;
	sprintf(YmsgG,"FINAL OVERLAP PENALTY: %d    ", penaltyG );
	M( MSG, NULL, YmsgG ) ;
	sprintf(YmsgG, "FINAL VALUE OF TOTAL COST IS: %d\n", 
					    funccostG + penaltyG ) ;
	M( MSG, NULL, YmsgG ) ;
	sprintf(YmsgG,"MAX NUMBER OF ATTEMPTED FLIPS PER T:%8d\n",attmaxG);
	M( MSG, NULL, YmsgG ) ;
	break ;
    } 
}
return ;
}

from_middle()
{

    init_control(-1);		/* set move generation controller. */

    attmaxG = attprcelG * moveable_cellsG ;
    M( MSG, NULL, "\nIter T        fds    Wire     Penal  Time  P_lim ");
    M( MSG, NULL, " err binC rowC  timeC  s/p  rej. Acc.\n");
    sprintf(YmsgG,"%3d: %6.2le %6ld %-8ld %-6ld %-6ld",
	iterationG++,0.0,0,funccostG,rowpenalG,timingcostG);
    M( MSG, NULL, YmsgG ) ;
    sprintf(YmsgG,"%6ld %4.2lf %4.2lf %5.2lf %4.2lf %4.2lf %4.2lf %5.3lf\n",
	P_limitG,0.0,binpenConG,roLenConG,timeFactorG,0.0,0.0,ratioG);
    M( MSG, NULL, YmsgG ) ;
    fflush(fpoG);
}


from_beginning()
{

    init_uloop();
    init_control(1);		/* set move generation controller. */
    attmaxG = attprcelG * moveable_cellsG ;
    iterationG = -1 ;
    TG = 1.0e10;	/* set to VERY HIGH temperature. */
    uloop();

    if( moveable_cellsG < 300 ) {
	attmaxG *= 2;
    }
    /* collect statistics */
    uloop();			/* discard old result and try again. */

    /* init_parameters(attmax,ratio) ; */

    attmaxG = attprcelG * moveable_cellsG ;
    M( MSG, NULL, "\nIter T        fds    Wire     Penal  Time     P_lim ");
    if( swappable_gates_existG ) {
	M( MSG, NULL, " err binC  rowC timeC  s/p  rej. Acc.");
	M( MSG, NULL, " g_sw Vwt\n");
    } else {
	M( MSG, NULL, " err binC  rowC timeC  s/p  rej. Acc. Vwt\n");
    }
    sprintf(YmsgG,"%3d: %6.2le %6ld %-8ld %-6ld %-8ld",
	iterationG++,0.0,0,funccostG,rowpenalG,timingcostG );
    M( MSG, NULL, YmsgG ) ;
    sprintf(YmsgG,"%6ld %4.2lf %4.2lf %5.2lf %5.2lf %4.2lf %4.2lf %5.3lf\n",
	P_limitG,0.0,binpenConG,roLenConG,timeFactorG,0.0,0.0,ratioG);
    M( MSG, NULL, YmsgG ) ;
    fflush(fpoG);
}

INT compute_attprcel(flag)
INT flag;
{
    INT cell, n;

    /* 1 March 1990 by Carl
    if( gate_array ) {
        for( cell = 1 ; cell <= numcellsG - extra_cellsG ; cell++ ) {
	    if( !rigidly_fixed_cellsG ) {
		if( carrayG[cell]->cclass < 0 ) {
		    continue ;
		}
	    }
	    moveable_cells++ ;
        }
    } else {
        for( cell = 1 ; cell <= numcellsG ; cell++ ) {
	    if( !rigidly_fixed_cellsG ) {
		if( carrayG[cell]->cclass < 0 ) {
		    continue ;
		}
	    }
	    moveable_cells++ ;
        }
    }
    */
    moveable_cellsG = numcellsG - extra_cellsG ;

    if( moveable_cellsG <= 500 ) {
	if( moveable_cellsG <= 0 ){
	    M( ERRMSG, "compute_attprcel", "FATAL:no moveable cells\n" ) ;
	    YexitPgm( PGMFAIL ) ;
	} 
	n = 12500 / moveable_cellsG ;
    } else {
	/*  n to the 4/3 power  */
	n = (INT)(25.0 * 
		    pow( (DOUBLE) moveable_cellsG / 500.0, 1.0 / 3.0 ) ) ;
    }

    if( flag && tw_fastG ) {
	n /= tw_fastG ;
    } else if( tw_slowG ) {
	n *= tw_slowG ;
    }

    return(n);
}

rm_overlapping_feeds()
{

INT row , cell , *rowptr , target ;
INT cells_in_row , index , num_deleted ;
IPBOXPTR imptr , save_ptr ;

num_deleted = 0 ;
for( row = 1 ; row <= numRowsG ; row++ ) {
    rowptr = pairArrayG[row] ;
    cells_in_row = pairArrayG[row][0] ;
    index = 0 ;
    while( ++index < cells_in_row ) {
	cell = rowptr[index] ;
	if( carrayG[cell]->corient <= 1 ) {
	    /*  delete the last feed  */
	    target = carrayG[cell]->tileptr->right ;
	} else {
	    /*  delete the first feed */
	    target = carrayG[cell]->tileptr->left ;
	}
        imptr = carrayG[cell]->imptr ;
	if( imptr != NULL ) {
	    if( imptr->txpos == target ) {
		save_ptr = imptr->next ;
		Ysafe_free( imptr ) ;
		num_deleted++ ;
		carrayG[cell]->imptr = save_ptr ;
	    } else {
		while( imptr->next != NULL ) {
		    if( imptr->next->txpos == target ) {
			save_ptr = imptr->next->next ;
			Ysafe_free( imptr->next ) ;
			num_deleted++ ;
			imptr->next = save_ptr ;
			break ;
		    }
		    imptr = imptr->next  ;
		}
	    } 
	}
    }
}
return( num_deleted ) ;
}




route_only_critical_nets() 
{

GLISTPTR pptr ;       /* pointer to paths of a cell */
GLISTPTR net_of_path ;
PATHPTR path ;

INT path_num , net , cell ;


for( net = 1 ; net <= numnetsG ; net++ ) {
    netarrayG[net]->ignore = -1 ;
}
for( cell = 1 ; cell <= numcellsG ; cell++ ) {
    for( pptr = carrayG[cell]->paths; pptr ; pptr = pptr->next ) {
	path_num = pptr->p.path ;
	path = patharrayG[path_num] ;
	for( net_of_path=path->nets;net_of_path;
				    net_of_path=net_of_path->next ){
	    netarrayG[net_of_path->p.net]->ignore = 0 ;
	}
    }
}
elim_nets(0) ;

return ;
}




elim_nets(print_flag)
INT print_flag ;
{

DBOXPTR dimptr;   /* bounding box for net */
PINBOXPTR netptr ;  /* traverse the pins on a net */
PINBOXPTR pinptr ;  /* traverse the pins on a cell */
PINBOXPTR freepin ; /* used to free the pin box record */
INT net , cell ;


for( net = 1 ; net <= numnetsG ; net++ ) {
    dimptr = netarrayG[net] ;
    if( (INT) dimptr->ignore == -1 ) {
	/*  we are not to global route this net 		     */
	/*  therefore we eliminate this net from the data structures */
	if( print_flag ) {
	    sprintf(YmsgG,"Global router is eliminating net: <%s>\n",
						netarrayG[net]->name ) ;
	    M( MSG, NULL, YmsgG ) ;
	}
	for( netptr = dimptr->pins ; netptr ; ) {
	    cell = netptr->cell ;
	    /* need to free all pins on netarray list and */
	    /* cell list of pins */
	    pinptr = carrayG[cell]->pins ;
	    if( pinptr == NULL ) {
		/* this should never occur WPS */
		sprintf( YmsgG, "cell:%d pinlist is null\n", cell ) ;
		M( ERRMSG, "elim_nets", YmsgG ) ;
		continue ;
	    }
	    /* look for matching pin in cell list of pins */
	    if( pinptr == netptr ) {
		/*  eliminate this pin on the cell  */
		carrayG[cell]->pins = pinptr->nextpin ;
	    } else {
		for( ; pinptr->nextpin ; pinptr = pinptr->nextpin ) {
		    if( pinptr->nextpin == netptr ) {
			/*  eliminate this pin on the cell  */
			pinptr->nextpin = pinptr->nextpin->nextpin;
			break ;
		    }
		}
	    }
	    freepin = netptr ;
	    netptr = netptr->next ;
	    Ysafe_free( freepin ) ;
	}
	dimptr->pins = NULL ;    /* null reference to list */
	dimptr->numpins = 0 ;

    }
}
return ;
}





refine_fixed_placement()
{

CBOXPTR cellptr , cellptr1 ;
PINBOXPTR termptr ;
INT *rowptr , *rowptr1 , *rowptr2 , **p_array , *p_rowptr ;
INT *spacer_list, *moveable_list, *next_limit, *next_index, *filled_to ;
INT slack, row, last_index, i, index, count , limit ;
INT cells_in_row, target, last_cell, ok, min, save_row ;
INT save_m, min_slack, m, all_l_fit, cell, first_index, prev_cell ;
INT success , filled_to_edge , target_edge , width , gap , shift ;
INT block, start, end, max_row_length , right_boundary ;
DOUBLE scale ;


next_limit = (INT *) Ysafe_malloc( (1 + numRowsG) * sizeof(INT) ) ;
next_index = (INT *) Ysafe_malloc( (1 + numRowsG) * sizeof(INT) ) ;
filled_to  = (INT *) Ysafe_malloc( (1 + numRowsG) * sizeof(INT) ) ;

/* create list of spacer cells */
spacer_list = (INT *) Ysafe_malloc( (1 + extra_cellsG) * sizeof(INT) ) ;
spacer_list[0] = extra_cellsG ;
count = numcellsG + 1 ;
for( i = 1 ; i <= extra_cellsG ; i++ ) {
    spacer_list[i] = --count ;
    carrayG[count]->clength  = spacer_widthG ;
    carrayG[count]->cycenter = GATE_ARRAY_MAGIC_CONSTANT ;
}


limit = 1 ;
for( row = 1 ; row <= numRowsG ; row++ ) {
    if( pairArrayG[row][0] > limit ) {
	limit = pairArrayG[row][0] ;
    }
}
limit += 1 + (extra_cellsG / numRowsG) * 4 ;

/* create temporary pairArray */
p_array = (INT **) Ysafe_malloc( ( numRowsG + 1 ) * sizeof(INT *) ) ;
for( row = 1 ; row <= numRowsG ; row++ ) {
    p_array[row] = (INT *) Ysafe_malloc( limit * sizeof( INT ) ) ;
}

/* begin the refinement process */
moveable_list = (INT *) Ysafe_malloc( (1 + numcellsG) * sizeof(INT) ) ;

/* for Dick Davis of DEC */
if( prop_rigid_cellsG ) {
    /* find longest row */    
    max_row_length = 0 ;
    for( block = 1 ; block <= numRowsG ; block++ ) {
	start = carrayG[ pairArrayG[block][1] ]->cxcenter +
			    carrayG[ pairArrayG[block][1] ]->tileptr->left ;
	end = carrayG[ pairArrayG[block][ pairArrayG[block][0] ] ]->cxcenter +
		carrayG[ pairArrayG[block][ pairArrayG[block][0] ] ]->tileptr->right ;
	if( max_row_length < end - start ) {
	    max_row_length = end - start ;
	}
    }
    scale = (DOUBLE) max_row_length / (DOUBLE) orig_max_row_lengthG ;
}

for( row = 1 ; row <= numRowsG ; row++ ) {
    moveable_list[0] = 0 ;

    /*  initialize p_array, the temporary pairArray  */
    cells_in_row = pairArrayG[row][0] ; 
    last_cell = pairArrayG[row][cells_in_row] ;
    right_boundary = carrayG[last_cell]->cxcenter + carrayG[last_cell]->tileptr->right ;
    if( right_boundary < barrayG[row]->bxcenter + barrayG[row]->bright ) {
	right_boundary = barrayG[row]->bxcenter + barrayG[row]->bright ;
    }
    for( i = 0 ; i <= cells_in_row ; i++ ) {
	p_array[row][i] = pairArrayG[row][i] ;
    }

    /* leave only fixed cells in p_array at the start of the process */
    rowptr = p_array[row] ; 
    cells_in_row = rowptr[0] ; 
    rowptr[0] = 0 ;
    for( i = 1 ; i <= cells_in_row ; i++ ) {
	cell = rowptr[i] ;
	cellptr = carrayG[cell] ;
	if( cellptr->cclass < 0 ) {  /* was "== -5" */
	    if( cellptr->border >= 0 ) {
		if( prop_rigid_cellsG ) {
		    target = (INT)( (DOUBLE) cellptr->border * scale ) ;
		    if( gate_arrayG ) {
			while( target % spacer_widthG != 0 ) {
			    target++ ;
			}
		    }
		    target += barrayG[row]->bxcenter + barrayG[row]->bleft ;
		} else {
		    target = cellptr->border + barrayG[row]->bxcenter +
					   barrayG[row]->bleft ;
		}
	    } else if( cellptr->border < -1 ) {
		target = cellptr->border + right_boundary - cellptr->clength ;
	    } else {
		target = right_boundary - cellptr->clength ;
	    }
	    /*
	    cellptr->cxcenter = target - cellptr->tileptr->left ;
	    */
	    shift = target - cellptr->tileptr->left - cellptr->cxcenter ;
	    cellptr->cxcenter += shift ;
	    /* ...update pin positions now..... */
	    for( termptr = cellptr->pins; termptr; 
					termptr = termptr->nextpin ) {
		termptr->xpos += shift ;
	    }
	    rowptr[ ++rowptr[0] ] = cell ;
	} else {
	    moveable_list[ ++moveable_list[0] ] = cell ;
	}
    }

    Yquicksort( (char *) ( rowptr + 1 ) , 
		    rowptr[0] , sizeof( INT ), comparex ) ;
    
    Yquicksort( (char *) ( moveable_list + 1 ) , 
		    moveable_list[0] , sizeof( INT ), comparex ) ;

    /* initialize filled_to[] and next_limit[]  */
    
    last_cell = p_array[row][0] ;
    filled_to[row] = barrayG[row]->bxcenter + barrayG[row]->bleft;
    i = 1 ;
    next_index[row] = 1 ; /* index yielding the next limit */
    while( i <= last_cell ) {
	cellptr = carrayG[ p_array[row][i] ] ;
	next_limit[row] = cellptr->cxcenter+cellptr->tileptr->left;
	if( next_limit[row] == filled_to[row] ) {
	    i++ ;
	    filled_to[row] += cellptr->clength ;
	    next_index[row]++ ;
	} else {
	    break ;
	}
    }
    if( i > last_cell ) {
	next_index[row] = -1 ;
	next_limit[row] = 10000000 ;
    }

    /* begin the placement of the moveable cells */
    while( moveable_list[0] > 0 ) {
	if( filled_to[row] == 10000000 ) {
	    break ;
	}
	/* min_slack = 20000000 ; */
	save_m = -1 ;
	/* all_l_fit = 1 ; */
	/*  look ahead by 1 cells in the moveable_list */
	for( m = 1 ; m <= 1 ; m++ ) {
	    if( m > moveable_list[0] ) {
		break ;
	    }
	    slack = next_limit[row] - (filled_to[row] + 
			    carrayG[ moveable_list[m] ]->clength) ; 
	    if( slack >= 0 ) {
		save_m = m ;
		break ;
	    }
	    /*
	    if( slack < 0 ) {
		all_l_fit = 0 ;
	    }
	    if( slack >= 0 && slack < min_slack ) {
		min_slack = slack ;
		save_m = m ;
	    }
	    */
	}
	/*
	if( all_l_fit && save_m > 0 ) {
	    save_m = 1 ;
	}
	*/
	if( save_m == -1 ) {  /* none fit */
	    if( next_index[row] != -1 ) {
		cellptr = carrayG[ p_array[row][ next_index[row] ] ];
		gap = next_limit[row] - filled_to[row] ;

		/*  New: 1 March 1990 by Carl  */
		if( cellptr->cclass != -5 ) {  /* let it slide a bit */
		    cellptr->cxcenter -= gap ;
		    /* ...update pin positions now..... */
		    for( termptr = cellptr->pins; termptr; 
					    termptr = termptr->nextpin ) {
			termptr->xpos -= gap ;
		    }

		}
		filled_to[row] = cellptr->cxcenter + 
				    cellptr->tileptr->right ;
		last_cell = p_array[row][0] ;
		i = ++next_index[row] ;
		while( i <= last_cell ) {
		    cellptr = carrayG[ p_array[row][i] ] ;
		    next_limit[row] = cellptr->cxcenter + 
					    cellptr->tileptr->left;
		    if( next_limit[row] == filled_to[row] ) {
			i++ ;
			filled_to[row] += cellptr->clength ;
			next_index[row]++ ;
		    } else {
			break ;
		    }
		}
		if( i > last_cell ) {
		    next_index[row] = -1 ;
		    next_limit[row] = 10000000 ;
		}
	    } else {
		/* can't add to this row and can't get another limit  */
		filled_to[row] = 10000000 ;
	    }
	} else {
	    /*  we can profitably add the cell to "row"  */
	    cellptr = carrayG[ moveable_list[save_m] ] ;
	    cellptr->cycenter = barrayG[row]->bycenter ;
	    cellptr->cblock   = row ;
	    if( next_index[row] != -1 ) {
		last_index = next_index[row]++ ;
		first_index = p_array[row][0] ;
		index = first_index ;
		for( ; index >= last_index ; index-- ) {
		    p_array[row][index+1] = p_array[row][index] ;
		}
		p_array[row][0]++ ;
		p_array[row][last_index] = moveable_list[save_m] ;
	    } else {
		last_index = ++p_array[row][0] ;
		p_array[row][last_index] = moveable_list[save_m] ;
	    }
	    if( last_index > 1 ) {
		prev_cell = p_array[row][last_index-1] ;
		/*
		cellptr->cxcenter = carrayG[prev_cell]->cxcenter +
				carrayG[prev_cell]->tileptr->right - 
				cellptr->tileptr->left ;
		*/
		shift = carrayG[prev_cell]->cxcenter +
			    carrayG[prev_cell]->tileptr->right - 
			    cellptr->tileptr->left - cellptr->cxcenter ;
		cellptr->cxcenter += shift ;
		/* ...update pin positions now..... */
		for( termptr = cellptr->pins; termptr; 
					termptr = termptr->nextpin ) {
		    termptr->xpos += shift ;
		}
	    } else {
		/*
		cellptr->cxcenter = barrayG[row]->bxcenter +
			    barrayG[row]->bleft - cellptr->tileptr->left;
		*/
		shift = barrayG[row]->bxcenter +
			    barrayG[row]->bleft - cellptr->tileptr->left -
						    cellptr->cxcenter ;
		cellptr->cxcenter += shift ;
		/* ...update pin positions now..... */
		for( termptr = cellptr->pins; termptr; 
					termptr = termptr->nextpin ) {
		    termptr->xpos += shift ;
		}
	    }
	    filled_to[row] += cellptr->clength ;

	    last_index = moveable_list[0] ;
	    for( index = save_m + 1 ; index <= last_index ; index++ ) {
		moveable_list[index-1] = moveable_list[index] ;
	    }
	    moveable_list[0]-- ;
	}
    }
}

/*  Now insert the spacers  */
for( row = 1 ; row <= numRowsG ; row++ ) {
    filled_to_edge = barrayG[row]->bxcenter + barrayG[row]->bleft ;
    p_rowptr = p_array[row] ;
    rowptr = pairArrayG[row] ;
    rowptr[0] = 0 ;
    last_index = p_rowptr[0] ;
    for( index = 1 ; index <= last_index ; index++ ) {
	cellptr1 = carrayG[ p_rowptr[index] ] ;
	width = cellptr1->clength ;
	target_edge = cellptr1->cxcenter + cellptr1->tileptr->left ;
	if( rigidly_fixed_cellsG ) {
	    while( filled_to_edge < target_edge ) {
		if( spacer_list[0] == 0 ) {
		    /* attempt fails */
		    return(0);
		}
		cell = spacer_list[ spacer_list[0]-- ] ;
		rowptr[++rowptr[0]] = cell ;
		carrayG[cell]->cycenter = barrayG[row]->bycenter ;
		carrayG[cell]->cblock   = row ;
		carrayG[cell]->cxcenter = filled_to_edge - carrayG[cell]->tileptr->left ;
		filled_to_edge += spacer_widthG ;
	    }
	}
	rowptr[++rowptr[0]] = p_rowptr[index] ;
	filled_to_edge += width ;
    }
}
sprintf(YmsgG,"\nActually added %d gate array spacers\n",
			extra_cellsG - spacer_list[0] ) ;
M( MSG, NULL, YmsgG ) ;
/* nullify the other nonused spacer cells */
last_index = spacer_list[0] ; 
for( index = 1 ; index <= last_index ; index++ ) {
    carrayG[ spacer_list[index] ]->clength = 0 ;
}


Ysafe_free( spacer_list ) ;
Ysafe_free( moveable_list ) ;
for( row = 1 ; row <= numRowsG ; row++ ) {
    Ysafe_free( p_array[row] ) ;
}
Ysafe_free( p_array ) ;
Ysafe_free( next_limit ) ;
Ysafe_free( next_index ) ;
Ysafe_free( filled_to  ) ;

return(1) ;
}
