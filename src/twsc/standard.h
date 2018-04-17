/* ----------------------------------------------------------------- 
"@(#) standard.h (Yale) version 4.16 4/2/92"
FILE:	    standard.h                                       
DESCRIPTION:TimberwolfSC main insert file.
CONTENTS:   
DATE:	    Mar 27, 1989 
REVISIONS:  Sat Dec 15 22:08:21 EST 1990 - modified pinloc values
		so that it will always be positive.
	    Thu Dec 20 00:05:40 EST 1990 - made timing values doubles
		for more accuracy.
	    Thu Jan 24 16:34:51 PST 1991 - added user initialization.
	    Fri Mar 22 15:20:24 CST 1991 - added SHORT_LONG
		definitions for large designs.
	    Tue Mar 26 11:33:47 EST 1991 - added NO_FEED_INSTANCES
		conditional compile.
----------------------------------------------------------------- */
#ifndef YSTANDARD_H 
#define YSTANDARD_H 

#ifdef VMS
#define lINT 
#endif
/*  a cheap way of avoiding the inclusions of the sccs
    stuff for VMS systems
    */

#include <yalecad/base.h>


#ifndef NO_FEED_INSTANCES
#define FEED_INSTANCES  /* ---selection of this keyword will cause  */
/*                            each feed-through cell to have        */
/*                            a distinct (instance) name            */
#endif /* NO_FEED_INSTANCES */

/* I/O macros */
#include <yalecad/file.h>

/* Pin list includes an embedded hash table */
#include <yalecad/hash.h>

#ifdef MAIN_VARS
#define EXTERN 
#else
#define EXTERN extern 
#endif

/* ***********LEAVE THE BELOW UNCHANGED *************************** */
/* remove graphics lines of code if compile switch is on */
#ifdef NOGRAPHICS
#define G(x_xz) 

#else 
#define G(x_xz)   x_xz

#endif /* NOGRAPHICS */

#define MAXINT 0x7FFFFFFF

#define ALLOC(type)    (  (type *) Ysafe_malloc ( sizeof(type) )  )
#define SetBin(x) (( (TrybinG=((x)-binOffstG)/binWidthG)<0 ) ? 0 : \
                  ((TrybinG>numBinsG) ? numBinsG : TrybinG )) 

#ifdef USER_DEFS   /* allow user to add custom interface */

#define USER_INITIALIZATION()    User_initialization()
#define USER_NEXT_METER()        User_next_meter()
#define USER_INCR_METER()        User_incr_meter()
#define USER_SEND_VALUE( a )     User_send_value( a )

#else  /* NO USER DEFINITIONS */

#define USER_INITIALIZATION()
#define USER_NEXT_METER() 
#define USER_INCR_METER()  
#define USER_SEND_VALUE( a ) 

#endif /* USER_DEFINITIONS */

#define CULLNULL (CBOXPTR) NULL
#define PINNULL (PINBOXPTR) NULL
#define DIMNULL (DBOXPTR) NULL
#define TILENULL (TIBOXPTR) NULL
#define LEFT_MOST 0
#define RITE_MOST 1
#define FEED_FLAG 2
#define NOT_DONE -2
#define NO_PINS -1000
#define BIG_NUMBER 32000
#define TW_PRIME 49999
#define TW_PRIME2 1009
#define GATE_ARRAY_MAGIC_CONSTANT -1000001

/* used to describe pin location  */
#define LEFTCELL   1    /* was -2 */
#define BOTCELL    2    /* was -1 */
#define NEITHER    3    /* was 0  */
#define TOPCELL    4    /* was 1  */
#define RITECELL   5    /* was 2  */
#define PINLOC_OFFSET 3  /* diff needed to make field positive */

typedef struct blockbox {
    INT bxcenter  ;
    INT bycenter  ;
    INT bleft     ;
    INT bright    ;
    INT desire    ;
    INT blength   ;
    INT oldsize   ;
    INT newsize   ;
    INT orig_desire ;
    INT bbottom   ;
    INT btop      ;
    INT bheight   ;
    SHORT bclass    ;
    SHORT borient   ;
} *BBOXPTR, BBOX ;

typedef struct glistbox {  /* generic list */
    union {
	INT net ;      /* make code easier to read */
	INT path ;     
	INT cell ;     
    } p ; 
    struct glistbox *next ;
} GLISTBOX , *GLISTPTR ;


typedef struct equiv_box {
    char *pinname ;
    char unequiv ;
    char txoff ;
    SHORT_LONG typos ;
}
EQ_NBOX ,
*EQ_NBOXPTR ;

typedef struct pinbox {
    struct pinbox *next ;          /* next pin on this net */
    struct pinbox *nextpin ;       /* next pin on this cell */
    char *pinname ;
    INT terminal ;
    INT xpos     ;                 /* global positions */
    INT ypos     ;
    INT newx     ;
    INT newy     ;
    SHORT_LONG txpos[2] ;          /* cell relative position */
    SHORT_LONG typos[2] ;          /* cell relative position */
    INT cell ;
    INT net ;
    SHORT row  ;
    char pinloc   ;
    char flag     ;
    struct adjacent_seg *adjptr ;
    struct equiv_box *eqptr ;
} *PINBOXPTR, PINBOX ;

typedef struct pathbox {
    INT path_len ;     /* bound on the calculated half perim */
    INT new_path_len ; /* new path */
    INT priority ;
    INT upper_bound ;
    INT lower_bound ;
    GLISTPTR nets ;
    struct pathbox *next ;  /* build a list first then array for speed */
} PATHBOX , *PATHPTR ;


