/* ----------------------------------------------------------------- 
FILE:	    readgeo.c                                       
DESCRIPTION:Read the information from the placement program.
CONTENTS:   readgeo( fp )
		FILE *fp ;
DATE:	    Jan 29, 1988 
REVISIONS:  Feb  8, 1990 - Initial coding of routing tiles.
	    Mon Aug 12 16:06:47 CDT 1991 - modified for new
		Yrbtree_init.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) readgeo.c version 1.3 8/12/91" ;
#endif


#include <geo.h>
#include <yalecad/debug.h>

readgeo( fp )
FILE *fp ;
{

#define EXPECTNUMEDGES 400          /*** average problem size ***/
#define EXPECTNUMCELLS  50

char input[32] ;
int vertices , v , vx , vy , ux , uy , ux0 , uy0 ;
int rippleCount , cell , i ;
int cell_minx, cell_miny ;
int cell_maxx, cell_maxy ;
int cellAlloc ;
POINTPTR last_pt ;
INT compare_tiles() ;


bbrG = INT_MIN ;
bbtG = INT_MIN ;
bblG = INT_MAX ;
bbbG = INT_MAX ;
edgeCountG = 0 ;
rippleCount = 0 ;
cell = 0 ;
hFixedEdgeRootG = (TNODEPTR) NULL ;
vFixedEdgeRootG = (TNODEPTR) NULL ;
hEdgeRootG = (TNODEPTR) NULL ;
vEdgeRootG = (TNODEPTR) NULL ;
edgeListG = (EDGEBOXPTR) Ysafe_malloc( 
	    (EXPECTNUMEDGES+1)*sizeof(EDGEBOX) ) ;

cellAlloc = EXPECTNUMCELLS ;
cellarrayG = (CELLBOXPTR) Ysafe_malloc( cellAlloc * sizeof(CELLBOX) ) ;

while( fscanf( fp , " %s " , input ) == 1 ) {
    if( strcmp( input , "cell") == 0 ) {
	if( ++cell >= cellAlloc ){
	    cellAlloc += EXPECTNUMCELLS ;
	    cellarrayG = (CELLBOXPTR) 
		Ysafe_realloc( cellarrayG,cellAlloc*sizeof(CELLBOX) ) ;
	}
	last_pt = cellarrayG[cell].pts = NULL ;
	cell_minx = INT_MAX ;
	cell_miny = INT_MAX ;
	cell_maxx = INT_MIN ;
	cell_maxy = INT_MIN ;

	fscanf( fp , " %s " , input ) ; /* cell name */
	fscanf( fp , " %d " , &vertices ) ;
	fscanf( fp , " %s " , input ) ; /* keyword: vertices */
	for( v = 1 ; v <= vertices ; v++ ) {
	    fscanf( fp , " %d %d " , &vx , &vy ) ;

	    /* add to the list of cell vertices */
	    if( last_pt ){
		last_pt->next = (POINTPTR) Ysafe_malloc( sizeof(POINT) );
		last_pt = last_pt->next ;
	    } else {
		last_pt = cellarrayG[cell].pts = 
		    (POINTPTR) Ysafe_malloc( sizeof(POINT) ) ;
	    }
	    /* fill up the data */
	    last_pt->next = NULL ;
	    last_pt->x = vx ;
	    last_pt->y = vy ;

	    /* find the cell bounding box */
	    cell_minx = MIN( cell_minx, vx ) ;
	    cell_miny = MIN( cell_miny, vy ) ;
	    cell_maxx = MAX( cell_maxx, vx ) ;
	    cell_maxy = MAX( cell_maxy, vy ) ;

	    if( v == 1 ) {
		ux0 = ux = vx ;
		uy0 = uy = vy ;
	    } else {
		edgeCountG++ ;

		if( ++rippleCount > EXPECTNUMEDGES ) {
		    edgeListG = (EDGEBOXPTR) Ysafe_realloc( edgeListG,
			(edgeCountG + EXPECTNUMEDGES) * sizeof(EDGEBOX));
		    rippleCount = 1 ;
		}
		if( v % 2 == 0 ) {
		    edgeListG[edgeCountG].start  = (uy <= vy) ? uy : vy ;
		    edgeListG[edgeCountG].end    = (uy <= vy) ? vy : uy ;
		    edgeListG[edgeCountG].loc    = ux ;
		    edgeListG[edgeCountG].length = (uy <= vy) ? 
						(vy - uy) : (uy - vy) ;
		    edgeListG[edgeCountG].UorR   = (uy <= vy) ? 1 : -1 ;
		    tinsert( &vFixedEdgeRootG, ux , edgeCountG ) ;
		    tinsert( &vEdgeRootG, ux , edgeCountG ) ;
		} else {
		    edgeListG[edgeCountG].start  = (ux <= vx) ? ux : vx ;
		    edgeListG[edgeCountG].end    = (ux <= vx) ? vx : ux ;
		    edgeListG[edgeCountG].loc    = uy ;
		    edgeListG[edgeCountG].length = (ux <= vx) ? 
						(vx - ux) : (ux - vx) ;
		    edgeListG[edgeCountG].UorR   = (ux <= vx) ? 1 : -1 ;
		    tinsert( &hFixedEdgeRootG, uy , edgeCountG ) ;
		    tinsert( &hEdgeRootG, uy , edgeCountG ) ;
		}
		edgeListG[edgeCountG].cell  = cell ;
		edgeListG[edgeCountG].fixed = 1 ;
		edgeListG[edgeCountG].nextEdge = edgeCountG + 1 ;
		if( v == 2 ) {
		    edgeListG[edgeCountG].prevEdge = edgeCountG + 
							vertices - 1 ;
		} else {
		    edgeListG[edgeCountG].prevEdge = edgeCountG - 1 ;
		}
		ux = vx ;
		uy = vy ;
	    }
	}
	vx = ux0 ;
	vy = uy0 ;
	edgeCountG++ ;


	if( ++rippleCount > EXPECTNUMEDGES ) {
	    edgeListG = (EDGEBOXPTR) Ysafe_realloc( edgeListG,
		    (edgeCountG + EXPECTNUMEDGES) * sizeof(EDGEBOX));
	    rippleCount = 1 ;
	}
	edgeListG[edgeCountG].cell   = cell ;
	edgeListG[edgeCountG].start  = (ux <= vx) ? ux : vx ;
	edgeListG[edgeCountG].end    = (ux <= vx) ? vx : ux ;
	edgeListG[edgeCountG].loc    = uy ;
	edgeListG[edgeCountG].length = (ux <= vx) ? (vx - ux) : (ux - vx);
	edgeListG[edgeCountG].UorR   = (ux <= vx) ? 1 : -1 ;
	edgeListG[edgeCountG].prevEdge = edgeCountG - 1 ;
	edgeListG[edgeCountG].nextEdge = edgeCountG - (vertices - 1) ;
	edgeListG[edgeCountG].fixed    = 1 ;
	tinsert( &hFixedEdgeRootG, uy , edgeCountG ) ;
	tinsert( &hEdgeRootG, uy , edgeCountG ) ;

	/* now find the core bounding box */
	if( cell_maxx > bbrG ) {
	    bbrG = cell_maxx ;
	}
	if( cell_minx < bblG ) {
	    bblG = cell_minx ;
	}
	if( cell_maxy > bbtG ) {
	    bbtG = cell_maxy ;
	}
	if( cell_miny < bbbG ) {
	    bbbG = cell_miny ;
	}

	/* save the cell center */
	cellarrayG[cell].xc = (cell_minx + cell_maxx ) / 2 ;
	cellarrayG[cell].yc = (cell_miny + cell_maxy ) / 2 ;
	cellarrayG[cell].tiles = Yrbtree_init( compare_tiles ) ;

    } else {
	fprintf(fpdebugG,"Found unparsable keyword: < %s > in ",
						    input );
	fprintf(fpdebugG,"the .geo file\n");
	fprintf(fpdebugG,"Current cell: %d\n", cell ) ;
	YexitPgm(PGMFAIL);
    }
}
numberCellsG = cell ;

