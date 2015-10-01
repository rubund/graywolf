/*
 *   Copyright (C) 1988-1991 Yale University
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
FILE:	    initialize.c
DESCRIPTION:This file contains the utility routines called from the
	    parser to set up the TimberWolfMC data structures.
CONTENTS:   
	    setErrorFlag()
DATE:	    Aug  7, 1988 
REVISIONS:  Oct 27, 1988 - added add_cell_to_group, initializeCorner
		and modified fixCell in order to handle group cells.
	    Jan 20, 1989 - added build_soft_array call.
	    Jan 29, 1989 - changed msg to YmsgG and added \n's.
	    Feb 28, 1989 - added initialization to tile weights.
	    Mar  7, 1989 - added cell instances.
			 - added cell hash table for instance names.
	    Apr  5, 1989 - added multiple tiles for softcells and
		added instance handling.  Instances are handled by
		disturbing routines as little as possible and using
		pointer swaps to switch in a new instance as much as
		possible.
	    Apr 9, 1989 - added sortpin to cleanupCells for incremental
		bounding box scheme.
	    Apr 30, 1989 - changed exchange class for group cells.
			 - fixed problem with pad group children.   
	    May  1, 1989 - orient the pads correctly.
	    May 11, 1989 - now use the orientList[HOWMANYORIENT] to
		specify the number of valid orientations.
	    May 12, 1989 - added layer information.
	    May 25, 1989 - changed tiles list to FIFO from LIFO for
		cell gridding..
	    Jul 19, 1989 - added numstdcells to save work for partition case
		with no hard or soft cells.
	    Sep 29, 1989 - remove max tile limit by making dynamic.
	    Oct 18, 1989 - Added tile to pad macros for density calc.
	    Apr 15, 1990 - Modified cur_inst fields.  Added vertices
		fields to cell box for placing pins.  Moved pintype
		field to PINBOX from SOFTBOX.  Made fixed field of
		GROUPBOX a pointer.
	    Apr 17, 1990 - added load_soft_pins which verifies
		user input and correctly sets up wire estimator.
	    Apr 23, 1990 - now handle pingroup instances and also
		fixed wire area estimator for instances by looking
		at all instances when calculating the perimeter.
	    Apr 26, 1990 - distinguish arbitrarily assigned layer
		information using negative numbers.
	    May  2, 1990 - Now all pins on softcells have softinfo
		records to avoid access violations.  Added error
		checking to processCorners to catch points not entered
		in a CW fashion.
	    Jun 21, 1990 - moved the location of genorient so that
		the pins will be in the correct orientation before the
		check.
	    Oct 1, 1990 - updated analog input data structures.
	    Sun Dec 16 00:38:44 EST 1990 - reworked analog data
		structures and added check for rectilinear pins.
	    Tue Dec 18 01:29:30 EST 1990 - make sure the buster
		check error messages go to screen properly.
	    Wed Dec 19 23:58:12 EST 1990 - added check_pos.
	    Dec 21, 1990 - added set_pin_pos and rewrote addPin
		to make pins more general.
	    Thu Jan 17 00:55:28 PST 1991 - added numpins initialization.
	    Wed Jan 23 14:42:30 PST 1991 - added findsidestr in order
		to make language context free.
	    Fri Jan 25 18:05:56 PST 1991 - now scale data correctly.
	    Wed Jan 30 14:13:36 EST 1991 - now left justify orientation
		rotations.
	    Mon Feb  4 02:10:28 EST 1991 - made changes to make new pin
		placement code work and fixed numsides bug.
	    Wed Feb 13 23:52:38 EST 1991 - modified for new pad code.
	    Sat Feb 23 00:21:10 EST 1991 - updated for new pad placement
		code.
	    Thu Apr 18 01:34:59 EDT 1991 - added more error checking
		and fixed problem with initial orientation.
	    Sat Apr 27 01:15:05 EDT 1991 - now detect doPartition case
		automatically.
	    Thu Aug 22 22:10:09 CDT 1991 - fixed problem with
		fixed cells moving during pairwise flips.
	    Fri Oct 18 00:06:37 EDT 1991 - moved buster code to library
		and updated for new NIL definition.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) initialize.c version 3.24 10/18/91" ;
#endif

#include <string.h>
#include <custom.h>
#include <pads.h>
#include <analog.h>
#include <initialize.h>
#include <yalecad/buster.h>
#include <yalecad/hash.h>
#include <yalecad/debug.h>
#include <yalecad/string.h>


/* below is what we expect to be a large floorplanning input */
/* user may change parameters if they wish. Subject to change */
#define EXPECTEDNUMCELLS   500
#define EXPECTEDNUMNETS   1000
#define EXPECTEDNUMUNCON  100
#define EXPECTEDNUMPADS   10
#define EXPECTEDCORNERS   8
#define EXPECTEDINSTANCES 1
#define EXPECTEDPINGRP    1

#define PIN   (BOOL)      TRUE       /* for pinFlag below */ 
#define EQUIV (BOOL)      FALSE 
#define NUMPADMACROS      4 /* there are always 4 pad macros */
#define INITIALIZE        (BOOL) TRUE  /* for updateFixedCells */ 

/* ERRORABORT is a macro which forces routines not to do any work */
/* when we find a syntax error in the input routine. */
#define ERRORABORT() \
{ \
    if( errorFlagS ){ \
	return ; /* don't do any work for errors */ \
    } \
} \

/* ######################  STATIC definitions ######################### */
static INT  curCellTypeS ;  /* current cell type - ie, softcell,pad etc.*/
static INT  cellinstanceS ; /* number of instances of current cell      */
static INT  cornerCountS ;  /* current number of corners                */
static INT  tileptAllocS ;  /* allocation allotted to tile structures   */
static INT  xcenterS, ycenterS ;/* current cell center coordinates      */
static INT  minxS, minyS ;  /* bounding box of current cell             */
static INT  maxxS, maxyS ;  /* bounding box of current cell             */
static INT  totPinS ;       /* total number of pins over all instances  */
static INT  netAllocS ;     /* current space in netarray                */
static INT  cellAllocS ;    /* current space in cellarray               */
static INT  childAllocS ;   /* current space in current pptr->children  */
static INT  instAllocS ;    /* current number of instance allocated     */
static INT  equivpinS ;   /* total number of equivs for the current pin */
static INT  totxS, totyS ;  /* counters for equiv pin postions          */
static INT  numchildrenS ;  /* # of children in current pad or pingroup */
static INT  numpingroupS ;  /* # of pingroups for this cell             */
static INT cur_restrict_objS ; /* set by set_restrict type              */
static char *curCellNameS ; /* current cell name                        */
static char *curPinNameS ;   /* current pin name                        */
static BOOL portFlagS ; /* tells whether port(TRUE) or not(FALSE)       */
static BOOL errorFlagS ;    /* switch allow us to find multiple errors  */
static BOOL analog_errorS ; /* whether an analog error occurs           */
static DOUBLE scaleS ;      /* how to scale the data                    */
static CELLBOXPTR ptrS ;    /* pointer to current cell box              */
static PADBOXPTR pptrS ;    /* pointer to current pad box               */
static YBUSTBOXPTR cornerArrayS;/* holds array of pts for cell boundary */
static YHASHPTR netTableS ;    /* hash table for cross referencing nets */
static YHASHPTR cellTableS ;   /* hash table for referencing cells      */
static PSIDEBOX   *pSideArrayS;/* used to calc the side weighs of cell  */
static KBOXPTR kArrayS ;       /* used in making sides for softpins     */
static GROUPBOXPTR curGroupS ; /* pointer to current group record       */
static PINBOXPTR   pinS ;      /* current active pin of cell            */
static PINBOXPTR   softpinS ;  /* current active softpin of cell        */
static PINBOXPTR pingroupS ;   /* current pin group record              */
static ANALOGPTR analogS ;     /* current analog record                 */
static SOFTBOXPTR spinptrS ;   /* current softpin record                */
static INSTBOXPTR instS ;      /* pointer to current inst record        */
PINBOXPTR findTerminal() ;
/* ################## END STATIC definitions ########################## */



static check_pos();



/* set processing switch to avoid work when an error is found */
setErrorFlag()
{
errorFlagS = TRUE ;
} /* end setErrorFlag */
/* ***************************************************************** */

/* return the net hash table */
YHASHPTR getNetTable()
{
return( netTableS ) ;
} /* end getNetTable */
/* ***************************************************************** */

/* initialize global and static information */
initCellInfo()
{
numcellsG = 0 ;
numnetsG = 0 ;
numsoftG = 0 ;
numstdcellG = 0 ;
numpadsG = 0 ;
numpadgroupsG = 0 ;
numsupercellsG = 0 ;
numinstancesG = 0 ;
numpinsG = 0 ;
totalcellsG = 0 ;
totalpadsG = 0 ;
errorFlagS = FALSE ;
analog_errorS = FALSE ;
totPinS = 0 ;
unique_classG = 0 ;
net_cap_matchG = NIL(INT **) ;
net_res_matchG = NIL(INT **) ;
scaleS = (DOUBLE) scale_dataG ;
cornerCountS = 0 ;
tileptAllocS = EXPECTEDCORNERS ;
cornerArrayS = ( YBUSTBOXPTR ) 
    Ysafe_malloc( tileptAllocS * sizeof( YBUSTBOX ) );
pSideArrayS  = (PSIDEBOX *) Ysafe_malloc( tileptAllocS * sizeof( PSIDEBOX ) ) ;
kArrayS      = (KBOXPTR) Ysafe_calloc( (MAXSITES + 1), sizeof( KBOX ));
/* make hash table for nets */
netTableS = Yhash_table_create( EXPECTEDNUMNETS ) ;
/* make hash table for cells */
cellTableS = Yhash_table_create( EXPECTEDNUMCELLS ) ;
netAllocS = EXPECTEDNUMNETS ;
netarrayG = (NETBOXPTR *) Ysafe_malloc( netAllocS * sizeof(NETBOXPTR));
cellAllocS = EXPECTEDNUMCELLS ;
cellarrayG = (CELLBOXPTR *)Ysafe_malloc(cellAllocS*sizeof(CELLBOXPTR));
} /* end initCellInfo */
/* ***************************************************************** */

