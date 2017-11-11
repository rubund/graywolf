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
FILE:	    clean.c                                       
DESCRIPTION:overlap calculations.
CONTENTS:   new_old( c )
		int c ;
	    old_assgnto_new1( alobin , ahibin , anewlobin , anewhibin )
		int alobin , ahibin , anewlobin , anewhibin ;
	    new_assgnto_old1( alobin , ahibin , anewlobin , anewhibin )
		int alobin , ahibin , anewlobin , anewhibin ;
	    old_assgnto_new2( a1lobin , a1hibin , a2lobin , a2hibin ,
			      b1lobin , b1hibin , b2lobin , b2hibin )
		int a1lobin , a1hibin , a2lobin , a2hibin ;
		int b1lobin , b1hibin , b2lobin , b2hibin ;
	    new_assgnto_old2( a1lobin , a1hibin , a2lobin , a2hibin ,
			      b1lobin , b1hibin , b2lobin , b2hibin )
		int a1lobin , a1hibin , a2lobin , a2hibin ;
		int b1lobin , b1hibin , b2lobin , b2hibin ;
	    sub_penal( startx , endx , block , LoBin , HiBin )
		int startx , endx , block , LoBin , HiBin ;
	    add_penal( startx , endx , block , LoBin , HiBin )
		int startx , endx , block , LoBin , HiBin ;
	    term_newpos( antrmptr , xcenter , ycenter , newaor )
		TEBOXPTR antrmptr ;
		int xcenter , ycenter , newaor ;
DATE:	    Mar 27, 1989 
REVISIONS:  
----------------------------------------------------------------- */
#include "allheaders.h"

void new_old(int c)
{

int old, new ;

if( ablockG != bblockG ) {
    barrayG[ablockG]->newsize = barrayG[ablockG]->oldsize + c ;
    barrayG[bblockG]->newsize = barrayG[bblockG]->oldsize - c ;
    old  = ABS(barrayG[ablockG]->oldsize - barrayG[ablockG]->desire) +
	   ABS(barrayG[bblockG]->oldsize - barrayG[bblockG]->desire) ;
    new  = ABS(barrayG[ablockG]->newsize - barrayG[ablockG]->desire) +
	   ABS(barrayG[bblockG]->newsize - barrayG[bblockG]->desire) ;
    newrowpenalG += (new - old) ;

}
return ;
}

void old_assgnto_new1( int alobin , int ahibin , int anewlobin , int anewhibin )
{
	int bin , lobin , hibin ;

	if( ablockG == bblockG ) {
			lobin = ( alobin <= anewlobin ) ? alobin : anewlobin ;
			hibin = ( ahibin >= anewhibin ) ? ahibin : anewhibin ;
			for( bin = lobin ; bin <= hibin ; bin++ ) {
				binptrG[ablockG][bin]->nupenalty = binptrG[ablockG][bin]->penalty ;
			}
		} else {
			for( bin = alobin ; bin <= ahibin ; bin++ ) {
				binptrG[ablockG][bin]->nupenalty = binptrG[ablockG][bin]->penalty ;
			}
			for( bin = anewlobin ; bin <= anewhibin ; bin++ ) {
				binptrG[bblockG][bin]->nupenalty = binptrG[bblockG][bin]->penalty ;
			}
	} 
}

void new_assgnto_old1( int alobin , int ahibin , int anewlobin , int anewhibin )
{
	int bin , lobin , hibin ;

	if( ablockG == bblockG ) {
		lobin = ( alobin <= anewlobin ) ? alobin : anewlobin ;
		hibin = ( ahibin >= anewhibin ) ? ahibin : anewhibin ;
		for( bin = lobin ; bin <= hibin ; bin++ ) {
			binptrG[ablockG][bin]->penalty = binptrG[ablockG][bin]->nupenalty ;
		}
	} else {
		for( bin = alobin ; bin <= ahibin ; bin++ ) {
			binptrG[ablockG][bin]->penalty = binptrG[ablockG][bin]->nupenalty ;
		}
		for( bin = anewlobin ; bin <= anewhibin ; bin++ ) {
			binptrG[bblockG][bin]->penalty = binptrG[bblockG][bin]->nupenalty ;
		}
	} 
}

void old_assgnto_new2( int a1lobin , int a1hibin , int a2lobin , int a2hibin , int b1lobin , int b1hibin , int b2lobin , int b2hibin )
{

int clobin , chibin , dlobin , dhibin ;
int lobin , hibin , bin ;

clobin = ( a1lobin <= b2lobin ) ? a1lobin : b2lobin ;
chibin = ( a1hibin >= b2hibin ) ? a1hibin : b2hibin ;
dlobin = ( a2lobin <= b1lobin ) ? a2lobin : b1lobin ;
dhibin = ( a2hibin >= b1hibin ) ? a2hibin : b1hibin ;
if( ablockG == bblockG ) {
    lobin = ( clobin <= dlobin ) ? clobin : dlobin ;
    hibin = ( chibin >= dhibin ) ? chibin : dhibin ;
    for( bin = lobin ; bin <= hibin ; bin++ ) {
	binptrG[ablockG][bin]->nupenalty = 
		     binptrG[ablockG][bin]->penalty ;
    }
} else {
    for( bin = clobin ; bin <= chibin ; bin++ ) {
	binptrG[ablockG][bin]->nupenalty = 
		     binptrG[ablockG][bin]->penalty ;
    }
    for( bin = dlobin ; bin <= dhibin ; bin++ ) {
	binptrG[bblockG][bin]->nupenalty = 
		     binptrG[bblockG][bin]->penalty ;
    }
}
}

