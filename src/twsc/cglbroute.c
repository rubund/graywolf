/*
 *   Copyright (C) 1989-1990 Yale University
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
FILE:	    cglbroute.c                                       
DESCRIPTION:coarse global routing routines.
CONTENTS:   cglb_initial()
	    proj_tree_to_grid( )
	    set_cbucket( )
	    cglbroute()
	    free_cglb_initial()
	    reinitial_Hdensity()
	    update_switchvalue()
	    rebuild_cbucket()
	    check_cbucket()
	    print_bucket( row )
		INT row ;
DATE:	    Mar 27, 1989 
REVISIONS:  Sat Dec 15 22:08:21 EST 1990 - modified pinloc values
		so that it will always be positive.
----------------------------------------------------------------- */
#ifndef VMS
#ifndef lint
static char SccsId[] = "@(#) cglbroute.c (Yale) version 4.5 12/15/90" ;
#endif
#endif

#include "standard.h"
#include "main.h"
#include "groute.h"
#define OVERHEAD_INIT 100
#define OVERHEAD_DELTA 20

typedef struct coarsedensity {
    SHORT density ;
    SHORT x_coord ;
    struct coarsedensity *next ;
    struct coarsedensity *prev ;
} HCAPBOX , *HCAPPTR ;

static HCAPPTR **HcapacityS ;
static HCAPPTR **entryptS ;
static int	entrysizeS ;
static SEGBOXPTR *swLsegptrS ;
static INT LswitchsegS , svalueS , evalueS ;
static INT *crowdmaxS , glb_crowdmaxS , *node_rightS ;
static DOUBLE  ctrackContS , factor_hereS , factor_oneS , factor_twoS ;

cglb_initial()
{

INT i , j , net , x , tilted_seg ;
PINBOXPTR ptr1 , ptr2 ;
SEGBOXPTR segptr ;

tilted_seg = 0 ;
ctrackContS = 2.25 ;
factor_hereS = 1.0 ;
factor_oneS = 0.5 ;
factor_twoS = 0.2 ;
svalueS = hznode_sepG * 4 / 10 ;
evalueS = hznode_sepG * 6 / 10 ;
node_rightS = (INT *)Ysafe_malloc( ( chan_node_noG + 1 ) * sizeof(INT) );
node_rightS[1] = blkleftG + ( hznode_sepG + 1 ) / 2 ;
for( i = 2 ; i <= chan_node_noG ; i++ ) {
    node_rightS[i] = node_rightS[i-1] + hznode_sepG ;
}
HcapacityS = (HCAPPTR **)Ysafe_malloc( numChansG * sizeof(HCAPPTR *) );
for( i = 1 ; i <= numRowsG ; i++ ) {
    HcapacityS[i] = (HCAPPTR *) Ysafe_calloc( chan_node_noG + 1 , 
	sizeof( HCAPPTR ) ) ;
    for( j = 1 ; j <= chan_node_noG ; j++ ) {
	HcapacityS[i][j] = ( HCAPPTR )Ysafe_calloc(1,sizeof(HCAPBOX));
	HcapacityS[i][j]->x_coord = j ;
    }
}
for( net = 1 ; net <= numnetsG ; net++ ) {
    for( segptr = netsegHeadG[net]->next ; segptr ;
				segptr = segptr->next ) {
	ptr1 = segptr->pin1ptr ;
	ptr2 = segptr->pin2ptr ;
	x = ABS( ptr1->xpos - ptr2->xpos ) ;
	if( ptr1->row != ptr2->row ) {
	    if( add_Lcorner_feedG && x >= average_feed_sepG ) {
		segptr->flag = FALSE ;
	    } else {
		segptr->flag = TRUE ;
	    }
	    if( x >= average_feed_sepG ) {
		segptr->switchvalue = swL_up ;
	    } else {
		segptr->switchvalue = nswLINE ;
	    }
	    tilted_seg++ ;
	} else if( ABS( ptr1->pinloc - ptr2->pinloc ) > 1 ) {
	    segptr->flag = FALSE ;
	    if( x >= average_feed_sepG ) {
		segptr->switchvalue = swL_up ;
	    } else {
		segptr->switchvalue = nswLINE ;
	    }
	} else {
	    segptr->flag = TRUE ;
	    segptr->switchvalue = nswLINE ;
	}
    }
}
fprintf(fpoG," the number of net = %d\n", numnetsG ) ;
fprintf(fpoG," the number of tilted segment = %d\n", tilted_seg ) ;
}


