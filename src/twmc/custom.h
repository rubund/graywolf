/* ----------------------------------------------------------------- 
FILE:	    custom.h                                       
CONTENTS:   definitions for globals structures and variables
	    file organized - defines, typedefs, globals.
DATE:	    Jan 29, 1988 
REVISIONS:
	    Feb 13, 1988 - changed numBinsX & numBinsY -> maxBinX
		and maxBinY.  Also redefine LOCBOX to be termbox.
	    Feb 19, 1988 - changed aveChanWid to DOUBLE from int.
	    Oct 21, 1988 - remove Hweight and Vweight from DIMBOX.
	    Oct 26, 1988 - updated FIXEDBOX for neighborhoods.
		changed PATHNETPTR to generic pointer and added
		groupbox to cell data structure.
	    Dec  3, 1988 - added PSETPTR record for timing driven code
			 - add list of paths to cell record.
	    Jan 20, 1989 - added softPinArrayG for softcells.
	    Feb 26, 1989 - added G suffix for all global variables.
			 - changed SetBin to SETBIN so looks like macro.
			 - added CUSTOM_H compile switch.
	    Mar 01, 1989 - remove netctr variable.
			 - made iteration a global variable.
	    Mar 02, 1989 - moved start_overlap, end_overlap, etc. to 
		penalties.c
	    Mar 07, 1989 - added numinstancesG to list of cell info.
	    Mar 12, 1989 - added group_nested field to CELLBOX so
		that groups can be hierarchical.
	    Mar 16, 1989 - rewrote entire netlist data structures.
		Deleted unnecessary data structures.
	    Mar 30, 1989 - changed tile datastructure and MOVEBOX.
	    May 18, 1989 - added layer and HOWMANYORIENTs.
	    May 18, 1989 - removed extraneous variables.  Added pad
		control variables and defines.
	    Jul 19, 1989 - added numstdcellG global for avoiding work.
	    Oct 18, 1989 - removed pitch and track_spacing. Added
		two wire factors - one for each direction.
	    Mar 28, 1990 - major rewrite of softcell code.
	    Apr 11, 1990 - changed group record.
	    Nov 23, 1990 - changed mean_cellAreaG and dev_cellareaG
		to doubles.
	    Sun Dec 16 00:27:36 EST 1990 - modified to handle analog
		information.
	    Thu Jan 17 00:45:41 PST 1991 - added numpins to VERTBOX.
	    Mon Feb  4 02:03:45 EST 1991 - added new definitions to 
		make a correct softpin placer.
	    Wed Feb 13 23:34:29 EST 1991 - rewrote pad code.
	    Sat Feb 23 00:19:51 EST 1991 - finalized pad data structure
		to be like placepad algorithm.
	    Thu Apr 18 01:30:38 EDT 1991 - got rid of grid variables
		now use library function.
	    Sun May  5 14:22:05 EDT 1991 - now user can set origin.
	    Mon May  6 02:34:54 EDT 1991 - added lo and hi pos to
		make pin placement better.
	    Thu Aug 22 22:08:22 CDT 1991 - added unique_classG to fix
		problem with fixed cells.
----------------------------------------------------------------- */
/* *****************************************************************
   "@(#) custom.h (Yale) version 3.15 8/22/91"
***************************************************************** */
#ifndef CUSTOM_H
#define CUSTOM_H 

#include <main.h>
#include <analog.h>

#ifndef CUSTOM_DEFS
#define EXTERN extern

#else
#define EXTERN
#endif

/* ----------------------------------------------------------------- 
   DEFINE STATEMENTS FOR PLACEMENT STAGE
----------------------------------------------------------------- */
#define SETBINX(x) (( (TrybinG=((x)-binXOffstG)/binWidthXG)<0 ) ? 0 : \
                   ((TrybinG>maxBinXG) ? maxBinXG : TrybinG )) 
#define SETBINY(x) (( (TrybinG=((x)-binYOffstG)/binWidthYG)<0 ) ? 0 : \
                   ((TrybinG>maxBinYG) ? maxBinYG : TrybinG )) 
