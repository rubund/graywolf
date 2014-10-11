/* ----------------------------------------------------------------- 
FILE:	    rmain.c
DESCRIPTION:This file now contain calls the global router mickey.
CONTENTS:   
DATE:	    Feb 28, 1990 
REVISIONS:  Sun Dec 16 00:36:43 EST 1990 - Modified for Dahe's
		new version of the global router.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) rmain.c version 3.6 2/23/91" ;
#endif

#include <yalecad/base.h>
#include <yalecad/debug.h>
#include <yalecad/file.h>
#include <yalecad/string.h>
#include <custom.h>

#include "config-build.h"

#define GROUTEPROG      "Mickey"
#define GROUTEPATH      "../Mickey"


rmain( constraint_flag )
BOOL constraint_flag ;
{
    INT closegraphics() ;
    char filename[LRECL] ;
    char *Yrelpath() ;
    char *pathname ;
    char *twdir ;       /* path of TimberWolf directory */
    char *getenv() ;    /* used to get TWDIR environment variable */
    FILE *fp ;

    /* first build .gpar file for global router */
    sprintf( filename, "%s.gpar", cktNameG ) ;
    fp = TWOPEN( filename, "w", ABORT ) ;
    switch( get_circuit_type() ){
    case NO_CKT_TYPE:
    case DIGITAL:
	fprintf( fp, "digital.circuit\n" ) ;
	break ;
    case ANALOG:
	fprintf( fp, "analog.circuit\n" ) ;
	break ;
    case MIXED:
	fprintf( fp, "mixed.circuit\n" ) ;
	break ;
    }
    fprintf( fp, "limit_iteration 99999\n" ) ;
    if( constraint_flag ){
	fprintf( fp, "objective 2\n" ) ;
    } else {
	fprintf( fp, "objective 1\n" ) ;
    }
    TWCLOSE( fp ) ;

    /* now call the global router */
    /* find the path of compactor relative to main program */
    pathname = Yrelpath( argv0G, GROUTEPATH ) ;
    if( !(YfileExists(pathname))){
	if( twdir = TWFLOWDIR ){
	    sprintf( filename, "%s/bin/%s", twdir, GROUTEPROG ) ;
	    pathname = Ystrclone( filename ) ;
	}
    }
    sprintf( YmsgG, "%s %s", pathname, cktNameG ) ;
    M( MSG, NULL, YmsgG ) ;
    M( MSG, NULL, "\n" ) ;
    /* Ysystem will kill program if catastrophe occurred */
    Ysystem( GROUTEPROG, ABORT, YmsgG, closegraphics ) ;
    Ysafe_free( pathname ) ; /* free name created in Yrelpath */
    /* ############# end of gengraph execution ############# */

} /* end gmain */
