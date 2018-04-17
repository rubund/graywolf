/*
 *   Copyright (C) 1989-1992 Yale University
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
FILE:	    parser.c                                       
DESCRIPTION:syntax checker.
CONTENTS:   parser( fp )
		FILE *fp ;
DATE:	    Mar 27, 1989 
REVISIONS:  Mon Nov 19 04:04:18 EST 1990 - added temperature
		schedule initialization.
	    Nov 23, 1990 - added correct drawing for rectilinear
		cells.
	    Dec  8, 1990 - make hash table more efficient for nets.
	    Dec 11, 1990 - fixed problem with output orientations
		for macro cells in new format.
	    Sat Dec 15 22:08:21 EST 1990 - modified pinloc values
		so that it will always be positive.
	    Wed Jan  2 10:52:13 CST 1991 - removed extraneous
		code from initial placement phase.
	    Mon Jan  7 18:32:58 CST 1991 - fixed pad translation
		problem and overlap check.
	    Tue Jan 15 20:22:37 PST 1991 - fixed problem with
		fix_placement due to fixed_type and fixed_loc bug.
	    Fri Jan 25 18:54:39 PST 1991 - fixed declaration of rowsG
		and fixed eqptr problem.  Now also allow layer 0.
	    Tue Feb  5 18:57:49 EST 1991 - fixed problem with
		pads with no side restrictions.
	    Thu Feb 14 02:43:43 EST 1991 - modified for new pad
		placement algorithm.
	    Fri Feb 15 15:18:54 EST 1991 - fixed overwrite of
		placearrayG.
	    Sun Feb 17 21:08:19 EST 1991 - fixed pad placement code
		in the presence of macro cells.
	    Thu Mar 14 16:17:10 CST 1991 - now handle both views for 
		the pads.
	    Fri Mar 15 18:34:08 CST 1991 - fixed problem with macros
		and padgroups using the old .blk format.
	    Thu Apr 18 01:51:24 EDT 1991 - added initial orient
		feature and renamed to the new library names.
	    Wed Jul 24 17:53:44 CDT 1991 - removed problem with
		excepts.
	    Thu Aug 22 22:15:03 CDT 1991 - fixed some error checking.
	    Thu Aug 22 22:34:00 CDT 1991 - Carl added some missing
		lines for fences and rigidly fixed cells.
	    Wed Aug 28 15:02:02 EDT 1991 - fixed problem with
		missing unequiv switch.
	    Wed Sep 11 11:21:42 CDT 1991 - fixed for
		new global router feed problem.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) parser.c (Yale) version 4.40 5/15/92" ;
#endif

/* #define MITLL */

#define PARSER_VARS

#include <string.h>
#include "standard.h"
#include "groute.h"
#include "parser.h"
#include "main.h"
#include "readblck.h"
#include "readpar.h"
#include "config.h"
#include "feeds.h"
#include "pads.h"
#include <yalecad/hash.h>
#include <yalecad/file.h>
#include <yalecad/message.h>
#include <yalecad/relpos.h>
#include <yalecad/string.h>
#include <yalecad/debug.h>

/* global variables */
INT ECOs_existG = 0 ;
INT orig_max_row_lengthG ;

extern INT totalRG ;
extern INT spacer_widthG ;
extern INT *spacer_feedsG ;
extern INT uniform_rowsG ;
extern INT total_row_lengthG ;
extern INT total_except_widthG ;
extern INT approximately_fixed_factorG ;
extern BOOL fences_existG ;
extern BOOL turn_off_checksG ;
extern BOOL spacer_name_twfeedG ;
extern BOOL rigidly_fixed_cellsG ;
extern BOOL stand_cell_as_gate_arrayG ;

/* below is what we expect to be a typical standard cell input */
/* user may change parameters if they wish. Subject to change */
#define EXPECTEDNUMCELLS  3500
#define EXPECTEDNUMNETS   1000
#define EXPECTEDSWAPGROUPS  10
#define EXPECTEDNUMPADS     10
#define E                    0

/* ERRORABORT is a macro which forces routines not to do any work */
/* when we find a syntax error in the input routine. */
#define ERRORABORT() \
{ \
    if( abortS ){ \
	return ; /* don't do any work for errors */ \
    } \
} \

/* ##########################  STATIC definitions ########################### */
static INT  curCellTypeS ;        /* current cell type - ie, stdcell,pad etc. */
static INT  curNetS ;             /* current net number                       */
static INT  netAllocS ;           /* current space in netarray                */
static INT  cellAllocS ;          /* current space in cellarray               */
static INT  ptAllocS ;            /* current space in point arrays            */
static INT  swapAllocS ;          /* current space in swap_group_listG        */
static INT  totalCellS ;          /* current cell number and running total    */
static INT  swap_groupS ;         /* current swap group number from hashtable */
static INT  swap_nextS ;          /* next swap group number to assign	      */
static INT  pin_typeS ;           /* current pin type                         */
static INT  numchildrenS ;        /* # of children in current padgroup        */
static INT  childAllocS ;         /* current space in current pptr->children  */
static INT  minxS, maxxS ;        /* X bounding box for cell or pad           */
static INT  minyS, maxyS ;        /* Y bounding box for cell or pad           */
static char *curCellNameS ;       /* current cell name                        */
static BOOL abortS = FALSE ;      /* switch allow us to find multiple errors  */
static INT pin_group_light_is_onS;/* currently in a pin group at position cnt */
static BOOL need_swap_groupS;     /* pin_group called without swap_group      */
static BOOL old_pad_formatS = FALSE;/* we need to do more work if old format  */
static CBOXPTR ptrS ;             /* pointer to current cell box              */
static PINBOXPTR pinptrS ;        /* the current pin                          */
static PADBOXPTR pptrS ;          /* the current pad box record               */
static IPBOXPTR imptrS ;          /* current implicit feed ptr                */
static YHASHPTR net_hash_tableS ; /* hash table for cross referencing nets    */
static YHASHPTR swap_hash_tableS ;/* hash table for referencing swap_groups   */
static INT transTableS[5][8] = {  /* translate from old pad format */
    { E, E, E, E, E, E, E, E   },   /* error state */
    { 6, 5, 4, 7, 1, 2, 3, 0   },   /* PADL state */
    { 1, 0, 3, 2, 6, 7, 4, 5   },   /* PADT state */
    { 7, 4, 5, 6, 2, 1, 0, 3   },   /* PADR state */
    { 0, 1, 2, 3, 4, 5, 7, 6   }    /* PADB state */
} ;


static layer_test();
static check_pin();

/* ###################### END STATIC definitions ############################ */
static get_stat_hints()
{
    FILE *fp ;           /* current file */
    char buffer[LRECL] ; /* temp storage */
    char *bufferptr ;    /* pointer to start of the line */
    char **tokens ;      /* tokenized line */
    INT numtokens ;      /* number of tokens on the line */


    sprintf( buffer, "%s.stat", cktNameG ) ;
    if( fp = TWOPEN( buffer, "r", NOABORT ) ){
	/* parse the hints */
	cellAllocS = 0 ;
	while( bufferptr = fgets( buffer, LRECL, fp )){
	    tokens = Ystrparser( bufferptr, ":\t\n", &numtokens ) ;
	    if( numtokens != 2 ){
		continue ;
	    } else if( strcmp( tokens[0],"num_stdcells") == STRINGEQ ){
		cellAllocS += atoi( tokens[1] ) ;
		if( cellAllocS <= 0 ){
		    /* reset if bogus */
		    cellAllocS = EXPECTEDNUMCELLS ;
		}
	    } else if( strcmp( tokens[0],"num_nets") == STRINGEQ ){
		netAllocS = atoi( tokens[1] ) ;
		if( netAllocS <= 0 ){
		    /* reset if bogus */
		    netAllocS = EXPECTEDNUMNETS ;
		}
	    /* look for macros and pads normally - ports if new format */
	    } else if( strcmp( tokens[0],"num_macros") == STRINGEQ ){
		if( rowsG == 0 ){
		    cellAllocS += atoi( tokens[1] ) ;
		}
	    } else if( strcmp( tokens[0],"num_pads") == STRINGEQ ){
		if( rowsG == 0 ){
		    cellAllocS += atoi( tokens[1] ) ;
		}
	    } else if( strcmp( tokens[0],"num_ports") == STRINGEQ ){
		if( rowsG != 0 ){
		    cellAllocS += atoi( tokens[1] ) ;
		}
	    }
	}
	M( MSG, NULL, "\n" ) ;
	sprintf( YmsgG, "Found hints in <%s.stat> file\n", cktNameG ) ;
	M( MSG, "get_stat_hints", YmsgG ) ;
	/* add +1 afterward to account for the 0 record */
	sprintf( YmsgG, "Total cells:%d Number of nets:%d\n\n",cellAllocS++,netAllocS++) ;
	M( MSG, "get_stat_hints", YmsgG ) ;
    } else {
	cellAllocS = EXPECTEDNUMCELLS ;
	netAllocS = EXPECTEDNUMNETS ;
    }
    /* now allocate the data structures */
    netarrayG = (DBOXPTR *) Ysafe_malloc( netAllocS * sizeof( DBOXPTR ) ) ;
    carrayG = (CBOXPTR *) Ysafe_malloc( cellAllocS * sizeof( CBOXPTR ) ) ;
    swap_group_listG = (SWAPBOX *) Ysafe_calloc( 1, sizeof(SWAPBOX) ) ;

} /* end get_stat_hints */

set_error_flag()
{
    abortS = TRUE ;
} /* set_error_flag */

