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
    INT          x1 ;
    INT          x2 ;
    INT     	 y1 ;
    INT     	 y2 ;
    UNSIGNED_INT ref ; /* may be referenced by this number */
    UNSIGNED_INT color ;
    UNSIGNED_INT label ;
} DATABOX, *DATAPTR ;

#endif /* DBINARY_H */