void new_assgnto_old2( int a1lobin , int a1hibin , int a2lobin , int a2hibin , int b1lobin , int b1hibin , int b2lobin , int b2hibin )
{

int clobin , chibin , dlobin , dhibin ;
int lobin , hibin , bin ;

clobin = ( a1lobin <= b2lobin ) ? a1lobin : b2lobin ;
chibin = ( a1hibin >= b2hibin ) ? a1hibin : b2hibin ;
dlobin = ( a2lobin <= b1lobin ) ? a2lobin : b1lobin ;
dhibin = ( a2hibin >= b1hibin ) ? a2hibin : b1hibin ;
if( ablockG == bblockG ) {
    lobin = ( clobin <= dlobin ) ? clobin : dlobin ;
    hibin = ( chibin >= dhibin ) ? chibin : dhibin ;
    for( bin = lobin ; bin <= hibin ; bin++ ) {
	binptrG[ablockG][bin]->penalty = 
		     binptrG[ablockG][bin]->nupenalty ;
    }
} else {
    for( bin = clobin ; bin <= chibin ; bin++ ) {
	binptrG[ablockG][bin]->penalty = 
		     binptrG[ablockG][bin]->nupenalty ;
    }
    for( bin = dlobin ; bin <= dhibin ; bin++ ) {
	binptrG[bblockG][bin]->penalty = 
		     binptrG[bblockG][bin]->nupenalty ;
    }
}
}


void sub_penal( int startx , int endx , int block , int LoBin , int HiBin )
{

BINPTR bptr ;
int bin ;


if( LoBin == HiBin ) {
    bptr = binptrG[block][LoBin] ;
    newbinpenalG     -= ABS( bptr->nupenalty ) ; 
    bptr->nupenalty -= endx - startx          ;
    newbinpenalG     += ABS( bptr->nupenalty ) ;
} else {
    for( bin = LoBin ; bin <= HiBin ; bin++ ) {
	bptr = binptrG[block][bin] ;
	if( bin == LoBin ) {
	    newbinpenalG     -= ABS( bptr->nupenalty ) ;
	    bptr->nupenalty -= bptr->right - startx   ;
	    newbinpenalG     += ABS( bptr->nupenalty ) ;
        } else if( bin == HiBin ) {
	    newbinpenalG     -= ABS( bptr->nupenalty ) ;
	    bptr->nupenalty -= endx - bptr->left      ;
	    newbinpenalG     += ABS( bptr->nupenalty ) ;
        } else {
	    newbinpenalG     -= ABS( bptr->nupenalty ) ;
	    bptr->nupenalty -= binWidthG               ;
	    newbinpenalG     += ABS( bptr->nupenalty ) ;
        }
    }
}
}

void add_penal( int startx , int endx , int block , int LoBin , int HiBin )
{

BINPTR bptr ;
int bin ;

if( LoBin == HiBin ) {
    bptr = binptrG[block][LoBin] ;
    newbinpenalG     -= ABS( bptr->nupenalty ) ; 
    bptr->nupenalty += endx - startx          ;
    newbinpenalG     += ABS( bptr->nupenalty ) ;
} else {
    for( bin = LoBin ; bin <= HiBin ; bin++ ) {
	bptr = binptrG[block][bin] ;
	if( bin == LoBin ) {
	    newbinpenalG     -= ABS( bptr->nupenalty ) ;
	    bptr->nupenalty += bptr->right - startx   ;
	    newbinpenalG     += ABS( bptr->nupenalty ) ;
        } else if( bin == HiBin ) {
	    newbinpenalG     -= ABS( bptr->nupenalty ) ;
	    bptr->nupenalty += endx - bptr->left      ;
	    newbinpenalG     += ABS( bptr->nupenalty ) ;
        } else {
	    newbinpenalG     -= ABS( bptr->nupenalty ) ;
	    bptr->nupenalty += binWidthG               ;
	    newbinpenalG     += ABS( bptr->nupenalty ) ;
        }
    }
}
}


void term_newpos( PINBOXPTR antrmptr  , int xcenter , int ycenter , int newaor )
{

register PINBOXPTR pinptr ;

for( pinptr = antrmptr ; pinptr; pinptr = pinptr->nextpin ) {
    netarrayG[ pinptr->net ]->dflag = 1 ;
    pinptr->flag = 1 ;
    pinptr->newx = pinptr->txpos[ newaor/2 ] + xcenter ;
    pinptr->newy = pinptr->typos[ newaor%2 ] + ycenter ;
}
}
