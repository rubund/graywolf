/* ----------------------------------------------------------------- 
FILE:	    dens.h                                       
CONTENTS:   definitions for cell routing area tiles.
DATE:	    Jan 20, 1990 
REVISIONS:  Thu Jan 17 00:46:38 PST 1991 - added side to RTILEBOX
		and added the TILE macro definitions.
	    Wed May  1 19:16:12 EDT 1991 - added switchbox definition.
----------------------------------------------------------------- */
/* *****************************************************************
   static char SccsId[] = "@(#) dens.h version 3.5 5/1/91" ;
***************************************************************** */
#ifndef DENS_H
#define DENS_H

#define TILEL      1
#define TILET      2
#define TILER      3
#define TILEB      4

typedef struct rtilebox {
    int x1 ;
    int y1 ;
    int x2 ;
    int y2 ;
    int side ;
    BOOL switchbox ;
    struct rtilebox *next ;
} RTILEBOX, *RTILEBOXPTR ;

/* list of tiles to be added to each cell to accommodate routing */
RTILEBOXPTR *routingTilesG ;

#endif /* DENS_H */
