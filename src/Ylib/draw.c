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
FILE:	    draw.c                                       
DESCRIPTION:graphic drawing routine both screen and file.
    ********************************************************
    THERE ARE TWO FAMILIES OF ROUTINES IN THIS MODULE 
    1) Graphic routines that write to the CRT screen 
    2) Routines which write to a set of binary file later
	to be drawn by an independent program.
    The user can select whether he wants either of the routines
	or both.
    ********************************************************
DATE:	    Jan 25, 1989 - major rewrite of version 1.0
CONTENTS:   BOOL TWcheckServer()
	    TWsetMode( mode ) 
		INT mode ;
	    BOOL TWinitGraphics(argc,argv,numC,colors,dumpOnly,menu,refresh_func)
		INT  argc, numC, (*refresh_func)() ;
		char *argv[], **colors ;
		BOOL dumpOnly ;
		TWMENUPTR menu ;
	    BOOL TWinitParasite(argc,argv,numC,colors,dumpOnly,menu,refresh_func,w)
		INT  argc, numC, (*refresh_func)() ;
		char *argv[], **colors ;
		BOOL dumpOnly ;
		TWMENUPTR menu ;
		Window w ;
	    static BOOL TWinit(argc,argv,numC,desiredColors,dumpOnly,
		menu,refresh_func)
		INT  argc, numC, (*refresh_func)() ;
		char *argv[], **colors ;
		BOOL dumpOnly ;
		TWMENUPTR menu ;
	    TWINFOPTR TWgetDrawInfo() 
	    TWsetDrawInfo( winheight, winwidth ) 
		INT winheight, winwidth ;
	    TWcloseGraphics()
	    TWzoom()
	    TWfullView()
	    TWsetwindow( left, bottom, right, top ) 
		INT left, bottom, right, top ;
	    TWtranslate()
	    TWflushFrame()
	    TWsync()
	    static void initcolors( desiredColorArray, numC )
		char **desiredColorArray ;
		INT  numC ;
	    static startDFrame()
	    static VOID drawDLine(ref,x1,y1,x2,y2,color,label)
		INT	ref, color ;
		register INT	x1,y1,x2,y2 ;
		char	*label ;
	    static VOID drawDRect(ref,x1,y1,x2,y2,color,label)
		INT	ref, color ;
		register INT	x1,y1,x2,y2 ;
		char	*label ;
	    TWarb_init()
	    TWarb_addpt( xpos, ypos )
		INT xpos, ypos ;
	    static VOID drawDArb( ref, color, label )
		INT	ref, color ;
		char	*label ;
	    TWhighLightRect( x1,y1,x2,y2 )
		register INT	x1,y1,x2,y2 ;
	    TWmoveRect( x1, y1, x2, y2, ptx, pty )
		INT *x1, *y1, *x2, *y2, ptx, pty ;
	    static XFontStruct *TWgetfont( fname, font )
		Font *font ;
	    TWstartFrame()
	    static closeFrame()
	    TWsetFrame( number )
		INT number ;
	    static drawWLine( ref_num,x1,y1,x2,y2,color,label)
		INT     ref_num ;
		INT	x1,y1,x2,y2,color ;
		char	*label;
	    static drawWRect( ref_num, x1,y1,x2,y2,color,label)
		INT     ref_num ;
		INT	x1,y1,x2,y2, color;
		char	*label;
	    static drawWArb( ref, color, label )
		INT	ref, color ;
		char	*label ;
REVISIONS:  Jan 31, 1989 - added screen routines.
	    Feb 21, 1989 - initial graphic routines.
	    Feb 27, 1989 - added no graphics conditional compile.
	    Mar 22, 1989 - made TWdrawCell and TWdrawRect separate
		routines.  Added info.h TWinitGraphics now returns
		a BOOLEAN value on whether it was successful.
	    Mar 23, 1989 - made program exit if problem.  Also fixed
		pointer problem on foreign machines.
	    Apr  1, 1989 - fixed black/white stipple pattern problem.
		Added parasite mechanism which allows another program
		to access main window.
	    Apr  9, 1989 - handle change of mode correctly now.
	    Apr 16, 1989 - added XGetDefaults to get user defined
		screen position.
	    Apr 27, 1989 - update for Y routines.
	    May 11, 1989 - fixed parasite problem.
	    Sep 23, 1989 - added color switch for TWtoggleColor()
	    Oct  2, 1989 - menus now can be compiled into program.
	    Nov 15, 1989 - TWsetFrame now can reset to last frame.
	    Mar  3, 1990 - added zoom out and removed clear screen
		in parasite mode.
	    Aug  1, 1990 - rewrote to be MOTIF compatible.
	    Sep 25, 1990 - fixed B&W mode. Made 9x15 default font.
	    Oct  5, 1990 - rewrote using prototypes.  Added 
		TWcolorXOR.  Fixed problem with TWfullview.
		Also added dumpOnly mode.  Added arbitrary rectangle
		drawing.
	    Oct 12, 1990 - rewrote draw routines to use 4 windows
		and added pixmap for fast redraws.  Also added
		TWforceRedraw.
	    Oct 14, 1990 - fixed parasite and dump arbitrary figures.
	    Oct 21, 1990 - fixed parasite resizing.
	    Nov  4, 1990 - added Xdefault bw.
	    Dec  5, 1990 - added stipple to info record.
	    Mon Jan  7 18:17:44 CST 1991 - made SAFE_WAIT_TIME
		user programmable for slow machines.
	    Sun Feb 17 17:12:39 EST 1991 - added 3D graphics functions.
	    Fri Feb 22 23:38:02 EST 1991 - fixed sensitivity of
		3D camera.
	    Sat Feb 23 04:57:21 EST 1991 - fixed a very pesky bug with
		the pixmap.
	    Thu Mar  7 01:26:04 EST 1991 - now store the refresh function.
	    Thu Apr 18 00:36:25 EDT 1991 - fixed argument to refresh
		function and fixed sun XOR.
	    Sun Apr 28 22:06:15 EDT 1991 - set colors right for
		reverse video.
	    Wed May  1 18:55:13 EDT 1991 - added TWget_arb_fill.
	    Mon Aug 12 15:52:55 CDT 1991 - added rectangular
		fill switch.
	    Sun Nov  3 12:51:46 EST 1991 - fixed gcc complaints.
	    Sun Jan 26 03:38:49 EST 1992 - fixed window manager hint problem.
	    Thu Mar  5 17:01:09 EST 1992 - added clipping for faster draw
		during zoom.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) draw.c (Yale) version 3.41 3/10/92" ;
#endif

#ifndef NOGRAPHICS

#include <stdio.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>

#include <yalecad/base.h>
#include <yalecad/file.h>
#include <yalecad/message.h>
#include <yalecad/hash.h>
#include <yalecad/string.h>
#include <yalecad/draw.h>
#include <yalecad/debug.h>
#include <yalecad/buster.h>
#include <yalecad/dbinary.h>
#include <yalecad/project.h>

#define YDRAW_VARS
#include "info.h"

#define EXPECTEDCOLORS  201  /* expected number of colors in table */
#define NODIRECTORY     1
#define NOINIT          2

#define OBSCURED	(1)
#define UNOBSCURED	(0)

/* the place where these graphic contexts are stored */ 
#define HIGHLITE   	(0)
#define WHITE    	(1)
#define BLACK   	(2)

#define DIV_2           >> 1

/****  THE MODES AND CHOICES *******/
static  INT        modeS ; /* determines which mode we are in */

/* the global routines - seen by outside world */
VOID (*_TWdrawRect)(P7( INT ref_num, INT x1, INT y1, INT x2, INT y2, INT color,
		    char *label )) ;
VOID (*_TWdrawLine)(P7( INT ref_num, INT x1, INT y1, INT x2, INT y2, INT color,
		    char *label )) ;
VOID (*_TWdrawArb)(P3( INT ref_num, INT color, char *label )) ;


/* the local routines seen by the routines in this file only */
static VOID drawDRect() ;
static VOID drawWRect() ;
static VOID drawDArb() ;
static VOID drawWArb() ;
static VOID drawDLine() ;
static VOID drawWLine() ;
static VOID initcolors( P2(char **desiredColorArray,INT  numC ) ) ;
static closeFrame(P1(void)) ;
static VOID set_viewing_transformation() ;
extern VOID TW3Dperspective( P5(DOUBLE x, DOUBLE y, DOUBLE z, 
    DOUBLE *pX, DOUBLE *pY ) ) ;
static BOOL TWinit( P7(INT argc,char *argv[],INT numC,char **desiredColors,
		    BOOL dumpOnly, TWMENUPTR menu, INT (*refresh_func)() ) ) ;
static VOID set_clip_window( P4(INT l, INT r, INT b, INT t) ) ;

/********** THE CRT ROUTINE STATIC DEFINITIONS *************/
static TWINFO      infoBoxS ;          /* information for other modules*/
static XColor      *colorArrayS ;      /* array of color available */
static GC          *graphicContextS ;  /* array of color contexts */
static INT         numColorS;          /* the number of colors in table */
static INT         borderColorS=BLACK; /* the default border color */
static Display     *dpyS;              /* the display connection */
static Window      backS;              /* the back window */
static Window      drawS;              /* the current window */
static Window      parentS;            /* the parent window */
static Colormap    cmapS ;             /* default color map */
static Pixmap      pixmapS ;           /* offscreen copy of data */
static INT         screenS;            /* the current screen */
static Font        fontS ;             /* selected font */
static XFontStruct *fontinfoS ;        /* font information */
static BOOL        fullViewS ;         /* used for zoom window */
static BOOL        dumpOnlyS ;         /* if TRUE only dump to file */
static BOOL        reverseS = FALSE;   /* reverse video */
static BOOL        parasiteS ;          /* this process is a parasite? */
static BOOL        colorS ;            /* color display ??? */
static BOOL        stippleS = FALSE ;  /* stipple pattern on */
static BOOL        displayOpenS=FALSE; /* display has been opened ??? */
static BOOL        *colorOnS ;         /* is color turned on ??? */
static BOOL        fillArbS = TRUE ;   /* whether to fill arbitrary figures */
static BOOL        rect_fillS = TRUE ; /* whether to fill rectangle */
static char        **userColorS ;      /* colors user gave us */

/* FOR 3D perspective routines */
static BOOL	   perspectiveS;    	/* perspective toggle flag */
static DOUBLE      phiS, thetaS, rhoS;  /* 3D view point parameters */
static DOUBLE      radiansPerDegreeS;
static DOUBLE      v11S, v12S, v13S,
                   v21S, v22S, v23S,
                         v32S, v33S,
		               v43S;

/*
 * Initial Window Postion & Dimensions for half screen.
 */
