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
FILE:	    outcm.c                                       
DESCRIPTION:input/output to connection machine.
CONTENTS:   outcm()
	    incm(fp)
		FILE *fp ;
DATE:	    Mar 27, 1989 
REVISIONS:  
----------------------------------------------------------------- */
#include <globals.h>
#include "allheaders.h"

#if SIZEOF_VOID_P == 64
#define intSCANSTR "%ld"
#else
#define intSCANSTR "%d"
#endif

void outcm()
{

char filename[64] ;
FILE *fp ;
PINBOXPTR netptr ;
int net , x , local_pin_count , pin_count ;
int cell , row , length , xstart ;


sprintf( filename , "%s.cm" , cktNameG ) ;
fp = TWOPEN ( filename , "w", ABORT ) ;

pin_count = 0 ;
for( net = 1 ; net <= numnetsG ; net++ ) {
    local_pin_count = 0 ;
    for( netptr = netarrayG[net]->pins; netptr; netptr = netptr->next){
	if( netptr->cell <= numcellsG ) {
	    local_pin_count++ ;
	}
    }
    if( local_pin_count > 1 ) {
	pin_count += local_pin_count ;
    }
}
fprintf(fp,"%d\n", pin_count ) ;

for( net = 1 ; net <= numnetsG ; net++ ) {
    local_pin_count = 0 ;
    for( netptr = netarrayG[net]->pins; netptr; netptr = netptr->next){
	if( netptr->cell <= numcellsG ) {
	    local_pin_count++ ;
	}
    }
    if( local_pin_count <= 1 ) {
	continue ;
    }
    for( netptr = netarrayG[net]->pins; netptr; netptr = netptr->next){
	if( netptr->cell > numcellsG ) {
	    continue ;
	}
	cell = netptr->cell ;
	row  = carrayG[cell]->cblock ;
	x    = netptr->xpos ;
	length = carrayG[cell]->tileptr->right - 
					carrayG[cell]->tileptr->left ;
	xstart = carrayG[cell]->cxcenter + carrayG[cell]->tileptr->left ;

	fprintf(fp,"%8d %8d %8d %8d %8d %8d\n", row, cell, net,
						 x, length, xstart );
    }
}
TWCLOSE(fp);

return ;
}

void incm(FILE *fp)
{

CBOXPTR ptr ;
int net , x , pin_count , pin ;
int cell , row , length , xstart ;


fscanf(fp, intSCANSTR, &pin_count ) ;

for( pin = 1 ; pin <= pin_count ; pin++ ) {
    fscanf(fp, intSCANSTR " " intSCANSTR " " intSCANSTR " "
		intSCANSTR " " intSCANSTR " " intSCANSTR,
		&row, &cell, &net, &x, &length, &xstart );
    ptr = carrayG[cell] ;
    ptr->cblock = row ;
    ptr->cxcenter = xstart + length / 2 ;
    ptr->cycenter = barrayG[row]->bycenter ;
}
TWCLOSE(fp);

return ;
}
