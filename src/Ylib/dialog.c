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
FILE:	    dialog.c                                       
DESCRIPTION:creates a dialog box for TimberWolf pgms.
    It returns information for the user.
CONTENTS:   TWDRETURNPTR TWdialog( fieldp )
		TWDIALOGPTR fieldp ;
	    TWDRETURNPTR TWread_dialog( filename )
		char *filename ;
DATE:	    Aug 16, 1989 - original coding.
REVISIONS:  Sep 16, 1989 - all debug directed to stderr.
	    Sep 23, 1989 - rewrote data structures to handle cases
	    Oct  2, 1989 - now user can set position of dialog box.
		Also fixed bug with font size.
	    Feb 20, 1990 - tell the user data has changed.
	    Sep 25, 1990 - added B&W mode.
	    Dec  5, 1990 - now handle stipple correctly and added
		user_function to the dialog procedure.
	    Dec 27, 1990 - eliminated multiple redraws upon startup.
	    Mon Jan  7 18:17:44 CST 1991 - made SAFE_WAIT_TIME
		user programmable for slow machines.
	    Fri Jan 18 18:36:44 PST 1991 - added event debug.
	    Fri Jan 25 16:17:21 PST 1991 - now look for backspace
		or delete key when in window.
	    Fri Feb 22 23:36:13 EST 1991 - wait for screen to appear.
	    Thu Mar  7 01:24:55 EST 1991 - added focus requests for
		SUN openwindows.  Also added draw screen refresh.
	    Sun Nov  3 12:50:53 EST 1991 - fixed gcc complaints.
	    Thu Jan 30 02:55:16 EST 1992 - added window manager hints
		and now allow different font for dialog box.
----------------------------------------------------------------- */

#ifndef NOGRAPHICS 
#include <globals.h>
#include "info.h"

#define WHITE       1                /* white parent gc is one in array */
#define BLACK       2                /* black parent gc is two in array */
#define CANCEL      0                /* code for cancel menu activity */
#define DELETE_CH   '\177'           /* delete character */
#define BACKSPACE   '\010'           /* backspace character */
#define RETURN      '\015'           /* return charachter */
#define COMMENT     '#'              /* comment character in first col */
#define RATIO       0.70
#define ACCEPTWIN   0
#define REJECTWIN   1

/* definitions for parsing dialog file */
#define FIELD      0
#define FIELDNUM   1
#define COLUMN     2
#define ROW        3
#define STRING     4
#define LEN        5
#define TYPE       6
#define COLOR      7
#define CASEGROUP  8

/* definitions for the fonts */
#define NEW_FONT    TRUE
#define REVERT_FONT 2

TWINFOPTR    infoS ;          /* information about main details */
Display      *dpyS;           /* the display */
Window       menuS;           /* the current menu window */
Window       wS;              /* the main TW display window */
Window       dialogS;         /* the dialog display window */
GC           *contextArrayS ; /* array of context window */
GC           reverseGCS ;     /* reverse gc for dialog  */
int          screenS ;        /* the current screen */
unsigned int backgrdS ;
unsigned int foregrdS ;
int          winwidthS ;      /* window width */
Window       *winS ;          /* contains info about menus */
XFontStruct  *fontinfoS ;     /* font information */
Font         fontS ;          /* current font */
int          xdS ;            /* origin of dialog window */
int          ydS ;            /* origin of dialog window */
int          fwidthS ;        /* font width in pixels */
int          fheightS ;       /* font height in pixels */
int          numwinS ;        /* number of window in dialog box */
TWDRETURNPTR dataS ;          /* return data array */
TWDIALOGPTR  fieldS ;         /* the current dialog array */

/* function definitions */
int world2pix_x() ;
int world2pix_y() ;
int world2fonty() ;
int pixlen() ;
void set_stipple_font(BOOL stippleOn, int font_change) ;
void debug_dialog(TWDIALOGPTR fieldp) ;
void check_cases(TWDIALOGPTR fieldp, int select, void (*user_function)() );
void draw_fields(TWDIALOGPTR fieldp) ;
void TWfreeWindows() ;
void find_font_boundary() ;
void edit_field(int field, Window win, XEvent event, void (*user_function)()) ;