initialize_parser()
{
    INT except ;          /* counter */
    INT width ;           /* width of exception */
    INT height ;          /* height of exception */
    EXCEPTPTR exceptptr ; /* current exception */

    numcellsG = 0 ;
    numtermsG = 0 ;
    numpadgrpsG = 0 ;
    numMacroG = 0 ;
    TotRegPinsG = 0 ;
    numnetsG = 0 ;
    implicit_feed_countG = 0 ;
    totalCellS = 0 ;
    swap_groupS = 0 ;
    swap_nextS = 0 ;
    pin_group_light_is_onS = 0 ;
    need_swap_groupS = FALSE ;

    fixLRBTG  = (INT *) Ysafe_malloc( 4 * sizeof( INT ) ) ;
    fixLRBTG[0] = 0 ;
    fixLRBTG[1] = 0 ;
    fixLRBTG[2] = 0 ;
    fixLRBTG[3] = 0 ;
    /* set the initial datastructure sizes */
    get_stat_hints() ;

    net_hash_tableS = Yhash_table_create( 2 * netAllocS ) ;
    swap_hash_tableS = Yhash_table_create( TW_PRIME2 ) ;


    if( num_exceptsG > 0 ) {
	rigidly_fixed_cellsG = TRUE ;
    }

    /*   add exceptionsG into carrayG at this time  */
    for( except = 1 ; except <= num_exceptsG ; except++ ) {
	addCell( Ystrclone("TW_EXCEPT"), EXCEPTTYPE ) ;
	exceptptr = &(exceptionsG[except]) ;
	/* now override the defaults */
	ptrS->cclass = -5 ;
	ptrS->orflag = 0 ;
	ptrS->cblock = exceptptr->row ;
	ptrS->cxcenter = exceptptr->ll_x - 
		(barrayG[ ptrS->cblock ]->bxcenter +
		 barrayG[ ptrS->cblock ]->bleft) ;
	ptrS->cycenter = 1 ;
	width  = exceptptr->ur_x - exceptptr->ll_x ;
	height = exceptptr->ur_y - exceptptr->ll_y ;
	ptrS->border = exceptptr->ll_x -
		(barrayG[ ptrS->cblock ]->bxcenter +
		 barrayG[ ptrS->cblock ]->bleft) ;
	/* add_tile( left, bottom, right, top */
	add_tile( -width/2, -height/2, width-width/2, height-height/2 ) ;
    }
    Ysafe_free( exceptionsG ) ;

} /* initialize_parser */

addCell( cellname, celltype )
char *cellname ;
INT celltype ;
{
    /* save current cell name and type for error messages */
    curCellNameS = cellname ;
    curCellTypeS = celltype ;

    /* Undefine pin and swap groups, if used in the last cell */
    pin_group_light_is_onS = 0 ;
    need_swap_groupS = FALSE ;

    totalCellS++ ;

    ERRORABORT() ;
    /* make sure allocation is ok */
    if( totalCellS >= cellAllocS ){
	cellAllocS += EXPECTEDNUMCELLS ;
	carrayG = (CBOXPTR *) Ysafe_realloc( carrayG, 
	    cellAllocS * sizeof( CBOXPTR ) ) ;
    }
    ptrS = carrayG[totalCellS] = (CBOXPTR) Ysafe_malloc( sizeof(CBOX) ) ;

    /* now initialize the data */
    ptrS->cname = cellname ; /* allocated previously in lex */
    ptrS->cxcenter = 0 ;
    ptrS->cycenter = 0 ;
    ptrS->cheight  = 0 ;
    ptrS->clength  = 0 ;
    ptrS->cblock   = 0 ;
    ptrS->border   = 0 ;
    ptrS->ECO_flag = 0 ;
    ptrS->cclass = 0 ;
    ptrS->orflag   = 1 ;
    ptrS->cbclass[0]  = 0 ;
    ptrS->cbclass[1]  = 0 ;
    ptrS->cbclass[2]  = 0 ;
    ptrS->cbclass[3]  = 0 ;
    ptrS->cbclass[4]  = 0 ;
    ptrS->cbclass[5]  = 0 ;
    ptrS->cbclass[6]  = 0 ;
    ptrS->cbclass[7]  = 0 ;
    ptrS->corient  = 0 ;
    ptrS->numterms = 0 ;
    ptrS->num_swap_group = 0 ;
    ptrS->swapgroups = NULL ;
    ptrS->fence    = NULL ;
    ptrS->paths    = NULL ;
    ptrS->tileptr  = TILENULL ;
    ptrS->imptr    = NULL     ;
    ptrS->pins     = NULL     ;
    ptrS->padptr  = NULL ;

    if( celltype == STDCELLTYPE || celltype == EXCEPTTYPE ){
	numcellsG++ ;
    } else if( celltype == PADTYPE || celltype == PORTTYPE || 
	celltype == PADGROUPTYPE || celltype == HARDCELLTYPE ){
	ptrS->cclass = 0 ;
	ptrS->orflag = 0 ;
	numtermsG++ ;
	pptrS = ptrS->padptr = (PADBOXPTR) Ysafe_calloc( 1, sizeof(PADBOX) ) ;
	pptrS->fixed = FALSE ;
	pptrS->padside = ALL ;
	pptrS->permute = FALSE ;
	pptrS->ordered = FALSE ;
	pptrS->oldformat = FALSE ;
	pptrS->lowerbound  = 0.0 ;
	pptrS->upperbound  = 1.0 ;
	pptrS->macroNotPad = FALSE ; /* assume a pad */
	pptrS->numcorners = 0 ;
	pptrS->padtype = celltype ;
	pptrS->cellnum = totalCellS ;
	pptrS->valid_side[0] = TRUE ;
	pptrS->valid_side[1] = FALSE ;
	pptrS->valid_side[2] = FALSE ;
	pptrS->valid_side[3] = FALSE ;
	pptrS->valid_side[4] = FALSE ;

	if( celltype == PADGROUPTYPE ){
	    numpadgrpsG++ ;
	    numchildrenS = 0 ;
	    childAllocS = EXPECTEDNUMPADS ;
	    pptrS->children = (INT *)
		Ysafe_malloc((childAllocS)*sizeof(INT));
	    pptrS->hierarchy  = ROOT;
	} else { /* PADTYPE or PORTTYPE */
	    pptrS->children = NULL;
	    pptrS->hierarchy     = NONE;
	}
	if( celltype == HARDCELLTYPE ){
	    /* set the padside to MMC to avoid processing in placepads */
	    pptrS->padside = MMC ;
	    pptrS->macroNotPad = TRUE ;
	    numMacroG++ ;
	}
	
    }

} /* end addCell */

add_tile( left, bottom, right, top )
INT left, bottom, right, top ;
{
    INT width ;         /* width of tile */
    INT height ;        /* height of tile */
    TIBOXPTR tile ;     /* current tile */

    if( (right + left != 0  &&  right + left != 1) || left > 0 ) {
	M( ERRMSG, "add_tile", "The given center of the cell ");
	M( ERRMSG, NULL, "is not (right + left) div 2\n");
	M( ERRMSG, NULL, "Where the remainder is truncated - ");
	sprintf( YmsgG, "Current cell number:%d  Name:<%s>\n\n", 
	    totalCellS , curCellNameS ) ;
	M( ERRMSG, NULL, YmsgG ) ;
	abortS = TRUE ;
    }
    if( (top + bottom != 0 && top + bottom != 1) || bottom > 0 ) {
	M( ERRMSG, "add_tile", "The given center of the cell ");
	M( ERRMSG, NULL, "is not (top + bottom) div 2\n");
	M( ERRMSG, NULL, "Where the remainder is truncated - ");
	sprintf( YmsgG, "current cell number:%d  Name:<%s>\n\n", 
	    totalCellS , curCellNameS ) ;
	M( ERRMSG, NULL, YmsgG ) ;
	abortS = TRUE ;
    }
    ERRORABORT() ;

    /* add tile to datastructures */
    tile = ptrS->tileptr = (TIBOXPTR) Ysafe_malloc( sizeof(TIBOX) ) ;
    tile->left = left ;
    tile->right = right ;
    tile->bottom = bottom ;
    tile->top = top ;
    width = right - left ;
    height = top - bottom ;
    ptrS->cheight = height ;
    ptrS->clength = width ;
    if( curCellTypeS == STDCELLTYPE ){
	celllenG += width ;
    }
    if( gate_arrayG ){
	if( curCellTypeS == STDCELLTYPE || curCellTypeS == EXCEPTTYPE ||
	    curCellTypeS == EXTRATYPE ) {
	    if( width % spacer_widthG != 0 ) {
		sprintf( YmsgG, 
		"the width of cell <%s> is not a multiple of the spacer_width\n",
		ptrS->cname ) ;
		M( ERRMSG, "add_tile", YmsgG ) ;
		abortS = TRUE ;
	    }
	}
    }

} /* end add_tile */

add_initial_orient( orient )
INT orient ;
{

    ERRORABORT() ;
    if( orient >= 0 && orient <= 3 ){
	ptrS->corient = orient ;
    } else {
	sprintf( YmsgG, 
	"initial orientation is not in the range [0..3] for cell <%s>\n",
	    ptrS->cname ) ;
	M( ERRMSG, "add_initial_orient", YmsgG ) ;
	abortS = TRUE ;
    }
} /* end add_initial_orient */

static char *add_swap_func()
{
    INT *data ;   /* pointer to allocated space for swap_group record */

    ERRORABORT() ;

    /* how to add the data to the hash table */
    /* create space for data */
    data = (INT *) Ysafe_malloc( sizeof(INT) ) ;
    /* if data is not found in hash table update swap_groupS */
    *data = ++swap_nextS ;
    return( (char *) data ) ;
} /* end add_swap_func */

add_swap_group( swap_name )
char *swap_name ;
{
    INT i ;            /* counter */
    INT *groupptr ;    /* the return group from searching in hash table */
    INT oldalloc ;     /* allocation before expanding the array */
    BOOL newflag ;     /* TRUE if this item has been added to hash table */
    SWAPBOX *swapptr ; /* pointer to the current swap group list */
    SGLIST *sglistptr ;

    ERRORABORT() ;
    swappable_gates_existG = TRUE ;
    groupptr = (INT *)Yhash_add( swap_hash_tableS, swap_name, add_swap_func, &newflag ) ;
    /* check to make sure all is well. newflag is set if swap_name wasn't in hashtable */
    if(!(groupptr) || (*groupptr <= 0)) {
	sprintf(YmsgG, "Problem with swap hash table for group <%s>\n", swap_name );
	M( ERRMSG, "add_swap_group", YmsgG ) ;
	abortS = TRUE ;
    }
    swap_groupS = *groupptr;

    /* Check if swapgroups exists.  Create it if not. */

    if (ptrS->swapgroups == NULL) {
	ptrS->swapgroups = (SGLIST *) Ysafe_malloc( sizeof( SGLIST ) ) ;
	ptrS->num_swap_group = 1;
	sglistptr = ptrS->swapgroups;
	sglistptr->num_pin_group = 0;
	sglistptr->swap_group = swap_groupS;
    }
    else {

	/* Does this swap group already exist in the list?  If	*/
	/* so, then ignore it, otherwise add it to the list.	*/

	for (i = 0; i < ptrS->num_swap_group; i++) {
	    sglistptr = ptrS->swapgroups + i;
	    if (sglistptr->swap_group == swap_groupS) break;
	}
	if (i == ptrS->num_swap_group) {
	    ptrS->num_swap_group++;
	    ptrS->swapgroups = (SGLIST *) Ysafe_realloc( ptrS->swapgroups,
		ptrS->num_swap_group * sizeof( SGLIST ) ) ;
	    sglistptr = ptrS->swapgroups + i;
	    sglistptr->swap_group = swap_groupS;	   
	    sglistptr->num_pin_group = 0;
	}
    }

    if (newflag && need_swap_groupS) {
	sprintf(YmsgG, "Implicit swap group <%s> created\n", swap_name );
	M( MSG, "add_swap_group", YmsgG ) ;
    }

    if( newflag && swap_groupS >= swapAllocS ){
	oldalloc = swapAllocS ;
	swapAllocS += EXPECTEDSWAPGROUPS ;
	swap_group_listG = (SWAPBOX *) Ysafe_realloc( swap_group_listG,
	    swapAllocS * sizeof( SWAPBOX ) ) ;
	for( i = oldalloc; i < swapAllocS; i++ ){
	    swapptr = &(swap_group_listG[i]) ;
	    swapptr->num_pin_grps = 0 ;
	    swapptr->pin_grp_hash = Yhash_table_create( TW_PRIME2 );
	}
    }

} /* end add_swap_group */

