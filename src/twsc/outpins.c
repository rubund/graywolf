/*
 *   Copyright (C) 1989-1991 Yale University
 *   Copyright (C) 2015 Tim Edwards <tim@opencircuitdesign.com>
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
FILE:	    outpins.c                                       
DESCRIPTION:output the pin (global route) information.
CONTENTS:   outpins()
	    do_outpins( ptr , flag )
		PINBOXPTR ptr ;
		INT flag ;
	    do_left_pseudo_pins( ptr , channel , groupS_number )
		PINBOXPTR ptr ;
		INT channel , groupS_number ;
	    do_right_pseudo_pins( ptr , channel , groupS_number )
		PINBOXPTR ptr ;
		INT channel , groupS_number ;
	    do_left_vertical_channel( ptr )
		PINBOXPTR ptr ;
	    do_right_vertical_channel( ptr )
		PINBOXPTR ptr ;
	    do_bottom_channel( ptr )
		PINBOXPTR ptr ;
	    do_top_channel( ptr )
		PINBOXPTR ptr ;
DATE:	    Mar 27, 1989 
REVISIONS:  Thu Feb  7 00:10:27 EST 1991 - now also support old
		pin output format.
	    Wed Mar 13 13:46:21 CST 1991 - made the new format
		the default.
	    Thu Dec  5 21:58:34 EST 1991 - fixed problem with
		macro cell output pins.
----------------------------------------------------------------- */
#ifndef VMS
#ifndef lint
static char SccsId[] = "@(#) outpins.c (Yale) version 4.9 12/5/91" ;
#endif
#endif

#include <string.h>
#include "standard.h"
#include "groute.h"
#include "main.h"
#include "readpar.h"
#include "pads.h"

#include <yalecad/debug.h>

/* #define NSC */

/* global variables */
extern INT *root_G ;
extern INT *count_G ;
extern INT *stack_G ;
extern INT *father_G ;
extern INT Max_numPinsG ;
extern BOOL new_row_formatG ;
extern PINBOXPTR *vertex_G ;

/* static definitions */
static INT vtxS ;
static INT *hashS ;
static INT *groupS ;
static INT numpinS ;
static INT *rite_edgeS ;
static INT *left_edgeS ;
static FILE *fpS ;
static BOOL old_formatS = FALSE ;

char *find_layer( /* pinname, layer */ ) ;

static do_outpins();
static do_macropins();
static do_left_vertical_channel();
static do_right_vertical_channel();
static do_bottom_channel();
static do_top_channel();