#define INT_WIDTH	(500)
#define INT_HEIGHT	(500)
#define INT_X		(1)
#define INT_Y		(1)
static DOUBLE      scaleFactorS = 1.0 ;/* scale data to screen dim.*/
static INT         xoffsetS ;          /* data offset to x screen dim.*/
static INT         yoffsetS ;          /* data offset to y screen dim.*/
static DOUBLE      oldscaleFactorS ;   /* remember old for zoom */
static INT         oldxoffsetS ;       /* remember x for zoom return */
static INT         oldyoffsetS ;       /* remember y for zoom return */
static unsigned int winwidthS ;        /* draw window width in pixels */
static unsigned int winheightS ;       /* draw window height in pixels */
static int 	   winxS ;             /* window origin x from ul */
static int         winyS ;             /* window origin y from ul */
static INT         leftS ;             /* left of user data */
static INT         rightS ;            /* right of user data */
static INT         topS ;              /* top of user data */
static INT         bottomS ;           /* bottom of user data */
static INT lS, rS, bS, tS ;            /* current user data */
static INT arblS, arbrS, arbbS, arbtS ;/* current user data */
/* used to draw arbitrary rectilinear shapes */
#define EXPECTEDPTS 12
static INT numptS ;          /* current number of points */
static INT ptAllocS = 0 ;    /* size of point array */
static XPoint *ptS = NIL(XPoint *) ; /* array of pts for arb figure */



/********** STATIC DEFINITIONS FOR MIXED FUNCTIONS *************/
static  char *dirNameS ;      /* pathname including DATA directory */
static  BOOL initS = FALSE ;  /* tells whether initialization performed */
static  INT  frameCountS ;    /* current number of display frames */
static  BOOL frameOpenS ;     /* tells whether frame files are open */

BOOL TWcheckServer()
{
    char *hostmon ;
    char *Ygetenv() ;

    /* get host name from environment variable */
    if(!(hostmon = Ygetenv("DISPLAY"))) {
	M( WARNMSG,"TWcheckServer","Cannot get DISPLAY environment variable\n");
	return( FALSE ) ;
    }
    /* open display */
    if(!(dpyS = XOpenDisplay(hostmon))) {
	M( WARNMSG,"TWcheckServer","Cannot connect to X server\n");
	return( FALSE ) ;
    } else {
	XCloseDisplay( dpyS ) ;
    }
    return( TRUE ) ;
} /* end TWcheckServer */

TWsetMode( mode ) 
INT mode ;
{
    if( dumpOnlyS && mode != TWWRITEONLY ){
	D( "TWsetMode",
	    fprintf( stderr, 
	    "Drawing mode should not be changed in dumpOnly operation\n");
	) ;
	return ;
    }
    modeS = mode ;
    /* select routines */
    /* ------------------------------------------------------
       NOTE THAT WRITE ROUTINES ALL CHECK MODE FOR TWWRITENDRAW
       CASE AND CALL THE DRAW ROUTINES. It is done this way
       since write routines take alot of time anyway.
       ALSO note that TWinitGraphic and TWcloseGraphics are
       implemented as mixed families, since they are only called
       once each during a run.
       ------------------------------------------------------ */
    switch( mode ){
	case TWDRAWONLY:
	    /* pick from the D routines */
	    _TWdrawRect   = drawDRect ;
	    _TWdrawLine   = drawDLine ;
	    _TWdrawArb    = drawDArb ;
	    break ;
	case TWWRITEONLY:
	case TWWRITENDRAW:
	    /* pick from the W routines */
	    _TWdrawRect   = drawWRect ;
	    _TWdrawLine   = drawWLine ;
	    _TWdrawArb    = drawWArb ;
	    break ;
    }

} /* end TWsetMode */

/* start a new window system */
BOOL TWinitGraphics(argc,argv,numC,colors,dumpOnly,menu,refresh_func)
INT argc;
char *argv[];
char **colors ;
BOOL dumpOnly ;
INT  numC ;
TWMENUPTR menu ;
INT (*refresh_func)() ;
{
    parasiteS = FALSE ;

    radiansPerDegreeS = acos( (double) 0.0 ) / (double) 90.0 ;
    
    return(TWinit(argc,argv,numC,colors,dumpOnly,menu,refresh_func));
} /* end TWinitGraphics */

/* TWinitParasite takes over windows that were already opened */
BOOL TWinitParasite(argc,argv,numC,colors,dumpOnly,menu,refresh_func,w)
INT argc;
char *argv[];
INT  numC ;
char **colors ;
BOOL dumpOnly ;
TWMENUPTR menu ;
INT (*refresh_func)() ;
INT w ;
{
    parasiteS = TRUE ;
    backS = (Window) w ;
    return(TWinit(argc,argv,numC,colors,dumpOnly,menu,refresh_func));
} /* end TWinitParasite */

static BOOL TWinit(argc,argv,numC,desiredColors,dumpOnly,menu,refresh_func)
INT argc;
char *argv[];
INT  numC ;
char **desiredColors ;
BOOL dumpOnly ;
TWMENUPTR menu ;
INT (*refresh_func)() ;
{

    XSetWindowAttributes attr;
    XWindowAttributes wattr;
    XEvent event ;            /* describes event */
    char *font ;              /* user font request */
    char *hostmon ;           /* the host name of display */
    char *Ygetenv() ;         /* get the environment variables */
    char *reply ;             /* get xdefaults */
    INT depth;
    BOOL saveflag ;
    Window root;
    XSizeHints hints ;	      /* setup hints for window manager */
    char *Yfixpath() ;
    char *winstr ; /* position of Xdefault window */
    long event_mask ;  /* used to set input selection to window */
    INT  m ;       /* mask for determining window position*/

    dumpOnlyS = dumpOnly ; /* save for rest of execution */
    if(!(dirNameS = Ygetenv( "DATADIR" ))){
	/* find fullpathname of data directory */
	/* dirNameS = Yfixpath( "./DATA", FALSE ) ; */
	/* Continue with no data dumps;  this is okay! */
	dirNameS = NULL;
    }

    /* first look for data directory */
    if((dirNameS != NULL) && !(YdirectoryExists(dirNameS)) ){
	/* always send this message to the screen */
	saveflag = Ymessage_get_mode() ;
	Ymessage_mode( M_VERBOSE ) ;
	M(MSG, NULL,"\n\n" ) ;
	sprintf( YmsgG,"Not Fatal:can't find data directory:%s\n", dirNameS );
	M(ERRMSG,"TWinitGraphics",YmsgG ) ;
	M(MSG,NULL, "\tKill program and use \"setenv DATADIR <directoryPath>\" ");
	M(MSG,NULL, "to set data directory\n");
	M(MSG,NULL, "\tor allow program to continue with the inability to " ) ;
	M(MSG,NULL, "perform screen dumps\n");
	Ymessage_mode( saveflag ) ;
	dirNameS = NULL ;
    }

    /* set count to zero */
    frameCountS = 0 ;

    frameOpenS = FALSE ;

    /* -------------------------------------------------------
	Set the mode of the graphic routines.
	mode                      Function
	0 TWDRAWONLY      Draw graphics to screen.
	1 TWWRITEONLY     Write graphics to draw program binary files.
	2 TWWRITENDRAW    Draw graphics to screen and write draw files.
    For mode 0 hostmon must be valid.
    For mode 1 dataDir (path of data directory) must be valid.
    For all modes desiredColors must be valid.
    --------------------------------------------------------- */
    if( dumpOnlyS ){
	/* we are done for the dump_graphics mode */
	initS = TRUE ;
	TWsetMode( TWWRITEONLY ) ;  /* always enable both modes */
	return ;
    } else {
	/* OTHERWISE INITIALIZE BOTH MODES */
	TWsetMode( TWWRITENDRAW ) ;  /* always enable both modes */
    }
    /*****  BEGIN SCREEN GRAPHICS IN THIS ROUTINE **** */

    /* get host name from environment variable */
    if(!(hostmon = Ygetenv("DISPLAY"))) {
	M(ERRMSG,"TWinitGraphics","Could not get DISPLAY environment variable.\n");
	YexitPgm(NOINIT) ;
    }
    /* open display */
    if(!(dpyS = XOpenDisplay(hostmon))) {
	M(ERRMSG,"TWinitGraphics","Could not connect to X server.\n");
	YexitPgm(NOINIT) ;
    }
    /* get various information about display */
    screenS = DefaultScreen(dpyS);
    cmapS = DefaultColormap(dpyS,screenS);
    parentS = root = RootWindow(dpyS,screenS);
    depth = DefaultDepth(dpyS,screenS);

    /* check whether machine is color or not */
    if( (colorS = XDisplayCells( dpyS, screenS )) > 2 ){ 
	/* if color number of display cells > 0 */
	colorS = TRUE ;
	if( reply = XGetDefault( dpyS, GRAPHICS, "bw" )){
	    if( strcmp( reply, "on" ) == STRINGEQ ){
		colorS = FALSE ;
	    }
	}
    } else {
	colorS = FALSE ;
    }


    /* set font and get font info */
    if(font = XGetDefault( dpyS, GRAPHICS, "font" )){
        fontinfoS = TWgetfont( font, &fontS ) ;
	infoBoxS.fontname = font ;  
    } else {
	/* we perfer our default to be 9x15 */
        fontinfoS = TWgetfont( "9x15", &fontS ) ;
	if(!(fontinfoS )){
	    fontinfoS = TWgetfont( "fixed", &fontS ) ;
	    infoBoxS.fontname = "fixed" ;
	} else {
	    infoBoxS.fontname = "9x15" ;
	}
    }
    /* see if we should turn on the stipple pattern */
    if( reply = XGetDefault( dpyS, GRAPHICS, "stipple" )){
	if( strcmp( reply, "on" ) == STRINGEQ ){
	    stippleS = TRUE ;
	}
    }
    /* see if we should turn on/off the rectangular fill */
    if( reply = XGetDefault( dpyS, GRAPHICS, "rectangle_fill" )){
	if( strcmp( reply, "off" ) == STRINGEQ ){
	    rect_fillS = FALSE ; /* dont fill rectangles - default on */
	} 
    }
    /* see if we should turn on/off the arbitrary fill */
    if( reply = XGetDefault( dpyS, GRAPHICS, "arbitrary_fill" )){
	if( strcmp( reply, "off" ) == STRINGEQ ){
	    fillArbS = FALSE ; /* dont fill arbs - default on */
	} 
    }
    /* see if we should turn off the reverse video */
    if( reply = XGetDefault( dpyS, GRAPHICS, "reverse" )){
	if( strcmp( reply, "on" ) == STRINGEQ ){
	    reverseS = TRUE ;
	}
    }

    /* see if we need to reset the wait time to redraw */
    if( reply = XGetDefault( dpyS, GRAPHICS, "wait_time" )){
	TWsafe_wait_timeG = atoi( reply ) ;
    } else {
	TWsafe_wait_timeG = 2 ;
    }


    /* initialize position */
    if( winstr = XGetDefault( dpyS, GRAPHICS, "geometry" )){
	m = XParseGeometry( winstr,&winxS,&winyS,&winwidthS,&winheightS) ;
	if( m & XNegative ){
	    winxS = XDisplayWidth( dpyS, screenS ) + winxS ;
	}
	if( m & YNegative ){
	    winyS = XDisplayHeight( dpyS, screenS ) + winyS ;
	}
	/* these two lines insure that uses doesn't have to press */
	/* button using twm window manager */
	if( winxS == 0 ) winxS++ ;
	if( winyS == 0 ) winyS++ ;
	hints.flags = USPosition | USSize ;
    } else {
	winwidthS = INT_WIDTH;
	winheightS = INT_HEIGHT;
	winxS = INT_X;
	winyS = INT_Y;
	hints.flags = PPosition | PSize ;
    }
    /* end initializing position of window */

    XSelectInput(dpyS,root,SubstructureNotifyMask );

    attr.event_mask = StructureNotifyMask
		    | SubstructureNotifyMask
		    | VisibilityChangeMask
		    | ExposureMask;

    attr.override_redirect = FALSE ;
    attr.save_under = FALSE ;
    attr.backing_store = NotUseful ;

    if(reverseS == TRUE){ 
	attr.background_pixel = BlackPixel(dpyS,screenS);
    } else {
	attr.background_pixel = WhitePixel(dpyS,screenS);
    }

    if(!(parasiteS)){
	backS = XCreateWindow(dpyS,root,winxS,winyS,winwidthS,
		winheightS,
		0,depth,InputOutput,DefaultVisual(dpyS,screenS),
		CWEventMask|CWBackPixel|CWOverrideRedirect|
		CWSaveUnder | CWBackingStore,&attr);

	drawS = XCreateSimpleWindow( dpyS, backS, 
	    0, MENUHEIGHT, winwidthS, winheightS - 2*MENUHEIGHT,
		0L, BlackPixel(dpyS,screenS), attr.background_pixel ) ;

	event_mask = ExposureMask | ButtonPressMask ;
	XSelectInput(dpyS,drawS,event_mask);

	/* initialize colors and/or stipple patterns */
	initcolors( desiredColors, numC ) ;

	/* set the window manager hints */
	hints.x = winxS ;
	hints.y = winyS ;
	hints.width = winwidthS ;
	hints.height = winheightS ;
	XSetStandardProperties( dpyS,backS,GRAPHICS,GRAPHICS,None,argv,argc,&hints);

	XMapWindow(dpyS,backS);
	XMapRaised(dpyS,drawS);

	/* wait to get window */
	XSync(dpyS, FALSE );
	/* -------------------------------------------------------------
	   Now wait to window to become visible.  This code is necessary 
	   since some window managers (uwm) map the window as a ghost 
	   image and wait for user to resize window.  Other window 
	   managers (twm) map window as requested.  Need to accomodate
	   both.
	-------------------------------------------------------------- */
	while( TRUE ){
	    if( XCheckTypedWindowEvent(dpyS,backS,VisibilityNotify,&event)){
		if( event.xvisibility.state == VisibilityUnobscured ){
		    break ;
		}
	    }
	}

    } else { /* for a parasite case */
	/* retrieve the window information */
	drawS = TWgetWindowId( dpyS, backS ) ;
	/* set the event mask for the windows */
	event_mask = StructureNotifyMask | 
	    SubstructureNotifyMask | VisibilityChangeMask ;
	XSelectInput(dpyS,backS,event_mask);
	event_mask = ExposureMask | ButtonPressMask ;
	XSelectInput(dpyS,drawS,event_mask);
	/* initialize colors and/or stipple patterns */
	initcolors( desiredColors, numC ) ;
    }
	
    /* if the window manager added border around window we */
    /* need to save new origin of window */
    /* also if user changed size we need to get it */
    XGetWindowAttributes( dpyS, backS, &wattr ) ;
    winxS = infoBoxS.winx = wattr.x ;
    winyS = infoBoxS.winy = wattr.y ;
    winwidthS = infoBoxS.winwidth = wattr.width ;
    winheightS = infoBoxS.winheight = wattr.height - 2 * MENUHEIGHT ;
    /* default user data boarder to size of window */
    leftS = 0 ;
    bottomS = MENUHEIGHT ;
    rightS = winwidthS ;
    topS = winheightS ;
    fullViewS = TRUE ; 

    /* save the refresh function */
    infoBoxS.refresh_func = refresh_func ;

    TW3Dnormal_view() ;

    if(!(parasiteS)){
	XClearWindow(dpyS,drawS);
    }

    /* off screen copy of the data */
    pixmapS = XCreatePixmap( dpyS, drawS,
	(unsigned)winwidthS, (unsigned)winheightS, 
	XDefaultDepth(dpyS,screenS) ) ;

    if(!(TWinitMenuWindow( menu ))){
	initS = TRUE ; /* fake out TWcloseGraphics */
	TWcloseGraphics() ;
	YexitPgm(NOINIT) ;
    }


    TWdrawMenus() ;
    XFlush( dpyS ) ;

    /* initialization has been done sucessfully */
    displayOpenS = TRUE; /* display is open for business */
    initS = TRUE ;
    TWsetMode( TWDRAWONLY ) ;
    return( initS ) ;

} /* end function TWinitGraphics */

