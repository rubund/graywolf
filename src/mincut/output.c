/*
 *   Copyright (C) 1989-1991 Yale University
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
FILE:	    output.c
DESCRIPTION:This file accumulates data for TimberWolf system.
CONTENTS:   
DATE:	    Apr 24, 1989 
REVISIONS:  May 24, 1989 - updated argument to YcurTime.
	    May 15, 1990 - now add up cell area so that standard cell
		macros size will be the average macro size.
	    Aug 3,  1990 - added port counting to output.
	    Thu Apr 18 01:12:20 EDT 1991 - added more error checking.
	    Wed Jun 12 13:40:11 CDT 1991 - added check for pins
		outside cell boundary.
	    Wed Jul 24 21:06:02 CDT 1991 - added a more
		meaningful output error message.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) output.c version 1.1 7/30/91" ;
#endif

#include <string.h>
#include "globals.h"
#include <yalecad/base.h>
#include <yalecad/message.h>
#include <yalecad/hash.h>
#include <yalecad/yreadpar.h>

#define EXPECTEDNUMNETS         10009

typedef struct {
    BOOL io_signal ;
    char *net ;
} NETBOX, *NETPTR ;

static int objectS = 0 ;       /* number of objects read */
static int celltypeS ;         /* current cell type */
static DOUBLE total_cell_lenS = 0.0 ;
static DOUBLE total_cell_heightS = 0.0;
static DOUBLE total_areaS = 0.0 ;
static DOUBLE core_areaS = 0.0 ;
static DOUBLE average_cell_heightS ;
static DOUBLE row_sepS ;
static DOUBLE row_sep_absS ;
static int total_std_cellS = 0 ;
static char current_cellS[LRECL] ; /* the current cell name */
static char cur_pinnameS[LRECL] ;  /* current pinname */
static YHASHPTR netTableS ;    /* hash table for cross referencing nets */
/* *************************************************************** */
init()
{
    /* get ready for parsing */
    /* make hash table for nets */
    netTableS = Yhash_table_create( EXPECTEDNUMNETS ) ;
} /* end init */

addCell( celltype, cellname )
int celltype ;
char *cellname ;
{

    strcpy( current_cellS, cellname ) ;
    Ysafe_free( cellname ) ;
    /* passify the user */
    if( (++objectS % 50) == 0 ){
	sprintf( YmsgG, "Read %4d objects so far...\n", objectS ) ;
	M( MSG, NULL, YmsgG ) ;
    }
    celltypeS = celltype ;   /* save for determining instances etc. */

} /* end addCell */

addNet( signal )
char *signal ;
{
    NETPTR data ;

    if( strcmp( signal, "TW_PASS_THRU" ) == STRINGEQ ){
	return ; /* not a net so return */
    }
    if( data = (NETPTR) Yhash_search( netTableS, signal, NULL, FIND )){
	/* this net now makes an io connection mark it as such */
	switch( celltypeS ){
	case HARDCELLTYPE:
	case SOFTCELLTYPE:
	case PADCELLTYPE:
	    data->io_signal = TRUE ;
	    break ;
	case STDCELLTYPE:
	    break ;
	}
    } else {
	/* else a new net - load data holder */
	data = (NETPTR) Ysafe_malloc( sizeof(NETBOX) ) ;
	data->net = signal ;
	switch( celltypeS ){
	case HARDCELLTYPE:
	case SOFTCELLTYPE:
	case PADCELLTYPE:
	    data->io_signal = TRUE ;
	    break ;
	case STDCELLTYPE:
	    data->io_signal = FALSE ;
	    break ;
	}
	if( Yhash_search( netTableS, signal, (char *)data, ENTER )){
	    sprintf( YmsgG, "Trouble adding signal:%s to hash table\n",
		signal ) ;
	    M(ERRMSG,"addNet",YmsgG ) ;
	}
    }
} /* end addNet */

set_bbox( left, right, bottom, top )
INT left, right, bottom, top ;
{
    DOUBLE width, height ;

    width = (DOUBLE) (right - left) ;
    total_cell_lenS += width ;
    height = (DOUBLE) (top - bottom) ;
    total_cell_heightS += height ;
    total_areaS += width * height ;
    core_areaS += width * (height + row_sep_absS) ;
    total_std_cellS++ ;
} /* end set_bbox */