/* cleanup operations at the end of readcells */
cleanupReadCells()
{
    INT cell ;           /* cell counter */
    PADBOXPTR padptr ;   /* current pad */
    CELLBOXPTR ptr ;     /* current pad cell */

if( errorFlagS || analog_errorS ){
    /* we found all our syntax errors so abort */
    closegraphics() ;
    YexitPgm( FAIL ) ;
}

/* the last cells - create pad macros for channel graph generator */
addCell("pad.macro.l",PADMACROTYPE ) ;
/* add a tile for processing borders */
ptrS->tiles = (TILEBOXPTR) Ysafe_calloc( 1, sizeof(TILEBOX) ) ;
endCell() ;
addCell("pad.macro.t",PADMACROTYPE ) ;
/* add a tile for processing borders */
ptrS->tiles = (TILEBOXPTR) Ysafe_calloc( 1, sizeof(TILEBOX) ) ;
endCell() ;
addCell("pad.macro.r",PADMACROTYPE ) ;
/* add a tile for processing borders */
ptrS->tiles = (TILEBOXPTR) Ysafe_calloc( 1, sizeof(TILEBOX) ) ;
endCell() ;
addCell("pad.macro.b",PADMACROTYPE ) ;
/* add a tile for processing borders */
ptrS->tiles = (TILEBOXPTR) Ysafe_calloc( 1, sizeof(TILEBOX) ) ;
endCell() ;

/* reallocate arrays to correct size */
netarrayG = (NETBOXPTR *) 
    Ysafe_realloc( netarrayG, (numnetsG+1) * sizeof(NETBOXPTR) ) ;
/* allocate and initialize terminal array */
termarrayG = (PINBOXPTR *) 
    Ysafe_calloc( (numpinsG + 1), sizeof( PINBOXPTR ) );
/* now trim cellarray to proper size */
endsuperG = numcellsG + numsupercellsG ;
endpadsG = endsuperG + numpadsG ;
endpadgrpsG = endpadsG + numpadgroupsG ;
ASSERT( totalcellsG == endpadgrpsG+NUMPADMACROS,"cleanupReadCells",
    "cells don't add up" ) ;
cellarrayG = (CELLBOXPTR *) 
    Ysafe_realloc( cellarrayG,(totalcellsG+1)* sizeof(CELLBOXPTR) ) ;

/* build the pad arrays */
padarrayG = (PADBOXPTR *) Yvector_alloc(1,totalpadsG,sizeof(PADBOXPTR)) ;
sortarrayG = (PADBOXPTR *) Yvector_alloc(1,totalpadsG,sizeof(PADBOXPTR));
placearrayG = (PADBOXPTR *) Yvector_alloc( 1,numpadsG,sizeof(PADBOXPTR)) ;
for( cell = 1; cell <= totalpadsG; cell++ ){
    ptr = cellarrayG[endsuperG + cell] ;
    padptr = ptr->padptr ;
    sortarrayG[cell] = padarrayG[cell] = padptr ;
    if( padptr->padtype == PADCELLTYPE ){
	placearrayG[cell] = padptr ;
    }
}

/* set loop index range variables for cellarray */
pinsPerLenG = (DOUBLE) totPinS / (DOUBLE) perimG ;
setpwates() ;
sortpins() ;
prnt_netinfo() ;   /* tell user net information */
genorient(1,endpadsG ) ;
updateFixedCells( INITIALIZE ) ;
loadTermArray() ;
build_active_array() ;
build_soft_array() ;
set_up_pinplace() ;   /* for soft cell pin placement */
update_pins(TRUE) ;  /* initial pin placement */

/* free memory */
Ysafe_free(cornerArrayS) ;
Ysafe_free(pSideArrayS) ;
Ysafe_cfree(kArrayS) ;
Ybuster_free() ;

} /* end cleanupReadCells */
/* ***************************************************************** */

/* add another cell to cell list and initialize fields */
addCell( cellName, cellType )
char *cellName ;
CELLTYPE  cellType ;
{
INT i ;
INT *data ;

curCellNameS = cellName ; /* for error messages */
curCellTypeS = cellType ;
ERRORABORT() ;

/* check memory of cell array */
if( ++totalcellsG >= cellAllocS ){
    cellAllocS += EXPECTEDNUMCELLS ;
    cellarrayG = (CELLBOXPTR *) 
	Ysafe_realloc( cellarrayG, cellAllocS*sizeof(CELLBOXPTR) );
}
ptrS = cellarrayG[totalcellsG] = (CELLBOXPTR) 
    Ysafe_malloc( sizeof(CELLBOX) ) ;

/* add cell to hash table */
data = (INT *) Ysafe_malloc( sizeof(INT) ) ;
*data = totalcellsG ;
if( Yhash_search( cellTableS, curCellNameS, (char *) data, ENTER ) ){
    sprintf( YmsgG, "Cellnames not unique:%s\n", curCellNameS ) ;
    M(ERRMSG,"addCell",YmsgG ) ;
    Ysafe_free( data ) ;
    errorFlagS = TRUE ;
} 
ptrS->cname = cellName ; /* memory allocation in yylex */
ptrS->celltype = cellType ;
ptrS->xcenter    = 0 ;
ptrS->ycenter    = 0 ;
ptrS->orient     = 0 ;
ptrS->numtiles   = 0 ;
ptrS->numpins    = 0 ;
ptrS->cur_inst   = 0 ;
ptrS->fixed = NULL ;
ptrS->group_nested = FALSE ;
ptrS->paths = NULL ;
ptrS->aspect = 1.0 ;
ptrS->aspUB = 1.0 ;
ptrS->aspLB = 1.0 ;
ptrS->pinptr = NULL ;
ptrS->softpins = NULL ;
ptrS->instptr = NULL ;
ptrS->padptr = NULL ;
ptrS->group = NULL ;
ptrS->tiles = NULL ;
ptrS->bounBox = NULL ;
ptrS->nets = NULL ;
ptrS->orientList[HOWMANYORIENT] = 0 ;
if( cellType == SOFTCELLTYPE || cellType == CUSTOMCELLTYPE ||
    cellType == STDCELLTYPE ){
    for( i = 0 ; i < 8 ; i++ ) {
	ptrS->orientList[i] = FALSE ;
    }
} else {
    /* for pads all orientations are valid */
    for( i = 0 ; i < 8 ; i++ ) {
	ptrS->orientList[i] = TRUE ;
    }
}
if( cellType == SOFTCELLTYPE ){
    ptrS->padptr = NULL ;
    ptrS->softflag = TRUE ;
    /* allocate space for uncommitted pins array */
    numcellsG++ ;
    numsoftG++ ;
    numpingroupS = 0 ;
} else if( cellType == STDCELLTYPE ){
    ptrS->padptr = NULL ;
    ptrS->softflag = TRUE ;
    /* allocate space for uncommitted pins array */
    numcellsG++ ;
    numstdcellG++ ;
    doPartitionG = TRUE ;
} else if( cellType == CUSTOMCELLTYPE){
    ptrS->softflag = FALSE ;
    ptrS->padptr = NULL ;
    numcellsG++ ;
} else if( cellType == PADCELLTYPE || cellType == PADGROUPTYPE){
    ptrS->softflag = FALSE ;
    pptrS =ptrS->padptr = 
	(PADBOXPTR) Ysafe_malloc( sizeof(PADBOX) ) ;
    pptrS->fixed = FALSE ;
    pptrS->padside = ALL ;
    pptrS->permute = FALSE ;
    pptrS->ordered = FALSE ;
    pptrS->lowerbound  = 0.0 ;
    pptrS->upperbound  = 1.0 ;
    pptrS->padtype = cellType ;
    pptrS->cellnum = totalcellsG ;
    pptrS->valid_side[0] = TRUE ;
    pptrS->valid_side[1] = FALSE ;
    pptrS->valid_side[2] = FALSE ;
    pptrS->valid_side[3] = FALSE ;
    pptrS->valid_side[4] = FALSE ;
    totalpadsG++ ;
    if( cellType == PADGROUPTYPE ){
	numchildrenS = 0 ;
	childAllocS = EXPECTEDNUMPADS ;
	pptrS->children = (INT *)
	    Ysafe_malloc((childAllocS)*sizeof(INT));
	pptrS->hierarchy  = ROOT;
	numpadgroupsG++ ;
    } else { /* PADCELLTYPE */
	pptrS->children = NULL;
	pptrS->hierarchy = NONE;
	numpadsG++ ;
    }
} else if( cellType == SUPERCELLTYPE || cellType == GROUPCELLTYPE ){
    numsupercellsG++ ;
    unique_classG-- ;/* start new exchange class for group cells */
    /* allocate group record and set static */
    curGroupS =ptrS->group = 
	(GROUPBOXPTR) Ysafe_malloc( sizeof(GROUPBOX) ) ;
    curGroupS->fixed = (FIXEDBOXPTR) Ysafe_calloc( 1,sizeof(FIXEDBOX) ) ;
    curGroupS->cells = NULL ;
} else if( cellType == PADMACROTYPE ){
    /* do nothing right now */
}
cellinstanceS = 0 ; /* beginning of possible instancelist */

/* save cellnumber */
ptrS->cellnum = totalcellsG ;

/* reset xmin, ymax, etc. counters */
minxS = INT_MAX ;
minyS = INT_MAX ;
maxxS = INT_MIN ;
maxyS = INT_MIN ;
cornerCountS = 0 ;
portFlagS = FALSE ;

} /* end addCell */
/* ***************************************************************** */