TWINFOPTR TWgetDrawInfo() 
{
    TWINFOPTR info ;

    /* put all information in record for other modules use */
    /* a inefficient attempt at object orient programming */
    info = &infoBoxS ;
    info->graphicContext = graphicContextS ;
    info->dpy = dpyS ;
    info->drawWindow = drawS ;
    info->backWindow = backS ;
    info->rootWindow = parentS ;
    info->screen = screenS ;
    info->fontinfo = fontinfoS ;   
    info->scaleFactor = scaleFactorS ; 
    info->xoffset = xoffsetS ;
    info->yoffset = yoffsetS ;
    info->winwidth = winwidthS ;
    info->winheight = winheightS ;
    info->winx = winxS ;
    info->winy = winyS ;
    info->colorOn = colorOnS ;
    info->numColors = numColorS ;
    info->colors = userColorS ;
    info->pixmap = pixmapS ;
    info->stipple = stippleS ;
    return( info ) ;
} /* end TWgetDrawInfo */


TWsetDrawInfo( winheight, winwidth, pixmap ) 
INT winheight, winwidth ;
Pixmap pixmap ;
{
    infoBoxS.winwidth = winwidthS = winwidth ;
    infoBoxS.winheight = winheightS = winheight ;
    infoBoxS.pixmap = pixmapS = pixmap ;
    fullViewS = TRUE ;
    TWsetwindow( leftS, bottomS, rightS, topS ) ;

} /* end TWsetDrawInfo */

TWforceRedraw()
{
    XEvent event ;          /* describes configuration event */

    /* set a redraw command for a zoom */
    event.xexpose.type = Expose ;
    event.xexpose.display = dpyS ;
    event.xexpose.window = drawS ;
    event.xexpose.x = 0 ;
    event.xexpose.y = 0 ;
    event.xexpose.count = 0 ;
    XSendEvent( dpyS, drawS, TRUE, ExposureMask, &event ) ;
} /* end TWforceRedraw */

TWcloseGraphics()
{

    if(!(initS )){
	fprintf(stderr,"ERROR[closeGraphics]:initialization was not" ) ;
	fprintf(stderr,"performed\n  before calling closeGraphics\n" ) ;
	YexitPgm( NOINIT ) ;
    }

    /* check to see if other files are open */
    if( frameOpenS ){
	closeFrame() ;
    }
    if( displayOpenS ){
	TWfreeMenuWindows() ;
	XCloseDisplay( dpyS ) ;
	displayOpenS = FALSE ;
    }

} /* end TWcloseGraphics */

/***********  BEGIN STRICT GRAPHICS ROUTINES ************* */
/* perform a zoom in main graphics window */
TWzoom()
{
    INT x1, y1 ; /* first point of user zoom */
    INT x2, y2 ; /* second point of user zoom */
    INT left, right, bottom, top ; /* for calculating window */
    INT span ;   /* span of design */

    TWmessage( "[ZOOM]:Pick or enter first point of bounding box:" ) ;
    TWgetPt2( &x1, &y1 ) ;
    TWmessage( "[ZOOM]:Pick or enter second point of bounding box:" ) ;
    TWgetPt2( &x2, &y2 ) ;

    if( fullViewS ){
	/* only save full view window size */
	oldxoffsetS = xoffsetS ;
	oldyoffsetS = yoffsetS ;
	oldscaleFactorS = scaleFactorS ;
	fullViewS = FALSE ;
    }

    if( x1 >= x2 && y1 >= y2 ){
	/* zoom out */
	span = rightS - leftS ;
	span *= 2 ;
	left = leftS = ( (x1 + x2 ) / 2 ) - span / 2 ;
	right = rightS = ( (x1 + x2 ) / 2 ) + span / 2 ;
	span = topS - bottomS ;
	span *= 2 ;
	bottom = bottomS = ( (y1 + y2 ) / 2 ) - span / 2 ;
	top = topS = ( (y1 + y2 ) / 2 ) + span / 2 ;

    } else {
	/* zoom in */
	left = MIN( x1, x2 ) ;
	right = MAX( x1, x2 ) ;
	bottom = MIN( y1, y2 ) ;
	top = MAX( y1, y2 ) ;
    }

    /* set new window */
    TWsetwindow( left, bottom, right, top ) ;
    /* important that we reset full view to FALSE at this point */
    /* since TWsetwindow always sets it to TRUE */
    fullViewS = FALSE ;
    TWmessage( " " ) ;

    TWforceRedraw() ;

}

/* returns to full screen after zoom */
TWfullView()
{
    if( fullViewS ){
	return ;
    }
    TWmessage( "[FULLSCREEN]: Returning to original size" ) ;
    fullViewS = TRUE ;
    infoBoxS.xoffset = xoffsetS = oldxoffsetS ;
    infoBoxS.yoffset = yoffsetS = oldyoffsetS ; 
    infoBoxS.scaleFactor = scaleFactorS = oldscaleFactorS ; 
    TWforceRedraw() ;

} /* end TWfullScreen */

/* set the window area for bar */
TWsetwindow( left, bottom, right, top ) 
INT left, bottom, right, top ;
{
    INT xspan, yspan ; /* span of data */
    DOUBLE xscaleF, yscaleF ;  /* scale data to window span */

    if( modeS == TWWRITEONLY ){
	return ;
    }
    /* save what the user gave us in case we need to use it */
    /* for a reconfiguration request - ie, resize window */
    if( fullViewS ){
	leftS = left ; rightS = right ; bottomS = bottom ; topS = top ;
    }

    xspan = ABS( right - left ) ;
    if( xspan == 0 ){
	if( initS ){
	    TWmessage( "ERROR: xspan is zero. Aborting zoom..." ) ;
	    return ;
	} else {
	    M(ERRMSG,"TWsetwindow", "xspan is zero" ) ;
	    return ;
	}
    }
    yspan = ABS( top - bottom ) ;
    if( yspan == 0 ){
	if( initS ){
	    TWmessage( "ERROR: yspan is zero. Aborting zoom..." ) ;
	    return ;
	} else {
	    M(ERRMSG,"TWsetwindow", "yspan is zero" ) ;
	    return ;
	}
    }
    /* scale data to fit to window */
    xscaleF = (DOUBLE) winwidthS / (DOUBLE) xspan ;
    yscaleF = (DOUBLE) winheightS / (DOUBLE) yspan ;
    /* pick smallest of factors for scale factor */
    scaleFactorS = MIN( xscaleF, yscaleF) ;

    /* offset from beginning of data */
    xoffsetS = - left ;
    yoffsetS = - bottom ;

    if(!(fullViewS)){
	set_clip_window( left, right, bottom, top ) ;
	lS = left ; bS = bottom ;
	if( xspan > yspan ){
	    rS = right ; 
	    tS = bottom + xspan ;
	} else if( xspan < yspan ){
	    rS = left + yspan ; 
	    tS = top ;
	} else {
	    rS = right ; 
	    tS = top ;
	}
    }

    TWinforMenus() ;
    fullViewS = TRUE ;

} /* end TWsetwindow */

