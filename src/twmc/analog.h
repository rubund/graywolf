/* ----------------------------------------------------------------- 
FILE:	    analog.h                                       
CONTENTS:   definitions for analog information.
DATE:	    Dec 13, 1990 
REVISIONS:
----------------------------------------------------------------- */
/* *****************************************************************
   "@(#) analog.h (Yale) version 1.2 2/23/91"
***************************************************************** */
#ifndef ANALOG_H
#define ANALOG_H 

#define INIT_DROP              -1000000.0
#define NO_CONSTRAINT_ON_DROP   -100000.0
#define INIT_CURRENT           -1000000.0
#define NO_CURRENT_SPECIFIED    -100000.0
#define INIT_CAP                     -1.0
#define INIT_RES                     -1.0

/* analog net type */
#define SHIELDING_NET      1
#define SHIELDMASK       0x1
#define NOISY_NET          2
#define NOISYMASK        0x2
#define SENSITIVE_NET      4
#define SENSITIVEMASK    0x4

/* the circuit type */
#define NO_CKT_TYPE     0
#define DIGITAL       0x1
#define ANALOG        0x2
#define MIXED         0x3

typedef struct abox { 
    INT    *x_contour       ;  /* x coordinates of pin contour          */
    INT    *y_contour       ;  /* y coordinates of pin contour          */
    INT    num_corners      ;  /* number of vertices in pin contour     */
    FLOAT  current          ;  /* current for the pin                   */
    FLOAT  power            ;  /* power  for the pin                    */
    BOOL   no_layer_change  ;  /* whether layer change are allowed      */
} ANALOGBOX, *ANALOGPTR ;

typedef struct common_pt {
    INT num_pins        ;  /* number of pins in this common point */
    INT *common_set     ;  /* set of pins in the common point */
    INT *cap_match      ;  /* pins that must match cap. for each cpt.*/
    INT *res_match      ;  /* pins that must match res. for each cpt.*/
} COMMONBOX, *COMMONPTR ;

typedef struct analognet {
    DOUBLE cap_upper_bound  ;  /* capacitance upper bound */
    DOUBLE res_upper_bound  ;  /* resistance upper bound */
    INT net_type     	    ;  /* noisy, sensitive, shielding */
    INT num_common_points   ;  /* number of common points for the net */
    COMMONPTR  *common_pts  ;  /* array of the common points */
    DOUBLE max_drop         ;  /* max voltage drop */
} ANETBOX, *ANETPTR ;

#endif /* ANALOG_H */
