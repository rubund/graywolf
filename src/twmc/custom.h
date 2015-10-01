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
	INT net ;      /* make code easier to read */
	INT path ;     
	INT cell ;     
	INT side ;     
    } p ; 
    struct glistbox *next ;
} GLISTBOX , *GLISTPTR ;

typedef struct movebox {
    INT   xcenter ;
    INT   ycenter ;
    INT   r ;
    INT   l ;
    INT   t ;
    INT   b ;
    DOUBLE   rw ;  /* the tile weights */
    DOUBLE   lw ;
    DOUBLE   tw ;
    DOUBLE   bw ;
    INT   binR ;
    INT   binL ;
    INT   binT ; 
    INT   binB ;
    INT   cell ; 
    INT   orient ; 
    INT   numtiles ; 
    BOOL  loaded_previously ; 
} MOVEBOX, *MOVEBOXPTR ; 

typedef struct eq_pinbox {   /*  for hard equiv pins and addequiv pins */
    char *pinname          ; /*         equivalent pin name            */
    INT *restrict1          ;  /* softpin is restricted to these sides  */
    INT *txpos             ; /* array of cell instance xpos. cell rel. */
    INT *typos             ; /* array of cell instance ypos. cell rel. */
    INT layer              ; /* layer of the equivalent pin            */
    struct eq_pinbox *next;  /*    pointer to next equivalent pin      */
} EQUIVBOX, *EQUIVPTR ;

typedef struct sbox { 
    struct pinbox **children;  /* array of the pin's children           */
    struct pinbox *parent;     /* pointer back to softpin pinbox        */
    INT    *restrict1        ;  /* softpin is restricted to these sides  */
    INT    hierarchy        ;  /* type of pin root leaf, subroot etc    */
    INT    side             ;  /* current side of the softpin           */
    BOOL   permute          ;  /* true if rank ordered                  */
    BOOL   ordered          ;  /* true if ordered in a group            */
    BOOL   fixed            ;  /* true if rank is fixed                 */
    INT    lo_pos           ;  /* low valid position                    */
    INT    hi_pos           ;  /* hi valid position                     */
    FLOAT  lowerbound       ;  /* bounds of a pingroup                  */
    FLOAT  upperbound       ;  /* upper bound of a pin group            */
} SOFTBOX, *SOFTBOXPTR ;

typedef struct pinbox {
    struct pinbox *next    ; /*       next pin for this net            */
    struct pinbox *nextpin ; /*       next pin for this cell           */
    SOFTBOXPTR    softinfo ; /* soft pin info if appl. otherwise NULL  */
    char *pinname          ; /*               pin record               */
    INT pin                ; /*          global index of pin           */
    INT net                ; /*          global index of net           */
    INT cell               ; /*          global index of cell          */
    INT xpos               ; /*           global x position            */
    INT ypos               ; /*           global y position            */
    INT instance           ; /*      current cell instance             */
    INT txpos_new          ; /* proposed cell relative position of pin */
    INT typos_new          ; /* proposed cell relative position of pin */
    INT txpos              ; /* current cell relative position of pin  */
    INT typos              ; /* current cell relative position of pin  */
    INT *txpos_orig        ; /* orig array of cell inst xpos. cell rel.*/
    INT *typos_orig        ; /* orig array of cell inst ypos. cell rel.*/
    INT newx               ; /* scratch for determining new global xpos*/
    INT newy               ; /* scratch for determining new global ypos*/
    INT flag               ;
    INT skip               ;
    INT layer              ; /*        layer information               */
    INT type               ;  /* softpin, hardpin, equiv pin           */
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
    INT left       ;   /* current tile sides */
    INT right      ;
    INT bottom     ;
    INT top        ;
    INT orig_left      ;/* original tile sides specified by user */
    INT orig_right     ;/* for hard cell left = orig_left, that is */
    INT orig_bottom    ;/* sides don't change throughout run. But for */
    INT orig_top       ;/* soft cells use this to calculate current side*/
    struct tilebox *next ; /* next tile in list */
} *TILEBOXPTR , TILEBOX ;

typedef struct psidebox {
    INT length   ;
    DOUBLE pincount;
    INT vertical ;
    INT position ;
} PSIDEBOX ;

typedef struct {
    INT   r ;
    INT   l ;
    INT   t ;
    INT   b ;
} BOUNBOX, *BOUNBOXPTR ;

typedef struct {
    INT   *x ;                /* the current positions of vertices */
    INT   *x_orig ;           /* the original positions of vertices */
    INT   *x_new ;            /* the proposed positions of the vertices */
    INT   *y ;                /* the current positions of vertices */
    INT   *y_orig ;           /* the original positions of vertices */
    INT   *y_new ;            /* the proposed positions of the vertices */
    FLOAT *numpins;           /* number of pins on each side */
} VERTBOX, *VERTBOXPTR ;

