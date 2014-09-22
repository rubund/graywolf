/* ----------------------------------------------------------------- 
FILE:	    dens.c                                       
DESCRIPTION:Form routing tiles from global routing density info.
CONTENTS   :
	    density( crossref )
		INT *crossref ; 
	    add_dens( cell, edge, loend, hiend, extraSpace )
		INT cell, edge, loend, hiend, extraSpace ;
	    static make_tile( cell, x1, y1, x2, y2 )
		INT cell, x1, y1, x2, y2 ;
	    read_par_file()
DATE:	    Jan 29, 1988 
REVISIONS:  Feb  8, 1990 - Initial coding of routing tiles.
	    Mar  1, 1990 - Now a separate program.
	    Oct 14, 1990 - added missing initialization.
	    Tue Dec 18 01:31:14 EST 1990 - Updated for new global
		routing format which now returns space rather than
		tracks.
	    Tue Jan 15 22:30:35 PST 1991 - now return the side
		that tile is found in the .mtle file.
	    Fri Jan 25 16:23:49 PST 1991 - added closegraphics calls
		so that code ends correctly.
	    Sat Feb 23 00:31:03 EST 1991 - now handle wildcarding
		properly.
	    Thu Apr 18 00:56:33 EDT 1991 - added new design rule
		routines.
	    Wed May  1 19:09:26 EDT 1991 - major rewrite of this
		code.  Now added routing tiles along entire length
		of cell including switchboxes rather than just the
		critical regions.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) dens.c version 1.14 3/2/92" ;
#endif

/* #undef DEBUG */

#define DENS_DEFS

#include <string.h>
#include <yalecad/base.h>
#include <yalecad/debug.h>
#include <yalecad/file.h>
#include <yalecad/message.h>
#include <yalecad/string.h>
#include <geo.h>
#include <dens.h>

#define  DEFAULTTRACKS        "default.tracks.per.channel"
#define VEDGE         FALSE
#define HEDGE         TRUE
#define UPPERLEFTC    1
#define UPPERRITEC    2
#define LOWERRITEC    3
#define LOWERLEFTC    4
#define INVALIDC      0

static FILE *tilefileS ;   /* the output file giving routing tiles */
static BOOL switchboxS = FALSE ; /* denote switchboxes in output file.*/



static process_channels();
static make_tile();
static process_switchboxes(); 
static readpar();

INT compare_tiles( tile1, tile2 )
TILEPTR tile1, tile2 ;
{
    INT side ;   /* tile side */

    /* explicitly enumerate cases to avoid wraparound */
    /* sort on edge */
    if( tile1->edge == tile2->edge ){
	/* sides will be the same */
	side = tile1->side ;
	/* in case of tie sort on x or y location */
	if( side == TILET || side == TILEB ){
	    if( tile1->l == tile2->l ){
		return( 0 ) ;
	    } else if( tile1->l < tile2->l ){
		return( -1 ) ;
	    } else {
		return( 1 ) ;
	    }
	} else { /* TILEL | TILER */
	    if( tile1->b == tile2->b ){
		return( 0 ) ;
	    } else if( tile1->b < tile2->b ){
		return( -1 ) ;
	    } else {
		return( 1 ) ;
	    }
	}
    } else if( tile1->edge < tile2->edge ){
	return( -1 ) ;
    } else {
	return( 1 ) ;
    }
} /* end compare_tiles */