/* perform cleanup operations on a cell */
endCell()
{
ERRORABORT() ;

/* set the pSideArray for the softpins */
load_soft_pins( ptrS, pSideArrayS ) ;

if( curCellTypeS == CUSTOMCELLTYPE || curCellTypeS == SOFTCELLTYPE ||
    curCellTypeS == STDCELLTYPE ){
    watesides( ptrS, pSideArrayS ) ;
}
if( curCellTypeS == SOFTCELLTYPE || curCellTypeS == STDCELLTYPE ){
} else if( curCellTypeS == PADGROUPTYPE){
    /* realloc size of children array to final size */
    pptrS->children = (INT *)
	Ysafe_realloc( pptrS->children,(numchildrenS+1) * sizeof(INT));
    pptrS->children[HOWMANY] = numchildrenS ;
}
if( cellinstanceS ){
    /* first save values to instance arrays */
    instS->tile_inst[cellinstanceS] = ptrS->tiles ;
    instS->vert_inst[cellinstanceS] = ptrS->vertices ;
    instS->numtile_inst[cellinstanceS] = ptrS->numtiles ;
    instS->name_inst[cellinstanceS] = ptrS->cname ;
    instS->bounBox[cellinstanceS] = ptrS->bounBox ;
    instS->numsides[cellinstanceS] = ptrS->numsides ;
    ptrS->cur_inst = cellinstanceS ;
}

} /* end function endCell */

/* ***************************************************************** */
static INT findsidestr( side, direction )
char *side ;
BOOL direction ;
{
    if( direction == TRUE ){
	/* BT case */
	if( strcmp( side, "B" ) == STRINGEQ ){
	    return( TRUE ) ;
	} else if( strcmp( side, "T" ) == STRINGEQ ){
	    return( FALSE ) ;
	}
    } else {
	/* LR case */
	if( strcmp( side, "L" ) == STRINGEQ ){
	    return( TRUE ) ;
	} else if( strcmp( side, "R" ) == STRINGEQ ){
	    return( FALSE ) ;
	}
    }
    sprintf( YmsgG, "Unknown side:%s\n", side ) ;
    M(ERRMSG,"findside", YmsgG ) ;
    setErrorFlag() ;
    return( FALSE ) ;
} /* end findsidestr */

/* ***************************************************************** */

fixCell( fixedType, xloc, lorR, yloc, borT, xloc2, lorR2, yloc2, borT2 )
INT fixedType ;  /* valid types - neighborhood. point, group */
INT xloc, yloc, xloc2, yloc2 ;
char *lorR, *borT, *lorR2, *borT2 ;
{

INT leftOrRight, bottomOrTop ;
INT leftOrRight2, bottomOrTop2 ;
FIXEDBOXPTR fixptr ;

if( scale_dataG ){
    xloc  = (INT) ( (DOUBLE) xloc / scaleS )  ;
    yloc  = (INT) ( (DOUBLE) yloc / scaleS ) ;
    xloc2 = (INT) ( (DOUBLE) xloc2 / scaleS ) ;
    yloc2 = (INT) ( (DOUBLE) yloc2 / scaleS ) ;
}

leftOrRight = findsidestr( lorR, FALSE ) ; /* left right */
bottomOrTop = findsidestr( borT, TRUE ) ;  /* bottom top */
leftOrRight2 = findsidestr( lorR2, FALSE ) ; /* left right */
bottomOrTop2 = findsidestr( borT2, TRUE ) ;  /* bottom top */

if( fixedType == GROUPFLAG || fixedType == FIXEDGROUPFLAG ){
    fixptr = ptrS->group->fixed ;
    if( fixedType == FIXEDGROUPFLAG ){
	/* note that this cell has been fixed */
       ptrS->fixed = (FIXEDBOXPTR) TRUE ;
    }
} else {
    fixptr =ptrS->fixed = (FIXEDBOXPTR) 
	Ysafe_malloc( sizeof(FIXEDBOX) ) ;
}
fixptr->fixedType = fixedType ;
fixptr->xcenter = xloc ;
fixptr->ycenter = yloc ;
if( leftOrRight == TRUE ){
    fixptr->leftNotRight = TRUE ;
} else if( leftOrRight == FALSE ){
    fixptr->leftNotRight = FALSE ;
} else {
    M(ERRMSG,"fixCell", "Problem passing arguments to function\n" ) ;
    setErrorFlag() ;
}

if( bottomOrTop == TRUE ){
    fixptr->bottomNotTop = TRUE ;
} else if( bottomOrTop == FALSE ){
    fixptr->bottomNotTop = FALSE ;
} else {
    M(ERRMSG,"fixCell", "Problem passing arguments to function\n" ) ;
    setErrorFlag() ;
}
if( fixedType != POINTFLAG ){
    fixptr->xloc1 = xloc ;
    fixptr->yloc1 = yloc ;
    fixptr->xloc2 = xloc2 ;
    fixptr->yloc2 = yloc2 ;
    if( leftOrRight2 == TRUE ){
	fixptr->leftNotRight2 = TRUE ;
    } else if( leftOrRight2 == FALSE ){
	fixptr->leftNotRight2 = FALSE ;
    } else {
	M(ERRMSG,"fixCell","Problem passing arguments to function\n");
	setErrorFlag() ;
    }

    if( bottomOrTop2 == TRUE ){
	fixptr->bottomNotTop2 = TRUE ;
    } else if( bottomOrTop2 == FALSE ){
	fixptr->bottomNotTop2 = FALSE ;
    } else {
	M(ERRMSG,"fixCell","Problem passing arguments to function\n");
	setErrorFlag() ;
    }
}

} /* end fixCell */

