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
FILE:	    menus.c                                       
DESCRIPTION:graphic drawing routines for handling menus
    There are two windows of interest in this file:
	wS - parent window is the large TW drawing window.
	menuS - INT small window at the top of the large TW window.
    The name of the menus are displayed in the menuS window but when 
    a menu is selected it is draw in the large wS window.  The 
    variables which have top in their name refer to the menuS window and
    variable with entry in their name refer to the menu entry pixmaps
    which are drawn in the large window.
CONTENTS:   TWinforMenus( )
	    INT TWsaveState()
	    TWrestoreState()
	    BOOL TWinitMenuWindow( menu_fields, parasite )
		TWMENUPTR menu_fields ;
		BOOL parasite ;
	    TWdrawMenus()
	    static set_window_lights( flag )
		BOOL flag ;
	    static checkwindow_lights()
	    INT TWcheckMouse()
	    TWgetPt( x, y )
		INT *x, *y ;
	    TWmessage( message )
		char *message ;
	    char *TWgetString( directions )
		char *directions ;
	    BOOL TWgetPt2( x, y )
		INT *x, *y ;
	    BOOL TWcheckExposure()
	    BOOL TWinterupt()
	    TWcheckReconfig()
	    TWfreeMenuWindows()
	    TWMENUPTR TWread_menus( filename )
		char *filename ;
	    static char *cap_item( sptr )
		char *sptr ;
	    static debug_menus( menu_field )
		TWMENUPTR menu_field ;

DATE:	    Jan 25, 1989 - major rewrite of version 1.0
REVISIONS:  Jan 31, 1989 - added screen routines.
	    Feb 21, 1989 - original version of graphics routines.
	    Mar 16, 1989 - turned off XSync in TWinterupt - too slow.
	    Mar 22, 1989 - added info.h.
	    Mar 23, 1989 - now TWinitMenus returns a boolean value
		signifying whether it was sucessful.
	    May 10, 1989 - added parasite window properties.
	    May 24, 1989 - fixed TWmessage and added TWDIR environ var.
	    Jul 31, 1989 - fixed problem with NULL in TWgetPt2.
	    Aug 16, 1989 - added user defined fonts and fixed
		problem with TWreconfigure and TWcheckExposure so
		that reconfigs work properly at least in 
		nonparasite mode.
	    Sep 11, 1989 - added subwindows for top menu window to make
		more user friendly.
	    Sep 16, 1989 - all debug directed to stderr.
	    Oct  6, 1989 - now menus can be compiled into program.
	    Sep 15, 1990 - made a compromise fix for TWcheckExposure.
	    Oct 11, 1990 - added sleep so that program won't spend much
		time in TWcheckMouse if nothing is going on for 10 secs.
	    Oct 12, 1990 - Rewrote draw routines to use 4 windows 
		and added pixmap for fast redraws.
	    Oct 21, 1990 - fixed parasite resizing.
	    Dec 27, 1990 - fixed multiple redraws on TWcheckReconfig.
	    Dec 28, 1990 - worked on multiple redraws on slow nodes.
	    Mon Jan  7 18:17:44 CST 1991 - made SAFE_WAIT_TIME
		user programmable for slow machines.
	    Fri Jan 25 16:17:21 PST 1991 - now look for backspace
		or delete key when in window.
	    Sat Feb 16 15:34:23 EST 1991 - now set input focus for
		TWgetString.
	    Sun Feb 17 17:13:23 EST 1991 - added TWmouse_tracking
		routines for 3D graphics.
	    Thu Mar  7 01:26:56 EST 1991 - tried to fix a SUN race
		condition.
	    Thu Mar 28 14:36:22 EST 1991 - added three button mouse
		option.
	    Wed Apr 17 23:38:31 EDT 1991 - put back missing Xselect
		input.
	    Sun Apr 28 22:06:15 EDT 1991 - set colors right for
		reverse video.
	    Wed Jul 24 15:59:59 CDT 1991 - added message window timeout.
	    Sun Nov  3 12:53:20 EST 1991 - fixed gcc complaints.
	    Thu Jan 30 00:10:58 EST 1992 - made expert mode for three button
		mouse key.
	    Wed Feb 26 03:54:12 EST 1992 - added persistent windows and
		added dim features.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) menus.c (Yale) version 3.36 2/26/92" ;
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
#include <yalecad/debug.h>
#include <yalecad/draw.h>
#include <yalecad/colors.h>
#include "info.h"

#define DEFAULT_TIMEOUT      10 * 1000       /* 10 seconds to timeout on message window */

/* data record for menu information */
typedef struct {
    Window   top_window ;            /* heading window for menu */
    Window   *window ;               /* window for each menu entry */
    char     *name ;                 /* the name of this menu */
    INT      xpos ;                  /* x pos of entry menu in messageW */
    INT      name_len ;              /* name length of top menu */
    INT      pix_len ;               /* pixlength of top menu entry */
    INT      entry_wid ;             /* width in pix of max menu entry */
    INT      numentries ;            /* number of entries in menu */
    INT      *function ;             /* function number of each entry */
    char     **entry ;               /* name of each menu entry */
    INT      *entry_len ;            /* len of each menu entry */
    INT      *xpos_adj ;             /* position of each menu entry str */
    INT      width ;                 /* width of menu entry */
    BOOL     *bool_entry ;           /* tells whether entry is boolean */
    BOOL     *state ;                /* state of this window entry */
    INT      *function2 ;            /* alternate function for boolean */
    char     **entry2 ;              /* for Boolean entries */
    INT      *xpos_adj2 ;            /* position of alternate menu entry*/
    INT      *entry_len2 ;           /* len of each menu entry */
    BOOL     *enabled ;              /* whether menu item is enable or not */
} MENUBOX, *MENUPTR ;

static TWINFOPTR    infoS ;          /* information about main details */
static Display      *dpyS;           /* the display */
static Window       menuS;           /* the current menu window */
static Window       drawS;           /* the main draw window */
static Window       backS;           /* the backing window */
static Window       messageS;        /* the message display window */
static GC           menuGCS ;        /* graphics context for menus */
static GC           menuRGCS ;       /* reverse gc for turning on menus */
static INT          screenS ;        /* the current screen */
static UNSIGNED_INT backgrdS ;
static UNSIGNED_INT foregrdS ;
static INT          winwidthS ;      /* window width */
static MENUPTR      *menuArrayS ;    /* contains info about menus */
static XFontStruct  *fontinfoS ;     /* font information */
static Font         fontS ;          /* current font */
static INT          numMenuS ;       /* number of menus */
static INT          stepsizeS ;      /* spacing between menus in x direction */
static BOOL         parasiteS=FALSE; /* whether this menu init is a parasite */
static Pixmap       pixmapS ;        /* offscreen copy of data */
static BOOL         three_button_mouseS ; /* whether 3 button mouse */
static INT          message_timeoutS ;/* how long to wait in message window */
static char persistent_messageS[LRECL];/* stores persistent message */
static BOOL persistenceS = TRUE ;    /* whether message is persistent */

#define MENUKEYWORD "MENU"
#define MENUBORDER  20               /* give menu extra pixels in width */
#define MENUYPOS    14               /* y position of top menu strings */
#define POS         5                /* start of MSG in pixels*/
#define WHITE       1                /* white parent gc is one in array */
#define BLACK       2                /* black parent gc is two in array */
#define CANCEL      0                /* code for cancel menu activity */
#define NUMWINDOWS  3                /* number of windows normally shown*/
#define DELETE_CH   '\177'           /* delete character */
#define BACKSPACE   '\010'           /* backspace character */
#define RETURN      '\015'           /* return charachter */
#define COMMENT     '#'              /* comment character in first col */
#define SLEEPTIME   (unsigned) 2     /* sleep for two seconds */

/* define static functions */
static set_window_lights( P1(BOOL flag) ) ;
static resize_windows( P2( INT winwidth, INT winheight ) ) ;
static debug_menus( P1(TWMENUPTR menu_field) ) ;
static draw_persistent_message( P1(char *message) ) ;


/* get information from main draw routine and set it */
TWinforMenus( )
{
    TWINFOPTR TWgetDrawInfo() ;

    infoS = TWgetDrawInfo() ;
    /* save display info for future use */
    dpyS = infoS->dpy ;
    screenS = infoS->screen ;
    drawS = infoS->drawWindow ;
    backS = infoS->backWindow ;
    pixmapS = infoS->pixmap ;
    fontinfoS = infoS->fontinfo ;
    fontS = fontinfoS->fid ;
    winwidthS = infoS->winwidth ;

} /* end set StaticInfo */