#define PICK_INT(l,u) (((l)<(u)) ? ((RAND % ((u)-(l)+1))+(l)) : (l))

#define deltaX  2
#define deltaY  2
#define EXPCELLPERBIN   5   /* we expect no more than 5 cells in a bin */
#define HOWMANYORIENT   8

/* control for pad spacing */
#define UNIFORM_PADS  0
#define VARIABLE_PADS 1
#define ABUT_PADS     2
#define EXACT_PADS    3

/* ----------------------------------------------------------------- 
   STUCTURE AND TYPEDEFS FOR PLACEMENT STAGE
----------------------------------------------------------------- */
typedef struct glistbox {  /* generic list */
    union {
	int net ;      /* make code easier to read */
	int path ;     
	int cell ;     
	int side ;     
    } p ; 
    struct glistbox *next ;
} GLISTBOX , *GLISTPTR ;

typedef struct movebox {
    int   xcenter ;
    int   ycenter ;
    int   r ;
    int   l ;
    int   t ;
    int   b ;
    DOUBLE   rw ;  /* the tile weights */
    DOUBLE   lw ;
    DOUBLE   tw ;
    DOUBLE   bw ;
    int   binR ;
    int   binL ;
    int   binT ; 
    int   binB ;
    int   cell ; 
    int   orient ; 
    int   numtiles ; 
    BOOL  loaded_previously ; 
} MOVEBOX, *MOVEBOXPTR ; 

typedef struct eq_pinbox {   /*  for hard equiv pins and addequiv pins */
    char *pinname          ; /*         equivalent pin name            */
    int *restrict1          ;  /* softpin is restricted to these sides  */
    int *txpos             ; /* array of cell instance xpos. cell rel. */
    int *typos             ; /* array of cell instance ypos. cell rel. */
    int layer              ; /* layer of the equivalent pin            */
    struct eq_pinbox *next;  /*    pointer to next equivalent pin      */
} EQUIVBOX, *EQUIVPTR ;

typedef struct sbox { 
    struct pinbox **children;  /* array of the pin's children           */
    struct pinbox *parent;     /* pointer back to softpin pinbox        */
    int    *restrict1        ;  /* softpin is restricted to these sides  */
    int    hierarchy        ;  /* type of pin root leaf, subroot etc    */
    int    side             ;  /* current side of the softpin           */
    BOOL   permute          ;  /* true if rank ordered                  */
    BOOL   ordered          ;  /* true if ordered in a group            */
    BOOL   fixed            ;  /* true if rank is fixed                 */
    int    lo_pos           ;  /* low valid position                    */
    int    hi_pos           ;  /* hi valid position                     */
    FLOAT  lowerbound       ;  /* bounds of a pingroup                  */
    FLOAT  upperbound       ;  /* upper bound of a pin group            */
} SOFTBOX, *SOFTBOXPTR ;

typedef struct pinbox {
    struct pinbox *next    ; /*       next pin for this net            */
    struct pinbox *nextpin ; /*       next pin for this cell           */
    SOFTBOXPTR    softinfo ; /* soft pin info if appl. otherwise NULL  */
    char *pinname          ; /*               pin record               */
    int pin                ; /*          global index of pin           */
    int net                ; /*          global index of net           */
    int cell               ; /*          global index of cell          */
    int xpos               ; /*           global x position            */
    int ypos               ; /*           global y position            */
    int instance           ; /*      current cell instance             */
    int txpos_new          ; /* proposed cell relative position of pin */
    int typos_new          ; /* proposed cell relative position of pin */
    int txpos              ; /* current cell relative position of pin  */
    int typos              ; /* current cell relative position of pin  */
    int *txpos_orig        ; /* orig array of cell inst xpos. cell rel.*/
    int *typos_orig        ; /* orig array of cell inst ypos. cell rel.*/
    int newx               ; /* scratch for determining new global xpos*/
    int newy               ; /* scratch for determining new global ypos*/
    int flag               ;
    int skip               ;
    int layer              ; /*        layer information               */
    int type               ;  /* softpin, hardpin, equiv pin           */
    FLOAT *timing          ; /* array of timing information for a pin  */
    SOFTBOXPTR *soft_inst  ; /* array of spin info if appl. o.w. NULL  */
    EQUIVPTR   eqptr       ; /*        list of equivalent pins         */
    ANALOGPTR analog       ; /* analog information if given            */
    ANALOGPTR *ainst       ; /* analog instance information            */
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
    Note: For all pins t?pos_orig is what the user gave us.  We need
    to use original position to calculate new pin position for aspect 
    ratio moves for softcells and for outputing pin information to
    global router for hard pins.  T?pos is the current x and y
    positions of the pin and for hard cells it is the average of all 
    the equivs for a pin and for softcells it is the current position
    due to the current aspect ratio.  For softcell, softinfo is valid
    otherwise it is NULL.
   - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
} *PINBOXPTR , PINBOX ;