add_pingroup()
{
    INT i ;    /* counter */
    INT j ;    /* counter */
    SGLIST *sglistptr = NULL;

    ERRORABORT() ;

    /* Find the swap group in the cell record */
    for (i = 0; i < ptrS->num_swap_group; i++) {
       sglistptr = ptrS->swapgroups + i;
       if (sglistptr->swap_group == swap_groupS)
	  break;
    }
    if (i == ptrS->num_swap_group) {
	// If no swap group was defined for the cell, then
	// each pin_group will implicitly define a swap
	// group with the name of the first pin before the
	// slash character.
	need_swap_groupS = TRUE;
	return;
    }

    sglistptr->num_pin_group++ ;
    pin_group_light_is_onS = 1 ;

} /* end add_pingroup */

end_pingroup()
{
    pin_group_light_is_onS = 0 ;

    // Reset the swap_group if each pin group defines its own
    // swap group (enabled by not having a "swap_group" line in
    // the cell options).
    if (need_swap_groupS == TRUE) swap_groupS = 0;
} /* end end_pingroup */

static add_implicit_feed( pin_name, signal, layer, xpos, ypos )
char *pin_name, *signal ;
INT layer, xpos, ypos ;
{

    ERRORABORT() ;
    imptrS = ( IPBOXPTR )Ysafe_malloc( sizeof( IPBOX ) ) ;
    /* now fill in data */
    if( pin_layers_givenG ) {
	layer_test( layer ) ;
	imptrS->pinname = (char *) Ysafe_malloc(
		(strlen( pin_name ) + 3 ) * sizeof( char ) ) ;
	sprintf( imptrS->pinname , "]%1d%s" , layer, pin_name ) ;
	Ysafe_free( pin_name ) ;

    } else {
	imptrS->pinname = pin_name ;
    }
    imptrS->txpos = xpos ;
    imptrS->cell  = totalCellS ;
    imptrS->terminal = 0 ; /* wait till later to fill in */
    imptrS->next = ptrS->imptr ;
    ptrS->imptr = imptrS ;
    if( one_pin_feedthruG ) {
	if( pin_layers_givenG ) {
	    imptrS->eqpinname = (char *) Ysafe_malloc(
		    (strlen(pin_name) + 3 ) * sizeof( char ) ) ;
	    sprintf(imptrS->eqpinname,"]%1d%s", layer, pin_name ) ;
	} else {
	    imptrS->eqpinname = Ystrclone( pin_name ) ;
	}
    }
} /* end add_implicit_feed */

static char *add_net_func()
{
    INT *data ;   /* pointer to allocated space for net record in hashtable */

    ERRORABORT() ;
    /* how to add the data to the hash table */
    /* create space for data */
    data = (INT *) Ysafe_malloc( sizeof(INT) ) ;
    /* if data is not found in hash table update numnetsG */
    *data = ++numnetsG ;
    return( (char *) data ) ;
} /* end add_swap_func */

static char *add_pin_func()
{
    INT *data ;   /* pointer to allocated space for pin_grp_hash record */

    ERRORABORT() ;

    /* how to add the data to the hash table */
    /* create space for data */
    data = (PINLIST *) Ysafe_malloc( sizeof(PINLIST) ) ;
    return( (char *) data ) ;
} /* end add_swap_func */

add_pin( pin_name, signal, layer, xpos, ypos )
char *pin_name, *signal ;
INT layer, xpos, ypos ;
{
    INT *netreturn ;          /* net number found in hash table */
    INT newflag ;
    BOOL notInTable ;         /* net added to table if true */
    DBOXPTR nptr ;            /* the current net record */
    PINLISTPTR pin_ptr ;      /* pointer to current pinlistgroup */
    PINLISTPTR new_pin_ptr ;  /* pointer to new pinlistgroup */
    static PINBOXPTR botpinL = NULL ; /* save the last pinptr */

    ERRORABORT() ;

    // If a swap group was not defined for this cell in the
    // traditional way (with "swap_group" in cell options list),
    // but pin_group was called, then we generate an implicit
    // swap_group with the name of the pin before the slash.

    if (need_swap_groupS) {
	char *slashptr = strrchr(pin_name, '/');
	if (slashptr != NULL) {
	   *slashptr = '\0';
	   add_swap_group(pin_name);
	   *slashptr = '/';
	   add_pingroup();
	}
	/* In case of error this is just a normal pin, not	*/
	/* swappable, which may result in non-optimal		*/
	/* placement but does not invalidate the netlist.	*/
    }

    if( curCellTypeS == PADTYPE || curCellTypeS == HARDCELLTYPE ){
	/* these types may have global pin positions */
	/* subtract off cell center */
	xpos -= ptrS->cxcenter ;
	ypos -= ptrS->cycenter ;
    }
    check_pin( xpos, ypos, pin_name ) ;

    if( strcmp( signal, "TW_PASS_THRU" ) == STRINGEQ ){
	pin_typeS = PASS_THRU ;
	implicit_feed_countG++ ;
	add_implicit_feed( pin_name, signal, layer, xpos, ypos ) ;
	return ;
    } else if( strcmp( signal, "TW_SWAP_PASS_THRU" ) == STRINGEQ ){
	pin_typeS = SWAP_PASS ;
	implicit_feed_countG++ ;
    } else {
	pin_typeS = PINTYPE ;
    }

    ptrS->numterms++ ;
    /* add to netlist */
    netreturn = (INT *)Yhash_add( net_hash_tableS, signal, add_net_func, &notInTable ) ;
    /* check to make sure all is well. notInTable is set if signal wasn't in hashtable */
    if(!(netreturn)){
	sprintf( YmsgG, "Trouble adding signal:%s to hash table\n", signal ) ;
	M(ERRMSG,"add_pin",YmsgG ) ;
	abortS = TRUE ;
	return ;
    }
    curNetS = *netreturn ;
    /* check memory of netarray */
    if( numnetsG >= netAllocS ){
	netAllocS += EXPECTEDNUMNETS ;
	netarrayG = (DBOXPTR *) 
	    Ysafe_realloc( netarrayG, netAllocS * sizeof(DBOX) ) ;
    }

    /* see if this is the first time for this signal */
    if( notInTable ){
	nptr = netarrayG[curNetS] = (DBOXPTR) Ysafe_malloc( sizeof( DBOX ) ) ;

	nptr->pins  = PINNULL ;
	nptr->name    = signal  ;
	nptr->paths   = NULL    ;
	nptr->dflag   = 0       ;
	nptr->xmin    = 0       ;
	nptr->newxmin = 0       ;
	nptr->xmax    = 0       ;
	nptr->newxmax = 0       ;
	nptr->ymin    = 0       ;
	nptr->newymin = 0       ;
	nptr->ymax    = 0       ;
	nptr->newymax = 0       ;
	nptr->feedflag = 0      ;
	nptr->ignore   = 0      ;
    } else {
	Ysafe_free( signal ) ; /* no need to keep this copy */
	nptr = netarrayG[curNetS] ;
    }

    /* create list of pins on the net */
    pinptrS = (PINBOXPTR) Ysafe_malloc( sizeof(PINBOX) );
    pinptrS->next = nptr->pins ;
    nptr->pins = pinptrS ;

    /* create a list of pins for this cell in order as given */
    if( ptrS->pins ){ /* list has already been started */
	botpinL->nextpin = pinptrS ;
    } else {  /* start new list */
	ptrS->pins = pinptrS ;
    }
    pinptrS->nextpin = NULL ;
    botpinL = pinptrS ;

    /* now initialize data */
    pinptrS->terminal = ++TotRegPinsG ;
    pinptrS->net      = curNetS ;
    pinptrS->xpos     = 0    ;
    pinptrS->ypos     = 0    ;
    pinptrS->newx     = 0    ;
    pinptrS->newy     = 0    ;
    pinptrS->flag     = 0    ;
    pinptrS->row     = (SHORT) 0 ;
    pinptrS->cell = totalCellS ;
    pinptrS->eqptr    = NULL ;
    pinptrS->adjptr   = (ADJASEGPTR)Ysafe_calloc( 1, sizeof(ADJASEG) ) ;

    if( pin_layers_givenG ) {
	layer_test( layer ) ;
	pinptrS->pinname = (char *) Ysafe_malloc(
		(strlen( pin_name ) + 3 ) * sizeof( char ) ) ;
	sprintf( pinptrS->pinname , "]%1d%s" , layer, pin_name ) ;
	Ysafe_free( pin_name ) ;

    } else {
	pinptrS->pinname = pin_name ;
    }

    pinptrS->txpos[0] = xpos ;
    pinptrS->typos[0] = ypos ;

    if( pin_group_light_is_onS > 0 ) {

	pin_ptr = (PINLISTPTR) Yhash_add( swap_group_listG[swap_groupS].pin_grp_hash,
		ptrS->cname, add_pin_func, &newflag ) ;

	if (newflag) {
	   /* This is the first pin group for this swap group in this cell */
	   pin_ptr->swap_pin = pinptrS;
	   pin_ptr->next = NULL;
	   pin_ptr->next_grp = NULL;
	}
	else {
	   if (pin_group_light_is_onS == 1) {
	      /* Additional pin group for this swap group in this cell */
	      while (pin_ptr->next_grp) pin_ptr = pin_ptr->next_grp;
	      new_pin_ptr = (PINLISTPTR) Ysafe_malloc( sizeof(PINLIST) ) ;
	      pin_ptr->next_grp = new_pin_ptr;
	   }
	   else {
	      /* Additional pins in this pin group */
	      while (pin_ptr->next) pin_ptr = pin_ptr->next;
	      new_pin_ptr = (PINLISTPTR) Ysafe_malloc( sizeof(PINLIST) ) ;
	      pin_ptr->next = new_pin_ptr;
	   }
	   new_pin_ptr->swap_pin = pinptrS;
	   new_pin_ptr->next = NULL;
	   new_pin_ptr->next_grp = NULL;
	}

	if( pin_typeS == SWAP_PASS ){
	    swap_netG = curNetS ;
	    netarrayG[curNetS]->ignore = 1 ;
	}

	pin_group_light_is_onS++;
    }

    /* pin location determination */
    if( curCellTypeS == PADTYPE ) {
	if( pptrS->padside == L ) {
	    pinptrS->pinloc = TOPCELL ;
	} else if( pptrS->padside == R ) {
	    pinptrS->pinloc = TOPCELL ;
	} else if( pptrS->padside == MUL ) {
	    pinptrS->pinloc = BOTCELL ;
	} else if( pptrS->padside == MUR ) {
	    pinptrS->pinloc = BOTCELL ;
	} else if( pptrS->padside == MLL ) {
	    pinptrS->pinloc = TOPCELL ;
	} else if( pptrS->padside == MLR ) {
	    pinptrS->pinloc = TOPCELL ;
	} else if( pptrS->padside == ML ) {
	    pinptrS->pinloc = TOPCELL ;
	} else if( pptrS->padside == MR ) {
	    pinptrS->pinloc = TOPCELL ;
	} else {
	    pinptrS->pinloc = NEITHER ;
	}
    } else if( curCellTypeS == PORTTYPE || curCellTypeS == HARDCELLTYPE ) {
	pinptrS->pinloc =  NEITHER ;
    } else if( ypos > 0 ) {
	pinptrS->pinloc =  TOPCELL ;
    } else {
	pinptrS->pinloc = BOTCELL ;
    }

} /* end add_pin */

