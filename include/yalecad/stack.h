/* ----------------------------------------------------------------- 
FILE:	    stack.h
DESCRIPTION:Macro descriptions for implementing simple stack operations.
CONTENTS:   
DATE:	    Dec  6, 1989 
REVISIONS:  
----------------------------------------------------------------- */
#ifndef lint
static char StackSccsId[] = "@(#) stack.h version 1.3 10/9/90" ;
#endif

#ifndef STACK_H

#define STACK_H

#define YSIZE_STACK 0 
#define YSTACK_POINT 1 
#include <yalecad/message.h>

#define YINITSTACK( stack_xyz, size_xyz ) \
{   \
    stack_xyz = (INT *) Ysafe_malloc( (size_xyz+2) * sizeof(int) ) ; \
    stack_xyz[YSIZE_STACK] = size_xyz + 1 ; \
    stack_xyz[YSTACK_POINT] = 1 ; \
}

#define YCLEARSTACK( stack_xyz ) \
{   \
    stack_xyz[YSTACK_POINT] = 1 ; \
}

#define YPOPSTACK( stack_xyz ) \
 ( stack_xyz[YSTACK_POINT] > YSTACK_POINT ? stack_xyz[stack_xyz[YSTACK_POINT]--] : 0 )

#define YPUSHSTACK( stack_xyz, data_xyz ) \
{   \
    if( ++stack_xyz[YSTACK_POINT] <= stack_xyz[YSIZE_STACK] ){ \
	stack_xyz[ stack_xyz[YSTACK_POINT] ] = data_xyz ; \
    } else { \
	M( ERRMSG, "YPUSHSTACK", "stack_overflow\n" ) ; \
    } \
}

#endif /* STACK_H */