processCorners( numcorners )
INT numcorners ;
{
char *buster_msg ;           /* message string to used by buster */
INT xx1, yy1, xx2, yy2 ;     /* temp points */
INT k ;                      /* point counter */
INT xsum, ysum ;             /* used to truncate cell correctly */
TILEBOXPTR tile ;
YBUSTBOXPTR busterptr ;      /* return record for busting routine */
VERTBOXPTR vert ;            /* ptr to record of vertices of cell */
BOUNBOXPTR bounptr,          /* ptr to bounBox[0] */
    bounptr_orig ;           /* ptr to bounBox[8] - original bbox */


/* +++++++++++++++++++ perform error checking +++++++++++++++++++ */
ERRORABORT() ;
if( cornerCountS != numcorners ){
    sprintf( YmsgG, "Incorrect number of corners for cell:%s\n",
	curCellNameS ) ;
    M(ERRMSG,"processCorners", YmsgG ) ;
    setErrorFlag() ;
}
if( scale_dataG ){
    if( maxxS - minxS <= 1 || maxyS - minyS <= 1 ){
	sprintf( YmsgG, 
	"Cell:%s smaller than given scale data - resized to portsize.\n",
	curCellNameS ) ;
	M( WARNMSG, "processCorners", YmsgG ) ;
	/* make into a port instead of a possible 0 width object */
	numcorners = cornerCountS = 4 ;
	cornerArrayS[1].x = -1 ; cornerArrayS[1].y = -1 ;
	cornerArrayS[2].x = -1 ; cornerArrayS[2].y =  1 ;
	cornerArrayS[3].x =  1 ; cornerArrayS[3].y =  1 ;
	cornerArrayS[4].x =  1 ; cornerArrayS[4].y = -1 ;
	minxS = -1 ; maxxS = 1;
	minyS = -1 ; maxyS = 1;
	/* set portFlag to true so we can move pins to boundary */
	portFlagS = TRUE ;
    }
}
xsum = minxS + maxxS ;
ysum = minyS + maxyS ;
xcenterS = xsum / 2 ;
ycenterS = ysum / 2 ;
if( xsum < 0 && xsum % 2 ){
    /* cell is asymmetrical and negative */
    xcenterS-- ;
}
if( ysum < 0 && ysum % 2 ){
    /* cell is asymmetrical and negative */
    ycenterS-- ;
}

/* save the number of sides */
ptrS->numsides = numcorners ;

Ybuster_init() ;
sprintf( YmsgG, " (cell:%s) ", curCellNameS ) ;
buster_msg = Ystrclone( YmsgG ) ;

for( k = 1 ; k <= numcorners ; k++ ) {
    xx1 = cornerArrayS[k].x ;
    yy1 = cornerArrayS[k].y ;
    Ybuster_addpt( xx1, yy1 ) ;
    if( k == numcorners ) {
	xx2 = cornerArrayS[1].x ;
	yy2 = cornerArrayS[1].y ;
    } else {
	xx2 = cornerArrayS[ k + 1 ].x ;
	yy2 = cornerArrayS[ k + 1 ].y ;
    }
}
if(!(Ybuster_verify( buster_msg ))){
    setErrorFlag() ;
    return ;
}
Ysafe_free( buster_msg ) ;

/* +++++++++++++++++++ end perform error checking +++++++++++++++++++ */
/* This data structure allow quick change between x and x_new */
/* since placepin always uses the x fields and not x_new */
vert = ptrS->vertices = (VERTBOXPTR) Ysafe_malloc( sizeof(VERTBOX) ) ;
/* the members of the structure */
vert->x = (INT *) Ysafe_malloc( (numcorners+1)*sizeof(INT) ) ;
vert->x_orig = (INT *) Ysafe_malloc( (numcorners+1)*sizeof(INT) ) ;
vert->x_new = (INT *) Ysafe_malloc( (numcorners+1)*sizeof(INT) ) ;
vert->y = (INT *) Ysafe_malloc( (numcorners+1)*sizeof(INT) ) ;
vert->y_orig = (INT *) Ysafe_malloc( (numcorners+1)*sizeof(INT) ) ;
vert->y_new = (INT *) Ysafe_malloc( (numcorners+1)*sizeof(INT) ) ;
vert->numpins = (FLOAT *) Yvector_alloc( 1,numcorners,sizeof(FLOAT) ) ;
for( k = 1 ; k <= numcorners ; k++ ) {
    vert->x[k] = vert->x_orig[k] = cornerArrayS[k].x - xcenterS ;
    vert->y[k] = vert->y_orig[k] = cornerArrayS[k].y - ycenterS ;
} 
if( curCellTypeS == SOFTCELLTYPE || curCellTypeS == CUSTOMCELLTYPE ||
    curCellTypeS == STDCELLTYPE ){
    /* calculate total cell perimeter before bust operation */
    /* calculate over all instances */
    perimG += perimeter( cornerArrayS, numcorners ) ;
    /* load pSideArray for weighing cells in dynamic wiring estimator */
    for( k = 1 ; k <= numcorners ; k++ ) {
	if( k < numcorners ) {
	    if( k % 2 == 1 ) {
		pSideArrayS[k].length = 
		    ABS(cornerArrayS[k + 1].y - cornerArrayS[k].y) ;
		pSideArrayS[k].vertical = 1 ;
		pSideArrayS[k].pincount = 0 ;
		pSideArrayS[k].position = cornerArrayS[k].x ;
	    } else {
		pSideArrayS[k].length = 
		    ABS(cornerArrayS[k + 1].x - cornerArrayS[k].x) ;
		pSideArrayS[k].vertical = 0 ;
		pSideArrayS[k].pincount = 0 ;
		pSideArrayS[k].position = cornerArrayS[k].y ;
	    }
	} else {
	    pSideArrayS[k].length = 
		ABS(cornerArrayS[1].x - cornerArrayS[k].x) ;
	    pSideArrayS[k].vertical = 0 ;
	    pSideArrayS[k].pincount = 0 ;
	    pSideArrayS[k].position = cornerArrayS[k].y ;
	}
    } /* end for loop */
}
/* -----------------now build tiles for cell---------------------- */

/* create bounding box of cell bounding box has 8 view for all cells*/
/* plus 9 field for original view of the bounding box */
ptrS->bounBox = (BOUNBOXPTR *) 
    Ysafe_malloc( 9 * sizeof( BOUNBOXPTR ) ) ;
for( k=0;k<=8;k++){
   ptrS->bounBox[k] = (BOUNBOXPTR) 
	Ysafe_malloc(sizeof( BOUNBOX ) ) ;
}
ptrS->boun_valid = FALSE ;
/* now save the original bounding box for uaspect in 8 */
bounptr = ptrS->bounBox[0] ;
bounptr_orig = ptrS->bounBox[8] ;
bounptr_orig->l = bounptr->l = minxS - xcenterS ;
bounptr_orig->r = bounptr->r = maxxS - xcenterS ;
bounptr_orig->b = bounptr->b = minyS - ycenterS ;
bounptr_orig->t = bounptr->t = maxyS - ycenterS ;
ptrS->xcenter = xcenterS ; 
ptrS->ycenter = ycenterS ;
    
tile = NIL(TILEBOXPTR) ;
while( busterptr = Ybuster() ){
    /* l = busterptr[1].x */
    /* r = busterptr[4].x */
    /* b = busterptr[1].y */
    /* t = busterptr[2].y */
    if( tile ){
	tile->next = (TILEBOXPTR) Ysafe_malloc( sizeof( TILEBOX ));
	tile = tile->next ;
    } else {
	tile = ptrS->tiles = 
	    (TILEBOXPTR) Ysafe_malloc( sizeof( TILEBOX ));
    }
    tile->next = NULL ;
    ptrS->numtiles++ ;
    tile->left   = tile->orig_left   = busterptr[1].x - xcenterS ;
    tile->right  = tile->orig_right  = busterptr[4].x - xcenterS ;
    tile->bottom = tile->orig_bottom = busterptr[1].y - ycenterS ;
    tile->top    = tile->orig_top    = busterptr[2].y - ycenterS ;
    tile->lweight = 0.0 ;
    tile->rweight = 0.0 ;
    tile->bweight = 0.0 ;
    tile->tweight = 0.0 ;
}

/* add aspect ratio to design */
ptrS->orig_aspect = ptrS->aspect = 
    (DOUBLE)(maxyS - minyS) / (DOUBLE)(maxxS - minxS);

/* -----------------end build tiles for cell---------------------- */
} /* end processCorners */
/* ***************************************************************** */

addCorner( xpos, ypos )
INT xpos, ypos ;
{
if( ++cornerCountS >= tileptAllocS ){
    tileptAllocS = cornerCountS + 1 ;
    cornerArrayS = ( YBUSTBOXPTR ) 
	Ysafe_realloc( cornerArrayS, tileptAllocS * sizeof( YBUSTBOX ) );
    pSideArrayS  = (PSIDEBOX *) 
	Ysafe_realloc( pSideArrayS, tileptAllocS * sizeof( PSIDEBOX ) ) ;
}
if( scale_dataG ){
    if( portFlagS ){
	/* port pins need to be on boundary for global router */
	xpos = 0 ;
	ypos = 1 ;
    } else {
	xpos = (INT) ( (DOUBLE) xpos / scaleS ) ;
	ypos = (INT) ( (DOUBLE) ypos / scaleS ) ;
    }
}
cornerArrayS[cornerCountS].x = xpos ;
cornerArrayS[cornerCountS].y = ypos ;
minxS = MIN( minxS, xpos ) ;
maxxS = MAX( maxxS, xpos ) ;
minyS = MIN( minyS, ypos ) ;
maxyS = MAX( maxyS, ypos ) ;
} /* end addCorner */
/* ***************************************************************** */

initializeCorner( cell )
INT cell ;
{
ptrS = cellarrayG[cell] ;
curCellTypeS = ptrS->celltype ;
cornerCountS = 0 ;
} /* end initializeCorner */
/* ***************************************************************** */

addClass( class )
INT class ;
{
ERRORABORT() ;

if( ptrS->class >= 0 ){
    /* this test is necessary since fix neighborhood may set it negative */
    /* and we will not want to touch the value of it */
    ptrS->class = class ;
}
} /* end addClass */
/* ***************************************************************** */

/* first in the list is the initial orientation */
initOrient( orient )
INT orient ;
{
ERRORABORT() ;

ptrS->orient = - orient ;
addOrient( orient ) ;
} /* end initOrient */
/* ***************************************************************** */

/* addOrient sets orientation valid for this cell */
addOrient( orient )
INT orient ;
{
ERRORABORT() ;

ptrS->orientList[orient] = TRUE ; /* allow this orientation */
ptrS->orientList[HOWMANYORIENT]++ ;
} /* end addOrient */
/* ***************************************************************** */

/* if this routine is called it means we are reading the input of
   a previous TimberWolf run.  
*/
set_cur_orient( orient )
INT orient ;
{
ERRORABORT() ;

ptrS->orient = orient ;
} /* end set_cur_orient */
/* ***************************************************************** */

/* load aspect ratios */
addAspectBounds( lowerBound, upperBound )
DOUBLE lowerBound, upperBound ;
{
ERRORABORT() ;

ptrS->aspLB = lowerBound ;
ptrS->aspUB = upperBound ;

} /* end addAspectBounds */
/* ***************************************************************** */