/* build a dialog box and get info */
TWDRETURNPTR TWdialog( TWDIALOGPTR fieldp, char *dialogname, void (*user_function)())
{
    unsigned int white, black ;
    int i ;               /* counter */
    long event_mask ;     /* set up event catching with this mask */
    int screenwidth ;     /* width of root window */
    unsigned int widthd ; /* adjusted width of dialog screen */
    int screenheight ;    /* height of root window */
    unsigned int heightd ;/* adjusted height of dialog screen */
    int win_num ;         /* window counter */
    int time ;            /* current time */
    TWDIALOGPTR fptr ;    /* current dialog field */
    TWDRETURNPTR dptr ;   /* current return field */
    BOOL bw ;             /* tells whether display is color or not */
    BOOL foundWindow ;    /* used in window search to find match */
    int lasttimeL; /* last time of exposure event */
    XEvent event ;        /* describes button event */
    Window win ;          /* temporary for selected window */
    Cursor cursor ;       /* cursor for typing */
    char *winstr ;        /* used for get user window default */
    int  m ;              /* mask for determining window position */
    char resource[LRECL] ;/* look for match in database */
    XSizeHints hints ;	  /* setup hints for window manager */
    char *font ;          /* user font request */
    BOOL font_loaded ;    /* whether dialog box has its own font */


    /* get static information from main draw module */
    /* try to do crude object oriented programming */
    infoS = TWgetDrawInfo() ;
    /* save display info for future use */
    dpyS = infoS->dpy ;
    screenS = infoS->screen ;
    wS = infoS->rootWindow ;

    black = BlackPixel(dpyS,screenS);
    white = WhitePixel(dpyS,screenS);
    backgrdS = black ;
    foregrdS = white ;

    /* check whether machine is color or not */
    if( (bw = XDisplayCells( dpyS, screenS )) > 2 ){ 
	/* if color number of display cells > 0 */
	bw = FALSE ;
    } else {
	bw = TRUE ;
    }

    /* calculate where to put master window */
    /* we want to center the window and take 70% of */ 
    /* the available screen */
    screenwidth = XDisplayWidth(dpyS,screenS);
    screenheight = XDisplayHeight(dpyS,screenS);

    sprintf( resource, "geometry_%s", dialogname ) ;
    D( "dialog", printf( "resource:%s\n", resource ) ) ;
    if( winstr = XGetDefault( dpyS, GRAPHICS, resource )){
	m = XParseGeometry( winstr,&xdS,&ydS,&widthd,&heightd) ;
	if( m & XNegative ){
	    xdS += screenwidth ;
	}
	if( m & YNegative ){
	    ydS += screenheight ;
	}
	/* these two lines insure that uses doesn't have to press */
	/* button using twm window manager */
	if( xdS == 0 ) xdS++ ;
	if( ydS == 0 ) ydS++ ;
	hints.flags = USPosition | USSize ;
    } else {
	widthd  = (int) (RATIO * (double) screenwidth ) ;
	heightd = (int) (RATIO * (double) screenheight ) ;
	xdS = (screenwidth - widthd ) / 2 ;
	ydS = (screenheight - heightd ) / 2 ;
	hints.flags = PPosition | PSize ;
    }
    sprintf( resource, "font_%s", dialogname ) ;
    D( "dialog", printf( "font resource:%s\n", resource ) ) ;
    /* set font and get font info */
    font_loaded = FALSE ;
    if(font = XGetDefault( dpyS, GRAPHICS, resource )){
	if( strcmp( font, infoS->fontname ) != STRINGEQ ){
	    fontinfoS = TWgetfont( font, &fontS ) ;
	    font_loaded = TRUE ;
	}
    } else {
	fontinfoS = infoS->fontinfo ;
	fontS = fontinfoS->fid ;
    }

    /* normal case - need to create menu window */
    dialogS = XCreateSimpleWindow( dpyS, wS, xdS, ydS, 
	widthd, heightd, 1L, backgrdS, foregrdS ) ;

    event_mask = ExposureMask | VisibilityChangeMask ;
    XSelectInput(dpyS,dialogS,event_mask);

    cursor = XCreateFontCursor( dpyS, XC_hand2 ) ;
    XDefineCursor( dpyS, dialogS, cursor ) ;

    /* set the window manager hints */
    hints.x = xdS ;
    hints.y = ydS ;
    hints.width = widthd ;
    hints.height = heightd ;
    XSetStandardProperties(dpyS,dialogS,dialogname,dialogname,None,NULL,0,&hints);

    /* set graphic contexts */
    contextArrayS = infoS->graphicContext ;
    reverseGCS = contextArrayS[WHITE] ;
    set_stipple_font( FALSE, font_loaded ) ;

    /* count number of windows required */
    fieldS = fieldp ;
    numwinS = 0 ;
    for( i=0 ; fieldp[i].row ; i++ ){ 
	numwinS++ ;
    }
    /* allocate an array of windows */
    winS = YCALLOC( numwinS+1, Window ) ;
    dataS = YCALLOC( numwinS+1,TWDRETURNBOX ) ;

    /* find pixel bounding box */
    /* we assume fixed fonts but variable fonts will be ok */
    find_font_boundary() ;

    for( i=0 ; i < numwinS ; i++ ){ 
	fptr = &(fieldp[i]) ;
	dptr = &(dataS[i]) ;
	if( fptr->string ){
	    dptr->string = Ystrclone( fptr->string ) ;
	} else {
	    dptr->string = NULL ;
	}
	if( bw ){
	    /* on a black and white machine set color to backgrd color */
	    fptr->color = BLACK ;
	}
	if( fptr->type == INPUTTYPE || fptr->type == BUTTONTYPE ){
	    winS[i] = XCreateSimpleWindow( dpyS, dialogS, 
		world2pix_x( fptr->column ),
		world2pix_y( fptr->row - 1 ),
		pixlen( fptr->len ), fheightS,
		1L, backgrdS, foregrdS ) ;
	    XDefineCursor( dpyS, winS[i], cursor ) ;

	} else if( fptr->type == CASETYPE ){
	    /* initial on button of a case switch */
	    ASSERTNCONT( fptr->group > 0 && fptr->group < numwinS,
		"TWdialog", "init_switch out of bounds\n" ) ;
	    dataS[fptr->group].bool = TRUE ;
	}
    }
    /* now raise all the subwindows */
    /* and initialize input selection*/
    for( i=0; i< numwinS; i++ ){
	if(!(winS[i])){
	    /* skip over label fields which don't have windows */
	    continue ;
	}

	/* now set all window masks */
	fptr = &(fieldp[i]) ;
	if( fptr->type == BUTTONTYPE ){
	    event_mask = ButtonPressMask | EnterWindowMask | LeaveWindowMask ;
	} else if( fptr->type == INPUTTYPE ){
	    event_mask = KeyPressMask | EnterWindowMask | LeaveWindowMask ;
	} else {
	    /* use just exposure mask here */
	    event_mask = ExposureMask ;
	}
	XSelectInput(dpyS,winS[i],event_mask);
	XMapRaised( dpyS, winS[i] ) ;
    }

    /* now raise menu window so we can see it */	
    XMapWindow( dpyS, dialogS ) ;

    /* wait for all the windows to be raised */
    XSync( dpyS, FALSE ) ;
    /* -------------------------------------------------------------
       Now wait to window to become visible.  This code is necessary 
       since some window managers (uwm) map the window as a ghost 
       image and wait for user to resize window.  Other window 
       managers (twm) map window as requested.  Need to accomodate
       both.
    -------------------------------------------------------------- */
    while( TRUE ){
	if( XCheckTypedWindowEvent(dpyS,dialogS,VisibilityNotify,&event)){
	    if( event.xvisibility.state == VisibilityUnobscured ||
		event.xvisibility.state == VisibilityPartiallyObscured ){
		break ;
	    }
	}
    }
    event_mask = ExposureMask ;
    XSelectInput(dpyS,dialogS,event_mask);
    XClearWindow( dpyS, dialogS ) ;

    draw_fields( fieldp ) ;
    /* update time for slow machines */
    YcurTime( &lasttimeL ) ;

    /* now look for button press to end */
    event_mask = ButtonPressMask | KeyPressMask | EnterWindowMask |
	LeaveWindowMask | ExposureMask ;
    while( TRUE ){
	if( XCheckMaskEvent( dpyS, event_mask, &event ) ){
	    D( "TWdialog/event",
		printf( "Event:%d window:%lu\n",
		    event.type, event.xany.window ) ;
	    ) ;
	    switch( event.type ){
	    case ButtonPress:
		/* how we exit */
		win = event.xbutton.window ;
		if( win == winS[ACCEPTWIN] ){
		    TWfreeWindows() ;
		    set_stipple_font( TRUE, REVERT_FONT ) ;
		    return( dataS ) ;
		} else if( win == winS[REJECTWIN] ){
		    TWfreeWindows() ;
		    set_stipple_font( TRUE, REVERT_FONT ) ;
		    return( NULL ) ;
		} else {
		    /* BUTTON TYPE which is a case switch */
		    foundWindow = FALSE ;
		    for( i = 0; i < numwinS; i++ ){
			if( win == winS[i] ){
			    foundWindow = TRUE ;
			    break ;
			}
		    } /* end search for window */
		    if( foundWindow ){
			check_cases( fieldp, i, user_function ) ;
		    }
		}
		break ;
	    case EnterNotify:
		/* light up window */
		/* find window match */
		win = event.xcrossing.window ;
		foundWindow = FALSE ;
		for( i = 0; i < numwinS; i++ ){
		    if( win == winS[i] ){
			foundWindow = TRUE ;
			break ;
		    }
		} /* end search for window */

		if( foundWindow ){ /* a match light up window */
		    fptr = &(fieldp[i]) ;
		    dptr = &(dataS[i]) ;
		    XFillRectangle( dpyS,win, contextArrayS[fptr->color],
			0,0,
			pixlen( fptr->len ), fheightS ) ;
		    if( dptr->string ){
			XDrawString( dpyS, win, reverseGCS, 
			    0L, world2fonty( 0L ),
			    dptr->string, strlen(dptr->string)) ;
		    }
		    XSetInputFocus( dpyS, win, 
			RevertToPointerRoot, CurrentTime ) ;
		}
		break ;
		    
	    case LeaveNotify:
		/* turn off window */
		/* find window match */
		win = event.xcrossing.window ;
		foundWindow = FALSE ;
		for( i = 0; i < numwinS; i++ ){
		    if( win == winS[i] ){
			foundWindow = TRUE ;
			break ;
		    }
		} /* end search for window */

		if( foundWindow ){ /* turn off window */
		    dptr = &(dataS[i]) ;
		    XClearWindow( dpyS, win ) ;
		    if( dptr->string ){
			if( fieldp[i].type != BUTTONTYPE ){
			    /* normal case */
			    XDrawString( dpyS, winS[i], 
				contextArrayS[fptr->color], 
				0L, world2fonty( 0L ),
				dptr->string, 
				strlen(dptr->string)) ;
			} else if( dptr->bool ){
			    /* a case switch that is on */
			    XFillRectangle( dpyS,win, 
				contextArrayS[fptr->color],
				0,0,
				pixlen( fptr->len ), fheightS ) ;
			    XDrawString( dpyS, win, reverseGCS, 
				0L, world2fonty( 0L ),
				dptr->string, strlen(dptr->string)) ;
			} else {
			    /* a case switch that is off */
			    XDrawString( dpyS, winS[i], 
				contextArrayS[fptr->color], 
				0L, world2fonty( 0L ),
				dptr->string, 
				strlen(dptr->string)) ;
			}
		    }
		    XSetInputFocus( dpyS, PointerRoot, 
			RevertToParent, CurrentTime );
		}
		break ;
	    case KeyPress:
		win = event.xkey.window ;
		foundWindow = FALSE ;
		for( i = 0; i < numwinS; i++ ){
		    if( win == winS[i] ){
			foundWindow = TRUE ;
			break ;
		    }
		} /* end search for window */

		if( foundWindow ){ /* turn off window */
		    edit_field( i, win, event, user_function ) ;
		}
		break ;
	    case Expose:
		win = event.xexpose.window ;
		if( win == infoS->drawWindow ){
		    (*infoS->refresh_func)() ;
		} else {
		    /* window managers sometimes send us too many */
		    /* exposure events therefore check time and */
		    /* make multiple exposures invalid */
		    if( event.xexpose.count == 0 ){
			(void) YcurTime( &time ) ;
			if( time - lasttimeL < TWsafe_wait_timeG ){
			    D( "TWdialog/exposure", 
				printf(
				    "Dialog Exposure:0 @time = %d\n",
				    time);
			    ) ;
			    break ;
			}
			D( "TWdialog/exposure", 
			    printf(
				"Dialog Exposure:1 @time = %d\n",
				time);
			) ;
			draw_fields( fieldp ) ;
			/* update time for slow machines */
			(void) YcurTime( &time ) ;
			lasttimeL = time ;
		    }
		}
		break ;
	    } /* end event switch */
	} /* end check on event */
    } /* end wait loop */

} /* end TWdialog */