static check_pin( xpos, ypos, pinname )
INT xpos, ypos ;
char *pinname ;
{
    INT layer ;
    char *find_layer() ;
    TIBOXPTR tptr ;   /* current tileptr */

    ERRORABORT() ;
    tptr = ptrS->tileptr ;
    if( !turn_off_checksG ) {
	if( xpos < tptr->left || xpos > tptr->right || 
				ypos < tptr->bottom || ypos > tptr->top ) {
	    pinname = find_layer( pinname, &layer ) ;
	    sprintf( YmsgG, "the pin <%s> assigned to cell <%s> is\n",
			    pinname, curCellNameS ) ;
	    M( ERRMSG, "check_pin", YmsgG ) ;
	    M( ERRMSG, NULL, "outside the cell's bounding box:\n" ) ;
	    sprintf( YmsgG, "\tcell l:%d r:%d b:%d t:%d  pin x:%d y:%d\n",
		tptr->left, tptr->right, tptr->bottom, tptr->top,
		xpos, ypos ) ;
	    M( ERRMSG, NULL, YmsgG ) ;
	    abortS = TRUE ;
	}
    }
} /* end check_pin */

add_equiv( equiv_name, layer, eq_xpos, eq_ypos, unequiv_flag )
char *equiv_name ;
INT layer, eq_xpos, eq_ypos ;
BOOL unequiv_flag ;
{
    INT xpos ;                 /* position of pin */
    INT ypos ;                 /* position of pin */
    char *pname ;              /* temp for swapping names */
    static EQ_NBOXPTR eqptrL ; /* current equivalent pointer */

    if( curCellTypeS == PADTYPE || curCellTypeS == HARDCELLTYPE ){
	/* these types may have global pin positions */
	/* subtract off cell center */
	eq_xpos -= ptrS->cxcenter ;
	eq_ypos -= ptrS->cycenter ;
    }
    check_pin( eq_xpos, eq_ypos, equiv_name ) ;
    ERRORABORT() ;
    if( unequiv_flag ){
	case_unequiv_pinG = TRUE ;
    }
    if( pin_typeS != PASS_THRU ){
	xpos = pinptrS->txpos[0] ;
	ypos = pinptrS->typos[0] ;
	if(!(pinptrS->eqptr)){ 
	    /* the first equivalent pin */
	    if( ( ypos > 0 && eq_ypos < 0 ) || ( ypos < 0 && eq_ypos > 0 ) ) {
		pinptrS->pinloc = NEITHER ;
		pinptrS->typos[0] = 0 ;
		eqptrL = (EQ_NBOXPTR)Ysafe_malloc( sizeof(EQ_NBOX) );
		netarrayG[curNetS]->pins->eqptr = eqptrL ;
		eqptrL->unequiv = unequiv_flag ;
		eqptrL->txoff   = eq_xpos - xpos ;
		eqptrL->typos   = eq_ypos ;
		if( pin_layers_givenG != 0 ) {
		    eqptrL->pinname = (char *) Ysafe_malloc(
			(strlen(equiv_name) + 3 ) * sizeof( char ) ) ;
		    sprintf( eqptrL->pinname , "]%1d%s", layer, equiv_name ) ;
		    Ysafe_free( equiv_name ) ;
		} else {
		    eqptrL->pinname = equiv_name ;
		}
	    }
	} else { /* more than one equiv pin */
	    if( ( ypos > 0 && eq_ypos < 0 ) || ( ypos < 0 && eq_ypos > 0 ) ) {
		pinptrS->pinloc = NEITHER ;
		if( ABS( eq_xpos - xpos ) < ABS( eqptrL->txoff ) ) {
		    eqptrL->txoff = eq_xpos - xpos ;
		    eqptrL->typos = eq_ypos ;
		    if( pin_layers_givenG ) {
			eqptrL->pinname = (char *) Ysafe_realloc( eqptrL->pinname,
			    (strlen(equiv_name) + 3 ) * sizeof(char));
			sprintf( eqptrL->pinname , "]%1d%s", layer, equiv_name ) ;
		    } else {
			(char *) Ysafe_realloc( eqptrL->pinname ,
			    (strlen(equiv_name) + 1 ) * sizeof(char));
			sprintf(eqptrL->pinname,"%s", equiv_name ) ;
		    }
		    Ysafe_free( equiv_name ) ;
		}
	    }
	}
    } else {  /* PASS_THRU CASE */
	if( pin_layers_givenG ) {
	    imptrS->eqpinname = (char *) Ysafe_malloc(
		    (strlen( equiv_name ) + 3 ) * sizeof( char ) ) ;
	    sprintf( imptrS->eqpinname , "]%1d%s" , layer, equiv_name ) ;
	    Ysafe_free( equiv_name ) ;
	} else {
	    imptrS->eqpinname = equiv_name ;
	}
	if( eq_ypos > 0 ) { /* swap the pinnames */
	    pname = imptrS->pinname ;
	    imptrS->pinname = imptrS->eqpinname ;
	    imptrS->eqpinname = pname ;
	}
    }
} /* end add_equiv */

add_port( portname, signal, layer, xpos, ypos )
char *portname, *signal ;
INT xpos, ypos ;
{
    ERRORABORT() ;
    addCell( portname, PORTTYPE ) ;
    add_tile( 0, 0, 0, 0 ) ;
    /* now perform overrides */
    ptrS->cxcenter = xpos ;
    ptrS->cycenter = ypos ;
    add_pin( portname, signal, layer, 0, 0 ) ;
    if( ypos > top_of_top_rowG ) {
	pptrS->padside = T ;
    } else if( ypos < bot_of_bot_rowG ) {
	pptrS->padside = B ;
    }
} /* end add_port */



static layer_test( layer )
INT layer ;
{
    if( layer != 0 && layer != 1 && layer != 2 && layer != 3 ) {
	M( ERRMSG, "layer_test", "Currently, a layer number ");
	M( ERRMSG, NULL,"in the .cel file must be either 0, 1, 2,\n");
	M( ERRMSG, NULL, "or 3.   \n") ;
	sprintf( YmsgG, "Current cell number:%d  Name:<%s>\n", 
	    totalCellS - num_exceptsG, curCellNameS ) ;
	M( ERRMSG, NULL, YmsgG ) ;
	abortS = TRUE ;
    }
} /* end layer_test */

init_legal_blocks( numblocks )
INT numblocks ;
{
    ERRORABORT() ;
    ptrS->cclass = 0 ;
} /* end init_legal_blocks */

add_legal_blocks( block_class )
INT block_class ;
{
    INT  row ;            /* row counter */
    INT  shift ;          /* used to calculate bit class */
    INT  index ;          /* used to calculate class */
    INT  bit_class ;      /* used to calculate class */
    BOOL found ;          /* used to search for row */

    if( block_class > 256 ) {
	M( ERRMSG, "add_legal_blocks","cannot handle more than 256 block classes\n");
	abortS = TRUE ;
    }
    ERRORABORT() ;
    found = FALSE ;
    for( row = 1 ; row <= numRowsG ; row++ ) {
	if( block_class == barrayG[row]->bclass ) {
	    found = TRUE ;
	}
    }
    if( !found ) {
	sprintf( YmsgG,"block class specified for cell <%s> was\n",
			curCellNameS ) ;
	M( ERRMSG, "add_legal_blocks", YmsgG ) ;
	M( ERRMSG, NULL,"not specified in the .blk file\n");
	abortS = TRUE ;
    }
    ptrS->cclass = 1 ;  /* this field must be set > 0 */
    bit_class = 1 ;
    index = (block_class-1) / 32 ;
    shift = block_class - 32 * index ;
    bit_class <<= (shift - 1) ;
    ptrS->cbclass[index] += bit_class ;
} /* end add_legal_blocks */

set_mirror_flag()
{
    ERRORABORT() ;
    /* this is for the current cell */
    ptrS->orflag = 0 ;
} /* end set_mirror_flag */

add_orient( orient )
INT orient ;
{
    ERRORABORT() ;
    /* this is for the current cell */
    ptrS->corient = orient ;
} /* end add_orient */