PINBOXPTR addPinAndNet( pinName, signal )
char *pinName, *signal ;
{
    static PINBOXPTR botpinS ; /* keep track of end of list */
    INT *data, netx ;
    BOOL notInTable ;
    PINBOXPTR pinptr ;
    NETBOXPTR netptr ;

    notInTable = TRUE ;
    totPinS++ ; /* count the pins over all instances */
    if( data = (INT *) Yhash_search( netTableS, signal, NULL, FIND ) ){
	netx = *data ;
	notInTable = FALSE ;
    } else {
	/* else a new net load data holder */
	data = (INT *) Ysafe_malloc( sizeof(INT) ) ;
	*data = netx = ++numnetsG ;
	if( Yhash_search( netTableS, signal, (char*) data, ENTER )){
	    sprintf( YmsgG, "Trouble adding signal:%s to hash table\n",
		signal ) ;
	    M(ERRMSG,"addPinAndNet",YmsgG ) ;
	    errorFlagS = TRUE ;
	}
    }
    if( cellinstanceS ){
	if( notInTable ){
	    sprintf(YmsgG,"No match for net:%s in primary instance:%s\n",
		pinName, *ptrS->cname ) ;
	    M( ERRMSG, "addPinAndNet", YmsgG ) ;
	    errorFlagS = TRUE ;
	}
	/* at this point we are done */ 
	return( NULL ) ;
    } 
    /* increment number of pins */
    numpinsG++ ;
    /* check memory of netarray */
    if( numnetsG >= netAllocS ){
	netAllocS += EXPECTEDNUMNETS ;
	netarrayG = (NETBOXPTR *) 
	    Ysafe_realloc( netarrayG, netAllocS * sizeof(NETBOXPTR) ) ;
    }

    /* see if this is the first time for this signal */
    if( notInTable ){
	netptr = netarrayG[netx] =
	    (NETBOXPTR) Ysafe_malloc( sizeof(NETBOX) ) ;
	netptr->nname = signal ; /* allocated by yylex */
	netptr->pins = NULL ; /* initialize list */
	netptr->paths = NULL ; /* initialize list */
	netptr->skip = 0 ; /* initialize list */
	netptr->driveFactor = (FLOAT) 0.0 ; /* initialize timing */
	netptr->max_driver = (FLOAT) 1.0 ; /* initialize timing */
	netptr->min_driver = (FLOAT) 1.0 ; /* initialize timing */
	netptr->analog_info = NIL(ANETPTR) ;
    } else {
	Ysafe_free( signal ) ; /* no need to keep this copy */
	netptr = netarrayG[netx] ;
    }
    /* create list of pins on the net */
    pinptr = (PINBOXPTR) Ysafe_malloc( sizeof(PINBOX) );
    pinptr->next = netptr->pins ;
    netptr->pins = pinptr ;

    /* create a list of pins for this cell in order as given */
    if( ptrS->pinptr ){ /* list has already been started */
	botpinS->nextpin = pinptr ;
    } else {  /* start new list */
	ptrS->pinptr = pinptr ;
    }
    pinptr->nextpin = NULL ;
    botpinS = pinptr ;

    /* now initialize data */
    pinptr->pinname    = pinName ; /* allocated by yylex */
    pinptr->pin        = numpinsG ;
    pinptr->net        = netx ;
    pinptr->cell       = totalcellsG ;
    pinptr->xpos       = 0    ;
    pinptr->ypos       = 0    ;
    pinptr->instance   = cellinstanceS ;
    /* allocate only size for one instance by default */
    pinptr->txpos      = 0 ;
    pinptr->typos      = 0 ;
    pinptr->txpos_orig = (INT *) Ysafe_malloc( sizeof(INT) ) ;
    pinptr->typos_orig = (INT *) Ysafe_malloc( sizeof(INT) ) ;
    pinptr->newx     = 0    ;
    pinptr->newy     = 0    ;
    pinptr->flag     = 0    ;
    pinptr->skip     = 0    ;
    pinptr->eqptr  = NULL ;
    pinptr->analog  = NIL(ANALOGPTR) ;

    return( pinptr ) ;

} /* add pin and net */
/* ***************************************************************** */

addPin( pinName, signal, layer, pinType )
char *pinName ;
char *signal ;
INT layer ;
INT pinType ;
{
    INT side ;
    INT howmany ;            /* number of children - equivs */
    SOFTBOXPTR sptr ;        /* current soft information */


    curPinNameS = pinName ;
    analogS = NIL(ANALOGPTR) ;
    ERRORABORT() ;

    if( pinType == SOFTEQUIVTYPE ){
        /* get signal name from soft pin */
	signal = Ystrclone( netarrayG[softpinS->net]->nname ) ;
    }

    pinS = addPinAndNet( pinName, signal ) ;

    equivpinS = 1 ;

    /* Note: initial pin positions will be last instance */
    if( pinS ){
	ptrS->numpins++ ;
	pinS->type = pinType ;
	pinS->layer = layer ;
    } else if( cellinstanceS ){
	if(!(pinS = findTerminal( pinName, totalcellsG))){
	    sprintf(YmsgG,"No match for pin:%s in primary instance:%s\n",
		pinName, cellarrayG[totalcellsG]->cname ) ;
	    M( ERRMSG, "addHardPin", YmsgG ) ;
	    errorFlagS = TRUE ;
	    return ;
	}
	/* reallocate space if necessary */
	pinS->txpos_orig = (INT *) 
	    Ysafe_realloc( pinS->txpos_orig,(cellinstanceS+1)*sizeof(INT));
	pinS->typos_orig = (INT *) 
	    Ysafe_realloc( pinS->typos_orig,(cellinstanceS+1)*sizeof(INT));
	pinS->txpos = pinS->txpos_orig[cellinstanceS] = 0 ;
	pinS->typos = pinS->typos_orig[cellinstanceS] = 0 ;
    } 

    /* now handle soft pin information */
    if( ptrS->softflag ){
	/* allocate space for array of instances */
	if( cellinstanceS == 0 ){
	    pinS->soft_inst = (SOFTBOXPTR *) 
		Ysafe_malloc( sizeof(SOFTBOXPTR) );
	    pinS->layer = layer ;
	} else {
	    pinS->soft_inst = (SOFTBOXPTR *) Ysafe_realloc( 
		pinS->soft_inst,instAllocS * sizeof(SOFTBOXPTR) );
	}
	/*  now allocate space for this instance */
	spinptrS = pinS->soft_inst[cellinstanceS] = pinS->softinfo =
	    (SOFTBOXPTR) Ysafe_malloc(sizeof(SOFTBOX)) ;
	spinptrS->children = NULL ;
	spinptrS->hierarchy = NONE ;
	spinptrS->parent = NULL ;
	/* build the restrict field and initialize HOWMANY [0] to 0 */
	spinptrS->restrict1 = (INT *) Ysafe_calloc( 1, sizeof(INT) ) ;
	spinptrS->permute = FALSE ;
	spinptrS->fixed = FALSE ;
	spinptrS->ordered = FALSE ;
	spinptrS->lowerbound = 0.0 ;
	spinptrS->upperbound = 1.0 ;
    }

    if( pinType == SOFTPINTYPE ){
	/* save the main softpin for future equiv use */
	softpinS = pinS ;
    } else if( pinType == SOFTEQUIVTYPE ){
	/* add to this pin's children */
	sptr = softpinS->softinfo ;
	if( sptr->children ){
	    howmany = (INT) sptr->children[HOWMANY] ;
	    sptr->children = (PINBOXPTR *)
		Ysafe_realloc( sptr->children,
		    (++howmany+1) * sizeof(PINBOXPTR) ) ;
	} else {
	    howmany = 1 ;
	    sptr->children = (PINBOXPTR *)
		Ysafe_malloc( (howmany+1) * sizeof(PINBOXPTR) ) ;
	}
	sptr->children[HOWMANY] = (PINBOXPTR) howmany ; 
	sptr->children[howmany] = pinS ;
	spinptrS->parent = softpinS ;
    } else if( !(ptrS->softflag) ){
	pinS->softinfo = NULL ;
    }

} /* end addPin */
/* ***************************************************************** */

set_pin_pos( xpos, ypos )
INT xpos, ypos ;
{
    INT side ;
    INT howmany ;            /* number of children - equivs */

    ERRORABORT() ;

    if( scale_dataG ){
	if( portFlagS ){
	    xpos = 0 ;
	    ypos = 1 ;
	} else {
	    xpos = (INT) ( (DOUBLE) xpos / scaleS ) ;
	    ypos = (INT) ( (DOUBLE) ypos / scaleS ) ;
	}
    }

    check_pos( curPinNameS, xpos, ypos ) ;
    side = findside( pSideArrayS, ptrS , xpos , ypos ) ;
    loadside( pSideArrayS, side , 1.0 ) ;

    totxS = xpos ;
    totyS = ypos ;

    /* set global coordinates */
    pinS->xpos = xpos ;
    pinS->ypos = ypos ;

    xpos -= xcenterS ;
    ypos -= ycenterS ;

    /* Note: initial pin positions will be last instance */
    pinS->txpos = pinS->txpos_orig[cellinstanceS] = xpos ;
    pinS->typos = pinS->typos_orig[cellinstanceS] = ypos ;
} /* end set_pin_pos */
/* ***************************************************************** */

static check_pos( pinname, xpos, ypos )
char *pinname ;
INT xpos, ypos ;
{
    if( xpos < minxS || xpos > maxxS || ypos < minyS || ypos > maxyS ){
	sprintf( YmsgG, "Pin:%s cell:%s @(%d,%d) is outside cell boundary\n",
	    pinname, curCellNameS, xpos, ypos ) ;
	M( ERRMSG, "check_pos", YmsgG ) ;
	setErrorFlag() ;
    }
} /* end check_pos */