typedef struct tilebox { 
    DOUBLE lweight ;
    DOUBLE rweight ;
    DOUBLE bweight ;
    DOUBLE tweight ;
    int left       ;   /* current tile sides */
    int right      ;
    int bottom     ;
    int top        ;
    int orig_left      ;/* original tile sides specified by user */
    int orig_right     ;/* for hard cell left = orig_left, that is */
    int orig_bottom    ;/* sides don't change throughout run. But for */
    int orig_top       ;/* soft cells use this to calculate current side*/
    struct tilebox *next ; /* next tile in list */
} *TILEBOXPTR , TILEBOX ;

typedef struct psidebox {
    int length   ;
    DOUBLE pincount;
    int vertical ;
    int position ;
} PSIDEBOX ;

typedef struct {
    int   r ;
    int   l ;
    int   t ;
    int   b ;
} BOUNBOX, *BOUNBOXPTR ;

typedef struct {
    int   *x ;                /* the current positions of vertices */
    int   *x_orig ;           /* the original positions of vertices */
    int   *x_new ;            /* the proposed positions of the vertices */
    int   *y ;                /* the current positions of vertices */
    int   *y_orig ;           /* the original positions of vertices */
    int   *y_new ;            /* the proposed positions of the vertices */
    FLOAT *numpins;           /* number of pins on each side */
} VERTBOX, *VERTBOXPTR ;

typedef struct {
    int  fixedType ;  /* POINT, NEIGHBORHOOD, GROUP, FIXEDGROUP */
    int   xcenter ;
    int   ycenter ;
    BOOL  leftNotRight ;
    BOOL  bottomNotTop ;
    int   xloc1 ;              /* for fixing cell within a neighborhood */
    int   yloc1 ;              /* remembers relative position to core */
    int   xloc2 ;             
    int   yloc2 ;
    BOOL  leftNotRight2 ;
    BOOL  bottomNotTop2 ;
    int   x1, y1 ;             /* global position */
    int   x2, y2 ;
    int   xspan ;             /* for speed of calculation */
    int   yspan ;
} FIXEDBOX, *FIXEDBOXPTR ;

typedef struct pad_rec {
    int    cellnum          ;  /* index in carray */
    int    length           ;
    int    height           ;
    int    position         ;  /* linear placement position for a side */
    int    tiebreak         ;  /* ideal location to place the pad */
    int    *children        ;  /* added for pad code */
    int    padside          ;  /* current side */
    int    padtype          ;  /* type of pad - pad or padgroup */
    int    hierarchy        ;  /* type of pad - root, lead, pad, etc */
    int    lo_pos           ;  /* low valid position -1 if every valid */
    int    hi_pos           ;  /* hi valid position PINFINITY if valid */
    BOOL   ordered          ;  /* whether a pad is ordered in a group */
    BOOL   permute          ;  /* whether rank is to be enforced */
    BOOL   fixed            ;  /* whether pad is fixed on a side */
    BOOL   valid_side[5]    ;  /* valid side a pad may be placed */
    DOUBLE lowerbound       ;
    DOUBLE upperbound       ;
} PADBOX, *PADBOXPTR ;

