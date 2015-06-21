/*
 *   Copyright (C) 1989-1991 Yale University
 *   Copyright (C) 2013 Tim Edwards <tim@opencircuitdesign.com>
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
FILE:	    configure.c                                       
DESCRIPTION:cell topology configuration routines.
CONTENTS:   configure() 
DATE:	    Mar 27, 1989 
REVISIONS:  Fri Jan 25 18:29:22 PST 1991 - fixed problem with
		different row heights.
	    Tue Mar 19 16:27:35 CST 1991 - fixed calculation
		problem with rowSep.
----------------------------------------------------------------- */
#ifndef VMS
#ifndef lint
static char SccsId[] = "@(#) configure.c (Yale) version 4.12 3/19/91" ;
#endif
#endif

#define CONFIG_VARS

#include "standard.h"
#include "config.h"
#include "parser.h"
#include "pads.h"
#include "main.h"
#include "readpar.h"
#include <yalecad/debug.h>

/*  
 *  Configure the rows of standard cells along with possibly
 *  one or more macro blocks;  The macro blocks may be 
 *  specified as: MUL,MUR,MLL,MLR, ML,MR,MB,MT, MM, MTT, or MBB
 */
#define MAXMACRO 50 

INT totalRG ;
BOOL doubleback_rows_start_at_oneG ;

extern INT individual_rowSepsG ;
extern INT total_row_lengthG ;
extern DOUBLE indentG ;
extern DOUBLE *relativeLenG ;
extern DOUBLE *rowSepsG ;
extern INT *rowSepsAbsG ;

/* static definitions */
static INT heightS ;
static INT widthS ;
static INT mttshiftS ;
static INT mbbshiftS ;


