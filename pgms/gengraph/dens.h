/* ----------------------------------------------------------------- 
FILE:	    dens.h                                       
CONTENTS:   definitions for cell routing area tiles.
DATE:	    Jan 20, 1990 
REVISIONS:  Tue Jan 15 22:29:34 PST 1991 - added TILE definitions.
----------------------------------------------------------------- */
/* *****************************************************************
   static char SccsId[] = "@(#) dens.h version 1.3 4/28/91" ;
***************************************************************** */
#ifndef DENS_H
#define DENS_H

#undef EXTERN 
#ifndef DENS_DEFS
#define EXTERN extern

#else
#define EXTERN
#endif

#define TILEL      1
#define TILET      2
#define TILER      3
#define TILEB      4

typedef struct tilebox {
    INT l  ;
    INT r ;
    INT b ;
    INT t ;
    INT side ;
    INT edge ;
    INT space ;
    BOOL orig ;
} TILEBOX, *TILEPTR ;

#endif /* DENS_H */