typedef struct {            /* keeps track of group information */
    FIXEDBOXPTR fixed       ;  /* the groups neighborhood */     
    GLISTPTR cells          ;  /* the cells of a group */
} GROUPBOX, *GROUPBOXPTR ;

typedef struct {
    char **name_inst        ;  /* array of names for each instance      */
    int numinstances        ;  /* number of instances for this cell     */
    int *numtile_inst       ;  /* number of tiles for each instance     */
    int *numsides           ;  /* array of number of sides for inst.    */
    TILEBOXPTR *tile_inst   ;  /* array of tile lists for each inst     */
    BOUNBOXPTR **bounBox    ;  /* the bbox in each view 0 each inst     */
    VERTBOXPTR *vert_inst   ;  /* array of vertices of cell orient 0    */
} INSTBOX, *INSTBOXPTR ;

typedef struct cellbox { 
    char *cname             ;
    int cellnum             ;  
    int class               ;
    int xcenter             ;
    int ycenter             ;
    int orientList[9]       ;
    int orient              ;
    int numpins             ;  /* number of pins for this cell          */
    int numtiles            ;  /* number of tiles in current instance   */
    int numsides            ;  /* number of sides for the current inst  */
    int cur_inst            ;  /* current instance of cell [0,numinst]  */
    BOOL softflag           ;  /* TRUE if it is a soft cell             */
    CELLTYPE celltype       ;  /* custom, soft , pad etc                */
    GLISTPTR paths          ;  /* paths of a cell                       */
    GLISTPTR nets           ;  /* all the nets that a cell is on        */
    FIXEDBOXPTR fixed       ;  /* cell can't move if non NULL           */
    INSTBOXPTR  instptr     ;  /* valid if cell has instances           */
    PADBOXPTR padptr        ;  /* if pad -  pad attributes              */
    GROUPBOXPTR  group      ;  /* whether cell belongs to a group       */
    BOOL   group_nested     ;  /* shows whether this cell is nested     */
    DOUBLE orig_aspect      ;  /* original aspect ratio                 */
    DOUBLE aspect           ;  /* current aspect ratio                  */
    DOUBLE aspUB            ;  /* aspect ratio upper bound              */
    DOUBLE aspLB            ;  /* aspect ration lower bound             */
    TILEBOXPTR tiles        ;  /* current tiles of a cell-head of list  */
    VERTBOXPTR vertices     ;  /* array of vertices of cell orient 0    */
    BOOL       boun_valid   ;  /* boundary calc. - use delayed calc.    */
    BOUNBOXPTR *bounBox     ;  /* the bounding box excluding routing    */
    PINBOXPTR  pinptr       ;  /* all the pins of a cell                */
    PINBOXPTR  *softpins    ;  /* all the pins including softpingroups  */
} *CELLBOXPTR , CELLBOX     ;

typedef struct netbox {
    char *nname             ;  /* net name */
    PINBOXPTR  pins         ;  /* pins of this net */
    GLISTPTR   paths        ;  /* paths which this net belongs */
    int skip                ;
    int xmin                ;
    int newxmin             ;
    int xmax                ;
    int newxmax             ;
    int ymin                ;
    int nflag               ;
    int newymin             ;
    int ymax                ;
    int newymax             ;
    int numpins             ;
    int Bnum                ;  /* no. of pins on the bottom edge of bb */
    int Lnum                ;  /* no. of pins on the left edge of bb */
    int Rnum                ;  /* no. of pins on the right edge of bb */
    int Tnum                ;  /* no. of pins on the top edge of bb */
    int newBnum             ;  /* no. of pins on the bottom edge of bb */
    int newLnum             ;  /* no. of pins on the left edge of bb */
    int newRnum             ;  /* no. of pins on the right edge of bb */
    int newTnum             ;  /* no. of pins on the top edge of bb */
    int newhalfPx           ;  /* new half perimeter bounding box for x */
    int newhalfPy           ;  /* new half perimeter bounding box for y */
    int halfPx              ;  /* current half perimeter bounding box for x */
    int halfPy              ;  /* current half perimeter bounding box for y */
    FLOAT driveFactor       ;  /* driver strength of this net */
    FLOAT max_driver        ;  /* largest driver on this net */
    FLOAT min_driver        ;  /* smallest driver on this net */
    ANETPTR analog_info     ;  /* for analog nets */
} *NETBOXPTR , NETBOX ;

