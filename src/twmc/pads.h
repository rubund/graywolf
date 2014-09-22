/* *****************************************************************
   static char SccsId[] = "@(#) pads.h version 3.6 2/23/91" ;
***************************************************************** */
#ifndef PADS_H
#define PADS_H 

/* control for pad spacing */
#define UNIFORM_PADS  0
#define VARIABLE_PADS 1
#define ABUT_PADS     2
#define EXACT_PADS    3

/* pad sides */
#define ALL     0
#define L       1
#define T       2
#define R       3
#define B       4

#define MINI  0
#define MAXI  1

#define X     0
#define Y     1

#define HOWMANY 0
#define LARGE 100000
#define PINFINITY INT_MAX / 8
#define PICK_INT(l,u) (((l)<(u)) ? ((RAND % ((u)-(l)+1))+(l)) : (l))

/* pad hierarchy */
#define NONE      0
#define LEAF      1
#define SUBROOT   2
#define ROOT      3

#endif /* PADS_H */
