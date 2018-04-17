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
FILE:	    debug.c                                       
DESCRIPTION:Various debug functions.
CONTENTS:   cellbox_data( first_cell , last_cell )
		INT first_cell , last_cell ;
	    cellterm_data( first_cell , last_cell )
		INT first_cell , last_cell ;
	    dbx_terminal( first_net , last_net )
		INT first_net , last_net ;
	    dbx_grd( ptr1 , ptr2 )
		CHANGRDPTR ptr1 , ptr2 ;
	    pairCheck( first_row , last_row )
		INT first_row , last_row ;
	    fcellheight( pin , fcell , status )
		INT pin , *fcell , status ;
	    dbx_track( start_chan , end_chan )
		INT start_chan , end_chan ;
DATE:	    Mar 27, 1989 
REVISIONS:  
----------------------------------------------------------------- */
#ifndef VMS
#ifndef lint
static char SccsId[] = "@(#) debug.c (Yale) version 4.5 9/7/90" ;
#endif
#endif

#include "standard.h"
#include "groute.h"
#include "pads.h"

cellbox_data( first_cell , last_cell )
INT first_cell , last_cell ;
{

FILE *fp ;
CBOXPTR ptr ;
INT cell ;
INT padside ;

fp = TWOPEN( "cellbox.dat" , "w", ABORT ) ;

fprintf(fp," cell xcenter ycenter corient padside  row class\n" ) ;
for( cell = first_cell ; cell <= last_cell ; cell++ ) {
    ptr = carrayG[ cell ] ;
    if( ptr->padptr ){
	padside = ptr->padptr->padside ;
    } else {
	padside = 0 ;
    }
    fprintf( fp , "%5d  %6d  %6d      %2d      %2d %4d  %4d\n" ,
	cell , ptr->cxcenter , ptr->cycenter , ptr->corient ,
	padside , ptr->cblock , ptr->cclass ) ;
}

TWCLOSE( fp ) ;

}


cellterm_data( first_cell , last_cell )
INT first_cell , last_cell ;
{

FILE *fp ;
CBOXPTR ptr ;
PINBOXPTR termptr ;
INT cell , corient, padside ;

fp = TWOPEN( "cellterm.dat" , "w", ABORT ) ;

for( cell = first_cell ; cell <= last_cell ; cell++ ) {
    fprintf(fp,"\n cell xcenter ycenter corient padside left right\n");
    ptr = carrayG[ cell ] ;
    if( ptr->padptr ){
	padside = ptr->padptr->padside ;
    } else {
	padside = 0 ;
    }
    corient = ptr->corient ;

    fprintf( fp , "%5d  %6d  %6d      %2d      %2d %4d  %4d\n" ,
	cell , ptr->cxcenter , ptr->cycenter , corient ,
	padside , ptr->tileptr->left , ptr->tileptr->right ) ;
    fprintf(fp,"   pin  net   tx   ty   xpos   ypos   newx   newy ") ;
    fprintf(fp,"loc flag\n" ) ;
    if( cell <= numcellsG ) {
	for( termptr = ptr->pins ;termptr;termptr = termptr->nextpin ) {
	    fprintf(fp," %5d %4d %4d %4d %6d %6d %6d %6d %3d %2d\n"
		, termptr->terminal , termptr->net , 
		termptr->txpos[ corient/2 ] ,
		termptr->typos[ corient%2 ] , termptr->xpos ,
		termptr->ypos , termptr->newx , termptr->newy ,
		termptr->pinloc , termptr->flag );
	}
    } else {
	for( termptr = ptr->pins; termptr; termptr = termptr->next ){
	    fprintf(fp," %5d %4d %4d %4d %6d %6d %6d %6d %3d %2d\n"
		, termptr->terminal , termptr->net ,termptr->txpos[1]
		, termptr->typos[1] , termptr->xpos ,
		termptr->ypos , termptr->newx , termptr->newy ,
		termptr->pinloc , termptr->flag );
	}
    }
}
    
TWCLOSE( fp ) ;
}


