/* ----------------------------------------------------------------- 
FILE:	    graphics.c                                       
DESCRIPTION:graphics routines
CONTENTS:   
DATE:	    
REVISIONS: 
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) graphics.c (Yale) version 1.2 9/23/91" ;
#endif


#include <yalecad/debug.h>
#include <yalecad/message.h>
#include <yalecad/colors.h>
#include <yalecad/draw.h>
#include <yalecad/stat.h>
#include <yalecad/system.h>
#include <globals.h>

#ifndef NOGRAPHICS


#define MENUP        "xxx_menu"
#define INTRO        "Welcome to XXX"
#define DEFAULTBINS  20
#define ALLPINS      0

/* #define DEVELOPMENU */
/* During development use TWread_menus in place of menuS */
/* to create menu record, ie.  TWread_menus(MENUP) */
#ifdef DEVELOPMENU
#define MENUREAD()   menuS = TWread_menus(MENUP) ;
#else
#define MENUREAD()
#endif

#include <menus.h>


/* ***************************************************************** */
static BOOL auto_drawS = TRUE ;/* whether to draw immediately after exp.*/
static INT  *bin_arrayS = NIL(INT) ;
static INT  binsizeS ;
static INT  numbinS =  DEFAULTBINS ;
static INT  numpinS = ALLPINS ;
static INT  square_itS ;
static INT  max_countS ;
static INT  max_lengthS ;
static INT  expandS ;
static DOUBLE meanS ;
static DOUBLE varS ;

/* Forward declaration */
INT draw_the_data() ;

init_graphics( argc, argv, windowId )
INT argc ;
char *argv[] ;
INT windowId ;
{
    char *host ;

    /* we need to find host for display */
    if(!(host = Ygetenv("DISPLAY"))) {
	M(WARNMSG,"initGraphics","Can't get environment variable ");
	M(MSG,NULL, "for display.  Aborting graphics...\n\n" ) ;
	return ;
    }

    MENUREAD() ;
    if( windowId ){
	/* init windows as a parasite */
	if(!(TWinitParasite(argc,argv,TWnumcolors(),TWstdcolors(),
	    FALSE,menuS, draw_the_data, windowId ))){
	    M(ERRMSG,"initGraphics","Aborting graphics.");
	    return ;
	}
    } else {
	/* init window as a master */
	if(!(TWinitGraphics(argc,argv,TWnumcolors(),TWstdcolors(),FALSE,
	    menuS,draw_the_data ))){
	    M(ERRMSG,"initGraphics","Aborting graphics.");
	    return ;
	}
    }
    TWsetwindow( 0, 0, 10, 10 ) ;
    TWdrawCell( 0, 0,0,11,11, TWBLUE, INTRO ) ;
    set_window() ;
    TWdrawMenus() ;
    TWflushFrame() ;
    /* use TWinterupt to turn off window enter/leave lights */
    TWinterupt() ;

} /* end initGraphics */

