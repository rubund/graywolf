/* ----------------------------------------------------------------- 
FILE:	    main.c                                       
DESCRIPTION:Main routine for convert draw program.  Converts between
		old human readable format and new binary format.
CONTENTS:   main( argc, argv )
		int	argc;
		char *argv[];
DATE:	    Jan 26, 1989 - original coding WPS
REVISIONS:  Sat Nov 23 21:29:50 EST 1991 - now also output mif
		file for FrameMaker.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) main.c version 1.7 5/8/92" ;
#endif

#define  MAIN_DEFS
#include	<signal.h>
#include	<stdio.h>
#include	<yalecad/base.h>
#include	<yalecad/cleanup.h>
#include	<yalecad/debug.h>
#include	<yalecad/file.h>
#include	<yalecad/wgraphics.h>
#include	<yalecad/message.h>
#include	<yalecad/program.h>


#define NODIRECTORY 1

main(argc,argv)
int	argc;
char	*argv[];
{
    char fileName[LRECL] ;
    BOOL frameGiven ;
    BOOL asciiGiven ;
    BOOL mif ;
    BOOL fill ;
    int i ;
    int frameNum ;
    char *ptr ;
    char  *dirPath, *Yfixpath() ;
    char *Ygetenv() ;
    char  cellFileA[LRECL] ;
    char  cellFileB[LRECL] ;
    char  netFileA[LRECL] ;
    char  netFileB[LRECL] ;
    char  symbFileB[LRECL] ;
    char  mifFile[LRECL] ;
    FILE  *cellIn ;
    FILE  *cellOut ;
    FILE  *netIn ;
    FILE  *netOut ;
    FILE  *symb ;
    FILE  *mifOut ;


    /* start up cleanup handler */
    YINITCLEANUP( argv[0], NULL, NODUMP ) ;

    YdebugMemory( FALSE ) ;

    fprintf( stdout, "\n%s\n", YinitProgram("condraw","v1.0",NULL) ) ;

    if( argc < 2 ){
       M(ERRMSG,"condraw","Incorrect syntax.  Correct syntax:\n");
       M(MSG,NULL,"condraw [-rfm] [frameNum] [dataDirectoryPath]\n ");
       M(MSG,NULL,"\twhose options is one or more of the following:\n");
       M(MSG,NULL,"\t\tm - binary-->MIF format (FrameMaker 3.0+)\n");
       M(MSG,NULL,"\t\t    use version 6 for FrameMaker 2.0\n");
       M(MSG,NULL,"\t\tr - ASCII-->binaryformat\n");
       M(MSG,NULL,"\t\t    default - binaryformat-->ASCII\n");
       M(MSG,NULL,"\t\tf - convert starting at a specific frame no.\n");
       M(MSG,NULL,"\t\t    default - frame number 1.\n");
       M(MSG,NULL,"\t\tNote:if dataDirectorPath is not specified,\n");
       M(MSG,NULL,"\t\tthe environment variable DATADIR will be used\n");
       M(MSG,NULL,"Example: condraw -rf 3 ~bills/test/DATA\n\n");
       YexitPgm(PGMFAIL);

    } 
    i = 0 ;
    asciiGiven = FALSE ;
    frameGiven = FALSE ;
    fill = TRUE ;
    mif = FALSE ;
    if( *argv[1] == '-' ){
	for( ptr = ++argv[++i]; *ptr; ptr++ ){
	    switch( *ptr ){
	    case 'd':
		YsetDebug( TRUE ) ;
		break ;
	    case 'f':
		frameGiven = TRUE ;
		break ;
	    case 'm':
		mif = TRUE ;
		break ;
	    case 'o':
		fill = FALSE ;
		break ;
	    case 'r':
		asciiGiven = TRUE ;
		break ;
	    default:
		sprintf( YmsgG,"Unknown option\n" ) ;
		M(ERRMSG,"main",YmsgG);
		YexitPgm(PGMFAIL);
	    }
	}
    }
    if( frameGiven ){
	frameNum = atoi(argv[++i]);
	if( frameNum == 0 ){
	    sprintf( YmsgG,"Frame number invalid\n" ) ;
	    M(ERRMSG,"main",YmsgG);
	    YexitPgm(PGMFAIL);
	}
    } else {
	frameNum = 1 ;
    }

    /* find fullpathname */
    if( argc > ++i ){
	strcpy( fileName, argv[++i] ) ;
	dirPath = Yfixpath( fileName, FALSE ) ;
    } else {
	if(!(dirPath = Ygetenv( "DATADIR" ))){
	    M( ERRMSG, "main", 
	    "Couldn't get DATADIR environment variable.  Must abort.\n" ) ;
	    YexitPgm( PGMFAIL ) ;
	}
    }

    /* first look for data directory */
    if( !(YdirectoryExists(dirPath)) ){
	fprintf(stderr,"Can't find data directory:%s\n", dirPath );
	YexitPgm( NODIRECTORY ) ;
    } else {
	fprintf(stderr,"Directory path:%s\n", dirPath );
    }

    if( asciiGiven ){
	TWinitGraphics( NULL,TWstdcolors(),TWnumcolors(),1,dirPath,NULL);
    }
    for( ;frameNum; frameNum++ ){

	/* initialize the names of the file prefixes */
	/* A suffix ASCII B suffix binary */
	sprintf( cellFileA, "%s\/cell.file.%d", dirPath, frameNum ) ;
	sprintf( cellFileB, "%s\/cell.bin.%d",  dirPath, frameNum ) ;
	sprintf( netFileA,  "%s\/net.file.%d",  dirPath, frameNum ) ;
	sprintf( netFileB,  "%s\/net.bin.%d",   dirPath, frameNum ) ;
	sprintf( symbFileB, "%s\/symb.bin.%d",  dirPath, frameNum ) ;
	sprintf( mifFile,   "%s\/tw%d.mif",     dirPath, frameNum ) ;

	/* open the files */
	if( asciiGiven ){
	    cellIn  = YopenFile( cellFileA, "r", NOABORT ) ;
	    netIn  = YopenFile( netFileA, "r", NOABORT ) ;
	    if( !(cellIn) || !(netIn) ){
		/* can't do any more work */
		break ;
	    }
	    TWsetWFrame( frameNum - 1 ) ;
	    convert_to_binary( cellIn, netIn ) ;
	    fclose( cellIn ) ;
	    fclose( netIn ) ;
	} else {
	    cellIn  = YopenFile( cellFileB, "r", NOABORT ) ;
	    netIn  = YopenFile( netFileB, "r", NOABORT ) ;
	    symb   = YopenFile( symbFileB, "r", NOABORT ) ;
	    if( !(cellIn) || !(netIn) || !(symb) ){
		/* can't do any more work */
		break ;
	    }
	    if( mif ){
		mifOut = YopenFile( mifFile, "w", ABORT ) ;
		convert_to_mif( cellIn, netIn, symb, mifOut, fill ) ;
		fclose( mifOut ) ;
	    } else {
		cellOut = YopenFile( cellFileA, "w", ABORT ) ;
		netOut = YopenFile( netFileA, "w", ABORT ) ;
		convert_to_ascii( cellIn, cellOut, netIn, netOut, symb ) ;
		fclose( cellOut ) ;
		fclose( netOut ) ;
	    }
	    fclose( symb ) ;
	    fclose( cellIn ) ;
	    fclose( netIn ) ;
	}
    }
    if( asciiGiven ){
	TWcloseGraphics() ;
    }
    YexitPgm( 0 ) ;

}
