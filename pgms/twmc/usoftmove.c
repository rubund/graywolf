/* ----------------------------------------------------------------- 
FILE:	    usoftmove.c                                       
DESCRIPTION:pin move routine.
CONTENTS:   BOOL usoftmove( acellptr,UCpin,seq,firstNewSite,lastNewSite ) 
		INT UCpin , seq , firstNewSite , lastNewSite ;
		CELLBOXPTR acellptr ;
DATE:	    Jun 27, 1988 
REVISIONS:  Oct 21, 1988 - removed Hweight and Vweight fields.
	    Mar 16, 1989 - rewrote data structure and move upin to 
		usoftmove.c.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) usoftmove.c version 3.3 9/5/90" ;
#endif

#include <custom.h>
#include <yalecad/relpos.h>
#include <yalecad/debug.h>

/* ----------------------------------------------------------------- 
   important global definitions - defined in custom.h used for feedback.
   INT overfillG, overpenalG ;
   DOUBLE pinFactorG ;
*/