dbx_terminal( first_net , last_net )
INT first_net , last_net ;
{

FILE *fp ;
PINBOXPTR netptr ;
INT net ;

fp = TWOPEN( "netbox.dat" , "w", ABORT ) ;

for( net = first_net ; net <= last_net ; net++ ) {
    fprintf(fp,"net %d\n" , net ) ;
    fprintf(fp," terminal  pinname  xpos  cell loc row \n");
    for( netptr = netarrayG[net]->pins ; netptr ; netptr = netptr->next ) {
	fprintf(fp,"    %5d %8s %5d %5d %3d %2d\n" ,
	    netptr->terminal , netptr->pinname , netptr->xpos ,
	    netptr->cell , netptr->pinloc, netptr->row ) ;
    }
}
    
TWCLOSE( fp ) ;
}


dbx_grd( ptr1 , ptr2 )
CHANGRDPTR ptr1 , ptr2 ;
{

CHANGRDPTR ptr ;
INT x2 , flag = 0 ;

x2 = ptr2->netptr->xpos ;
for( ptr = ptr1 ; ptr->netptr->xpos <= x2 ; ptr = ptr->nextgrd ) {
    if( ptr == ptr2 ) {
	flag = 1 ;
    }
}
if( flag == 1 ) {
    printf(" they are in the same linked list of channel grid\n" ) ;
} else {
    printf(" they are not in the same linked list of channel grid\n" ) ;
}
}

pairCheck( first_row , last_row )
INT first_row , last_row ;
{

FILE *fp ;
CBOXPTR cellptr ;
TIBOXPTR tileptr ;
INT row , i , *Aray , cell ;
INT most_left , most_rite , n ;
INT curr_cell_left , curr_cell_rite , prev_cell_rite ;
extern INT fdWidthG ;

fp = TWOPEN( "row.dat" , "w", ABORT ) ;
/*
fprintf(fp,"cedge_binwidth = %d num_edgebin = %d\n",
			cedge_binwidth, num_edgebin ) ;
*/

for( row = first_row ; row <= last_row ; row++ ) {
    Aray = pairArrayG[row] ;
    fprintf(fp,"\nROW %d\n" , row ) ;
    fprintf(fp," total number of cells in this row = %d\n",Aray[0] ) ;
    cellptr = carrayG[ Aray[1] ] ;
    most_left = cellptr->cxcenter + cellptr->tileptr->left ;
    cellptr = carrayG[ Aray[Aray[0]] ] ;
    most_rite = cellptr->cxcenter + cellptr->tileptr->right ;
    fprintf(fp," most_left is at %d most_rite is at %d\n", 
				    most_left , most_rite ) ;
    fprintf(fp,"   i  cell  left right   n\n" ) ;
    prev_cell_rite = carrayG[ Aray[1] ]->cxcenter +
		     carrayG[ Aray[1] ]->tileptr->left ;
    for( i = 1 ; i <= Aray[0] ; i++ ) {
	cell = Aray[i] ;
	cellptr = carrayG[ cell ] ;
	tileptr = cellptr->tileptr ;
	curr_cell_left = cellptr->cxcenter + tileptr->left ;
	curr_cell_rite = cellptr->cxcenter + tileptr->right ;
	n = ( curr_cell_left - prev_cell_rite ) / fdWidthG ;
	prev_cell_rite = curr_cell_rite ;
	fprintf(fp," %3d %5d %5d %5d %3d\n",
	    i , cell , curr_cell_left , curr_cell_rite , n ) ;
    }
}
TWCLOSE( fp ) ;
}




dbx_track( start_chan , end_chan )
INT start_chan , end_chan ;
{
INT chan ;
FILE *fp ;
CHANGRDPTR gdptr ;
PINBOXPTR netptr ;


fp = TWOPEN( "track.dat" , "w", ABORT ) ;
for( chan = start_chan ; chan <= end_chan ; chan++ ) {
fprintf(fp,"\n channel = %d\n" , chan ) ;
fprintf(fp,"\n terminal   xpos track   net cell\n");
for( gdptr = BeginG[chan] ; gdptr ; gdptr = gdptr->nextgrd ) {
    netptr = gdptr->netptr ;
    fprintf(fp ,"%9d %6d %5d %5d %5d \n" 
    , netptr->terminal , netptr->xpos , gdptr->tracks ,
    netptr->net , netptr->cell ) ;
}
}
TWCLOSE( fp ) ;
}