outpins()
{

PINBOXPTR ptr1 , ptr2 , ptr ;
CBOXPTR cellptr ;
SEGBOXPTR seg ;
PADBOXPTR pp1, pp2 ;
char filename[64] ;
INT i , a , b , net , row , padside ;
INT upFlag , downFlag , groupS_index ;
extern char *strtok() ;

if( old_formatS ){
    old_outpins() ;
    return ;
}

hashS  = (INT *)Ysafe_malloc( 2 * Max_numPinsG * sizeof( INT ) ) ;
groupS = (INT *)Ysafe_malloc( 2 * Max_numPinsG * sizeof( INT ) ) ;
rite_edgeS = (INT *)Ysafe_malloc( numChansG * sizeof( INT ) ) ;
left_edgeS = (INT *)Ysafe_malloc( numChansG * sizeof( INT ) ) ;
for( row = 1 ; row <= numRowsG ; row++ ) {
    cellptr = carrayG[ pairArrayG[row][ pairArrayG[row][0] ] ] ;
    rite_edgeS[row] = cellptr->cxcenter + cellptr->tileptr->right ;
}
for( row = 1 ; row <= numRowsG ; row++ ) {
    left_edgeS[row] = barrayG[row]->bxcenter + barrayG[row]->bleft ;
}

sprintf( filename , "%s.pin" , cktNameG ) ;
fpS = TWOPEN ( filename , "w", ABORT ) ;

groupS_index = 0 ;
for( net = 1 ; net <= numnetsG ; net++ ) {
    vtxS = 0 ;
    for( ptr = netarrayG[net]->pins ; ptr ; ptr = ptr->next ) {
	vertex_G[ ++vtxS ] = ptr ;
	ptr->newy = vtxS ;
    }
    numpinS = vtxS ;
    // if( numpinS <= 1 ) {
    if( numpinS == 0 ) {
	continue ;
    }
    for( ptr = netarrayG[net]->pins ; ptr ; ptr = ptr->next ) {
	vertex_G[ ++vtxS ] = ptr ;
    }
    for( i = 1 ; i <= vtxS ; i++ ) {
	count_G[i] = 1 ;
	father_G[i] = 0 ;
	root_G[i] = i ;
	hashS[i] = 0 ;
	groupS[i] = 0 ;
    }
    for( seg = netsegHeadG[net]->next ; seg ; seg = seg->next ) {
	ptr1 = seg->pin1ptr ;
	ptr2 = seg->pin2ptr ;
	if( seg->switchvalue == swUP ) {
	    a = find_set_name( ptr1->newy + numpinS ) ;
	    b = find_set_name( ptr2->newy + numpinS ) ;
	} else if( seg->switchvalue == swDOWN ) {
	    a = find_set_name( ptr1->newy ) ;
	    b = find_set_name( ptr2->newy ) ;
	} else if( ptr1->row > ptr2->row ) {
	    a = find_set_name( ptr1->newy ) ;
	    b = find_set_name( ptr2->newy + numpinS ) ;
	} else if( ptr1->row < ptr2->row ) {
	    a = find_set_name( ptr1->newy + numpinS ) ;
	    b = find_set_name( ptr2->newy ) ;
	} else if( ptr1->row == 0 ) {
	    a = find_set_name( ptr1->newy + numpinS ) ;
	    b = find_set_name( ptr2->newy + numpinS ) ;
	} else if( ptr1->row == numChansG ) {
	    a = find_set_name( ptr1->newy ) ;
	    b = find_set_name( ptr2->newy ) ;
	} else if( (pp1 = carrayG[ptr1->cell]->padptr) &&
		   (pp2 = carrayG[ptr2->cell]->padptr) &&
		   pp1->padside && pp2->padside ) {
	    a = find_set_name( ptr1->newy ) ;
	    b = find_set_name( ptr2->newy ) ;
	} else if( seg->flag > ptr1->row ) {
	    a = find_set_name( ptr1->newy + numpinS ) ;
	    b = find_set_name( ptr2->newy + numpinS ) ;
	} else {
	    a = find_set_name( ptr1->newy ) ;
	    b = find_set_name( ptr2->newy ) ;
	}
	do_set_union( a , b ) ;
    }
    for( ptr = netarrayG[net]->pins ; ptr ; ptr = ptr->next ) {
	if( (pp1 = carrayG[ptr->cell]->padptr) && pp1->padside ) {
	    a = find_set_name( ptr->newy + numpinS ) ;
	    b = find_set_name( ptr->newy ) ;
	    do_set_union( a , b ) ;
	}
    }
    for( i = 1 ; i <= vtxS ; i++ ) {
	a = find_set_name( i ) ;
	if( count_G[a] > 1 ) {
	    if( hashS[a] == 0 ) {
		groupS[i] = ++groupS_index ;
		hashS[a] = groupS_index ;
	    } else {
		groupS[i] = hashS[a] ;
	    }
	}
    }
    for( ptr = netarrayG[net]->pins ; ptr ; ptr = ptr->next ) {
	a = find_set_name( ptr->newy + numpinS ) ;
	if( count_G[a] > 1 ) {
	    upFlag = 1 ;
	} else {
	    upFlag = 0 ;
	}
	b = find_set_name( ptr->newy ) ;
	if( count_G[b] > 1 ) {
	    downFlag = 1 ;
	} else {
	    downFlag = 0 ;
	}
	pp1 = carrayG[ ptr->cell ]->padptr ;
	if( pp1 ){
	    padside = pp1->padside ;
	} else {
	    padside = 0 ;
	}
	switch( padside ){
	    case L:
		do_left_vertical_channel( ptr ) ;
		break ;
	    case T:
		do_top_channel( ptr ) ;
		break ;
	    case R:
		do_right_vertical_channel( ptr ) ;
		break ;
	    case B:
		do_bottom_channel( ptr ) ;
		break ;
	    case MMC:
		do_macropins( ptr ) ;
		break ;
	    case 0:
	    default:
		if( upFlag ) {
		    do_outpins( ptr , 1 ) ;
		}
		if( downFlag ) {
		    do_outpins( ptr , 0 ) ;
		}
		if( !upFlag && !downFlag ) {
		    do_outpins( ptr, -1 ) ;	// Handle singletons
		}
		break ;
	} /* end switch */
    }
    /* the groupS index for vertex_G[i] = groupS[ find_groupS_name(i) ] */
}
Ysafe_free( count_G ) ;
Ysafe_free( father_G ) ;
Ysafe_free( root_G ) ;
Ysafe_free( stack_G ) ;
Ysafe_free( vertex_G ) ;
Ysafe_free( groupS ) ;
Ysafe_free( hashS ) ;
Ysafe_free( left_edgeS ) ;
Ysafe_free( rite_edgeS ) ;
TWCLOSE( fpS ) ;
}

