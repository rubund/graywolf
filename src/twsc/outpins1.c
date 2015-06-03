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
FILE:	    outpins.c                                       
DESCRIPTION:output the pin (global route) information.
CONTENTS:   outpins()
	    do_outpins( ptr , flag )
		PINBOXPTR ptr ;
		int flag ;
	    do_left_pseudo_pins( ptr , channel , group_number )
		PINBOXPTR ptr ;
		int channel , group_number ;
	    do_right_pseudo_pins( ptr , channel , group_number )
		PINBOXPTR ptr ;
		int channel , group_number ;
	    do_left_vertical_channel( ptr )
		PINBOXPTR ptr ;
	    do_right_vertical_channel( ptr )
		PINBOXPTR ptr ;
	    do_bottom_channel( ptr )
		PINBOXPTR ptr ;
	    do_top_channel( ptr )
		PINBOXPTR ptr ;
DATE:	    Mar 27, 1989 
REVISIONS:  
----------------------------------------------------------------- */
#ifndef VMS
#ifndef lint
static char SccsId[] = "@(#) outpins1.c (Yale) version 1.3 4/18/91" ;
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

char *strtok() ;

/* global variables */
extern INT *root_G ;
extern INT *count_G ;
extern INT *stack_G ;
extern INT *father_G ;
extern INT Max_numPinsG ;
extern BOOL new_row_formatG ;
extern PINBOXPTR *vertex_G ;
extern char *find_layer() ;

/* static definitions */
static INT vtxS ;
static INT *hashS ;
static INT *groupS ;
static INT numpinS ;
static INT *rite_edgeS ;
static INT *left_edgeS ;
static FILE *fpS ;

static do_outpins();
static do_vertical_channel();
static do_left_vertical_channel();
static do_right_vertical_channel();
static do_bottom_channel();
static do_top_channel();

