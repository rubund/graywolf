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

void fixCell( INT fixedType, INT xloc, char *lorR, INT yloc, char *borT, INT xloc2, char *lorR2, INT yloc2, char *borT2 );

void addClass( INT class );

void addCell( char * cellName, CELLTYPE cellType );

void endCell();

void addOrient( INT orient );

#endif /* INITIALIZE_H */