output( fp )
FILE *fp ;
{
    INT g ;

    if( total_std_cellS > 0 ){
	average_cell_heightS = total_cell_heightS / 
	    (DOUBLE) total_std_cellS ;
    } else {
	average_cell_heightS = 0.0 ;
    }
    core_areaS *= (row_sepS + 1.0) ;

    fprintf( stderr, "\n----------------------------\n" ) ;
    fprintf( stderr, "Total stdcells     :%d\n", total_std_cellS ) ;
    fprintf( stderr, "Total cell width   :%4.2le\n", total_cell_lenS ) ;
    fprintf( stderr, "Total cell height  :%4.2le\n", total_cell_heightS ) ;
    fprintf( stderr, "Total cell area    :%4.2le\n", total_areaS ) ;
    fprintf( stderr, "Total core area    :%4.2le\n", core_areaS ) ;
    fprintf( stderr, "Average cell height:%4.2le\n\n",
	average_cell_heightS ) ;


    /* the first instance take as a rectangle - initially a square */
    g = (INT) sqrt( core_areaS ) ;
    fprintf( fp, "cluster 1 name core\n" ) ;
    fprintf( fp, "corners 4 0 0   0 %d  %d %d   %d 0\n", g, g, g, g ) ;
    write_softpins( fp ) ;

    /* for the second instance use an L shape */
    g = (INT) sqrt( core_areaS / 3.0 ) ;
    if( g > 2 ){
	fprintf( fp, "instance core_L\n" ) ;
	fprintf( fp, "corners 6 " ) ;
	fprintf( fp, "0 0 " ) ;
	fprintf( fp, "0 %d ", 2*g ) ;
	fprintf( fp, "%d %d ", g, 2*g ) ;
	fprintf( fp, "%d %d ", g, g ) ;
	fprintf( fp, "%d %d ", 2*g, g ) ;
	fprintf( fp, "%d 0\n", 2*g ) ;
	write_softpins( fp ) ;
    }
    
    /* for the third instance use a T shape */
    g = (INT) sqrt( core_areaS / 4.0 ) ;
    if( g > 2 ){
	fprintf( fp, "instance core_T\n" ) ;
	fprintf( fp, "corners 8 " ) ;
	fprintf( fp, "%d 0 ", g ) ;
	fprintf( fp, "%d %d ", g, g ) ;
	fprintf( fp, "0 %d ", g ) ;
	fprintf( fp, "0 %d ", 2*g ) ;
	fprintf( fp, "%d %d ", 3*g, 2*g ) ;
	fprintf( fp, "%d %d ", 3*g, g ) ;
	fprintf( fp, "%d %d ", 2*g, g ) ;
	fprintf( fp, "%d 0\n", 2*g ) ;
	write_softpins( fp ) ;
    }

#ifdef USHAPE
    /* for the third instance use a U shape */
    g = (INT) sqrt( core_areaS / 5.0 ) ;
    if( g > 2 ){
	fprintf( fp, "instance core_U\n" ) ;
	fprintf( fp, "corners 8 " ) ;
	fprintf( fp, "0 0 " ) ;
	fprintf( fp, "0 %d ", 2*g ) ;
	fprintf( fp, "%d %d ", g, 2*g ) ;
	fprintf( fp, "%d %d ", g, g ) ;
	fprintf( fp, "%d %d ", 2*g, g ) ;
	fprintf( fp, "%d %d ", 2*g, 2*g ) ;
	fprintf( fp, "%d %d ", 3*g, 2*g ) ;
	fprintf( fp, "%d 0\n", 3*g ) ;
	write_softpins( fp ) ;
    }
#endif

    /* for the fourth instance use a modified L shape */
    g = (INT) sqrt( core_areaS / 5.0 ) ;
    if( g > 2 ){
	fprintf( fp, "instance core_L2\n" ) ;
	fprintf( fp, "corners 6 " ) ;
	fprintf( fp, "0 0 " ) ;
	fprintf( fp, "0 %d ", 2*g ) ;
	fprintf( fp, "%d %d ", 2*g, 2*g ) ;
	fprintf( fp, "%d %d ", 2*g, g ) ;
	fprintf( fp, "%d %d ", 3*g, g ) ;
	fprintf( fp, "%d 0\n", 3*g ) ;
	write_softpins( fp ) ;
    }

} /* end output */

write_softpins( fp )
FILE *fp ;
{
    YTABLEPTR thread ;
    NETPTR net ;
    int pin_count ;

    fprintf( fp, "asplb 0.5 aspub 2.0\n" ) ;
    fprintf( fp, "class 0 orientations 0 1 2 3 4 5 6 7\n") ;

    pin_count = 0 ;
    for( thread=netTableS->thread;thread;thread=thread->threadNext){
	net = (NETPTR) thread->data ;
	if( net->io_signal ){
	    fprintf( fp, "softpin name pin%d signal %s\n", 
		++pin_count, net->net ) ;
	    
	}
    }
    fprintf( fp, "\n" ) ;
} /* end write_softpins */

read_par()
{
    char input[LRECL] ;
    char *bufferptr ;
    char **tokens ;
    INT  numtokens ;
    INT  line ;
    BOOL onNotOff ;
    BOOL wildcard ;
    BOOL found ;

    found = FALSE ;
    Yreadpar_init( cktNameG, USER, TWSC, TRUE ) ;
    while( tokens = Yreadpar_next( &bufferptr, &line, &numtokens, 
	&onNotOff, &wildcard )){
	if( numtokens == 0 ){
	    /* skip over empty lines */
	    continue ;
	}
	if ((numtokens != 2) && (numtokens != 3)) {
	    continue ;
	}
	if( strcmp( tokens[0], "rowSep" ) == STRINGEQ ){
	    row_sepS = atof( tokens[1] ) ;
	    if (numtokens == 3)
		row_sep_absS = atof( tokens[2] ) ;
	    found = TRUE ;
	}
    }
    if(!(found)){
	M(WARNMSG, "read_par", "Couldn't find rowsep in parameter file\n" ) ;
	M(WARNMSG, NULL,"Using default of 1.0\n" ) ; 
	row_sepS = 1.0 ;
	row_sep_absS = 0.0 ;
    }
} /* end readpar */

update_stats( fp )
FILE *fp ;
{
    fprintf( fp, "tot_length:%d\n", (INT)total_cell_lenS);
    fprintf( fp, "num_soft:1\n" ) ;
    fprintf( fp, "cell_height:%d\n", (INT) average_cell_heightS);
} /* end update_stats */
