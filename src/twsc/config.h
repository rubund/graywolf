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

extern MACROBOX *macroArrayG ;
extern ROWBOX *rowArrayG ;

extern int max_blklengthG ;
extern int route2actG;   /* added for placement adjusted for routing */
extern int track_pitchG; /* used only if global routing is performed */
extern int gridCellG;    /* used only if global routing is performed */
extern int desiredLG ;
extern double *macspaceG ;

extern int *padArrayG ;
extern int extraSpaceG ;
extern int coreHeightG ;
extern int coreWidthG ;
extern int separationG ;
extern int spaceG ;
extern int maxHeight ;
extern int lastG ;

#endif /* CONFIG_H */
