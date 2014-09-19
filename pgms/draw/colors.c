/* ----------------------------------------------------------------- 
FILE:	    color.c                                       
DESCRIPTION:Processes colormap.
CONTENTS:   char **process_color_map( fp, numcolors )
	    FILE *fp ;
	    int  *numcolors ;
DATE:	    Jan 25, 1989 - major rewrite of version 1.0
REVISIONS:  
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) colors.c version 3.4 9/7/90" ;
#endif

#include	<stdio.h>

#include	"globals.h"
#include	<yalecad/base.h>
#include	<yalecad/debug.h>
#include	<yalecad/string.h>
#include	<yalecad/message.h>

#define  MAXCOLORS 256

char **process_color_map( fp, numcolors )
FILE *fp ;
int  *numcolors ;
{
    char buffer[LRECL], *bufferptr ;
    char **tokens ;     /* for parsing menu file */
    char **colors ;
    int  line ;
    int  colornum ;
    INT  numtokens ;

    colors = (char **) Ysafe_calloc( MAXCOLORS, sizeof(char *)) ;
    line = 0 ;
    while( bufferptr=fgets(buffer,LRECL,fp )){
	/* parse file */
	line ++ ; /* increment line number */
	tokens = Ystrparser( bufferptr, ":\t\n", &numtokens );

	if( numtokens != 2 ){
	    sprintf( YmsgG, "Problem on line %d\n", line ) ; 
	    M( ERRMSG, "process_color_map", YmsgG ) ;
	    continue ;
	} else {
	    colornum = atoi( tokens[0] ) ;
	    if( colornum >= 0 && colornum <= 255 && colornum == line ){
		colors[colornum] = Ystrclone( tokens[1] ) ;
	    } else {
		sprintf( YmsgG, "Problem on line %d\n", line ) ; 
		M( ERRMSG, "process_color_map", YmsgG ) ;
	    }
	}
    }
    *numcolors = colornum ;
    return( colors ) ;

} /* end process_color_map */
