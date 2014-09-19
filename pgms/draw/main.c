/* ----------------------------------------------------------------- 
FILE:	    main.c                                       
DESCRIPTION:Main routine for drawing program.
CONTENTS:   main( argc, argv )
		int	argc;
		char *argv[];
DATE:	    Jan 25, 1989 - major rewrite of version 1.0
REVISIONS:  Apr  9, 1989 - added process_color_map to change colors.
	    Jun 19, 1989 - fixed order of argument checking.
	    Apr 28, 1990 - added CIF output capability.
	    Oct 4, 1990 - fixed exit problem - no YexitPgm.
	    Oct 14, 1990 - modified to work with new lib routines.
	    Mon May  6 20:37:12 EDT 1991 - added renumber_frames
		and new menus.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) main.c version 3.16 5/6/91" ;
#endif


#include	<ctype.h>
#include	<stdio.h>

#define  MAIN_DEFS
#include	"globals.h"
#include	<yalecad/base.h>
#include	<yalecad/cleanup.h>
#include	<yalecad/debug.h>
#include	<yalecad/draw.h>
#include	<yalecad/file.h>
#include	<yalecad/colors.h>
#include	<yalecad/message.h>
#include	<yalecad/string.h>


#define MENUPATH "draw_menu"
#define VERSION  "v3.0"
#define EXPECTEDMEMORY  (1024 * 1024)

/* #define DEVELOPMENU */
/* During development use TWread_menus in place of menuS */
/* to create menu record, ie.  TWread_menus(MENUP) */
#ifdef DEVELOPMENU
#define MENU   TWread_menus(MENUPATH)
#else
#define MENU   menuS
#endif

#include <menus.h>

