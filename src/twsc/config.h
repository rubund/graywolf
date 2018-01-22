/* ----------------------------------------------------------------- 
"@(#) config.h (Yale) version 4.4 2/17/91"
FILE:	    config.h                                       
DESCRIPTION:TimberwolfSC insert file for configuration parameters.
CONTENTS:   
DATE:	    Mar 27, 1989 
REVISIONS:  Sun Feb 17 21:11:18 EST 1991 - moved numMacroG to pad.h
----------------------------------------------------------------- */
#ifndef CONFIG_H
#define CONFIG_H

#ifdef CONFIG_VARS
#define EXTERN 
#else
#define EXTERN extern 
#endif

typedef struct macrobox {
    int mx ;
    int my ;
} MACROBOX ;

typedef struct rowbox {
    int startx  ;
    int endx    ;
    int endx1   ;
    int startx2 ;
    int ypos    ;
    int desiredL ;
} ROWBOX ;

EXTERN MACROBOX *macroArrayG ;
EXTERN ROWBOX *rowArrayG ;

EXTERN int max_blklengthG ;
EXTERN int route2actG;   /* added for placement adjusted for routing */
EXTERN int track_pitchG; /* used only if global routing is performed */
EXTERN int gridCellG;    /* used only if global routing is performed */
EXTERN int desiredLG ;
EXTERN DOUBLE *macspaceG ;

EXTERN int *padArrayG ;
EXTERN int extraSpaceG ;
EXTERN int coreHeightG ;
EXTERN int coreWidthG ;
EXTERN int separationG ;
EXTERN int spaceG ;
EXTERN int maxHeight ;
EXTERN int lastG ;

#undef EXTERN  

#endif /* CONFIG_H */