static VOID set_clip_window( left, right, bottom, top ) 
INT left, right, bottom, top ;
{
    INT xspan, yspan ;

    lS = left ; bS = bottom ;
    xspan = ABS( right - left ) ;
    yspan = ABS( top - bottom ) ;
    if( xspan > yspan ){
	rS = right ; 
	tS = bottom + xspan ;
    } else if( xspan < yspan ){
	rS = left + yspan ; 
	tS = top ;
    } else {
	rS = right ; 
	tS = top ;
    }
} /* end set_clip_window() */ 

TWtranslate()
{
    INT x1, y1 ;
    INT last_xoff, last_yoff ;

    if( fullViewS ){
	/* only save full view window size */
	oldxoffsetS = xoffsetS ;
	oldyoffsetS = yoffsetS ;
	oldscaleFactorS = scaleFactorS ;
	fullViewS = FALSE ;
	set_clip_window( leftS, rightS, bottomS, topS ) ;
    }
    last_xoff = xoffsetS ; last_yoff = yoffsetS ;
    TWmessage( "[TRANSLATE]:Pick or enter center of view:" ) ;
    TWgetPt2( &x1, &y1 ) ;
    sprintf( YmsgG, "new center - %d,%d", x1, y1 ) ;
    TWmessage( YmsgG ) ;
    /* translate to origin first */
    xoffsetS = - x1 ;
    yoffsetS = - y1 ;
    /* now center in screen - half of xspan */
    xoffsetS += (INT) ( (DOUBLE) winwidthS / scaleFactorS / 2.0 ) ;
    yoffsetS += (INT) ( (DOUBLE) winheightS / scaleFactorS / 2.0 );
    TWinforMenus() ; /* tell other module about info */
    TWforceRedraw() ;
    /* update clipping window */
    lS -= (xoffsetS - last_xoff) ;
    rS -= (xoffsetS - last_xoff) ;
    bS -= (yoffsetS - last_yoff) ;
    tS -= (yoffsetS - last_yoff) ;
    D( "TWtranslate",
	fprintf( stderr, "l:%d r:%d b:%d t:%d\n", lS, rS, bS, tS ) ;
    ) ;

} /* end TWtranslate */

/* copy pixmap to screen and flush screen output buffer */
/* flush screen output buffer */
TWflushFrame()
{
    if( modeS == TWWRITEONLY ){
	return ;
    }
    XFlush( dpyS ) ;
    TWcheckExposure() ;
} /* end TWflushFrame */

/* process everything in buffer */
TWsync()
{
    if( modeS == TWWRITEONLY ){
	return ;
    }
    XFlush( dpyS ) ;
    XSync( dpyS, 0 ) ;
} /* end TWsync */

static VOID initcolors( desiredColorArray, numC )
char **desiredColorArray ;
INT  numC ;
{
    unsigned long backgrd;
    unsigned long foregrd;
    unsigned long whitepix ;
    unsigned long blackpix ;
    GC       gc ;      /* temp for graphics context */
    char     *color ;
    XColor   ecolor ;  /* exact color - do need to use it */
    INT      i, j, k ;
    INT      pattern ;
    char     *stipple ;
    char     *TWstdStipple() ;
    char     row, bit ;
    Pixmap   *stipplePix ;  /* array of pixmaps for stipple */

    /* make copy of the users colors */
    userColorS = YMALLOC( numC+1, char * ) ;
    for( i=0; i <= numC; i++ ){
	if( desiredColorArray[i] ){
	    userColorS[i] = Ystrclone( desiredColorArray[i] ) ;
	} else {
	    userColorS[i] = NULL ;
	}
    }


    /* get default backgrounds white and black */
    whitepix = WhitePixel(dpyS,screenS);
    blackpix = BlackPixel(dpyS,screenS);
    if( reverseS == TRUE ){
	backgrd = blackpix ;
	foregrd = whitepix ;
    } else {
	backgrd = whitepix ;
	foregrd = blackpix ;
    }
    if( !(colorS) || stippleS ){
	stippleS = TRUE ;
	stipple = TWstdStipple() ;
	stipplePix = YMALLOC( numC+1, Pixmap ) ;
	{
	unsigned int x, y, xret, yret  ;
	x = 8 ;
	y = 8 ;
	XQueryBestSize( dpyS, StippleShape, drawS, x,y, &xret, &yret ) ;
	}
    }

    /* allocate size of colorArrayS */
    colorArrayS = YMALLOC( numC+1, XColor ) ;
    /* allocate array for whether color is turned on or off */
    colorOnS = YMALLOC( numC+1, BOOL ) ;
    /* allocate initial size of graphic context one for each color */
    graphicContextS = YMALLOC( numC+1, GC ) ;

    /* initialize color on array */
    for( i = 0; i <= numC; i++ ){
	colorOnS[i] = TRUE ;
    }

    /**** now create GRAPHIC CONTEXT for highlighting data **** */
    gc = graphicContextS[0] = XCreateGC( dpyS,drawS,0L,NULL ) ; 
    XSetFont( dpyS, gc, fontS ) ;
    XSetFunction( dpyS, gc, GXxor ) ;
#ifdef sun
    XSetForeground( dpyS, gc, foregrd ) ;
    XSetBackground( dpyS, gc, backgrd ) ;
#else
    XSetForeground( dpyS, gc, backgrd ) ;
    XSetBackground( dpyS, gc, foregrd ) ;
#endif
    XSetFillStyle( dpyS, gc, FillSolid ) ;
    XSetLineAttributes( dpyS,gc,1,LineSolid,CapButt,JoinBevel ) ;
    /**** end create GRAPHIC CONTEXT for highlighting data **** */

    /* lookup color in color table */
    for( i=1; i<= numC;  i++ ){

	/* take user given colors and look them up in color table */
	if( !(color = desiredColorArray[i] ) ){
	    fprintf( stderr, "ERROR:Color is NULL\n" ) ;
	    break ;
	    /* avoid nulls */
	} 
	/* check to make sure first two entries are */
	if( i == 1 ){  /* user must give white */
	    if( strcmp( desiredColorArray[i], "white" ) != STRINGEQ ){
		fprintf( stderr, 
		    "ERROR:Element 1 must be white in color array.\n") ;
	    }
	} else if( i == 2 ){  /* user must give black */
	    if( strcmp( desiredColorArray[i], "black" ) != STRINGEQ ){
		fprintf( stderr, 
		    "ERROR:Element 2 must be black in color array.\n") ;
	    }
	}

	if( colorS ){
	    /* now fill up this element of the color array */
	    if( XAllocNamedColor(dpyS,cmapS,color,&(colorArrayS[i]),
		&ecolor) <= 0){ 

		/* default to white if call is unsucessful <= 0 */
		fprintf( stderr,"Can't find color:%s ", color ) ;
		fprintf( stderr,"in color table. Defaulting to white.\n");
		colorArrayS[i].pixel = whitepix ;
	    }
	} 

	/* now create GRAPHIC CONTEXT */
	graphicContextS[i] =XCreateGC( dpyS,drawS,0L,NULL ) ; 

	/* set font */
	XSetFont( dpyS, graphicContextS[i], fontS ) ;

	XSetFunction( dpyS, graphicContextS[i], GXcopy ) ;
	XSetBackground( dpyS, graphicContextS[i], backgrd ) ;
	if( i == 1 ){
	    XSetForeground( dpyS, graphicContextS[i], whitepix ) ;
	    XSetBackground( dpyS, graphicContextS[i], blackpix ) ;
	} else if( i == 2 ){
	    XSetForeground( dpyS, graphicContextS[i], blackpix ) ;
	    XSetBackground( dpyS, graphicContextS[i], whitepix ) ;
	} else if( colorS ){
	    XSetForeground( dpyS, graphicContextS[i], 
		colorArrayS[i].pixel ) ;
	}
	if( colorS || i <= 2 ){
	    XSetFillStyle( dpyS, graphicContextS[i], FillSolid ) ;
	    XSetLineAttributes( dpyS,graphicContextS[i],1,LineSolid,
		CapRound, JoinBevel ) ;
	}
	if( i >= 3 && stippleS ){ /* i > 2 and black and white */
	    if( !(colorS)){
		if( reverseS ){
		    XSetForeground( dpyS, graphicContextS[i], backgrd ) ;
		} else {
		    XSetForeground( dpyS, graphicContextS[i], foregrd ) ;
		}
	    }
	    XSetLineAttributes( dpyS,graphicContextS[i],1,LineSolid,
		CapRound, JoinBevel ) ;
	    /* set stipple patterns for black and white */
	    XSetFillStyle( dpyS, graphicContextS[i], FillTiled );
	    /* now create an 8 x 8 pixmap for the stipple pattern */
	    stipplePix[i] = XCreatePixmap( dpyS,drawS,
		(unsigned)8, (unsigned)8, XDefaultDepth(dpyS,screenS) ) ;
	    /* Clear the pixmap - this is very subtle. Use */
	    /* XFillRectangle to insure pixmap doesn't have junk in it */
	    if( reverseS ){
		XFillRectangle( dpyS, stipplePix[i],
		    graphicContextS[BLACK], 0, 0, 8, 8 ) ;
	    } else {
		XFillRectangle( dpyS, stipplePix[i],
		    graphicContextS[WHITE], 0, 0, 8, 8 ) ;
	    }
	    /* pick ith stipple pattern fill in TWstdStipple array */
	    /* pattern is offset into array */
	    pattern = 8 * (i - 3) ;
	    for( j = 0; j < 8 ; j++ ){
		row = stipple[pattern+j] ;
		for( k = 0; k < 8 ; k++ ){
		    bit = row >> k ;
		    /* now get low order bit */
		    bit &= 0x01 ;
		    if( bit ){
			if( reverseS ){
			    XDrawPoint( dpyS, stipplePix[i], 
				graphicContextS[WHITE], j, k ) ;
			} else {
			    if( colorS ){
				XDrawPoint( dpyS, stipplePix[i], 
				    graphicContextS[i], j, k ) ;
			    } else {
				XDrawPoint( dpyS, stipplePix[i], 
				    graphicContextS[BLACK], j, k ) ;
			    }
			}
		    }
		}
	    } /* at this point we are done building pixmap */
	    /* apply stipple to graphic context */
	    XSetTile( dpyS, graphicContextS[i], stipplePix[i] ) ;
	    XSetFillRule( dpyS, graphicContextS[i], WindingRule ) ;
	    XSetTSOrigin( dpyS, graphicContextS[i], 0, 0 ) ;

	} /* end building stipple pattern */

    }

    numColorS = numC ;

    ASSERT( numC != 0, "initcolor", 
	"User didn't give any color data" ) ;
    
} /* end initcolor */