fix_placement( fixed_type, from, fixed_loc, block )
char *fixed_type, *fixed_loc ;
INT from, block;
{
    FENCEBOXPTR fence ;              /* current fence */
    BOOL approx_fixed = FALSE ;

    ERRORABORT() ;

    
    if( strcmp( fixed_type, "fixed" ) == STRINGEQ ) {
	ptrS->cclass = -1 ;
	ptrS->orflag = 0 ;
        fences_existG = TRUE ;
        approx_fixed = TRUE ;
    } else if( strcmp( fixed_type , "rigidly_fixed" ) == STRINGEQ ) {
	if( !gate_arrayG ) {
	    M(ERRMSG,"fix_placement","if you use rigidly_fixed cells, you must run\n");
	    M(ERRMSG,NULL,"the circuit as a \"gate array\", that is, specifying \n");
	    M(ERRMSG,NULL,"the \"spacer_width\" parameter in the .par file\n");
	    M(ERRMSG,NULL,"Please refer to the manual\n");
	    abortS = TRUE ;
	}
	rigidly_fixed_cellsG = TRUE ;
	ptrS->cclass = -5 ;
	ptrS->orflag = 0 ;
    } else if( strcmp( fixed_type, "approximately_fixed" ) == STRINGEQ ) {
        fences_existG = TRUE ;
        approx_fixed = TRUE ;
    } else if( strcmp( fixed_type, "nonfixed" ) != STRINGEQ ){
	sprintf( YmsgG, "Unknown fixed type:%s\n", fixed_type ) ;
	M(ERRMSG,"fix_placement", YmsgG ) ;
	abortS = TRUE ;
    }
    Ysafe_free(fixed_type) ;

    /* fixed_loc "left" or "right"  */
    if( strcmp( fixed_loc, "left" ) == STRINGEQ ) {
	ptrS->cxcenter = from ;
	ptrS->cycenter = 1 ;
	ptrS->border = from ;
    } else if( strcmp( fixed_loc, "right" ) == STRINGEQ ) {
	ptrS->cxcenter = -from ;
	ptrS->cycenter = -1 ;
	if( from != 0 ) {
	    ptrS->border = -from ;
	} else {
	    ptrS->border = -1 ;
	}
    } else {
	sprintf( YmsgG, "Invalid fixed_location:%s\n", fixed_loc ) ;
	M(ERRMSG,"fix_placement", YmsgG ) ;
	abortS = TRUE ;
    }
    Ysafe_free(fixed_loc) ;

    ptrS->cblock = block ;
    if( block > numRowsG || block < 1 ) {
	sprintf(YmsgG, "block assigned to cell <%s> was\n", curCellNameS );
	M( ERRMSG, "fix_placement", YmsgG ) ;
	M( ERRMSG, NULL, "out of range as specified in the .blk file\n");
	abortS = TRUE ;
    }

    if( approx_fixed ) {
	fence = ptrS->fence ;
	ptrS->fence = (FENCEBOXPTR)
			Ysafe_malloc( sizeof(FENCEBOX) ) ;
	ptrS->fence->next_fence = fence ;

	if( ptrS->cclass < 0 ) {
	    ptrS->fence->min_block = block ;
	    ptrS->fence->max_block = block ;
	} else {
	    if( block > approximately_fixed_factorG ) {
		ptrS->fence->min_block = block - approximately_fixed_factorG ;
	    } else {
		ptrS->fence->min_block = 1 ;
	    }
	    if( block + approximately_fixed_factorG <= numRowsG ) {
		ptrS->fence->max_block = block + approximately_fixed_factorG ;
	    } else {
		ptrS->fence->max_block = numRowsG ;
	    }
	}
    }
} /* end fix_placement */

add_extra_cells()
{

    INT row ;                 /* row counter */
    INT pin ;                 /* pin counter */
    INT slack ;               /* excess row length */
    INT target ;               /* target total row length */
    INT avg_width ;           /* average except width */
    char buffer[LRECL] ;      /* temp buffer */
    IPBOXPTR imptr ;          /* current implicit feed ptr */

    /* at this time if we get an error exit */
    if( abortS ){
	closegraphics() ;
	YexitPgm( PGMFAIL ) ;
    }
    if( gate_arrayG && !rigidly_fixed_cellsG && rowsG ) {
	target = (int) (1.3 * (DOUBLE) celllenG ) ;
	while( total_row_lengthG - barrayG[numRowsG]->blength >= target ) {
	    total_row_lengthG -= barrayG[numRowsG]->blength ;
	    numRowsG-- ;
	    fprintf(fpoG,"The number of available rows in the gate_array \n");
	    fprintf(fpoG,"is being decremented in add_extra_cells()\n\n");
	}
    }

#ifndef MITLL
    if( gate_arrayG || rowsG > 0 ) {
	slack = total_row_lengthG - celllenG ;
	avg_width = (celllenG - total_except_widthG) / numcellsG ;
	if( slack < numRowsG * 2 * avg_width ) {
	    slack = (numRowsG * 2 * avg_width - slack) / numRowsG ;
	    while( slack % spacer_widthG != 0 ) {
		slack++ ;
	    }
	    total_row_lengthG += slack * numRowsG ;
	}
	slack = total_row_lengthG - celllenG ;
	numcellsG += extra_cellsG = (slack) / spacer_widthG ;
	sprintf(YmsgG,"Added %d spacer cells to the gate array\n", extra_cellsG);
	M( MSG, "add_extra_cells", YmsgG ) ;
    } else {
	extra_cellsG = 0 ;
	spacer_widthG = 0 ;
    }

#else
    if( total_row_lengthG < celllenG ) {
	total_row_lengthG = celllenG ;
    }
    extra_cellsG = (total_row_lengthG - celllenG ) / spacer_widthG ;
    while( (extra_cellsG + numcellsG) % numRowsG != 0 ) {
	extra_cellsG++ ;
    }
    numcellsG += extra_cellsG ;
    sprintf(YmsgG,"Added %d spacer cells to the gate array\n", extra_cellsG);
    M( MSG, "add_extra_cells", YmsgG ) ;
#endif

    /* at this point number numcellsG will have extra cells added but */
    /* totalCellS will have the number of cells added so far */
    /* loop until they are equal - addCell will increment totalCellS */
    if( extra_cellsG > 0 ) {
	while( totalCellS < numcellsG ) { 
	    if( !spacer_name_twfeedG ) {
	        addCell( Ystrclone( "GATE_ARRAY_SPACER"), EXTRATYPE ) ;
	    } else {
#ifdef FEED_INSTANCES
		sprintf( buffer, "twfeed%d", 100000 + totalCellS ) ;
#else
		sprintf( buffer, "twfeed" ) ;
#endif
		addCell( Ystrclone( buffer), EXTRATYPE ) ;
	    }
	    ptrS->cheight = barrayG[1]->bheight ;
	    ptrS->clength = spacer_widthG ;
	    ptrS->orflag = 0 ;
	    ptrS->cycenter = GATE_ARRAY_MAGIC_CONSTANT ;
	    add_tile( - spacer_widthG / 2, 
		- barrayG[1]->bheight / 2,
	        spacer_widthG - spacer_widthG / 2, 
		barrayG[1]->bheight - barrayG[1]->bheight / 2 ) ;

#ifdef OLD_WAY
	    for( pin = 1 ; pin <= spacer_feedsG[0] ; pin++ ) {
		imptr = ( IPBOXPTR )Ysafe_malloc( sizeof( IPBOX ) ) ;
		if( pin_layers_givenG ) {
		    sprintf( buffer,"]%1d%s%d",feedLayerG,"SPACER_FEED_TOP_", pin ) ;
		} else {
		    sprintf( buffer, "%s%d" , "SPACER_FEED_TOP_" , pin ) ;
		}
		imptr->pinname = Ystrclone( buffer ) ;
		imptr->txpos = spacer_feedsG[pin] - spacer_widthG / 2 ;
		imptr->cell  = totalCellS ;
		imptr->terminal = -1 ; /* to be set later */
		imptr->next = ptrS->imptr ;
		ptrS->imptr = imptr ;

		if( pin_layers_givenG != 0 ) {
		    sprintf( buffer, "]%1d%s%d",feedLayerG,"SPACER_FEED_BOTTOM_",pin );
		} else {
		    sprintf( buffer, "%s%d" , "SPACER_FEED_BOTTOM_" , pin );
		}
		imptr->eqpinname = Ystrclone( buffer ) ;
	    }
#endif /* OLD_WAY */

	}
    }	
} /* end add_extra_cells */

static INT free_swap_data( data )
INT *data ;
{
    Ysafe_free( data ) ;
} /* free_swap_data */

static trans_tile( ptr, orient )
CBOXPTR ptr ;
INT orient ;
{
    INT l,r,b,t ;         /* needed for orientation calc. */
    TIBOXPTR tptr ;       /* tilebox ptr */

    /* rotate the tiles to proper orientation */
    tptr = ptr->tileptr ;
    /* need to assign to variable since tileptr values are short */
    l = (INT) tptr->left ;
    r = (INT) tptr->right ;
    b = (INT) tptr->bottom ;
    t = (INT) tptr->top ;
    YtranslateT( &l, &b, &r, &t, orient ) ;
    tptr->left   = (SHORT) l ;
    tptr->right  = (SHORT) r ;
    tptr->bottom = (SHORT) b ;
    tptr->top    = (SHORT) t ;
    ptr->clength = r - l ;
    ptr->cheight = t - b ;
} /* end trans_tile */

static build_pad_group( side, sidename, padgroupname )
INT side ;
char *sidename, *padgroupname ;
{
    INT cell ;            /* cell counter */
    BOOL build_padgroup ; /* whether to build group or not */
    CBOXPTR ptr ;         /* current cell pointer */

    /* see if we need to first build left padgroup */
    build_padgroup = FALSE ;
    for( cell=numcellsG+1;cell<=lastpadG;cell++ ){
	ptr = carrayG[cell] ;
	if( ptr->padptr->padside == side && pptrS->lowerbound <= 0.0 &&
	    pptrS->upperbound >= 1.0 ){
	    build_padgroup = TRUE ;
	}
    }
    if( build_padgroup ){
	addCell( padgroupname, PADGROUPTYPE ) ;
	setPermutation( FALSE ) ;
	add_tile( 0,0,0,0 ) ;
	add_padside( sidename ) ;
	for( cell=numcellsG+1;cell<=lastpadG;cell++ ){
	    ptr = carrayG[cell] ;
	    if( ptr->padptr->padside == side && pptrS->lowerbound <= 0.0 &&
		pptrS->upperbound >= 1.0 ){
		add2padgroup( ptr->cname, TRUE ) ; /* fixed */
	    }
	}
	end_padgroup() ;
    }
} /* end build_pad_group() */

