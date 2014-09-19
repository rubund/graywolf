/* ----------------------------------------------------------------- 
FILE:	    relpos.h                                      
DESCRIPTION:macro definition for relative position to global position.
CONTENTS:   REL_POS( orient_R, globalY_R, globalX_R,
		relativeX_R, relativeY_R, cellX_R, cellY_R ) ;
	    INT orient_R, globalY_R, globalX_R, relativeX_R, 
		relativeY_R, cellX_R, cellY_R ;
DATE:	    Jun 30, 1988 
REVISIONS:  Aug 13, 1990 - moved to library for MC.
	    Thu Feb  7 00:04:51 EST 1991 - now correctly handle
		the truncated center regardless of orientation 
		with the addtion of the REL_POST MACROs.
----------------------------------------------------------------- */
/* *****************************************************************
   static char SccsId[] = "@(#) relpos.h version 1.5 3/16/92" ;
***************************************************************** */

/* ----------------------------------------------------------------- 
    The following is a macro which calculates the global pin position
    given an orientation, a cell center ( global position ) and the
    pin position relative to the cell center.  The macro was designed
    for speed.  For test purposes, the macro can be short circuited,
    and instead call a the rel_pos function - defined in output.c
    The _R is an attempt to avoid variable substition problems.
----------------------------------------------------------------- */
#ifndef RELPOS_H
#define RELPOS_H

#ifndef YTRANS_DEFS
#define EXTERN extern

#else
#define EXTERN
#endif

#ifndef RELPOSFUNC

#define REL_POS( orient_R,globX_R,globY_R,relX_R,relY_R,cellX_R,cellY_R )\
{ switch( orient_R ){                             \
	case 0:                                   \
	    globX_R = cellX_R + relX_R ;          \
	    globY_R = cellY_R + relY_R ;          \
	    break ;                               \
	case 1:                                   \
	    globX_R = cellX_R + relX_R ;          \
	    globY_R = cellY_R - relY_R ;          \
	    break ;                               \
	case 2:                                   \
	    globX_R = cellX_R - relX_R ;          \
	    globY_R = cellY_R + relY_R ;          \
	    break ;                               \
	case 3:                                   \
	    globX_R = cellX_R - relX_R ;          \
	    globY_R = cellY_R - relY_R ;          \
	    break ;                               \
	case 4:                                   \
	    globX_R = cellX_R - relY_R ;          \
	    globY_R = cellY_R - relX_R ;          \
	    break ;                               \
	case 5:                                   \
	    globX_R = cellX_R + relY_R ;          \
	    globY_R = cellY_R + relX_R ;          \
	    break ;                               \
	case 6:                                   \
	    globX_R = cellX_R - relY_R ;          \
	    globY_R = cellY_R + relX_R ;          \
	    break ;                               \
	case 7:                                   \
	    globX_R = cellX_R + relY_R ;          \
	    globY_R = cellY_R - relX_R ;          \
	    break ;                               \
	}                                         \
}

/* this macro performs centering in addition */
#define REL_POST( orient_R,globX_R,globY_R,relX_R,relY_R,cellX_R,cellY_R )\
{ switch( orient_R ){                             \
	case 0:                                   \
	    globX_R = cellX_R + relX_R ;          \
	    globY_R = cellY_R + relY_R ;          \
	    break ;                               \
	case 1:                                   \
	    globX_R = cellX_R + relX_R ;          \
	    globY_R = cellY_R - relY_R ;          \
	    break ;                               \
	case 2:                                   \
	    globX_R = cellX_R - relX_R ;          \
	    globY_R = cellY_R + relY_R ;          \
	    break ;                               \
	case 3:                                   \
	    globX_R = cellX_R - relX_R ;          \
	    globY_R = cellY_R - relY_R ;          \
	    break ;                               \
	case 4:                                   \
	    globX_R = cellX_R - relY_R ;          \
	    globY_R = cellY_R - relX_R ;          \
	    break ;                               \
	case 5:                                   \
	    globX_R = cellX_R + relY_R ;          \
	    globY_R = cellY_R + relX_R ;          \
	    break ;                               \
	case 6:                                   \
	    globX_R = cellX_R - relY_R ;          \
	    globY_R = cellY_R + relX_R ;          \
	    break ;                               \
	case 7:                                   \
	    globX_R = cellX_R + relY_R ;          \
	    globY_R = cellY_R - relX_R ;          \
	    break ;                               \
	}                                         \
	if( Ytrans_xflagG ){                      \
	    globX_R++ ;                           \
	}                                         \
	if( Ytrans_yflagG ){                      \
	    globY_R++ ;                           \
	}                                         \
}

#else  /* defeat macro call function for test */
/* Note we need to pass address of global_R variable to effect a change */

#define REL_POS( orient_R,globX_R,globY_R,relX_R,relY_R,cellX_R,cellY_R )\
{                                                  \
    Ytrans_rel_pos( orient_R, &(globX_R), &(globY_R),     \
		relX_R, relY_R,                    \
		cellX_R, cellY_R ) ;               \
}
#define REL_POST(orient_R,globX_R,globY_R,relX_R,relY_R,cellX_R,cellY_R)\
{                                                  \
    Ytrans_rel_post( orient_R, &(globX_R), &(globY_R),     \
		relX_R, relY_R,                    \
		cellX_R, cellY_R ) ;               \
}


#endif /* RELPOSFUNC */

/* global variables */
EXTERN BOOL Ytrans_xflagG ;
EXTERN BOOL Ytrans_yflagG ;

/* global function definitions */

extern VOID Ytranslate( P5(INT *l,INT *b,INT *r,INT *t, INT orient) ) ; 
extern VOID Ytranslatef( P5(DOUBLE *l,DOUBLE *b,DOUBLE *r,DOUBLE *t, INT orient));
extern VOID Ytrans_init( P5(INT l,INT b,INT r,INT t, INT new_orient ) ) ;
extern INT Ytrans_inv_orient( P1(INT orient ) ) ;
extern VOID YtranslateC( P5(INT *l,INT *b,INT *r,INT *t, INT orient) ) ; 
extern VOID YtranslateT( P5(INT *l,INT *b,INT *r,INT *t, INT orient) ) ; 
extern VOID Ytrans_rel_pos(P7(INT ort,INT *gX,INT *gY,INT lX,INT lY,INT cx,INT cy));
/*
extern VOID Ytrans_rel_post(P7(INT ort,INT *X,INT *Y,INT lX,INT lY,INT cx,INT cy));
*/
extern VOID Ytrans_boun_init( P1(void) ) ;
extern VOID Ytrans_boun_add( P2(INT x,INT y ) ) ;
extern VOID Ytrans_boun( P4(INT orient,INT xc,INT yc,BOOL use_new_orient ) ) ;
extern BOOL Ytrans_boun_pt( P2(INT *x_ret,INT *y_ret ) ) ;
extern VOID Ytrans_boun_free( P1(void) ) ;


#endif /* RELPOS_H */
