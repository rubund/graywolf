#include	<globals.h>
#include 	<yalecad/base.h>
#include 	<yalecad/message.h>
#include 	<yalecad/debug.h>
#ifndef lint
static char SccsId[] = "@(#) window_manager.c version 3.14 5/6/91" ;
#endif

#ifndef NOGRAPHICS

#include <menus.h>

static int         countS ;         /* the frame we are on */

window_manager()
{
    static int      count ;
    char *reply ;
    char filename[LRECL] ;
    int answer ;
    BOOL auto_draw ; /* whether to draw immediately or not */
    BOOL ok ;
    INT x, y ;

    countS = maxCountG ;

    /* read in initial data */
    read_max_xy(countS) ;
    setwindow() ;

    /* draw the data the first time */
    draw_data() ;

    /* welcome the user */
    TWmessage( "Welcome to the draw program!" ) ;

    answer = 1 ;
    auto_draw = TRUE ;
    while( answer != QUIT ){
	answer = TWcheckMouse() ;
	switch( answer ){
	    case CANCEL:
		break;
	    case REDRAW:
		draw_data() ;
		/* since data is redrawn flush exposures */
		TWcheckExposure() ;
		break;
	    case ZOOM:
		TWzoom() ;
		break;
	    case TRANSLATE:
		TWtranslate() ;
		break;
	    case FULLSCREEN:
		TWfullView() ;
		break;
	    case COLORS:
		TWtoggleColors() ;
		break;
	    case AUTO_REDRAW_ON:
		auto_draw = TRUE ;
		TWmessage( "Automatic redraw turned on" ) ;
		break;
	    case AUTO_REDRAW_OFF:
		auto_draw = FALSE ;
		TWmessage( "Automatic redraw turned off" ) ;
		break;
	    case QUIT:
		return ;
	    case TELL_POINT:
		TWmessage( "Pick a point" ) ;
		TWgetPt( &x, &y ) ;
		sprintf( YmsgG,"The point is (%d,%d)",x,y ) ;
		TWmessage( YmsgG ) ;
		break ;
	    case FORWARD_FRAME:
		/* go to the next valid frame */
		do {
		    countS++ ;
		    if( frame_exists( countS )){
			break ;
		    }
		} while (countS <= maxCountG ) ;
		if( countS <= maxCountG ){
		    read_max_xy(countS);
		    setwindow() ;
		    sprintf( YmsgG, "Frame advanced to %d",countS ) ;
		    TWmessage( YmsgG ) ;
		    TWforceRedraw() ;
		} else { 
		    sprintf( YmsgG, "End of film. Frame remains at %d",
			maxCountG ) ;
		    TWmessage( YmsgG ) ;
		    countS-- ;
		}
		break;
	    case BACKWARD_FRAME:
		do {
		    countS-- ;
		    if( frame_exists( countS )){
			break ;
		    }
		} while ( countS > 0 ) ;
		if( countS > 0 ){
		    read_max_xy(countS);
		    setwindow() ;
		    sprintf( YmsgG, "Frame rewound to %d",countS ) ;
		    TWmessage( YmsgG ) ;
		    TWforceRedraw() ;
		} else { 
		    sprintf( YmsgG, "End of film.  Frame remains at %d",
			++countS );
		    TWmessage( YmsgG ) ;
		}
		break;
	    case JUMP_TO_FRAME:
		/* get a frame from the user */
		ok = FALSE ;
		do {
		    /* get string from user */
		    reply = TWgetString( "Enter frame number:") ;
		    countS = atoi( reply ) ;
		    if( frame_exists( countS )){
			ok = TRUE ;
		    } else {
			TWmessage( "ERROR:invalid frame number!" ) ;
			(void) sleep( (unsigned) 2 ) ;
		    }
		} while ( !(ok) ) ;
		read_max_xy(countS);
		setwindow() ;
		sprintf( YmsgG, "Frame advanced to %d",countS ) ;
		TWmessage( YmsgG ) ;
		TWforceRedraw() ;
		break;
	    case DELETE_FRAME:
		/* get string from user */
		reply = TWgetString( "Do you wish to delete frame? [yes|no]") ;
		if( reply && strcmp( reply, "yes" ) == STRINGEQ ){
		    delete_files( countS ) ;
		} else {
		    break ;
		}
		/* go to the next valid frame */
		do {
		    countS++ ;
		    if( frame_exists( countS )){
			break ;
		    }
		} while (countS <= maxCountG ) ;
		if( countS <= maxCountG ){
		    read_max_xy(countS);
		    setwindow() ;
		    sprintf( YmsgG, "Frame advanced to %d",countS ) ;
		    TWmessage( YmsgG ) ;
		    TWforceRedraw() ;
		} else { 
		    sprintf( YmsgG, "End of film. Frame remains at %d",
			maxCountG ) ;
		    TWmessage( YmsgG ) ;
		    countS-- ;
		}
		break;
	    case RENUMBER_FRAMES:
		renumber_frames() ;
		break;
	    case REREAD_INPUT:
		/* find max number of frames of data */ 
		for( maxCountG=1;;maxCountG++ ){
		    if(!(frame_exists( maxCountG ))){
			maxCountG-- ;
			/* last frame successfully read is one less */
			break ;
		    }
		}
		countS = maxCountG ;
		read_max_xy(countS);
		setwindow() ;
		TWforceRedraw() ;
		break;
	    case WRITE_CIF_FILE:
		reply = TWgetString( "Enter name of cif file:" ) ;
		dump_cif( reply ) ;
		break ;
	    case DRAW_NETS:
		turnNetsOn( TRUE ) ;
		TWforceRedraw() ;
		break;
	    case IGNORE_NETS:
		turnNetsOn( FALSE ) ;
		TWforceRedraw() ;
		break;
	    case DRAW_LABELS:
		turnLabelsOn( TRUE ) ;
		TWforceRedraw() ;
		break;
	    case IGNORE_LABELS:
		turnLabelsOn( FALSE ) ;
		TWforceRedraw() ;
		break;
	}
	/* now check for expose events if automatic redraw is on */
	if( auto_draw && TWcheckExposure() ){
	    draw_data() ;
	}
    }
} /* end window_manager */


setwindow()
{
    int xspan, yspan ;
    int xbuf, ybuf ;
    int l, b, r, t ;

    xspan = maxxG - minxG ;
    yspan = maxyG - minyG ;
    /* add 10% to make look nice */
    xbuf = (int) (0.10 * (double) xspan ) ;
    ybuf = (int) (0.10 * (double) yspan ) ;
    l = minxG - xbuf ;
    b = minyG - ybuf ;
    r = maxxG + xbuf ;
    t = maxyG + ybuf ;
    D( "setwindow",
	fprintf( stderr, "window - l:%d r:%d b:%d t:%d\n", l, r, b, t )) ;
    TWsetwindow( l, b, r, t );

} /* end setwindow */

#endif /* NOGRAPHICS */