old_outpins()
{

PINBOXPTR ptr1 , ptr2 , ptr ;
CBOXPTR cellptr ;
SEGBOXPTR seg ;
PADBOXPTR pp1, pp2 ;
char filename[64] ;
int i , a , b , net , row , padside ;
int upFlag , downFlag , group_index ;


hashS  = (INT *)Ysafe_malloc( 2 * Max_numPinsG * sizeof( int ) ) ;
groupS = (INT *)Ysafe_malloc( 2 * Max_numPinsG * sizeof( int ) ) ;
rite_edgeS = (INT *)Ysafe_malloc( numChansG * sizeof( int ) ) ;
left_edgeS = (INT *)Ysafe_malloc( numChansG * sizeof( int ) ) ;
for( row = 1 ; row <= numRowsG ; row++ ) {
    cellptr = carrayG[ pairArrayG[row][ pairArrayG[row][0] ] ] ;
    rite_edgeS[row] = cellptr->cxcenter + cellptr->tileptr->right ;
}
for( row = 1 ; row <= numRowsG ; row++ ) {
    left_edgeS[row] = barrayG[row]->bxcenter + barrayG[row]->bleft ;
}

sprintf( filename , "%s.pin" , cktNameG ) ;
fpS = TWOPEN ( filename , "w", ABORT ) ;

group_index = 0 ;
for( net = 1 ; net <= numnetsG ; net++ ) {
    vtxS = 0 ;
    for( ptr = netarrayG[net]->pins ; ptr ; ptr = ptr->next ) {
	vertex_G[ ++vtxS ] = ptr ;
	ptr->newy = vtxS ;
    }
    numpinS = vtxS ;
    if( numpinS <= 1 ) {
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
		    pp1->padside &&
		   (pp2 = carrayG[ptr2->cell]->padptr) &&
		   pp2->padside ) {
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
		groupS[i] = ++group_index ;
		hashS[a] = group_index ;
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
	if( !padside ) {
	    if( upFlag ) {
		do_outpins( ptr , 1 ) ;
	    }
	    if( downFlag ) {
		do_outpins( ptr , 0 ) ;
	    }
	} else {
	    if( ptr->row == 0 && padside != L && padside != R ) {
		if( rowsG == 0 && new_row_formatG == 0 ) {
		    do_outpins( ptr , 1 ) ;
		} else {
		    do_bottom_channel( ptr ) ;
		}
	    } else if( ptr->row == numChansG && 
				padside != L && padside != R ) {
		if( rowsG == 0 && new_row_formatG == 0 ) {
		    do_outpins( ptr , 0 ) ; 
		} else {
		    do_top_channel( ptr ) ;
		}
	    } else if( rowsG > 0 ) {
		do_vertical_channel( ptr ) ;
	    } else if( ptr->xpos > rite_edgeS[ptr->row] ||
		padside == R || padside == MR || 
		padside == MUR || padside == MLR ) {
		do_right_vertical_channel( ptr ) ;
	    } else {
		do_left_vertical_channel( ptr ) ;
	    }
	}
    }
    /* the group index for vertex_G[i] = groupS[ find_group_name(i) ] */
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
 * 9. 0                                                              *
 *-------------------------------------------------------------------*/

static do_outpins( ptr , flag )
PINBOXPTR ptr ;
int flag ;
{


int x , y , channel , pinloc , group_number , layer , i ;
CBOXPTR cellptr ;
PADBOXPTR pptr ;
char *pinname , *tmp_pinname ;
int length ;
char master_name[128] , pin_id[128] , tmp_name[128] ;
char instance_name[128] , p_name[128] , *tmp_string ;

cellptr = carrayG[ ptr->cell ] ;
if( flag ) {
    channel = ptr->row + 1 ;
    group_number = groupS[ ptr->newy + numpinS ] ;
    pinloc = -1 ;
    y = cellptr->cycenter + cellptr->tileptr->top ;
} else {
    channel = ptr->row ;
    group_number = groupS[ ptr->newy ] ;
    pinloc = 1 ;
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
    netarrayG[ptr->net]->name , group_number ,
	instance_name , p_name ,
	    x , y , channel , pinloc , layer ,
	    master_name , pin_id );
#else
fprintf(fpS,"%s %d %s %s %d %d %d %d %d\n" ,
    netarrayG[ptr->net]->name , group_number ,
	carrayG[ptr->cell]->cname , tmp_pinname ,
	    x , y , channel , pinloc , layer );
#endif
return ;
}


static do_pseudo_pins( ptr , channel , group_number )
PINBOXPTR ptr ;
int channel , group_number ;
{


int x , y , x_1 , x_2 , y_middle , top_row , bot_row , exit_end ;
/* outer-channel-bounding box */
int outer_ch_bb_llx ;
int outer_ch_bb_lly ;
int outer_ch_bb_urx ;
int outer_ch_bb_ury ;

if( channel == numRowsG + 1 ) {
    outer_ch_bb_llx = barrayG[numRowsG]->bxcenter + barrayG[numRowsG]->bleft ;
    outer_ch_bb_urx = barrayG[numRowsG]->bxcenter + barrayG[numRowsG]->bright ;
    outer_ch_bb_lly = barrayG[numRowsG]->bycenter + barrayG[numRowsG]->btop ;
    outer_ch_bb_ury = outer_ch_bb_lly + 1 ;
    y_middle = outer_ch_bb_lly ;
} else if( channel == 1 ) {
    outer_ch_bb_llx = barrayG[1]->bxcenter + barrayG[1]->bleft ;
    outer_ch_bb_urx = barrayG[1]->bxcenter + barrayG[1]->bright ;
    outer_ch_bb_ury = barrayG[1]->bycenter + barrayG[1]->bbottom ;
    outer_ch_bb_lly = outer_ch_bb_ury - 1 ;
    y_middle = outer_ch_bb_ury + 1 ;
} else {
    top_row = channel ;
    bot_row = channel - 1 ;
    x_1 = barrayG[bot_row]->bxcenter + barrayG[bot_row]->bleft ;
    x_2 = barrayG[top_row]->bxcenter + barrayG[top_row]->bleft ;
    if( x_1 < x_2 ) {
	outer_ch_bb_llx = x_1 ;
    } else {
	outer_ch_bb_llx = x_2 ;
    }
    x_1 = barrayG[bot_row]->bxcenter + barrayG[bot_row]->bright ;
    x_2 = barrayG[top_row]->bxcenter + barrayG[top_row]->bright ;
    if( x_1 > x_2 ) {
	outer_ch_bb_urx = x_1 ;
    } else {
	outer_ch_bb_urx = x_2 ;
    }
    outer_ch_bb_ury = barrayG[top_row]->bycenter + barrayG[top_row]->bbottom ;
    outer_ch_bb_lly = barrayG[bot_row]->bycenter + barrayG[bot_row]->btop ;
    y_middle = (outer_ch_bb_lly + outer_ch_bb_ury) / 2 ;
}

/* we have to set exit_end to -2 for exit left (or 2 for exit right) */
/* OR:
/* we have to set exit_end to -1 for bottom (or 1 for top of channel) */

if( ptr->xpos >= outer_ch_bb_llx && ptr->xpos <= outer_ch_bb_urx ) {
    /* exit_end will be either 1 or -1 */
    if( ptr->ypos > y_middle ) {
	exit_end = 1 ;
	x = ptr->xpos ;
	y = outer_ch_bb_ury ;
    } else {
	exit_end = -1 ;
	x = ptr->xpos ;
	y = outer_ch_bb_lly ;
    }
} else if( ptr->xpos < outer_ch_bb_llx ) {
    exit_end = -2 ;
    x = outer_ch_bb_llx ;
    y = y_middle ;
} else {
    exit_end = 2 ;
    x = outer_ch_bb_urx ;
    y = y_middle ;
}

#ifdef NSC
fprintf(fpS,"%s %d PSEUDO_CELL PSEUDO_PIN %d %d %d %d 0 0 0\n" ,
		netarrayG[ptr->net]->name , group_number , 
		x , y , channel, exit_end ) ;
#else
fprintf(fpS,"%s %d PSEUDO_CELL PSEUDO_PIN %d %d %d %d 0\n" ,
		netarrayG[ptr->net]->name , group_number , 
		x , y , channel, exit_end ) ;
#endif
}




static do_left_pseudo_pins( ptr , channel , group_number )
PINBOXPTR ptr ;
int channel , group_number ;
{


int x , y ;

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
netarrayG[ptr->net]->name , group_number , x , y , channel ) ;

fprintf(fpS,"%s %d PSEUDO_CELL PSEUDO_PIN %d %d -1 -1 0 0 0\n" ,
netarrayG[ptr->net]->name , group_number , x , y ) ;
#else
fprintf(fpS,"%s %d PSEUDO_CELL PSEUDO_PIN %d %d %d -2 0\n" ,
netarrayG[ptr->net]->name , group_number , x , y , channel ) ;

fprintf(fpS,"%s %d PSEUDO_CELL PSEUDO_PIN %d %d -1 -1 0\n" ,
netarrayG[ptr->net]->name , group_number , x , y ) ;
#endif
}


static do_right_pseudo_pins( ptr , channel , group_number )
PINBOXPTR ptr ;
int channel , group_number ;
{


int x , y ;

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
netarrayG[ptr->net]->name , group_number , x , y , channel ) ;

fprintf(fpS,"%s %d PSEUDO_CELL PSEUDO_PIN %d %d -2 -1 0 0 0\n" ,
netarrayG[ptr->net]->name , group_number , x , y ) ;
#else
fprintf(fpS,"%s %d PSEUDO_CELL PSEUDO_PIN %d %d %d 2 0\n" ,
netarrayG[ptr->net]->name , group_number , x , y , channel ) ;

fprintf(fpS,"%s %d PSEUDO_CELL PSEUDO_PIN %d %d -2 -1 0\n" ,
netarrayG[ptr->net]->name , group_number , x , y ) ;
#endif
}