typedef struct {
    INT  fixedType ;  /* POINT, NEIGHBORHOOD, GROUP, FIXEDGROUP */
    INT   xcenter ;
    INT   ycenter ;
    BOOL  leftNotRight ;
    BOOL  bottomNotTop ;
    INT   xloc1 ;              /* for fixing cell within a neighborhood */
    INT   yloc1 ;              /* remembers relative position to core */
    INT   xloc2 ;             
    INT   yloc2 ;
    BOOL  leftNotRight2 ;
    BOOL  bottomNotTop2 ;
    INT   x1, y1 ;             /* global position */
    INT   x2, y2 ;
    INT   xspan ;             /* for speed of calculation */
    INT   yspan ;
} FIXEDBOX, *FIXEDBOXPTR ;

typedef struct pad_rec {
    INT    cellnum          ;  /* index in carray */
    INT    length           ;
    INT    height           ;
    INT    position         ;  /* linear placement position for a side */
    INT    tiebreak         ;  /* ideal location to place the pad */
    INT    *children        ;  /* added for pad code */
    INT    padside          ;  /* current side */
    INT    padtype          ;  /* type of pad - pad or padgroup */
    INT    hierarchy        ;  /* type of pad - root, lead, pad, etc */
    INT    lo_pos           ;  /* low valid position -1 if every valid */
    INT    hi_pos           ;  /* hi valid position PINFINITY if valid */
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
    INT numinstances        ;  /* number of instances for this cell     */
    INT *numtile_inst       ;  /* number of tiles for each instance     */
    INT *numsides           ;  /* array of number of sides for inst.    */
    TILEBOXPTR *tile_inst   ;  /* array of tile lists for each inst     */
    BOUNBOXPTR **bounBox    ;  /* the bbox in each view 0 each inst     */
    VERTBOXPTR *vert_inst   ;  /* array of vertices of cell orient 0    */
} INSTBOX, *INSTBOXPTR ;

typedef struct cellbox { 
    char *cname             ;
    INT cellnum             ;  
    INT class               ;
    INT xcenter             ;
    INT ycenter             ;
    INT orientList[9]       ;
    INT orient              ;
    INT numpins             ;  /* number of pins for this cell          */
    INT numtiles            ;  /* number of tiles in current instance   */
    INT numsides            ;  /* number of sides for the current inst  */
    INT cur_inst            ;  /* current instance of cell [0,numinst]  */
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
    INT skip                ;
    INT xmin                ;
    INT newxmin             ;
    INT xmax                ;
    INT newxmax             ;
    INT ymin                ;
    INT nflag               ;
    INT newymin             ;
    INT ymax                ;
    INT newymax             ;
    INT numpins             ;
    INT Bnum                ;  /* no. of pins on the bottom edge of bb */
    INT Lnum                ;  /* no. of pins on the left edge of bb */
    INT Rnum                ;  /* no. of pins on the right edge of bb */
    INT Tnum                ;  /* no. of pins on the top edge of bb */
    INT newBnum             ;  /* no. of pins on the bottom edge of bb */
    INT newLnum             ;  /* no. of pins on the left edge of bb */
    INT newRnum             ;  /* no. of pins on the right edge of bb */
    INT newTnum             ;  /* no. of pins on the top edge of bb */
    INT newhalfPx           ;  /* new half perimeter bounding box for x */
    INT newhalfPy           ;  /* new half perimeter bounding box for y */
    INT halfPx              ;  /* current half perimeter bounding box for x */
    INT halfPy              ;  /* current half perimeter bounding box for y */
    FLOAT driveFactor       ;  /* driver strength of this net */
    FLOAT max_driver        ;  /* largest driver on this net */
    FLOAT min_driver        ;  /* smallest driver on this net */
    ANETPTR analog_info     ;  /* for analog nets */
} *NETBOXPTR , NETBOX ;

typedef struct kbox {
    INT cap ;
    INT HV ;
    INT sp ;
    INT x ;
    INT y ;
} KBOX , *KBOXPTR ;

typedef struct pathbox {
    INT lo_path_len ;     /* lower bound on the calculated half perim */
    INT hi_path_len ;     /* upper bound on the calculated half perim */
    INT new_lo_path_len ; /* new low path */
    INT new_hi_path_len ; /* new upper path */
    INT priority ;
    INT upper_bound ;
    INT lower_bound ;
    GLISTPTR nets ;
    struct pathbox *next ;  /* build a list first then array for speed */
} PATHBOX , *PATHPTR ;

