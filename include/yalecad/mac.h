/* ----------------------------------------------------------------- 
FILE:	    mac.h                                       
CONTENTS:   Definitions for directing conditional compiles on the Mac.
DATE:	    May 2, 1990 
	    
REVISIONS:  
----------------------------------------------------------------- */
#ifndef MAC_H
#define MAC_H

#ifndef lint
static char Ymac_HId[] = "@(#) mac.h version 1.4 1/8/92" ;
#endif

#ifdef THINK_C

#include <limits.h>
#include <float.h>
#define ALTERNATE
#define DEBUG
/* #define CLEANUP_C */
#define SIGUSR1 1

#endif /* THINK_C */

#ifdef _AUX_SOURCE
#define NBPG	4096	/* number of bytes per page */
#endif /* _AUX_SOURCE */

#endif /* MAC_H */