proj_tree_to_grid( )
{

SEGBOXPTR segptr ;
PINBOXPTR ptr1 , ptr2 , netptr ;
INT lowV , highV , lowH , highH ;
INT i , h , k , net ;

for( net = 1 ; net <= numnetsG ; net++ ) {
    for( netptr = steinerHeadG[net]->next;netptr;netptr = netptr->next ) {
	if( netptr->flag && 1 <= netptr->row && netptr->row <= numRowsG){
	    k = set_node( netptr->xpos ) ;
	    feedpptrG[netptr->row][k]->needed++ ;
	}
    }
}
for( net = 1 ; net <= numnetsG ; net++ ) {
    for( segptr = netsegHeadG[net]->next ;
	 segptr ; segptr = segptr->next ) {
	ptr1  = segptr->pin1ptr ;
	ptr2  = segptr->pin2ptr ;
	lowV  = ptr1->row ;
	highV = ptr2->row ;
	lowH  = set_node( ptr1->xpos ) ;
	highH = set_node( ptr2->xpos ) ;
	if( segptr->switchvalue == nswLINE ) {
	    if( lowV != highV ) {
		if( ptr1->pinloc >= NEITHER || ptr1->row == 0 ) {
		    lowV++ ;
		}
		if( ptr2->pinloc <= NEITHER || ptr2->row == numChansG ) {
		    highV-- ;
		}
		for( i = lowV ; i <= highV ; i++ ) {
		    feedpptrG[i][lowH]->needed++ ;
		}
	    } else if( 1 <= lowV && lowV <= numRowsG ) {
		if( lowH == highH ) {
		    HcapacityS[lowV][lowH]->density++ ;
		} else if( ptr2->xpos >=
		    ptr1->xpos + average_feed_sepG ) {
		    for( i = lowH + 1 ; i < highH ; i++ ) {
			HcapacityS[lowV][i]->density++ ;
		    }
		    if( node_rightS[lowH] - ptr1->xpos >= svalueS ) {
			HcapacityS[lowV][lowH]->density++ ;
		    }
		    if( node_rightS[highH] - ptr2->xpos <= evalueS ) {
			HcapacityS[lowV][highH]->density++ ;
		    }
		} else if( ptr1->xpos >=
			   ptr2->xpos + average_feed_sepG ){
		    for( i = highH + 1 ; i < lowH ; i++ ) {
			HcapacityS[lowV][i]->density++ ;
		    }
		    if( node_rightS[highH] - ptr2->xpos >= svalueS ) {
			HcapacityS[lowV][highH]->density++ ;
		    }
		    if( node_rightS[lowH] - ptr1->xpos <= evalueS ) {
			HcapacityS[lowV][lowH]->density++ ;
		    }
		}
		if( segptr->flag == FALSE ) {
			/* segptr->pin1ptr->pinloc == BOTCELL 
			&& segptr->pin2ptr->pinloc ==  TOPCELL */
		    feedpptrG[lowV][lowH]->needed++ ;
		}
	    }
	    continue ;
	} else if( segptr->switchvalue == swL_up ) {
	    h = lowH ;
	    k = highV ;
	    if( ptr1->pinloc >= NEITHER || ptr1->row == 0 ) {
		lowV++ ;
	    }
	    if( ptr2->pinloc <= NEITHER && segptr->flag ||
				ptr2->row == numChansG ) {
		highV-- ;
	    }
	    if( k <= numRowsG ) {
		if( lowH == highH ) {
		    HcapacityS[k][lowH]->density++ ;
		} else if( lowH < highH ) {
		    for( i = lowH + 1 ; i < highH ; i++ ) {
			HcapacityS[k][i]->density++ ;
		    }
		    if( node_rightS[lowH] - ptr1->xpos >= svalueS ) {
			HcapacityS[k][lowH]->density++ ;
		    }
		    if( node_rightS[highH] - ptr2->xpos <= evalueS ) {
			HcapacityS[k][highH]->density++ ;
		    }
		} else {
		    for( i = highH + 1 ; i < lowH ; i++ ) {
			HcapacityS[k][i]->density++ ;
		    }
		    if( node_rightS[highH] - ptr2->xpos >= svalueS ) {
			HcapacityS[k][highH]->density++ ;
		    }
		    if( node_rightS[lowH] - ptr1->xpos <= evalueS ) {
			HcapacityS[k][lowH]->density++ ;
		    }
		}
	    }
	} else { /* switchvalue == swL_down */
	    h = highH ;
	    k = lowV  ;
	    if( ptr1->row == 0 || ptr1->pinloc >= NEITHER && segptr->flag ) {
		lowV++ ;
	    }
	    if( ptr2->row == numChansG || ptr2->pinloc <= NEITHER ) {
		highV-- ;
	    }
	    if( 1 <= k ) {
		if( lowH == highH ) {
		    HcapacityS[k][lowH]->density++ ;
		} else if( lowH < highH ) {
		    for( i = lowH + 1 ; i < highH ; i++ ) {
			HcapacityS[k][i]->density++ ;
		    }
		    if( node_rightS[lowH] - ptr1->xpos >= svalueS ) {
			HcapacityS[k][lowH]->density++ ;
		    }
		    if( node_rightS[highH] - ptr2->xpos <= evalueS ) {
			HcapacityS[k][highH]->density++ ;
		    }
		} else {
		    for( i = highH + 1 ; i < lowH ; i++ ) {
			HcapacityS[k][i]->density++ ;
		    }
		    if( node_rightS[highH] - ptr2->xpos >= svalueS ) {
			HcapacityS[k][highH]->density++ ;
		    }
		    if( node_rightS[lowH] - ptr1->xpos <= evalueS ) {
			HcapacityS[k][lowH]->density++ ;
		    }
		}
	    }
	}
	for( i = lowV ; i <= highV ; i++ ) {
	    feedpptrG[i][h]->needed++ ;
	}
    }
}
}