cleanup_readcells()
{
    INT trl ;             /* total_row_length */
    INT row ;             /* row counter */
    INT cell ;            /* cell counter */
    INT cell1 ;           /* look for match */
    INT cell2 ;           /* look for match */
    INT xpos ;            /* temp for pin calc */
    INT ypos ;            /* temp for pin calc */
    INT block ;           /* block counter */
    INT orient ;          /* current orientation */
    INT impcount ;        /* count implicit feeds */
    INT inverse ;         /* inv. orient to get back to bottom */
    INT b1, l1, r1, t1 ;  /* coordinates of pads */
    INT b2, l2, r2, t2 ;  /* coordinates of pads */
    INT total_desire  ;
    INT deviate_by_row  ;
    BOOL bogus_rows;      /* true if we have bogus rows */
    BOOL xFlag ;          /* true if unsymmetric in x */        
    BOOL yFlag ;           /* true if unsymmetric in y */
    DOUBLE fraction ;     /* check row length sanity */
    CBOXPTR ptr ;         /* current cell pointer */
    TIBOXPTR tptr ;       /* current tile */
    PINBOXPTR pinptr ;    /* current pin */
    PADBOXPTR padptr ;    /* current pad information */
    IPBOXPTR imptr ;      /* current implicit feed ptr */

    /* at this time if we get an error exit */
    if( abortS ){
	closegraphics() ;
	YexitPgm( PGMFAIL ) ;
    }

    if( old_pad_formatS ){
	
	/* add the old pads to a padgroup */
	lastpadG = numcellsG + numtermsG ;

	/* see if we need to first build left padgroup */
	build_pad_group( L, "L", "left_pads" ) ;
	build_pad_group( T, "T", "top_pads" ) ;
	build_pad_group( R, "R", "right_pads" ) ;
	build_pad_group( B, "B", "bottom_pads" ) ;

	if( abortS ){
	    closegraphics() ;
	    YexitPgm( PGMFAIL ) ;
	}
    }

    /* cleanup the memory at this time this should shrink the memory */
    carrayG = (CBOXPTR *) Ysafe_realloc( carrayG, 
	    (totalCellS+1) * sizeof( CBOXPTR ) ) ;
    netarrayG = (DBOXPTR *) 
	Ysafe_realloc( netarrayG, (numnetsG+1) * sizeof(DBOX) ) ;

    /*  This is for the TW_SWAP_PASS_THRU feature  */
    maxtermG = implicit_feed_countG + TotRegPinsG ;
    tearrayG = (PINBOXPTR *) Ysafe_calloc( maxtermG + 1, sizeof(PINBOXPTR) ) ;
    /* now load the pins in tearray */
    last_pin_numberG = maxtermG ;
    impcount = TotRegPinsG ;
    lastpadG = numcellsG + numtermsG - numpadgrpsG ;
    init_acceptance_rate() ;
    for( cell = 1 ; cell <= totalCellS ; cell++ ) {
	ptr = carrayG[cell] ;
	/* first the given pins - they should already be numbered */
	for( pinptr = ptr->pins; pinptr ; pinptr = pinptr->nextpin ){
	    if( pinptr->terminal <= 0 || pinptr->terminal > TotRegPinsG ){
		sprintf( YmsgG, "Problem with pin %s\n", pinptr->pinname ) ;
		M( ERRMSG, "cleanup_readcells", YmsgG ) ;
		abortS = TRUE ;
	    }
	    tearrayG[pinptr->terminal] = pinptr ;
	}
	/* now due the implicit feeds they haven't been numbered */
	/* pins labeled with terminal -1 are not to be added to tearray ??? */ 
	for( imptr = ptr->imptr; imptr ; imptr = imptr->next ){
	    if( imptr->terminal == -1 ){
		imptr->terminal = ++last_pin_numberG ;
	    } else {
		imptr->terminal = ++impcount ;
	    }
	}
    }

    /*
	NOW WE HAVE TO LOAD IN THE OTHER CONFIGURATIONS
    */
    for( cell = 1 ; cell <= numcellsG ; cell++ ) {
	if( carrayG[cell]->orflag == 1 ) {
	    if( maxCellOG == 0 ) {
		maxCellOG = 2 ;
	    } else if( maxCellOG == 1 ) {
		maxCellOG = 3 ;
	    } 
	    break ;
	}
    }
    for( cell = 1 ; cell <= numcellsG - extra_cellsG ; cell++ ) {

	ptr = carrayG[ cell ] ;
	if( ptr->clength %2 != 0 ) {
	    xFlag = TRUE ;
	} else {
	    xFlag = FALSE ;
	}
	if( ptr->cheight %2 != 0 ) {
	    yFlag = TRUE ;
	} else {
	    yFlag = FALSE ;
	}
	for( pinptr = ptr->pins ; pinptr; pinptr = pinptr->nextpin ) {
	    if( xFlag ) {
		pinptr->txpos[1] = -pinptr->txpos[0] + 1 ;
	    } else {
		pinptr->txpos[1] = -pinptr->txpos[0] ;
	    }
	    if( yFlag && pinptr->typos[0] != 0 ) {
		pinptr->typos[1] = -pinptr->typos[0] + 1 ;
	    } else {
		pinptr->typos[1] = -pinptr->typos[0] ;
	    }
	}

    } /* end processing core cells */

    numpadsG = 0 ;
    /* now the pad cells and macro cells */
    for( cell = numcellsG + 1 ; cell <= lastpadG; cell++ ) {
	
	ptr = carrayG[ cell ] ;
	padptr = ptr->padptr ;
	tptr = ptr->tileptr ;
	orient = ptr->corient ;
	inverse = 0 ;
	if(!(padptr->macroNotPad)){
	    numpadsG++ ;
	}
	if( padptr->oldformat ){
	    /* calculate effective orientation to pad B */
	    inverse = transTableS[padptr->padside][orient] ;
	} else if( padptr->padside == MMC ){
	    /* calculate effective orientation for hardcell. */
	    /* Normal inverse use side B */
	    inverse = transTableS[B][orient] ;
	} else {  /* the new pad format */
	    if( orient ){

		/* ************** INITIAL ORIENTATION NOT ZERO ************ */
		/* normally user inputs pads in orientation 0.  However, MC will */
		/* output the pad with the orientation set to the appropriate side */
		/* rotate to orientation 0 perform the inverse operation if */
		/* necessary */
		switch (padptr->padside) {
		    case L:
			/* rotate 270 | -90 */
			inverse = 6 ;
			break;
		    case T:
			/* rotate 180 */
			inverse = 3 ;
			break;
		    case R:
			/* rotate 90 */
			inverse = 7 ;
			break;
		    case ALL:
			/* this is the case of no restriction */
			/* and we came from floorplanning */
			if( orient == 1 || orient == 3 ){
			    /* this is the top side */
			    inverse = orient ;
			    break ;
			} else if( orient == 4 || orient == 6 ){
			    /* this is the right side */
			    if( orient == 6 ){
				inverse = 7 ;
			    } else {
				inverse = 4 ;
			    }
			    break ;
			} else if( orient == 5 || orient == 7 ){
			    /* this is the left side */
			    if( orient == 6 ){
				inverse = 6 ;
			    } else {
				inverse = 5 ;
			    }
			    break ;
			} else if( orient == 2 ){
			    /* this is the bottom side */
			    inverse = 2 ;
			    break ;
			}
		    case B:
		    default:
			M( ERRMSG, "cleanup_readcell",
			    "\nNON-PAD or PAD w/o valid side rotatation\n");
			sprintf( YmsgG, "\tpad:%s is bogus\n", ptr->cname ) ;
			M( ERRMSG, NULL, YmsgG ) ;
			abortS = TRUE ;
			break;
		}

	    } else {
		/* zero orientation means bottom side */
		padptr->padside = B ;
	    }
	}
	/* now translate the tile to correct orientation */
	trans_tile( ptr, inverse ) ;

	if( (ptr->cheight % 2 != 0 && (inverse == 4 || inverse == 6))||
	    (ptr->clength % 2 != 0 && (inverse == 2 || inverse == 3))){
	    xFlag = TRUE ; 
	} else {
	    xFlag = FALSE ;
	}
	if( (ptr->cheight % 2 != 0 && (inverse == 1 || inverse == 3))||
	    (ptr->clength % 2 != 0 && (inverse == 4 || inverse == 7))){
	    yFlag = TRUE ; 
	} else {
	    yFlag = FALSE ;
	}

	/* calculate the txpos[1] and typos[1] fields which */
	/* hold the cell relative pin position in current orientation */
	for( pinptr = ptr->pins ; pinptr; pinptr = pinptr->nextpin ) {
	    if( orient ){
		/* if orientation is set and not a hardcell, it means */
		/* we need to take the inverse first */
		xpos = pinptr->txpos[0] ;
		ypos = pinptr->typos[0] ;
		REL_POS( inverse, 
		    pinptr->txpos[0],                  /* result */
		    pinptr->typos[0],                  /* result */
		    xpos, ypos,
		    0, 0 ) ;                           /* center */
	    }

	    if( xFlag ) {
		xpos = pinptr->txpos[0] + 1 ;
	    } else {
		xpos = pinptr->txpos[0] ;
	    }
	    if( yFlag && pinptr->typos[0] != 0 ) {
		ypos = pinptr->typos[0] + 1 ;
	    } else {
		ypos = pinptr->typos[0] ;
	    }
	    REL_POS( ptr->corient,
		pinptr->txpos[1], pinptr->typos[1],  /* result */
		xpos, ypos,                         /* cell rel. */
		0, 0 ) ;               /* make result cell relative */
	}

	/* now set the length and height fields */
	padptr->length = tptr->right - tptr->left ;
	padptr->height = tptr->top - tptr->bottom ;
    }

    /* 
     *   Configure the blocks, place the macro blocks and pads.
     *   Also generate an initial placement of the standard cells
     */
    totalpadsG = numpadsG + numpadgrpsG ;
    /* build the pad arrays */
    /* now create sort array */
    padarrayG = (PADBOXPTR *) Yvector_alloc(1,totalpadsG,sizeof(PADBOXPTR) ) ;
    sortarrayG = (PADBOXPTR *) Yvector_alloc(1,totalpadsG,sizeof(PADBOXPTR) ) ;
    placearrayG = (PADBOXPTR *) Yvector_alloc( 1,numpadsG,sizeof(PADBOXPTR) ) ;
    cell1 = 0 ;
    for( cell = 1; cell <= numtermsG; cell++ ){
	ptr = carrayG[numcellsG + cell] ;
	padptr = ptr->padptr ;
	if( padptr->macroNotPad ){
	    continue ;
	}
	cell1++ ;
	sortarrayG[cell1] = padarrayG[cell1] = padptr ;
	if( padptr->padtype == PADTYPE ){
	    placearrayG[cell1] = padptr ;
	}
    }
    ASSERT( cell1 == totalpadsG, "cleanup_readcells", 
	"Pad initialization problem" ) ;

    if( rowsG == 0 ) {
	configure() ;
    } else {
	total_desire = 0 ;
	bogus_rows = FALSE ;
	for( row = 1 ; row <= numRowsG ; row++ ) {
	    total_desire += barrayG[row]->desire ;
	    if( barrayG[row]->desire > celllenG && numRowsG > 1 ){
		sprintf( YmsgG, 
		    "Unreasonable row length for row:%d. Please check\n", row ) ;
		M( ERRMSG, "cleanup_readcells", YmsgG ) ;
		bogus_rows = TRUE ;
	    }
	}
	if( celllenG > 0 ){
	    fraction = (DOUBLE) total_desire / (DOUBLE) celllenG ;
	    if( fraction < 0.70 || fraction > 1.30 ){
		bogus_rows = TRUE ;
	    }
	} else {
	    bogus_rows = TRUE ;
	}

	if( bogus_rows ){
	    M( WARNMSG, "cleanup_readcells", 
		"Cell length doesn't match row length\n" ) ;
	    deviate_by_row = (celllenG - total_desire) / numRowsG ;
	    for( row = 1 ; row <= numRowsG ; row++ ) {
		barrayG[row]->desire      += deviate_by_row ;
		barrayG[row]->orig_desire += deviate_by_row ;
		barrayG[row]->blength     += deviate_by_row ;
		barrayG[row]->bright      += deviate_by_row ;
	    }
	}
	random_placement() ;
    }


    /* now check for rigidly_fixed cells which may illegally overlap */
    for( cell1 = 1 ; cell1 < numcellsG ; cell1++ ) {
	if( carrayG[cell1]->cclass != -5 ) {
	    continue ;
	}
	row = carrayG[cell1]->cblock ;
	for( cell2 = cell1 + 1 ; cell2 <= numcellsG ; cell2++ ) {
	    if( carrayG[cell2]->cclass != -5 ) {
		continue ;
	    }
	    if( row != carrayG[cell2]->cblock ) {
		continue ;
	    }
	    /* must conduct the overlap test */
	    if( carrayG[cell1]->cxcenter + carrayG[cell1]->tileptr->left >=
		    carrayG[cell2]->cxcenter + carrayG[cell2]->tileptr->right ){
		continue ;
	    }
	    if( carrayG[cell2]->cxcenter + carrayG[cell2]->tileptr->left >=
		    carrayG[cell1]->cxcenter + carrayG[cell1]->tileptr->right ){
		continue ;
	    }
	    sprintf( YmsgG, 
	    "\nFATAL -- rigidly_fixed cells <%s> and <%s> overlap\n",
			    carrayG[cell1]->cname , carrayG[cell2]->cname ) ;
	    M( ERRMSG, "cleanup_readcells", YmsgG ) ;
	    abortS = TRUE ;
	}
    }


    /* now check for rigidly_fixed pads which may illegally overlap */
    if( padspacingG == EXACT_PADS ){
	for( cell1 = numcellsG + 1 ; cell1 < lastpadG; cell1++ ) {
	    ptr = carrayG[ cell1 ]  ;
	    orient = ptr->corient ;
	    tptr = ptr->tileptr ;
	    l1 = tptr->left ;
	    r1 = tptr->right ;
	    b1 = tptr->bottom ;
	    t1 = tptr->top ;
	    YtranslateT( &l1, &b1, &r1, &t1, orient ) ;
	    l1 += ptr->cxcenter ;
	    r1 += ptr->cxcenter ;
	    b1 += ptr->cycenter ;
	    t1 += ptr->cycenter ;
	    for( cell2 = cell1 + 1 ; cell2 <= lastpadG ; cell2++ ) {
		ptr = carrayG[ cell2 ]  ;
		orient = ptr->corient ;
		tptr = ptr->tileptr ;
		l2 = tptr->left ;
		r2 = tptr->right ;
		b2 = tptr->bottom ;
		t2 = tptr->top ;
		YtranslateT( &l2, &b2, &r2, &t2, orient ) ;
		l2 += ptr->cxcenter ;
		r2 += ptr->cxcenter ;
		b2 += ptr->cycenter ;
		t2 += ptr->cycenter ;
		/* must conduct the overlap test */
		if( l1 >= r2 ){
		    continue ;
		}
		if( l2 >= r1 ){
		    continue ;
		}
		if( b1 >= t2 ){
		    continue ;
		}
		if( b2 >= t1 ){
		    continue ;
		}
		/* the only case that would be fatal is two macro's overlapping */
		if( carrayG[cell1]->padptr->macroNotPad && ptr->padptr->macroNotPad ){
		    sprintf( YmsgG, "\nmacros <%s> and <%s> overlap -- FATAL\n",
				    carrayG[cell1]->cname , carrayG[cell2]->cname ) ;
		    M( ERRMSG, "cleanup_readcells", YmsgG ) ;
		    abortS = TRUE ;
		} else {
		    sprintf( YmsgG, "\npads (or macros) <%s> and <%s> overlap -- NONFATAL\n",
				    carrayG[cell1]->cname , carrayG[cell2]->cname ) ;
		    M( ERRMSG, "cleanup_readcells", YmsgG ) ;
		    M( ERRMSG, NULL, "Padspacing set to abut\n\n" ) ;
		    padspacingG = ABUT_PADS ;
		}

	    }
	}
    }

    /* check if position of fixed cells are within bounds of their rows */
    for( cell = 1 ; cell <= numcellsG - extra_cellsG ; cell++ ) {
	if( carrayG[cell]->cclass >= 0 ) {
	    continue ;
	}
	if( ABS(carrayG[cell]->border) > 
			    barrayG[carrayG[cell]->cblock]->blength ) {
	    sprintf( YmsgG, "initial placement assigned to cell <%s> is\n",
				    carrayG[cell]->cname );
	    M( ERRMSG, "cleanup_readcells", YmsgG ) ;
	    sprintf( YmsgG, "\toutside the length of its block (%d vs. %d)\n",
		ABS(carrayG[cell]->border), 
		barrayG[carrayG[cell]->cblock]->blength );
	    M( ERRMSG, NULL, YmsgG ) ;
	    abortS = TRUE ;
	}
    }
    if( abortS ){
	M( ERRMSG, NULL,"\n\nFATAL errors exist in input\n" );
	if(!(doGraphicsG)){
	    M( ERRMSG, NULL,"You are not running graphics.\n" );
	    M( ERRMSG, NULL,"Turn on graphics to see the problem!\n" );
	} else {
	    M( ERRMSG, NULL,"Entering process graphics to determine error\n" ) ;
	}
	G( init_heat_index() ) ;
	G( process_graphics() ) ;
	closegraphics() ;
	YexitPgm(PGMFAIL);
    }

    totallenG = 0;
    /* for Dick Davis at DEC */
    orig_max_row_lengthG = 0 ;
    for( block = 1 ; block <= numRowsG ; block++ ) {
	totallenG += barrayG[ block ]->blength ;
	if( orig_max_row_lengthG < barrayG[block]->blength ) {
	    orig_max_row_lengthG = barrayG[block]->blength ;
	}
    }
    fprintf( fpoG, "total cell length: %d\n",  celllenG ) ;
    fprintf( fpoG, "total block length: %d\n", totallenG ) ;

    if( rowsG > 0 ) {
	totalRG = celllenG ;
	rowHeightG = barrayG[1]->btop - barrayG[1]->bbottom ;
    }

    if( swappable_gates_existG ) {
	Yhash_table_delete( swap_hash_tableS, free_swap_data ) ;
    }

    return ;
} /* end cleanup_readcells */


