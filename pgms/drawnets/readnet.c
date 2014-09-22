/* ----------------------------------------------------------------- 
FILE:	    readnet.c                                       
DESCRIPTION:Read the output of the circuitName.pth
CONTENTS: 
DATE:	    Apr 19, 1991 - Original coding.
REVISIONS:  
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) readnet.c (Yale) version 1.1 4/21/91" ;
#endif

#define NETKEYWORD     "net"

#include <yalecad/base.h>
#include <yalecad/string.h>
#include <yalecad/file.h>
#include <globals.h>

readnets( fin )
FILE *fin ;
{
    INT line ;
    INT numtokens ;
    char *bufferptr ;
    char buffer[LRECL] ;
    char **tokens ;


    line = 0 ;
    numnetsG = 0 ;
    while( bufferptr=fgets(buffer,LRECL,fin )){
	line++ ; 
	tokens = Ystrparser( bufferptr, " :\t\n", &numtokens ); 
	if( numtokens == 0 ){ 
	    /* skip over empty lines */ 
	    continue ; 
	} else if( strcmp( tokens[0], NETKEYWORD ) == STRINGEQ){ 
	    /* look at first field for menu keyword */ 
	    /* there better be only eleven tokens on this line */ 
	    if( numtokens != 11 && numtokens != 12 ){ 
		fprintf( stderr, "Syntax error on line:%d\n", line ) ;
		continue ;
	    } 
	    /* here is where we do processing */
	    if( numtokens == 11 ){
		numnetsG++ ;
	    }
	}
    }

    /* allocate space for nets */
    net_nameG = (char **) Yvector_alloc( 0, numnetsG, sizeof(char *) ) ;
    net_lengthG = (INT *) Yvector_alloc( 0, numnetsG, sizeof(INT) ) ;
    net_numpinsG = (INT *) Yvector_alloc( 0, numnetsG, sizeof(INT) ) ;
    lengthG = (INT *) Yvector_alloc( 0, numnetsG, sizeof(INT) ) ;

    /* reread */
    rewind( fin ) ;
    numnetsG = 0 ;
    while( bufferptr=fgets(buffer,LRECL,fin )){
	line++ ; 
	tokens = Ystrparser( bufferptr, " :\t\n", &numtokens ); 
	if( numtokens == 0 ){ 
	    /* skip over empty lines */ 
	    continue ; 
	} else if( strcmp( tokens[0], NETKEYWORD ) == STRINGEQ){ 
	    /* look at first field for menu keyword */ 
	    /* there better be only eleven tokens on this line */ 
	    if( numtokens != 11 ){ 
		continue ;
	    } 
	    /* here is where we do processing */
	    net_nameG[numnetsG] = Ystrclone( tokens[2] ) ;
	    net_lengthG[numnetsG] = atoi( tokens[8] ) ;
	    net_numpinsG[numnetsG] = atoi( tokens[10] ) ;
	    numnetsG++ ;
	}
    }
    TWCLOSE( fin ) ;


} /* end readnet */