typedef struct binbox {
    INT left ;
    INT right ;
    INT top ;
    INT bottom ;
    INT penalty ;
    INT nupenalty ;
    INT *cells ;   /* array of cells in bin */
    INT space ;    /* size of binbox->cells array */
} BINBOX ,
*BINBOXPTR ;

typedef struct psetrec {
    INT  member; /* integer for determining membership */
    INT  path ;  /* data */
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
EXTERN INT **net_cap_matchG ;         /* set of match nets */
EXTERN INT **net_res_matchG ;         /* set of match nets */

/* ----------------------------------------------------------
   The global variable definitions for Jimmy's controller.
---------------------------------------------------------- */
EXTERN INT d_costG;
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
EXTERN INT *newCellListG ;  /* *** bin cell list *** */ 
EXTERN INT TrybinG ;        /* used in setBin calculations */ 

/* ----------------------------------------------------------
   global definitions for configuration calculations
---------------------------------------------------------- */
EXTERN INT bdxlengthG , bdylengthG ;
EXTERN INT blocklG , blockrG , blocktG , blockbG ;
EXTERN INT blockmxG , blockmyG ;
EXTERN INT halfXspanG , halfYspanG ;
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
EXTERN INT numcellsG  ;
EXTERN INT numpadsG   ;
EXTERN INT totalpadsG ;
EXTERN INT numsoftG   ;
EXTERN INT numstdcellG ;
EXTERN INT numpadgroupsG ;
EXTERN INT numinstancesG ;
EXTERN INT numsupercellsG  ;
EXTERN INT numpathsG  ;
EXTERN INT endpadsG ;
EXTERN INT endpadgrpsG ;
EXTERN INT endsuperG ;
EXTERN INT totalcellsG;
EXTERN INT activecellsG;  /* number of active cells not fixed or merged */
EXTERN INT numnetsG ;
EXTERN INT numpinsG ;
EXTERN INT maxBinXG   ;
EXTERN INT maxBinYG   ;
EXTERN INT binWidthXG ;
EXTERN INT binWidthYG ;
EXTERN INT binXOffstG ;
EXTERN INT binYOffstG ;
EXTERN INT maxWeightG ;
EXTERN INT baseWeightG;
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
EXTERN INT penaltyG   ;
EXTERN INT binpenalG  ;
EXTERN INT funccostG  ;
EXTERN INT offsetG    ;
EXTERN INT timingcostG ;
EXTERN INT timingpenalG ;
EXTERN DOUBLE coreFactorG ;
EXTERN DOUBLE lapFactorG ;
EXTERN DOUBLE timeFactorG ;
EXTERN DOUBLE vertical_wire_weightG ;
EXTERN DOUBLE vertical_path_weightG ;

/* annealing stuff */
EXTERN DOUBLE TG ;
EXTERN FILE *fpoG ;
EXTERN INT randVarG ;
EXTERN INT attmaxG  ;
EXTERN INT iterationG ;

EXTERN DOUBLE pinsPerLenG ;
EXTERN INT layersFactorG ;
EXTERN INT unique_classG ;

/* ----------------------------------------------------------
   global definitions for calculated statistics
---------------------------------------------------------- */
EXTERN INT perimG ;
EXTERN INT totChanLenG ;
EXTERN INT totNetLenG  ;
EXTERN DOUBLE aveCellSideG ;
EXTERN DOUBLE expandExtraG ;

EXTERN INT flipsG ;
EXTERN DOUBLE avg_funcG , avgsG ;

EXTERN INT iwireG, iwirexG , iwireyG , icostG ;
EXTERN INT fwireG, fwirexG , fwireyG , fcostG ;

/* control for pad code */
EXTERN INT padspacingG ;  /* may be UNIFORM, VARIABLE, or ABUT */
EXTERN BOOL contiguousG ;
EXTERN BOOL external_pad_programG ; /* TRUE for calling placepads */
EXTERN INT min_pad_spacingG ;
EXTERN INT coreG[2][2] ;
EXTERN INT perdimG[2] ;
EXTERN PADBOXPTR *padarrayG ;   /* array of all the pads and padgroups */
EXTERN PADBOXPTR *sortarrayG ;  /* the sorted array of pads */
EXTERN PADBOXPTR *placearrayG ; /* array where the pads will be placed */

/* control gridding of cells */
EXTERN BOOL gridGivenG ;
EXTERN BOOL gridCellsG ;
EXTERN BOOL coreGivenG ;
EXTERN INT  x_originG, y_originG ;


#undef EXTERN


#endif /* CUSTOM_H */