density( crossref )
INT *crossref ;            /* crossref is the new (user) number */
{

    INT i ;                   /* counter */
    INT c ;                   /* counter */
    INT *get_reverse_ref() ;  /* get original node number in graph */
    INT *reverse_ref ;        /* node translation table */
    INT eIndex ;              /* current edge */
    INT space ;               /* current edge */
    INT count, length ;       /* temp variables */
    INT cell1 , cell2 ;       /* two cells of the channel */
    INT edge1 , edge2 ;       /* two cell edges of the channel */
    INT extraSpace1 ;         /* how much area to add to cell 1 */
    INT extraSpace2 ;         /* how much area to add to cell 2 */
    INT node1, node2 ;        /* the two nodes of channel */
    INT loend, hiend ;        /* the location of the channel */
    INT first_chan ;          /* first channel in an empty room */
    INT last_chan ;           /* last channel in an empty room */
    INT half_chan ;           /* halfway channel in an empty room */
    INT small_chan ;          /* add extra space to cell with low # */
    INT small_cell ;          /* smaller number cell in channel */
    INT valid_first_chan ;    /* this channel touches a cell */
    INT valid_last_chan ;     /* this channel touches a cell */
    INT lo_edge ;             /* lo end of channel */
    INT hi_edge ;             /* hi end of channel */
    INT line ;                /* current parsing line */
    BOOL abort ;              /* abort if error occurs */
    INT numtokens ;           /* number of tokens on the line */
    char filename[LRECL] ;    /* filename */
    char buffer[LRECL] ;      /* temporary string workspace */
    char *bufferptr ;         /* pointer to beginning of workspace */
    char **tokens ;           /* tokenized buffer */
    FILE *fp ;                /* current file pointer */
    WCPTR ptr ;               /* search thru the x and y graphs */
    EBOXPTR chanEdgePtr ;     /* look at the edges */
    EBOXPTR eptr ;            /* look at the edges */

    /* ---------------------------------------------------------------
        Find the original node numbers.
        Global routing uses crossref numbers. Need to translate back.
    --------------------------------------------------------------- */
    reverse_ref = get_reverse_ref() ;

    /* *********** READ CHANNEL DENSITIES FROM GLOBAL ROUTER ********* */
    sprintf( filename, "%s.dens", cktNameG ) ;
    fp = TWOPEN( filename, "r", ABORT ) ;

    /* parse file */
    line = 0 ;
    abort = FALSE ;
    D( "gengraph/density", fprintf(stderr,"CHANNEL DENSITIES:\n") ) ;    
    while( bufferptr=fgets(buffer,LRECL,fp )){
	/* parse file */
	line ++ ; /* increment line number */
	tokens = Ystrparser( bufferptr, " \t\n", &numtokens );

	if( numtokens == 0 ){
	    /* skip over empty lines */
	    continue ;
	} else if( numtokens == 5 ){
	    node1 = atoi(tokens[1]) ;
	    node1 = reverse_ref[ node1 ] ; /* get reverse crossref */
	    node2 = atoi(tokens[2]) ;
	    node2 = reverse_ref[ node2 ] ; /* get reverse crossref */
	    eIndex = eIndexArrayG[ node1 ][ node2 ] ;
	    eArrayG[eIndex].density = atoi( tokens[4] ) ;

	    D( "gengraph/density",
		fprintf(stderr,
		"channel:%4d  %4d - %4d density:%d\n",
		eIndex, crossref[ eArrayG[eIndex].index1] , 
		crossref[ eArrayG[eIndex].index2 ], 
		eArrayG[eIndex].density ) ) ;
	} else {
	    sprintf( YmsgG, "Syntax error on line:%d\n", line ) ;
	    M(ERRMSG, "density", YmsgG ) ;
	    abort = TRUE ;
	}
    }
    if( abort ){
	closegraphics() ;
	YexitPgm( PGMFAIL ) ;
    }
    TWCLOSE( fp ) ;
    /* ************************ end read ***************************** */

    /* now process information */
    sprintf( filename, "%s.mtle", cktNameG ) ;
    tilefileS = TWOPEN( filename, "w", ABORT ) ;

    for( i = 1 ; i <= numXnodesG ; i++ ) {
	for( ptr = xNodeArrayG[i] ; ptr ; ptr = ptr->next ){
	    if( ptr->fixedWidth == 1 ) {
		continue ;
	    }
	    process_channels( ptr, track_pitchXG ) ;
	}
    }


    for( i = 1 ; i <= numYnodesG ; i++ ) {
	for( ptr = yNodeArrayG[i] ; ptr ; ptr = ptr->next ){
	    if( ptr->fixedWidth == 1 ) {
		continue ;
	    }
	    process_channels( ptr, track_pitchYG ) ;
	}
    }


    for( eIndex = 1 ; eIndex <= eNumG ; eIndex++ ) {
	eptr = &(eArrayG[ eIndex ] ) ;
        edge1 = eptr->edge1 ;	
        edge2 = eptr->edge2 ;	
	cell1 = edgeListG[edge1].cell ;
	cell2 = edgeListG[edge2].cell ;
	if( eIndex <= edgeTransitionG ){
	    /* vertical edge */
	    lo_edge = rectArrayG[eptr->index1].yc ;
	    hi_edge = rectArrayG[eptr->index2].yc ;
	    ASSERT( lo_edge <= hi_edge,"density","lo_edge > hi_edge\n");
	} else {
	    /* horizontal edge */
	    lo_edge = rectArrayG[eptr->index1].xc ;
	    hi_edge = rectArrayG[eptr->index2].xc ;
	    ASSERT( lo_edge <= hi_edge,"density","lo_edge > hi_edge\n");
	}
	/* calculate the extra space need for routing area */
	length = eptr->density ;
	if( cell1 > 0 && cell2 > 0 ){
	    /* only case to divide is when we have only two cells on a channel */
	    extraSpace2 = length / 2 ;
	    /* now grid down */
	    Ygridx_down( &extraSpace2 ) ;
	    extraSpace1 = length - extraSpace2 ;
	} else if( cell1 > 0 ){
	    extraSpace1 = length ;
	} else if( cell2 > 0 ){
	    extraSpace2 = length ;
	}
	/* add density to cell for horizontal case */
	add_dens( cell1, edge1, lo_edge, hi_edge, extraSpace1 ) ;
	add_dens( cell2, edge2, lo_edge, hi_edge, extraSpace2 ) ;
    }

    process_switchboxes() ;

    TWCLOSE( tilefileS ) ;
    return ;
}