/*------------------------------------------------------------------ *
 *  The nine field have to be written down to cktname.pin file are : *
 * 1. Name of net to which the pn belongs.                           *
 * 2. Group number of the pin.                                       *
 * 3. Cell name to which the pin belongs                             *
 * 4. The name of the pin.                                           *
 * 5. The x coordinate of the pin.                                   *
 * 6. The y coordinate of the pin.                                   *
 * 7. The channel to which the pin belong to.                        *
 * 8. Where the pin is at the top (1) , bottom (-1) or neither (0)   *
 * 9. layer                                                          *
 *-------------------------------------------------------------------*/

static do_outpins( ptr , flag )
PINBOXPTR ptr ;
INT flag ;
{


INT x , y , channel , pinloc , groupS_number , layer , i ;
CBOXPTR cellptr ;
PADBOXPTR pptr ;
char *pinname , tmp_char[2] , *tmp_pinname ;
INT length ;
char master_name[128] , pin_id[128] , tmp_name[128] ;
char instance_name[128] , p_name[128] , *tmp_string ;

tmp_char[1] = EOS ; /* terminate string */

cellptr = carrayG[ ptr->cell ] ;
if( flag == 1 ) {
    channel = ptr->row + 1 ;
    groupS_number = groupS[ ptr->newy + numpinS ] ;
    pinloc = -1 ;
    y = cellptr->cycenter + cellptr->tileptr->top ;
} else if ( flag == 0 ) {
    channel = ptr->row ;
    groupS_number = groupS[ ptr->newy ] ;
    pinloc = 1 ;
    y = cellptr->cycenter + cellptr->tileptr->bottom ;
} else {
    // Singleton
    channel = ptr->row ;
    groupS_number = groupS[ ptr->newy ] ;
    pinloc = 0 ;
    y = cellptr->cycenter + cellptr->tileptr->bottom ;
}
if( (pptr = cellptr->padptr) && pptr->padside ) {
    pinname = ptr->pinname ;
    x = ptr->xpos ;
} else {
    if( ptr->eqptr ) { /* there is at least one equivalent pin */
	if( cellptr->corient == 0 ) {
	    if( flag ) { /* pin is on top side of the row */
		if( ptr->eqptr->typos > 0 ) {
		    /* equivalent pin is on top side of the row
			    when the cell orientation is "0" */
		    pinname = ptr->eqptr->pinname ;
		    x = ptr->xpos + ptr->eqptr->txoff ;
		} else {
		    /* equivalent pin is on the bottom side of
			the row when the cell orientation is "0" */
		    pinname = ptr->pinname ;
		    x = ptr->xpos ;
		}
	    } else { /* pin is on bottom side of the row */
		if( ptr->eqptr->typos < 0 ) {
		    pinname = ptr->eqptr->pinname ;
		    x = ptr->xpos + ptr->eqptr->txoff ;
		} else {
		    pinname = ptr->pinname ;
		    x = ptr->xpos ;
		}
	    }
	} else if( cellptr->corient == 1 ) { /* mirror about x-axis */
	    if( flag ) {
		if( ptr->eqptr->typos < 0 ) {
		    pinname = ptr->eqptr->pinname ;
		    x = ptr->xpos + ptr->eqptr->txoff ;
		} else {
		    pinname = ptr->pinname ;
		    x = ptr->xpos ;
		}
	    } else {
		if( ptr->eqptr->typos > 0 ) {
		    pinname = ptr->eqptr->pinname ;
		    x = ptr->xpos + ptr->eqptr->txoff ;
		} else {
		    pinname = ptr->pinname ;
		    x = ptr->xpos ;
		}
	    }
	} else if( cellptr->corient == 2 ) { /* mirror about y-axis */
	    if( flag ) {
		if( ptr->eqptr->typos > 0 ) {
		    pinname = ptr->eqptr->pinname ;
		    x = ptr->xpos - ptr->eqptr->txoff ;
		} else {
		    pinname = ptr->pinname ;
		    x = ptr->xpos ;
		}
	    } else {
		if( ptr->eqptr->typos < 0 ) {
		    pinname = ptr->eqptr->pinname ;
		    x = ptr->xpos - ptr->eqptr->txoff ;
		} else {
		    pinname = ptr->pinname ;
		    x = ptr->xpos ;
		}
	    }
	} else { /* 180 degree rotation w.r.t orientation 0 */
	    if( flag ) {
		if( ptr->eqptr->typos < 0 ) {
		    pinname = ptr->eqptr->pinname ;
		    x = ptr->xpos - ptr->eqptr->txoff ;
		} else {
		    pinname = ptr->pinname ;
		    x = ptr->xpos ;
		}
	    } else {
		if( ptr->eqptr->typos > 0 ) {
		    pinname = ptr->eqptr->pinname ;
		    x = ptr->xpos - ptr->eqptr->txoff ;
		} else {
		    pinname = ptr->pinname ;
		    x = ptr->xpos ;
		}
	    }
	}
    } else {
	pinname = ptr->pinname ;
	x = ptr->xpos ;
    }
}
tmp_pinname = find_layer( pinname, &layer ) ;

#ifdef NSC

length = strcspn( tmp_pinname , ":" ) ;
if( length < strlen( tmp_pinname ) ) {
    tmp_string = strtok( tmp_pinname , ":" ) ;
    sprintf( p_name , "%s" , tmp_string ) ;
    tmp_string = strtok( NULL , ":" ) ;
    sprintf( pin_id , "%s" , tmp_string ) ;
} else {
    sprintf( p_name , "%s" , tmp_pinname ) ;
    sprintf( pin_id , "%s" , "0" ) ;
}
strcpy( tmp_name , carrayG[ptr->cell]->cname ) ;
length = strcspn( tmp_name , ":" ) ;
if( length < strlen( tmp_name ) ) {
    tmp_string = strtok( tmp_name , ":" ) ;
    sprintf( master_name , "%s" , tmp_string ) ;
    tmp_string = strtok( NULL , ":" ) ;
    sprintf( instance_name, "%s" , tmp_string ) ;
} else {
    sprintf( master_name , "%s" , tmp_name ) ;
    sprintf( instance_name , "%s" , tmp_name ) ;
}

fprintf(fpS,"%s %d %s %s %d %d %d %d %d %s %s\n" ,
    netarrayG[ptr->net]->name , groupS_number ,
	instance_name , p_name ,
	    x , y , channel , pinloc , layer ,
	    master_name , pin_id );
#else
fprintf(fpS,"%s %d %s %s %d %d %d %d %d\n" ,
    netarrayG[ptr->net]->name , groupS_number ,
	carrayG[ptr->cell]->cname , tmp_pinname ,
	    x , y , channel , pinloc , layer );
#endif
return ;
}

