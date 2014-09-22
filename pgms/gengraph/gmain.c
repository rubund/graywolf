/* ----------------------------------------------------------------- 
FILE:	    gmain.c                                       
DESCRIPTION:This file contains the main routine for the channel 
	    definition program.  It is a separate entity and reads
	    the output of the placement program.
CONTENTS:   main( argc , argv )
		int argc ;
		char *argv[] ;
	    syntax()
	    gmain()
DATE:	    Apr 17, 1988 
REVISIONS:  Jun 07, 1988 - added routing order code.
	    Jun 17, 1988 - added free of channel graph.
	    Mar 16, 1989 - removed channel ordering code.
	    Sep 14, 1989 - added polar graphs for new global router.
	    Sep 19, 1989 - fixed nographics bug.
	    Feb  8, 1990 - Reset pointer to NULL when freed.
	    Mar  1, 1990 - Made into a separate program.
	    Sun Dec 16 01:27:38 EST 1990 - rearranged order of gentwf
		and printgph.
	    Fri Jan 25 16:25:17 PST 1991 - added closegraphics calls
		so that code ends correctly.
	    Thu Mar  7 01:38:12 EST 1991 - added Xdebug switch.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) gmain.c version 1.8 4/21/91" ;
#endif

#define GEO_DEFS  /*** put all chandef globals at top of this file ***/
#include <geo.h>
#undef DEBUG
#include <yalecad/debug.h>
#include <yalecad/cleanup.h>
#include <yalecad/file.h>
#include <yalecad/message.h>
#include <yalecad/string.h>

#define EXPECTEDMEMORY  (256 * 1024)  /* 256k is more than enough */
#define PNAME           "gengraph"

static int windowIdS ;   /* the window id */
static BOOL routingS ;   /* TRUE if reading global routing density */
static BOOL allNodeS ;   /* if TRUE should original graph */

main( argc , argv )
int argc ;
char *argv[] ;
{

    char        *ptr ;
    BOOL        parasite ;

#ifdef DEBUGX
    extern int _Xdebug ;
    _Xdebug = TRUE ;
#endif

    /* start up cleanup handler */
    YINITCLEANUP( argv[0], NULL, MAYBEDUMP ) ;

    Yinit_memsize( EXPECTEDMEMORY ) ;

    if( argc < 2 || argc > 4 ){
	syntax() ;
    } else {
	doGraphicsG  = TRUE ;
	parasite   = FALSE ;
	debugG     = FALSE ;
	windowIdS  = 0 ;
	routingS   = FALSE ;
	allNodeS   = FALSE ;
	if( *argv[1] == '-' ){
	    for( ptr = ++argv[1]; *ptr; ptr++ ){
		switch( *ptr ){
		case 'a':
		    allNodeS = TRUE ;
		    break ;
		case 'd':
		    debugG = TRUE ;
		    break ;
		case 'n':
		    doGraphicsG = FALSE ;
		    break ;
		case 'r':
		    routingS = TRUE ;
		    break ;
		case 'w':
		    parasite = TRUE ;
		    break ;
		default:
		    sprintf( YmsgG,"Unknown option:%c\n", *ptr ) ;
		    M(ERRMSG,"main",YmsgG);
		    syntax() ;
		}
	    }
	    YdebugMemory( debugG ) ;
	    fprintf( stdout, "\n%s\n",YinitProgram(PNAME,"v1.0",NULL));

	    cktNameG = Ystrclone( argv[2] );
	    /* now tell the user what he picked */
	    M(MSG,NULL,"gengraph switches:\n" ) ;
	    if( debugG ){
		YsetDebug( TRUE ) ;
		M(MSG,NULL,"\tdebug on\n" ) ;
	    } 
	    if( doGraphicsG ){
		M(MSG,NULL,"\tGraphics mode on\n" ) ;
	    } else {
		M(MSG,NULL,"\tGraphics mode off\n" ) ;
	    }
	    if( routingS ){
		M(MSG,NULL,"\tRouting update mode on\n" ) ;
	    } else {
		M(MSG,NULL,"\tGenerating channel graph\n" ) ;
	    }
	    if( parasite ){
		M(MSG,NULL,"\tgengraph will inherit window\n" ) ;
		/* look for windowid */
		if( argc != 4 ){
		    M(ERRMSG,"gengraph","Need to specify windowID\n" ) ;
		    syntax() ;
		} else {
		    windowIdS = atoi( argv[3] ) ;
		} 
	    }
	    M(MSG,NULL,"\n" ) ;
	} else if( argc == 2 ){
	    cktNameG = Ystrclone( argv[1] );
	    YdebugMemory( FALSE ) ;
	    fprintf( stdout, "\n%s\n",YinitProgram(PNAME,"v1.0",NULL));
	} else {
	    syntax() ;
	}
    }

    /* ******************** end initialization *********************** */
    gmain(argc,argv) ; /* the main algorithm */

    closegraphics() ;
    YexitPgm(PGMOK);

} /* end main */