static process_channels( ptr, pitch )
WCPTR ptr ;               /* search thru the x and y graphs */
INT pitch ;
{

    INT i ;                   /* counter */
    INT c ;                   /* counter */
    INT eIndex ;              /* current edge */
    INT space ;               /* current edge */
    INT count, length ;       /* temp variables */
    INT cell1 , cell2 ;       /* two cells of the channel */
    INT edge1 , edge2 ;       /* two cell edges of the channel */
    INT extraSpace1 ;         /* how much area to add to cell 1 */
    INT extraSpace2 ;         /* how much area to add to cell 2 */
    INT node1, node2 ;        /* the two nodes of channel */
    INT loend, hiend ;        /* the location of the channel */
    INT first_chan ;          /* first channel in an empty room */
    INT last_chan ;           /* last channel in an empty room */
    INT half_chan ;           /* halfway channel in an empty room */
    INT small_chan ;          /* add extra space to cell with low # */
    INT small_cell ;          /* smaller number cell in channel */
    INT valid_first_chan ;    /* this channel touches a cell */
    INT valid_last_chan ;     /* this channel touches a cell */
    EBOXPTR eptr ;            /* look at the edges */

    ptr->length = 0 ;
    count = ptr->channels[0] ;
    first_chan = ptr->channels[1] ;
    last_chan  = ptr->channels[count] ;
    half_chan = count / 2 ;

    /* 
	Determine if we have a valid first channel.
	Make sure this channel borders a cell.
    */
    eptr = &(eArrayG[ first_chan ] ) ;
    edge1 = eptr->edge1 ;	
    edge2 = eptr->edge2 ;	
    cell1 = edgeListG[edge1].cell ;
    cell2 = edgeListG[edge2].cell ;
    if( cell1 > 0 || cell2 > 0 ){
	valid_first_chan = first_chan ;
    } else {
	valid_first_chan = 0 ;
    }
    /* 
	Determine if we have a valid last channel.
	Make sure this channel borders a cell.
    */
    eptr = &(eArrayG[ last_chan ] ) ;
    edge1 = eptr->edge1 ;	
    edge2 = eptr->edge2 ;	
    cell1 = edgeListG[edge1].cell ;
    cell2 = edgeListG[edge2].cell ;
    if( cell1 > 0 || cell2 > 0 ){
	valid_last_chan = last_chan ;
    } else {
	valid_last_chan = 0 ;
    }
    if( valid_first_chan && !valid_last_chan ){
	/* map everything to first channel */
	valid_last_chan = valid_first_chan ;
	small_chan = valid_first_chan ;
    } else if( !valid_first_chan && valid_last_chan ){
	/* map everything to last channel */
	valid_first_chan = valid_last_chan ;
	small_chan = valid_last_chan ;
    } else if( !valid_first_chan && !valid_last_chan ){
	M( ERRMSG, "density", "no valid channels\n" ) ;
	valid_first_chan = first_chan ;
	valid_last_chan = last_chan ;
	small_chan = first_chan ;
    }
	
    /* only case to divide is when we have only two cells on a channel */
    /* 
	half way is only meaningful for empty rooms.  It is
	the halfway point thru the empty room.  Move all 
	density of interior empty rooms to outside edges.
	If we have one channel, then
	    first_chan = last_chan = 1 ;
	    half_chan = 0 ;
	    and no rules fire.
	If we have two channels, then
	    first_chan = 1 ;
	    last_chan = 2 ;
	    half_chan = 1 ;
	    and again no rules fire.
	If we have three or more channels, then
	    first_chan = 1 ;
	    last_chan = 3 | count ;
	    half_chan = 1 | count / 2 ;
	When moving to a new edge make sure it has a cell
	    bordering with it.
    */
    for( c = 1 ; c <= count ; c++ ) {
	eIndex = ptr->channels[c] ;
	ptr->length += eArrayG[ eIndex ].density ;
	if( eArrayG[ eIndex ].density <= 0 ){
	    continue ;
	}
	if( c > 1 && c < half_chan ){
	    /* move density to first channel */
	    eArrayG[valid_first_chan].density += eArrayG[eIndex].density ;
	    eArrayG[eIndex].density = 0 ;
	    D( "density", 
		fprintf( stderr, "moving density in %d to %d...\n",
		    eIndex, valid_first_chan ) ;
	    ) ;
	} else if( c > half_chan && c < count ){
	    /* move density to last channel */
	    eArrayG[valid_last_chan].density += eArrayG[eIndex].density ;
	    eArrayG[eIndex].density = 0 ;
	    D( "gengraph/density", 
		fprintf( stderr, "moving density in %d to %d...\n",
		    eIndex, valid_last_chan ) ;
	    ) ;
	} else if( c == half_chan && c > 1 && c < count ){
	    /* move density to first channel */
	    eArrayG[valid_first_chan].density += eArrayG[eIndex].density ;
	    eArrayG[eIndex].density = 0 ;
	    D( "gengraph/density", 
		fprintf( stderr, "moving density in %d to %d...\n",
		    eIndex, valid_first_chan ) ;
	    ) ;
	}
    }

    /* 
	Always put the space for spacing to cell edge on
	the first edge. This will tend to minimize
	the jaggedness of the profiles.
    */
    if( ptr->length > 0 ) {
	/* plus one for spacing to cell edge */
	space = (defaultTracksG +1) * pitch ;
	ptr->length += space ;
	eArrayG[valid_first_chan].density += space ;
    } else if( defaultTracksG > 0 ) {
	space = defaultTracksG * pitch ;
	ptr->length = space ;
	eArrayG[valid_first_chan].density += space ;
    }
} /* end process_channels */