static do_macropins( ptr )
PINBOXPTR ptr ;
{


INT x , y , channel , pinloc , groupS_number , layer , i ;
CBOXPTR cellptr ;
PADBOXPTR pptr ;
char *pinname , tmp_char[2] , *tmp_pinname ;
INT length ;
char master_name[128] , pin_id[128] , tmp_name[128] ;
char instance_name[128] , p_name[128] , *tmp_string ;

tmp_char[1] = EOS ; /* terminate string */

cellptr = carrayG[ ptr->cell ] ;
channel = ptr->row ;
groupS_number = groupS[ ptr->newy ] ;
pinloc = 0 ;
pinname = ptr->pinname ;
x = ptr->xpos ;
y = ptr->ypos ;
tmp_pinname = find_layer( pinname, &layer ) ;

#ifdef NSC

length = strcspn( tmp_pinname , ":" ) ;
if( length < strlen( tmp_pinname ) ) {
    tmp_string = strtok( tmp_pinname , ":" ) ;
    sprintf( p_name , "%s" , tmp_string ) ;
    tmp_string = strtok( NULL , ":" ) ;
    sprintf( pin_id , "%s" , tmp_string ) ;
} else {
    sprintf( p_name , "%s" , tmp_pinname ) ;
    sprintf( pin_id , "%s" , "0" ) ;
}
strcpy( tmp_name , carrayG[ptr->cell]->cname ) ;
length = strcspn( tmp_name , ":" ) ;
if( length < strlen( tmp_name ) ) {
    tmp_string = strtok( tmp_name , ":" ) ;
    sprintf( master_name , "%s" , tmp_string ) ;
    tmp_string = strtok( NULL , ":" ) ;
    sprintf( instance_name, "%s" , tmp_string ) ;
} else {
    sprintf( master_name , "%s" , tmp_name ) ;
    sprintf( instance_name , "%s" , tmp_name ) ;
}

fprintf(fpS,"%s %d %s %s %d %d %d %d %d %s %s\n" ,
    netarrayG[ptr->net]->name , groupS_number ,
	instance_name , p_name ,
	    x , y , channel , pinloc , layer ,
	    master_name , pin_id );
#else
fprintf(fpS,"%s %d %s %s %d %d %d %d %d\n" ,
    netarrayG[ptr->net]->name , groupS_number ,
	carrayG[ptr->cell]->cname , tmp_pinname ,
	    x , y , channel , pinloc , layer );
#endif
return ;
}