/* heart of the graphic syskem processes user input */
process_graphics()
{

    INT x, y ;           /* coordinates from pointer */
    INT selection ;     /* the users pick */
    char *reply ;       /* user reply to a querry */
    BOOL ok ;           /* loop until this value is true */

    /* data might have changed so show user current config */
    /* any function other that the draw controls need to worry about */
    /* this concurrency problem -  show user current config */
    process_nets() ;
    draw_the_data() ;
    /* use TWcheckExposure to flush exposure events since we just */
    /* drew the data */
    TWcheckExposure() ;
    TWmessage( "Analyze nets is waiting for your response..." ) ;

    selection  = CANCEL ;
    while( selection != CONTINUE_PGM ){ /* loop until exit */
	selection = TWcheckMouse() ;
	switch( selection ){
	case CANCEL:
	    /* do nothing */
	    break ;
	case AUTO_REDRAW_ON:
	    auto_drawS = TRUE ;
	    break ;
	case AUTO_REDRAW_OFF:
	    auto_drawS = FALSE ;
	    break ;
	case COLORS:
	    TWtoggleColors() ;
	    break ;
	case CONTINUE_PGM:
	    break ;
	case DUMP_GRAPHICS:
	    graphics_dump() ;
	    break ;
	case FULLVIEW:
	    TWfullView() ;
	    break ;
	case CLOSE_GRAPHICS:
	    closegraphics() ;
	    return ;
	case REDRAW:
	    draw_the_data() ;
	    /* use TWcheckExposure to flush exposure events since */
	    /* we just drew the data */
	    TWcheckExposure() ;
	    break ;
	case TELL_POINT:
	    TWmessage( "Pick a point" ) ;
	    TWgetPt( &x, &y ) ;
	    sprintf( YmsgG,"The point is (%d,%d)",x, 
		ROUND( (DOUBLE) y / (DOUBLE) square_itS) ) ;
	    TWmessage( YmsgG ) ;
	    break ;
	case TRANSLATE:
	    TWtranslate() ;
	    break ;
	case ZOOM:
	    TWzoom() ;
	    break ;
	case NUMBINS:
	    ok = FALSE ;
	    do {
		/* get string from user */
		reply = TWgetString( "Enter number of bins:") ;
		numbinS = atoi( reply ) ;
		if( numbinS > 0 && numbinS < numnetsG ){
		    ok = TRUE ;
		    TWforceRedraw() ;
		} else {
		    TWmessage( "ERROR:invalid number of bins!" ) ;
		    (void) sleep( (unsigned) 2 ) ;
		}
	    } while ( !(ok) ) ;

	    process_nets() ;
	    break ;
	case NUMPINS:
	    ok = FALSE ;
	    do {
		/* get string from user */
		reply = TWgetString( "Enter number of pins:") ;
		numpinS = atoi( reply ) ;
		if( numpinS >= 0 ){
		    ok = TRUE ;
		    TWforceRedraw() ;
		} else {
		    TWmessage( "ERROR:invalid number of pins!" ) ;
		    (void) sleep( (unsigned) 2 ) ;
		}
	    } while ( !(ok) ) ;
	    process_nets() ;
	    break ;

	} /*********************** end graphics SWITCH *****************/
	if( auto_drawS && TWcheckExposure() ){
	    draw_the_data() ;
	}

    } 
    TWmessage("Continuing - to interupt program click on top menu window") ;
    /* use TWinterupt to turn off window enter/leave lights */
    TWinterupt() ;

} /* end process_graphics */


/* the graphics program can draw the results at each desired */
/* timestep. */
INT draw_the_data()
{

    INT i, x0, x1, y0, y1 ;
    INT color ;
    INT exp, tick ;
    DOUBLE num, rem, numticks, sigma ;
    char label[LRECL] ;

    if( !(graphicsG) ){
	return ;
    }
    TWstartFrame() ;
    TWmessage( "Drawing the data...Please wait" ) ;

    x0 = 0 ;
    x1 = 0 ;
    y0 = 0 ;
    y1 = 0 ;
    color = TWGREEN ;
    for( i = 0; i < numbinS; i++ ){
	x1 += binsizeS ;
	y1 = bin_arrayS[i] * square_itS ;
	TWdrawCell(i,x0,y0,x1,y1,color,NULL) ;
	x0 = x1 ;
    }
    /* draw axis */
    x1 = max_lengthS + expandS ;
    y1 = square_itS * max_countS + expandS ;
    TWdrawLine(i,0,0,x1,0,TWBLACK,NULL) ;
    TWdrawLine(i,0,0,0,y1,TWBLACK,NULL) ;

    /* now calculate the horizontal tick marks */
    exp = (INT) log10( (DOUBLE) max_lengthS ) ;
    num = pow( 10.0, (DOUBLE) exp ) ;
    rem = (DOUBLE) max_lengthS / num ;
    if( rem < 3.0 ){
	num /= 10.0 ;
    }
    numticks = (DOUBLE) max_lengthS / num ;
    for( i = 0; i <= (INT) numticks; i++ ){
	tick = (INT) ( (DOUBLE) i * num ) ;
	TWdrawLine(i, tick, 0, tick, -expandS / 2, TWBLACK, NULL ) ; 
	sprintf( label, "%d", tick ) ;
	TWdrawString( tick, -expandS, TWBLACK, label ) ;
    }

    /* now calculate the vertical tick marks */
    exp = (INT) log10( (DOUBLE) max_countS ) ;
    num = pow( 10.0, (DOUBLE) exp ) ;
    rem = (DOUBLE) max_countS / num ;
    if( rem < 3.0 ){
	num /= 10.0 ;
    }
    numticks = (DOUBLE) max_countS / num ;
    for( i = 0; i <= (INT) numticks; i++ ){
	tick = (INT) ( (DOUBLE) i * (DOUBLE) square_itS * num ) ;
	TWdrawLine(i, -expandS / 2, tick, 0, tick, TWBLACK, NULL ) ; 
	rem = (DOUBLE) i * num ;
	if( rem >= 1.0 ){
	    sprintf( label, "%d", (INT) rem ) ;
	    TWdrawString( -expandS, tick, TWBLACK, label ) ;
	}
    }

    /* draw the mean and variance */
    x1 = (INT) meanS ;
    y1 = square_itS * max_countS ;
    TWdrawLine(i,x1,0,x1,y1,TWRED, "mean") ;
    sigma = sqrt( varS ) ;
    x1 = (INT) ( meanS + sigma ) ;
    TWdrawLine(i,x1,0,x1,y1,TWBLUE, "+sigma") ;
    x1 = (INT) ( meanS - sigma ) ;
    if( x1 > 0 ){
	TWdrawLine(i,x1,0,x1,y1,TWBLUE, "-sigma") ;
    }


    /* clear wait message and FLUSH OUTPUT BUFFER */
    TWmessage( NULL ) ;
    TWflushFrame() ;

} /* end draw_the_data */