/* add density to the list of cell edges */
add_dens( cell, edge, loend, hiend, extraSpace )
INT cell, edge, loend, hiend, extraSpace ;
{
    EDGEBOXPTR edgeptr ;

    if( cell <= 0 || extraSpace <= 0 ) {
	return ; /* no work to do */
    }

    edgeptr = &(edgeListG[ edge ]) ;

    /* now modify hi and loend to be maximum length of edge */
    if( loend < edgeptr->start ){
	loend = edgeptr->start ;
    }
    if( edgeptr->end < hiend ){
	hiend = edgeptr->end ;
    }


    D( "gengraph/add_dens", fprintf( stderr, 
	"cell:%d edge:%d lowend:%d hiend:%d extraSpace:%d VnotH:%d\n",
	cell, edge, loend, hiend, extraSpace, edge%2 ) ) ;


    /* see if edge is horizontal or vertical */
    /* horizontal edges are multiples of 2 */
    if( edge % 2 == 0 ){

	if( edgeptr->UorR == 1 ){
	    /* edge to the right */
	    /* make tile on top */
	    make_tile( cell, edge, extraSpace,
		loend, edgeptr->loc, hiend, edgeptr->loc + extraSpace, TILET ) ;
	} else {
	    /* edge to the left */
	    /* make tile on bottom */
	    make_tile( cell, edge, extraSpace,
		loend, edgeptr->loc - extraSpace, hiend, edgeptr->loc, TILEB ) ;
	}
    } else {
	/* vertical cell edge */
	if( edgeptr->UorR == 1 ){
	    /* edge going up */
	    /* make tile to left */
	    make_tile( cell, edge, extraSpace,
		edgeptr->loc - extraSpace, loend, edgeptr->loc, hiend, TILEL ) ;
	} else {
	    /* edge going down
	    /* make tile to right */
	    make_tile( cell, edge, extraSpace,
		edgeptr->loc, loend, edgeptr->loc + extraSpace, hiend, TILER ) ;
	}
    }
} /* end add_dens */