static do_left_pseudo_pins( ptr , channel , groupS_number )
PINBOXPTR ptr ;
INT channel , groupS_number ;
{


INT x , y ;

if( channel <= numRowsG ) {
    y = barrayG[channel]->bycenter - rowHeightG ;
} else {
    y = barrayG[ numRowsG ]->bycenter + rowHeightG ;
}
if( channel == 1 ) {
    x = left_edgeS[1] ;
} else if( channel == numChansG ) {
    x = left_edgeS[numRowsG] ;
} else {
    if( left_edgeS[channel] <= left_edgeS[channel-1] ) {
	x = left_edgeS[channel] ;
    } else {
	x = left_edgeS[channel-1] ;
    }
}
#ifdef NSC
fprintf(fpS,"%s %d PSEUDO_CELL PSEUDO_PIN %d %d %d -2 0 0 0\n" ,
netarrayG[ptr->net]->name , groupS_number , x , y , channel ) ;

fprintf(fpS,"%s %d PSEUDO_CELL PSEUDO_PIN %d %d -1 -1 0 0 0\n" ,
netarrayG[ptr->net]->name , groupS_number , x , y ) ;
#else
fprintf(fpS,"%s %d PSEUDO_CELL PSEUDO_PIN %d %d %d -2 0\n" ,
netarrayG[ptr->net]->name , groupS_number , x , y , channel ) ;

fprintf(fpS,"%s %d PSEUDO_CELL PSEUDO_PIN %d %d -1 -1 0\n" ,
netarrayG[ptr->net]->name , groupS_number , x , y ) ;
#endif
}


static do_right_pseudo_pins( ptr , channel , groupS_number )
PINBOXPTR ptr ;
INT channel , groupS_number ;
{


INT x , y ;

if( channel == 1 ) {
    y = barrayG[1]->bycenter - rowHeightG ;
    x = rite_edgeS[1] ;
} else if( channel <= numRowsG ) {
    y = barrayG[channel]->bycenter - rowHeightG ;
    if( rite_edgeS[channel] < rite_edgeS[channel-1] ) {
	x = rite_edgeS[channel] ;
    } else {
	x = rite_edgeS[channel-1] ;
    }
} else {
    y = barrayG[numRowsG]->bycenter + rowHeightG ;
    x = rite_edgeS[numRowsG] ;
}
#ifdef NSC
fprintf(fpS,"%s %d PSEUDO_CELL PSEUDO_PIN %d %d %d 2 0 0 0\n" ,
netarrayG[ptr->net]->name , groupS_number , x , y , channel ) ;

fprintf(fpS,"%s %d PSEUDO_CELL PSEUDO_PIN %d %d -2 -1 0 0 0\n" ,
netarrayG[ptr->net]->name , groupS_number , x , y ) ;
#else
fprintf(fpS,"%s %d PSEUDO_CELL PSEUDO_PIN %d %d %d 2 0\n" ,
netarrayG[ptr->net]->name , groupS_number , x , y , channel ) ;

fprintf(fpS,"%s %d PSEUDO_CELL PSEUDO_PIN %d %d -2 -1 0\n" ,
netarrayG[ptr->net]->name , groupS_number , x , y ) ;
#endif
}