INT TWsaveState()
{
    /* turn off all event reporting to these windows */
    XSelectInput(dpyS,drawS,NoEventMask) ;
    XSelectInput(dpyS,backS,NoEventMask) ;
    XSelectInput(dpyS,menuS,NoEventMask) ;
    XSelectInput(dpyS,messageS,NoEventMask) ;
    set_window_lights( FALSE ) ;
    /* next flush event queue by calling XSync with discard true */
    XSync( dpyS, TRUE ) ;
    return( (INT) backS ) ;
} /* end TWgetWindowId */

TWrestoreState()
{
    long event_mask ;  /* used to set input selection to window */
    XWindowAttributes wattr;  /* get the window attributes */

    /* restore normal state to all the windows */
    event_mask = ButtonPressMask ;
    XSelectInput(dpyS,menuS,event_mask);
    event_mask = StructureNotifyMask | SubstructureNotifyMask
	    | VisibilityChangeMask ;
    if( three_button_mouseS ){
	event_mask |= ButtonPressMask ;
    }
    XSelectInput(dpyS,backS,event_mask);
    event_mask = ExposureMask | ButtonPressMask ;
    XSelectInput(dpyS,drawS,event_mask);
    XSelectInput(dpyS,messageS,NoEventMask) ;
    set_window_lights( TRUE ) ;
    /* next flush event queue by calling XSync with discard true */
    XSync( dpyS, TRUE ) ;
    /* now see if the user resized window in parasite */
    TWinforMenus() ;
    XGetWindowAttributes( dpyS, backS, &wattr ) ;
    wattr.height -= 2 * MENUHEIGHT ;
    if( wattr.width != infoS->winwidth ||
        wattr.height != infoS->winheight ){
	resize_windows( wattr.width, wattr.height ) ;  
    }

} /* end TWgetWindowId */

/* retrieve the window information only occurs during a parasite */
Window TWgetWindowId( dpy, backwindow )
Display *dpy ;
Window backwindow ;
{
    Atom menuAtom ; /* need to store menu property */
    Atom messageAtom ; /* need to store message property */
    Atom drawAtom ;    /* need to store draw property */
    Atom actual ;      /* actual atom return from XGetWindowProperty */
    char windowIdString[LRECL] ; /* buffer for window id string */
    unsigned char *prop; /* used to retrieve the property added to window */
    int junk1 ;
    unsigned long junk2 ; /* ignore junk */
    unsigned long length ; /* length of string */

    backS = backwindow ;
    parasiteS = TRUE ; /* save for future use */
    menuAtom    = XInternAtom( dpy, "menuWindowId", FALSE ) ;
    messageAtom = XInternAtom( dpy, "messageWindowId", FALSE ) ;
    drawAtom = XInternAtom( dpy, "drawWindowId", FALSE ) ;
    /* if parasite get windows back by looking at menu and */
    /* message properties stores in main window. Were stored */
    /* there on creation of those windows during normal case */
    XGetWindowProperty(dpy, backS, menuAtom, 0L, 200L, False,
	XA_STRING, &actual, &junk1, &junk2, &length, &prop);
    if( !(menuS = atoi( prop ))){
	M( ERRMSG,"TWinitMenus","Could not find menu property\n" ) ;
	return( FALSE ) ;
    }
    XGetWindowProperty(dpy, backS, messageAtom, 0, 200, False,
	XA_STRING, &actual, &junk1, &junk2, &length, &prop);
    if( !(messageS = atoi( prop ))){
	M( ERRMSG,"TWinitMenus","Could not find message property\n") ;
	return( FALSE ) ;
    }
    XGetWindowProperty(dpy, backS, drawAtom, 0, 200, False,
	XA_STRING, &actual, &junk1, &junk2, &length, &prop);
    if( !(drawS = atoi( prop ))){
	M( ERRMSG,"TWinitMenus","Could not find draw property\n") ;
	return( FALSE ) ;
    }
    /* new to return draw window */
    return( drawS ) ;
} /* end TWgetWindowId */