not_supported( object )
char *object ;
{
    sprintf( YmsgG, "%s is not supported -- sorry!\n", object ) ;
    M( ERRMSG, "not_supported", YmsgG ) ;
    abortS = TRUE ;
}

YHASHPTR get_net_table()
{
    return( net_hash_tableS ) ;
} /* end get_net_table */

add_eco()
{
    ERRORABORT() ;
    ECOs_existG++ ;
    ptrS->ECO_flag = TRUE ;
} /* end add_eco */

/* ***************************************************************** */
/* added below for pad capability */
init_corners()
{
    minxS = INT_MAX ;
    maxxS = INT_MIN ;
    minyS = INT_MAX ;
    maxyS = INT_MIN ;

    ERRORABORT() ;
    /* assume a rectangle */
    pptrS->xpoints = (INT *) Ysafe_malloc( 4 * sizeof(INT) ) ;
    pptrS->ypoints = (INT *) Ysafe_malloc( 4 * sizeof(INT) ) ;
    ptAllocS = 4 ;
} /* end init_corners */

add_corner( x, y )
INT x, y ;
{
    INT pt ;    /* point counter */

    minxS = MIN( x, minxS ) ;
    maxxS = MAX( x, maxxS ) ;
    minyS = MIN( y, minyS ) ;
    maxyS = MAX( y, maxyS ) ;

    ERRORABORT() ;
    pt = ++(pptrS->numcorners) ;
    if( pt >= ptAllocS ){
	ptAllocS += 2 ;
	pptrS->xpoints = (INT *) 
	    Ysafe_realloc( pptrS->xpoints, ptAllocS * sizeof(INT) ) ;
	pptrS->ypoints = (INT *) 
	    Ysafe_realloc( pptrS->ypoints, ptAllocS * sizeof(INT) ) ;
    }
    /* store 0..numcorners-1 */
    pptrS->xpoints[--pt] = x ;
    pptrS->ypoints[pt] = y ;
} /* end add_corner */