static do_left_vertical_channel( ptr )
PINBOXPTR ptr ;
{

PINBOXPTR core_ptr ;
SEGBOXPTR segptr ;
ADJASEG *adj ;
PADBOXPTR pptr ;
INT groupS_number , layer , i ;
char tmp_char[2] , tmp_pinname[1024] ;
INT length ;
char master_name[128] , pin_id[128] , tmp_name[128] ;
char instance_name[128] , p_name[128] , *tmp_string ;

groupS_number = groupS[ ptr->newy ] ;
strcpy( tmp_pinname , ptr->pinname ) ;
if( pin_layers_givenG != 0 ) {
    if( strncmp( ptr->pinname, "]", 1 ) == 0 ) {
	strncpy( tmp_char , tmp_pinname + 1 , 1 ) ;
	tmp_char[1] = '\0' ;
	layer = atoi( tmp_char ) ;
	for( i = 0 ; ; i++ ) {
	    tmp_pinname[i] = tmp_pinname[i+2] ;
	    if( tmp_pinname[i] == '\0' ) {
		break ;
	    }
	}
    } else {
	layer = feedLayerG ;
    }
} else {
    layer = 0 ;
}
#ifdef NSC

length = strcspn( tmp_pinname , ":" ) ;
if( length < strlen( tmp_pinname ) ) {
    tmp_string = strtok( tmp_pinname , ":" ) ;
    sprintf( p_name , "%s" , tmp_string ) ;
    tmp_string = strtok( NULL , ":" ) ;
    sprintf( pin_id , "%s" , tmp_string ) ;
} else {
    sprintf( p_name , "%s" , tmp_pinname ) ;
    sprintf( pin_id , "%s" , "0" ) ;
}
strcpy( tmp_name , carrayG[ptr->cell]->cname ) ;
length = strcspn( tmp_name , ":" ) ;
if( length < strlen( tmp_name ) ) {
    tmp_string = strtok( tmp_name , ":" ) ;
    sprintf( master_name , "%s" , tmp_string ) ;
    tmp_string = strtok( NULL , ":" ) ;
    sprintf( instance_name, "%s" , tmp_string ) ;
} else {
    sprintf( master_name , "%s" , tmp_name ) ;
    sprintf( instance_name , "%s" , tmp_name ) ;
}

fprintf(fpS,"%s %d %s %s %d %d -1 1 %d %s %s\n" ,
    netarrayG[ptr->net]->name , groupS_number , 
    instance_name , p_name , 
    ptr->xpos , ptr->ypos , layer ,
    master_name , pin_id );
#else
fprintf(fpS,"%s %d %s %s %d %d -1 1 %d\n" ,
    netarrayG[ptr->net]->name , groupS_number , 
    carrayG[ptr->cell]->cname , tmp_pinname , 
    ptr->xpos , ptr->ypos , layer ) ;
#endif
for( adj = ptr->adjptr->next ; adj ; adj = adj->next ) {
    segptr = adj->segptr ;
    if( ptr == segptr->pin1ptr ) {
	core_ptr = segptr->pin2ptr ;
    } else {
	core_ptr = segptr->pin1ptr ;
    }
    if( !(pptr = carrayG[core_ptr->cell]->padptr) || pptr->padside == MMC ) {
	do_left_pseudo_pins( ptr, segptr->flag , groupS_number ) ;
    }
}
}


static do_right_vertical_channel( ptr )
PINBOXPTR ptr ;
{

PINBOXPTR core_ptr ;
SEGBOXPTR segptr ;
ADJASEG *adj ;
PADBOXPTR pptr ;
INT groupS_number , layer , i ;
char tmp_char[2] , *tmp_pinname ;
INT length ;
char master_name[128] , pin_id[128] , tmp_name[128] ;
char instance_name[128] , p_name[128] , *tmp_string ;

groupS_number = groupS[ ptr->newy ] ;
tmp_pinname = find_layer( ptr->pinname, &layer ) ;
#ifdef NSC

length = strcspn( tmp_pinname , ":" ) ;
if( length < strlen( tmp_pinname ) ) {
    tmp_string = strtok( tmp_pinname , ":" ) ;
    sprintf( p_name , "%s" , tmp_string ) ;
    tmp_string = strtok( NULL , ":" ) ;
    sprintf( pin_id , "%s" , tmp_string ) ;
} else {
    sprintf( p_name , "%s" , tmp_pinname ) ;
    sprintf( pin_id , "%s" , "0" ) ;
}
strcpy( tmp_name , carrayG[ptr->cell]->cname ) ;
length = strcspn( tmp_name , ":" ) ;
if( length < strlen( tmp_name ) ) {
    tmp_string = strtok( tmp_name , ":" ) ;
    sprintf( master_name , "%s" , tmp_string ) ;
    tmp_string = strtok( NULL , ":" ) ;
    sprintf( instance_name, "%s" , tmp_string ) ;
} else {
    sprintf( master_name , "%s" , tmp_name ) ;
    sprintf( instance_name , "%s" , tmp_name ) ;
}

fprintf(fpS,"%s %d %s %s %d %d -2 1 %d %s %s\n" ,
    netarrayG[ptr->net]->name , groupS_number , 
    instance_name , p_name , 
    ptr->xpos , ptr->ypos , layer ,
    master_name , pin_id );
#else
fprintf(fpS,"%s %d %s %s %d %d -2 1 %d\n" ,
    netarrayG[ptr->net]->name , groupS_number , 
    carrayG[ptr->cell]->cname, tmp_pinname , 
    ptr->xpos , ptr->ypos , layer ) ;
#endif
for( adj = ptr->adjptr->next ; adj ; adj = adj->next ) {
    segptr = adj->segptr ;
    if( ptr == segptr->pin1ptr ) {
	core_ptr = segptr->pin2ptr ;
    } else {
	core_ptr = segptr->pin1ptr ;
    }
    if( !(pptr = carrayG[core_ptr->cell]->padptr) || pptr->padside == MMC ) {
	do_right_pseudo_pins( ptr, segptr->flag , groupS_number) ;
    }
}
}


