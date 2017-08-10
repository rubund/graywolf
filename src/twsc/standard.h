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
	    Fri Mar 22 15:20:24 CST 1991 - added short
		definitions for large designs.
	    Tue Mar 26 11:33:47 EST 1991 - added NO_FEED_INSTANCES
		conditional compile.
----------------------------------------------------------------- */
#ifndef YSTANDARD_H 
#define YSTANDARD_H 

#include <globals.h>

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
    int bxcenter  ;
    int bycenter  ;
    int bleft     ;
    int bright    ;
    int desire    ;
    int blength   ;
    int oldsize   ;
    int newsize   ;
    int orig_desire ;
    int bbottom   ;
    int btop      ;
    int bheight   ;
    short bclass    ;
    short borient   ;
} *BBOXPTR, BBOX ;

typedef struct glistbox {  /* generic list */
    union {
	int net ;      /* make code easier to read */
	int path ;     
	int cell ;     
    } p ; 
    struct glistbox *next ;
} GLISTBOX , *GLISTPTR ;


typedef struct equiv_box {
    char *pinname ;
    char unequiv ;
    char txoff ;
    short typos ;
}
EQ_NBOX ,
*EQ_NBOXPTR ;

typedef struct pinbox {
    struct pinbox *next ;          /* next pin on this net */
    struct pinbox *nextpin ;       /* next pin on this cell */
    char *pinname ;
    int terminal ;
    int xpos     ;                 /* global positions */
    int ypos     ;
    int newx     ;
    int newy     ;
    short txpos[2] ;          /* cell relative position */
    short typos[2] ;          /* cell relative position */
    int cell ;
    int net ;
    short row  ;
    char pinloc   ;
    char flag     ;
    struct adjacent_seg *adjptr ;
    struct equiv_box *eqptr ;
} *PINBOXPTR, PINBOX ;

typedef struct pathbox {
    int path_len ;     /* bound on the calculated half perim */
    int new_path_len ; /* new path */
    int priority ;
    int upper_bound ;
    int lower_bound ;
    GLISTPTR nets ;
    struct pathbox *next ;  /* build a list first then array for speed */
} PATHBOX , *PATHPTR ;


typedef struct tilebox { 
    short left     ;
    short right    ;
    short bottom   ;
    short top      ;
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
    short swap_group ;
    short num_pin_group ;
} *SGLISTPTR, SGLIST ;

typedef struct cellbox { 
    char *cname           ;
    char corient          ;
    char orflag           ;
    char ECO_flag	  ;
    int cxcenter          ;
    int cycenter          ;
    int border            ;
    int cclass            ;
    unsigned int cbclass[8] ;
    short cheight    ;
    short clength    ;
    short cblock      ;
    short numterms    ;
    short num_swap_group  ;
    SGLISTPTR swapgroups ;
    GLISTPTR paths        ;  /* timing paths of a cell */
    struct pad_rec *padptr;
    struct fencebox *fence;
    struct imp_box *imptr ;
    PINBOXPTR pins        ;  /* the pins of the cell */
    TIBOXPTR tileptr ;
} *CBOXPTR, CBOX ;

typedef struct fencebox {
    int min_block   ;
    int max_block   ;
    int min_xpos    ;
    int max_xpos    ;
    struct fencebox *next_fence ;
} *FENCEBOXPTR, FENCEBOX ;

typedef struct dimbox {
    PINBOXPTR pins ;         /* pins of the net */
    char *name   ;
    char dflag    ;
    char feedflag ;
    char ignore   ;
    int xmin     ;
    int newxmin  ;
    int xmax     ;
    int newxmax  ;
    int ymin     ;
    int newymin  ;
    int ymax     ;
    int newymax  ;
    short Lnum     ;
    short newLnum  ;
    short Rnum     ;
    short newRnum  ;
    short Bnum     ;
    short newBnum  ;
    short Tnum     ;
    short newTnum  ;
    short numpins  ;
    GLISTPTR paths ;     /* paths which this net belongs */
    int newhalfPx ;     /* new half perimeter bounding box */
    int newhalfPy ;     /* new half perimeter: y portion */
    int halfPx ;        /* current half perimeter bounding box */
    int halfPy ;        /* current half perimeter: y portion */
} *DBOXPTR, DBOX ;


typedef struct hash {
    char *hname ;
    int hnum ;
    struct hash *hnext ;
} HASHBOX, *HASHPTR ;

typedef struct binbox {
    int left ;
    int right ;
    int *cell ;
    int penalty ;
    int nupenalty ;
} BINBOX, *BINPTR ;

typedef struct pin_list {              /* list of pins */
    PINBOXPTR swap_pin ;
    struct pin_list *next ;		/* Next pin in pin group */
    struct pin_list *next_grp ;		/* Next pin group in same cell */
} PINLIST, *PINLISTPTR ;

typedef struct swapbox {        /* list of list of pins to be swapped */
    int num_pin_grps ;
    YHASHPTR pin_grp_hash ;	
} SWAPBOX ;

/* ****************** GLOBALS ************************** */
/* THE MAJOR PARTS OF THE DATA STRUCTURES */
extern CBOXPTR  *carrayG  ;
extern DBOXPTR   *netarrayG   ;
extern PINBOXPTR *tearrayG  ;
extern BBOXPTR *barrayG ;
extern BINBOX ***binptrG ;
extern PATHPTR *patharrayG ;  /* array of timing paths */

double vertical_path_weightG ;
double horizontal_path_weightG ;
double vertical_wire_weightG ;

/* the configuration */
extern int numcellsG ;
extern int numtermsG ;
extern int numnetsG ;
extern int numpadgrpsG ;
extern int lastpadG ;
extern int maxtermG ;
extern int numRowsG ;
extern int numChansG ;
extern int numpathsG ;
extern int numBinsG ;
extern int binWidthG ;
extern int binOffstG ;
extern int TotRegPinsG ;
extern int implicit_feed_countG ;

/* for the penalties */
int TrybinG   ;
int binpenalG ;
int funccostG ;
int newbinpenalG ;
int newrowpenalG ;
int penaltyG  ;
int rowpenalG ;
int timingcostG ;
double binpenConG ;
double roLenConG ;
double timeFactorG ;

/* *********************** PROTOTYPES FOR TWSC ******************** */
void init_table( void ) ;
BOOL acceptt( int d_wire, int d_time, int d_penal ) ;
BOOL accept_greedy( int d_wire, int d_time, int d_penal ) ;

#endif /* YSTANDARD_H */