main(argc,argv)
int	argc;
char	*argv[];
{
    char        fileName[LRECL] ;
    char        *ptr ;               /* arg pointer */
    char        *colormapfile ;      /* pointer to colormap file */
    int         acount ;             /* argument counter */
    int         yaleIntro() ;
    int         *findColorNums() ;
    int         numcolors;
    int         i ;
    int         windowId ;
    char        *dirName ;
    char        *host ;
    char        *Ygetenv() ;
    char        *Yfixpath() ;
    char        **colors,
		**process_color_map() ;
    FILE        *fp ;
    BOOL        debug ;              /* TRUE if debug is on */
    BOOL        parasite ;           /* parsite mode */
    BOOL        colormap ;           /* TRUE if colormap file is given */
    extern INT	draw_data() ;        /* how to draw the data */

/* ********************** start initialization *********************** */
    /* start up cleanup handler */
    YINITCLEANUP( argv[0], NULL, MAYBEDUMP ) ;

    Yinit_memsize( EXPECTEDMEMORY ) ;

    if( argc > 4 ){
	syntax() ;
    } else {
	debug       = FALSE ;
	parasite    = FALSE ;
	colormap    = FALSE ;
	windowId    = 0 ;
	acount      = 1 ;
	if( argc == 1 ){
	    /* order is important here */
	    YdebugMemory( FALSE ) ;
	    YinitProgram( "draw", VERSION, yaleIntro ) ;

	} else if( *argv[1] == '-' ){
	    acount++ ;
	    for( ptr = ++argv[1]; *ptr; ptr++ ){
		switch( *ptr ){
		case 'd':
		    debug = TRUE ;
		    break ;
		case 'c':
		    colormap = TRUE ;
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
	    YdebugMemory( debug ) ;

	    YinitProgram( "draw", VERSION, yaleIntro ) ;

	    /* now tell the user what he picked */
	    M(MSG,NULL,"\n\ndraw switches:\n" ) ;
	    if( debug ){
		YsetDebug( TRUE ) ;
		M(MSG,NULL,"\tdebug on\n" ) ;
	    } 
	    if( colormap ){
		M(MSG,NULL,"\tGiven colormap will be used.\n" ) ;
		colormapfile = argv[acount++] ;

	    } 
	    if( parasite ){
		M(MSG,NULL,"\tdraw will inherit window\n" ) ;
		/* look for windowid */
		if( acount < argc ){
		    windowId = atoi( argv[acount] ) ;
		} else {
		    M(ERRMSG,"main","Need to specify windowID\n" ) ;
		    syntax() ;
		} 
	    }
	    M(MSG,NULL,"\n" ) ;
	} else {
	    syntax() ;
	}
    }
/* ********************** end initialization ************************* */
    if(!(dirName = Ygetenv( "DATADIR" ))){
	/* find fullpathname of data directory */
	dirName = Yfixpath( "./DATA", FALSE ) ;
    }
    if( !(YdirectoryExists(dirName)) ){
	fprintf(stderr,"Can't find data directory:%s\n", dirName );
	YexitPgm( NODIRECTORY ) ;
    }

    /* initialize the names of the file prefixes */
    sprintf( fileName, "%s/cell.bin", dirName ) ;
    cellFileG = Ystrclone( fileName ) ;
    sprintf( fileName, "%s/symb.bin", dirName ) ;
    symbFileG = Ystrclone( fileName ) ;
    sprintf( fileName, "%s/net.bin", dirName ) ;
    netFileG = Ystrclone( fileName ) ;

    /* find max number of frames of data */ 
    for( maxCountG=1;;maxCountG++ ){

	sprintf(fileName,"%s.%d",cellFileG,maxCountG ) ;
	if(! (YfileExists(fileName) )){
	    maxCountG-- ; /* last file successfully read is one less */
	    break ;
	}
    }

    if( maxCountG == 0 ){/* we didnt find any file in correct format */
	fprintf(stderr,"Can't find data file:%s\n", fileName );
	YexitPgm( NOFILES ) ;
    }
    /* else continue onward */

#ifndef NOGRAPHICS
    /* open X display */
    /* we need to find host for display */
    if(!(host = Ygetenv("DISPLAY"))) {
	M(ERRMSG,"main","Can't get environment variable ");
	M(MSG,NULL, "for display.  Aborting graphics...\n\n" ) ;
	YexitPgm( PGMFAIL ) ;
    }

    /* first set colors */
    if( colormap ){
	/* look for colortable given by user */
	if( fp = TWOPEN( colormapfile,"r", ABORT )){
	    sprintf( YmsgG,
	    "Color table found - using defined colors in table\n\n" ) ;
	    M( MSG, NULL, YmsgG ) ;
	    colors = process_color_map( fp, &numcolors ) ;
	} 
    } else {
	colors = TWstdcolors() ;
	numcolors = TWnumcolors() ;
    }

    /* next check for parasite mode */
    if( windowId ){
	/* init windows as a parasite */
	if( !( TWinitParasite(argc,argv,numcolors,colors,
	    FALSE, MENU, draw_data,windowId ))){
	    M(ERRMSG,"main","Aborting graphics.");
	    YexitPgm( PGMFAIL ) ;
	}
    } else {
	/* init window as a master */
	if(!(TWinitGraphics(argc,argv,numcolors,colors,
	    FALSE, MENU, draw_data ))){
	    M(ERRMSG,"main","Aborting graphics.");
	    YexitPgm( PGMFAIL ) ;
	}
    }
    if( colormap ){
	/* now we can free color data */
	for( i=0;i<=numcolors;i++ ){
	    if( colors[i] ){
		Ysafe_free( colors[i] ) ;
	    }
	}
	Ysafe_free( colors ) ;
    }

    /* initalize cell data to NULL so we can tell when to free */
    cell_file_ptrG = NULL ; 
    net_file_ptrG = NULL ; 
    lookupLabelG = NULL ;

    window_manager() ;
    renumber_frames() ;

    TWcloseGraphics() ;
#endif
    YexitPgm(PGMOK);

} /* end main */

yaleIntro() 
{
    /* write name of program - stored in msg */
    printf("%s",YmsgG ) ;
    printf("     Yale University\n\n");

} /* end yaleIntro */

/* give user correct syntax */
syntax()
{
   M(ERRMSG,NULL,"\n" ) ; 
   M(MSG,NULL,"Incorrect syntax.  Correct syntax:\n");
   sprintf( YmsgG, 
       "\ndraw [-dcw] [colormap] [windowId] \n" ) ;
   M(MSG,NULL,YmsgG ) ; 
   M(MSG,NULL,"\twhose options are zero or more of the following:\n");
   M(MSG,NULL,"\t\td - prints debug info and performs extensive\n");
   M(MSG,NULL,"\t\t    error checking\n");
   M(MSG,NULL,"\t\tc - substitute colormap file for colors.\n");
   M(MSG,NULL,"\t\t    Default is to use TW standard colors.\n");
   M(MSG,NULL,"\t\tw - parasite mode - user must specify windowId\n");
   YexitPgm(PGMFAIL);
} /* end syntax */
