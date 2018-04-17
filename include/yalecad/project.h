/* ----------------------------------------------------------------- 
FILE:	    project.h                                       
CONTENTS:   definitions for projection package.
DATE:	    Tue Oct 29 15:05:57 EST 1991
REVISIONS:  Thu Mar  5 03:41:11 EST 1992 - added very useful
		macro function Yproject_intersect.
----------------------------------------------------------------- */
#ifndef YPROJECT_H
#define YPROJECT_H

#ifndef lint
static char Yproject_HId[] = "@(#) project.h version 1.2 3/5/92" ;
#endif


#define NOTOUCH       0               /* tiles don't touch or overlap */
#define TOUCH        -1               /* tiles touch but dont overlap */
#define OVERLAP1      1               /* tiles overlap completely */
#define OVERLAP2      2               /* tiles overlap to right (top) */
#define OVERLAP3      3               /* tiles overlap to left (bot) */

/* very useful macro which determines intersection */
#define Yproject_intersect( t1_l, t1_r, t1_b, t1_t, t2_l, t2_r, t2_b, t2_t ) \
( YprojectX((t1_l),(t1_r),(t2_l),(t2_r) ) ? \
  YprojectY((t1_b),(t1_t),(t2_b),(t2_t) ) : 0 )

extern Yproject_space( P2(INT xspace, INT yspace ) ) ;

extern INT YprojectX( P4( INT tile1_left, INT tile1_right,
		          INT tile2_left, INT  tile2_right ) ) ;
extern INT YprojectY( P4( INT tile1_bot, INT tile1_top, 
			  INT tile2_bot, INT tile2_top ) ) ;

#endif /* YPROJECT_H */
