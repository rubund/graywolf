/* globals.h - global insert file for draw */ 
/* static char SccsId[] = "@(#) globals.h version 3.6 9/21/91" ; */

#include <yalecad/base.h>
#include <yalecad/dbinary.h>
#include <yalecad/draw.h>

#ifndef MAIN_DEFS
#define EXTERN extern
#else
#define EXTERN
#endif

#ifdef MAIN_DEFS
/********************************************************/
/* Default colors					*/
/********************************************************/
/* On a color display. The names of all valid colors   		*/ 
/*	are in /usr/lib/rgb.txt					*/
/* On a monochrome display, all colors specified in this 	*/
/*	file with the exception of black, will be displayed	*/
/*	as gray							*/
char *HLineColor = "blue",		/* vertical line */
    *VLineColor = "red",		/* horizontal line */
    *CellFillColor = "gold",
    *ViaColor = "black",
    *PinColor = "navy",
    *MstColor = "orange",
    *FontColor = "magenta";

/********************************************************/
/* Default font		 				*/
/********************************************************/
char *default_font = "helv10bssx";

#else
extern char *HLineColor, *VLineColor, *CellFillColor,
    *ViaColor, *PinColor, *MstColor, *FontColor ;
extern char *default_font ;

#endif

/* remove graphics lines of code if compile switch is on */
#ifdef NOGRAPHICS
#define G(x_xz) 

#else 
#define G(x_xz)   x_xz

#endif /* NOGRAPHICS */


/* define the possible errors */
#define  NODIRECTORY  1
#define  NOFILES      2
#define  NODISPLAY    3
#define  FAILBINARY   4
#define	 Graph_offset	0	/* room for LR corner */

/* global variables */
EXTERN int maxxG,maxyG;	 	 /* max x and y of graph */
EXTERN int minxG,minyG;		 /* min x and y of graph */
EXTERN int maxCountG;		 /* maximum frame number */
EXTERN int maxNetnoG;		 /* highest net # */
EXTERN int wHeight;		 /* height of display window */
EXTERN char *cellFileG;          /* file name for cell file */
EXTERN char *netFileG;           /* file name for net file */
EXTERN char *symbFileG;          /* file name for symb file */
EXTERN int num_cel_recG;         /* number of cell records read in */
EXTERN int num_net_recG;         /* number of net records read in */
EXTERN int num_charG;            /* number of characters in symbol table*/
EXTERN int  *colorArrayG;        /* maps color index to Xcolor index */
EXTERN DATABOX *cell_file_ptrG ; 
EXTERN DATABOX *net_file_ptrG ; 
EXTERN char  *lookupLabelG ;     /* symbol table for labels */