set_cbucket( )
{

HCAPPTR hcaptr , headptr ;
SEGBOXPTR segptr ;
INT j , last_j , row , max , net ;

glb_crowdmaxS = 0 ;
crowdmaxS = (INT *)Ysafe_calloc( numRowsG + 1, sizeof(INT) ) ;
for( row = 1 ; row <= numRowsG ; row++ ) {
    max = 0 ;
    for( j = 1 ; j <= chan_node_noG ; j++ ) {
	if( HcapacityS[row][j]->density > max ) {
	    max = HcapacityS[row][j]->density ;
	}
    }
    crowdmaxS[row] = max ;
    if( max > glb_crowdmaxS ) {
	glb_crowdmaxS = max ;
    }
}

entryptS = (HCAPPTR **)Ysafe_malloc( ( numRowsG+1 ) * sizeof(HCAPPTR *) ) ;
entrysizeS = glb_crowdmaxS + OVERHEAD_INIT;
for( row = 1 ; row <= numRowsG ; row++ ) {
    entryptS[row] = (HCAPPTR *)Ysafe_malloc( ( entrysizeS + 1 ) * sizeof( HCAPPTR )) ;
    for( j = 0 ; j <= entrysizeS ; j++ ) {
	entryptS[row][j] = (HCAPPTR)Ysafe_calloc( 1,sizeof(HCAPBOX)) ;
    }
}

for( row = 1 ; row <= numRowsG ; row++ ) {
    for( j = 1 ; j <= chan_node_noG ; j++ ) {
	hcaptr = HcapacityS[row][j] ;
	headptr = entryptS[row][ hcaptr->density ] ;
	if( headptr->next != NULL ) {
	    hcaptr->next  = headptr->next ;
	    hcaptr->next->prev = hcaptr ;
	    hcaptr->prev  = headptr ;
	    headptr->next = hcaptr ;
	} else {
	    headptr->next = hcaptr ;
	    hcaptr->prev = headptr ;
	    hcaptr->next = NULL ;
	}
    }
}
LswitchsegS = 0 ;
for( net = 1 ; net <= numnetsG ; net++ ) {
    for( segptr = netsegHeadG[net]->next ; segptr ;
			segptr = segptr->next ) {
	if( segptr->switchvalue != nswLINE ) {
	    LswitchsegS++ ;
	}
    }
}
fprintf(fpoG," the number of switchable L segment = %d\n", LswitchsegS );
swLsegptrS = ( SEGBOXPTR *)Ysafe_malloc( 
	    ( LswitchsegS + 2 * numnetsG ) * sizeof( SEGBOXPTR ) ) ;

j = 0 ;
for( net = 1 ; net <= numnetsG ; net++ ) {
    for( segptr = netsegHeadG[net]->next ; segptr ;
			segptr = segptr->next ) {
	if( segptr->switchvalue != nswLINE ) {
	    swLsegptrS[ ++j ] = segptr ;
	}
    }
}
}