TWcolorXOR( color, exorFlag )
INT color ;
BOOL exorFlag ;
{
    /* check to make sure color is valid */
    if( color <= 0 || color > numColorS ){
	if( initS ){ /* graphics are available */
	    sprintf( YmsgG, "Color number:%d is out of range", color ) ;
	    TWmessage( YmsgG ) ;
	} else {
	    fprintf( stderr,"Color number:%d is out of range", color ) ;
	}
	return ;
    }
    if( exorFlag ){
	XSetFunction( dpyS, graphicContextS[color], GXxor ) ;
    } else {
	XSetFunction( dpyS, graphicContextS[color], GXcopy ) ;
    }
} /* end TWcolorXOR */


/* start a new slate */
static startDFrame()
{
    XClearWindow( dpyS, drawS ) ;
    if( reverseS ){
	XFillRectangle( dpyS, pixmapS,
	    graphicContextS[BLACK], 0, 0, winwidthS, winheightS ) ;
    } else {
	XFillRectangle( dpyS, pixmapS,
	    graphicContextS[WHITE], 0, 0, winwidthS, winheightS ) ;
    }
    TWdrawMenus() ;
    XFlush( dpyS ) ;
} /* end startDFrame */

static VOID drawDLine(ref,x1,y1,x2,y2,color,label)
/* draw a one pixel tall line segment from x1,y1 to x2,y2 */
INT	ref, color ;
register INT	x1,y1,x2,y2 ;
char	*label ;
{	

    /* check to make sure color is valid */
    if( color <= 0 || color > numColorS ){
	if( initS ){ /* graphics are available */
	    sprintf( YmsgG, "Color number:%d is out of range", color ) ;
	    TWmessage( YmsgG ) ;
	} else {
	    fprintf( stderr,"Color number:%d is out of range", color ) ;
	}
	return ;
    } else if(!(colorOnS[color])){
	return ;
    }
    if(!(colorS )){
	/* set all lines to black in B&W mode */
	color = BLACK ;
    }
    /* see if we need to clip line. We use to let Xserver do this */
    /* but we can avoid an interprocess communication if we do it ourself */
    if(!(fullViewS)){
	if(!(Yproject_intersect( x1, x2, y1, y2, lS, rS, bS, tS ) )){
	    return ;
	}
    }

    /* add users data offset */
    /* next scale coordinates to window */
    /* account for inversion of y axis */
    x1 = (INT) ( (DOUBLE) (x1 + xoffsetS) * scaleFactorS ) ;
    x2 = (INT) ( (DOUBLE) (x2 + xoffsetS) * scaleFactorS ) ;
    y1 = winheightS - (INT) ( (DOUBLE) (y1 + yoffsetS) * scaleFactorS ) ;
    y2 = winheightS - (INT) ( (DOUBLE) (y2 + yoffsetS) * scaleFactorS ) ;
    /* now draw line */
    XDrawLine( dpyS,drawS,graphicContextS[color], 
	x1,y1,x2,y2 ) ;
    XDrawLine( dpyS,pixmapS,graphicContextS[color], 
	x1,y1,x2,y2 ) ;
    if( label ){
	if( *label != EOS ){
	    XDrawImageString( dpyS, drawS, graphicContextS[color], 
		(x1+x2)/2, (y1+y2)/2, label, strlen(label) ) ;
	    XDrawImageString( dpyS, pixmapS, graphicContextS[color], 
		(x1+x2)/2, (y1+y2)/2, label, strlen(label) ) ;
	}
    }
} /* end drawDLine */

static VOID drawDRect(ref,x1,y1,x2,y2,color,label)
/* draw a rectangle whose diagonals are (x1,y1) and (x2,y2) */
/* 	if the specified color is default or invalid, use default color */
/* A border will be draw around the cell if specified black (default). */
INT	ref, color ;
register INT	x1,y1,x2,y2 ;
char	*label ;
{	
    UNSIGNED_INT width, height ;
    INT len ;

    /* check to make sure color is valid */
    if( color <= 0 || color > numColorS ){
	if( initS ){ /* graphics are available */
	    sprintf( YmsgG, "Color number:%d is out of range", color ) ;
	    TWmessage( YmsgG ) ;
	} else {
	    fprintf( stderr,"Color number:%d is out of range", color ) ;
	}
	return ;
    } else if(!(colorOnS[color])){
	return ;
    }
    if(!(fullViewS)){
	/* clip if necessary for speed. Avoid interprocess communication */
	if(!(Yproject_intersect( x1, x2, y1, y2, lS, rS, bS, tS ) )){
	    return ;
	}
    }

    /* first add data offset to move to relative to 0,0 */
    x1 += xoffsetS ;
    x2 += xoffsetS ;
    y1 += yoffsetS ;
    y2 += yoffsetS ;
    /* next scale coordinates to window */
    x1 = (INT) ( (DOUBLE) x1 * scaleFactorS ) ;
    x2 = (INT) ( (DOUBLE) x2 * scaleFactorS ) ;
    y1 = (INT) ( (DOUBLE) y1 * scaleFactorS ) ;
    y2 = (INT) ( (DOUBLE) y2 * scaleFactorS ) ;
    width = x2 - x1 ;
    height = y2 - y1 ;
    /* account for inversion of y axis */
    y1 = winheightS - y1 ;
    y2 = winheightS - y2 ;
    if( rect_fillS ){
	XFillRectangle( dpyS,drawS,graphicContextS[color],
	    x1,y2,width,height ) ;
	XFillRectangle( dpyS,pixmapS,graphicContextS[color],
	    x1,y2,width,height ) ;
	if( borderColorS ){
	    XDrawRectangle( dpyS,drawS,graphicContextS[borderColorS],
		x1,y2,width,height ) ;
	    XDrawRectangle( dpyS,pixmapS,graphicContextS[borderColorS],
		x1,y2,width,height ) ;
	}
    } else {
	XDrawRectangle( dpyS,drawS,graphicContextS[color],
	    x1,y2,width,height ) ;
	XDrawRectangle( dpyS,pixmapS,graphicContextS[color],
	    x1,y2,width,height ) ;
    }
    if( label ){
	if( *label != EOS ){
	    len = strlen(label) ;
	    /* now find width of string as offset */
	    width = XTextWidth( fontinfoS, label, len ) ;
	    /* need image string so you can write on top of fill */
	    XDrawImageString( dpyS, drawS, graphicContextS[color], 
		(x1+x2-width)/2, (y1+y2)/2, label, strlen(label) ) ;
	    XDrawImageString( dpyS, pixmapS, graphicContextS[color], 
		(x1+x2-width)/2, (y1+y2)/2, label, strlen(label) ) ;
	}
    }
} /* end drawDCell */

TWarb_init()
{
    /* allocate memory if needed */
    if(!(ptS)){
	ptAllocS = EXPECTEDPTS ;
	ptS = YMALLOC( ptAllocS, XPoint );
    }
    if( modeS == TWWRITEONLY || modeS == TWWRITENDRAW ){
	Ybuster_init() ;
    }
    /* make sure we cannot match the 0 record in the redundancy */
#if 0
    /* Yikes! XPoint x and y records are defined as type short. . . */
    ptS[0].x = INT_MIN ;
    ptS[0].y = INT_MIN ;
#else
    ptS[0].x = SHRT_MIN ;
    ptS[0].y = SHRT_MIN ;
#endif
    numptS = 0 ;
    if( !(fullViewS)){
	arblS = INT_MAX ; arbrS = INT_MIN ; arbbS = INT_MAX ; arbtS = INT_MIN ;
    }
} /* end TWarb_init */
/* ***************************************************************** */

TWarb_addpt( xpos, ypos )
INT xpos, ypos ;
{

    if( modeS == TWWRITEONLY || modeS == TWWRITENDRAW ){
	Ybuster_addpt( xpos, ypos ) ;
    }
    if(!(fullViewS)){
	arblS = MIN( arblS, xpos ) ;
	arbbS = MIN( arbbS, ypos ) ;
	arbrS = MAX( arbrS, xpos ) ;
	arbtS = MAX( arbtS, ypos ) ;
    }
    /* first add data offset to move to relative to 0,0 */
    xpos += xoffsetS ;
    ypos += yoffsetS ;
    /* next scale coordinates to window */
    xpos = (INT) ( (DOUBLE) xpos * scaleFactorS ) ;
    ypos = (INT) ( (DOUBLE) ypos * scaleFactorS ) ;
    /* account for inversion of y axis */
    ypos = winheightS - ypos ;
    /* now points are in X coordinates */
    if( xpos == (INT) ptS[numptS].x && ypos == (INT) ptS[numptS].y ){
	/* avoid redundant points */
	return ;
    }
    /* increase the space if necessary */
    if( ++numptS >= ptAllocS ){
	ptAllocS += EXPECTEDPTS ;
	ptS = YREALLOC( ptS,  ptAllocS, XPoint ) ;
    }
    ptS[numptS].x = (SHORT) xpos ;
    ptS[numptS].y = (SHORT) ypos ;
} /* end TWarb_addpt */
/* ***************************************************************** */


