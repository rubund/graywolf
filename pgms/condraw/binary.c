/* ----------------------------------------------------------------- 
FILE:	    binary.c                                       
DESCRIPTION:Convert ASCII to binary representation for graphics routines.
CONTENTS:   
DATE:	    Jan 25, 1989 
REVISIONS:  
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) binary.c version 1.3 10/14/90" ;
#endif

#include <yalecad/base.h>
#include <yalecad/file.h>
#include <yalecad/message.h>
#include <yalecad/string.h>
#include <yalecad/dbinary.h>
#include <yalecad/wgraphics.h>
#include <yalecad/colors.h>
#include <yalecad/debug.h>

static int numcolorS ;
static char **colorS ;

convert_to_binary( cellF, netF )
FILE *cellF, *netF ;
{
    char **tokens ;
    char buffer[LRECL], *bufferptr = buffer ;
    char *color ;
    char *label ;
    int  numtokens, i ;
    int  cellNum ;
    int  x1, x2, y1, y2 ;
    int  net, pin ;
    int  cnum ;

    colorS = TWstdcolors() ;
    numcolorS = TWnumcolors() ;

    TWstartFrame() ;

    /* process the cell file */
    cellNum = 0 ;
    while( bufferptr=fgets(buffer,LRECL,cellF )){
	/* parse file */
	tokens = Ystrparser( bufferptr, ",:\t\n", &numtokens );

	ASSERTNCONT( numtokens >= 6 && numtokens <= 8 , 
	    "convert_to_binary",
	    "incorrect number of tokens in cell file" ) ;
	    
	x1 = atoi( Yremove_lblanks( tokens[0] ) ) ;
	y1 = atoi( Yremove_lblanks( tokens[1] ) ) ;
	x2 = atoi( Yremove_lblanks( tokens[2] ) ) ;
	y2 = atoi( Yremove_lblanks( tokens[3] ) ) ;
	color = Yremove_lblanks( tokens[5] ) ;
	cnum = getcolor( color ) ;
	if( numtokens == 8 ){
	    label = Yremove_lblanks( tokens[7] ) ;
	    TWdrawCell( ++cellNum, x1,y1,x2,y2,cnum,label) ;
	} else {
	    /* no label given */
	    TWdrawCell( ++cellNum, x1,y1,x2,y2,cnum,NULL) ;
	}
    }

    /* process the net file */
    while( bufferptr=fgets(buffer,LRECL,netF )){
	/* parse file */
	tokens = Ystrparser( bufferptr, ",:\t\n", &numtokens );

	ASSERTNCONT( numtokens >= 6 && numtokens <= 9 , 
	    "convert_to_binary",
	    "incorrect number of tokens in cell file" ) ;
	    
	net = atoi( Yremove_lblanks( tokens[0] ) ) ;
	x1 = atoi( Yremove_lblanks( tokens[1] ) ) ;
	y1 = atoi( Yremove_lblanks( tokens[2] ) ) ;
	x2 = atoi( Yremove_lblanks( tokens[3] ) ) ;
	y2 = atoi( Yremove_lblanks( tokens[4] ) ) ;
	color = Yremove_lblanks( tokens[6] ) ;
	cnum = getcolor( color ) ;
	if( numtokens == 9 ){
	    label = Yremove_lblanks( tokens[8] ) ;
	    TWdrawNet( net,x1,y1,x2,y2,cnum,label) ;
	} else {
	    /* no label given */
	    TWdrawNet( net, x1,y1,x2,y2,cnum,NULL) ;
	}
    }

}/* end  convert_to_binary */



static int getcolor( color ) 
char *color ;
{
    int i ;

    for( i = 1; i <= numcolorS ; i++ ){
	if( strcmp( colorS[i], color) == STRINGEQ){
	    return( i ) ;
	} 
    }
    return( TWBLACK ) ;
		
} /* end color function */