typedef struct kbox {
    int cap ;
    int HV ;
    int sp ;
    int x ;
    int y ;
} KBOX , *KBOXPTR ;

typedef struct pathbox {
    int lo_path_len ;     /* lower bound on the calculated half perim */
    int hi_path_len ;     /* upper bound on the calculated half perim */
    int new_lo_path_len ; /* new low path */
    int new_hi_path_len ; /* new upper path */
    int priority ;
    int upper_bound ;
    int lower_bound ;
    GLISTPTR nets ;
    struct pathbox *next ;  /* build a list first then array for speed */
} PATHBOX , *PATHPTR ;

typedef struct binbox {
    int left ;
    int right ;
    int top ;
    int bottom ;
    int penalty ;
    int nupenalty ;
    int *cells ;   /* array of cells in bin */
    int space ;    /* size of binbox->cells array */
} BINBOX ,
*BINBOXPTR ;

typedef struct psetrec {
    int  member; /* integer for determining membership */
    int  path ;  /* data */
    struct psetrec *next ;
} PSETBOX, *PSETPTR ; /* path set record */

/* ----------------------------------------------------------
   The global variable definitions 
---------------------------------------------------------- */
EXTERN CELLBOXPTR *cellarrayG   ;
EXTERN CELLBOXPTR *activeCellarrayG ; /* cells that are active */
EXTERN CELLBOXPTR *softPinArrayG ;    /* softcells that have pins */
EXTERN NETBOXPTR  *netarrayG     ;    /* array of nets */
EXTERN PATHPTR    *patharrayG ;       /* array of paths */
EXTERN PINBOXPTR  *termarrayG  ;      /* array of pins */
EXTERN BINBOXPTR  **binptrG ;         /* overlap data structure */
EXTERN int **net_cap_matchG ;         /* set of match nets */
EXTERN int **net_res_matchG ;         /* set of match nets */

/* ----------------------------------------------------------
   The global variable definitions for Jimmy's controller.
---------------------------------------------------------- */
EXTERN int d_costG;
EXTERN DOUBLE init_accG;
EXTERN DOUBLE ratioG ;


/* ----------------------------------------------------------
   global definitions for overlap calculations
---------------------------------------------------------- */
/* move box array */
EXTERN MOVEBOXPTR *old_aposG, *new_aposG, *old_bposG, *new_bposG ;
/* first element of move box array */
EXTERN MOVEBOXPTR old_apos0G, new_apos0G, old_bpos0G, new_bpos0G ;
EXTERN BINBOXPTR  newbptrG; /* *** bin cell ptr *** */ 
EXTERN int *newCellListG ;  /* *** bin cell list *** */ 
EXTERN int TrybinG ;        /* used in setBin calculations */ 

/* ----------------------------------------------------------
   global definitions for configuration calculations
---------------------------------------------------------- */
EXTERN int bdxlengthG , bdylengthG ;
EXTERN int blocklG , blockrG , blocktG , blockbG ;
EXTERN int blockmxG , blockmyG ;
EXTERN int halfXspanG , halfYspanG ;
EXTERN DOUBLE chipaspectG ;

