/* ----------------------------------------------------------------- 
FILE:	    makelink.c                                       
DESCRIPTION:Link the edges.
CONTENTS:   makelink()
	    makeVtree()
	    makeHtree()
DATE:	    Mar  3, 1990 
REVISIONS:  Mar  3, 1990 - Now a standalone program.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) makelink.c version 1.1 3/3/90" ;
#endif

#include <geo.h>
#include <yalecad/debug.h>

makelink()
{

TNODEPTR junkptr ;
DLINK1PTR ptr , pptr , nptr ;
int location , index ;

hFixedListG = (DLINK1PTR) Ysafe_malloc( sizeof( DLINK1 ) ) ;
hFixedListG->edge = edgeCountG ;
hFixedListG->next = (DLINK1PTR) Ysafe_malloc( sizeof( DLINK1 ) ) ;
hFixedListG->prev = (DLINK1PTR) NULL ;
hFixedListG->next->next = (DLINK1PTR) NULL ;
hFixedListG->next->prev = hFixedListG ;
hFixedListG->next->edge = edgeCountG - 2 ;
hFixedEndG = hFixedListG->next ;

vFixedListG = (DLINK1PTR) Ysafe_malloc( sizeof( DLINK1 ) ) ;
vFixedListG->edge = edgeCountG - 3 ;
vFixedListG->next = (DLINK1PTR) Ysafe_malloc( sizeof( DLINK1 ) ) ;
vFixedListG->prev = (DLINK1PTR) NULL ;
vFixedListG->next->next = (DLINK1PTR) NULL ;
vFixedListG->next->prev = vFixedListG ;
vFixedListG->next->edge = edgeCountG - 1 ;
vFixedEndG = vFixedListG->next ;

pptr = hFixedListG ;
nptr = hFixedListG->next ;
for( ; ; ) {
    tpop( &hFixedEdgeRootG , &junkptr , &location , &index ) ;
    if( junkptr == (TNODEPTR) NULL ) {
	break ;
    }
    ptr = (DLINK1PTR) Ysafe_malloc( sizeof( DLINK1 ) ) ;
    ptr->edge = index ;
    ptr->prev = pptr ;
    ptr->next = nptr ;
    pptr->next = ptr ;
    nptr->prev = ptr ;
    pptr = ptr ;
}

pptr = vFixedListG ;
nptr = vFixedListG->next ;
for( ; ; ) {
    tpop( &vFixedEdgeRootG , &junkptr , &location , &index ) ;
    if( junkptr == (TNODEPTR) NULL ) {
	break ;
    }
    ptr = (DLINK1PTR) Ysafe_malloc( sizeof( DLINK1 ) ) ;
    ptr->edge = index ;
    ptr->prev = pptr ;
    ptr->next = nptr ;
    pptr->next = ptr ;
    nptr->prev = ptr ;
    pptr = ptr ;
}
makeVtree() ;
makeHtree() ;

return ;
}




makeVtree()
{

DLINK1PTR vptr ;
int last , edge , count ;

VrootG = (TNODEPTR) NULL ;
VptrsG = (DLINK1PTR *) Ysafe_malloc( 100 * sizeof(DLINK1PTR) ) ;
count = 0 ;

last = INT_MIN ;
for( vptr = vFixedListG ; vptr != (DLINK1PTR) NULL ; vptr = vptr->next ) {
    edge = vptr->edge ;
    if( edgeListG[edge].loc > last ) {
	last = edgeListG[edge].loc ;
	if( ++count % 100 == 0 ) {
	    VptrsG = (DLINK1PTR *) Ysafe_realloc( VptrsG ,
				(count + 100) * sizeof(DLINK1PTR) ) ;
	}
	VptrsG[count] = vptr ;
	tinsert( &VrootG , last , count ) ;
    }
}

return ;
}




makeHtree()
{

DLINK1PTR hptr ;
int last , edge , count ;

HrootG = (TNODEPTR) NULL ;
HptrsG = (DLINK1PTR *) Ysafe_malloc( 100 * sizeof(DLINK1PTR) ) ;
count = 0 ;

last = INT_MIN ;
for( hptr = hFixedListG ; hptr != (DLINK1PTR) NULL ; hptr = hptr->next ) {
    edge = hptr->edge ;
    if( edgeListG[edge].loc > last ) {
	last = edgeListG[edge].loc ;
	if( ++count % 100 == 0 ) {
	    HptrsG = (DLINK1PTR *) Ysafe_realloc( HptrsG ,
				(count + 100) * sizeof(DLINK1PTR) ) ;
	}
	HptrsG[count] = hptr ;
	tinsert( &HrootG , last , count ) ;
    }
}

return ;
}