void set_stipple_font(BOOL stippleOn , int font_change)
{
    int i ;        /* counter */

    if( infoS->stipple ){
	if( stippleOn ){
	    for( i = 3; i <= infoS->numColors; i++ ){
		XSetFillStyle( dpyS, contextArrayS[i], FillTiled ) ;
	    }
	} else {
	    for( i = 3; i <= infoS->numColors; i++ ){
		XSetFillStyle( dpyS, contextArrayS[i], FillSolid ) ;
	    }
	}
    }
    if( font_change ){
	if( font_change == NEW_FONT ){
	    for( i=0; i <= infoS->numColors; i++ ){
		XSetFont( dpyS, contextArrayS[i], fontS ) ;
	    }
	} else if( font_change = REVERT_FONT ){
	    for( i=0; i <= infoS->numColors; i++ ){
		XSetFont( dpyS, contextArrayS[i], infoS->fontinfo->fid ) ;
	    }
	}
    }
} /* end set_stipple_font */

/* check the case fields and set all member of group to false */
void check_cases( TWDIALOGPTR fieldp, int select, void (*user_function)() )
{
    int i ;               /* counter */
    int group ;           /* case group */
    TWDIALOGPTR fptr ;    /* current dialog field */
    TWDRETURNPTR dptr ;   /* current return field */

    group = fieldp[select].group ;
    for( i=0 ; i < numwinS ; i++ ){ 
	fptr = &(fieldp[i]) ;
	dptr = &(dataS[i]) ;
	if( fptr->group == group ){
	    if( i == select ){
		dptr->bool = TRUE ;
	    } else {
		dptr->bool = FALSE ;
	    }
	}
    }
    if( user_function ){
	(*user_function)( dataS, select ) ;
    }
    /* redraw fields so user can see change */
    draw_fields( fieldp ) ;

    XFlush( dpyS ) ;

} /* end check_cases */