set_window()
{
    /* this function fits the data to the window */
}


/* dumps the data to a file for future study */
graphics_dump() 
{
    /* now change mode to dump to file */
    TWsetMode(1) ;
    /* dump the data to a file now instead of screen */
    draw_the_data() ;
    /* restore the state to previous condition and set draw to screen */
    TWsetMode(0) ;
}



process_nets()
{

    INT i ; /* counter */
    INT count ;
    INT expand ;
    DOUBLE temp ;

    if( bin_arrayS ){
	Ysafe_free( bin_arrayS ) ;
    }

    count = 0 ;
    for( i = 0; i < numnetsG; i++ ){
	if( numpinS == 0 || net_numpinsG[i] == numpinS ){
	    lengthG[count++] = net_lengthG[i] ;
	}
    }

    max_lengthS = (INT) Ystat_max( lengthG, count, sizeof(INT) ) ;
    if( max_lengthS == 0 ){
	return( FALSE ) ;
    }
    meanS = Ystat_mean( lengthG, count, sizeof(INT) ) ;
    varS = Ystat_var( lengthG, count, sizeof(INT), meanS ) ;

    if( numbinS > numnetsG ){
	numbinS = numnetsG ;
    }

    binsizeS = max_lengthS / numbinS ;
    binsizeS++ ;
    bin_arrayS = (INT *) Ysafe_calloc( numbinS+1, sizeof(INT) ) ;

    /* now compile the statistics */
    for( i = 0; i < numnetsG; i++ ){
	if( numpinS == 0 || net_numpinsG[i] == numpinS ){
	    bin_arrayS[ net_lengthG[i] / binsizeS ] ++ ;
	}
    }

    max_countS = (INT) Ystat_max( bin_arrayS, numbinS, sizeof(INT) ) ;
    expandS = ROUND( 0.05 * (DOUBLE) max_lengthS ) ;
    expand = 2 * expandS ;
    /* try to make display square */
    if( max_lengthS > max_countS ){
	square_itS = ROUND( (DOUBLE) max_lengthS / (DOUBLE) max_countS ) ;
	if( square_itS < 1 ) square_itS = 1 ;
	TWsetwindow( -expand, -expand, max_lengthS + expand, 
	    square_itS * max_countS + expand ) ;

    } else {
	square_itS = 1 ;
	TWsetwindow( -expand, -expand, max_lengthS + expand, 
	    max_countS + expand ) ;
    }
}
#endif /* NOGRAPHICS */


/* close graphics window on fault */
closegraphics( )
{
    G( TWcloseGraphics() ) ;
} /* end closegraphics */