static VOID drawDArb( ref, color, label )
INT	ref, color ;
char	*label ;
{
    INT    i ;           /* counter */
    INT    len ;         /* length of string if given */
    INT x1, y1, x2, y2 ; /* bounding box of figure */
    UNSIGNED_INT width ; /* width of font */
    XPoint *points ;     /* array starts from 1 not zero */

    if( color <= 0 || color > numColorS ){
	if( initS ){ /* graphics are available */
	    sprintf( YmsgG, "Color number:%d is out of range", color ) ;
	    TWmessage( YmsgG ) ;
	} else {
	    fprintf( stderr,"Color number:%d is out of range", color ) ;
	}
	return ;
    } else if(!(colorOnS[color])){
	return ;
    }
    points = &(ptS[1]) ;

    /* close the figure if it is not already closed */
    if( ptS[numptS].x != ptS[1].x || ptS[numptS].y != ptS[1].y ){
	/* increase the space if necessary */
	if( ++numptS >= ptAllocS ){
	    ptAllocS += EXPECTEDPTS ;
	    ptS = YREALLOC( ptS,  ptAllocS, XPoint ) ;
	}
	ptS[numptS].x = ptS[1].x ;
	ptS[numptS].y = ptS[1].y ;
    }
    x1 = x2 = (INT) ptS[1].x ;
    y1 = y2 = (INT) ptS[1].y ;
    for( i = 2; i <= numptS; i++ ){
	x1 = MIN( x1, (INT) ptS[i].x ) ;
	x2 = MAX( x2, (INT) ptS[i].x ) ;
	y1 = MIN( y1, (INT) ptS[i].y ) ;
	y2 = MAX( y2, (INT) ptS[i].y ) ;
    }
    if(!(fullViewS)){
	/* clip if necessary for speed. Avoid interprocess communication */
	if(!(Yproject_intersect( arblS, arbrS, arbbS, arbtS, lS, rS, bS, tS ) )){
	    return ;
	}
    }

    if( fillArbS ){
	/* the fill automatically closed the region. if we don't */
	/* let it close the region, we have problems - bug in XFillPolygon */
	XFillPolygon( dpyS, drawS, graphicContextS[color], points, numptS,
	    Complex, CoordModeOrigin ) ;
	XFillPolygon( dpyS, pixmapS, graphicContextS[color], points, numptS,
	    Complex, CoordModeOrigin ) ;
    }

    if( label ){
	if( *label != EOS ){
	    len = strlen(label) ;
	    /* now find width of string as offset */
	    width = XTextWidth( fontinfoS, label, len ) ;
	    /* need image string so you can write on top of fill */
	    /* calculate where we need to put the label */
	    x1 = x2 = (INT) ptS[1].x ;
	    y1 = y2 = (INT) ptS[1].y ;
	    for( i = 2; i <= numptS; i++ ){
		x1 = MIN( x1, (INT) ptS[i].x ) ;
		x2 = MAX( x2, (INT) ptS[i].x ) ;
		y1 = MIN( y1, (INT) ptS[i].y ) ;
		y2 = MAX( y2, (INT) ptS[i].y ) ;
	    }
	    XDrawImageString( dpyS, drawS, graphicContextS[color], 
		(x1+x2-width)/2, (y1+y2)/2, label, strlen(label) ) ;
	    XDrawImageString( dpyS, pixmapS, graphicContextS[color], 
		(x1+x2-width)/2, (y1+y2)/2, label, strlen(label) ) ;
	}
    }
    if( fillArbS ){
	if( borderColorS ){
	    XDrawLines( dpyS, drawS, graphicContextS[borderColorS], 
		points, numptS, CoordModeOrigin ) ;
	    XDrawLines( dpyS, pixmapS, graphicContextS[borderColorS], 
		points, numptS, CoordModeOrigin ) ;
	}
    } else {
	XDrawLines( dpyS, drawS, graphicContextS[color], 
	    points, numptS, CoordModeOrigin ) ;
	XDrawLines( dpyS, pixmapS, graphicContextS[color], 
	    points, numptS, CoordModeOrigin ) ;
    }
} /* end drawDArb */

TWarb_fill( flag )
BOOL flag ;
{
    fillArbS = flag ;
} /* end TWarb_fill */

BOOL TWget_arb_fill()
{
    return( fillArbS ) ;
} /* end TWget_arb_fill */

TWrect_fill( flag )
BOOL flag ;
{
    rect_fillS = flag ;
} /* end TWrect_fill */

BOOL TWget_rect_fill()
{
    return( rect_fillS ) ;
} /* end TWget_rect_fill */

TWhighLightRect( x1,y1,x2,y2 )
/* draw a rectangle whose diagonals are (x1,y1) and (x2,y2) */
register INT	x1,y1,x2,y2 ;
{	
    UNSIGNED_INT width, height ;

    /* first add data offset to move to relative to 0,0 */
    x1 += xoffsetS ;
    x2 += xoffsetS ;
    y1 += yoffsetS ;
    y2 += yoffsetS ;
    /* next scale coordinates to window */
    x1 = (INT) ( (DOUBLE) x1 * scaleFactorS ) ;
    x2 = (INT) ( (DOUBLE) x2 * scaleFactorS ) ;
    y1 = (INT) ( (DOUBLE) y1 * scaleFactorS ) ;
    y2 = (INT) ( (DOUBLE) y2 * scaleFactorS ) ;
    width = x2 - x1 ;
    height = y2 - y1 ;
    /* account for inversion of y axis */
    y2 = winheightS - y2 ;
    XFillRectangle( dpyS,drawS,graphicContextS[HIGHLITE],
	x1,y2,width,height ) ;
    XFillRectangle( dpyS,pixmapS,graphicContextS[HIGHLITE],
	x1,y2,width,height ) ;
} /* end TWhighLightRect */

TWmoveRect( x1, y1, x2, y2, ptx, pty )
INT *x1, *y1, *x2, *y2, ptx, pty ;
/* x1, y1, x2, y2 are all user data absolute coordinates */
/* ptx and pty are the value of the pointer from TWgetPt */
{
    BOOL press ;              /* tells whether button has been released */
    XEvent event ;            /* describes event */
    long event_mask ;         /* set events */
    INT x, y ;                /* current position of pointer */
    INT last_time ;           /* last time rectangle was moved */
    INT dx_user, dy_user ;    /* rect pos rel to pointer in user scale */
    INT dx_pix, dy_pix ;      /* rect pos rel to pointer in pixels */
    INT oldx, oldy ;          /* rect pos rel to pointer in pixels */
    UNSIGNED_INT width_user ; /* width of rectangle user coordinates */
    UNSIGNED_INT height_user; /* height of rectangle user coordinates */
    UNSIGNED_INT width_pix ; /* width of rectangle pixel coordinates */
    UNSIGNED_INT height_pix; /* height of rectangle pixel coordinates */

    width_user = *x2 - *x1 ;
    dx_user = *x1 - ptx ;
    height_user = *y2 - *y1 ;
    dy_user = pty - *y2 ; /* note that y - axis is inverted */
    ptx += xoffsetS ;
    pty += yoffsetS ;
    /* next scale coordinates to window */
    ptx  =       (INT) ( (DOUBLE) ptx * scaleFactorS ) ;
    pty  =       (INT) ( (DOUBLE) pty * scaleFactorS ) ;
    width_pix  = (INT) ( (DOUBLE) width_user * scaleFactorS ) ;
    height_pix = (INT) ( (DOUBLE) height_user * scaleFactorS ) ;
    dx_pix     = (INT) ( (DOUBLE) dx_user * scaleFactorS ) ;
    dy_pix     = (INT) ( (DOUBLE) dy_user * scaleFactorS ) ;

    /* account for inversion of y axis */
    pty = winheightS - pty ;

    /* turn on event mask for main drawing window - known as wS */
    event_mask = StructureNotifyMask | SubstructureNotifyMask
	    | VisibilityChangeMask | ExposureMask | ButtonPressMask |
	    PointerMotionMask ;
    XSelectInput(dpyS,drawS,event_mask);

    oldx = ptx ;
    oldy = pty ;
    /* draw rectangle at absolute coordinates */
    XDrawRectangle( dpyS,drawS,graphicContextS[HIGHLITE],
	ptx+dx_pix,pty+dy_pix,width_pix,height_pix ) ;

    /* now look for either event - button press or keyboard */
    press = FALSE ;
    last_time = 0 ;
    while(!(press )){
	/* check for user input from mouse */
	if( press = XCheckTypedWindowEvent( dpyS,drawS,
		ButtonPress,&event ) ){
	    /* we have an event from the pointer */
	    /* put event back on queue  and call TWgetPt */
	    XPutBackEvent( dpyS, &event ) ;
	    TWgetPt( &x, &y ) ;
	    *x1 = x + dx_user ;
	    *x2 = *x1 + width_user ;
	    *y2 = y - dy_user ; /* - because of y axis inversion */
	    *y1 = *y2 - height_user ;
	} /* otherwise continue to loop */
	/* move rectangle */
	if( XCheckTypedWindowEvent( dpyS,drawS,
		MotionNotify,&event ) ){
	    /* avoid to many events to screen wait 50 msec.*/
	    if( event.xmotion.time - last_time < 50 ){
		continue ;
	    }
	    last_time = event.xmotion.time ;
	    x = event.xmotion.x ;
	    y = event.xmotion.y ;
	    /* draw rectangle at old position absolute coordinates */
	    XDrawRectangle( dpyS,drawS,graphicContextS[HIGHLITE],
		oldx+dx_pix,oldy+dy_pix,width_pix,height_pix ) ;
	    XFillRectangle( dpyS,drawS,graphicContextS[HIGHLITE],
		oldx+dx_pix,oldy+dy_pix,width_pix,height_pix ) ;
	    XDrawRectangle( dpyS,drawS,graphicContextS[HIGHLITE],
		x+dx_pix,y+dy_pix,width_pix,height_pix ) ;
	    XFillRectangle( dpyS,drawS,graphicContextS[HIGHLITE],
		x+dx_pix,y+dy_pix,width_pix,height_pix ) ;
	    oldx = x ; oldy = y ;
	    XFlush( dpyS ) ;
	}
    } /* end while loop */
	
} /* end TWmoveRect */

XFontStruct *TWgetfont( fname, font )
char *fname ;
Font *font ;
{
    XFontStruct *fontinfo ;

    /* set font and get font info */
    /* this is a safe test to see if font exists */
    if(!(fontinfo = XLoadQueryFont( dpyS, fname ))){
	sprintf( YmsgG, "font:%s not available - using default:fixed\n", fname ) ;
	M( ERRMSG,"TWgetfont", YmsgG ) ;
	fontinfo = XLoadQueryFont( dpyS, "fixed" ) ;
    }
    *font = fontinfo->fid ;
    return( fontinfo ) ;
} /* end TWgetfont */

_TW3DdrawAxis( drawNotErase )
BOOL drawNotErase ;
{
    INT xspan, yspan, zspan ;
    INT c ;      /* string color */
    INT xstring, ystring ;
    DOUBLE X0, Y0, X, Y ;

    xspan = rightS - leftS ; 
    yspan = topS - bottomS ;
    zspan = MAX( xspan, yspan ) ;

    if( reverseS ){
	if( drawNotErase ){
	    c = 1 ;  /* white characters */
	} else {
	    /* erase */
	    c = 2 ;  /* black background */
	}
    } else {
	if( drawNotErase ){
	    c = 2 ;  /* black characters */
	} else {
	    /* erase */
	    c = 1 ;  /* white background */
	}
    }

    /* there are 4 points of interest */
    /* (0,0,0),  (xspan,0,0), (0,yspan,0),  (0,0,zspan) */

    /* first (0,0,0) */
    TW3Dperspective( (DOUBLE)0, (DOUBLE)0, (DOUBLE)0, &X0, &Y0 );

    /* next  (xspan,0,0) */
    TW3Dperspective( (DOUBLE)xspan, (DOUBLE)0, (DOUBLE)0, &X, &Y );
    /* draw X axis */
    TWdrawLine(0, (INT)X0, (INT)Y0, (INT)X, (INT)Y, 3, NIL(char *) ) ;
    /* draw label - find label coordinates */
    xstring = ( (INT) X0 + (INT) X ) DIV_2 ;
    ystring = ( (INT) Y0 + (INT) Y ) DIV_2 ;
    TWdrawString( xstring, ystring, c, "x" ) ;

    /* next  (0,yspan,0) */
    TW3Dperspective( (DOUBLE)0, (DOUBLE)yspan, (DOUBLE)0, &X, &Y );
    /* draw X axis */
    TWdrawLine(0, (INT)X0, (INT)Y0, (INT)X, (INT)Y, 3, NIL(char *) ) ;
    /* draw label - find label coordinates */
    xstring = ( (INT) X0 + (INT) X ) DIV_2 ;
    ystring = ( (INT) Y0 + (INT) Y ) DIV_2 ;
    TWdrawString( xstring, ystring, c, "y" ) ;

    /* next  (0,0,zspan) */
    TW3Dperspective( (DOUBLE)0, (DOUBLE)0, (DOUBLE)zspan, &X, &Y );
    /* draw X axis */
    TWdrawLine(0, (INT)X0, (INT)Y0, (INT)X, (INT)Y, 3, NIL(char *) ) ;
    /* draw label - find label coordinates */
    xstring = ( (INT) X0 + (INT) X ) DIV_2 ;
    ystring = ( (INT) Y0 + (INT) Y ) DIV_2 ;
    TWdrawString( xstring, ystring, c, "z" ) ;
} /* end _TW3DdrawAxis */

