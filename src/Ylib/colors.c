/*
 *   Copyright (C) 1989-1992 Yale University
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

/* ----------------------------------------------------------------- 
FILE:	    color.c                                       
DESCRIPTION:These routines give the standard colors for TimberWolf
    draw system.  They can be overriden by creating a color matrix 
    similar to below.
CONTENTS:   char **TWstdcolors()
	    INT TWnumcolors() 
DATE:	    Jan 25, 1989 - major rewrite of version 1.0
REVISIONS:  Jan 31, 1989 - added screen routines.
	    Sep 25, 1990 - redid the stipple patterns for B&W.
	    Oct  5, 1990 - rest of the gang wanted more colors.
	    Fri Jan 18 18:38:36 PST 1991 - fixed to run on AIX.
	    Fri Jan 25 15:41:25 PST 1991 - fixed to run on HPUX.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) colors.c version 3.8 2/26/92" ;
#endif

#include <yalecad/base.h>

#define NUMCOLORS 18
static char *TWstdcolorArrayS[NUMCOLORS+1] = {
    "",           /* through 0 element away */
    "white",
    "black",
    "gray",
    "red",
    "blue",
    "yellow",
    "green",
    "magenta",
    "orange",
    "medium aquamarine",
    "cyan",
    "SpringGreen",
    "SteelBlue",
    "firebrick",
    "gold",
    "navy",
    "thistle",
    "khaki"
} ;

static char stipple_patS[30*8] = {
'\210','\042','\210','\042','\210','\042','\210','\042',
'\167','\335','\167','\335','\167','\335','\167','\335',
'\356','\167','\273','\335','\356','\167','\273','\335',
'\201','\102','\044','\030','\030','\044','\102','\201',
'\003','\201','\300','\140','\060','\030','\014','\006',
'\314','\210','\021','\063','\314','\210','\021','\063',
'\252','\125','\252','\125','\252','\125','\252','\125',
'\300','\201','\003','\006','\014','\030','\060','\140',
'\034','\203','\160','\016','\301','\300','\007','\340',
'\000','\000','\314','\314','\000','\000','\314','\314',
'\000','\000','\000','\377','\000','\000','\000','\377',
'\104','\104','\104','\104','\104','\104','\104','\104',
'\125','\252','\125','\252','\125','\252','\125','\252',
'\063','\063','\377','\377','\314','\314','\377','\377',
'\104','\210','\042','\021','\104','\210','\042','\021',
'\104','\356','\104','\000','\104','\356','\104','\000',
'\300','\014','\300','\014','\300','\014','\300','\014',
'\000','\000','\000','\000','\360','\360','\360','\360',
'\210','\000','\000','\000','\210','\000','\000','\000',
'\000','\021','\000','\104','\000','\021','\000','\104',
'\000','\042','\210','\104','\000','\042','\210','\104',
'\360','\360','\360','\360','\000','\000','\000','\000',
'\100','\040','\020','\010','\004','\002','\001','\200',
'\210','\063','\210','\146','\210','\063','\210','\146',
'\314','\352','\314','\042','\314','\356','\314','\042',
'\200','\000','\200','\000','\010','\000','\010','\000',
'\002','\004','\010','\020','\040','\100','\200','\001',
'\000','\012','\000','\000','\000','\240','\000','\000'
} ;

/* returns the standard color array */
char **TWstdcolors()
{
    return( TWstdcolorArrayS ) ;
} /* end TWstdcolors */

/* returns the number of colors in standard color array */
INT TWnumcolors()
{
    return( NUMCOLORS ) ;
} /* end TWnumcolors */


/* returns array of the standard stipple patterns */
char *TWstdStipple() 
{
    return( stipple_patS ) ;
} /* end TWstdStipple */
