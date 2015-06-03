/*
 *   Copyright (C) 1990 Yale University
 *
 *   This work is distributed in the hope that it will be useful; you can
 *   redistribute it and/or modify it under the terms of the
 *   GNU General Public License as published by the Free Software Foundation;
 *   either version 2 of the License,
 *   or any later version, on the following conditions:
 *
 *   (a) YALE MAKES NO, AND EXPRESSLY DISCLAIMS
 *   ALL, REPRESENTATIONS OR WARRANTIES THAT THE MANUFACTURE, USE, PRACTICE,
 *   SALE OR
 *   OTHER DISPOSAL OF THE SOFTWARE DOES NOT OR WILL NOT INFRINGE UPON ANY
 *   PATENT OR
 *   OTHER RIGHTS NOT VESTED IN YALE.
 *
 *   (b) YALE MAKES NO, AND EXPRESSLY DISCLAIMS ALL, REPRESENTATIONS AND
 *   WARRANTIES
 *   WHATSOEVER WITH RESPECT TO THE SOFTWARE, EITHER EXPRESS OR IMPLIED,
 *   INCLUDING,
 *   BUT NOT LIMITED TO, WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A
 *   PARTICULAR
 *   PURPOSE.
 *
 *   (c) LICENSEE SHALL MAKE NO STATEMENTS, REPRESENTATION OR WARRANTIES
 *   WHATSOEVER TO
 *   ANY THIRD PARTIES THAT ARE INCONSISTENT WITH THE DISCLAIMERS BY YALE IN
 *   ARTICLE
 *   (a) AND (b) above.
 *
 *   (d) IN NO EVENT SHALL YALE, OR ITS TRUSTEES, DIRECTORS, OFFICERS,
 *   EMPLOYEES AND
 *   AFFILIATES BE LIABLE FOR DAMAGES OF ANY KIND, INCLUDING ECONOMIC DAMAGE OR
 *   INJURY TO PROPERTY AND LOST PROFITS, REGARDLESS OF WHETHER YALE SHALL BE
 *   ADVISED, SHALL HAVE OTHER REASON TO KNOW, OR IN FACT SHALL KNOW OF THE
 *   POSSIBILITY OF THE FOREGOING.
 *
 */

#ifndef lint
static char SccsId[] = "@(#) findloc.c version 3.3 9/5/90" ;
#endif

#include <custom.h>
#include <yalecad/debug.h>

typedef struct flogbox {
    INT pin ;
    INT placed ;
    INT finalx ;
    INT finaly ;
}
FBOX ,
*FBOXPTR ;



/*   
 *   We have here a little function which 
 *   takes as an argument the given UCpin and returns
 *   the best possible location in the site.
 */