process_corners()
{
    INT xcenter ; /* center of cell */
    INT ycenter ; /* center of cell */
    INT pt ;      /* point counter */
    INT xsum ;    /* sum of min and max in x */
    INT ysum ;    /* sum of min and max in y */

    ERRORABORT() ;

    xsum = minxS + maxxS  ;
    ysum = minyS + maxyS  ;
    xcenter = ( minxS + maxxS ) / 2 ;
    ycenter = ( minyS + maxyS ) / 2 ;
    if( xsum < 0 && xsum % 2 ){
	/* if cell is asymmetrical */
	xcenter-- ;
    }
    if( ysum < 0 && ysum % 2 ){
	/* if cell is asymmetrical */
	ycenter-- ;
    }
    if( curCellTypeS == HARDCELLTYPE || curCellTypeS == PADTYPE ){
    /* for hardcells and pads set the cell center location */
	ptrS->cxcenter = xcenter ;
	ptrS->cycenter = ycenter ;
    }
    minxS -= xcenter ;
    maxxS -= xcenter ;
    minyS -= ycenter ;
    maxyS -= ycenter ;

    /* subtract off x and y center */
    for( pt = 0; pt < pptrS->numcorners; pt++ ){
	pptrS->xpoints[pt] -= xcenter ;
	pptrS->ypoints[pt] -= ycenter ;
    }
    add_tile( minxS, minyS, maxxS, maxyS ) ;
} /* end process_corners */


add_padside( padside )
char *padside ;
{
    INT numsides ;         /* length of side restriction string */
    INT i ;                /* counter */

    ERRORABORT() ;

    if( strcmp( padside , "MUL" ) == STRINGEQ ) {
	pptrS->padside = MUL ;
	pptrS->oldformat = TRUE ;
	pptrS->macroNotPad = TRUE ;
	numMacroG++ ;
    } else if( strcmp( padside , "MUR" ) == STRINGEQ ) {
	pptrS->padside = MUR ;
	pptrS->oldformat = TRUE ;
	pptrS->macroNotPad = TRUE ;
	numMacroG++ ;
    } else if( strcmp( padside , "MLL" ) == STRINGEQ ) {
	pptrS->padside = MLL ;
	pptrS->oldformat = TRUE ;
	pptrS->macroNotPad = TRUE ;
	numMacroG++ ;
    } else if( strcmp( padside , "MLR" ) == STRINGEQ ) {
	pptrS->padside = MLR ;
	pptrS->oldformat = TRUE ;
	pptrS->macroNotPad = TRUE ;
	numMacroG++ ;
    } else if( strcmp( padside , "ML" ) == STRINGEQ ) {
	pptrS->padside = ML ;
	pptrS->oldformat = TRUE ;
	numMacroG++ ;
    } else if( strcmp( padside , "MR" ) == STRINGEQ ) {
	pptrS->padside = MR ;
	pptrS->oldformat = TRUE ;
	pptrS->macroNotPad = TRUE ;
	numMacroG++ ;
    } else if( strcmp( padside , "MB" ) == STRINGEQ ) {
	not_supported("macro at position MB" ) ;
    } else if( strcmp( padside , "MT" ) == STRINGEQ ) {
	not_supported("macro at position MT");
    } else if( strcmp( padside , "MM" ) == STRINGEQ ) {
	not_supported("macro at position MM");
    } else if( strcmp( padside , "MTT" ) == STRINGEQ ) {
	pptrS->padside = MTT ;
	pptrS->oldformat = TRUE ;
	pptrS->macroNotPad = TRUE ;
	numMacroG++ ;
    } else if( strcmp( padside , "MBB" ) == STRINGEQ ) {
	pptrS->padside = MBB ;
	pptrS->oldformat = TRUE ;
	pptrS->macroNotPad = TRUE ;
	numMacroG++ ;
    } else {
	pptrS->valid_side[ALL] = FALSE ;
	numsides = strlen( padside ) ;
	for( i = 0 ; i < numsides; i++ ){
	    switch( padside[i] ){
		case 'B' :
		    pptrS->valid_side[B] = TRUE ;
		    break ;
		case 'L' :
		    pptrS->valid_side[L] = TRUE ;
		    break ;
		case 'R' :
		    pptrS->valid_side[R] = TRUE ;
		    break ;
		case 'T' :
		    pptrS->valid_side[T] = TRUE ;
		    break ;
		default:
		    sprintf( YmsgG,
			"side restriction not specified properly for pad:%s\n",
			curCellNameS );
		M(ERRMSG,"add_padside",YmsgG ) ;
		abortS = TRUE ;
	    } /* end switch */
	}
    } 
} /* end add_padside */

add_sidespace( lower, upper )
DOUBLE lower, upper ;
{
    ERRORABORT() ;

    if( pptrS->padside == L ) {
	fixLRBTG[0] = 1 ;	
    } else if( pptrS->padside == R ) {
	fixLRBTG[1] = 1 ;	
    } else if( pptrS->padside == B ) {
	fixLRBTG[2] = 1 ;	
    } else if( pptrS->padside == T ) {
	fixLRBTG[3] = 1 ;	
    } else {
	macspaceG[ pptrS->padside ] = (lower + upper) / 2.0 ;
    }
    pptrS->fixed = TRUE ;
    if( lower > 1.0 || upper > 1.0 ){
	sprintf(YmsgG,
	    "side space must be less or equal to 1.0 for pad: %s\n",curCellNameS ) ;
	M(ERRMSG,"add_sidespace",YmsgG ) ;
	abortS = TRUE ;
    }
    if( lower < 0.0 || upper < 0.0 ){
	sprintf(YmsgG,
	    "side space must be greater or equal to 0.0 for pad: %s\n",curCellNameS ) ;
	M(ERRMSG,"add_sidespace",YmsgG ) ;
	abortS = TRUE ;
    }
    if( lower > upper ){
	sprintf(YmsgG,
	    "side space upper bound must be greater or equal to lower bound for pad: %s\n",curCellNameS ) ;
	M(ERRMSG,"add_sidespace",YmsgG ) ;
	abortS = TRUE ;
    }
    pptrS->lowerbound = lower ;
    pptrS->upperbound = upper ;
} /* end add_sidespace */
/* ***************************************************************** */

/* set whether a pad group can be permuted */
setPermutation( permuteFlag ) 
{
    ERRORABORT() ;
    pptrS->permute = permuteFlag ;
} /* end setPermutation */
/* ***************************************************************** */

set_old_format( padside )
char *padside ;
{
    ERRORABORT() ;
    if( strcmp( padside , "L" ) == STRINGEQ ||
        strcmp( padside , "T" ) == STRINGEQ ||
        strcmp( padside , "R" ) == STRINGEQ ||
        strcmp( padside , "B" ) == STRINGEQ ){
	pptrS->oldformat = TRUE ; /* set this switch to rotate pads */
	old_pad_formatS = TRUE ;  /* set this switch to add padgroups */
	if( strcmp( padside , "L" ) == STRINGEQ ){
	    pptrS->padside = L ;
	} else if( strcmp( padside , "T" ) == STRINGEQ ){
	    pptrS->padside = T ;
	} else if( strcmp( padside , "R" ) == STRINGEQ ){
	    pptrS->padside = R ;
	} else if( strcmp( padside , "B" ) == STRINGEQ ){
	    pptrS->padside = B ;
	}
    }
} /* set_old_format */

/* add this pad to the current pad group */
add2padgroup( padName, ordered ) 
char *padName ;
BOOL ordered ;  /* ordered flag is true if pad is fixed in padgroup */
{
    INT i, endofpads, endofgroups ;

    ERRORABORT() ;
    endofpads = numcellsG + numtermsG - numpadgrpsG ;
    /* check pads for correctness */
    for (i = numcellsG + 1; i <= endofpads; i++) {
	if (strcmp(padName, carrayG[i]->cname) == STRINGEQ) {
	    if (carrayG[i]->padptr->hierarchy == LEAF) {
		sprintf(YmsgG,
		    "pad %s was included in more than 1 pad group\n",
		    carrayG[i]->cname);
		M(ERRMSG,"add2padgroup",YmsgG ) ;
		abortS = TRUE ;
		return ;
	    }
	    /* check memory of pin array */
	    if( ++numchildrenS >= childAllocS ){
		childAllocS += EXPECTEDNUMPADS ;
		pptrS->children = (INT *)
		    Ysafe_realloc( pptrS->children,
		    childAllocS * sizeof(INT) ) ;
	    }
	    pptrS->children[numchildrenS]  = i - numcellsG - numMacroG ;
	    carrayG[i]->padptr->hierarchy = LEAF    ;
	    carrayG[i]->padptr->ordered = ordered ;
	    ptrS->numterms += carrayG[i]->numterms;
	    return;
	}
    }

    /* if no match above must be subroot */
    endofgroups = numcellsG + numtermsG ;
    for (i = endofpads; i <= endofgroups; i++ ) {
	if (strcmp(padName, carrayG[i]->cname) == STRINGEQ) {
	    if (carrayG[i]->padptr->hierarchy == SUBROOT) {
		sprintf(YmsgG,
		"pad group %s was included in more than 1 pad group\n",
		    curCellNameS ) ;
		M(ERRMSG,"add2padgroup",YmsgG ) ;
		abortS = TRUE ;
		return ;
	    }
	    /* check memory of pin array */
	    if( ++numchildrenS >= childAllocS ){
		childAllocS += EXPECTEDNUMPADS ;
		pptrS->children = (INT *)
		    Ysafe_realloc( pptrS->children,
		    childAllocS * sizeof(INT) ) ;
	    }
	    pptrS->children[numchildrenS] = i - numcellsG - numMacroG ;
	    carrayG[i]->padptr->hierarchy     = SUBROOT ;
	    carrayG[i]->padptr->ordered = ordered ;
	    /* total pins of the leaves */
	    ptrS->numterms += carrayG[i]->numterms;
	    return ;
	}
    }

    sprintf(YmsgG,"cannot find pad <%s> for pad_group <%s>\n",
	padName,ptrS->cname);
    M(ERRMSG,"add2padgroup",YmsgG ) ;
    return ;

} /* end add2PadGroup */

end_padgroup()
{
    ERRORABORT() ;

    /* realloc size of children array to final size */
    pptrS->children = (INT *)
	Ysafe_realloc( pptrS->children,(numchildrenS+1) * sizeof(INT));
    pptrS->children[HOWMANY] = numchildrenS ;
    // if( numchildrenS <= 1 ){
    if( numchildrenS < 1 ){
	M(ERRMSG,"end_padgroup",
//	    "Must have at least 2 pads in a padgroup.\n");
	    "Must have at least 1 pad in a padgroup.\n");
	sprintf( YmsgG, "\t%s only has %d pad\n", curCellNameS, 
	    numchildrenS ) ;
	abortS = TRUE ;
	M(ERRMSG, NULL, YmsgG ) ;
    }
} /* end_padgroup */
/* ***************************************************************** */
