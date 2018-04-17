/* ----------------------------------------------------------------- 
"@(#) info.h (Yale) version 3.10 1/30/92"
FILE:	    info.h                                       
DESCRIPTION:include file for graphics screen routines
CONTENTS:   typedefs, and external routines for TWinfo record.
DATE:	    Mar 21, 1989 - original coding moved from draw.h
REVISIONS:  Sep 23, 1989 - added color fields for TWtoggleColor()
	    Oct 12, 1990 - modified structure to accommodate 4
		draw windows.
	    Dec  7, 1990 - added stipple switch to information.
	    Mon Jan  7 18:17:44 CST 1991 - made SAFE_WAIT_TIME
		user programmable for slow machines.
	    Thu Mar  7 01:23:40 EST 1991 - added refresh function
		so that dialog box works correctly.
----------------------------------------------------------------- */
#ifndef INFO_H
#define INFO_H

#include <yalecad/base.h>

#define GRAPHICS        "graywolf"  /* the name of prog in Xdefaults */
#define MENUHEIGHT      20            /* give menu extra pixels in width */

#ifdef YDRAW_VARS
#define EXTERN 
#else
#define EXTERN extern 
#endif

typedef struct {
    GC          *graphicContext ;  /* array of color contexts */
    Display     *dpy ;             /* the display */
    Window      backWindow;        /* the backing window */
    Window      drawWindow;        /* the main drawing window */
    Window      rootWindow;        /* the root window */
    INT         screen;            /* the current screen */
    char        *fontname ;        /* main font window font */
    XFontStruct *fontinfo ;        /* font information */
    DOUBLE      scaleFactor ;      /* scale data to screen dim.*/
    INT         xoffset ;          /* data offset to x screen dim.*/
    INT         yoffset ;          /* data offset to y screen dim.*/
    INT         winwidth ;         /* window width in pixels */
    INT         winheight ;        /* window height in pixels */
    INT         winx ;             /* window origin x from ul */
    INT         winy ;             /* window origin y from ul */
    BOOL        *colorOn ;         /* array of whether color is on */
    BOOL        stipple ;          /* whether stipple has been requested*/
    INT         numColors ;        /* number of colors */
    char        **colors ;         /* names of the user given colors */
    Pixmap      pixmap ;           /* pixmap for redraw after menus */
    INT         (*refresh_func)() ;/* store the refresh function */
} TWINFO, *TWINFOPTR ;

/* ****************** GLOBALS ************************** */
/* GLOBAL VARIABLES */
EXTERN INT TWsafe_wait_timeG ;/* time to wait before redraw */

extern TWINFOPTR TWgetDrawInfo() ; /* TW library routines use this */
extern TWsetDrawInfo( P3( INT winheight, INT winwidth, Pixmap pixmap )) ;
extern Window TWgetWindowId( P2(Display *dpy, Window backwindow) ) ;
extern BOOL TWinitMenuWindow( P1(TWMENUPTR menu_fields) ) ;
extern XFontStruct *TWgetfont( P2(char *fname, Font *font) ) ;

#undef EXTERN

#endif /* INFO_H */
