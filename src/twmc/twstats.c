/*
 *   Copyright (C) 1988-1990 Yale University
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
FILE:	    twstats.c                                       
DESCRIPTION:user statistic information
CONTENTS:   twstats( )
	    prnt_netinfo( )
DATE:	    Feb 29, 1988 
REVISIONS:  Jan 30, 1989 - added number of net info at beginning of run.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) twstats.c version 3.3 9/5/90" ;
#endif

#include <custom.h>
#include <yalecad/debug.h>

#define MAXPININFO 100
static INT maxpinS = 0 ;


twstats()
{
    INT temp ;
    DOUBLE reduction ;

OUT3("\nInitial Wiring Cost: %d   Final Wiring Cost: %d\n",
						icostG , fcostG ) ;
if( icostG != 0 ) {
    temp = 100 - (INT)( (DOUBLE)fcostG / (DOUBLE)icostG * 100.0 ) ;
    OUT2("############ Percent Wire Cost Reduction: %d\n\n", temp ) ;
}

OUT3("\nInitial Wire Length: %d   Final Wire Length: %d\n",
					    iwireG, fwireG ) ;
if( icostG != 0 ) {
    temp = 100 - (INT)( (DOUBLE) fwireG / (DOUBLE) iwireG * 100.0 ) ;
    OUT2("*********** Percent Wire Length Reduction: %d\n\n", temp ) ;
}

OUT3("\nInitial Horiz. Wire: %d   Final Horiz. Wire: %d\n",
					    iwirexG , fwirexG ) ;
if( iwirexG != 0 ) {
    temp = 100 - (INT)( (DOUBLE)fwirexG / (DOUBLE)iwirexG * 100.0 ) ;
    OUT2("$$$$$$$$$ Percent H-Wire Length Reduction: %d\n\n", temp ) ;
}
OUT3("\nInitial Vert. Wire: %d   Final Vert. Wire: %d\n",
					    iwireyG , fwireyG ) ;
if( iwireyG != 0 ) {
    temp = 100 - (INT)( (DOUBLE)fwireyG / (DOUBLE)iwireyG * 100.0 ) ;
    OUT2("@@@@@@@@@ Percent V-Wire Length Reduction: %d\n\n", temp ) ;
}

OUT1("\nStatistics:\n");
OUT2("Number of Cells: %d\n", numcellsG );
OUT2("Number of Pads: %d\n", numpadsG );
OUT2("Number of Nets: %d \n", numnetsG ) ;
OUT2("Number of Pins: %d \n", numpinsG ) ;

/* write wire reduction to log file */
if( avg_funcG > 0 ){
    reduction = (DOUBLE) avg_funcG / (DOUBLE) fwireG ;
    sprintf( YmsgG,
	"TimberWolfMC reduction:%4.4le", reduction ) ;
    Ylog_start( cktNameG, YmsgG ) ;
}

return ;
}

static INT printPinS = 0 ;

set_print_pin( pin )
INT pin ;
{
    printPinS = pin ;
}

prnt_netinfo() 
{

SHORT numpins ;
INT net_pin_num[ MAXPININFO+1 ] ;
INT n, net, cell ;
NETBOXPTR dimptr ;
PINBOXPTR termptr ;

if( printPinS ){
    OUT2("\n\nPIN LISTING MODE ON FOR NETS WITH %d PINS\n", printPinS ) ; 
    OUT1(    "-----------------------------------------\n" ) ; 
}

for( n = 1 ; n <= MAXPININFO ; n++ ) {
    net_pin_num[ n ] = 0 ;
}
for( net = 1 ; net <= numnetsG ; net++ ) {
    dimptr =  netarrayG[ net ] ;
    /* count the number of pins on a net */
    numpins = 0 ;
    for( termptr=dimptr->pins;termptr;termptr=termptr->next){
	numpins++ ;
    }
    dimptr->numpins = numpins ;
    /* save the result */

    if( printPinS ){
	if( numpins == printPinS ){
	    OUT4( "A net with %d pin[s]:net%d - %s\n" , 
		printPinS, net, netarrayG[net]->nname ) ;
	    for( termptr=dimptr->pins;termptr;termptr=termptr->next){
		cell = termptr->cell ;
		OUT4( "\tpinname:%s cellname:%s cellnum:%d\n",
		    termptr->pinname, cellarrayG[cell]->cname, cell ) ;
	    }
	}
    }

    /* record the maximum number of pins for later use */
    maxpinS = MAX( maxpinS, numpins ) ;

    if( numpins > MAXPININFO ) {
	net_pin_num[ MAXPININFO ]++ ;
    } else {
	net_pin_num[ numpins ]++ ;
    }
}
for( n = 1 ; n < MAXPININFO ; n++ ) {
    if( net_pin_num[ n ] ){
	OUT3( "The number of nets with %2d pins is %d\n",
		 n , net_pin_num[ n ] ) ;
    }
}
OUT3( "The number of nets with %d pins or more is %d\n",
	      MAXPININFO,net_pin_num[ MAXPININFO ] ) ;
OUT2( "The total number of nets is: %d\n", numnetsG ) ;
OUT2( "The maximum number of pins on a single net is: %d\n", maxpinS ) ;

} /* end prnt_netinfo */

get_max_pin()
{
    return( maxpinS ) ;
} /* end get_max_pin */