configure() 
{

INT block ;
INT pad ;
INT row ;
INT macro ;
INT totalBlock ;
DOUBLE ratio ;
BBOXPTR *oldbarray ;
BBOXPTR boxptr ;
DOUBLE totalF ;
DOUBLE modulation ;
DOUBLE constant ;
DOUBLE dsumS ;
INT topRowY , centerY ;
INT k ;
INT macNumber ;
INT trow ;
INT brow ;
INT rows ;
INT bot ;
INT left ;
INT edge ;
INT rite ;
INT top ;
INT padside ;
INT middleRow ;
INT URmacro , Rmacro ;
INT count_mtt , count_mbb ;
INT Mf ;
INT rowcenter ;
INT rowtop ;
INT l, r, b, t ;
INT xc, yc ;
CBOXPTR padptr ;
TIBOXPTR tptr ;

if (macroArrayG) {
    Ysafe_free(macroArrayG);
}
macroArrayG = (MACROBOX *) Ysafe_malloc((MAXMACRO + 1) * sizeof(MACROBOX));
macNumber = 0 ;
numRowsG = numRowsG ;
if (rowArrayG) {
    Ysafe_free(rowArrayG);
}
rowArrayG = (ROWBOX *) Ysafe_malloc( (numRowsG + 1) * sizeof( ROWBOX ) ) ;
for( row = 1 ; row <= numRowsG ; row++ ) {
    rowArrayG[row].endx1   = -1 ;
    rowArrayG[row].startx2 = -1 ;
}

/*
    Total cell length = totalRowLength = totalRG ;
    numRowsG is entered by the user ;
    rowSepG and rowSepAbsG are also entered by the user ;
    row center-to-center spacing = (rowSepG + 1) * rowHeight + rowSepAbsG ;
*/
totalRG = celllenG ;
rowHeightG = 0 ;
for( row = 1 ; row <= numRowsG ; row++ ) {
    rowHeightG += barrayG[row]->btop - barrayG[row]->bbottom ;
}
rowHeightG /= numRowsG ;

/* Let bottom = 0 and 
	    top = (numRowsG - 1) * ((rowSepG + 1) * rowHeightG + rowSepAbsG) + 
						rowHeightG / 2 ;

    Position of each row n is as follows:
       starts at x = 0 ;
       y = (n - 1) * ((rowSepG + 1) * rowHeightG + rowSepAbsG) + rowHeightG / 2 ;
*/

if( intelG ) {
    if( doubleback_rows_start_at_oneG ) {
    	for( row = 1 ; row <= numRowsG ; row++ ) {
            rowArrayG[row].startx = 0 ;
	    if( row % 2 == 1 ) {
		rowArrayG[row].ypos = (row / 2)
			* (rowSepG + 2) * rowHeightG + rowSepAbsG
			+ rowHeightG / 2 ;
	    } else {
		rowArrayG[row].ypos = ((row / 2) - 1)
			* (rowSepG + 2) * rowHeightG + rowSepAbsG
			+ 3 * rowHeightG / 2 ;
	    }
	}
    } else {
    	for( row = 1 ; row <= numRowsG ; row++ ) {
            rowArrayG[row].startx = 0 ;
	    if( row % 2 == 1 ) {
		rowArrayG[row].ypos = (row / 2)
			* (rowSepG + 2) * rowHeightG + rowSepAbsG
			+ rowHeightG / 2 ;
	    } else {
		rowArrayG[row].ypos = ((row/2) - 1)
			* (rowSepG + 2) * rowHeightG
			+ rowSepG * rowHeightG + rowSepAbsG
			+ 3 * rowHeightG / 2 ;
	    }
	}
    }
} else {
    if( individual_rowSepsG > 0 ) {
	for( row = 1 ; row <= numRowsG ; row++ ) {
	    rowArrayG[row].startx = 0 ;
	    if( row == 1 ){
		rowcenter = barrayG[1]->bheight / 2 ;
	    } else {
		rowcenter = rowtop + 
		    rowSepsG[row] * barrayG[row]->bheight +
		    rowSepsAbsG[row] +
		    barrayG[row]->bheight / 2 ;
	    }
	    rowArrayG[row].ypos = rowcenter ;
	    rowtop = rowcenter + barrayG[row]->bheight - 
		        barrayG[row]->bheight / 2;

	    /* *****
	       the old way - does not take into account different row heights 
	    rowArrayG[row].ypos = (rowSepsG[row] + 1.0) * rowHeightG +
				rowSepsAbsG[row] + rowArrayG[row-1].ypos ;
	    */
	}
    } else {
	/* take into account different row heights */
	for( row = 1 ; row <= numRowsG ; row++ ) {
	    rowArrayG[row].startx = 0 ;
	    if( row == 1 ){
		rowcenter = barrayG[1]->bheight / 2 ;
	    } else {
		rowcenter = rowtop + 
		    rowSepG * barrayG[row]->bheight +
		    rowSepAbsG + barrayG[row]->bheight / 2 ;
	    }
	    rowArrayG[row].ypos = rowcenter ;
	    rowtop = rowcenter + barrayG[row]->bheight - 
		        barrayG[row]->bheight / 2;
	    /* *****
	       the old way - does not take into account different row heights 
	    rowArrayG[row].ypos = (row - 1)
			* ((rowSepG + 1.0) * rowHeightG + rowSepAbsG)
			+ rowHeightG / 2 ;
	    */
	}
    }
}
if( numRowsG > 1 ) {
    totalF = 0.0 ;
    for( r = 1 ; r <= numRowsG ; r++ ) {
	totalF += relativeLenG[r] ;
    }
    topRowY = rowArrayG[numRowsG].ypos ;
    centerY = (topRowY + rowArrayG[1].ypos) / 2 ;
    Mf = (INT) ( (DOUBLE) totalRG / totalF ) ;

    dsumS = 0.0 ;
    for( r = 1 ; r <= numRowsG ; r++ ) {
	dsumS += relativeLenG[r] * 
			    (DOUBLE)(ABS(rowArrayG[r].ypos - centerY));
    }
    dsumS /= totalF ;
    dsumS *= 1.0 / (DOUBLE)( topRowY - centerY ) ;

    constant = 1.0 + (1.0 - indentG) * (1.0 - dsumS) ;
    for( r = 1 ; r <= numRowsG ; r++ ) {
	rowArrayG[r].endx = (INT)((DOUBLE) Mf * 
				    constant * relativeLenG[r]);
	modulation = 1.0 + (1.0 - indentG) * ( 
			  ((DOUBLE)(ABS(rowArrayG[r].ypos - centerY)) / 
			   (DOUBLE)(topRowY - centerY)) - dsumS);
	rowArrayG[r].desiredL = (INT) ( 
		    (DOUBLE) Mf * relativeLenG[r] * modulation ) ;
    }
} else {
    rowArrayG[1].endx = totalRG ;
    rowArrayG[1].desiredL = rowArrayG[1].endx ;
}

/*
    Define extra spaceG = rowSepG * rowHeightG + rowSepAbsG
*/
extraSpaceG = rowSepG * rowHeightG + rowSepAbsG;
for( k = 1 ; k <= 15 ; k++ ) {
    if( macspaceG[k] < 0.0 ) {
	macspaceG[k] = extraSpaceG ;
    } else {
	macspaceG[k] *= rowHeightG ;
    }
}
middleRow = numRowsG / 2 ;

/*
    Now its time to add in a macro block.  Suppose we have such
    a macro block with a location specification as:
    MUL,MUR,MLL,MLR, ML,MR,MB,MT, or MM
    ---macro is heightS units tall and widthS units wide.
*/
if (padArrayG) {
    Ysafe_free(padArrayG);
}
padArrayG = (INT *) Ysafe_malloc( (numtermsG + 1) * sizeof( INT ) ) ;
for( pad = 1 ; pad <= numtermsG ; pad++ ) {
    padArrayG[pad] = 0 ;
}
numMacroG = 0 ;
for( pad = numcellsG + 1 ; pad <= lastpadG ; pad++ ) {
    padside = carrayG[pad]->padptr->padside ;
    if( padside == MUL || padside == ML || padside == MLL ) {
	padArrayG[++numMacroG] = pad ;
    }
}
for( pad = numcellsG + 1 ; pad <= lastpadG ; pad++ ) {
    padside = carrayG[pad]->padptr->padside ;
    if( padside == MT || padside == MM || padside == MB ) {
	padArrayG[++numMacroG] = pad ;
    }
}
for( pad = numcellsG + 1 ; pad <= lastpadG ; pad++ ) {
    padside = carrayG[pad]->padptr->padside ;
    if( padside == MUR ) {
	padArrayG[++numMacroG] = pad ;
	URmacro = numMacroG ;
    }
}
for( pad = numcellsG + 1 ; pad <= lastpadG ; pad++ ) {
    padside = carrayG[pad]->padptr->padside ;
    if( padside == MR ) {
	padArrayG[++numMacroG] = pad ;
	Rmacro = numMacroG ;
    }
}
for( pad = numcellsG + 1 ; pad <= lastpadG; pad++ ) {
    padside = carrayG[pad]->padptr->padside ;
    if( padside == MLR ) {
	padArrayG[++numMacroG] = pad ;
    }
}


for( macro = 1 ; macro <= numMacroG ; macro++ ) {
    pad = padArrayG[macro] ;
    padptr = carrayG[pad] ;
    tptr = padptr->tileptr ;
    l = tptr->left ;
    r = tptr->right ;
    b = tptr->bottom ;
    t = tptr->top ;
    YtranslateT( &l, &b, &r, &t, (INT) padptr->corient ) ;
    heightS = t - b ;
    widthS  = r - l ;

    macNumber++ ;

    switch( padptr->padptr->padside ) {
      case MUL : for( row = numRowsG ; row >= 1 ; row-- ) {
		    if( rowArrayG[row].ypos + 
				(rowSepG * rowHeightG + rowSepAbsG) * 0.33 <= 
				rowArrayG[numRowsG].ypos - heightS ) {
			break ;
		    }
		}
		/*  shorten the rows between row + 1 and numRowsG by:
		    widthS + macspaceG[MUL] ;
		    Number of "rows" affected is: numRowsG - row ;
		    Add to all "rows" by the amount:
			(rows * (widthS + macspaceG[MUL]) ) / numRowsG ;
		*/
		rows = numRowsG - row ;
		for( row++ ; row <= numRowsG ; row++ ) {
		    rowArrayG[row].startx = widthS + macspaceG[MUL] ;
		    rowArrayG[row].desiredL -= widthS + macspaceG[MUL] ;
		}
		for( row = 1 ; row <= numRowsG ; row++ ) {
		    rowArrayG[row].endx += (rows * 
				(widthS + macspaceG[MUL]) ) / numRowsG ;
		    rowArrayG[row].desiredL += (rows * 
				(widthS + macspaceG[MUL]) ) / numRowsG ;
		}
		macroArrayG[macNumber].mx = widthS / 2 ;
		macroArrayG[macNumber].my =
			(rowArrayG[numRowsG].ypos + rowHeightG / 2) -
							heightS / 2 ;
		break ;
      case MUR : for( row = numRowsG ; row >= 1 ; row-- ) {
		    if( rowArrayG[row].ypos + 
				 (rowSepG * rowHeightG + rowSepAbsG) * 0.33 <= 
				 rowArrayG[numRowsG].ypos - heightS ) {
			break ;
		    }
		}
		/*  shorten the rows between row + 1 and numRowsG by:
		    widthS + macspaceG[MUR] ;
		    Number of "rows" affected is: numRowsG - row ;
		    Add to all "rows" by the amount:
			(rows * (widthS + macspaceG[MUR]) ) / numRowsG ;
		*/
		rows = numRowsG - row ;
		for( row++ ; row <= numRowsG ; row++ ) {
		    rowArrayG[row].endx -= widthS + macspaceG[MUR] ;
		    rowArrayG[row].desiredL -= widthS + macspaceG[MUR] ;
		}
		for( row = 1 ; row <= numRowsG ; row++ ) {
		    rowArrayG[row].endx += (rows * 
				(widthS + macspaceG[MUR]) ) / numRowsG ;
		    rowArrayG[row].desiredL += (rows * 
				(widthS + macspaceG[MUR]) ) / numRowsG ;
		}
		macroArrayG[macNumber].mx = rowArrayG[numRowsG].endx +
					   macspaceG[MUR] + widthS / 2 ;
		macroArrayG[macNumber].my =
			(rowArrayG[numRowsG].ypos + rowHeightG / 2) -
							heightS / 2 ;
		break ;
      case MLL : for( row = 1 ; row <= numRowsG ; row++ ) {
		    if( rowArrayG[row].ypos - 
					(rowSepG * rowHeightG + rowSepAbsG) * 0.33 >= 
					rowArrayG[1].ypos + heightS ) {
			break ;
		    }
		}
		/*  shorten the rows between 1 and row - 1 by:
		    widthS + macspaceG[MLL] ;
		    Number of "rows" affected is: row - 1 ;
		    Add to all "rows" by the amount:
			(rows * (widthS + macspaceG[MLL]) ) / numRowsG ;
		*/
		rows = row - 1 ;
		for( row-- ; row >= 1 ; row-- ) {
		    rowArrayG[row].startx = widthS + macspaceG[MLL] ;
		    rowArrayG[row].desiredL -= widthS + macspaceG[MLL] ;
		}
		for( row = 1 ; row <= numRowsG ; row++ ) {
		    rowArrayG[row].endx += (rows * 
				(widthS + macspaceG[MLL]) ) / numRowsG ;
		    rowArrayG[row].desiredL += (rows * 
				(widthS + macspaceG[MLL]) ) / numRowsG ;
		}
		macroArrayG[macNumber].mx = widthS / 2 ;
		macroArrayG[macNumber].my =
			(rowArrayG[1].ypos - rowHeightG / 2) +
							heightS / 2 ;
		break ;
      case MLR : for( row = 1 ; row <= numRowsG ; row++ ) {
		    if( rowArrayG[row].ypos - 
					(rowSepG * rowHeightG + rowSepAbsG) * 0.33 >= 
					rowArrayG[1].ypos + heightS ) {
			break ;
		    }
		}
		/*  shorten the rows between 1 and row - 1 by:
		    widthS + macspaceG[MLR] ;
		    Number of "rows" affected is: row - 1 ;
		    Add to all "rows" by the amount:
			(rows * (widthS + macspaceG[MLR]) ) / numRowsG ;
		*/
		rows = row - 1 ;
		for( row-- ; row >= 1 ; row-- ) {
		    rowArrayG[row].endx -= widthS + macspaceG[MLR] ;
		    rowArrayG[row].desiredL -= widthS + macspaceG[MLR] ;
		}
		for( row = 1 ; row <= numRowsG ; row++ ) {
		    rowArrayG[row].endx += (rows * 
				(widthS + macspaceG[MLR]) ) / numRowsG ;
		    rowArrayG[row].desiredL += (rows * 
				(widthS + macspaceG[MLR]) ) / numRowsG ;
		}
		if( URmacro != 0 ) {
		    macroArrayG[URmacro].mx += (rows *
			    (widthS + macspaceG[MLR]) ) / numRowsG ;
		}
		if( Rmacro != 0 ) {
		    macroArrayG[Rmacro].mx += (rows *
			    (widthS + macspaceG[MLR]) ) / numRowsG ;
		}
		macroArrayG[macNumber].mx = rowArrayG[1].endx +
					   macspaceG[MLR] + widthS / 2 ;
		macroArrayG[macNumber].my =
			(rowArrayG[1].ypos - rowHeightG / 2) +
							heightS / 2 ;
		break ;
      case ML  : bot = rowArrayG[middleRow].ypos - heightS / 2 ;
		top = bot + heightS ;
		for( row = 1 ; row <= numRowsG ; row++ ) {
		    if( rowArrayG[row].ypos - rowHeightG / 2 - 
			    (rowSepG * rowHeightG + rowSepAbsG) * 0.33 >= top ) {
			break ;
		    }
		}
		trow = row ;
		for( row = numRowsG ; row >= 1 ; row-- ) {
		    if( rowArrayG[row].ypos + rowHeightG / 2 + 
				    (rowSepG * rowHeightG + rowSepAbsG) * 0.33 <= bot ) {
			break ;
		    }
		}
		brow = row ;
		/*  shorten the rows between brow and trow by:
		    widthS + macspaceG[ML] ;
		    Number of "rows" affected is: trow - brow - 1 ;
		    Add to all "rows" by the amount:
			(rows * (widthS + macspaceG[ML]) ) / numRowsG ;
		*/
		rows = trow - brow - 1 ;
		for( row = brow + 1 ; row < trow ; row++ ) {
		    rowArrayG[row].startx = widthS + macspaceG[ML] ;
		    rowArrayG[row].desiredL -= widthS + macspaceG[ML] ;
		}
		for( row = 1 ; row <= numRowsG ; row++ ) {
		    rowArrayG[row].endx += (rows * 
				(widthS + macspaceG[ML]) ) / numRowsG ;
		    rowArrayG[row].desiredL += (rows * 
				(widthS + macspaceG[ML]) ) / numRowsG ;
		}
		macroArrayG[macNumber].mx = widthS / 2 ;
		macroArrayG[macNumber].my = rowArrayG[middleRow].ypos;
		break ;
      case MR  : bot = rowArrayG[middleRow].ypos - heightS / 2 ;
		top = bot + heightS ;
		for( row = 1 ; row <= numRowsG ; row++ ) {
		    if( rowArrayG[row].ypos - rowHeightG / 2 -
				    (rowSepG * rowHeightG + rowSepAbsG) * 0.33 >= top ) {
			break ;
		    }
		}
		trow = row ;
		for( row = numRowsG ; row >= 1 ; row-- ) {
		    if( rowArrayG[row].ypos + rowHeightG / 2 +
				    (rowSepG * rowHeightG + rowSepAbsG) * 0.33 <= bot ) {
			break ;
		    }
		}
		brow = row ;
		/*  shorten the rows between brow and trow by:
		    widthS + macspaceG[MR] ;
		    Number of "rows" affected is: trow - brow - 1 ;
		    Add to all "rows" by the amount:
			(rows * (widthS + macspaceG[MR]) ) / numRowsG ;
		*/
		rows = trow - brow - 1 ;
		for( row = brow + 1 ; row < trow ; row++ ) {
		    rowArrayG[row].endx -= widthS + macspaceG[MR] ;
		    rowArrayG[row].desiredL -= widthS + macspaceG[MR] ;
		}
		for( row = 1 ; row <= numRowsG ; row++ ) {
		    rowArrayG[row].endx += (rows * 
				(widthS + macspaceG[MR]) ) / numRowsG ;
		    rowArrayG[row].desiredL += (rows * 
				(widthS + macspaceG[MR]) ) / numRowsG ;
		}
		if( URmacro != 0 ) {
		    macroArrayG[URmacro].mx += (rows *
				(widthS + macspaceG[MR]) ) / numRowsG ;
		}
		macroArrayG[macNumber].mx = 
				    rowArrayG[middleRow].endx +
				    macspaceG[MR] + widthS / 2 ;
		macroArrayG[macNumber].my = rowArrayG[middleRow].ypos;
		break ;
      case MM  : bot = rowArrayG[middleRow].ypos - heightS / 2 ;
		top = bot + heightS ;
		/*
		left = (rowArrayG[middleRow].startx + 
			rowArrayG[middleRow].endx ) / 2 - widthS / 2 ;
		*/
		left = (rowArrayG[middleRow].startx + 
			rowArrayG[middleRow].endx ) / 2 ;
		rite = left + widthS ;
		for( row = 1 ; row <= numRowsG ; row++ ) {
		    if( rowArrayG[row].ypos - rowHeightG / 2 -
				    (rowSepG * rowHeightG + rowSepAbsG) * 0.67 >= top ) {
			break ;
		    }
		}
		trow = row ;
		for( row = numRowsG ; row >= 1 ; row-- ) {
		    if( rowArrayG[row].ypos + rowHeightG / 2 +
				    (rowSepG * rowHeightG + rowSepAbsG) * 0.67 <= bot ) {
			break ;
		    }
		}
		brow = row ;
		/*  split the rows between brow and trow by:
		    widthS + 2 * macspaceG[MM] ;
		    Number of "rows" affected is: trow - brow - 1 ;
		    Add to all "rows" by the amount:
		      (rows * (widthS + 2 * macspaceG[MM]) ) / numRowsG ;
		*/
		rows = trow - brow - 1 ;
		for( row = brow + 1 ; row < trow ; row++ ) {
		    rowArrayG[row].endx1   = left - macspaceG[MM] ;
		    rowArrayG[row].startx2 = rite + macspaceG[MM] ;
		   rowArrayG[row].desiredL -= widthS + 2 * macspaceG[MM];
		}
		for( row = 1 ; row <= numRowsG ; row++ ) {
		    rowArrayG[row].endx += (rows * 
			      (widthS + 2 * macspaceG[MM]) ) / numRowsG ;
		    rowArrayG[row].desiredL += (rows * 
			      (widthS + 2 * macspaceG[MM]) ) / numRowsG ;
		}
		macroArrayG[macNumber].mx = (left + rite) / 2 ;
		macroArrayG[macNumber].my = (bot + top) / 2 ;
		break ;
      case MB  : bot = rowArrayG[1].ypos - rowHeightG / 2 ;
		top = bot + heightS ;
		left = (rowArrayG[1].startx + 
				rowArrayG[1].endx ) / 2 - widthS / 2 ;
		rite = left + widthS ;
		for( row = 1 ; row <= numRowsG ; row++ ) {
		    if( rowArrayG[row].ypos - rowHeightG / 2 -
				    (rowSepG * rowHeightG + rowSepAbsG) * 0.67 >= top ) {
			break ;
		    }
		}
		trow = row ;
		brow = 0 ;
		/*  split the rows between brow and trow by:
		    widthS + 2 * macspaceG[MB] ;
		    Number of "rows" affected is: trow - brow - 1 ;
		    Add to all "rows" by the amount:
		      (rows * (widthS + 2 * macspaceG[MB]) ) / numRowsG ;
		*/
		rows = trow - brow - 1 ;
		for( row = brow + 1 ; row < trow ; row++ ) {
		    rowArrayG[row].endx1   = left - macspaceG[MB] ;
		    rowArrayG[row].startx2 = rite + macspaceG[MB] ;
		   rowArrayG[row].desiredL -= widthS + 2 * macspaceG[MB];
		}
		for( row = 1 ; row <= numRowsG ; row++ ) {
		    rowArrayG[row].endx += (rows * 
			(widthS + 2 * macspaceG[MB]) ) / numRowsG ;
		    rowArrayG[row].desiredL += (rows * 
			(widthS + 2 * macspaceG[MB]) ) / numRowsG ;
		}
		macroArrayG[macNumber].mx = (left + rite) / 2 ;
		macroArrayG[macNumber].my = (bot + top) / 2 ;
		break ;
      case MT  : top = rowArrayG[numRowsG].ypos + rowHeightG / 2 ;
		bot = top - heightS ;
		left = (rowArrayG[numRowsG].startx + 
			rowArrayG[numRowsG].endx ) / 2 - widthS / 2 ;
		rite = left + widthS ;
		trow = numRowsG + 1 ;
		for( row = numRowsG ; row >= 1 ; row-- ) {
		    if( rowArrayG[row].ypos + rowHeightG / 2 +
				    (rowSepG * rowHeightG + rowSepAbsG) * 0.67 <= bot ) {
			break ;
		    }
		}
		brow = row ;
		/*  split the rows between brow and trow by:
		    widthS + 2 * macspaceG[MT] ;
		    Number of "rows" affected is: trow - brow - 1 ;
		    Add to all "rows" by the amount:
		      (rows * (widthS + 2 * macspaceG[MT]) ) / numRowsG ;
		*/
		rows = trow - brow - 1 ;
		for( row = brow + 1 ; row < trow ; row++ ) {
		    rowArrayG[row].endx1   = left - macspaceG[MT] ;
		    rowArrayG[row].startx2 = rite + macspaceG[MT] ;
		   rowArrayG[row].desiredL -= widthS + 2 * macspaceG[MT];
		}
		for( row = 1 ; row <= numRowsG ; row++ ) {
		    rowArrayG[row].endx += (rows * 
			     (widthS + 2 * macspaceG[MT]) ) / numRowsG ;
		    rowArrayG[row].desiredL += (rows * 
			     (widthS + 2 * macspaceG[MT]) ) / numRowsG ;
		}
		macroArrayG[macNumber].mx = (left + rite) / 2 ;
		macroArrayG[macNumber].my = (bot + top) / 2 ;
		break ;
	default: fprintf(fpoG,"screwup in finding padside\n");
		 YexitPgm(PGMFAIL);
		 break ;
    }
}

/*  
 *  Now its time to generate a pad placement around the
 *  periphery of the just defined core topology.
 *  First determine coreWidthG and coreHeight.
 */
coreHeightG = rowArrayG[numRowsG].ypos + rowHeightG - rowHeightG / 2 ;
coreWidthG = 0 ;
for( row = 1 ; row <= numRowsG ; row++ ) {
    if( rowArrayG[row].endx > coreWidthG ) {
	coreWidthG = rowArrayG[row].endx ;
    }
}
for( macro = 1 ; macro <= numMacroG ; macro++ ) {
    pad = padArrayG[macro] ;
    padptr = carrayG[pad] ;
    tptr = padptr->tileptr ;
    l = tptr->left ;
    r = tptr->right ;
    b = tptr->bottom ;
    t = tptr->top ;
    YtranslateT( &l, &b, &r, &t, (INT) padptr->corient ) ;
    edge = macroArrayG[macro].mx + r ;
    if( edge > coreWidthG ) {
	coreWidthG = edge ;
    }
}

/*
 *  Add in the MTT and MBB macros, if there are any
 */
mbbshiftS = 0 ;
heightS   = 0 ;
widthS    = 0 ;
count_mbb = 0 ;
for( pad = numcellsG + 1 ; pad <= lastpadG ; pad++ ) {
    padptr = carrayG[pad] ;
    if( padptr->padptr->padside == MBB ) {
	tptr = padptr->tileptr ;
	l = tptr->left ;
	r = tptr->right ;
	b = tptr->bottom ;
	t = tptr->top ;
	YtranslateT( &l, &b, &r, &t, (INT) padptr->corient ) ;
	if( t - b > heightS ){
	    heightS = t - b ;
	}
	widthS += r - l ;
	count_mbb++ ;
    }
}
if( count_mbb > 0 ) {
    mbbshiftS = heightS + rowSepG * rowHeightG + rowSepAbsG;
    /*
     *   Shift all rows up by heightS + rowSepG * rowHeightG + rowSepAbsG
     */
    for( row = 1 ; row <= numRowsG ; row++ ) {
	rowArrayG[row].ypos += mbbshiftS ;
    }
    coreHeightG += mbbshiftS ;
    if( widthS > coreWidthG ) {
	coreWidthG = widthS ;
    }
    spaceG = coreWidthG - widthS ;
    separationG = spaceG / (count_mbb + 1) ;
    if( separationG < 0 ) {
	separationG = 0 ;
    }

    maxHeight = heightS ;
    heightS    = 0 ;
    widthS     = 0 ;
    lastG      = separationG ;
    for( pad = numcellsG + 1 ; pad <= lastpadG ; pad++ ) {
	padptr = carrayG[pad] ;
	if( padptr->padptr->padside == MBB ) {
	    tptr = padptr->tileptr ;
	    l = tptr->left ;
	    r = tptr->right ;
	    b = tptr->bottom ;
	    t = tptr->top ;
	    YtranslateT( &l, &b, &r, &t, (INT) padptr->corient ) ;
	    heightS = t - b ;
	    widthS  = r - l ;

	    padptr->cxcenter = lastG + widthS / 2  ;
	    padptr->cycenter = maxHeight - heightS / 2 - 1 ;
	    lastG += separationG + widthS ;
	}
    }
}


mttshiftS = 0 ;
heightS   = 0 ;
widthS    = 0 ;
count_mtt = 0 ;
for( pad = numcellsG + 1 ; pad <= lastpadG ; pad++ ) {
    padptr = carrayG[pad] ;
    if( padptr->padptr->padside == MTT ) {
	tptr = padptr->tileptr ;
	l = tptr->left ;
	r = tptr->right ;
	b = tptr->bottom ;
	t = tptr->top ;
	YtranslateT( &l, &b, &r, &t, (INT) padptr->corient ) ;
	if( t - b > heightS ){
	    heightS = t - b ;
	}
	widthS += r - l ;
	count_mtt++ ;

    }
}
if( count_mtt > 0 ) {
    mttshiftS = heightS + rowSepG * rowHeightG + rowSepAbsG;
    coreHeightG += mttshiftS ;
    if( widthS > coreWidthG ) {
	coreWidthG = widthS ;
    }
    spaceG = coreWidthG - widthS ;
    separationG = spaceG / (count_mtt + 1) ;
    if( separationG < 0 ) {
	separationG = 0 ;
    }

    heightS    = 0 ;
    widthS     = 0 ;
    lastG      = separationG ;
    for( pad = numcellsG + 1 ; pad <= lastpadG ; pad++ ) {
	padptr = carrayG[pad] ;
	if( padptr->padptr->padside == MTT ) {
	    tptr = padptr->tileptr ;
	    l = tptr->left ;
	    r = tptr->right ;
	    b = tptr->bottom ;
	    t = tptr->top ;
	    YtranslateT( &l, &b, &r, &t, (INT) padptr->corient ) ;
	    heightS = t - b ;
	    widthS  = r - l ;

	    padptr->cxcenter = lastG + widthS / 2  ;
	    padptr->cycenter = (coreHeightG - mttshiftS) + rowSepG * rowHeightG + 
				rowSepAbsG + heightS / 2 ;
	    lastG += separationG + widthS ;
	}
    }
}

/*  
 *  Now its time to set the positions of the macros
 */
for( macro = 1 ; macro <= numMacroG ; macro++ ) {
    pad = padArrayG[macro] ;
    carrayG[pad]->cxcenter = macroArrayG[macro].mx ;
    carrayG[pad]->cycenter = macroArrayG[macro].my + mbbshiftS ;
}

/* 
 *  Now its time to blastG away the old barrayG and
 *  generate the new one.
 */

/*
 *  Find the number of blocks
 */
block = 0 ;
for( row = 1 ; row <= numRowsG ; row++ ) {
    block++ ;
    if( rowArrayG[row].endx1 > 0 ) {
	block++ ;
    }
}
totalBlock = block ;
oldbarray = barrayG ;

barrayG = (BBOXPTR *) Ysafe_malloc( (totalBlock + 1) * sizeof(BBOXPTR) ) ;
for( block = 1 ; block <= totalBlock ; block++ ) {
    barrayG[block] = (BBOXPTR) Ysafe_malloc( sizeof(BBOX) ) ;
}

block = 0 ;
for( row = 1 ; row <= numRowsG ; row++ ) {
    boxptr = oldbarray[row] ;
    if( rowArrayG[row].endx1 > 0 ) {
	block++ ;
	barrayG[block]->bxcenter = (rowArrayG[row].startx +
				    rowArrayG[row].endx1 ) / 2 ;
	barrayG[block]->bycenter = rowArrayG[row].ypos ;
	barrayG[block]->bleft   = rowArrayG[row].startx -
				    barrayG[block]->bxcenter ;
	barrayG[block]->bright  = rowArrayG[row].endx1 -
				    barrayG[block]->bxcenter ;
	barrayG[block]->bbottom = boxptr->bbottom ;
	barrayG[block]->btop    = boxptr->btop ;
	barrayG[block]->bheight = boxptr->btop - boxptr->bbottom ;
	barrayG[block]->blength = rowArrayG[row].endx1 -
				    rowArrayG[row].startx ;
	barrayG[block]->bclass  = boxptr->bclass ;
	barrayG[block]->borient = boxptr->borient ;
	barrayG[block]->oldsize = 0 ;

	block++ ;
	barrayG[block]->bxcenter = (rowArrayG[row].startx2 +
				    rowArrayG[row].endx ) / 2 ;
	barrayG[block]->bycenter = rowArrayG[row].ypos ;
	barrayG[block]->bleft   = rowArrayG[row].startx2 -
				    barrayG[block]->bxcenter ;
	barrayG[block]->bright  = rowArrayG[row].endx -
				    barrayG[block]->bxcenter ;
	barrayG[block]->bbottom = boxptr->bbottom ;
	barrayG[block]->btop    = boxptr->btop ;
	barrayG[block]->bheight = boxptr->btop - boxptr->bbottom ;
	barrayG[block]->blength = rowArrayG[row].endx -
				    rowArrayG[row].startx2 ;
	barrayG[block]->bclass  = boxptr->bclass ;
	barrayG[block]->borient = boxptr->borient ;

	ratio = (DOUBLE) barrayG[block - 1]->blength / (DOUBLE) 
	     (barrayG[block - 1]->blength + barrayG[block]->blength) ;
	barrayG[block]->desire = 1 + (INT)((1.0 - ratio) * 
				    rowArrayG[row].desiredL) ;
	barrayG[block - 1]->desire = 1 + (INT)(ratio * 
				    rowArrayG[row].desiredL) ;
	barrayG[block]->oldsize = 0 ;
    } else {
	block++ ;
	barrayG[block]->bxcenter = (rowArrayG[row].startx +
				    rowArrayG[row].endx ) / 2 ;
	barrayG[block]->bycenter = rowArrayG[row].ypos ;
	barrayG[block]->bleft   = rowArrayG[row].startx -
				    barrayG[block]->bxcenter ;
	barrayG[block]->bright  = rowArrayG[row].endx -
				    barrayG[block]->bxcenter ;
	barrayG[block]->bbottom = boxptr->bbottom ;
	barrayG[block]->btop    = boxptr->btop ;
	barrayG[block]->bheight = boxptr->btop - boxptr->bbottom ;
	barrayG[block]->blength = rowArrayG[row].endx -
				    rowArrayG[row].startx ;
	barrayG[block]->bclass  = boxptr->bclass ;
	barrayG[block]->borient = boxptr->borient ;
	barrayG[block]->desire  = rowArrayG[row].desiredL ;
	barrayG[block]->oldsize = 0 ;
    }
}


/*
 *  Now blow array the old block array, rowArrayG, macroArray
 */
for( row = 1 ; row <= numRowsG ; row++ ) {
    Ysafe_free( oldbarray[row] ) ;
}
Ysafe_free( oldbarray ) ;
numRowsG = totalBlock ;


fprintf(fpoG,"\n******************\nBLOCK DATA\n");

desiredLG = 0 ;
for( block = 1 ; block <= numRowsG ; block++ ) {
    barrayG[block]->orig_desire = barrayG[block]->desire ;
    desiredLG += barrayG[block]->desire ;
    fprintf(fpoG,"block:%d desire:%d\n",block,barrayG[block]->desire);

    D( "configure", 
	xc = barrayG[block]->bxcenter ;
	yc = barrayG[block]->bycenter ;
	fprintf( stderr, "\tblock:%d l:%d r:%d b:%d t:%d\n",
	    block, barrayG[block]->bleft+xc, barrayG[block]->bright+xc,
	    barrayG[block]->bbottom+yc, barrayG[block]->btop+yc ) ;
    ) ;
}
fprintf(fpoG,"Total Desired Length: %d\n", desiredLG ) ;

random_placement() ;
/*
 * Ah ha, done
 */
return ;
}





