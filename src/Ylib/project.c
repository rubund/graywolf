/*----------------------------------------------------------------- 
FILE:	    project.c                                       
DESCRIPTION:Overlap and projection of rectangles.
CONTENTS:   
DATE:	    Tue Oct 29 15:02:21 EST 1991
REVISIONS:  
 ----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) project.c (Yale) version 1.1 11/20/91" ;
#endif

#include <yalecad/base.h>
#include <yalecad/project.h>

static INT xspaceS = 0 ;
static INT yspaceS = 0 ;

Yproject_space( xspace, yspace )
INT xspace, yspace ;
{
    xspaceS = xspace ;
    yspaceS = yspace ;
} /* end Yproject_space() */

/* function returns whether one cell projects onto another */
INT YprojectX( tile1_left, tile1_right, tile2_left, tile2_right )
INT tile1_left, tile1_right, tile2_left, tile2_right ;
{
    /* -----------------------------------------------------
       First check case 2nd tile larger than first 
	complete overlap
    */
    if( tile2_left <= tile1_left && tile1_right <= tile2_right ){
	return( OVERLAP1 ) ;

    /* -----------------------------------------------------
       Check if an edge of tile two is encompassed by tile 1 
       Second check left edge of tile2 :
	tile1_left <= tile2_left < tile1_right + xspaceS
    */
    } else if( tile1_left<=tile2_left&&tile2_left<tile1_right+xspaceS ){
	return( OVERLAP2 ) ;
    /* -----------------------------------------------------
       Third check right edge of tile2 :
	tile1_left - xspaceS < tile2_right < tile1_right 
    */
    } else if( tile1_left-xspaceS<tile2_right&&tile2_right<=tile1_right){
	return( OVERLAP3 ) ;

    /* -----------------------------------------------------
       Fourth case tiles touching.
    */
    } else if( tile2_left == tile1_right + xspaceS || 
	       tile1_left - xspaceS == tile2_right ){
	return( TOUCH ) ;

    } else {
	return( NOTOUCH ) ;
    }
} /* end YprojectX */

/* function returns whether one cell projects onto another */
INT YprojectY( tile1_bot, tile1_top, tile2_bot, tile2_top )
INT tile1_bot, tile1_top, tile2_bot, tile2_top ;
{
    /* -----------------------------------------------------
       First check to see if 2nd tile larger than first 
    */
    if( tile2_bot <= tile1_bot && tile1_top <= tile2_top ){
	return( OVERLAP1 ) ;

    /* -----------------------------------------------------
       Check if an edge of tile two is encompassed by tile 1 
       Second check bottom edge of tile2 :
	tile1_bot <= tile2_bot < tile1_top 
    */
    } else if( tile1_bot <= tile2_bot && tile2_bot < tile1_top+yspaceS){

	return( OVERLAP2 ) ;
    /* -----------------------------------------------------
       Third check top edge of tile2 :
	tile1_bot < tile2_top <= tile1_top 
    */
    } else if( tile1_bot-yspaceS < tile2_top && tile2_top <= tile1_top ){
	return( OVERLAP3 ) ;

    /* -----------------------------------------------------
       Fourth case tiles touching.
    */
    } else if( tile2_bot == tile1_top + yspaceS || 
	       tile1_bot - yspaceS == tile2_top ){
	return( TOUCH ) ;

    } else {
	return( NOTOUCH ) ; /* no touch or overlap */ 
    }
}/* end YprojectY */
