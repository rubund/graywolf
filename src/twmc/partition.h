/* ----------------------------------------------------------------- 
"@(#) partition.h (Yale) version 3.3 9/5/90"
FILE:	    partition.h                                       
DESCRIPTION:insert file to support standard cell partitioner.
CONTENTS:   
DATE:	    May 17, 1989 
REVISIONS:  
----------------------------------------------------------------- */
#ifndef PARTITION_H
#define PARTITION_H

#define         INITROWSEP      1.0 
#define		EVEN_ODD( x )	( (x) & 01 )
#define		EVEN	0

typedef	struct d_field {
    int			x_lb	;
    int			x_ub	;
    int                 cell    ;
    struct d_field	*prev	;
    struct d_field	*next	;
} DFLD, *DFLDPTR ;

typedef	struct b_record {
    SHORT		state	;
    SHORT		number	;
    int			length	;
    int			x_lb	;
    int			x_ub	;
    int			y_lb	;
    int			y_ub	;
    struct d_field	*macro	;
} BLOCK, *BLOCKPTR ;

#endif /* PARTITION_H */
