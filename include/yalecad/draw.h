/* ----------------------------------------------------------------- 
FILE:	    draw.h                                       
DESCRIPTION:include file for draw programs and utilities
CONTENTS:   typedefs, and external routines for draw facility
DATE:	    Jan 25, 1989 - major rewrite of version 1.0
REVISIONS:  Jan 27, 1989 - added frame num
	    Oct 8,  1989 - separated menu reading function for
		user friendliness.
	    May 12, 1990 - added TWsetFrame to include file.
	    Oct  5, 1990 - added prototypes.
	    Oct 12, 1990 - added TWforceRedraw.
	    Fri Jan 25 15:38:41 PST 1991 - fixed inconsistent def.
	    Sun Feb 17 17:11:57 EST 1991 - added 3D graphics functions.
	    Tue May 21 17:14:39 CDT 1991 - fixed problem with
		initgraphics prototype.
----------------------------------------------------------------- */
#ifndef DRAW_H
#define DRAW_H

#ifndef lint
static char YdrawId[] = "@(#) draw.h (Yale) version 1.20 3/6/92" ;
#endif

#include <yalecad/base.h>  /* need for BOOL definitions used below */

/****  THE MODES *******/
#define TWDRAWONLY   0
#define TWWRITEONLY  1
#define TWWRITENDRAW 2

/* defininition necessary for menus */
typedef struct {
    char *item ;         /* menu or menu item */
    char *bool_item ;    /* used for Boolean items */
    BOOL bool_init ;     /* which item is initialized */
    BOOL menuNotItem ;   /* true if menu; false if menu item */
    INT  action_index ;  /* when selected returns this number */
    INT  action_indexb ; /* when selected returns this number */
} TWMENUBOX, *TWMENUPTR ;

/* the switchable routines */
extern VOID (*_TWdrawRect)(P7( INT ref_num, INT x1, INT y1, INT x2, INT y2, INT color,
		    char *label )) ;
extern VOID (*_TWdrawLine)(P7( INT ref_num, INT x1, INT y1, INT x2, INT y2, INT color,
		    char *label )) ;
extern VOID (*_TWdrawArb)(P3( INT ref_num, INT color, char *label )) ;

/* make calls easier for the user so he doesn't know that the */
/* routines are switchable according to mode */
#define TWdrawRect(a_xz, b_xz, c_xz, d_xz, e_xz, f_xz, g_xz )\
    (  (*_TWdrawRect)(a_xz, b_xz, c_xz, d_xz, e_xz, f_xz, g_xz) )
#define TWdrawLine(a_xz, b_xz, c_xz, d_xz, e_xz, f_xz, g_xz )\
    (  (*_TWdrawLine)(a_xz, b_xz, c_xz, d_xz, e_xz, f_xz, g_xz) )
#define TWdrawArb(a_xz, b_xz, c_xz )\
    (  (*_TWdrawArb)(a_xz, b_xz, c_xz) )
#define TWdrawPin(a_xz, b_xz, c_xz, d_xz, e_xz, f_xz, g_xz )\
    (  (*_TWdrawRect)(a_xz, b_xz, c_xz, d_xz, e_xz, f_xz, g_xz) )
#define TWdrawNet(a_xz, b_xz, c_xz, d_xz, e_xz, f_xz, g_xz )\
    (  (*_TWdrawLine)(a_xz, b_xz, c_xz, d_xz, e_xz, f_xz, g_xz) )
#define TWdrawCell(a_xz, b_xz, c_xz, d_xz, e_xz, f_xz, g_xz )\
    (  (*_TWdrawRect)(a_xz, b_xz, c_xz, d_xz, e_xz, f_xz, g_xz) )

/* new defines for 3D graphics */
#define TW3DdrawAxis()                    _TW3DdrawAxis( TRUE )
#define TW3Darb_init()                    TWarb_init()
#define TW3DdrawArb( ref, color, label )  TWdrawArb( ref, color, label )
#define TW3Darb_addpt( x1, y1, z1 )            \
{   DOUBLE X1, Y1 ; \
    TW3Dperspective( (DOUBLE)x1, (DOUBLE)y1, (DOUBLE)z1, &X1, &Y1 ); \
    TWarb_addpt( (INT)X1, (INT)Y1 ); \
}