/* add an equivalent pin-updates the pin position to effective position */
addEquivPin( pinName, layer, xpos, ypos, pinType )
char *pinName ;
INT layer ; 
INT xpos, ypos ;
INT pinType ;
{
    INT side ;
    EQUIVPTR temp, eqptr ;

    curPinNameS = pinName ;
    ERRORABORT() ;
    ASSERTNRETURN( pinS, "addEquivPin", "pinS is NULL" ) ;

    if( pinType != ADDEQUIVTYPE ){
	if( scale_dataG ){
	    if( portFlagS ){
		xpos = 0 ;
		ypos = 1 ;
	    } else {
		xpos = (INT) ( (DOUBLE) xpos / scaleS ) ;
		ypos = (INT) ( (DOUBLE) ypos / scaleS ) ;
	    }
	}

	if( curCellTypeS == SOFTCELLTYPE || 
	    curCellTypeS == CUSTOMCELLTYPE ||
	    curCellTypeS == STDCELLTYPE ){
	    check_pos( pinName, xpos, ypos ) ;
	    side = findside( pSideArrayS, ptrS , xpos , ypos ) ;
	    loadside( pSideArrayS, side , 1.0 ) ;
	}
	equivpinS++ ;
	totPinS++ ;
	totxS += xpos ;
	totyS += ypos ;

	/* average pin positions - note we leave _orig fields untouched */
	pinS->txpos = (totxS / equivpinS ) - xcenterS;
	pinS->typos = (totyS / equivpinS ) - ycenterS;
    } else {
	/* get pinname from main soft pin */
	pinName = softpinS->pinname ;
    }

    /* now save locations of equivalent pins for later output */
    if( temp = pinS->eqptr ){
	eqptr = pinS->eqptr = (EQUIVPTR) Ysafe_malloc(sizeof(EQUIVBOX)) ;
	eqptr->next = temp ;
    } else {
	eqptr = pinS->eqptr = (EQUIVPTR) Ysafe_malloc(sizeof(EQUIVBOX)) ;
	eqptr->next = NULL ;
    }
    if( cellinstanceS ){ /* more than one instance */
	eqptr->txpos = (INT *) 
	    Ysafe_realloc( eqptr->txpos,(cellinstanceS+1) * sizeof(INT) ) ;
	eqptr->typos = (INT *) 
	    Ysafe_realloc( eqptr->typos,(cellinstanceS+1) * sizeof(INT) ) ;
    } else {
	/* by default only expect one instance */
	eqptr->txpos      = (INT *) Ysafe_malloc( sizeof(INT) ) ;
	eqptr->typos      = (INT *) Ysafe_malloc( sizeof(INT) ) ;
    }
    /* now load the data */
    eqptr->txpos[cellinstanceS] = xpos - xcenterS;
    eqptr->typos[cellinstanceS] = ypos - ycenterS;
    eqptr->pinname = pinName ;
    /* if the user didn't give us any layer set to 2 for now */
    if( layer == 0 ){
	layer = -2 ;
    }
    eqptr->layer = layer ;
    if( pinType == ADDEQUIVTYPE ){
	/* build the restrict field and initialize HOWMANY [0] to 0 */
	eqptr->restrict1 = (INT *) Ysafe_calloc( 1, sizeof(INT) ) ;
    } else {
	eqptr->restrict1 = NULL ;
    }

} /* end addEquivPin */
/* ***************************************************************** */

set_restrict_type( object )
INT object ;
{
    cur_restrict_objS = object ;
}
/* ***************************************************************** */

addSideRestriction( side )
INT side ;
{
INT howmany ;
SOFTBOXPTR spin ;
EQUIVPTR eqptr ;

ERRORABORT() ;
if( side < 0 || side > cornerCountS ) {
    M(ERRMSG,"addSideRestriction","value of side out of range");
    sprintf(YmsgG,"\n\tside:%d  range:1 to %d ", side, 
	cornerCountS ) ;
    M(MSG,NULL,YmsgG);
    sprintf(YmsgG, "current cell is:%d\n", totalcellsG ) ;
    M(MSG,NULL,YmsgG);
    setErrorFlag() ;
}

/* this variable set by set_restrict type */
switch( cur_restrict_objS ){ 
    case PINGROUPTYPE:
	spin = pingroupS->softinfo ;
	howmany = ++(spin->restrict1[HOWMANY]) ;
	spin->restrict1 = (INT *) Ysafe_realloc( spin->restrict1,
	    (howmany+1)*sizeof(INT) ) ;
	spin->restrict1[howmany] = side ;
	break ;
    case SOFTPINTYPE:
    case SOFTEQUIVTYPE:
	spin = pinS->softinfo ;
	howmany = ++(spin->restrict1[HOWMANY]) ;
	spin->restrict1 = (INT *) Ysafe_realloc( spin->restrict1,
	    (howmany+1)*sizeof(INT) ) ;
	spin->restrict1[howmany] = side ;
	break ;
    case ADDEQUIVTYPE:
	eqptr = softpinS->eqptr ;
	howmany = ++(eqptr->restrict1[HOWMANY]) ;
	eqptr->restrict1 = (INT *) Ysafe_realloc( eqptr->restrict1,
	    (howmany+1)*sizeof(INT) ) ;
	eqptr->restrict1[howmany] = side ;
	break ;
} /* end switch on current object */

} /* end addSideRestriction *
/* ***************************************************************** */


add_pinspace( lower, upper )
DOUBLE lower ;
DOUBLE upper ;
{
    char *name ;         /* name of current object */
    SOFTBOXPTR spin ;    /* soft pin information of current pin or pg */

    ERRORABORT() ;

    switch( cur_restrict_objS ){ 
	case PINGROUPTYPE:
	    spin = pingroupS->softinfo ;
	    name = pingroupS->pinname ;
	    break ;
	case SOFTPINTYPE:
	case SOFTEQUIVTYPE:
	    spin = pinS->softinfo ;
	    name = pinS->pinname ;
	    break ;
	case ADDEQUIVTYPE:
	    return ;
    } /* end switch on current object */

    spin->fixed = TRUE ;
    if( lower > 1.0 || upper > 1.0 ){
	sprintf(YmsgG,
	    "side space must be less or equal to 1.0 for pin: %s\n", name ) ;
	M(ERRMSG,"add_pinspace",YmsgG ) ;
	setErrorFlag() ;
    }
    if( lower < 0.0 || upper < 0.0 ){
	sprintf(YmsgG,
	    "side space must be greater or equal to 0.0 for pin: %s\n",name ) ;
	M(ERRMSG,"add_pinspace",YmsgG ) ;
	setErrorFlag() ;
    }
    if( lower > upper ){
	sprintf(YmsgG,
	    "side space upper bound must be greater or equal to lower bound for pin: %s\n",
	    name ) ;
	M(ERRMSG,"add_pinspace",YmsgG ) ;
	setErrorFlag() ;
    }
    spin->lowerbound = lower ;
    spin->upperbound = upper ;
} /* end add_pinspace */
/* ***************************************************************** */

add_soft_array()
{
    INT i ;
    PINBOXPTR *sarray ; 
    PINBOXPTR pin ;

    if( cellinstanceS != 0 ){
	return ;
    }
    sarray = ptrS->softpins = (PINBOXPTR *) 
	Ysafe_malloc( (ptrS->numpins+1) * sizeof(PINBOXPTR) ) ;
    i = 0 ;
    for( pin = ptrS->pinptr; pin ; pin = pin->nextpin ){
	sarray[++i] = pin ;
    }
    sarray[HOWMANY] = (PINBOXPTR) i ;
}
/* end add_soft_array */

start_pin_group( pingroup, permute )
char *pingroup ;
BOOL permute ;
{

INT i ;
INT curpingroup ;
INT howmany ;  
PINBOXPTR *sarray ; 
SOFTBOXPTR spin ;

ERRORABORT() ;

numchildrenS = 0 ;
childAllocS = 1 ;

/* allocate space for array of instances */
if( cellinstanceS == 0 ){
    /* pins in array go 1 ... numpins, pingroup1, pingroup2 ... */
    curpingroup = ptrS->numpins + ++numpingroupS ;
    sarray = ptrS->softpins = (PINBOXPTR *) 
	    Ysafe_realloc( ptrS->softpins,
	    (curpingroup+1) * sizeof(PINBOXPTR) ) ;

    sarray[HOWMANY] = (PINBOXPTR) curpingroup ;

    /* allocate space for pingroup */
    pingroupS = sarray[curpingroup] = (PINBOXPTR) 
	Ysafe_calloc( 1, sizeof(PINBOX) );

    /* now initialize data */
    pingroupS->pinname    = pingroup ; /* allocated by yylex */
    pingroupS->txpos_orig = (INT *) Ysafe_malloc( sizeof(INT) ) ;
    pingroupS->typos_orig = (INT *) Ysafe_malloc( sizeof(INT) ) ;
    pingroupS->type = PINGROUPTYPE ;

    pingroupS->soft_inst = (SOFTBOXPTR *) Ysafe_malloc( sizeof(SOFTBOXPTR) );
} else {
    /* first find pingroup in softpin array */
    sarray = ptrS->softpins ;
    howmany = (INT) sarray[HOWMANY] ;
    pingroupS = NIL(PINBOXPTR) ;
    for( i = 1; i <= howmany; i++ ){
	if( strcmp( pingroup, sarray[i]->pinname ) == STRINGEQ ){
	    pingroupS = sarray[i] ;
	    break ;
	}
    }
    if( pingroupS ){
	pingroupS->soft_inst = (SOFTBOXPTR *) Ysafe_realloc( 
	    pingroupS->soft_inst,instAllocS * sizeof(SOFTBOXPTR) );
    } else {
	sprintf(YmsgG,"No match for pingroup:%s in primary instance:%s\n",
	    pingroup, cellarrayG[totalcellsG]->cname ) ;
	M( ERRMSG, "start_pin_group", YmsgG ) ;
	errorFlagS = TRUE ;
	return ;
    }
}
/*  now allocate space for this instance */
pingroupS->soft_inst[cellinstanceS] = pingroupS->softinfo =
    (SOFTBOXPTR) Ysafe_calloc( 1, sizeof(SOFTBOX)) ;

spin = pingroupS->softinfo ;
spin->permute = permute ;
spin->fixed = FALSE ;
spin->ordered = FALSE ;
spin->hierarchy = ROOT ;
spin->lowerbound = 0.0 ;
spin->upperbound = 1.0 ;
spin->children = (PINBOXPTR *) Ysafe_malloc( sizeof(PINBOXPTR) ) ;
spin->restrict1 = (INT *) Ysafe_calloc( 1, sizeof(INT) ) ;
spin->parent = NULL ;

} /* end start_pin_group */
/* ***************************************************************** */

