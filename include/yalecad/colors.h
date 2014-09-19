/* ----------------------------------------------------------------- 
FILE:	    colors.h                                       
DESCRIPTION:Insert file for Timberwolf standard colors.
CONTENTS:   
DATE:	    Jan 25, 1989 - major rewrite of version 1.0
REVISIONS:  Jan 27, 1989 - added frame num
	    Sep 12, 1989 - added cyan to color table.
	    Oct  5, 1990 - added more colors to color table.
----------------------------------------------------------------- */
#ifndef COLOR_H
#define COLOR_H

#ifndef lint
static char colors_hId[] = "@(#) colors.h version 1.8 2/26/92" ;
#endif

#define TWWHITE        1
#define TWBLACK        2
#define TWGRAY         3
#define TWRED          4
#define TWBLUE         5
#define TWYELLOW       6
#define TWGREEN        7
#define TWVIOLET       8
#define TWORANGE       9
#define TWMEDAQUA     10
#define TWCYAN        11
#define TWLIGHTGREEN  12
#define TWLIGHTBLUE   13
#define TWBROWN       14
#define TWLIGHTBROWN  15
#define TWNAVY        16
#define TWLIGHTVIOLET 17
#define TWKHAKI       18

/* returns the standard color array */
extern char **TWstdcolors() ;

/* returns the number of colors in standard color array */
extern INT TWnumcolors() ;

#endif /* COLOR_H */