bbbG-- ;
bbtG++ ;
bblG-- ;
bbrG++ ;

edgeListG = (EDGEBOXPTR) Ysafe_realloc( edgeListG,
			(3 * edgeCountG + 5) * sizeof(EDGEBOX));
    /*  Each fixed edge can give rise to at most two
	nonfixed edges */
edgeListG[++edgeCountG].start  = bbbG ;
edgeListG[edgeCountG].end      = bbtG ;
edgeListG[edgeCountG].loc      = bblG ;
edgeListG[edgeCountG].length   = bbtG - bbbG ;
edgeListG[edgeCountG].fixed    = 1 ;
edgeListG[edgeCountG].UorR     = -1 ;
edgeListG[edgeCountG].prevEdge = edgeCountG + 1 ;
edgeListG[edgeCountG].nextEdge = edgeCountG + 3 ;
edgeListG[edgeCountG].cell     = -1 ;

edgeListG[++edgeCountG].start  = bblG ;
edgeListG[edgeCountG].end      = bbrG ;
edgeListG[edgeCountG].loc      = bbtG ;
edgeListG[edgeCountG].length   = bbrG - bblG ;
edgeListG[edgeCountG].fixed    = 1 ;
edgeListG[edgeCountG].UorR     = -1 ;
edgeListG[edgeCountG].prevEdge = edgeCountG + 1 ;
edgeListG[edgeCountG].nextEdge = edgeCountG - 1 ;
edgeListG[edgeCountG].cell     = -1 ;

edgeListG[++edgeCountG].start  = bbbG ;
edgeListG[edgeCountG].end      = bbtG ;
edgeListG[edgeCountG].loc      = bbrG ;
edgeListG[edgeCountG].length   = bbtG - bbbG ;
edgeListG[edgeCountG].fixed    = 1 ;
edgeListG[edgeCountG].UorR     = 1 ;
edgeListG[edgeCountG].prevEdge = edgeCountG + 1 ;
edgeListG[edgeCountG].nextEdge = edgeCountG - 1 ;
edgeListG[edgeCountG].cell     = -1 ;

edgeListG[++edgeCountG].start  = bblG ;
edgeListG[edgeCountG].end      = bbrG ;
edgeListG[edgeCountG].loc      = bbbG ;
edgeListG[edgeCountG].length   = bbrG - bblG ;
edgeListG[edgeCountG].fixed    = 1 ;
edgeListG[edgeCountG].UorR     = 1 ;
edgeListG[edgeCountG].prevEdge = edgeCountG - 3 ;
edgeListG[edgeCountG].nextEdge = edgeCountG - 1 ;
edgeListG[edgeCountG].cell     = -1 ;

numProbesG = 0 ;

return ;
}