/* add this pad to the current pad group */
add2pingroup( pinName, ordered ) 
char *pinName ;
BOOL ordered ;  /* ordered flag is true if padgroup is fixed */
{

INT i ;
INT howmany ;
PINBOXPTR pin ;
PINBOXPTR cpin ;
SOFTBOXPTR spin ;
SOFTBOXPTR pingroup_spin ;
INT curpingroup ;

ERRORABORT() ;

/* check pads for correctness */
for( pin = ptrS->pinptr ; pin ; pin = pin->nextpin ){
    spin = pin->softinfo ;
    if( strcmp(pinName, pin->pinname) == STRINGEQ ){
	
	if( spin->hierarchy == LEAF ){
	    sprintf(YmsgG,
		"pin %s was included in more than 1 pin group\n",
		pin->pinname);
	    M(ERRMSG,"add2pingroup",YmsgG ) ;
	    setErrorFlag() ;
	    return ;
	}
	/* check memory of pin array */
	pingroup_spin = pingroupS->softinfo ;
	if( ++numchildrenS >= childAllocS ){
	    childAllocS += EXPECTEDPINGRP ;
	    pingroup_spin->children = (PINBOXPTR *)
		Ysafe_realloc( pingroup_spin->children,
		childAllocS * sizeof(PINBOXPTR) ) ;
	}
	pingroup_spin->children[numchildrenS] = pin ;
	pingroup_spin->children[HOWMANY] = (PINBOXPTR) numchildrenS ;
	spin->parent = pingroupS ;
	/* now update any equivalent subpins to leaves */
	if( spin->hierarchy == NONE && spin->children ){
	    howmany = (INT) spin->children[HOWMANY] ;
	    for( i = 1; i <= howmany; i++ ){
		cpin = spin->children[i] ;
		if( cpin->type == SOFTEQUIVTYPE ){
		    cpin->softinfo->hierarchy = LEAF ;
		    cpin->softinfo->ordered = ordered ;
		}
	    }
	}
	spin->hierarchy = LEAF ;
	spin->ordered = ordered ;
	return;
    }
}

/* if no match above must be subroot */
curpingroup = ptrS->numpins + numchildrenS ;
for( i = ptrS->numpins ; i < curpingroup; i++ ){
    pin = ptrS->softpins[i] ;
    spin = pin->softinfo ;
    if (strcmp(pinName, pin->pinname) == STRINGEQ) {
	if (spin->hierarchy == SUBROOT) {
	    sprintf(YmsgG,
		"pin group %s was included in more than 1 pin group\n",
		pin->pinname);
	    M(ERRMSG,"add2pingroup",YmsgG ) ;
	    setErrorFlag() ;
	    return ;
	}
	pingroup_spin = pingroupS->softinfo ;
	if( ++numchildrenS >= childAllocS ){
	    childAllocS += EXPECTEDPINGRP ;
	    pingroup_spin->children = (PINBOXPTR *)
		Ysafe_realloc( pingroup_spin->children,
		childAllocS * sizeof(PINBOXPTR) ) ;
	}
	pingroup_spin->children[numchildrenS] = pin ;
	pingroup_spin->children[HOWMANY] = (PINBOXPTR) numchildrenS ;
	spin->parent = pingroupS ;
	spin->hierarchy = SUBROOT ;
	spin->ordered = ordered ;
	return ;
    }
}

sprintf(YmsgG,"cannot find pin <%s> for pin_group <%s>\n",
    pinName,ptrS->softpins[curpingroup]->pinname );
M(ERRMSG,"add2pingroup",YmsgG ) ;
return ;

} /* end add2pingroup */
/* ***************************************************************** */

addSideSpace( lower, upper )
DOUBLE lower ;
DOUBLE upper ;
{
    ERRORABORT() ;

    pptrS->fixed = TRUE ;
    if( lower > 1.0 || upper > 1.0 ){
	sprintf(YmsgG,
	    "side space must be less or equal to 1.0 for pad: %s\n",ptrS->cname ) ;
	M(ERRMSG,"addSideSpace",YmsgG ) ;
	setErrorFlag() ;
    }
    if( lower < 0.0 || upper < 0.0 ){
	sprintf(YmsgG,
	    "side space must be greater or equal to 0.0 for pad: %s\n",ptrS->cname ) ;
	M(ERRMSG,"addSideSpace",YmsgG ) ;
	setErrorFlag() ;
    }
    if( lower > upper ){
	sprintf(YmsgG,
	    "side space upper bound must be greater or equal to lower bound for pad: %s\n",ptrS->cname ) ;
	M(ERRMSG,"addSideSpace",YmsgG ) ;
	setErrorFlag() ;
    }
    pptrS->lowerbound = lower ;
    pptrS->upperbound = upper ;
} /* end addSideSpace */
/* ***************************************************************** */

addPadSide( side  )
char *side ;
{

    INT numsides ;         /* length of side restriction string */
    INT i ;                /* counter */

    ERRORABORT() ;

    pptrS->valid_side[ALL] = FALSE ;
    numsides = strlen( side ) ;
    for( i = 0 ; i < numsides; i++ ){
	switch( side[i] ){
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
		    ptrS->cname );
	    M(ERRMSG,"addPadSide",YmsgG ) ;
		setErrorFlag() ;
	} /* end switch */
    } 

} /* end addPadSide */
/* ***************************************************************** */

/* set whether a pad group can be permuted */
setPermutation( permuteFlag ) 
{
ERRORABORT() ;
pptrS->permute = permuteFlag ;
} /* end setPermutation */
/* ***************************************************************** */

/* add this pad to the current pad group */
add2padgroup( padName, ordered ) 
char *padName ;
BOOL ordered ;  /* ordered flag is true if pad is ordered in padgroup */
{
INT i, endofpads, endofgroups, endofcells ;

ERRORABORT() ;
endofpads = numcellsG + numpadsG ;
endofcells = numcellsG + numsupercellsG ;
/* check pads for correctness */
for (i = numcellsG + 1; i <= endofpads; ++i) {
    if (strcmp(padName, cellarrayG[i]->cname) == STRINGEQ) {
	if (cellarrayG[i]->padptr->hierarchy == LEAF) {
	    sprintf(YmsgG,
		"pad %s was included in more than 1 pad group\n",
		cellarrayG[i]->cname);
	    M(ERRMSG,"add2padgroup",YmsgG ) ;
	    setErrorFlag() ;
	    return ;
	}
	/* check memory of pin array */
	if( ++numchildrenS >= childAllocS ){
	    childAllocS += EXPECTEDNUMPADS ;
	    pptrS->children = (INT *)
		Ysafe_realloc( pptrS->children,
		childAllocS * sizeof(INT) ) ;
	}
	pptrS->children[numchildrenS]  = i - endofcells ;
	cellarrayG[i]->padptr->hierarchy = LEAF ;
	cellarrayG[i]->padptr->ordered = ordered ;
	/* total pins of the leaves */
        ptrS->numpins += cellarrayG[i]->numpins;
	return;
    }
}

/* if no match above must be subroot */
endofgroups = numcellsG + numpadsG + numpadgroupsG ;
for (i = endofpads; i <= endofgroups; ++i) {
    if (strcmp(padName, cellarrayG[i]->cname) == STRINGEQ) {
	if (cellarrayG[i]->padptr->hierarchy == SUBROOT) {
	    sprintf(YmsgG,"pad group %s was included in more than 1 pad group\n",
		cellarrayG[i]->cname);
	    M(ERRMSG,"add2padgroup",YmsgG ) ;
	    setErrorFlag() ;
	    return ;
	}
	/* check memory of pin array */
	if( ++numchildrenS >= childAllocS ){
	    childAllocS += EXPECTEDNUMPADS ;
	    pptrS->children = (INT *)
		Ysafe_realloc( pptrS->children,
		childAllocS * sizeof(INT) ) ;
	}
	pptrS->children[numchildrenS]  = i - endofcells ;
	cellarrayG[i]->padptr->hierarchy = SUBROOT ;
	cellarrayG[i]->padptr->ordered = ordered ;
	/* total pins of the leaves */
	ptrS->numpins += cellarrayG[i]->numpins;
	return ;
    }
}

sprintf(YmsgG,"cannot find pad <%s> for pad_group <%s>\n",
    padName,ptrS->cname);
M(ERRMSG,"add2padgroup",YmsgG ) ;
return ;

} /* end add2PadGroup */
/* ***************************************************************** */
add_cell_to_group( cellName ) 
char *cellName ;
{
GLISTPTR tempCell ;
INT cell, *data ;
CELLBOXPTR cptr ;

if(!(data = (INT *) Yhash_search( cellTableS, cellName, NULL, FIND ))){
    sprintf( YmsgG, "Couldn't find cellname:%s for group\n",cellName );
    M(ERRMSG,"add_cell_to_group",YmsgG ) ;
    errorFlagS = TRUE ;
    return ;
} 
cell = *data ;

if( tempCell = curGroupS->cells ){
    curGroupS->cells = (GLISTPTR) Ysafe_malloc( sizeof(GLISTBOX) ) ;
    curGroupS->cells->next = tempCell ;
} else { /* start list */
    curGroupS->cells = (GLISTPTR) Ysafe_malloc( sizeof(GLISTBOX) ) ;
    curGroupS->cells->next = NULL ;
}

cptr = cellarrayG[cell] ;
if( cptr->group_nested ){
    /* group_nest field should be NULL if cell is not nested */
    sprintf( YmsgG, "cell %d:%s appears in more than one group\n",
	cell, cptr->cname ) ;
    M(ERRMSG,"add_cell_to_group",YmsgG ) ;
    errorFlagS = TRUE ;
} else {
    cptr->group = curGroupS ;
    /* save cell in generic list */
    curGroupS->cells->p.cell = cell ;
    cptr->group_nested = TRUE ;
}
/* set exchange class so cells can't leave group thru pairwise swap */
cptr->class = unique_classG ;

} /* end add_cell_to_group */