/* make routing tiles */
static make_tile( cell, edge, extraSpace, x1, y1, x2, y2, side )
INT cell, edge, extraSpace, x1, y1, x2, y2, side ;
{

    INT xc, yc ;
    TILEPTR tile ;

    tile = YMALLOC( 1, TILEBOX ) ;
    tile->l = x1 ;
    tile->r = x2 ;
    tile->b = y1 ;
    tile->t = y2 ;
    tile->side = side ;
    tile->edge = edge ;
    tile->space = extraSpace ;
    tile->orig = TRUE ;
    Yrbtree_insert( cellarrayG[cell].tiles, tile ) ;

    xc = cellarrayG[cell].xc ;
    yc = cellarrayG[cell].yc ;
    x1 -= xc ;
    y1 -= yc ;
    x2 -= xc ;
    y2 -= yc ;
    fprintf( tilefileS, "cell:%d xc:%d yc:%d l:%d b:%d r:%d t:%d side:%d",
	cell, xc, yc, x1, y1, x2, y2, side ) ;
    if( switchboxS ){
	fprintf( tilefileS, " switchbox\n" ) ;
    } else {
	fprintf( tilefileS, "\n" ) ;
    }
} /* end make_tile */

static BOOL valid_state( curHnotV, curUorR, nextHnotV, nextUorR )
BOOL curHnotV, nextHnotV ;
INT curUorR, nextUorR ;
{
    /* there are only 4 valid states */
    if( curHnotV == VEDGE && curUorR == 1 ){
	if( nextHnotV == HEDGE && nextUorR == 1 ){
	    return( UPPERLEFTC ) ;
	}
    } else if( curHnotV == HEDGE && curUorR == 1 ){
	if( nextHnotV == VEDGE && nextUorR == -1 ){
	    return( UPPERRITEC ) ;
	}
    } else if( curHnotV == VEDGE && curUorR == -1 ){
	if( nextHnotV == HEDGE && nextUorR == -1 ){
	    return( LOWERRITEC ) ;
	}
    } else if( curHnotV == HEDGE && curUorR == -1 ){
	if( nextHnotV == VEDGE && nextUorR == 1 ){
	    return( LOWERLEFTC ) ;
	}
    }
    return( INVALIDC ) ;
} /* end valid state */