/* perform initialization of menu windows */
BOOL TWinitMenuWindow( menu_fields )
TWMENUPTR menu_fields ;
{
    MENUPTR menuptr ;  /* temporary for speed */
    TWMENUPTR mptr ;   /* current field of the menu fields */
    UNSIGNED_INT white, black ;
    INT  menu ; /* counter for menus */
    INT  items ; /* counter for menu items */
    INT  length ; /* length of string */
    INT  i, j ;   /* counters */
    INT  strwidth ; /* temp for calculating width of string in pixels */
    INT  xpos ;      /* temp to calculate xposition of menu */
    INT  halfstep ;       /* half the calculated stepsize */
    INT  entry_width ;    /* size of entry pixmap in pixels */
    INT  entry ;          /* keep count of entries in each menu */
    long event_mask ;     /* set up event catching with this mask */
    Atom menuAtom ; /* need to store menu property */
    Atom messageAtom ; /* need to store message property */
    Atom drawAtom ;    /* need to store draw property */
    Atom actual ;      /* actual atom return from XGetWindowProperty */
    char windowIdString[LRECL] ; /* buffer for window id string */
    char *prop;       /* used to retrieve the property added to window */
    INT junk1, junk2 ; /* ignore junk */
    char *reply ;      /* reply back from Xdefaults */

    /* get static information from main draw module */
    /* try to do crude object oriented programming */
    TWinforMenus() ;

    black = BlackPixel(dpyS,screenS);
    white = WhitePixel(dpyS,screenS);
    backgrdS = black ;
    foregrdS = white ;

    menuAtom    = XInternAtom( dpyS, "menuWindowId", FALSE ) ;
    messageAtom = XInternAtom( dpyS, "messageWindowId", FALSE ) ;
    drawAtom = XInternAtom( dpyS, "drawWindowId", FALSE ) ;
    if( parasiteS == FALSE ){
	/* normal case - need to create menu window */
	menuS = XCreateSimpleWindow( dpyS, backS, 1, 1, 
	    infoS->winwidth, MENUHEIGHT, 1L, white, backgrdS ) ;

	/* now raise menu window so we can see it */	
	XMapWindow( dpyS, menuS ) ;

	/* now create a message window below main window */
	messageS = XCreateSimpleWindow( dpyS, backS, 
	    1, infoS->winheight + MENUHEIGHT, 
	    infoS->winwidth, MENUHEIGHT,1L,white,backgrdS ) ;
	/* now raise message window so we can see it */	
	XMapWindow( dpyS, messageS ) ;
	XClearWindow( dpyS, messageS ) ;

	/* store the windowIds of the two window so that a parasite */
	/* can find its way later */
	sprintf( windowIdString, "%d", drawS ) ; /* store as a string */
	XChangeProperty( dpyS, backS, drawAtom, XA_STRING, 8, PropModeAppend,
	    (unsigned char *) windowIdString, strlen(windowIdString) ) ;
	sprintf( windowIdString, "%d", menuS ) ; /* store as a string */
	XChangeProperty( dpyS, backS, menuAtom, XA_STRING, 8, PropModeAppend,
	    (unsigned char *) windowIdString, strlen(windowIdString) ) ;
	sprintf( windowIdString, "%d", messageS ) ;/* store as a string */
	XChangeProperty( dpyS, backS, messageAtom, XA_STRING, 8, 
	    PropModeAppend, (unsigned char *) windowIdString, 
	    strlen(windowIdString) ) ;
    } /* end normal case */

    /* set graphic contexts */
    menuGCS = infoS->graphicContext[WHITE] ;
    menuRGCS = infoS->graphicContext[BLACK] ;

    /* user deliberately gave us a NULL menu so we have no more work */
    if( !(menu_fields) ){
	return( TRUE ) ;
    }

    if( XGetDefault( dpyS, GRAPHICS, "three_button_mouse" )){
        three_button_mouseS = TRUE ;
	event_mask = StructureNotifyMask | SubstructureNotifyMask
	    | VisibilityChangeMask | ButtonPressMask ;
	XSelectInput(dpyS,backS,event_mask) ;
    } else {
        three_button_mouseS = FALSE ;
    }

    if( reply = XGetDefault( dpyS, GRAPHICS, "message_timeout" )){
	/* time is in milliseconds */
	message_timeoutS = atoi( reply ) * 1000 ;
    } else {
	message_timeoutS = DEFAULT_TIMEOUT ;
    }

    /* count number of menus and menu items */
    numMenuS = 0 ;
    items = 0 ;
    for( i=0; menu_fields[i].item ; i++ ){ 
	if( menu_fields[i].menuNotItem ){
	    numMenuS++ ;
	}
	items++ ;
    }

    /* allocate memory for menu data */
    menuArrayS = YMALLOC( numMenuS, MENUPTR ) ;
    for( i=0; i< numMenuS; i++ ){
	menuArrayS[i] = YCALLOC( 1, MENUBOX ) ;
    }


    menu = -1 ;
    for( i=0; i < items; i++ ){ 
	mptr = &(menu_fields[i]) ;
	if( mptr->menuNotItem ){  /* start of a new menu */
	    
	    /* increment menu count */
	    menuptr = menuArrayS[++menu] ;
	    /* find width of menu string in characters */
	    length = strlen( mptr->item ) ;
	    menuptr->name_len = length ;
	    /* find width of menu in pixels */
	    menuptr->pix_len = 
		XTextWidth( fontinfoS, mptr->item, length ) ;
	    /* save menu name */
	    menuptr->name = Ystrclone( mptr->item ) ;

	} else if( menu >= 0 ){ /* avoid obvious error if menu -1 */
	    /* add to number of entries */
	    menuArrayS[menu]->numentries++ ;
	    /* find maximum length of entries of this menu */
	    length  = strlen( mptr->item ) ;
	    strwidth = XTextWidth( fontinfoS, mptr->item, length ) ;
	    menuArrayS[menu]->entry_wid =
		MAX( menuArrayS[menu]->entry_wid, strwidth ) ;

	    if( mptr->bool_item ){ /* also check the complement */
		length  = strlen( mptr->bool_item ) ;
		strwidth = XTextWidth( fontinfoS,mptr->bool_item,length );
		menuArrayS[menu]->entry_wid =
		    MAX( menuArrayS[menu]->entry_wid, strwidth ) ;
	    }
	}
    }
    if( numMenuS < 0 || numMenuS != ++menu ){
	M( ERRMSG, "initMenus", "Problems with menu array\n" ) ;
	return( FALSE ) ;
    }

    /* first calculate position of the menus based on equal spacing */
    stepsizeS = winwidthS / numMenuS ;
    halfstep = stepsizeS / 2 ;
    xpos = halfstep ;  /* placement of first menu */

    /* build menus */
    menu = -1 ;
    for( i=0; i < items; i++ ){ 
	mptr = &(menu_fields[i]) ;

	if( mptr->menuNotItem ){  /* start of a new menu */
	    /* increment menu count */
	    menuptr = menuArrayS[++menu] ;
	    /* save where to draw menu title in menu window */
	    menuptr->xpos = halfstep - menuptr->pix_len / 2 ;
	    menuptr->width = entry_width = menuptr->entry_wid +
		MENUBORDER ;

	    /* create a top subwindow */
	    menuptr->top_window = XCreateSimpleWindow( dpyS, menuS, 
		    xpos - halfstep, 0, 
		    stepsizeS, MENUHEIGHT, 0L, black, backgrdS ) ;
	    /* now raise window so we can see it */	
	    XMapWindow( dpyS, menuptr->top_window ) ;

	    /* set the events for top subwindow */
	    event_mask = EnterWindowMask | LeaveWindowMask ;
	    XSelectInput(dpyS,menuptr->top_window,event_mask) ;


	    /* create an array of simple windows for menus */
	    menuptr->window = YMALLOC( menuptr->numentries, Window ) ;
	    for( j=0; j< menuptr->numentries; j++ ){
		menuptr->window[j] = XCreateSimpleWindow( dpyS, backS, 
		    xpos - menuptr->width / 2,
		    (j+1) * MENUHEIGHT, 
		    entry_width,
		    MENUHEIGHT, 1L, black, backgrdS ) ;
	    }

	    /* now create entry array for each menu entry string */
	    menuptr->entry = YMALLOC( menuptr->numentries, char * ) ;
	    /* now create function array for each entry window */
	    menuptr->function = YMALLOC( menuptr->numentries, INT ) ;
	    /* now create adjusted position array for each menu entry */
	    menuptr->xpos_adj = YMALLOC( menuptr->numentries, INT ) ;
	    /* now create name len array for each menu entry */
	    menuptr->entry_len = YMALLOC( menuptr->numentries, INT ) ;
	    /* now create enabled array for each entry window */
	    menuptr->enabled = YMALLOC( menuptr->numentries, BOOL ) ;

	    /* now create second version of arrays for Boolean */
	    /* complementary state */
	    menuptr->bool_entry = YMALLOC( menuptr->numentries, BOOL ) ;
	    menuptr->state = YMALLOC( menuptr->numentries, BOOL ) ;
	    menuptr->entry2 = YMALLOC( menuptr->numentries, char * ) ;
	    menuptr->function2 = YCALLOC( menuptr->numentries, INT ) ;
	    menuptr->xpos_adj2 = YMALLOC( menuptr->numentries, INT ) ;
	    /* now create name len array for each menu entry */
	    menuptr->entry_len2 = YMALLOC( menuptr->numentries, INT ) ;
	    entry = 0 ; /* reset entry */
	    /* now update position of next menu */
	    xpos += stepsizeS ;

	} else {
	    
	    length = strlen( mptr->item ) ;
	    /* save string in array */
	    menuptr->entry[entry] = Ystrclone( mptr->item ) ;
	    /* save string len in array */
	    menuptr->entry_len[entry] = length ;
	    /* save function number - last token */
	    menuptr->function[entry] = mptr->action_index ;
	    menuptr->enabled[entry] = TRUE ;

	    strwidth = XTextWidth( fontinfoS, mptr->item, length ) ;
	    /* save where to draw string */
	    menuptr->xpos_adj[entry] = (menuptr->width-strwidth) / 2 ;
	    if( mptr->bool_item ){
		menuptr->bool_entry[entry] = TRUE ;
		length  = strlen( mptr->bool_item ) ;
		/* save string in array */
		menuptr->entry2[entry] = Ystrclone( mptr->bool_item ) ;
		/* save string len in array */
		menuptr->entry_len2[entry] = length ;
		/* save function number - last token */
		menuptr->function2[entry] = mptr->action_indexb ;
		strwidth = XTextWidth( fontinfoS,mptr->bool_item,length );
		/* save where to draw string */
		menuptr->xpos_adj2[entry] = 
		    (menuptr->width - strwidth) / 2 ;
		/* set initial state 1 means first entry set active */
		menuptr->state[entry] = mptr->bool_init ;

	    } else { /* number of tokens == 2 */
		menuptr->bool_entry[entry] = FALSE ;
	    }
	    /* increment entry for next string */
	    entry++ ;
	}

    } /* end for loop */

    /* now enable it so we can see mouse button in this window */
    event_mask = ButtonPressMask ;
    XSelectInput(dpyS,menuS,event_mask);

    /* sucessful if we got to here */
    return( TRUE ) ;

	
} /* end TWinitMenuWindow */

TWdrawMenus()
{
    INT i ;
    MENUPTR menuptr ;

    XClearWindow( dpyS, menuS ) ;
    for( i = 0; i< numMenuS ; i++ ){
	menuptr = menuArrayS[i] ;
	XClearWindow( dpyS, menuptr->top_window ) ;
	XDrawString( dpyS, menuptr->top_window, menuGCS, 
	    menuptr->xpos, MENUYPOS,
	    menuptr->name, menuptr->name_len ) ;
    }
    XFlush( dpyS ) ;

} /* end TWdrawMenus */

/* turn top window entering and leaving lights */
static set_window_lights( flag )
BOOL flag ;
{
    INT i ;            /* window counter */
    MENUPTR menuptr ;  /* pointer to current window */
    long event_mask ;  /* used to set input selection to window */

    if( flag ){
	/* turn on menu subwindows */
	event_mask = EnterWindowMask | LeaveWindowMask ;
	for( i = 0; i< numMenuS ; i++ ){
	    menuptr = menuArrayS[i] ;
	    XSelectInput(dpyS,menuptr->top_window,event_mask) ;
	}
    } else {
	/* turn off menu subwindows */
	for( i = 0; i< numMenuS ; i++ ){
	    menuptr = menuArrayS[i] ;
	    XSelectInput(dpyS,menuptr->top_window,NoEventMask) ;
	}
    }
} /* end set_window_lights */