/* give user correct syntax */
syntax()
{
   M(ERRMSG,NULL,"\n" ) ; 
   M(MSG,NULL,"Incorrect syntax.  Correct syntax:\n");
   sprintf( YmsgG, 
       "\n%s [-dnrw] designName [windowId] \n",
	PNAME );
   M(MSG,NULL,YmsgG ) ; 
   M(MSG,NULL,"\twhose options are one or more of the following:\n");
   M(MSG,NULL,"\t\td - prints debug info and performs extensive\n");
   M(MSG,NULL,"\t\t    error checking\n");
   M(MSG,NULL,"\t\tn - no graphics - the default is to open the\n");
   M(MSG,NULL,"\t\t    display and output graphics to an Xwindow\n");
   M(MSG,NULL,"\t\tr - read the global routing information and\n");
   M(MSG,NULL,"\t\t    output the routing area tiles.\n");
   M(MSG,NULL,"\t\tw - parasite mode - user must specify windowId\n");
   YexitPgm(PGMFAIL);
} /* end syntax */

closegraphics()
{
    if( doGraphicsG ){
	G( TWcloseGraphics() ) ;
    }
} /* end closegraphics */

gmain(argc,argv)
INT argc ;
char *argv[] ;
{

    FILE *fp ;
    TNODEPTR tnode ;
    char filename[64] ;
    int dummy1 , dummy2 , i ;
    FLAREPTR fptr , f2ptr ;
    DLINK2PTR hrptr , hr2ptr ;
    DLINK1PTR hptr , h2ptr ;
    int longestEdge ;
    int *crossref, *remove_outside_channels() ;
    int *keep_pad_nodes() ;

    M(MSG,NULL,"Channel generation begins...\n") ;
    sprintf( filename , "%s.mdebug" , cktNameG ) ;
    fpdebugG = TWOPEN ( filename , "w", ABORT ) ;

    /* find out track spacing and default tracks */
    read_par_file() ;

    sprintf( filename , "%s.mgeo" , cktNameG ) ;
    fp = TWOPEN ( filename , "r", ABORT ) ;

    /* Standalone program reads placement output */
    readgeo( fp ) ;


    makelink()    ;

    /* find critical regions - (cr)  */
    hprobes()     ;
    vprobes()     ;

    /* cleanup */
    Ysafe_free( VptrsG ) ;
    Ysafe_free( HptrsG ) ;
    if( VrootG != (TNODEPTR) NULL ) {
	for( ; ; ) {
	    tpop( &VrootG , &tnode , &dummy1 , &dummy2 ) ;
	    if( tnode == (TNODEPTR) NULL ) {
		break ;
	    }
	}
    }
    if( HrootG != (TNODEPTR) NULL ) {
	for( ; ; ) {
	    tpop( &HrootG , &tnode , &dummy1 , &dummy2 ) ;
	    if( tnode == (TNODEPTR) NULL ) {
		break ;
	    }
	}
    }
    fulllink()  ;
    /* ----------------------------------------------------------------- 
	After full link, all critical regions have been discovered and
	all data structures( 23trees ) have been built. 
    */


    /* Find white spaces */
    findnodes() ; 

    /* Add edges to build channel graph */
    changraph() ;

    /* the channels formed outside the pads are only an artifact of
       generating the channel graph so remove them. */
    if( !(allNodeS) ){
	crossref = remove_outside_channels() ;
    } else {
	crossref = keep_pad_nodes() ;
    }

    /* turn on the graphics if requested */
    if( doGraphicsG ){
	G( init_graphics( argc,argv, windowIdS ) ) ;
    }
    G( draw_the_data() ) ;

    /* Build x and y graphs for compaction of channels */
    xgraph()    ;
    ygraph()    ;

    /* fix all empty rooms so that the capacities are correct */
    fix_empty_rooms() ;


    /* --------------------------------------------------------------
        If routing update don't build polar graphs and don't output 
	the pins and channel graph. We need to output the routing
	tiles instead.
    ---------------------------------------------------------------- */
    if( routingS ){
	density( crossref ) ;
    } else {
	/* Build polar graphs for compaction of channel */
	buildxPolarGraph() ;
	output_xgraph( crossref ) ;
	buildyPolarGraph() ;
	output_ygraph( crossref ) ;

	/* Output pin locations */
	gentwf( crossref )    ;

	/* Output channel graph */
	printgph( crossref )  ;
    }

#ifndef NOGRAPHICS
    if( doGraphicsG && TWinterupt() ){
	process_graphics() ;
    }
#endif


    /*   ***********************************************    */
    /*         We are done - cleanup memory space           */
    /*   ***********************************************    */


    hrptr = HRlistG ;
    while( hrptr != (DLINK2PTR) NULL ) {
	hr2ptr = hrptr->next ;
	Ysafe_free( hrptr ) ;
	hrptr = hr2ptr ;
    }
    HRlistG = NULL ;

    hrptr = VRlistG ;
    while( hrptr != (DLINK2PTR) NULL ) {
	hr2ptr = hrptr->next ;
	Ysafe_free( hrptr ) ;
	hrptr = hr2ptr ;
    }
    VRlistG = NULL ;

    Ysafe_free( LEptrsG ) ;
    Ysafe_free( BEptrsG ) ;
    if( LErootG != (TNODEPTR) NULL ) {
	for( ; ; ) {
	    tpop( &LErootG , &tnode , &dummy1 , &dummy2 ) ;
	    if( tnode == (TNODEPTR) NULL ) {
		break ;
	    }
	}
    }
    if( BErootG != (TNODEPTR) NULL ) {
	for( ; ; ) {
	    tpop( &BErootG , &tnode , &dummy1 , &dummy2 ) ;
	    if( tnode == (TNODEPTR) NULL ) {
		break ;
	    }
	}
    }

    hptr = HlistG ;
    while( hptr != (DLINK1PTR) NULL ) {
	h2ptr = hptr->next ;
	Ysafe_free( hptr ) ;
	hptr = h2ptr ;
    }
    HlistG = NULL ;

    hptr = VlistG ;
    while( hptr != (DLINK1PTR) NULL ) {
	h2ptr = hptr->next ;
	Ysafe_free( hptr ) ;
	hptr = h2ptr ;
    }
    VlistG = NULL ;

    Ysafe_free( VptrsG ) ;
    Ysafe_free( HptrsG ) ;
    if( VrootG != (TNODEPTR) NULL ) {
	for( ; ; ) {
	    tpop( &VrootG , &tnode , &dummy1 , &dummy2 ) ;
	    if( tnode == (TNODEPTR) NULL ) {
		break ;
	    }
	}
    }
    if( HrootG != (TNODEPTR) NULL ) {
	for( ; ; ) {
	    tpop( &HrootG , &tnode , &dummy1 , &dummy2 ) ;
	    if( tnode == (TNODEPTR) NULL ) {
		break ;
	    }
	}
    }

    Ysafe_free( VDptrsG ) ;
    Ysafe_free( HRptrsG ) ;
    if( VDrootG != (TNODEPTR) NULL ) {
	for( ; ; ) {
	    tpop( &VDrootG , &tnode , &dummy1 , &dummy2 ) ;
	    if( tnode == (TNODEPTR) NULL ) {
		break ;
	    }
	}
    }
    if( HRrootG != (TNODEPTR) NULL ) {
	for( ; ; ) {
	    tpop( &HRrootG , &tnode , &dummy1 , &dummy2 ) ;
	    if( tnode == (TNODEPTR) NULL ) {
		break ;
	    }
	}
    }


    hptr = hFixedListG ;
    while( hptr != (DLINK1PTR) NULL ) {
	h2ptr = hptr->next ;
	Ysafe_free( hptr ) ;
	hptr = h2ptr ;
    }
    hptr = vFixedListG ;
    while( hptr != (DLINK1PTR) NULL ) {
	h2ptr = hptr->next ;
	Ysafe_free( hptr ) ;
	hptr = h2ptr ;
    }

    for( i = 0 ; i <= numberCellsG + 1 ; i++ ) {
	fptr = xNodulesG[i].outList ;
	while( fptr != (FLAREPTR) NULL ) {
	    Ysafe_free( fptr->eindex ) ;
	    f2ptr = fptr->next ;
	    Ysafe_free( fptr ) ;
	    fptr = f2ptr ;
	}
	fptr = xNodulesG[i].inList ;
	while( fptr != (FLAREPTR) NULL ) {
	    Ysafe_free( fptr->eindex ) ;
	    f2ptr = fptr->next ;
	    Ysafe_free( fptr ) ;
	    fptr = f2ptr ;
	}
	fptr = yNodulesG[i].outList ;
	while( fptr != (FLAREPTR) NULL ) {
	    Ysafe_free( fptr->eindex ) ;
	    f2ptr = fptr->next ;
	    Ysafe_free( fptr ) ;
	    fptr = f2ptr ;
	}
	fptr = yNodulesG[i].inList ;
	while( fptr != (FLAREPTR) NULL ) {
	    Ysafe_free( fptr->eindex ) ;
	    f2ptr = fptr->next ;
	    Ysafe_free( fptr ) ;
	    fptr = f2ptr ;
	}
    }
    Ysafe_free( xNodulesG ) ;
    Ysafe_free( yNodulesG ) ;

    TWCLOSE( fpdebugG );
    TWCLOSE( fp );

    M(MSG,NULL,"Channel generation completed...\n") ;

} /* end gmain */