cglbroute()
{

SEGBOXPTR segptr ;
PINBOXPTR ptr1 , ptr2 ;
HCAPPTR hcaptr , denptr , headptr ;
INT trys , maxtrys ;
INT i , h , k , nh , nk , luck ;
INT intersect_max , cover_allmax ;
INT lowH , highH , startH , endH , lowV , highV ;
INT vt_beg , vt_end , nvt_beg , nvt_end ;
INT new_SwValue , density ;
INT range_one_diff , range_two_diff , diff_here ;
INT back_one_diff , back_two_diff , next_one_diff , next_two_diff ;
DOUBLE penalty , ctrack_penalty , attperLseg ;
DOUBLE penalty_here , penalty_one , penalty_two ;


trys = 0 ;
attperLseg = 15.0 ;
maxtrys = attperLseg * LswitchsegS ;

while( ++trys < maxtrys ) {
    luck = (INT)( (DOUBLE)LswitchsegS * ( (DOUBLE)RAND /
				    (DOUBLE) 0x7fffffff ) ) + 1 ;
    segptr = swLsegptrS[ luck ] ;
    ptr1  = segptr->pin1ptr ;
    ptr2  = segptr->pin2ptr ;
    lowH  = set_node( ptr1->xpos ) ;
    highH = set_node( ptr2->xpos ) ;
    lowV  = ptr1->row ;
    highV = ptr2->row ;
    if( lowH == highH ) {
	startH = endH = lowH ;
    } else if( lowH < highH ) {
	if( node_rightS[lowH] - ptr1->xpos >= svalueS ) {
	    startH = lowH ;
	} else {
	    startH = lowH + 1 ;
	}
	if( node_rightS[highH] - ptr2->xpos <= evalueS ) {
	    endH = highH ;
	} else {
	    endH = highH - 1 ;
	}
    } else {
	if( node_rightS[highH] - ptr2->xpos >= svalueS ) {
	    startH = highH ;
	} else {
	    startH = highH + 1 ;
	}
	if( node_rightS[lowH] - ptr1->xpos <= evalueS ) {
	    endH = lowH ;
	} else {
	    endH = lowH - 1 ;
	}
    }
    if( segptr->switchvalue == swL_up ) {
	h  = lowH  ;
	k  = highV ;
	nh = highH ;
	nk = lowV  ;
	new_SwValue = swL_down ;
	if( ptr1->pinloc >= NEITHER ) {
	    vt_beg = lowV + 1 ;
	    if( segptr->flag || ptr1->row == 0 ) {
		nvt_beg = lowV + 1 ;
	    } else {
		nvt_beg = lowV ;
	    }
	} else if( ptr1->row == 0 ) {
	     vt_beg = lowV + 1 ;
	    nvt_beg = lowV + 1 ;
	} else {
	     vt_beg = lowV ;
	    nvt_beg = lowV ;
	}
	if( ptr2->pinloc <= NEITHER ) {
	    if( segptr->flag || ptr2->row == numChansG ) {
		vt_end = highV - 1 ;
	    } else {
		vt_end = highV ;
	    }
	    nvt_end = highV - 1 ;
	} else if( ptr2->row == numChansG ) {
	     vt_end = highV - 1 ;
	    nvt_end = highV - 1 ;
	} else {
	     vt_end = highV ;
	    nvt_end = highV ;
	}

    } else {
	h  = highH ;
	k  = lowV  ;
	nh = lowH  ;
	nk = highV ;
	new_SwValue = swL_up ;
	if( ptr1->pinloc >= NEITHER ) {
	    if( segptr->flag || ptr1->row == 0 ) {
		vt_beg = lowV + 1 ;
	    } else {
		vt_beg = lowV ;
	    }
	    nvt_beg = lowV + 1 ;
	} else if( ptr1->row == 0 ) {
	     vt_beg = lowV + 1 ;
	    nvt_beg = lowV + 1 ;
	} else {
	     vt_beg = lowV ;
	    nvt_beg = lowV ;
	}
	if( ptr2->pinloc <= NEITHER ) {
	    vt_end = highV - 1 ;
	    if( segptr->flag || ptr2->row == numChansG ) {
		nvt_end = highV - 1 ;
	    } else {
		nvt_end = highV ;
	    }
	} else if( ptr2->row == numChansG ) {
	     vt_end = highV - 1 ;
	    nvt_end = highV - 1 ;
	} else {
	    vt_end = highV ;
	    nvt_end = highV ;
	}
    }
    if( 1 <= k && k <= numRowsG ) {
	cover_allmax  = TRUE ;
	for( hcaptr = entryptS[k][ crowdmaxS[k] ]->next ;
		    hcaptr != NULL ; hcaptr = hcaptr->next ) {
	    if( !( startH <= hcaptr->x_coord &&
		hcaptr->x_coord <= endH ) ) {
		cover_allmax = FALSE ;
		break ;
	    }
	}
    } else {
	cover_allmax = FALSE ;
    }

    intersect_max = FALSE ;
    if( 1 <= nk && nk <= numRowsG ) {
	for( i = startH ; i <= endH ; i++ ) {
	    if( HcapacityS[nk][i]->density == crowdmaxS[nk] ) {
		intersect_max = TRUE ;
		break ;
	    }
	}
    }
    penalty_here = 0.0 ;
    penalty_one = 0.0 ;
    penalty_two = 0.0 ;
    for( i = vt_beg ; i <= vt_end ; i++ ) {
	diff_here = feedpptrG[i][h]->actual - feedpptrG[i][h]->needed ;
	if( diff_here < 0 ) {
	    penalty_here-- ;
	}
	if( h > 2 ) {
	    back_two_diff = feedpptrG[i][h-2]->actual 
			  - feedpptrG[i][h-2]->needed ;
	    back_one_diff = feedpptrG[i][h-1]->actual 
			  - feedpptrG[i][h-1]->needed ;
	} else if( h == 2 ) {
	    back_two_diff = 0 ;
	    back_one_diff = feedpptrG[i][h-1]->actual 
			  - feedpptrG[i][h-1]->needed ;
	} else {
	    back_two_diff = 0 ;
	    back_one_diff = 0 ;
	}
	if( h <= chan_node_noG - 2 ) {
	    next_two_diff = feedpptrG[i][h+2]->actual
			  - feedpptrG[i][h+2]->needed ;
	    next_one_diff = feedpptrG[i][h+1]->actual
			  - feedpptrG[i][h+1]->needed ;
	} else if( h == chan_node_noG - 1 ) {
	    next_two_diff = 0 ;
	    next_one_diff = feedpptrG[i][h+1]->actual
			  - feedpptrG[i][h+1]->needed ;
	} else {
	    next_two_diff = 0 ;
	    next_one_diff = 0 ;
	}
	range_one_diff = diff_here + back_one_diff + next_one_diff ;
	range_two_diff = range_one_diff + back_two_diff + next_two_diff;
	if( range_one_diff < 0 ) {
	    penalty_one-- ;
	}
	if( range_two_diff < 0 ) {
	    penalty_two-- ;
	}
	feedpptrG[i][h]->needed-- ;
    }
    for( i = nvt_beg ; i <= nvt_end ; i++ ) {
	feedpptrG[i][nh]->needed++ ;
	diff_here = feedpptrG[i][nh]->actual - feedpptrG[i][nh]->needed ;
	if( diff_here < 0 ) {
	    penalty_here++ ;
	}
	if( nh > 2 ) {
	    back_two_diff = feedpptrG[i][nh-2]->actual 
			  - feedpptrG[i][nh-2]->needed ;
	    back_one_diff = feedpptrG[i][nh-1]->actual 
			  - feedpptrG[i][nh-1]->needed ;
	} else if( nh == 2 ) {
	    back_two_diff = 0 ;
	    back_one_diff = feedpptrG[i][nh-1]->actual 
			  - feedpptrG[i][nh-1]->needed ;
	} else {
	    back_two_diff = 0 ;
	    back_one_diff = 0 ;
	}
	if( nh <= chan_node_noG - 2 ) {
	    next_two_diff = feedpptrG[i][nh+2]->actual
			  - feedpptrG[i][nh+2]->needed ;
	    next_one_diff = feedpptrG[i][nh+1]->actual
			  - feedpptrG[i][nh+1]->needed ;
	} else if( nh == chan_node_noG - 1 ) {
	    next_two_diff = 0 ;
	    next_one_diff = feedpptrG[i][nh+1]->actual
			  - feedpptrG[i][nh+1]->needed ;
	} else {
	    next_two_diff = 0 ;
	    next_one_diff = 0 ;
	}
	range_one_diff = diff_here + back_one_diff + next_one_diff ;
	range_two_diff = range_one_diff + back_two_diff + next_two_diff;
	if( range_one_diff < 0 ) {
	    penalty_one++ ;
	}
	if( range_two_diff < 0 ) {
	    penalty_two++ ;
	}
    }
    if( cover_allmax && intersect_max ) {
	ctrack_penalty = crowdmaxS[nk] - crowdmaxS[k] ;
    } else if( cover_allmax && !intersect_max ) {
	ctrack_penalty = -1 ;
    } else if( !cover_allmax && intersect_max ) {
	ctrack_penalty = 1  ;
    } else {
	ctrack_penalty = 0  ;
    }
	
    penalty = ctrackContS * ctrack_penalty + factor_hereS * penalty_here
	    + factor_oneS * penalty_one + factor_twoS * penalty_two ;
    if( penalty <= 0 ) {
	if( 1 <= k && k <= numRowsG ) {
	    for( i = startH ; i <= endH ; i++ ) {
		denptr = HcapacityS[k][i] ;
		if( denptr->next != NULL ) {
		    denptr->next->prev = denptr->prev ;
		}
		denptr->prev->next = denptr->next ;
		density = --denptr->density ;

		headptr = entryptS[k][density] ;
		if( headptr->next != NULL ) {
		    denptr->next  = headptr->next ;
		    denptr->next->prev = denptr ;
		    denptr->prev  = headptr ;
		    headptr->next = denptr  ;
		} else {
		    headptr->next = denptr  ;
		    denptr->prev  = headptr ;
		    denptr->next  = NULL    ;
		}
	    }
	    if( cover_allmax ) {
		crowdmaxS[k]-- ;
	    }
	}
	if( 1 <= nk && nk <= numRowsG ) {
	    for( i = startH ; i <= endH ; i++ ) {
		denptr = HcapacityS[nk][i] ;
		if( denptr->next != NULL ) {
		    denptr->next->prev = denptr->prev ;
		}
		denptr->prev->next = denptr->next ;
		density = ++denptr->density ;

		// Need to check bounds and reallocate if density has
		// exceeded OVERHEAD.  This should be quite rare.

		if (density >= entrysizeS)
		{
		    INT row, j;

		    for( row = 1 ; row <= numRowsG ; row++ ) {
		        entryptS[row] = (HCAPPTR *)Ysafe_realloc( entryptS[row],
				( entrysizeS + OVERHEAD_DELTA + 1 ) * sizeof( HCAPPTR )) ;
			for( j = entrysizeS + 1; j <= entrysizeS + OVERHEAD_DELTA ; j++ ) {
			    entryptS[row][j] = (HCAPPTR)Ysafe_calloc( 1,sizeof(HCAPBOX)) ;
		        }
		    }
		    entrysizeS += OVERHEAD_DELTA;
		} 

		headptr = entryptS[nk][density] ;
		if( headptr->next != NULL ) {
		    denptr->next  = headptr->next ;
		    denptr->next->prev = denptr ;
		    denptr->prev  = headptr ;
		    headptr->next = denptr  ;
		} else {
		    headptr->next = denptr  ;
		    denptr->prev  = headptr ;
		    denptr->next  = NULL    ;
		}
	    }
	    if( intersect_max ) {
		crowdmaxS[nk]++ ;
	    }
	}
	segptr->switchvalue = new_SwValue ;
    } else {
	for( i = vt_beg ; i <= vt_end ; i++ ) {
	    feedpptrG[i][h]->needed++ ;
	}
	for( i = nvt_beg ; i <= nvt_end ; i++ ) {
	    feedpptrG[i][nh]->needed-- ;
	}
    }
}
}