/* draw whether cursor is entering window */
static BOOL checkwindow_lights()
{
    long event_mask ;  /* used to set input selection to window */
    INT i ;            /* window counter */
    MENUPTR menuptr ;  /* menu pointer */
    Window win ;       /* match window */
    BOOL foundWindow;  /* flag to match window */
    XEvent event ;     /* describes entering or leaving event */

    event_mask = EnterWindowMask | LeaveWindowMask ;
    foundWindow = FALSE ;

    if( XCheckMaskEvent( dpyS, event_mask, &event ) ){
	switch( event.type ){
	case EnterNotify:
	    /* light up window */
	    /* find window match */
	    win = event.xcrossing.window ;
	    foundWindow = FALSE ;
	    for( i = 0; i < numMenuS; i++ ){
		if( win == menuArrayS[i]->top_window ){
		    menuptr = menuArrayS[i] ;
		    foundWindow = TRUE ;
		    break ;
		}
	    } /* end search for window */

	    if( foundWindow ){ /* a match light up window */
		XFillRectangle( dpyS,win, menuGCS,
		    2,2,
		    stepsizeS-2, MENUHEIGHT-4 ) ;
		XDrawImageString( dpyS, win, menuRGCS, 
		    menuptr->xpos, MENUYPOS,
		    menuptr->name, menuptr->name_len ) ;
	    }
	    break ;
		
	case LeaveNotify:
	    /* turn off window */
	    /* find window match */
	    win = event.xcrossing.window ;
	    foundWindow = FALSE ;
	    for( i = 0; i < numMenuS; i++ ){
		if( win == menuArrayS[i]->top_window ){
		    menuptr = menuArrayS[i] ;
		    foundWindow = TRUE ;
		    break ;
		}
	    } /* end search for window */

	    if( foundWindow ){ /* turn off window */
		XClearWindow( dpyS, win ) ;
		XDrawString( dpyS, win, menuGCS, 
		    menuptr->xpos, MENUYPOS,
		    menuptr->name, menuptr->name_len ) ;
	    }
	    break ;
	} /* end switch */
    }
    return( foundWindow ) ;
} /* end checkwindow_lights */


/* check to see mouse button was click.  If true put up appropriate */
/* menu and return value to user */
INT TWcheckMouse()
{
    BOOL foundWindow ;      /* used in window search to find match */
    XEvent event ;          /* describes button event */
    INT i ;                 /* menu desired by user */
    INT x ;                 /* pixel location button was pushed */
    INT menu_requested ;    /* the menu the user requested */
    long event_mask ;       /* setup menus */
    Window win ;            /* temporary for selected menu window */
    MENUPTR menuptr ;       /* temporary for selected menu record */
    BOOL press ;            /* tells whether button has been pushed */
    GC menu_GC ;            /* normal menu graphics context */
    GC reverse_menuGC ;     /* reverse menu context */
    static INT last_timeL=0;/* the last time we interupted */
    static INT last_commandL = CANCEL ; /* the last command issued */
    static INT countL = 0 ; 		/* the last command issued */

    INT cur_time ;          /* the current time since start of process */

    /* now avoid looking for interuptions all the time */
    /* it take too much time*/
    Ytimer_elapsed( &cur_time ) ;
    if( cur_time - last_timeL > 10000 ){
	sleep( (UNSIGNED_INT) 1 ) ;
    }

    /* always set window lights on in this routine */
    set_window_lights( TRUE ) ;

    /* always check for window reconfiguration calls */
    TWcheckReconfig() ;

    if( checkwindow_lights() ){
	last_timeL = cur_time ;
    }

    /* check if there is any mouse activity */
    if( three_button_mouseS ){
	
	press = XCheckMaskEvent( dpyS, ButtonPressMask,&event ) ;
	D( "TWcheckMouse",
	    if( press ){
		fprintf( stderr, "window = %d\n", event.xbutton.window ) ;
	    }
	) ;
	if(!(press)){
	    /* nothin' shakin' */
	    return( CANCEL ) ;
	} else if( event.xbutton.window != menuS ){
	    if( event.xbutton.button == 2 ){
		if( last_commandL != CANCEL ){
		    TWmessage( "Repeating last command..." ) ;
		    if( ++countL <= 3 ){
			(VOID) sleep( (unsigned) 2 ) ;
		    }
		    return( last_commandL ) ;
		}
	    }
	    /* nothin' shakin' */
	    return( CANCEL ) ;
	}
    } else {
	/* a one button mouse */
	press = XCheckTypedWindowEvent( dpyS,menuS,ButtonPress,&event ) ;
	if(!(press)){
	    /* nothin' shakin' */
	    return( CANCEL ) ;
	} 
    }
    last_timeL = cur_time ;

    /* ****** otherwise process the mouse button press ******** */
    D( "TWcheckMouse",
	fprintf( stderr,"Button event-button:%d\n",event.xbutton.button);
    ) ;

    /* now disable top menu so we cant get false clicks */
    event_mask = NoEventMask ;
    XSelectInput(dpyS,menuS,event_mask);

    /* now disable main window clicks so we cant get false clicks */
    event_mask = NoEventMask ;
    XSelectInput(dpyS,drawS,event_mask) ;

    /* now draw menus */
    TWdrawMenus() ;

    /* determine which menu has been selected */
    x = event.xbutton.x ;
    i = x / stepsizeS ;
    /* make sure we are in range */
    if( i >= numMenuS ){
	i = numMenuS - 1 ;
    }
    /* pick window and menu for speed */
    menuptr = menuArrayS[i] ;

    /* show all windows for this menu and initialize input selection*/
    /* next draw the menu entry string */
    event_mask = ButtonPressMask | EnterWindowMask | LeaveWindowMask ;
    for( i = 0 ; i < menuptr->numentries; i++ ){
	XMapRaised( dpyS, menuptr->window[i] ) ;
	XSelectInput(dpyS,menuptr->window[i],event_mask);
	XClearWindow( dpyS, menuptr->window[i] ) ;
	/* check to see if we have a boolean menu entry */
	if( menuptr->enabled[i] ){ 
	    menu_GC = menuGCS ;
	} else {
	    menu_GC = infoS->graphicContext[TWGRAY] ;
	}
	if( menuptr->bool_entry[i] ){
	    /* see what state we are in */
	    if( menuptr->state[i] ){
		/* always draw what to change to */
		XDrawString( dpyS, menuptr->window[i], menu_GC, 
		    menuptr->xpos_adj2[i], MENUYPOS,
		    menuptr->entry2[i], menuptr->entry_len2[i] ) ;
	    } else {  /* draw the complement */
		XDrawString( dpyS, menuptr->window[i], menu_GC, 
		    menuptr->xpos_adj[i], MENUYPOS,
		    menuptr->entry[i], menuptr->entry_len[i] ) ;
	    }
	} else { /* regular menu entry */
	    XDrawString( dpyS, menuptr->window[i], menu_GC, 
		menuptr->xpos_adj[i], MENUYPOS,
		menuptr->entry[i], menuptr->entry_len[i] ) ;
	}
    }
    /* now draw menus */
    XFlush( dpyS ) ;

    /* now look for button press */
    press = FALSE ;
    menu_requested = -1 ;

    while(!(press )){
	if( XCheckMaskEvent( dpyS, event_mask, &event ) ){
	    switch( event.type ){
	    case ButtonPress:
		/* how we exit */
		win = event.xbutton.window ; /* window event occurred */
		for( i = 0; i < menuptr->numentries; i++ ){
		    if( win == menuptr->window[i] && menuptr->enabled[i] ){ 
			menu_requested = i ;
			press = TRUE ;
		    }
		}
		break ;
	    case EnterNotify:
		/* light up window */
		/* find window match */
		win = event.xcrossing.window ;
		foundWindow = FALSE ;
		for( i = 0; i < menuptr->numentries; i++ ){
		    if( win == menuptr->window[i] ){ 
			/* a match if enabled */
			foundWindow = TRUE ;
			break ;
		    }
		} /* end search for window */

	    
		if( foundWindow ){ /* a match - light up window */
		    if( menuptr->enabled[i] ){ 
			menu_GC = menuGCS ;
			reverse_menuGC = menuRGCS ;
		    } else {
			menu_GC = infoS->graphicContext[TWGRAY] ;
			reverse_menuGC = infoS->graphicContext[TWGRAY] ;
		    }
		    XFillRectangle( dpyS,event.xcrossing.window,
			menu_GC,
			0,0,menuptr->width,MENUHEIGHT ) ;
		    /* check for boolean entry */
		    if( menuptr->bool_entry[i] ){
			/* see what state we are in */
			if( menuptr->state[i] ){
			    /* always draw what to change to */
			    XDrawImageString( dpyS, menuptr->window[i], 
				reverse_menuGC, 
				menuptr->xpos_adj2[i], MENUYPOS,
				menuptr->entry2[i],
				menuptr->entry_len2[i]) ;
			} else {  /* draw the complement */
			    XDrawImageString( dpyS, menuptr->window[i], 
				reverse_menuGC, menuptr->xpos_adj[i], MENUYPOS,
				menuptr->entry[i],menuptr->entry_len[i]) ;
			}
		    } else { /* regular menu entry */
			XDrawImageString( dpyS, menuptr->window[i], 
			    reverse_menuGC, menuptr->xpos_adj[i], MENUYPOS,
			    menuptr->entry[i], menuptr->entry_len[i] ) ;
		    }
		}
		break ;
		    
	    case LeaveNotify:
		/* turn off window */
		/* find window match */
		win = event.xcrossing.window ;
		foundWindow = FALSE ;
		for( i = 0; i < menuptr->numentries; i++ ){
		    if( win == menuptr->window[i] ){
			foundWindow = TRUE ;
			break ;
		    }
		} /* end search for window */

		if( foundWindow ){ /* turn off window */
		    XClearWindow( dpyS, menuptr->window[i] ) ;
		    /* check for boolean entry */
		    if( menuptr->enabled[i] ){ 
			menu_GC = menuGCS ;
		    } else {
			menu_GC = infoS->graphicContext[TWGRAY] ;
		    }
		    if( menuptr->bool_entry[i] ){
			/* see what state we are in */
			if( menuptr->state[i] ){
			    /* always draw what to change to */
			    XDrawString( dpyS, menuptr->window[i], 
				menu_GC, menuptr->xpos_adj2[i], MENUYPOS,
				menuptr->entry2[i],
				menuptr->entry_len2[i]) ;
			} else {  /* draw the complement */
			    XDrawString( dpyS, menuptr->window[i], 
				menu_GC, menuptr->xpos_adj[i], MENUYPOS,
				menuptr->entry[i],menuptr->entry_len[i]) ;
			}
		    } else { /* regular menu entry */
			XDrawString( dpyS, menuptr->window[i], 
			    menu_GC, menuptr->xpos_adj[i], MENUYPOS,
			    menuptr->entry[i], menuptr->entry_len[i] ) ;
		    }
		}
		break ;
	    }
	}
    }

    /* we got an answer unmap window and turn off events for these */
    /* windows */
    event_mask = NoEventMask ;
    for( i = 0 ; i < menuptr->numentries; i++ ){
	XUnmapWindow( dpyS, menuptr->window[i] ) ;
	XSelectInput(dpyS,menuptr->window[i],event_mask);
    }

    /* now again enable top menu so we can get clicks */
    event_mask = ButtonPressMask ;
    XSelectInput(dpyS,menuS,event_mask);

    /* turn on the drawing window */
    event_mask = ExposureMask | ButtonPressMask ;
    XSelectInput(dpyS,drawS,event_mask);

    XCopyArea( dpyS, pixmapS, drawS, menuGCS, 0L, 0L, infoS->winwidth,
	infoS->winheight, 0L, 0L ) ;

    /* determine which menu entry has been selected */
    if( menu_requested >= 0 ){
	/* we need to set the state to complement if boolean menu */
	if( menuptr->bool_entry[menu_requested] ){
	    if( menuptr->state[menu_requested] ){
		/* was true change to false */
		menuptr->state[menu_requested] = FALSE ;
		last_commandL = menuptr->function2[menu_requested] ;

		D( "TWcheckMouse",fprintf( stderr,"return %d\n", 
		    last_commandL ) ) ;
		return( last_commandL ) ;
	    } else {
		/* was false change to true */
		menuptr->state[menu_requested] = TRUE ;
		last_commandL = menuptr->function[menu_requested] ;
		D( "TWcheckMouse",fprintf( stderr, "return %d\n",
		    last_commandL ) ) ;
		return( last_commandL ) ;
	    }
	}
	last_commandL = menuptr->function[menu_requested] ;
	D( "TWcheckMouse",
	    fprintf( stderr, "return %d\n", last_commandL ) ) ;
	return( last_commandL ) ;
    }

    /* outside windows return cancel */
    return( CANCEL ) ;

} /* end TWcheckMouse */

