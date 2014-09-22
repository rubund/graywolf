/* ----------------------------------------------------------------- 
FILE:	    graphics.c                                       
DESCRIPTION:graphics routines
CONTENTS:   init_graphics( windowId )
		int windowId ;
	    process_graphics()
	    drawCells()
	    draw_the_data()
	    draw_chan_graph()
	    drawPolarGraph()
	    draw_xygraph()
DATE:	    Feb 28, 1988 
REVISIONS:  Apr 20, 1990 - added switch to look at all nodes in dbx.
		Also fixed crossreference for the user.
	    Wed May  1 19:07:26 EDT 1991 - renumbered menus and
		added new labels switches.
	    Thu Jun  6 15:00:59 CDT 1991 - made debugging
		easier with normal_mode switch.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) graphics.c version 1.11 5/22/92" ;
#endif

#ifndef NOGRAPHICS

#include <geo.h>
#include <dens.h>
#include <yalecad/debug.h>
#include <yalecad/message.h>
#include <yalecad/colors.h>
#include <yalecad/draw.h>
#include <yalecad/system.h>
#include <menus.h>


#define MENUP        "gengraph_menu"
#define HOWMANY 0
#define TRUECELL            2
#define NODESIZE 	    5
#define NODESIZE2 	    8
#define NODECOLOR 	    TWBLACK
#define EDGECOLOR 	    TWCYAN
#define FIXVERTCOLOR 	    TWBLACK
#define CRVERTCOLOR 	    TWORANGE
#define FIXHORZCOLOR 	    TWBLUE
#define CRHORZCOLOR  	    TWRED
#define NODEHORZCOLOR  	    TWYELLOW
#define NODEVERTCOLOR  	    TWYELLOW
#define NODENEWVERTCOLOR    TWBLACK

/* #define DEVELOPMENU */
/* During development use TWread_menus in place of menuS */
/* to create menu record, ie.  TWread_menus(MENUP) */
#ifdef DEVELOPMENU
#define MENU   (TWMENUPTR) TWread_menus(MENUP)
#else
#define MENU   menuS
#endif

/* ***************************************************************** 
   DUMP CURRENT MC CONFIGURATION.  USER INTERFACE
*/
static BOOL drawxS = FALSE ;   /* whether or not to x graph */
static BOOL drawyS = FALSE ;   /* whether or not to y graph */
static BOOL drawxPolarS = FALSE ;    /* whether or not to polar graph */
static BOOL drawyPolarS = FALSE ;    /* whether or not to polar graph */
static BOOL drawLabelS = FALSE ;  /* whether to draw labels or not */
static BOOL auto_drawS = TRUE ;/* whether to draw immediately after exp.*/
static BOOL edgeFlagS  = FALSE ;  /* whether to draw edges of cells */
static BOOL whiteFlagS = FALSE ; /* whether to draw white spaces */
static BOOL whiteEdgeS = TRUE ; /* whether to draw edges of graph */
static BOOL nodeFlagS = TRUE ;  /* whether to draw switch boxes */
static BOOL normal_modeS = TRUE ; /* used to turn of graph cutting */
static BOOL drawEdgeLabelS = FALSE ; /* draw edge labels of changraph */
static BOOL drawRtileLabelS = FALSE ; /* draw edge labels on rtiles */
static int *crossrefS ;         /* crossreference table */

/* Forward references */
INT draw_the_data() ;