random_placement()
{

INT cell , row , borient ,  blk , widthS ;
INT block ;
INT empty ;
INT *filledTo ;
INT *endRow ;
CBOXPTR cellptr ;

/*
 *  And now, folks, its time for cell placement, that is,
 *  real random cell placement
 */

filledTo = (INT *) Ysafe_malloc( (numRowsG + 1) * sizeof( INT ) ) ;
endRow = (INT *) Ysafe_malloc( (numRowsG + 1) * sizeof( INT ) ) ;
for( row = 1 ; row <= numRowsG ; row++ ) {
    filledTo[row] = barrayG[row]->bxcenter + barrayG[row]->bleft ;
    endRow[row]  = barrayG[row]->bxcenter + barrayG[row]->bright ;
}
/*
 *  On first pass, place the cells with specific block assignments
 */
for( cell = 1 ; cell <= numcellsG - extra_cellsG ; cell++ ) {
    cellptr = carrayG[cell] ;
    blk = cellptr->cblock ;
    if( blk == 0 ) {
	continue ;
    }
    borient = barrayG[blk]->borient ;
    cellptr->cblock = blk ;

    widthS = cellptr->tileptr->right - cellptr->tileptr->left ;
    if( cellptr->cycenter > 0 ) {
	carrayG[cell]->cxcenter = barrayG[blk]->bxcenter + 
				    barrayG[blk]->bleft + 
				    cellptr->cxcenter + widthS / 2 ;
	carrayG[cell]->cycenter = barrayG[blk]->bycenter ;
    } else {
	carrayG[cell]->cxcenter = barrayG[blk]->bxcenter + 
				    barrayG[blk]->bright + 
		    cellptr->cxcenter - (widthS - widthS / 2) ;
	carrayG[cell]->cycenter = barrayG[blk]->bycenter ;
    }

    if( borient == 2 ) {
	cellptr->corient = 1 ;
    } else {  /* borient was 1 */
	cellptr->corient = 0 ;
    }
}

/*
 *  Final pass, place all unassigned cells
 */
for( cell = 1 ; cell <= numcellsG - extra_cellsG ; cell++ ) {
    if( carrayG[cell]->cblock != 0 ) {
	continue ;
    }
    /*  
     *  cute strategy:  place cell in the most empty row
     */
    empty = INT_MIN ;
    for( row = 1 ; row <= numRowsG ; row++ ) {
	if( endRow[row] - filledTo[row] > empty ) {
	    empty = endRow[row] - filledTo[row] ;
	    block = row ;
	}
    }
    borient = barrayG[block]->borient ;
    widthS = carrayG[cell]->tileptr->right - carrayG[cell]->tileptr->left ;
    carrayG[cell]->cblock = block ;
    if( borient > 0 ) {
	carrayG[cell]->cxcenter = filledTo[block] + widthS / 2 ;
	carrayG[cell]->cycenter = barrayG[block]->bycenter ;
	if( borient == 2 ) {
	    carrayG[cell]->corient = 1 ;
	} else {  /* borient was 1 */
	    carrayG[cell]->corient = 0 ;
	}
    } else {
	carrayG[cell]->cycenter = filledTo[block] + widthS / 2 ;
	carrayG[cell]->cxcenter = barrayG[block]->bxcenter ;
	if( borient == -2 ) {
	    carrayG[cell]->corient = 4 ;
	} else {  /* borient was -1 */
	    carrayG[cell]->corient = 7 ;
	}
    }
    filledTo[block] += widthS ;
}
/* now finish by placing the pads */
placepads() ;
return ;
}