TWdisableMenu( menu_item )
INT menu_item ;
{
    INT      menu ;            /* counter */
    INT      entry ;           /* counter */
    MENUPTR menuptr ;          /* temporary for selected menu record */

    for( menu = 0 ; menu < numMenuS; menu++ ){
	menuptr = menuArrayS[menu] ;
	for( entry = 0 ; entry < menuptr->numentries; entry++ ){
	    if( menu_item == menuptr->function[entry] ){
		menuptr->enabled[entry] = FALSE ;
		return ;
	    }
	    if( menu_item == menuptr->function2[entry] ){
		menuptr->enabled[entry] = FALSE ;
		return ;
	    }
	}
    }
} /* end TWdisableMenu() */

TWenableMenu( menu_item )
INT menu_item ;
{
    INT      menu ;            /* counter */
    INT      entry ;           /* counter */
    MENUPTR menuptr ;          /* temporary for selected menu record */

    for( menu = 0 ; menu < numMenuS; menu++ ){
	menuptr = menuArrayS[menu] ;
	for( entry = 0 ; entry < menuptr->numentries; entry++ ){
	    if( menu_item == menuptr->function[entry] ){
		menuptr->enabled[entry] = TRUE ;
		return ;
	    }
	    if( menu_item == menuptr->function2[entry] ){
		menuptr->enabled[entry] = TRUE ;
		return ;
	    }
	}
    }
} /* end TWenableMenu() */

TWgetPt( x, y )
INT *x, *y ;
{
    BOOL press ;            /* tells whether button has been pushed */
    XEvent event ;          /* describes button event */
    INT xtemp, ytemp ;      /* pixel location button was pushed */
    long event_mask ;       /* setup menus */
    
    /* turn on event mask for main drawing window - known as drawS */
    event_mask = ExposureMask | ButtonPressMask ;
    XSelectInput(dpyS,drawS,event_mask);

    /* now disable top menu so we cant get false clicks */
    event_mask = NoEventMask ;
    XSelectInput(dpyS,menuS,event_mask);

    /* now look for button press */
    press = FALSE ;
    while(!(press )){
	press = XCheckTypedWindowEvent( dpyS,drawS,ButtonPress,&event );
	if( press ){
	    /* now disable window so we can ignore extraneous clicks */
	    event_mask = ExposureMask ;
	    XSelectInput(dpyS,drawS,event_mask);

	    /* determine which menu entry has been selected */
	    xtemp = event.xbutton.x ;
	    ytemp = event.xbutton.y ;
	    /* account for inversion of y axis */
	    ytemp = infoS->winheight - ytemp ;
	    /* now reverse scale of coordinates */
	    xtemp = (INT) ( (DOUBLE) xtemp / infoS->scaleFactor ) ;
	    ytemp = (INT) ( (DOUBLE) ytemp / infoS->scaleFactor ) ;
	    /* now apply data offset */
	    *x = xtemp - infoS->xoffset ;
	    *y = ytemp - infoS->yoffset ;
	    D( "TWgetPt", fprintf( stderr, "pt = (%d,%d)\n", *x, *y ) ) ;
	}
    }
    /* now again enable top menu so we can get clicks */
    event_mask = ButtonPressMask ;
    XSelectInput(dpyS,menuS,event_mask);
    
} /* end TWgetPt */

TWmessage( message )
char *message ;
{
    if( persistenceS ){
	if( message ){
	    strcpy( persistent_messageS, message ) ;
	} else {
	    persistent_messageS[0] = EOS ;
	}
    }
    draw_persistent_message( message ) ;

} /* end TWmessage */

TWmessagePersistence(flag)
BOOL flag ;
{
    persistenceS = flag ;
    if( flag ){
	/* if turning on the flag again draw persistent message */
	draw_persistent_message( NULL ) ;
    }
} /* end TWmessagePersistence() */