init_graphics( argc, argv, windowId )
INT argc, windowId ;
char *argv[] ;
{
    char *host ;
    int xpand, ypand ;

    if( !(doGraphicsG) ){
	return ;
    }
	
    /* we need to find host for display */
    if(!(host = Ygetenv("DISPLAY"))) {
	M(WARNMSG,"initMCGraphics","Can't get environment variable ");
	M(MSG,NULL, "for display.  Aborting graphics...\n\n" ) ;
	doGraphicsG = FALSE ;
	return ;
    }
    if( windowId ){
	/* init windows as a parasite */
	if( !( TWinitParasite(argc,argv,TWnumcolors(),TWstdcolors(),
	    FALSE, MENU, draw_the_data,windowId ))){
	    M(ERRMSG,"initGraphics","Aborting graphics.");
	    doGraphicsG = FALSE ;
	    return ;
	}
    } else {
	/* init window as a master */
	if(!(TWinitGraphics(argc,argv,TWnumcolors(),TWstdcolors(),
	    FALSE, MENU, draw_the_data ))){
	    M(ERRMSG,"initGraphics","Aborting graphics.");
	    doGraphicsG = FALSE ;
	    return ;
	}
    }
    xpand = (int) ( 0.1 * (DOUBLE) (bbrG - bblG )) ;
    ypand = (int) ( 0.1 * (DOUBLE) (bbtG - bbbG )) ;
    TWsetwindow( bblG-xpand, bbbG-ypand, bbrG+xpand, bbtG+ypand ) ;
    TWdrawMenus() ;
    TWmessage("To interupt program click on top menu window") ;
    TWflushFrame() ;
    TWinterupt() ;
    sleep( (unsigned) 2 ) ;

} /* end initGraphics */

/* heart of the graphic syskem processes user input */
process_graphics()
{

    INT selection ;     /* the users pick */
    INT x, y ;

    /* data might have changed so show user current config */
    /* any function other that the draw controls need to worry about */
    /* this concurrency problem -  show user current config */
    draw_the_data() ;
    /* use TWcheckExposure to flush exposure events since we just */
    /* drew the data */
    TWcheckExposure() ;
    TWmessage( "gengraph is waiting for your response..." ) ;

    selection  = CANCEL ;
    while( selection != CONTINUE_PROGRAM ){ /* loop until exit */
	selection = TWcheckMouse() ;
	switch( selection ){
	case CANCEL:
	    /* do nothing */
	    break ;
	case AUTO_REDRAW_ON:
	    auto_drawS = TRUE ;
	    break ;
	case AUTO_REDRAW_OFF:
	    auto_drawS = FALSE ;
	    break ;
	case CLOSE_GRAPHICS:
	    TWcloseGraphics() ;
	    doGraphicsG = FALSE ;
	    return ;
	case COLORS:
	    TWtoggleColors() ;
	    break ;
	case CONTINUE_PROGRAM:
	    break ;
	case DUMP_GRAPHICS:
	    /* now change mode to dump to file */
	    TWsetMode(1) ;
	    /* dump the data to a file now instead of screen */
	    draw_the_data() ;
	    /* restore the state to previous condition and set */
	    /* draw to screen */
	    TWsetMode(0) ;
	    break ;
	case FULLVIEW:
	    TWfullView() ;
	    break ;
	case REDRAW:
	    draw_the_data() ;
	    /* use TWcheckExposure to flush exposure events since */
	    /* we just drew the data */
	    TWcheckExposure() ;
	    break ;
	case TELL_POINT:
	    TWmessage( "Pick a point" ) ;
	    TWgetPt( &x, &y ) ;
	    sprintf( YmsgG,"The point is (%d,%d)",x,y ) ;
	    TWmessage( YmsgG ) ;
	    break ;
	case TRANSLATE:
	    TWtranslate() ;
	    break ;
	case ZOOM:
	    TWzoom() ;
	    break ;
	case DRAW_CELL_EDGES:
	    edgeFlagS  = TRUE ;  /* draw edges of cells */
	    TWforceRedraw() ;
	    break ;
	case IGNORE_CELL_EDGES:
	    edgeFlagS  = FALSE ;  /* don't draw edges of cells */
	    TWforceRedraw() ;
	    break ;
	case DRAW_CHANGRAPH:
	    whiteEdgeS = TRUE ; /* draw edges of graph */
	    nodeFlagS = TRUE ;  /* draw switch boxes */
	    TWforceRedraw() ;
	    break ;
	case IGNORE_CHANGRAPH:
	    whiteEdgeS = FALSE ; /* don't draw edges of graph */
	    nodeFlagS = FALSE ;  /* don't draw switch boxes */
	    TWforceRedraw() ;
	    break ;
	case DRAW_LABELS:
	    drawLabelS = TRUE ;
	    TWmessage( "Labels will be drawn" ) ;
	    TWforceRedraw() ;
	    break ;
	case IGNORE_LABELS:
	    drawLabelS = FALSE ;
	    TWmessage( "Labels will be omitted from drawing" ) ;
	    TWforceRedraw() ;
	    break ;
	case DRAW_WHITE_SPACE :
	    whiteFlagS = TRUE ; /* draw white spaces */
	    TWforceRedraw() ;
	    break ;
	case IGNORE_WSPACE :
	    whiteFlagS = FALSE ; /* don't draw white spaces */
	    TWforceRedraw() ;
	    break ;
	case DRAW_X_POLAR_GRAPH :
	    drawxPolarS = TRUE ; /* draw polar graph */
	    TWforceRedraw() ;
	    break ;
	case IGNORE_X_POLAR :
	    drawxPolarS = FALSE ; /* don't draw polar graph */
	    TWforceRedraw() ;
	    break ;
	case DRAW_Y_POLAR_GRAPH :
	    drawyPolarS = TRUE ; /* draw polar graph */
	    TWforceRedraw() ;
	    break ;
	case IGNORE_Y_POLAR :
	    drawyPolarS = FALSE ; /* don't draw polar graph */
	    TWforceRedraw() ;
	    break ;
	case DRAW_X_GRAPH:
	    drawxS = TRUE ;   /* whether or not to x graph */
	    TWforceRedraw() ;
	    break ;
	case IGNORE_X_GRAPH:
	    drawxS = FALSE ;   /* whether or not to x graph */
	    TWforceRedraw() ;
	    break ;
	case DRAW_Y_GRAPH:
	    drawyS = TRUE ;   /* whether or not to y graph */
	    TWforceRedraw() ;
	    break ;
	case IGNORE_Y_GRAPH:
	    drawyS = FALSE ;   /* whether or not to y graph */
	    TWforceRedraw() ;
	    break ;
	case DRAW_EDGE_LABELS:
	    drawEdgeLabelS = TRUE ;
	    break ;
	case IGNORE_EDGE_LABELS:
	    drawEdgeLabelS = FALSE ;
	    break ;
	case DRAW_RTILE_LABELS:
	    drawRtileLabelS = TRUE ;
	    break ;
	case IGNORE_RTILE_LABELS:
	    drawRtileLabelS = FALSE ;
	    break ;

	} /*********************** end graphics SWITCH *****************/
	if( auto_drawS && TWcheckExposure() ){
	    draw_the_data() ;
	}

    } 

    TWmessage("Continuing-to interupt program click on top menu window");
    /* use TWinterupt to turn off window enter/leave lights */
    TWinterupt() ;

} /* end process_graphics */