free_cglb_initial()
{
INT i , j , last_j , row ;

Ysafe_free( node_rightS ) ;
for( i = 1 ; i <= numRowsG ; i++ ) {
    for( j = 1 ; j <= chan_node_noG ; j++ ) {
	Ysafe_free( HcapacityS[i][j] ) ;
    }
    Ysafe_free( HcapacityS[i] ) ;
}
Ysafe_free( HcapacityS ) ;
Ysafe_free( crowdmaxS ) ;
for( row = 1 ; row <= numRowsG ; row++ ) {
    for( j = 0 ; j <= entrysizeS ; j++ ) {
	Ysafe_free( entryptS[row][j] ) ;
    }
    Ysafe_free( entryptS[row] ) ;
}
Ysafe_free( entryptS ) ;
Ysafe_free( swLsegptrS ) ;
}


reinitial_Hdensity()
{

INT i , j ;

for( i = 1 ; i <= numRowsG ; i++ ) {
    for( j = 1 ; j <= chan_node_noG ; j++ ) {
	HcapacityS[i][j]->density = 0 ;
    }
}
}


update_switchvalue()
{

INT net , x , tilted_seg ;
PINBOXPTR ptr1 , ptr2 , netptr ;
SEGBOXPTR segptr ;

LswitchsegS = 0 ;
tilted_seg = 0 ;
for( net = 1 ; net <= numnetsG ; net++ ) {
    for( netptr = steinerHeadG[net]->next;netptr; netptr = netptr->next ){
	netptr->xpos = tearrayG[ netptr->newx ]->xpos ;
	/* update the steiner poINT position according 
	   to the reference pin position.               */
    }
    for( segptr = netsegHeadG[net]->next ; segptr ;
			    segptr = segptr->next ) {
	ptr1 = segptr->pin1ptr ;
	ptr2 = segptr->pin2ptr ;
	x = ABS( ptr1->xpos - ptr2->xpos ) ;
	if( ptr1->row != ptr2->row ) {
	    if( add_Lcorner_feedG && x >= average_feed_sepG ) {
		segptr->flag = FALSE ;
		if( segptr->switchvalue == nswLINE ) {
		    segptr->switchvalue = swL_up ;
		}
	    } else {
		segptr->flag = TRUE ;
		/* if( x >= average_feed_sep ) { */
		if( x ) {
		    if( segptr->switchvalue == nswLINE ) {
			segptr->switchvalue = swL_up ;
		    }
		} else {
		    segptr->switchvalue = nswLINE ;
		}
	    }
	    ++tilted_seg ;
	} else if( ABS( ptr1->pinloc - ptr2->pinloc ) > 1 ) {
	    segptr->flag = FALSE ;
	    if( x >= average_feed_sepG ) {
		if( segptr->switchvalue == nswLINE ) {
		    segptr->switchvalue = swL_up ;
		}
	    } else {
		segptr->switchvalue = nswLINE ;
	    }
	} else {
	    segptr->flag = TRUE ;
	    segptr->switchvalue = nswLINE ;
	}
	if( segptr->switchvalue != nswLINE ) {
	    swLsegptrS[ ++LswitchsegS ] = segptr ;
	}
    }
}
fprintf(fpoG," the number of tilted segment = %d\n", tilted_seg ) ;
fprintf(fpoG," the number of switchable L segment = %d\n", LswitchsegS );
}