void draw_fields(TWDIALOGPTR fieldp)
{
    int i ;               /* counter */
    TWDIALOGPTR fptr ;    /* current dialog field */
    TWDRETURNPTR dptr ;   /* current return field */

    /* now draw all the strings to dialog window */
    XClearWindow( dpyS, dialogS ) ;
    for( i=0 ; i < numwinS ; i++ ){ 
	fptr = &(fieldp[i]) ;
	dptr = &(dataS[i]) ;
	if(!(dptr->string)){
	    /* avoid null strings */
	    continue ;
	}
	if( fptr->type == LABELTYPE || fptr->type == CASETYPE ){
	    XDrawString( dpyS, dialogS, contextArrayS[fptr->color], 
		world2pix_x( fptr->column ), 
		world2fonty( fptr->row - 1 ),
		dptr->string, strlen(dptr->string)) ;
	} else if( fptr->type == INPUTTYPE ){
	    /* input case */
	    XClearWindow( dpyS, winS[i] ) ;
	    XDrawString( dpyS, winS[i], contextArrayS[fptr->color], 
		0L, world2fonty( 0L ),
		dptr->string, strlen(dptr->string)) ;
	} else if( fptr->type == BUTTONTYPE ){
	    XClearWindow( dpyS, winS[i] ) ;
	    if( dptr->bool ){
		/* true initially */
		XFillRectangle( dpyS,winS[i], contextArrayS[fptr->color],
		    0,0,
		    pixlen( fptr->len ), fheightS ) ;
		XDrawString( dpyS, winS[i], reverseGCS, 
		    0L, world2fonty( 0L ),
		    dptr->string, strlen(dptr->string)) ;
	    } else { /* off */
		XDrawString( dpyS, winS[i], contextArrayS[fptr->color], 
		    0L, world2fonty( 0L ),
		    dptr->string, strlen(dptr->string)) ;
	    }
	}
    }
} /* end draw_fields */