/* ----------------------------------------------------------
    cells are arranged in cellarray in the following fashion:
    1. - core cells - custom and softcells 1..numcells.
    1a.- core cells may have instances. so add numinstances to all below.
    2. - super cells - numcells+1..numcells+numsupercells.
    3. - pad cells - numcells+numsupercells+11..
		      numcells+numsupercells+numpads.
    4  - pad groups - numcells+numsupercells+numpads+1..
		      numcells+numpads+numpadgroups
    5. - padmacros - always four-for channel graph generator. see outgeo.c. 
    We have the following definitions to simplify region indexes:
	endsuper = numcells + numsupercells + numinstances ;
	endpads = numcells+numsupercells+numpads ;
	endpadgrps = numcells+numsupercells+numpads+numpadgroups ;
	totalcells = numcells+numsupercells+numpads+numpadgroups+NUMPADMACROS ;
---------------------------------------------------------- */
EXTERN int numcellsG  ;
EXTERN int numpadsG   ;
EXTERN int totalpadsG ;
EXTERN int numsoftG   ;
EXTERN int numstdcellG ;
EXTERN int numpadgroupsG ;
EXTERN int numinstancesG ;
EXTERN int numsupercellsG  ;
EXTERN int numpathsG  ;
EXTERN int endpadsG ;
EXTERN int endpadgrpsG ;
EXTERN int endsuperG ;
EXTERN int totalcellsG;
EXTERN int activecellsG;  /* number of active cells not fixed or merged */
EXTERN int numnetsG ;
EXTERN int numpinsG ;
EXTERN int maxBinXG   ;
EXTERN int maxBinYG   ;
EXTERN int binWidthXG ;
EXTERN int binWidthYG ;
EXTERN int binXOffstG ;
EXTERN int binYOffstG ;
EXTERN int maxWeightG ;
EXTERN int baseWeightG;
EXTERN DOUBLE mean_cellAreaG ;
EXTERN DOUBLE dev_cellAreaG  ;
EXTERN DOUBLE slopeXG    ;
EXTERN DOUBLE slopeYG    ;
EXTERN DOUBLE basefactorG;
EXTERN DOUBLE aveChanWidG;
EXTERN DOUBLE wireFactorXG;
EXTERN DOUBLE wireFactorYG;

/* ----------------------------------------------------------
   global definitions for cost function
---------------------------------------------------------- */
EXTERN int penaltyG   ;
EXTERN int binpenalG  ;
EXTERN int funccostG  ;
EXTERN int offsetG    ;
EXTERN int timingcostG ;
EXTERN int timingpenalG ;
EXTERN DOUBLE coreFactorG ;
EXTERN DOUBLE lapFactorG ;
EXTERN DOUBLE timeFactorG ;
EXTERN DOUBLE vertical_wire_weightG ;
EXTERN DOUBLE vertical_path_weightG ;

/* annealing stuff */
EXTERN DOUBLE TG ;
EXTERN FILE *fpoG ;
EXTERN int randVarG ;
EXTERN int attmaxG  ;
EXTERN int iterationG ;

EXTERN DOUBLE pinsPerLenG ;
EXTERN int layersFactorG ;
EXTERN int unique_classG ;

/* ----------------------------------------------------------
   global definitions for calculated statistics
---------------------------------------------------------- */
EXTERN int perimG ;
EXTERN int totChanLenG ;
EXTERN int totNetLenG  ;
EXTERN DOUBLE aveCellSideG ;
EXTERN DOUBLE expandExtraG ;

EXTERN int flipsG ;
EXTERN DOUBLE avg_funcG , avgsG ;

EXTERN int iwireG, iwirexG , iwireyG , icostG ;
EXTERN int fwireG, fwirexG , fwireyG , fcostG ;

/* control for pad code */
EXTERN int padspacingG ;  /* may be UNIFORM, VARIABLE, or ABUT */
EXTERN BOOL contiguousG ;
EXTERN BOOL external_pad_programG ; /* TRUE for calling placepads */
EXTERN int min_pad_spacingG ;
EXTERN int coreG[2][2] ;
EXTERN int perdimG[2] ;
EXTERN PADBOXPTR *padarrayG ;   /* array of all the pads and padgroups */
EXTERN PADBOXPTR *sortarrayG ;  /* the sorted array of pads */
EXTERN PADBOXPTR *placearrayG ; /* array where the pads will be placed */

/* control gridding of cells */
EXTERN BOOL gridGivenG ;
EXTERN BOOL gridCellsG ;
EXTERN BOOL coreGivenG ;
EXTERN int  x_originG, y_originG ;


#undef EXTERN


#endif /* CUSTOM_H */