static draw_routing( cell ) 
INT cell ;
{
    YTREEPTR tree ;
    TILEPTR tile ;
    char label[LRECL] ;
    char *labelptr ;

    tree = cellarrayG[cell].tiles ;

    for( tile = (TILEPTR) Yrbtree_enumerate( tree, TRUE ) ;
	tile ;
        tile = (TILEPTR) Yrbtree_enumerate( tree, FALSE ) ){
	if( drawRtileLabelS ){
	    sprintf( label, "space:%d edge:%d", tile->space,tile->edge ) ;
	    labelptr = label ;
	} else {
	    labelptr = NIL(char *) ;
	}
	TWdrawRect( cell, tile->l, tile->b, tile->r, tile->t,
	    TWBLUE, labelptr ) ;
    }

} /* end draw_routing */


drawCells()
{
    int i ;
    POINTPTR pt ;
    char label[LRECL] ;
    char *labelptr ;

    for( i = 1; i <= numberCellsG ; i++ ){
	TWarb_init() ;
	for( pt = cellarrayG[i].pts; pt; pt=pt->next ){
	    TWarb_addpt( pt->x, pt->y ) ;
	}
	if( drawLabelS ){
	    sprintf( label, "C:%d", i ) ;
	    labelptr = label ;
	} else {
	    labelptr = NIL(char *) ;
	}
	TWdrawArb( i, TWGREEN, labelptr ) ;
	if( TRUE ){
	    TWcolorXOR( TWBLUE, TRUE ) ;
	    draw_routing( i ) ;
	    TWcolorXOR( TWBLUE, FALSE ) ;
	}
    }

} /* end drawCells */