rebuild_cbucket()
{
INT row , j , last_j ;
HCAPPTR hcaptr , headptr ;

for( row = 1 ; row <= numRowsG ; row++ ) {
    crowdmaxS[row] = 0 ;
    for( j = 0 ; j <= entrysizeS ; j++ ) {
	entryptS[row][j]->next = NULL ;
    }
}
for( row = 1 ; row <= numRowsG ; row++ ) {
    for( j = 1 ; j <= chan_node_noG ; j++ ) {
	hcaptr = HcapacityS[row][j] ;

	// Watch for density exceeding OVERHEAD and increase
	// array sizes accordingly.

	if (hcaptr->density > entrysizeS) {
	    INT lrow, k;

	    for( lrow = 1 ; lrow <= numRowsG ; lrow++ ) {
	        entryptS[lrow] = (HCAPPTR *)Ysafe_realloc( entryptS[lrow],
			( entrysizeS + OVERHEAD_DELTA + 1 ) * sizeof( HCAPPTR )) ;
		for( k = entrysizeS + 1; k <= entrysizeS + OVERHEAD_DELTA ; k++ ) {
		    entryptS[lrow][k] = (HCAPPTR)Ysafe_calloc( 1,sizeof(HCAPBOX)) ;
	        }
	    }
	    entrysizeS += OVERHEAD_DELTA;
	} 

	headptr = entryptS[row][ hcaptr->density ] ;
	if( headptr->next != NULL ) {
	    hcaptr->next  = headptr->next ;
	    hcaptr->next->prev = hcaptr ;
	    hcaptr->prev  = headptr ;
	    headptr->next = hcaptr ;
	} else {
	    headptr->next = hcaptr ;
	    hcaptr->prev = headptr ;
	    hcaptr->next = NULL ;
	}
	if( hcaptr->density > crowdmaxS[row] ) {
	    crowdmaxS[row] = hcaptr->density ;
	}
    }
}
}