VOID TW3DsetCamera()
{
  INT x, y ;
  INT oldx, oldy ;
  BOOL first_time = TRUE ;
  DOUBLE FOURPI = 720;
  DOUBLE mouseScaleX ;
  DOUBLE mouseScaleY ;

  TWmouse_tracking_start() ;
  TWcolorXOR( 3, TRUE ) ;
  _TW3DdrawAxis( TRUE ) ;

  /* once across window scales to four pi radians */
  mouseScaleX = FOURPI / (DOUBLE) (rightS - leftS) ;
  mouseScaleY = FOURPI / (DOUBLE) (topS - bottomS) ;

  while( !(TWmouse_tracking_end() )){
      if( TWmouse_tracking_pt( &x, &y ) ){
	  /* draw the old one first */
	  _TW3DdrawAxis( FALSE ) ;
	  if( first_time ){
	      oldx = x ;  
	      oldy = x ;  
	      first_time = FALSE ;
	      continue ;
	  }
	  thetaS +=  ( x - oldx ) * radiansPerDegreeS * mouseScaleX;
	  phiS += ( y - oldy ) * radiansPerDegreeS *mouseScaleY;
	  /*
	  sprintf(YmsgG, "Current settings: theta:%3d phi:%3d", 
	    (INT) (thetaS / radiansPerDegreeS), 
	    (INT) (phiS / radiansPerDegreeS) ) ;
	  TWmessage(YmsgG) ;
	  */
	  oldx = x ; oldy = y ;
	  set_viewing_transformation();
	  /* now draw the new one */
	  _TW3DdrawAxis( TRUE ) ;

	} /* end check on whether coordinate changed */
    } /* end mouse tracking loop */

    TWcolorXOR( 3, FALSE ) ;

    /* translate to origin */
    xoffsetS = 0 ;
    yoffsetS = 0 ;
    /* now center in screen - half of xspan */
    xoffsetS += (INT) ( (DOUBLE) winwidthS / scaleFactorS / 2.0 ) ;
    yoffsetS += (INT) ( (DOUBLE) winheightS / scaleFactorS / 2.0 );
    TWinforMenus() ; /* tell other module about info */
    TWforceRedraw() ;
    _TW3DdrawAxis( TRUE ) ;
} /* end TW3DsetCamera */
	    
/*--------------------
  --------------------*/
VOID TW3DperspectiveOn()
{
    perspectiveS = TRUE;
} /* end TW3DperspectiveOn */

/*--------------------
  --------------------*/
VOID TW3DperspectiveOff()
{
    perspectiveS = FALSE;
} /* end TW3DperspectiveOff */

/*------------------
  Initialize the transformation matrices
  ------------------*/
static VOID set_viewing_transformation()
{
  DOUBLE cosine_of_theta;
  DOUBLE sine_of_theta;
  DOUBLE  cosine_of_phi;
  DOUBLE  sine_of_phi;

  if ( thetaS == acos( (double)0.0 ) && phiS == 0.0 ) {
    cosine_of_theta = 0.0;
    cosine_of_phi   = 1.0;
    sine_of_theta   = 1.0;
    sine_of_phi     = 0.0;
  } else {
    cosine_of_theta = cos (thetaS);
    cosine_of_phi   = cos (phiS);
    sine_of_theta   = sin (thetaS);
    sine_of_phi     = sin (phiS);
  }
  
  v11S = -sine_of_theta;  
  v12S = -cosine_of_phi   *  cosine_of_theta;
  v13S = -sine_of_phi     *  cosine_of_theta;
  
  v21S =  cosine_of_theta;
  v22S = -cosine_of_phi   * sine_of_theta;
  v23S = -sine_of_phi     * sine_of_theta;
  
  v32S =  sine_of_phi;
  v33S = -cosine_of_phi;
  
  v43S =  rhoS;

} /* end set_viewing_transformation */

/*------------------
  Perform a 3D transformation.  
  ------------------*/
VOID TW3Dperspective(x, y, z, pX, pY)
DOUBLE    x, y, z;
DOUBLE    *pX, *pY;
     
{
  DOUBLE x_eye, y_eye, z_eye;
  
  x = -x; y = -y; z = -z;
  
  x_eye = v11S * x + v21S * y;
  y_eye = v12S * x + v22S * y + v32S * z;
  z_eye = v13S * x + v23S * y + v33S * z + v43S;
  
  /* screen coordinates */
  if (perspectiveS) {
    *pX = x_eye/z_eye;
    *pY = y_eye/z_eye;
  }
  else {
    *pX = x_eye / v43S;
    *pY = y_eye / v43S;
  }

  D("perspective",
    fprintf(stderr,
	"IN x:%-9.1f y:%-9.1f z:%-9.1f  OUT x:%-9.1f y:%-9.1f\n",
	x,y,z,*pX,*pY ) ;
    );
} /* end perspective */

/* set the 3D translations to normal view */
VOID TW3Dnormal_view()
{
    /* Sperical coordinates ?                                 */
    /* rho   = distance to point                              */
    /* theta = angle from x-axis on xy plane                  */
    /* phi   = angle from z-axis to line from point to origin */
    /* the normal way to look at things from top */
    thetaS    =   acos( (double) 0.0 );
    phiS      =   0.0 ;
    rhoS      =   1.0 ;
    perspectiveS = FALSE;   /* draw in perspective */
    set_viewing_transformation() ;
    if( initS ){
	TWforceRedraw() ;
    }
    /* done 3D initialization */
} /* end TW3Dnormal_view */

/*-------------------------
    Draws a 3 dimensional cube.
  -------------------------*/
INT TW3DdrawCube(ref_num, x1, y1, z1, x2, y2, z2, color, label)
INT ref_num, x1, y1, z1, x2, y2, z2 ;
INT color;
char *label;
{
    /* try it as a solid */
    /* side face 1 */
    TW3Darb_init() ;
    TW3Darb_addpt( x1, y1, z1 ) ;
    TW3Darb_addpt( x1, y1, z2 ) ;
    TW3Darb_addpt( x1, y2, z2 ) ;
    TW3Darb_addpt( x1, y2, z1 ) ;
    TW3DdrawArb( ref_num, color, NIL(char *) ) ;
    /* side face 2 */
    TW3Darb_init() ;
    TW3Darb_addpt( x2, y1, z1 ) ;
    TW3Darb_addpt( x2, y1, z2 ) ;
    TW3Darb_addpt( x2, y2, z2 ) ;
    TW3Darb_addpt( x2, y2, z1 ) ;
    TW3DdrawArb( ref_num, color, NIL(char *) ) ;
    /* front face 3 */
    TW3Darb_init() ;
    TW3Darb_addpt( x1, y1, z1 ) ;
    TW3Darb_addpt( x1, y1, z2 ) ;
    TW3Darb_addpt( x2, y1, z2 ) ;
    TW3Darb_addpt( x2, y1, z1 ) ;
    TW3DdrawArb( ref_num, color, NIL(char *) ) ;
    /* front face 4 */
    TW3Darb_init() ;
    TW3Darb_addpt( x1, y2, z1 ) ;
    TW3Darb_addpt( x1, y2, z2 ) ;
    TW3Darb_addpt( x2, y2, z2 ) ;
    TW3Darb_addpt( x2, y2, z1 ) ;
    TW3DdrawArb( ref_num, color, NIL(char *) ) ;
    /* bottom face */
    TW3Darb_init() ;
    TW3Darb_addpt( x1, y1, z1 ) ;
    TW3Darb_addpt( x1, y2, z1 ) ;
    TW3Darb_addpt( x2, y2, z1 ) ;
    TW3Darb_addpt( x2, y1, z1 ) ;
    TW3DdrawArb( ref_num, color, NIL(char *) ) ;
    /* top face */
    TW3Darb_init() ;
    TW3Darb_addpt( x1, y1, z2 ) ; 
    TW3Darb_addpt( x1, y2, z2 ) ;
    TW3Darb_addpt( x2, y2, z2 ) ;
    TW3Darb_addpt( x2, y1, z2 ) ;
    TW3DdrawArb( ref_num, color, label ) ;

    if( fillArbS ){
	TW3DdrawLine(ref_num, x1, y1, z1, x2, y1, z1, 2, NIL(char *));
	TW3DdrawLine(ref_num, x1, y1, z1, x1, y2, z1, 2, NIL(char *));
	TW3DdrawLine(ref_num, x1, y1, z1, x1, y1, z2, 2, NIL(char *));

	TW3DdrawLine(ref_num, x2, y2, z2, x1, y2, z2, 2, NIL(char *));
	TW3DdrawLine(ref_num, x2, y2, z2, x2, y1, z2, 2, NIL(char *));
	TW3DdrawLine(ref_num, x2, y2, z2, x2, y2, z1, 2, NIL(char *));

	TW3DdrawLine(ref_num, x1, y2, z2, x1, y1, z2, 2, NIL(char *));
	TW3DdrawLine(ref_num, x1, y2, z2, x1, y2, z1, 2, NIL(char *));
	TW3DdrawLine(ref_num, x2, y2, z1, x1, y2, z1, 2, NIL(char *));

	TW3DdrawLine(ref_num, x2, y1, z1, x2, y2, z1, 2, NIL(char *));
	TW3DdrawLine(ref_num, x2, y1, z1, x2, y1, z2, 2, NIL(char *));
	TW3DdrawLine(ref_num, x1, y1, z2, x2, y1, z2, 2, NIL(char *));
    }

} /* end TW3DdrawCube */

/* returns string size in user coordinate system */
TWstringSize( string, width, height )
char *string ;
INT *width, *height ;
{
    INT len ;        /* length of string in characters */
    INT pix_width ;  /* width of string in pixels */
    INT pix_height ; /* height of string in pixels */
    len = strlen( string ) ;
    pix_width = XTextWidth( fontinfoS, string, len ) ;
    pix_height = fontinfoS->ascent + fontinfoS->descent ;
    /* now reverse scale of coordinates */
    *width  = (INT) ( (DOUBLE) pix_width / scaleFactorS ) ;
    *height = (INT) ( (DOUBLE) pix_height / scaleFactorS ) ;

}