#define TW3DdrawLine( ref_num, x1, y1, z1, x2, y2, z2, color, label) \
{   DOUBLE X1, Y1, X2, Y2 ;  \
    TW3Dperspective( (DOUBLE)x1, (DOUBLE)y1, (DOUBLE)z1, &X1, &Y1 ); \
    TW3Dperspective( (DOUBLE)x2, (DOUBLE)y2, (DOUBLE)z2, &X2, &Y2 ); \
    TWdrawLine(ref_num,(INT)X1,(INT)Y1,(INT)X2,(INT)Y2,color, label) ; \
}

extern  _TW3DdrawAxis( P1(BOOL drawNotErase) ) ;


/******** FUNCTIONS NORMALLY USED BY GRAPHIC PROGRAM USERS *********/
extern BOOL TWinitGraphics( P7(INT argc, char *argv[], INT numC, char *colors[],
		BOOL dumpOnly, TWMENUPTR menu, INT (*refresh_func)() )) ;
/* 
Function:
    Initialization of graphics package.  Needs to be called first.
    Argc is the number of arguments passed on the command line.
    Argv is the argument vector.
    Colors is an array of strings of the desired colors.  The
    colors must agree with the standard color map of the machine and
    in addition element 1 must be "white" and element two "black". Also
    element 0 is not used at this time.  Use TWstdcolors to use 
    the graphics packages builtin standard colors.
    NumC is the number of colors in the color array counting white and
    black.
    DumpOnly should normally be set to FALSE.  It should be only set to
    TRUE if you wish to only dump the DATA to the DATADIRECTORY.
    MenuPath is the pathname of the menu file.
    Refresh_func is the address of the function which repaints
    the entire drawing area.
    NOTE: it is advised that the TimberWolf standard colors be used if
    possible.  Use TWstdcolors() for argument colors and TWnumcolors()
    for numC arg and use include file <yalecad/colors.h>.
*/

extern BOOL TWinitParasite( P8( INT argc, char *argv[], INT numC, char **colors,
    BOOL dumpOnly,TWMENUPTR menu, INT (*refresh_func)(), INT w) ) ;
/*
Function:
    Take over the control of a window already created by
    TWinitGraphics by another process.  One additional parameter to the
    TWinitGraphics argument list, windowId which is the top window Id.
    It is found by calling TWsaveState in the calling processs.
*/

extern TWcloseGraphics() ;
/* 
Function:
    Closes graphics display and performs cleanup operations.
    Should be performed before end of program.
*/

extern TWsetMode( P1(INT mode) ) ;
/* 
Function:
    Allows user to change mode during run.  Helpful to dump the current
    state to the binary files and then continue drawing to the screen.
    WARNING: This should only be used if TWinitGraphics was called with
    mode TWWRITENDRAW; otherwise, error results.
*/

extern BOOL TWcheckServer() ;
/* 
Function:
    Returns TRUE if a connection can be made to server. False otherwise.
*/

/* The following draw routines have been defined using macro calls */
/* so it is convenient for the user.  Use the form shown in comments. */

/* TWdrawRect(ref_num, x1, y1, x2, y2, color, label ) */
/*
Arguments: 
    INT ref_num, x1, y1, x2, y2, color ;
    char *label ;
Function:
    Draws a rectangle to screen and/or file.  
    Ref_num is a reference number to associate rectangle - currently
    not used.
    X1, y1, x2, y2 are the lower left, upper right points of rectangle.
    Color is the element number in color array. For example white is 1
    and black is 2.
    Label is optional - a label is attached to figure if non-null.
*/

/* TWdrawLine(ref_num, x1, y1, x2, y2, color, label ) */
/*
Arguments: 
    INT ref_num, x1, y1, x2, y2, color ;
    char *label ;
Function:
    Draws a line to screen and/or file.  
    Ref_num is a reference number to associate line - currently
    not used.
    X1, y1, x2, y2 are the two points of the line.
    Color is the element number in color array. For example white is 1
    and black is 2.
    Label is optional - a label is attached to figure if non-null.
*/

/* TWdrawNet(ref_num, x1, y1, x2, y2, color, label ) */
/*
Arguments: 
    INT ref_num, x1, y1, x2, y2, color ;
    char *label ;
Function:
    Draws a net to screen and/or file.  
    Ref_num is a reference number to associate all lines of a net - 
	currently not used.
    X1, y1, x2, y2 are the two points of the line of a net.
    Color is the element number in color array. For example white is 1
    and black is 2.
    Label is optional - a label is attached to figure if non-null.
*/