/*  used for split rows only, which we don't allow any more
RtoB( row , from )
INT row , from ;
{
    INT y , block , block1 , block2 , endx1 , startx2 ;

    y = rowArrayG[row].ypos ;
    block1 = 0 ;
    block2 = 0 ;
    for( block = 1 ; block <= numRowsG ; block++ ) {
	if( barrayG[block]->bycenter == y ) {
	    block1 = block ;
	    break ;
	}
    }
    for( block++ ; block <= numRowsG ; block++ ) {
	if( barrayG[block]->bycenter == y ) {
	    block2 = block ;
	    break ;
	}
    }
    if( block1 == 0 ) {
	fprintf(fpoG,"RtoB failed to find block corresponding");
	fprintf(fpoG," to row:%d\n", row ) ;
	YexitPgm(PGMFAIL) ;
    }
    if( block2 == 0 ) {
	block = block1 ;
    } else {
	endx1 = barrayG[block1]->bxcenter + barrayG[block1]->bright ;
	startx2 = barrayG[block2]->bxcenter + barrayG[block2]->bleft ;
	if( from > 0 ) {
	    if( endx1 < startx2 ) {
		block = block1 ;
	    } else {
		block = block2 ;
	    }
	} else {
	    if( startx2 > endx1 ) {
		block = block2 ;
	    } else {
		block = block1 ;
	    }
	}
    }
    return(block) ;
}
*/