INT draw_the_data()
{

    int *get_cross_ref() ;

    if( !(doGraphicsG) ){
	return ;
    }
    TWstartFrame() ;

    drawCells() ;

    crossrefS = get_cross_ref() ;

    draw_chan_graph() ;

    drawPolarGraph() ;

    draw_xygraph() ;

    TWflushFrame() ;

} /* end draw_the_data */

draw_chan_graph()
{
    int  i ;
    int  index1, index2 ;
    DLINK1PTR curEdge ;
    DLINK2PTR curNode ;
    EDGEBOX edgeRec;
    RECT nodeRec, nodeRec2 ;
    char label[LRECL] ;
    char *labelptr ;

    if( whiteFlagS && VRlistG ){
	/* first output the vertical whiteSpaces */
	for( curNode=VRlistG;curNode;curNode=curNode->next ){
	    nodeRec = rectArrayG[curNode->index] ;
	    if( drawLabelS ){
		sprintf( label, "%d", curNode->index ) ;
		labelptr = label ;
	    } else {
		labelptr = NULL ;
	    }
	    TWdrawRect( 0,
		nodeRec.l, nodeRec.b, nodeRec.r, nodeRec.t,
		NODEVERTCOLOR, labelptr ) ;
	}
	/* now the horizontal whiteSpaces */
	for( curNode=HRlistG;curNode;curNode=curNode->next ){
	    nodeRec = rectArrayG[curNode->index] ;
	    if( drawLabelS ){
		sprintf( label, "%d", curNode->index ) ;
		labelptr = label ;
	    } else {
		labelptr = NULL ;
	    }
	    TWdrawRect( 0, nodeRec.l, nodeRec.b, nodeRec.r, nodeRec.t,
		NODEHORZCOLOR, labelptr ) ;
	}
    }

    if( edgeFlagS && VlistG ){
	/* first output the vertical edges */
	for( curEdge=VlistG;curEdge;curEdge=curEdge->next ){
	    edgeRec = edgeListG[curEdge->edge] ;
	    if( edgeRec.fixed ){ /* cell edges */
		TWdrawLine(
		    curEdge->edge,
		    edgeRec.loc,
		    edgeRec.start,
		    edgeRec.loc,
		    edgeRec.end,
		    FIXVERTCOLOR,
		    NULL ) ;
	    } else {
		TWdrawLine(
		    curEdge->edge,
		    edgeRec.loc,
		    edgeRec.start,
		    edgeRec.loc,
		    edgeRec.end,
		    CRVERTCOLOR,
		    NULL ) ;
	    }
	}
	/* now the horizontal edges */
	for( curEdge=HlistG;curEdge;curEdge=curEdge->next ){
	    edgeRec = edgeListG[curEdge->edge] ;
	    if( edgeRec.fixed ){ /* cell edges */
		TWdrawLine(
		    curEdge->edge,
		    edgeRec.start,
		    edgeRec.loc,
		    edgeRec.end,
		    edgeRec.loc,
		    FIXHORZCOLOR,
		    NULL ) ;
	    } else {
		TWdrawLine(
		    curEdge->edge,
		    edgeRec.start,
		    edgeRec.loc,
		    edgeRec.end,
		    edgeRec.loc,
		    CRHORZCOLOR,
		    NULL ) ;
	    }
	}
    }

    /* draw edges in channel graph */
    if( whiteEdgeS && rectArrayG ){
	for( i=1;i<=eNumG;i++ ){
	    index1 = eArrayG[i].index1 ;
	    index2 = eArrayG[i].index2 ;
	    if( normal_modeS && 
		(!(rectArrayG[index1].active) ||
	        !(rectArrayG[index2].active))){
		continue ;
	    }
	    nodeRec  = rectArrayG[index1] ;
	    nodeRec2 = rectArrayG[index2] ;
	    if( drawEdgeLabelS ){
		sprintf( label, "E%d:%d", i, eArrayG[i].density ) ;
		labelptr = label ;
	    } else {
		labelptr = NULL ;
	    }
	    TWdrawLine( i, nodeRec.xc, nodeRec.yc,
		    nodeRec2.xc, nodeRec2.yc, EDGECOLOR, labelptr ) ;
	}
    } 

    /* now output nodes of changraph */
    /* pins are the terminal points on a net */
    if( nodeFlagS && rectArrayG ){ /* only draw when requested */
	for( i = 1; i<= numRectsG; i++ ){
	    nodeRec = rectArrayG[i] ;
	    if( normal_modeS && !(nodeRec.active) ){
		continue ;
	    }
	    if( drawLabelS ){
		if( normal_modeS ){
		    sprintf( label, "N%d", crossrefS[i] ) ;
		} else {
		    sprintf( label, "N%d", i ) ;
		}
		labelptr = label ;
	    } else {
		labelptr = NULL ;
	    }
	    TWdrawRect(
		i,
		nodeRec.xc - NODESIZE, nodeRec.yc - NODESIZE,
		nodeRec.xc + NODESIZE, nodeRec.yc + NODESIZE,
		NODECOLOR, labelptr ) ;
	}
    }
} /* end draw_chan_graph */