TWdrawString( x, y, color, label )
INT x, y, color ;
char *label ;
{
    if( color <= 0 || color > numColorS ){
	if( initS ){ /* graphics are available */
	    sprintf( YmsgG, "Color number:%d is out of range", color ) ;
	    TWmessage( YmsgG ) ;
	} else {
	    fprintf( stderr,"Color number:%d is out of range", color ) ;
	}
	return ;
    } else if(!(colorOnS[color])){
	return ;
    }
    if(!(fullViewS)){
	if(!(Yproject_intersect( x, x, y, y, lS, rS, bS, tS ) )){
	    return ;
	}
    }
    /* add users data offset */
    /* next scale coordinates to window */
    /* account for inversion of y axis */
    x = (INT) ( (DOUBLE) (x + xoffsetS) * scaleFactorS ) ;
    y = winheightS - (INT) ( (DOUBLE) (y + yoffsetS) * scaleFactorS ) ;
    if( label ){
	if( *label != EOS ){
	    XDrawImageString( dpyS, drawS, graphicContextS[color], 
		x, y, label, strlen(label) ) ;
	    XDrawImageString( dpyS, pixmapS, graphicContextS[color], 
		x, y, label, strlen(label) ) ;
	}
    }
} /* end TWdrawString */


/******************************************************************
    END SCREEN GRAPHIC ROUTINES 
******************************************************************* */


/***  STATIC DEFINIONS FOR WRITE ROUTINES ONLY ****/
static  FILE *cellFileS = NULL ; /* cellfile pointer */   
static  FILE *netFileS = NULL ;  /* net file pointer */
static  FILE *symbFileS = NULL ; /* symbfile pointer */
static  INT  numCellS = 0 ; /* cell counter */
static  INT  numNetS = 0 ;  /* net counter */
static  INT  numPinS = 0 ;  /* pin counter */
static  INT  numCharS = 0 ; /* symbol table counter */


TWstartFrame()
{
    char filename[LRECL] ;
    char dummy[5] ;
    UNSIGNED_INT nitems ;


    if(!(initS )){
	fprintf(stderr,"ERROR[startNewFrame]:initialization was not" ) ;
	fprintf(stderr,"performed\n  before calling startNewFrame\n" ) ;
	YexitPgm( NOINIT ) ;
    }

    switch( modeS ){
	case TWDRAWONLY:
	    startDFrame() ;
	    return ;
	case TWWRITEONLY:
	    break ;
	case TWWRITENDRAW:
	    startDFrame() ;
	    break ;
    }

    if(!dirNameS){
	return ;
    }

    if( frameOpenS ){
	closeFrame() ;
    }
    /* signal that frame is now open */
    frameOpenS = TRUE ;

    /* now open next frame files */
    /* increment frame count */
    frameCountS++ ;

    /* first cell file */
    sprintf( filename, "%s/cell.bin.%d", dirNameS, frameCountS ) ;
    cellFileS = TWOPEN( filename, "w", ABORT ) ; 
    numCellS = 0 ; /* reset cell counter */

    /* next net file */
    sprintf( filename, "%s/net.bin.%d", dirNameS, frameCountS ) ;
    netFileS = TWOPEN( filename, "w", ABORT ) ; 
    numNetS = 0 ; /* reset net counter */

    /* next symb file */
    sprintf( filename, "%s/symb.bin.%d", dirNameS, frameCountS ) ;
    symbFileS = TWOPEN( filename, "w", ABORT ) ; 
    /* write a dummy character at start file makes test for label */
    /* index easier since symbtable[0] is meaningless now. */
    /* All indexes into symbol table which are valid must be positive */
    dummy[0] = EOS ;
    nitems = (UNSIGNED_INT) 1 ;
    fwrite( dummy, sizeof(char), nitems, symbFileS ) ;
    numCharS = 1 ; /* reset symbol table counter */

} /* end startNewFrame */

/* write size of data at end of files and close them if frames are open */
static closeFrame()
{
    char dummy[5] ;
    UNSIGNED_INT nitems ;
    INT numw ;
    INT excess ;

    if(!dirNameS){
	return ;
    }
    /* check to see if other files are open */
    if( frameOpenS ){
	nitems = (UNSIGNED_INT) 1 ;
	/* terminate the file with number of records in each file */
	numw = fwrite( &numCellS,sizeof(UNSIGNED_INT),nitems,cellFileS ) ;
	ASSERT( numw == 1, "startNewFrame", "Number written zero" ) ;
	numw = fwrite( &numNetS, sizeof(UNSIGNED_INT),nitems,netFileS ) ;
	ASSERT( numw == 1, "startNewFrame", "Number written zero" ) ;
	/* need to put on integer boundary */
	if( excess = numCharS % 4 ){
	    /* pad the remainder with dummy */
	    nitems = (UNSIGNED_INT) (4 - excess ) ;
	    numw = fwrite( dummy, sizeof(char), nitems, symbFileS ) ;
	    ASSERT( numw == nitems,"startNewFrame","Number written zero");
	}
	nitems = (UNSIGNED_INT) 1 ;
	numw = fwrite( &numCharS,sizeof(UNSIGNED_INT),nitems,symbFileS ) ;
	ASSERT( numw == 1, "startNewFrame", "Number written zero" ) ;

	/* files are open close them */
	ASSERT( cellFileS, "startNewFrame", "cell file should be open" ) ;
	TWCLOSE( cellFileS ) ;
	ASSERT( netFileS, "startNewFrame", "net file should be open" ) ;
	TWCLOSE( netFileS ) ;
	ASSERT( symbFileS, "startNewFrame", "symb file should be open" ) ;
	TWCLOSE( symbFileS ) ;
	/* signal that frame has been closed */
	frameOpenS = FALSE ;
    }

} /* closeFrame */

TWsetFrame( number )
INT number ;
{
    char fileName[LRECL] ;

    if(!dirNameS){
	return ;
    }
    if( number <= 0 ){
	/* search for the first valid file */
	/* find max number of frames of data */ 
	for( frameCountS=1;;frameCountS++ ){

	    sprintf( fileName,"%s/cell.bin.%d",dirNameS,frameCountS ) ;
	    if(! (YfileExists(fileName) )){
		/* last file successfully read is one less */
		frameCountS-- ; 
		break ;
	    }
	}
    } else {
	frameCountS = number ;
    }
}

/*   THE SPECIALIZED ROUTINES */
/* *********  GENERIC WRITE ROUTINES **************  */
/* draw a one pixel tall line segment from x1,y1 to x2,y2 */
static VOID drawWLine( ref_num,x1,y1,x2,y2,color,label)
INT     ref_num ; /* reference number */
INT	x1,y1,x2,y2,color ;
char	*label;
{	
    DATABOX record ;
    UNSIGNED_INT nitems ;
    INT numw ; /* number written */

    if(!(colorOnS[color]) || !dirNameS ){
	return ;
    }
    /* check modes call draw functions if necessary */
    switch( modeS ){
	case TWDRAWONLY:
	    /* this is an error should never call this */
	    fprintf( stderr,"ERROR[drawWPin]:problem with mode\n" ) ;
	    return ;
	case TWWRITEONLY:
	    break ;
	case TWWRITENDRAW:
	    drawDLine( ref_num,x1,y1,x2,y2,color,label) ;
	    break ;
    }
    /* fill up data record  file destination net file */
    record.ref = (UNSIGNED_INT) ref_num ;
    record.x1 = x1 ;
    record.x2 = x2 ;
    record.y1 = y1 ;
    record.y2 = y2 ;
    /* now take care of color */
    record.color = color ;
    /* now store string in symbol table if given */
    if( label ){
	/* write string to symbol table file */
	nitems = (UNSIGNED_INT) ( strlen( label ) + 1 ) ;
	numw = fwrite( label, sizeof(char), nitems, symbFileS ) ;
	ASSERT( numw == nitems, "drawLine", 
	    "Couldnt write to string table" );
	/* now store in net file offset in table */
	record.label = numCharS ;
	/* now update offset to include this string */
	numCharS += (INT) nitems ;
	
    } else {
	record.label = 0 ;
    }
    /* now write record */
    nitems = (UNSIGNED_INT) 1 ;
    numw = fwrite( &record, sizeof(DATABOX),nitems,netFileS ) ;
    ASSERT( numw == 1, "drawLine", "Record not written..." ) ;
    numNetS++ ;

} /* end drawWLine */

/* draw a rectangle whose diagonals are (x1,y1) and (x2,y2) */
/* 	if the specified color is default or invalid, use default color */
static VOID drawWRect( ref_num, x1,y1,x2,y2,color,label)
INT     ref_num ; /* reference number */
INT	x1,y1,x2,y2, color;
char	*label;
{
    DATABOX record ;
    UNSIGNED_INT nitems ;
    INT numw ; /* number of records written */

    if(!(colorOnS[color]) || !dirNameS ){
	return ;
    }
    /* check modes call draw functions if necessary */
    switch( modeS ){
	case TWDRAWONLY:
	    /* this is an error should never call this */
	    fprintf( stderr,"ERROR[drawWRect]:problem with mode\n" ) ;
	    return ;
	case TWWRITEONLY:
	    break ;
	case TWWRITENDRAW:
	    drawDRect( ref_num,x1,y1,x2,y2,color,label) ;
	    break ;
    }
    /* fill up data record  file destination net file */
    record.ref = (UNSIGNED_INT) ref_num ;
    record.x1 = x1 ;
    record.x2 = x2 ;
    record.y1 = y1 ;
    record.y2 = y2 ;
    record.color = color ;
    /* now store string in symbol table if given */
    if( label ){
	/* write string to symbol table file */
	nitems = (UNSIGNED_INT) ( strlen( label ) + 1 ) ;
	numw = fwrite( label, sizeof(char), nitems, symbFileS ) ;
	ASSERT( numw == nitems, "drawWRect", 
	    "Couldn't write to string table" );
	/* now store in net file offset in table */
	record.label = numCharS ;
	/* now update offset to include this string */
	numCharS += (INT) nitems ;
	
    } else {
	record.label = 0 ;
    }
    /* now write record */
    nitems = (UNSIGNED_INT) 1 ;
    numw = fwrite( &record, sizeof(DATABOX),nitems,cellFileS ) ;
    numCellS++ ;
    ASSERT( numw == 1, "drawWRect", "Record not written..." ) ;

} /* end drawWRect */

static VOID drawWArb( ref, color, label )
INT	ref, color ;
char	*label ;
{
    YBUSTBOXPTR bustptr ;

    if(!(colorOnS[color]) || !dirNameS ){
	return ;
    }
    /* check modes call draw functions if necessary */
    switch( modeS ){
	case TWDRAWONLY:
	    /* this is an error should never call this */
	    fprintf( stderr,"ERROR[drawWArb]:problem with mode\n" ) ;
	    return ;
	case TWWRITEONLY:
	    break ;
	case TWWRITENDRAW:
	    drawDArb( ref, color, label ) ;
	    break ;
    }
    while( bustptr = Ybuster() ){
	/* l = bustptr[1].x ; */
	/* r = bustptr[4].x ; */
	/* b = bustptr[1].y ; */
	/* t = bustptr[2].y ; */
	TWdrawCell( ref, bustptr[1].x, bustptr[1].y,
	    bustptr[4].x, bustptr[2].y, color, label ) ;
    }
} /* end drawDArb */

#endif /* NOGRAPHICS */
/* ************************************************************** */
