/* ----------------------------------------------------------------- 
"@(#) groute.h (Yale) version 4.7 3/23/92"
FILE:	    globe.h                                       
DESCRIPTION:global routing definitions and typedefs.
CONTENTS:   
DATE:	    Mar 27, 1989 
REVISIONS:  Fri Jan 25 23:47:24 PST 1991 - removed redundant variables.
	    Fri Mar 22 15:16:41 CST 1991 - changed short to
		int since it wasn't saving space and could
		be a problem for large designs.
----------------------------------------------------------------- */
#ifndef GROUTE_H
#define GROUTE_H

#ifdef INFINITY
#undef INFINITY
#endif
#define INFINITY 0x7fffffff
#define LEFT  -1
#define RIGHT 1
#define OLD 0
#define NEW 1
#define swLINE 1
#define nswLINE 2
#define swL_up 3
#define swL_down 4
#define VERTICAL 6
#define HORIZONTAL 7
#define swUP 8
#define swDOWN 9

typedef struct changrdbox {
    PINBOXPTR netptr ;
    short cross ;
    short tracks ;
    struct changrdbox *prevgrd ;
    struct changrdbox *nextgrd ;
    struct densitybox *dptr ;
}
*CHANGRDPTR ,
CHANGRDBOX ;

typedef struct tgridbox {
    CHANGRDPTR up ;
    CHANGRDPTR down ;
}
*TGRIDPTR ,
TGRIDBOX ;

typedef struct densitybox {
    CHANGRDPTR grdptr ;
    struct densitybox *next ;
    struct densitybox *back ;
}
*DENSITYPTR ,
DENSITYBOX ;

typedef struct segm_box {
    short flag ;
    unsigned int switchvalue : 8 ;
    unsigned int swap_flag   : 8 ;
    PINBOXPTR pin1ptr ;
    PINBOXPTR pin2ptr ;
    struct segm_box *next ;
    struct segm_box *prev ;
}
*SEGBOXPTR ,
SEGBOX ;

typedef struct adjacent_seg {
    struct adjacent_seg *next ;
    struct segm_box *segptr ;
}
*ADJASEGPTR ,
ADJASEG ;

typedef struct imp_box {
    char *pinname ;
    char *eqpinname ;
    int txpos ;
    struct imp_box *next ;
    struct imp_box *prev ;
    int cell ;
    int xpos ;
    int terminal ;
} IPBOX , *IPBOXPTR ;

typedef struct feedcountbox {
    short needed ;
    short actual ;
    struct imp_box *firstimp ;
    struct imp_box *lastimp  ;
} FEED_DBOX ,
*FEED_DATA ;

#define GRDNULL ( ( CHANGRDPTR ) NULL ) 
#define DENSENULL ( ( DENSITYPTR ) NULL ) 

#endif /* GROUTE_H */