static do_bottom_channel( ptr )
PINBOXPTR ptr ;
{
INT x , y , groupS_number , layer , i ;
char tmp_char[2] , *tmp_pinname ;
ADJASEG *adj ;
SEGBOXPTR segptr ;
PINBOXPTR core_ptr ;
PADBOXPTR pptr ;
INT length ;
INT padside ;
char master_name[128] , pin_id[128] , *tmp_name ;
char instance_name[128] , p_name[128] , *tmp_string ;

groupS_number = groupS[ ptr->newy ] ;
tmp_pinname = find_layer( ptr->pinname, &layer ) ;
#ifdef NSC

length = strcspn( tmp_pinname , ":" ) ;
if( length < strlen( tmp_pinname ) ) {
    tmp_string = strtok( tmp_pinname , ":" ) ;
    sprintf( p_name , "%s" , tmp_string ) ;
    tmp_string = strtok( NULL , ":" ) ;
    sprintf( pin_id , "%s" , tmp_string ) ;
} else {
    sprintf( p_name , "%s" , tmp_pinname ) ;
    sprintf( pin_id , "%s" , "0" ) ;
}
strcpy( tmp_name , carrayG[ptr->cell]->cname ) ;
length = strcspn( tmp_name , ":" ) ;
if( length < strlen( tmp_name ) ) {
    tmp_string = strtok( tmp_name , ":" ) ;
    sprintf( master_name , "%s" , tmp_string ) ;
    tmp_string = strtok( NULL , ":" ) ;
    sprintf( instance_name, "%s" , tmp_string ) ;
} else {
    sprintf( master_name , "%s" , tmp_name ) ;
    sprintf( instance_name , "%s" , tmp_name ) ;
}

fprintf(fpS,"%s %d %s %s %d %d -3 -1 %d %s %s\n" ,
    netarrayG[ptr->net]->name , groupS_number ,
    instance_name , p_name , 
    ptr->xpos , ptr->ypos , layer ,
    master_name , pin_id );
#else
fprintf(fpS,"%s %d %s %s %d %d -3 -1 %d\n" ,
    netarrayG[ptr->net]->name , groupS_number ,
    carrayG[ptr->cell]->cname , tmp_pinname , 
    ptr->xpos , ptr->ypos , layer );
#endif
y = barrayG[1]->bycenter + barrayG[1]->bbottom - 1 ;
for( adj = ptr->adjptr->next ; adj ; adj = adj->next ) {
    segptr = adj->segptr ;
    if( ptr == segptr->pin1ptr ) {
	core_ptr = segptr->pin2ptr ;
    } else {
	core_ptr = segptr->pin1ptr ;
    }
    if( pptr = carrayG[core_ptr->cell]->padptr ){
	padside = pptr->padside ;
    } else {
	padside = 0 ;
    }
    if( padside || core_ptr->row > 1 ) {
	continue ;
    } else {
	x = core_ptr->xpos ;
#ifdef NSC
	fprintf(fpS,"%s %d PSEUDO_CELL PSEUDO_PIN %d %d 1 -1 0 0 0\n" ,
	netarrayG[ptr->net]->name , groupS_number , x , y ) ;

	fprintf(fpS,"%s %d PSEUDO_CELL PSEUDO_PIN %d %d -3 1 0 0 0\n" ,
	netarrayG[ptr->net]->name , groupS_number , x , y ) ;
#else
	fprintf(fpS,"%s %d PSEUDO_CELL PSEUDO_PIN %d %d 1 -1 0\n" ,
	netarrayG[ptr->net]->name , groupS_number , x , y ) ;

	fprintf(fpS,"%s %d PSEUDO_CELL PSEUDO_PIN %d %d -3 1 0\n" ,
	netarrayG[ptr->net]->name , groupS_number , x , y ) ;
#endif
    }
}
}