void TWfreeWindows()
{
    int i, j ;              /* counters */

    for( i = 0; winS[i] ; i++ ){
	/* we must free window this way */
	XDestroyWindow( dpyS, winS[i] ) ;
    }
    YFREE( winS ) ;
    XDestroyWindow( dpyS, dialogS ) ;

} /* end TWfreeWindows */

void find_font_boundary() 
{
    fwidthS = fontinfoS->max_bounds.rbearing - 
	fontinfoS->min_bounds.lbearing ;

    fheightS = fontinfoS->ascent + fontinfoS->descent ;

} /* end find_font_boundary */

/* tranforms the world coordinate character column format */
/* to pixel coordinates */
int world2pix_x( int x )
{
    return( x * fwidthS ) ;
} /* end world2pix_x */

int world2pix_y( int y )
{
    return( y * fheightS ) ;
} /* end world2pix_y */

int world2fonty( int y )
{
    return( (++y) * fheightS - fontinfoS->max_bounds.descent ) ;
} /* end world2pix_y */
	
/* change length of string to pixel length */
int pixlen( int length )
{
    return( fwidthS * length ) ;
} /* end pixlen */

void edit_field(int field, Window win, XEvent event, void (*user_function)() ) /* describes the button event */
{
    TWDIALOGPTR fptr;    /* current field of dialog */
    TWDRETURNPTR dptr;   /* return field of dialog */
    BOOL press ;            /* tells whether keyboard has been pushed */
    BOOL finish ;           /* tells whether we have received a return */
    char buffer[LRECL] ;    /* used for keyboard translation */
    char data[LRECL];       /* current value of users input */
    KeySym keysym ;         /* return of keysym from user */
    XComposeStatus status ; /* where a compose key was pressed */
    int strwidth ;          /* width of string in pixels */
    int dataCount ;         /* number of characters in user input */

    fptr = &( fieldS[field] ) ;
    dptr = &( dataS[field] ) ;

    if( dptr->string ){
	/* initialize string buffers */
	dataCount = strlen( dptr->string ) ;
	strcpy( data, dptr->string ) ;
	/* now move pointer to end of current data */
	strwidth = XTextWidth( fontinfoS, dptr->string, dataCount) ;
	/* now warp pointer to message window */
	XWarpPointer( dpyS, None, win, 0, 0, 0, 0, 
	    strwidth + fwidthS/3, fheightS/2 ) ;
    } else {
	/* initialize string buffers */
	data[0] = EOS ;
	dataCount = 0 ;
    }

    /* now look for keyboard action */
    finish = FALSE ;
    press = TRUE ; /* initially we got a keyboard event */
    /* tell user data has changed */
    dptr->bool = TRUE ;
    do {
	if( press ){
	    /* initialize buffer */
	    buffer[0] = EOS ;
	    /* find what the user entered */
	    XLookupString( &(event.xkey), buffer,LRECL,&keysym, &status );
	    buffer[1] = EOS ; /* only get one character at a time */
	    D( "Yedit_field",printf( "char:%c\n", buffer[0] ) ) ;

	    /* look to see if we got a return */
	    if( buffer[0] == RETURN ){
		finish = TRUE ;
		if( dptr->string ){
		    YFREE( dptr->string ) ;
		}
		/* return answer in dptr field */
		dptr->string = Ystrclone( data ) ;
	    } else {
		/* look for more data */
		/* but first copy the data we have */
		if( buffer[0] == BACKSPACE || buffer[0] == DELETE_CH ){ 
		    /* look for backspace or delete  */
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
		XFillRectangle( dpyS,win, contextArrayS[fptr->color],
		    0,0,
		    pixlen( fptr->len ), fheightS ) ;
		XDrawString( dpyS, win, reverseGCS, 
		    0L, world2fonty( 0L ),
		    data, dataCount) ;
		D( "Yedit_field",printf( "data:%s\n", data ) ) ;
		D( "Yedit_field",printf("datacount:%d\n",
		    dataCount ) ) ;
		/* now move pointer to end of current data */
		strwidth = XTextWidth( fontinfoS, data,dataCount) ;
		/* now warp pointer to message window */
		XWarpPointer( dpyS, None, win, 0, 0, 0, 0, 
		    strwidth + fwidthS/3, fheightS/2 ) ;
		
	    }
	}
	/* get next keyboard stroke */
	press = XCheckTypedWindowEvent( dpyS,win,KeyPress,&event );

    } while(!finish) ; /* end wait loop */

    if( user_function ){
	(*user_function)( dataS, field ) ;
	/* redraw fields so user can see change */
	draw_fields( fieldS ) ;
    }


} /* end edit_field */

#ifdef DEBUG

TWDIALOGPTR TWread_dialog( filename )
char *filename ;
{

#define KEYWORD       "numfields"
#define FIELDKEYWORD  "field"

    FILE *fp ;
    char buffer[LRECL], *bufferptr ;
    char group[LRECL] ;  /* for parsing case groups */
    char **tokens ;      /* for parsing menu file */
    char **colors ;      /* the standard color array */
    int  numtokens ;     /* number of tokens on line */
    int  group_num ;     /* number of case fields given */
    int  i ;             /* counter */
    int  line ;          /* line number of TWmenu file */
    int  length ;        /* length of string */
    int  numfields ;     /* number of dialog fields */
    int  curfield ;      /* current dialog field */
    int  case_label ;    /* current case field */
    int  numcolors ;     /* the number of colors in color array */
    BOOL found ;         /* whether color was found */
    TWDIALOGPTR fields;  /* dialog array information */
    TWDIALOGPTR fptr;    /* current field of dialog */

    /* get colors for dialog window */
    infoS = TWgetDrawInfo() ;
    colors = infoS->colors ;
    numcolors = infoS->numColors ;

    /* parse dialog file */
    line = 0 ;
    curfield = -1 ;
    group_num = 0 ;
    fields = NULL ;
    fp = TWOPEN( filename, "r", ABORT ) ;
    while( bufferptr=fgets(buffer,LRECL,fp )){
	/* parse file */
	line++ ; /* increment line number */
	/* skip comments */
	if( *bufferptr == COMMENT ){
	    continue ;
	}
	tokens = Ystrparser( bufferptr, ";\t\n", &numtokens );

	if( numtokens == 0 ){
	    /* skip over empty lines */
	    continue ;
	} else if( strcmp( tokens[0], KEYWORD ) == STRINGEQ){
	    /* look at first field for menu keyword */
	    
	    /* there better be only two tokens on this line */
	    if( numtokens != 2 ){
		sprintf( YmsgG, "Syntax error on line:%d\n", line ) ;
		M(ERRMSG, "TWread_dialog", YmsgG ) ;
		break ;
	    } 
	    numfields = atoi( tokens[1] ) ;
	    ASSERTNBREAK( numfields > 0,"TWread_dialog","numfields must be >0\n" ) ;
	    fields = YCALLOC( numfields+1, TWDIALOGBOX ) ;

	} else if( strcmp( tokens[FIELD], FIELDKEYWORD ) == STRINGEQ){
	    if( ++curfield >= numfields ){
		M( ERRMSG,"TWread_dialog","number of fields mismatch\n" ) ;
		return( NULL ) ;
	    } 
	    fptr = &(fields[curfield]) ;
	    if( numtokens != 8 && numtokens != 9 ){
		sprintf( YmsgG, "Problem parsing line:%d in dialog file\n",
		    line ) ;
		M( ERRMSG,"TWread_dialog", YmsgG ) ;
		continue ;
		
	    }
	    fptr->column = atoi( tokens[COLUMN] ) ;
	    fptr->row = atoi( tokens[ROW] ) ;
	    fptr->len = atoi( tokens[LEN] ) ;
	    /* check color to make sure it is valid */
	    found = FALSE ;
	    for( i=1; i <= numcolors; i++ ){
		if( strcmp( tokens[COLOR], colors[i] ) == STRINGEQ ){
		    fptr->color = i ;
		    found = TRUE ;
		    break ;
		}
	    }
	    if(!(found)){
		sprintf( YmsgG,
		    "Color:%s has not been allocated\n",
		    tokens[COLOR] ) ;
		M( ERRMSG, "TWread_dialog", YmsgG ) ;
		return( NULL ) ;

	    }
	    if( strcmp( tokens[STRING], "NULL") == STRINGEQ ){
		fptr->string = NULL ;
	    } else {
		fptr->string = Ystrclone( tokens[STRING] ) ;
	    }

	    /* look at types */
	    if( strcmp( tokens[TYPE], "input") == STRINGEQ ){
		fptr->type = INPUTTYPE ;
	    } else if( strcmp( tokens[TYPE], "label") == STRINGEQ ){
		fptr->type = LABELTYPE ;
	    } else if( strcmp( tokens[TYPE], "accept") == STRINGEQ ){
		fptr->type = BUTTONTYPE ;
		if( curfield != 0 ){
		    M( ERRMSG, "TWread_dialog",
			"accept must be 1st field\n" ) ;
		    return( NULL ) ;
		}
	    } else if( strcmp( tokens[TYPE], "reject") == STRINGEQ ){
		fptr->type = BUTTONTYPE ;
		if( curfield != 1 ){
		    M( ERRMSG, "TWread_dialog",
			"reject must be 2nd field\n" ) ;
		    return( NULL ) ;
		}
	    } else if( strcmp( tokens[TYPE], "clabel") == STRINGEQ ){
		fptr->type = BUTTONTYPE ;
	    } else if( strcmp( tokens[TYPE], "case") == STRINGEQ ){
		fptr->type = CASETYPE ;
		group_num++ ;
		if( numtokens == 9 ){
		    strcpy( group, tokens[CASEGROUP] ) ;
		    tokens = Ystrparser( group, ",\t\n", &numtokens );
		    ASSERTNRETURN( numtokens>0,"TWread_dialog",
			"no cases found\n" ) ;
		    for( i = numtokens-1; i >= 0;i-- ){
			case_label = atoi( tokens[i] ) ;
			ASSERTNCONT( case_label > 0 && case_label <=numfields,
			    "TWdialog", "case_label out of bounds\n" ) ;
			fields[case_label].group = group_num ;
		    }
		    /* first one in list is default on switch */
		    fptr->group = case_label ;
		} else {
		    M( ERRMSG, "TWread_dialog",
			"wrong number of tokens for case type field\n" );
		    return( NULL ) ;
		}
	    } else {
		sprintf( YmsgG, "Problem parsing line:%d in dialog file\n",
		    line ) ;
		M( ERRMSG,"TWread_dialog", YmsgG ) ;
	    }
	} 
    } /* end parsing loop */

    TWCLOSE( fp ) ;

    if( fields[0].type != BUTTONTYPE ||
        fields[1].type != BUTTONTYPE ){
	M( ERRMSG, "TWread_dialog",
	    "accept and reject fields not setup correctly\n" ) ;
	return( NULL ) ;
    }

    if( fields ){
	debug_dialog( fields ) ;
	return( fields ) ;
    } else {
	return( NULL ) ;
    }

} /* end TWread_dialog */

static debug_dialog( fieldp )
TWDIALOGPTR fieldp ;
{
    int i ;                   /* counter */
    int count ;               /* number of fields */
    FILE *fp ;                /* file pointer */
    TWDIALOGPTR fptr ;        /* temporary pointer */

    fp = TWOPEN( "dialog.h", "w", ABORT ) ;

    count = 0 ;
    for( i=0 ; fieldp[i].row ; i++ ){ 
	count++ ;
    }

    fprintf( fp, "static TWDIALOGBOX dialogS[%d] = {\n", count+1 ) ;

    for( i=0 ; fieldp[i].row; i++ ){ 
	fptr = &(fieldp[i]) ;
	fprintf( fp, "    %d,%d,%d,",fptr->row,fptr->column,fptr->len ) ;
	fprintf( fp, "\"%s\",%d,%d,%d,\n", fptr->string,fptr->type,
	    fptr->color, fptr->group ) ;
    }
    fprintf( fp, "    0,0,0,0,0,0,0\n" ) ;
    fprintf( fp, "} ;\n\n" ) ;
    TWCLOSE(fp) ;
} /* end debug_dialog() */

#endif /* DEBUG */

#endif /* no graphics */