drawPolarGraph()
{

    int  i ;
    int  x2, y2 ;
    int  node2 ;
    int  *xref, *yref ;
    int  *get_xref_polar() ;
    int  *get_yref_polar() ;
    char label[LRECL] ;
    char *labelptr ;
    POLARPTR xptr, yptr ;
    POLAREDGEPTR edgeptr ;

    /* draw the xpolar graph */
    if( drawxPolarS ){
	xref = get_xref_polar() ;
	for( i=0; i <= numxPolarG; i++ ){
	    xptr = &( xPolarG[i] ) ;
	    if( !(xptr->valid) ){
		continue ;
	    }
	    /* translate to transformed node */
	    /* draw the nodes */
	    if( drawLabelS ){
		sprintf( label,"N%d", xref[i] ) ;
		labelptr = label ;
	    } else {
		labelptr = NULL ;
	    }
	    TWdrawRect( i, 
		xptr->xc - NODESIZE, xptr->yc - NODESIZE,
		xptr->xc + NODESIZE, xptr->yc + NODESIZE,
		TWBLACK, labelptr ) ;

	    /* now draw the edges */
	    for( edgeptr=xptr->adj;edgeptr; edgeptr=edgeptr->next ){
		if( !(edgeptr->valid) ){
		    continue ;
		}
		node2 = edgeptr->node ;
		x2 = xPolarG[node2].xc ;
		y2 = xPolarG[node2].yc ;

		TWdrawLine( i, xptr->xc, xptr->yc, x2, y2, TWRED, NULL ) ;
	    }
	}
    }

    /* draw the ypolar graph */
    if( drawyPolarS ){
	yref = get_yref_polar() ;
	for( i=0; i <= numyPolarG; i++ ){
	    yptr = &( yPolarG[i] ) ;
	    if( !(yptr->valid) ){
		continue ;
	    }
	    /* draw the nodes */
	    if( drawLabelS ){
		sprintf( label,"N%d", yref[i] ) ;
		labelptr = label ;
	    } else {
		labelptr = NULL ;
	    }
	    TWdrawRect( i, 
		yptr->xc - NODESIZE, yptr->yc - NODESIZE,
		yptr->xc + NODESIZE, yptr->yc + NODESIZE,
		TWBLACK, labelptr ) ;

	    /* now draw the edges */
	    for( edgeptr=yptr->adj;edgeptr; edgeptr=edgeptr->next ){
		if( !(edgeptr->valid) ){
		    continue ;
		}
		node2 = edgeptr->node ;
		x2 = yPolarG[node2].xc ;
		y2 = yPolarG[node2].yc ;

		TWdrawLine( i, yptr->xc, yptr->yc, x2, y2, TWBLUE, NULL ) ;
	    }
	}
    } /* end draw ypolar graph */

} /* end drawPolorGraph */