static do_top_channel( ptr )
PINBOXPTR ptr ;
{
INT x , y , groupS_number , layer , i ;
char tmp_char[2] , *tmp_pinname ;
ADJASEG *adj ;
SEGBOXPTR segptr ;
PINBOXPTR core_ptr ;
PADBOXPTR pptr ;
INT length ;
INT padside ;
char master_name[128] , pin_id[128] , *tmp_name[128] ;
char instance_name[128] , p_name[128] , *tmp_string ;

groupS_number = groupS[ ptr->newy ] ;
tmp_pinname = find_layer( ptr->pinname, &layer ) ;
#ifdef NSC

length = strcspn( tmp_pinname , ":" ) ;
if( length < strlen( tmp_pinname ) ) {
    tmp_string = strtok( tmp_pinname , ":" ) ;
    sprintf( p_name , "%s" , tmp_string ) ;
    tmp_string = strtok( NULL , ":" ) ;
    sprintf( pin_id , "%s" , tmp_string ) ;
} else {
    sprintf( p_name , "%s" , tmp_pinname ) ;
    sprintf( pin_id , "%s" , "0" ) ;
}
strcpy( tmp_name , carrayG[ptr->cell]->cname ) ;
length = strcspn( tmp_name , ":" ) ;
if( length < strlen( tmp_name ) ) {
    tmp_string = strtok( tmp_name , ":" ) ;
    sprintf( master_name , "%s" , tmp_string ) ;
    tmp_string = strtok( NULL , ":" ) ;
    sprintf( instance_name, "%s" , tmp_string ) ;
} else {
    sprintf( master_name , "%s" , tmp_name ) ;
    sprintf( instance_name , "%s" , tmp_name ) ;
}

fprintf(fpS,"%s %d %s %s %d %d -4 1 %d %s %s\n" ,
    netarrayG[ptr->net]->name , groupS_number ,
    instance_name , p_name , 
    ptr->xpos , ptr->ypos , layer ,
    master_name , pin_id );
#else
fprintf(fpS,"%s %d %s %s %d %d -4 1 %d\n" ,
    netarrayG[ptr->net]->name , groupS_number ,
    carrayG[ptr->cell]->cname , tmp_pinname , 
    ptr->xpos , ptr->ypos , layer );
#endif
y = barrayG[numRowsG]->bycenter + barrayG[numRowsG]->btop + 1 ;
for( adj = ptr->adjptr->next ; adj ; adj = adj->next ) {
    segptr = adj->segptr ;
    if( ptr == segptr->pin1ptr ) {
	core_ptr = segptr->pin2ptr ;
    } else {
	core_ptr = segptr->pin1ptr ;
    }

    if( pptr = carrayG[core_ptr->cell]->padptr ){
	padside = pptr->padside ;
    } else {
	padside = 0 ;
    }
    if( padside || core_ptr->row < numRowsG ) {
	continue ;
    } else {
	x = core_ptr->xpos ;
#ifdef NSC
	fprintf(fpS,"%s %d PSEUDO_CELL PSEUDO_PIN %d %d %d 1 0 0 0\n" ,
	netarrayG[ptr->net]->name , groupS_number , x , y , numChansG ) ;

	fprintf(fpS,"%s %d PSEUDO_CELL PSEUDO_PIN %d %d -4 -1 0 0 0\n" ,
	netarrayG[ptr->net]->name , groupS_number , x , y ) ;
#else
	fprintf(fpS,"%s %d PSEUDO_CELL PSEUDO_PIN %d %d %d 1 0\n" ,
	netarrayG[ptr->net]->name , groupS_number , x , y , numChansG ) ;

	fprintf(fpS,"%s %d PSEUDO_CELL PSEUDO_PIN %d %d -4 -1 0\n" ,
	netarrayG[ptr->net]->name , groupS_number , x , y ) ;
#endif
    }
}
}

char *find_layer( pinname, layer )
char *pinname ;
INT *layer ;
{
    static char pinbufL[LRECL] ;
    char layer_buffer[2] ;

    ASSERTNRETURN( pinname,"find_layer", "pinname is NULL" ) ;
    if( pin_layers_givenG ) {
	/* transform "]2name" --> "name" and layer = 2 */
	if( pinname && *pinname == ']' ){
	    layer_buffer[0] = pinname[1] ;
	    layer_buffer[1] = EOS ;
	    *layer = atoi( layer_buffer ) ;
	    strcpy( pinbufL, &(pinname[2]) ) ;
	} else {
	    *layer = feedLayerG ;
	    strcpy( pinbufL, pinname ) ;
	}
    } else {
	*layer = 0 ;
	strcpy( pinbufL, pinname ) ;
    }
    return( pinbufL ) ;
} /* end find_layer */


set_pin_format( flag )
BOOL flag ;
{
    old_formatS = flag ;
} /* end set_pin_format */
