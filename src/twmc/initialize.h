/* ----------------------------------------------------------------- 
FILE:	    initialize.h
DESCRIPTION:Header file for initialize.c
CONTENTS:   
DATE:	    March 15, 1990 
REVISIONS:  Wed Dec 19 19:38:46 EST 1990 - added analog pin type.
----------------------------------------------------------------- */
/* *****************************************************************
   static char SccsId[] = "@(#) initialize.h version 3.4 12/19/90" ;
***************************************************************** */
#ifndef INITIALIZE_H
#define INITIALIZE_H

/* set processing switch to avoid work when an error is found */
void setErrorFlag();
void initCellInfo();
void addCell( char *cellName, CELLTYPE cellType );
void endCell();
void fixCell( int fixedType, int xloc, char *lorR, int yloc, char *borT, int xloc2, char *lorR2, int yloc2, char *borT2 ); /* valid types - neighborhood. point, group */
void processCorners( int numcorners );
void addCorner( int xpos, int ypos );
void initializeCorner( int cell );
void addClass( int class );
void initOrient( int orient );
void addOrient( int orient );
void set_cur_orient(int orient );
void addAspectBounds( double lowerBound, double upperBound );
void addPin( char *pinName, char *signal, int layer, int pinType );
void set_pin_pos( int xpos, int ypos );
void check_pos( char *pinname, int xpos, int ypos );
void addEquivPin( char *pinName, int layer, int xpos, int ypos, int pinType );
void set_restrict_type( int object );
void addSideRestriction( int side );
void add_pinspace( double lower, double upper );
void add_soft_array();
void start_pin_group( char *pingroup, BOOL permute );
void add2pingroup( char *pinName, BOOL ordered );
void addSideSpace( double lower, double upper );
void addPadSide( char *side );
void setPermutation( int permuteFlag );
void add2padgroup( char *padName, BOOL ordered );
void add_cell_to_group( char *cellName );
void add_instance( char *instName );
void add_analog( int numcorners );
void add_pin_contour( int x, int y );
void add_current( float current );
void add_power( float power );
void no_layer_change();
void process_pin();
int cleanupReadCells();
int get_tile_count();

#endif /* INITIALIZE_H */