static process_switchboxes() 
{
    YTREEPTR tree ;
    TILEPTR tile ;
    TILEBOX lo, hi ;
    INT cell ;
    INT numcedges ;
    INT cedge ;
    INT next_edge ;
    INT state ;
    INT cspace ;
    INT nspace ;
    BOOL curHnotV ;
    BOOL nextHnotV ;

    switchboxS = TRUE ;
    numcedges = edgeCountG - 4 ; /* don't count bounding box */
    for( cedge = 1; cedge <= numcedges ; cedge++ ){
	cell = edgeListG[cedge].cell ;
	/* calculate next edge */
	if( edgeListG[cedge+1].cell == cell ){
	    next_edge = cedge + 1 ;
	} else {
	    /* this must be the closing edge */
	    for( next_edge = cedge ; next_edge > 1 ; next_edge-- ){
		if( edgeListG[next_edge - 1].cell != cell ){
		    break ;
		}
	    } 
	    ASSERT( edgeListG[next_edge].cell == cell,
		"process_switchboxes",
		"Problems finding ending edge\n" ) ;
	}
	/* at this point we have the current edge, cedge and
	    the next edge. Find HnotV and UorR for both edges.
	    HnotV = 1 if edge if horizontal even. !(edge%2)
		  = 0 if edge is vertical odd.
	*/
	curHnotV = !(cedge & 0x00000001) ;
	nextHnotV = !(next_edge & 0x00000001) ;
	state = valid_state( curHnotV, edgeListG[cedge].UorR, 
	    nextHnotV, edgeListG[next_edge].UorR ) ; 
	if( state == INVALIDC ){
	    /* go on to next corner */
	    continue ;
	}

	/*
	    Since we have a valid edge, find first routing tile
	    on next_edge and get the last tile on the current edge
	    referenced as going clockwise around the cell.
	*/

	tree = cellarrayG[cell].tiles ;
	if( state == UPPERLEFTC ){
	    /* 
		LOOK at the current edge get last tile 
	    */
	    lo.edge = hi.edge = cedge ;
	    lo.side = hi.side = TILEL ;
	    lo.b = edgeListG[cedge].start - 1 ;
	    hi.b = edgeListG[cedge].end + 1 ;
	    cspace = 0 ;
	    for( tile = (TILEPTR) Yrbtree_interval( tree,&lo,&hi,TRUE ) ;
		tile ;
		tile = (TILEPTR) Yrbtree_interval(tree,&lo,&hi,FALSE)){
		cspace = tile->space ;
	    }
	    if( cspace <= 0 ){
		/* there is no routing tiles on this side */
		continue ;
	    }
	    /* 
		LOOK at the next edge get first tile 
	    */
	    lo.edge = hi.edge = next_edge ;
	    lo.side = hi.side = TILET ;
	    lo.l = edgeListG[next_edge].start - 1 ;
	    hi.l = edgeListG[next_edge].end + 1 ;
	    if( tile = (TILEPTR) Yrbtree_interval( tree,&lo,&hi,TRUE ) ){
		nspace = tile->space ;
	    } else {
		/* no routing on this edge */
		continue ;
	    }
	    /* make a tile for Upper Left Corner */
	    make_tile( cell, cedge, 0, 
	        edgeListG[cedge].loc - cspace, edgeListG[next_edge].loc,
		edgeListG[cedge].loc, edgeListG[next_edge].loc+nspace,
		TILEL ) ;
	} else if( state == UPPERRITEC ){
	    /* 
		LOOK at the current edge get last tile 
	    */
	    lo.edge = hi.edge = cedge ;
	    lo.side = hi.side = TILET ;
	    lo.l = edgeListG[cedge].start - 1 ;
	    hi.l = edgeListG[cedge].end + 1 ;
	    cspace = 0 ;
	    for( tile = (TILEPTR) Yrbtree_interval( tree,&lo,&hi,TRUE ) ;
		tile ;
		tile = (TILEPTR) Yrbtree_interval(tree,&lo,&hi,FALSE)){
		cspace = tile->space ;
	    }
	    if( cspace <= 0 ){
		/* there is no routing tiles on this side */
		continue ;
	    }
	    /* 
		LOOK at the next edge get first tile actually
		last one in tree since sorted from bottom to top.
	    */
	    lo.edge = hi.edge = next_edge ;
	    lo.side = hi.side = TILER ;
	    lo.b = edgeListG[next_edge].start - 1 ;
	    hi.b = edgeListG[next_edge].end + 1 ;
	    nspace = 0 ;
	    for( tile = (TILEPTR) Yrbtree_interval( tree,&lo,&hi,TRUE ) ;
		tile ;
		tile = (TILEPTR) Yrbtree_interval(tree,&lo,&hi,FALSE)){
		nspace = tile->space ;
	    }
	    if( nspace <= 0 ){
		/* there is no routing tiles on this side */
		continue ;
	    }
	    /* make a tile for upper right corner */
	    make_tile( cell, cedge, 0, 
	        edgeListG[next_edge].loc, edgeListG[cedge].loc,
		edgeListG[next_edge].loc+nspace,edgeListG[cedge].loc+cspace,
		TILET ) ;

	} else if( state == LOWERRITEC ){
	    /* 
		LOOK at the current edge get last tile. Actually first due
		due to tile sorting.
	    */
	    lo.edge = hi.edge = cedge ;
	    lo.side = hi.side = TILER ;
	    lo.b = edgeListG[cedge].start - 1 ;
	    hi.b = edgeListG[cedge].end + 1 ;
	    if( tile = (TILEPTR) Yrbtree_interval( tree,&lo,&hi,TRUE ) ){
		cspace = tile->space ;
	    } else {
		/* no routing on this edge */
		continue ;
	    }
	    /* 
		LOOK at the next edge get first tile actually
		last one in tree since sorted from left to right.
	    */
	    lo.edge = hi.edge = next_edge ;
	    lo.side = hi.side = TILEB ;
	    lo.l = edgeListG[next_edge].start - 1 ;
	    hi.l = edgeListG[next_edge].end + 1 ;
	    nspace = 0 ;
	    for( tile = (TILEPTR) Yrbtree_interval( tree,&lo,&hi,TRUE ) ;
		tile ;
		tile = (TILEPTR) Yrbtree_interval(tree,&lo,&hi,FALSE)){
		nspace = tile->space ;
	    }
	    if( nspace <= 0 ){
		/* there is no routing tiles on this side */
		continue ;
	    }
	    /* make a tile for lower right corner */
	    make_tile( cell, cedge, 0, 
	        edgeListG[cedge].loc, edgeListG[next_edge].loc - nspace,
		edgeListG[cedge].loc+cspace,edgeListG[next_edge].loc,
		TILER ) ;

	} else if( state == LOWERLEFTC ){
	    /* 
		LOOK at the current edge get last tile. Actually first
		due to sorting.
	    */
	    lo.edge = hi.edge = cedge ;
	    lo.side = hi.side = TILEB ;
	    lo.l = edgeListG[cedge].start - 1 ;
	    hi.l = edgeListG[cedge].end + 1 ;
	    if( tile = (TILEPTR) Yrbtree_interval( tree,&lo,&hi,TRUE ) ){
		cspace = tile->space ;
	    } else {
		/* no routing on this edge */
		continue ;
	    }
	    /* 
		LOOK at the next edge get first tile.
	    */
	    lo.edge = hi.edge = next_edge ;
	    lo.side = hi.side = TILEL ;
	    lo.b = edgeListG[next_edge].start - 1 ;
	    hi.b = edgeListG[next_edge].end + 1 ;
	    if( tile = (TILEPTR) Yrbtree_interval( tree,&lo,&hi,TRUE ) ){
		nspace = tile->space ;
	    } else {
		/* no routing on this edge */
		continue ;
	    }
	    /* make a tile for lower left corner */
	    make_tile( cell, cedge, 0, 
	        edgeListG[next_edge].loc-nspace, edgeListG[cedge].loc - cspace,
		edgeListG[next_edge].loc,edgeListG[cedge].loc,
		TILEB ) ;

	} /* end last case */
    } /* traverse the cell edges */

} /* end process_switchboxes */