static draw_persistent_message( non_persistent_message )
char *non_persistent_message ;
{
    INT fwidth ; /* font width */
    char *message ;   /* message to output */

    XClearWindow( dpyS, messageS ) ;
    XDrawString( dpyS, messageS, infoS->graphicContext[WHITE], 
	POS,MENUYPOS, "MSG>", 4 ) ;
    if( non_persistent_message ){
	message = non_persistent_message ;
    } else if( persistenceS && persistent_messageS[0] ){
	message = persistent_messageS ;
    } else {
	message = NULL ;
    }
    if( message ){
	fwidth = XTextWidth( fontinfoS, "MSG>", 4 ) + POS ;
	XDrawString( dpyS, messageS, infoS->graphicContext[WHITE], 
	    POS+fwidth,MENUYPOS,message,strlen(message));
    }
    XFlush( dpyS ) ;
} /* end draw_persistent_message() */

char *TWgetString( directions )
char *directions ;
{
    BOOL press ;            /* tells whether keyboard has been pushed */
    BOOL finish ;           /* tells whether we have received a return */
    XEvent event ;          /* describes button event */
    long event_mask ;       /* setup menus */
    char buffer[LRECL] ;    /* used for keyboard translation */
    char curMsg[LRECL] ;    /* current value of message window */
    static char data[LRECL];/* current value of users input */
    KeySym keysym ;         /* return of keysym from user */
    XComposeStatus status ; /* where a compose key was pressed */
    INT strwidth ;          /* width of string in pixels */
    INT dataCount ;         /* number of characters in user input */
    INT cur_time ;          /* the current time */
    INT start_time ;        /* the start or last activity time */

    event_mask = KeyPressMask ;
    XSelectInput(dpyS,messageS,event_mask);

    TWmessage( directions ) ;

    /* now move pointer to line after directions to receive data */
    strwidth = XTextWidth( fontinfoS, directions, strlen(directions)) ;
    /* now warp pointer to message window */
    XWarpPointer( dpyS, None, messageS, 0, 0, 0, 0, 
	strwidth+40, MENUYPOS ) ;
    XSetInputFocus( dpyS, messageS, RevertToPointerRoot, CurrentTime ) ;

    /* initialize string buffers */
    data[0] = EOS ;
    dataCount = 0 ;

    /* now look for keyboard action */
    finish = FALSE ;
    /* get the starting time for timeout */
    Ytimer_elapsed( &start_time ) ;
    while(!(finish )){
	press = XCheckTypedWindowEvent( dpyS,messageS,KeyPress,&event );
	if( press ){
	    /* initialize buffer */
	    buffer[0] = EOS ;
	    /* activity reset the timer */
	    Ytimer_elapsed( &start_time ) ;
	    /* find what the user entered */
	    XLookupString( &(event.xkey), buffer,LRECL,&keysym, &status );
	    buffer[1] = EOS ; /* only get one character at a time */
	    D( "TWgetString",fprintf( stderr, "string:%s\n", buffer ) ) ;

	    /* look to see if we got a return */
	    if( buffer[0] == RETURN ){
		finish = TRUE ;
	    } else {
		/* look for more data */
		/* but first copy the data we have */
		if( buffer[0] == BACKSPACE || buffer[0] == DELETE_CH ){ 
		    /* look for backspace or delete */
		    if( dataCount > 0 ){
			dataCount-- ;
		    }
		    data[dataCount] = EOS ;
		    
		} else {
		    /* save data */
		    strcat( data, buffer ) ;
		    dataCount += strlen( buffer ) ;
		}
		/* now echo to screen */
		/* build current msg */
		/* initialize curMsg */
		curMsg[0] = EOS ;
		strcat( curMsg, directions ) ;
		strcat( curMsg, data ) ;
		TWmessage( curMsg ) ;
	    }

	} else {
	    /* check for no activity */
	    Ytimer_elapsed( &cur_time ) ;
	    if( cur_time - start_time > message_timeoutS ){
		TWmessage( "No activity timeout" ) ;
		data[0] = EOS ;
		break ; /* jump out of while loop */
	    }
	}
    }
    /* now again disable message window so we dont get false info */
    event_mask = NoEventMask ;
    XSelectInput(dpyS,messageS,event_mask);
    XSetInputFocus( dpyS, PointerRoot, RevertToParent, CurrentTime );

    /* clear message window */
    TWmessage( NULL ) ;

    /* if no data was entered return NULL otherwise return data */
    if( data[0] == EOS ){
	return( NULL ) ;
    } else {
	return(data) ;
    }
   
} /* end TWgetString() */


/* TWgetPt2 allows the user to get a point from either the keyboard */
/* or from the using the mouse */
/* returns TRUE if entered from keyboard, false from mouse */
BOOL TWgetPt2( x, y )
INT *x, *y ;
{
    BOOL press ;                /* tells whether button has been pushed */
    BOOL ok ;                     /* whether keyboard input is ok */
    BOOL method ;                 /* input is from keyboard or not? */
    XEvent event ;                /* describes button event */
    long event_mask ;             /* setup input */
    char **tokens;                /* for parsing keyboard data */
    INT numtokens ;               /* number of tokens in keyboard str */
    char *reply ;                 /* answer from user */
    
    /* turn on event mask for main drawing window - known as wS */
    event_mask = ExposureMask | ButtonPressMask ;
    XSelectInput(dpyS,drawS,event_mask);

    /* also look for events in message window */
    event_mask = KeyPressMask ;
    XSelectInput(dpyS,messageS,event_mask);

    /* now disable top menu so we cant get false clicks */
    event_mask = NoEventMask ;
    XSelectInput(dpyS,menuS,event_mask);

    /* now look for either event - button press or keyboard */
    press = FALSE ;
    while(!(press )){
	/* check for user input from keyboard */
	if( press=XCheckTypedWindowEvent(dpyS,messageS,KeyPress,&event)){
	    /* we have an event from the keyboard */
	    /* put event back on queue  and call TWgetString */
	    XPutBackEvent( dpyS, &event ) ;
	    ok = FALSE ;
	    reply = TWgetString( "Pick or enter point:" ) ;
	    while(!(ok)){
		/* now we need to parse back reply */
		tokens = Ystrparser( reply, ", \t\n", &numtokens );
		if( numtokens == 2 ){
		    *x = atoi( tokens[0] ) ;
		    *y = atoi( tokens[1] ) ;
		    /* now again enable top menu so we can get clicks */
		    event_mask = ButtonPressMask ;
		    XSelectInput(dpyS,menuS,event_mask);
		    ok = TRUE ;
		    method = TRUE ;
		} else {
		    reply = TWgetString( 
		    "Invalid input - please enter again! eg. 10, 20<cr>");
		}
	    } /* end keyboard processing loop */
	    
         /* other wise - check mouse */
	} else if( press = XCheckTypedWindowEvent( dpyS,drawS,
		ButtonPress,&event ) ){
	    /* we have an event from the pointer */
	    /* put event back on queue  and call TWgetPt */
	    XPutBackEvent( dpyS, &event ) ;
	    TWgetPt( x, y ) ;
	    method = FALSE ;
	} /* otherwise continue to loop */
    } /* end wait loop for user response */

    /* now again disable message window so we dont get false info */
    event_mask = NoEventMask ;
    XSelectInput(dpyS,messageS,event_mask);

    /* now disable window so we can ignore extraneous clicks */
    event_mask = ExposureMask ;
    XSelectInput(dpyS,drawS,event_mask);

    return( method ) ;

} /* end TWgetPt2 */

/* start receiving events concerning mouse tracking */
TWmouse_tracking_start()
{
    long event_mask ;         /* set events */

    /* turn on event mask for main drawing window - known as wS */
    event_mask = StructureNotifyMask | SubstructureNotifyMask
	| VisibilityChangeMask | ExposureMask | ButtonPressMask |
	PointerMotionMask ;
    XSelectInput(dpyS,drawS,event_mask);
	
} /* end TWmouse_tracking_start */

