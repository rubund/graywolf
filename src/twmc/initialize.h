/* ----------------------------------------------------------------- 
FILE:	    initialize.h
DESCRIPTION:Header file for initialize.c
CONTENTS:   
DATE:	    March 15, 1990 
REVISIONS:  Wed Dec 19 19:38:46 EST 1990 - added analog pin type.
----------------------------------------------------------------- */
#ifndef INITIALIZE_H
#define INITIALIZE_H

#define PINGROUPTYPE     1
#define HARDPINTYPE      2
#define SOFTPINTYPE      3
#define SOFTEQUIVTYPE    4
#define HARDEQUIVTYPE    5
#define ADDEQUIVTYPE     6
#define ANALOGPINTYPE    7

/* called from readcell.c */
void fixCell( INT fixedType, INT xloc, char *lorR, INT yloc, char *borT, INT xloc2, char *lorR2, INT yloc2, char *borT2 );
void addClass( INT class );
void addCell( char * cellName, CELLTYPE cellType );
void endCell(void);
void addOrient( INT orient );
void setPermutation( int permuteFlag );
void processCorners( INT numcorners );
void addCorner( INT xpos, INT ypos );
void initOrient(INT orient);
void set_cur_orient( INT orient);
void addAspectBounds(DOUBLE lowerBound, DOUBLE upperBound);
void add_soft_array(void);
void process_pin(void);
void addPin(char *pinName, char *signal, INT layer, INT pinType);
void set_pin_pos(INT xpos, INT ypos);
void add_analog(INT numcorners);
void add_pin_contour(INT x, INT y);
void add_current(FLOAT current);
void add_power(FLOAT current);
void no_layer_change(void);
void set_restrict_type(INT object);
void addEquivPin(char *pinName, INT layer, INT xpos, INT ypos, INT pinType);
void start_pin_group(char *pingroup, BOOL permute);
void add2pingroup(char *pinName, BOOL ordered);
void addSideRestriction(INT side);
void add_pinspace(DOUBLE lower, DOUBLE upper);
void addSideSpace(DOUBLE lower, DOUBLE upper);
void addPadSide(char *side);
void add2padgroup(char *padName, BOOL ordered);
void add_cell_to_group(char *cellName);

#endif /* INITIALIZE_H */
