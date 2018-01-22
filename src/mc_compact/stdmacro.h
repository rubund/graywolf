#ifndef STDMACROS_H
#define STDMACROS_H
/* --------------------------------------------------------------- 
  state   input    valid   next state
    H1     H        yes       H1
    H1     S        yes       S1
    S1     H        yes       H2
    S1     S        yes       S1
    H2     H        yes       H2
    H2     S        no        --
--------------------------------------------------------------- */
#define E  0
#define H1 1
#define S1 2
#define H2 3

typedef struct {
    int         node ;           /* node number */
    int         from_left ;      /* distance from left edge of bar */
    int         from_right ;     /* distance from right edge of bar */
    int         left_neighbor ;  /* left swapping neighbor */
    int         right_neighbor ; /* right swapping neighbor */
    BOOL        type ;           /* tile type */
    BOOL        candidate ;      /* whether it is an exchange candidate */
    COMPACTPTR  ptr ;            /* pointer to tile record */
} SELECTBOX, *SELECTPTR ;

void partition_compact();
#endif