typedef struct tilebox { 
    SHORT_LONG left     ;
    SHORT_LONG right    ;
    SHORT_LONG bottom   ;
    SHORT_LONG top      ;
} *TIBOXPTR, TIBOX ;

// Each cell defines any number of swap groups.
// Each swap group has a record containing the
// swap group number and the number of pin groups
// in the cell belonging to that swap group.
//
// The most common uses are clock/buffer tree
// optimization and scan chain optimization.
// For these, num_pin_group is normally 1.
//
// Note that this record does not specify
// where in the swap group to find the pin
// group(s).

typedef struct swapgrouplist {
    SHORT swap_group ;
    SHORT num_pin_group ;
} *SGLISTPTR, SGLIST ;

typedef struct cellbox { 
    char *cname           ;
    char corient          ;
    char orflag           ;
    char ECO_flag	  ;
    INT cxcenter          ;
    INT cycenter          ;
    INT border            ;
    INT cclass            ;
    UNSIGNED_INT cbclass[8] ;
    SHORT_LONG cheight    ;
    SHORT_LONG clength    ;
    SHORT cblock      ;
    SHORT numterms    ;
    SHORT num_swap_group  ;
    SGLISTPTR swapgroups ;
    GLISTPTR paths        ;  /* timing paths of a cell */
    struct pad_rec *padptr;
    struct fencebox *fence;
    struct imp_box *imptr ;
    PINBOXPTR pins        ;  /* the pins of the cell */
    TIBOXPTR tileptr ;
} *CBOXPTR, CBOX ;

typedef struct fencebox {
    INT min_block   ;
    INT max_block   ;
    INT min_xpos    ;
    INT max_xpos    ;
    struct fencebox *next_fence ;
} *FENCEBOXPTR, FENCEBOX ;

typedef struct dimbox {
    PINBOXPTR pins ;         /* pins of the net */
    char *name   ;
    char dflag    ;
    char feedflag ;
    char ignore   ;
    INT xmin     ;
    INT newxmin  ;
    INT xmax     ;
    INT newxmax  ;
    INT ymin     ;
    INT newymin  ;
    INT ymax     ;
    INT newymax  ;
    SHORT Lnum     ;
    SHORT newLnum  ;
    SHORT Rnum     ;
    SHORT newRnum  ;
    SHORT Bnum     ;
    SHORT newBnum  ;
    SHORT Tnum     ;
    SHORT newTnum  ;
    SHORT numpins  ;
    GLISTPTR paths ;     /* paths which this net belongs */
    INT newhalfPx ;     /* new half perimeter bounding box */
    INT newhalfPy ;     /* new half perimeter: y portion */
    INT halfPx ;        /* current half perimeter bounding box */
    INT halfPy ;        /* current half perimeter: y portion */
} *DBOXPTR, DBOX ;


typedef struct hash {
    char *hname ;
    INT hnum ;
    struct hash *hnext ;
} HASHBOX, *HASHPTR ;

typedef struct binbox {
    INT left ;
    INT right ;
    INT *cell ;
    INT penalty ;
    INT nupenalty ;
} BINBOX, *BINPTR ;

typedef struct pin_list {              /* list of pins */
    PINBOXPTR swap_pin ;
    struct pin_list *next ;		/* Next pin in pin group */
    struct pin_list *next_grp ;		/* Next pin group in same cell */
} PINLIST, *PINLISTPTR ;

typedef struct swapbox {        /* list of list of pins to be swapped */
    INT num_pin_grps ;
    YHASHPTR pin_grp_hash ;	
} SWAPBOX ;

/* ****************** GLOBALS ************************** */
/* THE MAJOR PARTS OF THE DATA STRUCTURES */
EXTERN CBOXPTR  *carrayG  ;
EXTERN DBOXPTR   *netarrayG   ;
EXTERN PINBOXPTR *tearrayG  ;
EXTERN BBOXPTR *barrayG ;
EXTERN BINBOX ***binptrG ;
EXTERN PATHPTR *patharrayG ;  /* array of timing paths */

EXTERN DOUBLE vertical_path_weightG ;
EXTERN DOUBLE horizontal_path_weightG ;
EXTERN DOUBLE vertical_wire_weightG ;

/* the configuration */
EXTERN INT numcellsG ;
EXTERN INT numtermsG ;
EXTERN INT numnetsG ;
EXTERN INT numpadgrpsG ;
EXTERN INT lastpadG ;
EXTERN INT maxtermG ;
EXTERN INT numRowsG ;
EXTERN INT numChansG ;
EXTERN INT numpathsG ;
EXTERN INT numBinsG ;
EXTERN INT binWidthG ;
EXTERN INT binOffstG ;
EXTERN INT TotRegPinsG ;
EXTERN INT implicit_feed_countG ;

/* for the penalties */
EXTERN INT TrybinG   ;
EXTERN INT binpenalG ;
EXTERN INT funccostG ;
EXTERN INT newbinpenalG ;
EXTERN INT newrowpenalG ;
EXTERN INT penaltyG  ;
EXTERN INT rowpenalG ;
EXTERN INT timingcostG ;
EXTERN DOUBLE binpenConG ;
EXTERN DOUBLE roLenConG ;
EXTERN DOUBLE timeFactorG ;

#undef EXTERN  

/* *********************** PROTOTYPES FOR TWSC ******************** */
extern init_table( P1(void) ) ;
extern BOOL acceptt( P3(INT d_wire,INT d_time,INT d_penal) ) ;
extern BOOL accept_greedy( P3(INT d_wire,INT d_time,INT d_penal) ) ;


#endif /* YSTANDARD_H */
