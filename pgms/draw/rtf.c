/* rtf.c */
#include	<stdio.h>
#define         TWDRAWCODE

#ifndef NOGRAPHICS

#include	<yalecad/base.h>
#include        <yalecad/draw.h>
#include        <yalecad/colors.h>
#include	"globals.h"

#ifndef lint
static char SccsId[] = "@(#) rtf.c version 3.7 8/8/91" ;
#endif


static  BOOL     drawNetS = TRUE ;
static  BOOL     drawLabelS = FALSE ;

/* reduced to fit takes data in data structures and writes to window */
draw_data()
{
    int     i ;
    char    *labelptr ;
    DATABOX ccell, pcell, ncell ;

    TWstartFrame() ;
    TWmessage( "Drawing data.  Please wait..." ) ;
    for( i=0;i<num_cel_recG;i++ ){
	ccell = cell_file_ptrG[i] ;
	if( drawLabelS ){
	    labelptr = lookupLabelG + ccell.label ;
	} else {
	    labelptr = NULL ;
	}
	TWdrawCell(ccell.ref,ccell.x1, ccell.y1, ccell.x2, ccell.y2,
	    ccell.color, labelptr);

    }
    if( drawNetS ){

	for( i=0;i<num_net_recG;i++ ){
	    ncell = net_file_ptrG[i] ;
	    if( drawLabelS ){
		labelptr = lookupLabelG + ncell.label ;
	    } else {
		labelptr = NULL ;
	    }
	    TWdrawLine(ncell.ref,ncell.x1, ncell.y1, ncell.x2, ncell.y2,
		ncell.color,labelptr);
	}
    }

    TWmessage( NULL ) ;
    TWflushFrame() ;
} /* end draw_data */

dump_cif( design )
char *design ;
{
    int     i ;
    DATABOX ccell, pcell, ncell ;

    YcifInit( design ) ;
    for( i=0;i<num_cel_recG;i++ ){
	ccell = cell_file_ptrG[i] ;
	YcifBox( ccell.x1, ccell.y1, ccell.x2, ccell.y2, ccell.color);
    }
    if( drawNetS ){

	for( i=0;i<num_net_recG;i++ ){
	    ncell = net_file_ptrG[i] ;
	    YcifLine( ncell.x1,ncell.y1,ncell.x2,ncell.y2,ncell.color ) ;
	}
    }
    YcifClose() ;
} /* end dump_cif */


turnNetsOn( flag )
int flag ;
{
    drawNetS = flag ;
}
turnLabelsOn( flag )
int flag ;
{
    drawLabelS = flag ;
}

#endif /* NOGRAPHICS */