static do_vertical_channel( ptr )
PINBOXPTR ptr ;
{

PINBOXPTR core_ptr ;
SEGBOXPTR segptr ;
ADJASEG *adj ;
PADBOXPTR pptr ;
int group_number , i , padside ;

group_number = groupS[ ptr->newy ] ;
for( adj = ptr->adjptr->next ; adj ; adj = adj->next ) {
    segptr = adj->segptr ;
    if( ptr == segptr->pin1ptr ) {
	core_ptr = segptr->pin2ptr ;
    } else {
	core_ptr = segptr->pin1ptr ;
    }
    pptr = carrayG[core_ptr->cell]->padptr ;
    if( pptr ){
	padside = pptr->padside ;
    } else {
	padside = 0 ;
    }
    if( !padside ){
	do_pseudo_pins( ptr, segptr->flag , group_number ) ;

    /* **** */
    } else {
	if( ptr->xpos > rite_edgeS[ptr->row] ||
			    padside == R || padside == MR || 
			    padside == MUR || padside == MLR ) {
	    do_right_vertical_channel( ptr ) ;
	} else {
	    do_left_vertical_channel( ptr ) ;
	}
    }
    /* **** */
}
}


static do_left_vertical_channel( ptr )
PINBOXPTR ptr ;
{

PINBOXPTR core_ptr ;
SEGBOXPTR segptr ;
ADJASEG *adj ;
PADBOXPTR pptr ;
int group_number , layer , i ;
int length ;
char *tmp_pinname ;
char master_name[128] , pin_id[128] , tmp_name[128] ;
char instance_name[128] , p_name[128] , *tmp_string ;

group_number = groupS[ ptr->newy ] ;
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

fprintf(fpS,"%s %d %s %s %d %d -1 1 %d %s %s\n" ,
    netarrayG[ptr->net]->name , group_number , 
    instance_name , p_name , 
    ptr->xpos , ptr->ypos , layer ,
    master_name , pin_id );
#else
fprintf(fpS,"%s %d %s %s %d %d -1 1 %d\n" ,
    netarrayG[ptr->net]->name , group_number , 
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
    pptr = carrayG[core_ptr->cell]->padptr ;
    if( !pptr || pptr->padside == 0 ){
	do_left_pseudo_pins( ptr, segptr->flag , group_number ) ;
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
int a , group_number , layer , i ;
char *tmp_pinname ;
int length ;
char master_name[128] , pin_id[128] , tmp_name[128] ;
char instance_name[128] , p_name[128] , *tmp_string ;

group_number = groupS[ ptr->newy ] ;
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
    netarrayG[ptr->net]->name , group_number , 
    instance_name , p_name , 
    ptr->xpos , ptr->ypos , layer ,
    master_name , pin_id );
#else
fprintf(fpS,"%s %d %s %s %d %d -2 1 %d\n" ,
    netarrayG[ptr->net]->name , group_number , 
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
    pptr = carrayG[core_ptr->cell]->padptr ;
    if( !pptr || pptr->padside == 0 ){
	do_right_pseudo_pins( ptr, segptr->flag , group_number) ;
    }
}
}


static do_bottom_channel( ptr )
PINBOXPTR ptr ;
{
int x , y , group_number , layer , i ;
char *tmp_pinname ;
ADJASEG *adj ;
SEGBOXPTR segptr ;
PINBOXPTR core_ptr ;
PADBOXPTR pptr ;
int length ;
char master_name[128] , pin_id[128] , tmp_name[128] ;
char instance_name[128] , p_name[128] , *tmp_string ;

group_number = groupS[ ptr->newy ] ;
tmp_pinname = find_layer( ptr->pinname, &layer ) ;
strcpy( tmp_pinname , ptr->pinname ) ;
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
    netarrayG[ptr->net]->name , group_number ,
    instance_name , p_name , 
    ptr->xpos , ptr->ypos , layer ,
    master_name , pin_id );
#else
fprintf(fpS,"%s %d %s %s %d %d -3 -1 %d\n" ,
    netarrayG[ptr->net]->name , group_number ,
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
    pptr = carrayG[core_ptr->cell]->padptr ;
    if( pptr && pptr->padside || core_ptr->row > 1 ) {
	continue ;
    } else {
	x = core_ptr->xpos ;
#ifdef NSC
	fprintf(fpS,"%s %d PSEUDO_CELL PSEUDO_PIN %d %d 1 -1 0 0 0\n" ,
	netarrayG[ptr->net]->name , group_number , x , y ) ;

	fprintf(fpS,"%s %d PSEUDO_CELL PSEUDO_PIN %d %d -3 1 0 0 0\n" ,
	netarrayG[ptr->net]->name , group_number , x , y ) ;
#else
	fprintf(fpS,"%s %d PSEUDO_CELL PSEUDO_PIN %d %d 1 -1 0\n" ,
	netarrayG[ptr->net]->name , group_number , x , y ) ;

	fprintf(fpS,"%s %d PSEUDO_CELL PSEUDO_PIN %d %d -3 1 0\n" ,
	netarrayG[ptr->net]->name , group_number , x , y ) ;
#endif
    }
}
}