/* TWdrawCell(ref_num, x1, y1, x2, y2, color, label ) */
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

/* TWdrawPin(ref_num, x1, y1, x2, y2, color, label ) */
/*
Function:
    Draws a pin to screen and/or file.  
    Ref_num is a reference number to associate pin - currently
	not used.
    X1, y1, x2, y2 are the lower left, upper right points of pin.
    Color is the element number in color array. For example white is 1
    and black is 2.
    Label is optional - a label is attached to figure if non-null.
*/

extern TWarb_init() ;
/*
Arguments:
    none
Function:
    Start a new arbitrary figure.
*/

extern TWarb_addpt( P2( INT xpos, INT ypos ) ) ;
/*
Function:
    Add a new point to the current arbitrary figure 
*/

/* TWdrawArb( ref_num, color, label ) */
/*
Function:
    Draws a arbitrary figure to screen and/or file.  
    Ref_num is a reference number to associate figure - currently
	not used.
    The points of the arbitrary figure are first added using TWarb_init
    and the TWarb_addpt.  There is no need to close the figure.
    Color is the element number in color array. For example white is 1
    and black is 2.
    Label is optional - a label is attached to figure if non-null.
*/

/* copy pixmap to screen and flush screen output buffer */
extern TWflushFrame() ;
/*
Arguments: None.
Function:
    Flush the screen output and/or file buffer.
    This call must be made after all data of a screen is written, that is
    after all TWdraws have performed.
*/

extern TWstartFrame() ;
/*
Arguments: None.
Function:
    Start a new screen and/or file frame.
    This call must be made before data is written, that is
    before any TWdraws are performed for a given screen or frame.
*/

/********** ROUTINES USED BY SCREEN GRAPHICS ONLY ******************/
extern TWzoom() ;
/* 
Arguments: None
Function:
    Performs a zoom in main graphics window.
*/

extern TWtranslate() ;
/* 
Arguments: None
Function:
    Translate center to picked or entered point in main graphics window.
*/

extern TWfullView() ;
/* 
Arguments: None
Function:
    Returns to full screen after zoom.  Full screen size is size
    determined by TWsetWindow.
*/

extern TWsetwindow( P4( INT left, INT bottom, INT right, INT top ) ) ; 
/* 
Function:
    set the boundary of the visible window according to user coordinates
    Must call this function before draws.
*/

extern TWcolorXOR( P2( INT color, BOOL exorFlag ) ) ;
/* 
Function:
    set a particular colors draw function.  If exorFlag is set to TRUE,
    then X will exor the next drawing with the current contents of the
    display.  If exorFlag is set to FALSE, X will copy the new figure
    over any current contents blocking the view.  The default is copy mode.
*/

extern TWhighLightRect( P4( INT x1,INT y1,INT x2,INT y2 )) ;
/* 
Function:
    Highlight the given area in black.
*/

extern TWsync( ) ;
/* 
Arguments: None
Function:
    Make program wait until XServer has processed all requests.  This
    function should not be need by the user normally.  It is used
    internally in the graphics module.
*/

extern TWmoveRect( P6( INT *x1, INT *y1, INT *x2, INT *y2, INT ptx, INT pty )) ;
/*
Function:
    Draw ghost figure of rectangle as the user moves it on the screen.
    X1, y1, x2, y2 are pointer to the original position of the rectangle
    in user data coordinates.  Ptx, and pty are the reference point of
    the mouse pointer.  The figure will move relative to the offset 
    between the pointer and the given rectangle coordinates.  The
    routine will return in x1, y1, x2, y2 the new position of the
    rectangle when the mouse button is pushed.
*/


/********** MENU ROUTINES-USED BY SCREEN GRAPHICS ONLY ******************/
extern INT TWcheckMouse() ;
/* 
Arguments: None
Function:
    Check to see if mouse button was clicked in menu window.  If so,
    put up appropriate menu and wait for user response.  Return integer
    value corresponding to the function value given in the menu file.
*/

extern TWgetPt( P2( INT *x, INT *y )) ;
/* 
Function:
    Wait for user to enter point with a mouse pointer.  Returns x,y
    position of pointer when clicked in user coordinate system.
*/

extern TWmessage( P1( char *message ) ) ;
/* 
Function:
    Write messsage to the message window.
*/