#include <yalecad/yreadpar.h>

read_par_file()
{
    char design[LRECL] ;
    INT length ;
    INT i ;                   /* counter */
    INT pitch ;               /* track pitch for given layer */
    INT numv_layers ;         /* no. of vertical layers */
    INT numh_layers ;         /* no. of horizontal layers */
    INT num_layers ;          /* total no. of horizontal layers */
    char *layer ;             /* current layer */

    defaultTracksG = 0 ;
    track_pitchXG = 0 ;
    track_pitchYG = 0 ;
    /* ******************** READ PARAMETER FILE ********************** */
    length = strlen( cktNameG ) ;
    /* look for _io as the last 3 characters special case */
    /* in this case we look for main design name */
    strcpy( design, cktNameG ) ; 
    if( strcmp( cktNameG+length-3, "_io" ) == STRINGEQ ){
	design[length-3] = EOS ;
    }
    readpar( design, TWMC ) ;
    readpar( design, USER ) ;

    /* now find track pitch */
    numv_layers = 0 ;
    numh_layers = 0 ;
    num_layers = Yreadpar_numlayers() ;
    for( i = 1; i <= num_layers; i++ ){
	layer = Yreadpar_id2layer( i ) ;
	pitch = Yreadpar_pitch( layer ) ;
	if( Yreadpar_layer_HnotV( layer ) ){
	    track_pitchYG += pitch ;
	    numh_layers++ ;
	} else {
	    track_pitchXG += pitch ;
	    numv_layers++ ;
	}
    }
    /* get the average pitch of all the layers */
    if( numv_layers > 0 ){
	track_pitchXG /= numv_layers ;
	if( track_pitchXG == 0 ){
	    track_pitchXG == 1 ;
	}
    }
    if( numh_layers > 0 ){
	track_pitchYG /= numh_layers ;
	if( track_pitchYG == 0 ){
	    track_pitchYG == 1 ;
	}
    }
    if( track_pitchXG == 0 || track_pitchYG == 0 ){
	M(ERRMSG, "density", "\nTrack pitch may not be zero\n" ) ;
	closegraphics() ;
	YexitPgm( PGMFAIL ) ;
    }
    Ygrid_setx( track_pitchXG, 0 ) ;
    Ygrid_sety( track_pitchYG, 0 ) ;
    
} /* ************************ end read ***************************** */
static readpar( design, parfile )
char *design ;
INT parfile ;
{
    char *bufferptr ;
    char **tokens ;   
    INT  line ;
    INT  numtokens ;
    BOOL onNotOff ;
    BOOL wildcard ;

    Yreadpar_init( design, parfile, TWMC, FALSE ) ;
    while( tokens = Yreadpar_next( &bufferptr, &line, &numtokens, 
	&onNotOff, &wildcard )){
	if( numtokens != 2 ){
	    continue ;
	} else if( strcmp( tokens[0], DEFAULTTRACKS ) == STRINGEQ ){
	    defaultTracksG = atoi(tokens[1]) ;
	}
    }
} /* end readpar */