static do_top_channel( ptr )
PINBOXPTR ptr ;
{
int x , y , group_number , layer , i ;
char *tmp_pinname ;
ADJASEG *adj ;
SEGBOXPTR segptr ;
PINBOXPTR core_ptr ;
PADBOXPTR pptr ;
int length ;
char master_name[128] , pin_id[128] , tmp_name[128] ;
char instance_name[128] , p_name[128] , *tmp_string ;

group_number = groupS[ ptr->newy ] ;
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
    netarrayG[ptr->net]->name , group_number ,
    instance_name , p_name , 
    ptr->xpos , ptr->ypos , layer ,
    master_name , pin_id );
#else
fprintf(fpS,"%s %d %s %s %d %d -4 1 %d\n" ,
    netarrayG[ptr->net]->name , group_number ,
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
    pptr = carrayG[core_ptr->cell]->padptr ;
    if( pptr && pptr->padside || core_ptr->row < numRowsG ) {
	continue ;
    } else {
	x = core_ptr->xpos ;
#ifdef NSC
	fprintf(fpS,"%s %d PSEUDO_CELL PSEUDO_PIN %d %d %d 1 0 0 0\n" ,
	netarrayG[ptr->net]->name , group_number , x , y , numChansG ) ;

	fprintf(fpS,"%s %d PSEUDO_CELL PSEUDO_PIN %d %d -4 -1 0 0 0\n" ,
	netarrayG[ptr->net]->name , group_number , x , y ) ;
#else
	fprintf(fpS,"%s %d PSEUDO_CELL PSEUDO_PIN %d %d %d 1 0\n" ,
	netarrayG[ptr->net]->name , group_number , x , y , numChansG ) ;

	fprintf(fpS,"%s %d PSEUDO_CELL PSEUDO_PIN %d %d -4 -1 0\n" ,
	netarrayG[ptr->net]->name , group_number , x , y ) ;
#endif
    }
}
}