extern char *TWgetString( P1( char *directions ) ) ;
/* 
Function:
    Write directions on what to do to message window and then return
    back a string in what the user entered in the window.
*/


extern BOOL TWgetPt2( P2( INT *x, INT *y ) ) ;
/* 
Function:
    Wait for user to enter point with a mouse pointer or by entering
    string in message window.  Returns x,y position in user coordinate 
    system.  Returns TRUE (1) if user entered data from keyboard and
    FALSE (0) if entered by picking with the mouse.
*/

/* check to see if main window has been covered by another window */
extern BOOL TWcheckExposure() ;
/* 
Arguments:None 
Function:
    Check to see if main window has been covered by another window.
    Returns TRUE (1) if this has occurred and FALSE (0) otherwise.  
    User uses this knowledge for redraw of window data.
*/

extern INT TWsaveState() ;
/* 
Arguments:None 
Function:
    Save the graphics state before calling a grahics parasite.  Returns
    the window ID which is needed in the TWinitParasite argument list.
*/

extern TWrestoreState() ;
/* 
Arguments:None 
Function:
    Return the graphics state to the state before the graphics parasite
    was called.
*/

/* check to see if main window has been requested to change size */
extern TWcheckReconfig() ;
/* 
Arguments:None 
Function:
    Check to see if main window has been requested to change size 
    and perform necessary modifications to windows.  Note: normally
    the user will not need to call this function since it is performed
    during each call of TWcheckMouse.  
*/


extern TWsetFrame( P1(INT number) ) ;
/* 
Function:
    Set the dump file to the given frame number.  Valid frame numbers start
    at 1.  If 0 is passed, the frame will be set to the next available frame
    in the DATADIR.
*/

extern TWMENUPTR TWread_menus( P1(char *filename) ) ;
/* 
Function:
    This convenience function reads a menu file and create the menu
    box structure necessary for TWinitgraphics.  Upon execution of
    the function,  a menus.h file will be created in the current 
    directory with all the necessary definitions for creating the
    menus.
*/

extern TWforceRedraw() ;
/* 
Function:
    This function forces a redraw by sending an exposure event
    to the server.  This is added so the user can control
    exposure events.
*/

extern TWdrawString( P4(INT x, INT y, INT color, char *label ) ) ;
/* 
Function:
    Draw a string left justified from the given location.
*/

/* *************** NEW 3D TRANSFORMATIONS ********************* */
/* TW3DdrawAxis()  */
/* 
Function:
    TW3DdrawAxis draws the x y z axis.
*/

extern VOID TW3Dnormal_view() ;
/* 
Function:
    Turn off the 3D viewing.
*/

extern VOID TW3DsetCamera() ;
/* 
Function:
    Turn on the 3D viewing.  The view is set with the mouse.
*/

/*------------------
  Perform a 3D transformation.  
  ------------------*/
extern VOID TW3Dperspective(P5(DOUBLE x, DOUBLE y, DOUBLE z, DOUBLE *pX, DOUBLE *pY));
/*
  Perform a 3D transformation.  
*/

extern VOID TW3DperspectiveOn() ;
/* 
Function:
    Turn on the 3D perspective.  Make things farther away smaller.
*/

extern VOID TW3DperspectiveOff() ;
/* 
Function:
    Turns off the 3D perspective.
*/

/* TW3DdrawLine( INT ref_num, INT x1, INT y1, INT z1, 
		 INT x2, INT y2, INT z2, INT color, char *label) */
/*
Function:
    Draws a line in 3 dimensions.
*/

extern INT TW3DdrawCube( P9(INT ref_num, INT x1, INT y1, INT z1,
    INT x2, INT y2, INT z2, INT color, char *label ) ) ;
/* 
Function:
    Draws a 3 dimensional cube.
*/

/* TW3Darb_init()  */
/*
Function:
    Starts an arbitrary figure in 3 dimensions.
*/

/* TW3Darb_addptr()  */
/*
Function:
    Adds another point to current arbitrary figure in 3D.
*/

/* TW3DdrawArb( INT ref, INT color, char *label )  */
/*
Function:
    Finished arbitrary point and draws it to the screen.
*/

extern TWarb_fill( P1(BOOL flag ) ) ;
/*
Function:
    If flag is true, arbitrary figures (both 2D and 3D) will be filled.
    Otherwise, no fill will be added.
*/

#endif /* DRAW_H */