/* get the current mouse position */
/* returns true if position has changed */
BOOL TWmouse_tracking_pt( x, y )
INT *x, *y ;
{
    XEvent event ;            /* describes event */
    INT xtemp, ytemp ;        /* current position of pointer */
    BOOL changed ;            /* whether position has changed */
    static INT xoldL, yoldL;  /* position from previous call */

    xtemp = xoldL ;           /* restore old point */
    ytemp = yoldL ;           /* restore old point */
    while( XCheckTypedWindowEvent( dpyS,drawS, MotionNotify,&event ) ){
	/* avoid to many events to screen wait 50 msec.*/
	xtemp = event.xmotion.x ;
	ytemp = event.xmotion.y ;
    } /* end while loop */

    if( xtemp == xoldL && ytemp == yoldL ){
	/* nothing has changed */
	changed = FALSE ;
    } else {
	/* position has changed */
	changed = TRUE ;
    }
    xoldL = xtemp ;     /* save current position for next call */
    yoldL = ytemp ;

    /* account for inversion of y axis */
    ytemp = infoS->winheight - ytemp ;
    /* now reverse scale of coordinates */
    xtemp = (INT) ( (DOUBLE) xtemp / infoS->scaleFactor ) ;
    ytemp = (INT) ( (DOUBLE) ytemp / infoS->scaleFactor ) ;
    /* now apply data offset */
    *x = xtemp - infoS->xoffset ;
    *y = ytemp - infoS->yoffset ;
    D( "TWmouse_tracking_pt", fprintf( stderr,"pt = (%d,%d)\n", *x, *y ));
    XFlush( dpyS ) ;

    return( changed ) ;
	
} /* end TWmouse_tracking_pt */

BOOL TWmouse_tracking_end()
{
    long event_mask ;         /* set events */
    XEvent event ;            /* describes event */


    if( XCheckTypedWindowEvent( dpyS, drawS, ButtonPress,&event ) ){
	/* turn off motion notify events for mouse */
	event_mask = StructureNotifyMask | SubstructureNotifyMask
	    | VisibilityChangeMask | ExposureMask | ButtonPressMask ;
	XSelectInput(dpyS,drawS,event_mask);
	return( TRUE ) ;
    } else {
	return( FALSE ) ;
    }

} /* end TWmouse_tracking_end */

/* check to see if main window has been covered by another window */
BOOL TWcheckExposure()
{


    BOOL exposed ;          /* tells whether window has been covered */
    XEvent event ;          /* describes event */
    INT time ;              /* current time */
    static INT lasttimeL ;  /* last time of exposure event */

    exposed = FALSE ;
    /* XSync( dpyS, FALSE ) ;*/ /* allow program to catch up */

    /* check if there was any exposed event activity */
    while( XCheckTypedWindowEvent( dpyS, drawS, Expose, &event ) ){
	/* if there is more than one expose event flush from queue */
	/* look at count to find only last expose event */
	if( event.xexpose.count == 0 ){
	    exposed = TRUE ;
	}
    }
    /* window managers sometimes send us too many exposure events */
    /* therefore check time and make multiple exposures invalid */
    if( exposed ){
	(void) YcurTime( &time ) ;
	if( event.xexpose.send_event == True ){
	    /* if we got a TWforceRedraw always redraw screen */
	    lasttimeL = time ;
	    D( "TWcheckExposure", 
		fprintf( stderr,"Exposure:f @time = %d\n",time);
	    ) ;
	    if( exposed ){
		draw_persistent_message( NULL ) ;
	    }
	    return( exposed ) ;
	}
	if( time - lasttimeL < TWsafe_wait_timeG ){
	    exposed = FALSE ;
	} else {
	    lasttimeL = time ;
	}
	D( "TWcheckExposure", 
	    fprintf( stderr,"Exposure:%d @time = %d\n",exposed,time);
	) ;
    }
    if( exposed ){
	draw_persistent_message( NULL ) ;
    }
    return( exposed ) ;
} /* end TWcheckExpose */

/* returns true if user has clicked in menu window */
/* flushes all remaining button presses in this window */
/* also sets entering and leaving window lighting for subwindows */
/* if we are using TWinterupt we which to turn off menu subwindows */
BOOL TWinterupt()
{
    static INT last_timeL=0; /* the last time we interupted */
    INT cur_time ;          /* the current time since start of process */
    BOOL press ;            /* tells whether button has been pushed */
    XEvent event ;          /* describes button event */

    /* now avoid looking for interuptions all the time */
    /* it take too much time*/
    Ytimer_elapsed( &cur_time ) ;
    if( cur_time - last_timeL < 1000 ){
	return( FALSE ) ;
    }
    last_timeL = cur_time ;
    press = FALSE ;
    /* XSync( dpyS, FALSE ) ; */ /* allow program to catch up */
    /* check if there is any mouse activity */
    while( XCheckTypedWindowEvent( dpyS, menuS, ButtonPress, &event ) ){
	/* if there is more than one button event flush from queue */
	press = TRUE ;
    }
    set_window_lights( press ) ;
    return( press ) ;

} /* end TWinterupt */

/* update windows if configuration changes */
TWcheckReconfig()
{
    INT height ;              /* height of current backing window */
    XEvent event ;            /* describes configuration event */
    INT winwidth, winheight ; /* size of window */
    BOOL redraw = FALSE ;     /* whether to redraw or not */

    height = infoS->winheight + 2 * MENUHEIGHT ;
    /* since all windows are children of backS only check backS */
    while( XCheckTypedEvent( dpyS, ConfigureNotify, &event ) ){
	if( event.xconfigure.window == backS ){
	    if( event.xconfigure.override_redirect ){
		/* window manager is telling us to leave it alone */
		continue ;
	    }
	    winheight = event.xconfigure.height ;
	    winwidth = event.xconfigure.width ;
	    if( winheight == height && 
		winwidth == infoS->winwidth ){
		/* nothing changed avoid the work */
		continue ;
	    }
	    /* if window changes size need to send expose */
	    /* event so you know to redraw event - this is cause */
	    redraw = TRUE ;

	    winheight -= 2 * MENUHEIGHT ;
	    resize_windows( winwidth, winheight ) ;  
	}
    }
    if( redraw ){
	D( "TWcheckReconfig",
	    {   INT time ;
		(void) YcurTime( &time ) ;
		fprintf( stderr,"TWcheckReconfig redraw:@time = %d\n",
		time);
	    }
	) ;

	/* remove any exposure events */
	/* ????
	TWcheckExposure() ;
	TWforceRedraw() ;
	*/
    }
} /* end TWcheckReconfig */


static resize_windows( winwidth, winheight )
INT winwidth, winheight ;
{
    INT halfstep ;            /* menu half spacing */
    INT xpos ;                /* position of menu */
    INT i, j ;                /* counters */
    MENUPTR menuptr ;       /* temporary for selected menu record */

    /* change size of draw window */
    XMoveResizeWindow( dpyS, drawS, 
	0, MENUHEIGHT, winwidth, winheight ) ;

    /* create a new pixmap */
    /* off screen copy of the data */
    /* free the old one first */
    XFreePixmap( dpyS, pixmapS ) ;
    pixmapS = XCreatePixmap( dpyS, drawS, 
	(unsigned)winwidth, (unsigned)winheight, 
	XDefaultDepth(dpyS,screenS) ) ;

    /* change size of menu window */
    XMoveResizeWindow( dpyS, menuS, 1, 1, 
	winwidth, MENUHEIGHT ) ;

    /* change size of message window */
    /* winy takes care of border */
    XMoveResizeWindow( dpyS, messageS, 
	1, winheight + MENUHEIGHT, 
	winwidth, MENUHEIGHT ) ;

    /* tell main draw routines what has happened */
    TWsetDrawInfo( winheight, winwidth, pixmapS ) ;

    /* reposition the menu window titles */
    /* calculate position of the menus based on equal spacing */
    stepsizeS = winwidth / numMenuS ;
    halfstep = stepsizeS / 2 ;
    xpos = halfstep ;  /* placement of first menu */
    for( i=0;i< numMenuS; i++ ){
	menuptr = menuArrayS[i] ;
	/* save where to draw menu title in menu window */
	menuptr->xpos = halfstep - menuptr->pix_len / 2 ;

	/* fix top window */
	XMoveResizeWindow( dpyS, menuptr->top_window, 
		xpos - halfstep, 0, 
		stepsizeS, MENUHEIGHT ) ;

	/* move the subwindows */
	for( j=0; j< menuptr->numentries; j++ ){
	    XMoveResizeWindow( dpyS, menuptr->window[j], 
		xpos - menuptr->width / 2,
		(j+1) * MENUHEIGHT, 
		menuptr->width,
		MENUHEIGHT ) ;
	}
	xpos += stepsizeS ;
    }
    /* redraw menus and initialize message window */
    TWdrawMenus() ;
    TWmessage( NULL ) ;

} /* end TWcheckReconfig */