/* add a cell to the instance array of the defining cell */
add_instance( instName )
char *instName ;
{
    if( cellinstanceS++ == 0 ){
	/* our first instance we now have two instances */
	instS = ptrS->instptr = (INSTBOXPTR) Ysafe_malloc( sizeof(INSTBOX) ) ;
	ptrS->cur_inst = 0 ;
	instS->numinstances = 1 ;
	instAllocS = 2 ;    /* default is just two instances */
	/* allocate arrays for instances */
	instS->tile_inst = (TILEBOXPTR *)  
	    Ysafe_calloc( instAllocS, sizeof(TILEBOXPTR) ) ;
	instS->bounBox = (BOUNBOXPTR **)  
	    Ysafe_calloc( instAllocS, sizeof(BOUNBOXPTR *) ) ;
	instS->numtile_inst = (INT *)      
	    Ysafe_calloc( instAllocS, sizeof(INT) ) ;
	instS->vert_inst = (VERTBOXPTR *)      
	    Ysafe_malloc( instAllocS*sizeof(VERTBOXPTR) ) ;
	instS->name_inst = (char **)      
	    Ysafe_malloc( instAllocS*sizeof(char *) ) ;
	/* store main instance in element 0 of arrays */
	instS->tile_inst[0] = ptrS->tiles ;
	instS->bounBox[0] = ptrS->bounBox ;
	instS->numtile_inst[0] = ptrS->numtiles ;
	instS->vert_inst[0] = ptrS->vertices ;
	instS->name_inst[0] = ptrS->cname ;
	instS->name_inst[1] = instName ;

	/* now for softcells */
	/* allocate arrays for instances */
	instS->numsides = (INT *) 
	    Ysafe_calloc( instAllocS, sizeof(INT) ) ;
	/* store main instance in element 0 of arrays */
	instS->numsides[0] = ptrS->numsides ;
    } else { /* realloc space for instances */
	/* instances are rare and we are only dealing with pointers so */
	/* always realloc, leave code general enough to change */
	instAllocS += EXPECTEDINSTANCES ;
	instS->tile_inst = (TILEBOXPTR *)  
	    Ysafe_realloc(instS->tile_inst,instAllocS*sizeof(TILEBOXPTR));
	instS->vert_inst = (VERTBOXPTR *)  
	    Ysafe_realloc(instS->vert_inst,instAllocS*sizeof(VERTBOXPTR));
	instS->numtile_inst = (INT *)  
	    Ysafe_realloc( instS->numtile_inst, instAllocS*sizeof(INT) ) ;
	instS->bounBox = (BOUNBOXPTR **)  
	    Ysafe_realloc(instS->bounBox,instAllocS*sizeof(BOUNBOXPTR *));
	instS->name_inst = (char **)      
	    Ysafe_realloc(instS->name_inst,instAllocS*sizeof(char *) ) ;
	instS->numsides = (INT *)  
	    Ysafe_realloc( instS->numsides,instAllocS*sizeof(INT));
    }
    minxS = INT_MAX ;
    minyS = INT_MAX ;
    maxxS = INT_MIN ;
    maxyS = INT_MIN ;
    ptrS->numtiles = 0 ;
    cornerCountS = 0 ;
    instS->numinstances++ ;
    numinstancesG++ ;

} /* end add_cell_instance */

INT get_tile_count()
{
    return( tileptAllocS / 2 ) ;
} /* end get_tile_count() */

add_analog( numcorners )
INT numcorners ;
{
    ERRORABORT() ;
    /* allocate space for array of instances */
    if( cellinstanceS == 0 ){
	pinS->ainst = (ANALOGPTR *) Ysafe_malloc( sizeof(ANALOGPTR) );
    } else {
	pinS->ainst = (ANALOGPTR *) Ysafe_realloc( pinS->ainst,
	    instAllocS * sizeof(ANALOGPTR) );
    }
    analogS = pinS->analog = (ANALOGPTR) Ysafe_malloc( sizeof(ANALOGBOX) ) ;
    pinS->ainst[cellinstanceS] = analogS ;

    /* now fill in data */
    if( numcorners >= 4 ){
	analogS->x_contour = (INT *) Ysafe_malloc( numcorners * sizeof(INT)) ;
	analogS->y_contour = (INT *) Ysafe_malloc( numcorners * sizeof(INT)) ;
	analogS->num_corners = numcorners ;
	cornerCountS = 0 ;
    } else {
	analogS->x_contour = NIL(INT) ;
	analogS->y_contour = NIL(INT) ;
	analogS->num_corners = 0 ;
    }
    analogS->current = INIT_CURRENT ;
    analogS->power =   (FLOAT) 0.0 ;
    analogS->no_layer_change = FALSE ;

} /* end add_analog */

add_pin_contour( x, y )
INT x, y ;
{
    if( cornerCountS >= analogS->num_corners ){
	sprintf( YmsgG, "Incorrect number of vertices for pin:%s\n",
	    curPinNameS ) ;
	M(ERRMSG,"add_pin_contour", YmsgG ) ;
	setErrorFlag() ;
	cornerCountS = 0 ; /* reset to avoid numerous error msgs */
    }
    ERRORABORT() ;
    analogS->x_contour[cornerCountS] = x ;
    analogS->y_contour[cornerCountS++] = y ;
} /* end start_pin_contour */


add_current( current )
FLOAT current ;
{
    if(!(analogS)){
	add_analog( 0 ) ;
    }
    analogS->current = current ;
} /* end add_current */

add_power( power )
FLOAT power ;
{
    if(!(analogS)){
	add_analog( 0 ) ;
    }
    analogS->power = power ;
} /* end add_power */

no_layer_change()
{
    if(!(analogS)){
	add_analog( 0 ) ;
    }
    analogS->no_layer_change = TRUE ;
} /* end no_cross_under */

process_pin()
{
    INT i ;                      /* point counter */
    INT side ;                   /* current side for pin */
    INT ptx, pty ;               /* current point of interest */
    INT xpos, ypos ;             /* center of pin */
    INT minx, miny, maxx, maxy ; /* bounding box of pin contour */
    char *buster_msg ;           /* message string to used by buster */
    INT xx1, yy1, xx2, yy2 ;     /* temp points */

    ERRORABORT() ;

    if(!(analogS) || analogS->num_corners < 4 ){
	return ; /* no work to do */
    }

    /* now check the points to make sure they are good */
    sprintf( YmsgG, " (cell:%s pin:%s) ", curCellNameS, pinS->pinname ) ;
    buster_msg = Ystrclone( YmsgG ) ;
    Ybuster_check_rect_init( buster_msg ) ;
    minx = INT_MAX ;
    miny = INT_MAX ;
    maxx = INT_MIN ;
    maxy = INT_MIN ;
    for( i = 0 ; i < analogS->num_corners ; i++ ) {
	xx1 = analogS->x_contour[i] ;
	yy1 = analogS->y_contour[i] ;
	if( i == analogS->num_corners-1 ) {
	    xx2 = analogS->x_contour[0] ;
	    yy2 = analogS->y_contour[0] ;
	} else {
	    xx2 = analogS->x_contour[ i + 1 ] ;
	    yy2 = analogS->y_contour[ i + 1 ] ;
	}
	if( Ybuster_check_rect( xx1, yy1, xx2, yy2 ) ){
	    analog_errorS = TRUE ;
	}

	/* find bounding box of pins */
	minx = MIN( minx, xx1 ) ;
	maxx = MAX( maxx, xx1 ) ;
	miny = MIN( miny, yy1 ) ;
	maxy = MAX( maxy, yy1 ) ;
    }
    Ysafe_free( buster_msg ) ;

    /* now set the center of the pin to this location */
    xpos = (minx + maxx) / 2 ;
    ypos = (miny + maxy) / 2 ;

    /* now subtract off the cell center */
    for( i = 0 ; i < analogS->num_corners ; i++ ) {
	analogS->x_contour[i] -= xcenterS ;
	analogS->y_contour[i] -= ycenterS ;
    }
    set_pin_pos( xpos, ypos ) ;

} /* end process_analog_pin */