findLoc( pinptr, lArray, nPinLocs, HorV ) 
PINBOXPTR pinptr ;
BOOL HorV ;
INT nPinLocs ;
FBOXPTR lArray ;
{

INT bigoX , bigoY , litoX , litoY ;
INT aPin, bestPos , meanx , meany , distant , i ;

bigoX = INT_MIN ;
bigoY = INT_MIN ;
litoX = INT_MAX ;
litoY = INT_MAX ;

aPin = pinptr->pin ; /* same pin */
for( pinptr = netarrayG[pinptr->net]->pins;pinptr;pinptr = pinptr->next ){
    if( pinptr->pin != aPin ) {
	if( pinptr->xpos < litoX ) {
	    litoX = pinptr->xpos ;
	} 
	if( pinptr->xpos > bigoX ) {
	    bigoX = pinptr->xpos ;
	}
	if( pinptr->ypos < litoY ) {
	    litoY = pinptr->ypos ;
	} 
	if( pinptr->ypos > bigoY ) {
	    bigoY = pinptr->ypos ;
	}
    }
}
if( HorV ) {
    if( lArray[ nPinLocs ].finaly > lArray[1].finaly ) {
	if( litoY >= lArray[ nPinLocs ].finaly ) {
	    bestPos = nPinLocs ;
	} else if( bigoY <= lArray[1].finaly ) {
	    bestPos = 1 ;
	} else if( bigoY <= lArray[ nPinLocs ].finaly &&
				   litoY >= lArray[1].finaly ) {
		/*
		 *   bounding box of net (sans aPin) is
		 *   within span of site.  Find nearest loc
		 *   (in the site) to the center of b.b.
		 */
	    meany = (bigoY + litoY) / 2 ;
	    distant = ABS( meany - lArray[1].finaly ) ;
	    for( i = 2 ; i <= nPinLocs ; i++ ) {
		if( ABS( meany - lArray[i].finaly ) < distant ) {
		    distant = ABS(meany - lArray[i].finaly);
		} else {
		    break ;
		}
	    }
	    bestPos = i - 1 ;
	} else if( bigoY >= lArray[ nPinLocs ].finaly &&
				   litoY <= lArray[1].finaly ) {
		/*
		 *   Site is spanned (contained) by the b.b.
		 *   Select middle loc.
		 */
	    bestPos = (nPinLocs + 1) / 2 ;
	} else {  /*  only some overlapping exists  */
		/*
		 *   Find edge of b.b. inside site.
		 */
	    if( litoY < lArray[ nPinLocs ].finaly &&
				litoY > lArray[ 1 ].finaly ) {
		    /*
		     *   Find nearest loc to litoY
		     */
		distant = ABS( litoY - lArray[1].finaly ) ;
		for( i = 2 ; i <= nPinLocs ; i++ ) {
		    if( ABS( litoY - lArray[i].finaly ) < distant ){
			distant = ABS(litoY - lArray[i].finaly) ;
		    } else {
			break ;
		    }
		}
		bestPos = ( i + nPinLocs ) / 2 ;
	    } else {
		    /*
		     *   Find nearest loc to bigoY
		     */
		distant = ABS( bigoY - lArray[1].finaly ) ;
		for( i = 2 ; i <= nPinLocs ; i++ ) {
		    if( ABS( bigoY - lArray[i].finaly ) < distant ){
			distant = ABS(bigoY - lArray[i].finaly) ;
		    } else {
			break ;
		    }
		}
		bestPos = i / 2 ;
	    }
	}
    } else {
	if( litoY >= lArray[1].finaly ) {
	    bestPos = 1 ;
	} else if( bigoY <= lArray[ nPinLocs ].finaly ) {
	    bestPos = nPinLocs ;
	} else if( bigoY <= lArray[1].finaly &&
			     litoY >= lArray[ nPinLocs ].finaly ) {
		/*
		 *   bounding box of net (sans aPin) is
		 *   within span of site.  Find nearest loc
		 *   (in the site) to the center of b.b.
		 */
	    meany = (bigoY + litoY) / 2 ;
	    distant = ABS( meany - lArray[1].finaly ) ;
	    for( i = 2 ; i <= nPinLocs ; i++ ) {
		if( ABS( meany - lArray[i].finaly ) < distant ) {
		    distant = ABS(meany - lArray[i].finaly);
		} else {
		    break ;
		}
	    }
	    bestPos = i - 1 ;
	} else if( bigoY >= lArray[1].finaly &&
			      litoY <= lArray[ nPinLocs ].finaly ) {
		/*
		 *   Site is spanned (contained) by the b.b.
		 *   Select middle loc.
		 */
	    bestPos = (nPinLocs + 1) / 2 ;
	} else {  /*  only some overlapping exists  */
		/*
		 *   Find edge of b.b. inside site.
		 */
	    if( litoY < lArray[1].finaly &&
			    litoY > lArray[ nPinLocs ].finaly ) {
		    /*
		     *   Find nearest loc to litoY
		     */
		distant = ABS( litoY - lArray[1].finaly ) ;
		for( i = 2 ; i <= nPinLocs ; i++ ) {
		    if( ABS( litoY - lArray[i].finaly ) < distant ){
			distant = ABS(litoY - lArray[i].finaly) ;
		    } else {
			break ;
		    }
		}
		bestPos = i / 2 ;
	    } else {
		    /*
		     *   Find nearest loc to bigoY
		     */
		distant = ABS( bigoY - lArray[1].finaly ) ;
		for( i = 2 ; i <= nPinLocs ; i++ ) {
		    if( ABS( bigoY - lArray[i].finaly ) < distant ){
			distant = ABS(bigoY - lArray[i].finaly) ;
		    } else {
			break ;
		    }
		}
		bestPos = ( i + nPinLocs ) / 2 ;
	    }
	}
    }
} else {  /*  a horizontal site  */
    if( lArray[ nPinLocs ].finalx > lArray[1].finalx ) {
	if( litoX >= lArray[ nPinLocs ].finalx ) {
	    bestPos = nPinLocs ;
	} else if( bigoX <= lArray[1].finalx ) {
	    bestPos = 1 ;
	} else if( bigoX <= lArray[ nPinLocs ].finalx &&
				   litoX >= lArray[1].finalx ) {
		/*
		 *   bounding box of net (sans aPin) is
		 *   within span of site.  Find nearest loc
		 *   (in the site) to the center of b.b.
		 */
	    meanx = (bigoX + litoX) / 2 ;
	    distant = ABS( meanx - lArray[1].finalx ) ;
	    for( i = 2 ; i <= nPinLocs ; i++ ) {
		if( ABS( meanx - lArray[i].finalx ) < distant ) {
		    distant = ABS(meanx - lArray[i].finalx);
		} else {
		    break ;
		}
	    }
	    bestPos = i - 1 ;
	} else if( bigoX >= lArray[ nPinLocs ].finalx &&
				   litoX <= lArray[1].finalx ) {
		/*
		 *   Site is spanned (contained) by the b.b.
		 *   Select middle loc.
		 */
	    bestPos = (nPinLocs + 1) / 2 ;
	} else {  /*  only some overlapping exists  */
		/*
		 *   Find edge of b.b. inside site.
		 */
	    if( litoX < lArray[ nPinLocs ].finalx &&
				litoX > lArray[ 1 ].finalx ) {
		    /*
		     *   Find nearest loc to litoX
		     */
		distant = ABS( litoX - lArray[1].finalx ) ;
		for( i = 2 ; i <= nPinLocs ; i++ ) {
		    if( ABS( litoX - lArray[i].finalx ) < distant ){
			distant = ABS(litoX - lArray[i].finalx) ;
		    } else {
			break ;
		    }
		}
		bestPos = ( i + nPinLocs ) / 2 ;
	    } else {
		    /*
		     *   Find nearest loc to bigoX
		     */
		distant = ABS( bigoX - lArray[1].finalx ) ;
		for( i = 2 ; i <= nPinLocs ; i++ ) {
		    if( ABS( bigoX - lArray[i].finalx ) < distant ){
			distant = ABS(bigoX - lArray[i].finalx) ;
		    } else {
			break ;
		    }
		}
		bestPos = i / 2 ;
	    }
	}
    } else {
	if( litoX >= lArray[1].finalx ) {
	    bestPos = 1 ;
	} else if( bigoX <= lArray[ nPinLocs ].finalx ) {
	    bestPos = nPinLocs ;
	} else if( bigoX <= lArray[1].finalx &&
			     litoX >= lArray[ nPinLocs ].finalx ) {
		/*
		 *   bounding box of net (sans aPin) is
		 *   within span of site.  Find nearest loc
		 *   (in the site) to the center of b.b.
		 */
	    meanx = (bigoX + litoX) / 2 ;
	    distant = ABS( meanx - lArray[1].finalx ) ;
	    for( i = 2 ; i <= nPinLocs ; i++ ) {
		if( ABS( meanx - lArray[i].finalx ) < distant ) {
		    distant = ABS(meanx - lArray[i].finalx);
		} else {
		    break ;
		}
	    }
	    bestPos = i - 1 ;
	} else if( bigoX >= lArray[1].finalx &&
			      litoX <= lArray[ nPinLocs ].finalx ) {
		/*
		 *   Site is spanned (contained) by the b.b.
		 *   Select middle loc.
		 */
	    bestPos = (nPinLocs + 1) / 2 ;
	} else {  /*  only some overlapping exists  */
		/*
		 *   Find edge of b.b. inside site.
		 */
	    if( litoX < lArray[1].finalx &&
			    litoX > lArray[ nPinLocs ].finalx ) {
		    /*
		     *   Find nearest loc to litoX
		     */
		distant = ABS( litoX - lArray[1].finalx ) ;
		for( i = 2 ; i <= nPinLocs ; i++ ) {
		    if( ABS( litoX - lArray[i].finalx ) < distant ){
			distant = ABS(litoX - lArray[i].finalx) ;
		    } else {
			break ;
		    }
		}
		bestPos = i / 2 ;
	    } else {
		    /*
		     *   Find nearest loc to bigoX
		     */
		distant = ABS( bigoX - lArray[1].finalx ) ;
		for( i = 2 ; i <= nPinLocs ; i++ ) {
		    if( ABS( bigoX - lArray[i].finalx ) < distant ){
			distant = ABS(bigoX - lArray[i].finalx) ;
		    } else {
			break ;
		    }
		}
		bestPos = ( i + nPinLocs ) / 2 ;
	    }
	}
    }
}
return( bestPos ) ;
}  /* end of findLoc */
