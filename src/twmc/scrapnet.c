/* ----------------------------------------------------------------- 
FILE:	    scrapnet.c                                       
DESCRIPTION:ignore a net during annealing if it has too many connections
DATE:	    Feb  7, 1990 
REVISIONS:  
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) scrapnet.c version 3.3 9/5/90" ;
#endif

#include <custom.h>
#include <yalecad/debug.h>

scrapnet()
{

    NETBOXPTR dimptr ;
    PINBOXPTR netptr ;
    INT *vector , criticalMass , count , temp , net , cell ;
    INT average_cells_per_net ;
    INT num_cell_for_net ;

    vector = (INT *) Ysafe_malloc( (1 + totalcellsG) * sizeof(INT) ) ;
    temp = (INT) ( 0.70 * (DOUBLE) numcellsG ) ;
    criticalMass = ( temp > 8 ) ? temp : 8 ;

    /* first calculate the average number of cells net is connected */
    /* in order to readjust critical mass in strange situations */
    num_cell_for_net = 0 ;
    for( net = 1 ; net <= numnetsG ; net++ ) {
	for( cell = 1 ; cell <= totalcellsG ; cell++ ) {
	    vector[cell] = 0 ;
	}
	count = 0 ;
	dimptr = netarrayG[net] ;
	for( netptr = dimptr->pins ; netptr ; netptr = netptr->next ) {
	    cell = netptr->cell ;
	    if( vector[cell] == 0 ) {
		vector[cell] = 1 ;
		count++ ;
	    }
	}
	num_cell_for_net += count ;
    }

    average_cells_per_net = 
	(INT) ceil( (DOUBLE)num_cell_for_net / (DOUBLE) numnetsG ) ;

    /* critical mass must at least be the average */
    criticalMass = MAX( criticalMass, average_cells_per_net ) ;

    /* now it safe to discard nets */
    for( net = 1 ; net <= numnetsG ; net++ ) {
	for( cell = 1 ; cell <= numcellsG ; cell++ ) {
	    vector[cell] = 0 ;
	}
	count = 0 ;
	dimptr = netarrayG[net] ;
	for( netptr = dimptr->pins ; netptr ; netptr = netptr->next ) {
	    cell = netptr->cell ;
	    if( cell <= numcellsG ) {
		if( vector[cell] == 0 ) {
		    vector[cell] = 1 ;
		    count++ ;
		}
	    }
	}
	if( count > criticalMass ) {
	    dimptr->skip = 1 ;
	}
    }
    Ysafe_free( vector ) ;

    return ;
}
