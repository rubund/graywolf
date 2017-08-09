/* ----------------------------------------------------------------- 
"@(#) dbinary.h (Yale) version 1.3 8/28/90"
FILE:	    dbinary.h                                       
DESCRIPTION:include file for binary format for graphics routines
CONTENTS:   typedefs for binary format.
DATE:	    Mar 21, 1989 - moved from draw.h
REVISIONS:  
----------------------------------------------------------------- */
#ifndef DBINARY_H
#define DBINARY_H

/* data record for draw binary files */
typedef struct {
    int          x1 ;
    int          x2 ;
    int     	 y1 ;
    int     	 y2 ;
    unsigned int ref ; /* may be referenced by this number */
    unsigned int color ;
    unsigned int label ;
} DATABOX, *DATAPTR ;

#endif /* DBINARY_H */