draw_xygraph()
{

    int  x1, y1 ;
    int  x2, y2 ;
    int  node ;
    int  cell ;
    int  iedge, oedge ;
    int  lastE ;
    int  howmany ;
    EDGEBOX e1ptr, e2ptr;
    FLAREPTR ifptr , ofptr ;

    /* now find nodes which are cells and empty rooms */
    /* this loop find nodes which are cells and build node */
    /* cross reference array */
    if( drawxS ){
	for( cell = 1 ; cell <= numberCellsG ; cell++ ) {
	    ifptr = xNodulesG[cell].inList ;
	    for( ; ifptr != (FLAREPTR) NULL ; ifptr = ifptr->next ) {
		iedge = ifptr->fixEdge ;
		e1ptr = edgeListG[iedge] ;
		x1 = e1ptr.loc ;
		y1 = ( e1ptr.start + e1ptr.end ) / 2 ;


		ofptr = xNodulesG[cell].outList ;
		for( ; ofptr != (FLAREPTR) NULL ; ofptr = ofptr->next ) {
		    oedge = ofptr->fixEdge ;
		    e2ptr = edgeListG[oedge] ;
		    x2 = e2ptr.loc;
		    y2 = (e2ptr.start + e2ptr.end ) / 2 ;
		    TWdrawLine( iedge, x1, y1, x2, y2, TWRED,
			NULL ) ;
		}
	    }
	}

	/* now we need to add edges to the graph */
	for( cell = 0 ; cell <= numberCellsG + 1 ; cell++ ) {
	    ofptr = xNodulesG[cell].outList ;
	    for( ; ofptr != (FLAREPTR) NULL ; ofptr = ofptr->next ) {
		iedge = ofptr->fixEdge ;
		e1ptr = edgeListG[iedge] ;
		x1 = e1ptr.loc ;
		y1 = ( e1ptr.start + e1ptr.end ) / 2 ;

		howmany = ofptr->eindex[HOWMANY] ;
		lastE = ofptr->eindex[ howmany ] ;
		e2ptr = edgeListG[ eArrayG[lastE].edge2 ] ;
		x2 = e2ptr.loc;
		y2 = (e2ptr.start + e2ptr.end ) / 2 ;
		TWdrawLine( iedge, x1, y1, x2, y2, TWBLUE, NULL ) ;
	    }
	}
    } /* end of drawxS */


    /* now find nodes which are cells and empty rooms */
    /* this loop find nodes which are cells and build node */
    /* cross reference array */
    if( drawyS ){
	for( cell = 1 ; cell <= numberCellsG ; cell++ ) {
	    ifptr = yNodulesG[cell].inList ;
	    for( ; ifptr != (FLAREPTR) NULL ; ifptr = ifptr->next ) {
		iedge = ifptr->fixEdge ;
		e1ptr = edgeListG[iedge] ;
		x1 = ( e1ptr.start + e1ptr.end ) / 2 ;
		y1 = e1ptr.loc ;


		ofptr = yNodulesG[cell].outList ;
		for( ; ofptr != (FLAREPTR) NULL ; ofptr = ofptr->next ) {
		    oedge = ofptr->fixEdge ;
		    e2ptr = edgeListG[oedge] ;
		    x2 = (e2ptr.start + e2ptr.end ) / 2 ;
		    y2 = e2ptr.loc;
		    TWdrawLine( iedge, x1, y1, x2, y2, TWORANGE, NULL ) ;
		}
	    }
	}

	/* now we need to add edges to the graph */
	for( cell = 0 ; cell <= numberCellsG + 1 ; cell++ ) {
	    ofptr = yNodulesG[cell].outList ;
	    for( ; ofptr != (FLAREPTR) NULL ; ofptr = ofptr->next ) {
		iedge = ofptr->fixEdge ;
		e1ptr = edgeListG[iedge] ;
		x1 = ( e1ptr.start + e1ptr.end ) / 2 ;
		y1 = e1ptr.loc ;

		howmany = ofptr->eindex[HOWMANY] ;
		lastE = ofptr->eindex[ howmany ] ;
		e2ptr = edgeListG[ eArrayG[lastE].edge2 ] ;
		x2 = (e2ptr.start + e2ptr.end ) / 2 ;
		y2 = e2ptr.loc;
		TWdrawLine( iedge, x1, y1, x2, y2, TWVIOLET, NULL ) ;
	    }
	}
    } /* end drawyS */

} /* end draw_xygraph */

#endif /* NOGRAPHICS compile */