TWfreeMenuWindows()
{
    INT i, j ;              /* counters */
    MENUPTR menuptr ;       /* temporary for selected menu record */

    for( i = 0; i < numMenuS ; i++ ){
	/* pick window and menu for speed */
	menuptr = menuArrayS[i] ;
	for( j = 0 ; j < menuptr->numentries; j++ ){
	    /* would nice to be able to use XDestroySubWindows */
	    /* but we would destroy the parents copy too.  Therefore */
	    /* we must free window this way */
	    XDestroyWindow( dpyS, menuptr->window[j] ) ;
	}
    }
    /* turn off all event reporting to these windows */
    XSelectInput(dpyS,drawS,NoEventMask) ;
    XSelectInput(dpyS,backS,NoEventMask) ;
    XSelectInput(dpyS,menuS,NoEventMask) ;
    XSelectInput(dpyS,messageS,NoEventMask) ;
    /* next flush event queue by calling XSync */
    XSync( dpyS, FALSE ) ;

} /* end TWfreeMenuWindow */


TWMENUPTR TWread_menus( filename )
char *filename ;
{
    char buffer[LRECL], *bufferptr ;
    char **tokens ;         /* for parsing menu file */
    INT  numtokens ;        /* number of tokens on the line */
    INT  line ;             /* count lines in input file */
    INT  item ;             /* number of menu fields */
    FILE *fp ;              /* open file pointer */
    TWMENUPTR menu_fields;  /* array of menu information */
    TWMENUPTR mptr;         /* current menu item */

    /* parse menu file */
    line = 0 ;
    item = 0 ;
    fp = TWOPEN( filename, "r", ABORT ) ;
    while( bufferptr=fgets(buffer,LRECL,fp )){
	/* parse file */
	line ++ ; /* increment line number */
	/* skip comments */
	if( *bufferptr == COMMENT ){
	    continue ;
	}
	tokens = Ystrparser( bufferptr, ",\t\n", &numtokens );

	if( numtokens == 0 ){
	    /* skip over empty lines */
	    continue ;
	} else if( strcmp( tokens[0], MENUKEYWORD ) == STRINGEQ){
	    /* look at first field for menu keyword */
	    /* there better be only two tokens on this line */
	    if( numtokens != 2 ){
		sprintf( YmsgG, "Syntax error on line:%d\n", line ) ;
		M(ERRMSG, "TWread_menus", YmsgG ) ;
	    } 
	    /* increment menu count */
	    item++ ;
	} else { /* menu item */
	    if( numtokens == 2 || numtokens == 5 ){
		item++ ;
	    } else {
		sprintf( YmsgG, "Problem parsing line:%d in menu file\n",
		    line ) ;
		M( ERRMSG,"TWread_menus", YmsgG ) ;
	    }
	}
    }
    if( item <= 0 ){
	sprintf( YmsgG,"Couldn't find any menu data in file:%s\n",
	    filename ) ;
	M( ERRMSG,"TWread_menus", YmsgG ) ;
	return( NULL ) ;
    }

    /* now allocate array */
    menu_fields = YCALLOC( ++item, TWMENUBOX ) ;

    /* reread file building menus */
    item = -1 ;
    line = 0 ;
    rewind( fp ) ;
    while( bufferptr=fgets(buffer,LRECL,fp )){
	/* parse file */
	line++ ;
	/* skip comments */
	if( *bufferptr == COMMENT ){
	    continue ;
	}
	tokens = Ystrparser( bufferptr, ",\t\n", &numtokens );
	if( numtokens == 0 ){
	    /* skip over empty lines */
	    continue ;
	} else if( strcmp( tokens[0], MENUKEYWORD ) == STRINGEQ){
	    mptr = &( menu_fields[++item] ) ;
	    mptr->item = Ystrclone( tokens[1] ) ;
	    mptr->bool_item = NULL ;
	    mptr->bool_init = 0 ;
	    mptr->menuNotItem = TRUE ;
	    mptr->action_index = 0 ;
	    mptr->action_indexb = 0 ;
	} else if( item >=  0 ){  /* avoid obvious errors */
	    if( numtokens == 2 || numtokens == 5 ){
		mptr = &( menu_fields[++item] ) ;
		mptr->item = Ystrclone( tokens[0] ) ;
		mptr->menuNotItem = FALSE ;
		mptr->action_index = atoi( tokens[1] ) ;
		if( numtokens == 5 ){ /* also check the complement */
		    mptr->bool_item = Ystrclone( tokens[2] ) ;
		    mptr->action_indexb = atoi( tokens[3] ) ;
		    /* set initial state 1 means first entry set active */
		    mptr->bool_init = atoi( tokens[4] ) ;
		} else { /* number of tokens == 2 */
		    mptr->bool_item = NULL ;
		    mptr->bool_init = 0 ;
		    mptr->action_indexb = 0 ;
		}
	    } else {
		sprintf( YmsgG, "Problem parsing line:%d in menu file\n", 
		    line ) ;
		M( ERRMSG,"TWread_menus", YmsgG ) ;
	    }
	} else {
	    sprintf( YmsgG, "Problem parsing line:%d in menu file\n", 
		line ) ;
	    M( ERRMSG,"TWread_menus", YmsgG ) ;
	}
    } /* end parsing loop */

    TWCLOSE( fp ) ;

#ifdef DEBUG
    debug_menus( menu_fields ) ;
#endif /* DEBUG */

    return( menu_fields ) ;
	
} /* end TWread_menus */


#ifdef DEBUG

#include <ctype.h>

static char *cap_item( sptr )
char *sptr ;
{
    static char bufferL[LRECL] ;
    INT len ;                 /* string length */
    INT j ;                   /* counter */
    INT pos ;                 /* position in buffer */

    /* always capitialize the item */
    len = strlen( sptr ) ;
    pos = 0 ; /* beginning of string */
    for( j = 0 ; j < len; j++ ){
	if( sptr[j] == '.' ){
	    /* don't do anything */
	} else if( sptr[j] == ' ' ){
	    bufferL[pos++] = '_' ;
	} else if( islower(sptr[j]) ){
	    bufferL[pos++] = toupper(sptr[j]) ;
	} else {
	    /* in other cases just copy */
	    bufferL[pos++] = sptr[j] ;
	}
    }
    bufferL[pos] = EOS ;
    return( bufferL ) ;
} /* end cap_item */

static debug_menus( menu_field )
TWMENUPTR menu_field ;
{
    INT i ;                   /* counter */
    INT count ;               /* number of fields */
    char label[LRECL] ;       /* copy of the menu item */
    FILE *fp ;                /* file pointer */
    TWMENUPTR mptr ;        /* temporary pointer */

    fp = TWOPEN( "menus.h", "w", ABORT ) ;

    count = 0 ;
    for( i=0 ; menu_field[i].item ; i++ ){ 
	count++ ;
    }

    /* write out the define statements */
    fprintf( fp, "\n/* TWmenu definitions */  \n" ) ;
    fprintf( fp, "#define TWNUMMENUS\t\t%d\n", count ) ;
    for( i=0 ; menu_field[i].item; i++ ){ 
	mptr = &(menu_field[i]) ;
	if(mptr->menuNotItem){
	    continue ;
	}
	fprintf( fp, "#define %s\t\t%d\n", 
	    cap_item(mptr->item), mptr->action_index ) ;
	if( mptr->bool_item ){
	    fprintf( fp, "#define %s\t\t%d\n", 
		cap_item(mptr->bool_item),
		mptr->action_indexb ) ;
	}
    }
    fprintf( fp, "\n\n" ) ;


    fprintf( fp, "static TWMENUBOX menuS[%d] = {\n", count+1 ) ;

    for( i=0 ; menu_field[i].item; i++ ){ 
	mptr = &(menu_field[i]) ;
	fprintf( fp, "    \"%s\",", mptr->item ) ;
	if( mptr->bool_item ){
	    fprintf( fp, "\"%s\",", mptr->bool_item ) ;
	} else {
	    fprintf( fp, "0," ) ;
	}
	fprintf( fp, "%d,%d,%d,%d,\n",
	    mptr->bool_init,mptr->menuNotItem,
	    mptr->action_index, mptr->action_indexb ) ;
    }
    fprintf( fp, "    0,0,0,0,0,0\n" ) ;
    fprintf( fp, "} ;\n\n" ) ;
    TWCLOSE(fp) ;
} /* end debug_dialog() */

#endif /* DEBUG */

#endif /* no graphics */
