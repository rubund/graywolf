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
BOOL Ytrans_xflagG ;
BOOL Ytrans_yflagG ;

/* global function definitions */

void Ytranslate(int *l,int *b,int *r,int *t, int orient); 
void Ytranslatef(double *l,double *b,double *r,double *t, int orient);
void Ytrans_init(int l,int b,int r,int t, int new_orient);
int Ytrans_inv_orient(int orient);
void YtranslateC(int *l,int *b,int *r,int *t, int orient); 
void YtranslateT(int *l,int *b,int *r,int *t, int orient); 
void Ytrans_rel_pos(int ort,int *gX,int *gY,int lX,int lY,int cx,int cy);
/*
void Ytrans_rel_post(P7(int ort,int *X,int *Y,int lX,int lY,int cx,int cy));
*/
void Ytrans_boun_init() ;
void Ytrans_boun_add(int x,int y);
void Ytrans_boun( int orient, int new_xc, int new_yc, BOOL use_new_orient );
BOOL Ytrans_boun_pt(int *x_ret,int *y_ret);
void Ytrans_boun_free();

#endif /* RELPOS_H */
