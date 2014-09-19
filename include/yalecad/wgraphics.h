/* ----------------------------------------------------------------- 
FILE:	    wgraphics.h                                       
DESCRIPTION:include file for binary file draw utilities
    NOTE: avoids X11 library link if graphics is to be dumped to file
    to be read later by the draw utility program.
CONTENTS:   typedefs, and external routines for draw facility
DATE:	    Mar 21, 1989 - began as derivative of draw.h
REVISIONS:  May 15, 1990 - added TWsetFrame to include file
	    Mon Aug 12 15:51:04 CDT 1991 - fixed problems
		with missing functions.
----------------------------------------------------------------- */
#ifndef WGRAPHICS_H
#define WGRAPHICS_H

#ifndef lint
static char YwgraphicsId[] = "@(#) wgraphics.h (Yale) version 1.7 8/12/91" ;
#endif

/******** FUNCTIONS NORMALLY USED BY GRAPHIC PROGRAM USERS *********/
#define TWinitGraphics( argc, argv, numC, colors,menuPath,refresh_func) \
    TWinitWGraphics( numC, colors ) 
/* 
Arguments:
    INT numC ;
    char **colors ;
Function:
    Initialization of graphics package.  Needs to be called first.
    DesiredColors is an array of strings of the desired colors.  The
    colors must agree with the standard color map of the machine and
    in addition element 1 must be "white" and element two "black". Also
    element 0 is not used at this time.  Use TWstdcolors to use 
    the graphics packages builtin standard colors.
    NumC is the number of colors in the color array counting white and
    black.
    NOTE: it is advised that the TimberWolf standard colors be used if
    possible.  Use TWstdcolors() for argument desiredColors and 
    TWnumcolors() for numC arg and use include file <yalecad/colors.h>.
    NOTE: In this use argc, argv, menuPath and refresh_func are ignored
*/

#define TWcloseGraphics()  (TWcloseWGraphics())
/* 
Arguments: None
Function:
    Closes graphics display and performs cleanup operations.
    Should be performed before end of program.
*/


#define TWdrawNet(ref_num, x1, y1, x2, y2, color, label ) \
TWdrawWLine(ref_num, x1, y1, x2, y2, color, label )

#define TWdrawLine(ref_num, x1, y1, x2, y2, color, label ) \
TWdrawWLine(ref_num, x1, y1, x2, y2, color, label )

/*
Arguments: 
    INT ref_num, x1, y1, x2, y2, color ;
    char *label ;
Function:
    Draws a net/line to screen and/or file.  
    Ref_num is a reference number to associate all lines of a net - 
	currently not used.
    X1, y1, x2, y2 are the two points of the line of a net.
    Color is the element number in color array. For example white is 1
    and black is 2.
    Label is optional - a label is attached to figure if non-null.
*/

#define TWdrawCell(ref_num, x1, y1, x2, y2, color, label )\
TWdrawWRect(ref_num, x1, y1, x2, y2, color, label )

#define TWdrawRect(ref_num, x1, y1, x2, y2, color, label )\
TWdrawWRect(ref_num, x1, y1, x2, y2, color, label )
/*
Arguments: 
    INT ref_num, x1, y1, x2, y2, color ;
    char *label ;
Function:
    Draws a cell to screen and/or file.  
    Ref_num is a reference number to associate cell - currently
	not used.
    X1, y1, x2, y2 are the lower left, upper right points of cell.
    Color is the element number in color array. For example white is 1
    and black is 2.
    Label is optional - a label is attached to figure if non-null.
*/

#define TWdrawPin(ref_num, x1, y1, x2, y2, color, label ) \
TWdrawWPin(ref_num, x1, y1, x2, y2, color, label )
/*
Arguments: 
    INT ref_num, x1, y1, x2, y2, color ;
    char *label ;
Function:
    Draws a pin to screen and/or file.  
    Ref_num is a reference number to associate pin - currently
	not used.
    X1, y1, x2, y2 are the lower left, upper right points of pin.
    Color is the element number in color array. For example white is 1
    and black is 2.
    Label is optional - a label is attached to figure if non-null.
*/
/* copy pixmap to screen and flush screen output buffer */

#define TWflushFrame()  TWflushWFrame()
/*
Arguments: None.
Function:
    Flush the screen output and/or file buffer.
    This call must be made after all data of a frame is written, that is
    after all TWdraws have performed.
*/

#define TWstartFrame()  TWstartWFrame()
/*
Arguments: None.
Function:
    Start a new file frame.
    This call must be made before data is written, that is
    before any TWdraws are performed for a given frame.
*/

#define TWsetFrame( frame_xz )  TWsetWFrame( frame_xz )
/*
Arguments: 
    INT frame_x ;
Function:
    Resets dump to screen dump to given file.
    If zero is given, it set the frame to one past the last frame found
    in the specified DATA directory.
*/
#endif