#ifdef DEBUG
check_cbucket()
{
INT row, j ;
HCAPPTR dptr , denptr ;

for( row = 1 ; row <= numRowsG ; row++ ) {
    for( j = 1 ; j <= chan_node_noG ; j++ ) {
	denptr = HcapacityS[row][j] ;
	for( dptr = entryptS[row][denptr->density]->next ;
				dptr ; dptr = dptr->next ) {
	    if( dptr == denptr ) {
		break ;
	    }
	}
	if( dptr == NULL ) {
	    printf(" something is going wrong\n" ) ;
	    abort() ;
	}
    }
}
}

print_bucket( row )
INT row ;
{
INT j ;
HCAPPTR denptr ;
FILE *fp ;

fp = TWOPEN("bucket.dat", "a", ABORT ) ;
fprintf(fp, "\n ROW = %d\n", row ) ;
fprintf(fp, " sizeof densitybox = %d\n", sizeof(HCAPBOX) ) ;
fprintf(fp, " %d %d\n", HcapacityS[row][1], HcapacityS[row][2] ) ;
fprintf(fp," density  cbin       denptr      nextptr      prevptr\n" ) ;
for( j = 1 ; j <= chan_node_noG ; j++ ) {
    denptr = HcapacityS[row][j] ;
    fprintf(fp," %7d %5d %12x %12x %12x\n", denptr->density,
    denptr->x_coord, denptr, denptr->next, denptr->prev ) ;
}
fprintf(fp,"\n\n" ) ;
for( j = crowdmaxS[row] ; j >= 0 ; j-- ) {
    denptr = entryptS[row][j]->next ;
    if( denptr == NULL ) continue ;
    fprintf(fp,"\n       denptr density x_coord\n" ) ;
    for( ; denptr ; denptr = denptr->next ) {
	fprintf(fp, " %12x %7d %7d\n",
	    denptr, denptr->density, denptr->x_coord ) ;
    }
}
TWCLOSE(fp) ;
}
#endif
