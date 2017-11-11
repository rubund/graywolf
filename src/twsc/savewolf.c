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
FILE:	    savewolf.c                                       
DESCRIPTION:checkpoint file routines.
CONTENTS:   savewolf(flag)
		int flag ;
	    TW_oldin( fp )
		FILE *fp ;
DATE:	    Mar 27, 1989 
REVISIONS:  Wed Mar 13 13:48:30 CST 1991 - make sure the .blk
		and the .res file are consistent.
	    Thu Sep 19 16:36:02 EDT 1991 - added more error checking.
----------------------------------------------------------------- */
#include <globals.h>
#include "allheaders.h"

#define INTSCANSTR "%d"

void savewolf(int flag)
{

FILE *fp ;
int xcenter , ycenter ;
int cell , block , orient ;
char filename[64] ;
char file1[1024], file2[1024];
CBOXPTR cellptr ;
double Ycpu_time();
double current_time;
static double last_time = 0.0;

if( !flag ) {
    /* make sure another checkpoint is worthwhile !! */
    current_time = Ycpu_time();
    if ((current_time - last_time) < 900.0 ) {
        return;
    }
    last_time = current_time;
}

sprintf( filename , "%s.tmp" , cktNameG ) ;
fp = TWOPEN( filename , "w", ABORT ) ;

fprintf( fp, "%f\n", TG ) ;
fprintf( fp, "%d\n", iterationG ) ;
fprintf( fp, "%d\n", ffeedsG ) ;
fprintf( fp, "1\n" ) ;

fprintf( fp, "%f\n", binpenConG ) ;
fprintf( fp, "%f\n", roLenConG ) ;
/* was cost_scale_factor added for backward compatibility */
fprintf( fp, "%f\n", 1.0 ) ;
fprintf( fp, "%d\n", estimate_feedsG ) ;
save_feeds( fp ) ;
save_control( fp ) ;


for( cell = 1 ; cell <= lastpadG ; cell++ ) {
    cellptr = carrayG[ cell ] ;
    orient  = cellptr->corient   ;
    block   = cellptr->cblock    ;
    xcenter = cellptr->cxcenter  ;
    ycenter = cellptr->cycenter  ;
    fprintf( fp , "%d %d %d %d %d\n", cell , block , 
			       orient , xcenter , ycenter ) ;
}
TWCLOSE( fp ) ;
#ifndef VMS
sprintf(file1, "%s.tmp", cktNameG);
sprintf(file2, "%s.sav", cktNameG);
rename(file1, file2);
#endif
return ;
}

void TW_oldin( FILE *fp )
{

int cell , orient , numfds ;
int block , xcenter , ycenter , dummy ;
int last_cell ;
int number_of_core_cells ;
double cost_scale_factor ;
CBOXPTR ptr ;
PINBOXPTR pinptr ;


fscanf( fp , "%lf" , &TG ) ;
fscanf( fp , INTSCANSTR , &iterationG ) ;
fscanf( fp , INTSCANSTR , &numfds ) ;
fscanf( fp , INTSCANSTR , &dummy ) ;

fscanf( fp , "%lf" , &binpenConG ) ;
fscanf( fp , "%lf" , &roLenConG ) ;
/* no longer use cost_scale_factor but save for backwards compatibility */
fscanf( fp , "%lf" , &cost_scale_factor ) ;
fscanf( fp , INTSCANSTR , &estimate_feedsG ) ;
read_feeds( fp ) ;

read_control( fp ) ;

/* add error checking to catch problems with different number of cells */
last_cell = 0 ;
number_of_core_cells = numcellsG - extra_cellsG ;
/* ignore the spacer cells */
while( fscanf( fp , " " INTSCANSTR " " INTSCANSTR " " INTSCANSTR
		" " INTSCANSTR " " INTSCANSTR " ",
		&cell , &block , &orient , &xcenter , &ycenter ) == 5 ) {
    ptr = carrayG[ cell ] ;
    if( cell <= number_of_core_cells ) {
	/* check to make sure block is valid */
	if( block < 1 || block > numRowsG ){
	    sprintf( YmsgG, "block out of bounds for cell:%s\n",
	    ptr->cname ) ;
	    M( ERRMSG, "TW_oldin", YmsgG ) ;
	    M( ERRMSG, NULL,"Restart file does not match block file\n");
	    M( ERRMSG, NULL, "Must exit\n\n" ) ;
	    TWCLOSE( fpoG ) ;
	    closegraphics() ;
	    YexitPgm(PGMFAIL) ;
	}
	ptr->cblock   = block   ;
	ptr->corient  = orient  ;
	ptr->cxcenter = xcenter ;
	ptr->cycenter = ycenter ;
	for( pinptr = ptr->pins; pinptr ; pinptr = pinptr->nextpin ) {
	    pinptr->xpos = pinptr->txpos[ orient/2 ] + xcenter ;
	    pinptr->ypos = pinptr->typos[ orient%2 ] + ycenter ;
	}
	last_cell = cell ;
    }
    if( strcmp( ptr->cname, "GATE_ARRAY_SPACER" ) == STRINGEQ ){
	/* no need to process spacer cells or pads */
	break ;
    }
}

/* now check to make sure we have the same number of cells */
if( last_cell != number_of_core_cells ){
    sprintf( YmsgG, "Wrong number of cells in restart file:%d vs %d in cell file.\n",
    last_cell, number_of_core_cells ) ;
    M( ERRMSG, "TW_oldin", YmsgG ) ;
    M( ERRMSG, NULL, "Fatal Error. Must exit\n\n" ) ;
    TWCLOSE( fpoG ) ;
    closegraphics() ;
    YexitPgm(PGMFAIL) ;
}
return ;
} /* end TW_oldin */
