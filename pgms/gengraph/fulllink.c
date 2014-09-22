/* ----------------------------------------------------------------- 
FILE:	    fulllink.c                                       
DESCRIPTION:link the edges.
CONTENTS:   
	    fulllink()
	    makeVertTree()
	    makeHoriTree()
	    makeVertDownTree()
	    makeHoriRiteTree()
DATE:	    Mar  3, 1990 
REVISIONS:  Mar  3, 1990 - Now a standalone program.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) fulllink.c version 1.1 3/3/90" ;
#endif

#include <geo.h>
#include <yalecad/debug.h>

#define EXPECTNUMDLINKS  100

fulllink()
{

TNODEPTR junkptr ;
DLINK1PTR ptr , pptr , nptr ;
int location , index ;

HlistG = (DLINK1PTR) Ysafe_malloc( sizeof( DLINK1 ) ) ;
HlistG->edge = edgeCountG ;
HlistG->next = (DLINK1PTR) Ysafe_malloc( sizeof( DLINK1 ) ) ;
HlistG->prev = (DLINK1PTR) NULL ;
HlistG->next->next = (DLINK1PTR) NULL ;
HlistG->next->prev = HlistG ;
HlistG->next->edge = edgeCountG - 2 ;
HendG = HlistG->next ;

VlistG = (DLINK1PTR) Ysafe_malloc( sizeof( DLINK1 ) ) ;
VlistG->edge = edgeCountG - 3 ;
VlistG->next = (DLINK1PTR) Ysafe_malloc( sizeof( DLINK1 ) ) ;
VlistG->prev = (DLINK1PTR) NULL ;
VlistG->next->next = (DLINK1PTR) NULL ;
VlistG->next->prev = VlistG ;
VlistG->next->edge = edgeCountG - 1 ;
VendG = VlistG->next ;

pptr = HlistG ;
nptr = HlistG->next ;
for( ; ; ) {
    tpop( &hEdgeRootG , &junkptr , &location , &index ) ;
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

pptr = VlistG ;
nptr = VlistG->next ;
for( ; ; ) {
    tpop( &vEdgeRootG , &junkptr , &location , &index ) ;
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

makeVertDownTree() ;
makeHoriRiteTree() ;
makeVertTree()     ;
makeHoriTree()     ;

return ;
}

makeVertTree()
{

DLINK1PTR vptr ;
int last , edge , count ;

VrootG = (TNODEPTR) NULL ;
VptrsG = (DLINK1PTR *) Ysafe_malloc( EXPECTNUMDLINKS*sizeof(DLINK1PTR) ) ;
count = 0 ;

last = INT_MIN ;
for( vptr = VlistG ; vptr != (DLINK1PTR) NULL ; vptr = vptr->next ) {
    edge = vptr->edge ;
    if( edgeListG[edge].loc > last ) {
	last = edgeListG[edge].loc ;
	if( ++count % EXPECTNUMDLINKS == 0 ) {
	    VptrsG = (DLINK1PTR *) Ysafe_realloc( VptrsG ,
		(count + EXPECTNUMDLINKS) * sizeof(DLINK1PTR) ) ;
	}
	VptrsG[count] = vptr ;
	tinsert( &VrootG , last , count ) ;
    }
}

return ;
}




makeHoriTree()
{

DLINK1PTR hptr ;
int last , edge , count ;

HrootG = (TNODEPTR) NULL ;
HptrsG = (DLINK1PTR *) Ysafe_malloc( EXPECTNUMDLINKS*sizeof(DLINK1PTR)) ;
count = 0 ;

last = INT_MIN ;
for( hptr = HlistG ; hptr != (DLINK1PTR) NULL ; hptr = hptr->next ) {
    edge = hptr->edge ;
    if( edgeListG[edge].loc > last ) {
	last = edgeListG[edge].loc ;
	if( ++count % EXPECTNUMDLINKS == 0 ) {
	    HptrsG = (DLINK1PTR *) Ysafe_realloc( HptrsG ,
		(count + EXPECTNUMDLINKS) * sizeof(DLINK1PTR) ) ;
	}
	HptrsG[count] = hptr ;
	tinsert( &HrootG , last , count ) ;
    }
}

return ;
}

makeVertDownTree()
{

DLINK1PTR vptr ;
int last , edge , count ;

VDrootG = (TNODEPTR) NULL ;
VDptrsG = (DLINK1PTR *) Ysafe_malloc( EXPECTNUMDLINKS*sizeof(DLINK1PTR));
count = 0 ;

last = INT_MIN ;
for( vptr = VlistG ; vptr != (DLINK1PTR) NULL ; vptr = vptr->next ) {
    edge = vptr->edge ;
    if( edgeListG[edge].UorR > 0 ) {
	continue ;
    }
    if( edgeListG[edge].loc > last ) {
	last = edgeListG[edge].loc ;
	if( ++count % EXPECTNUMDLINKS == 0 ) {
	    VDptrsG = (DLINK1PTR *) Ysafe_realloc( VDptrsG ,
		    (count + EXPECTNUMDLINKS) * sizeof(DLINK1PTR) ) ;
	}
	VDptrsG[count] = vptr ;
	tinsert( &VDrootG , last , count ) ;
    }
}

return ;
}



makeHoriRiteTree()
{

DLINK1PTR hptr ;
int last , edge , count ;

HRrootG = (TNODEPTR) NULL ;
HRptrsG = (DLINK1PTR *) Ysafe_malloc( EXPECTNUMDLINKS*sizeof(DLINK1PTR)) ;
count = 0 ;

last = INT_MIN ;
for( hptr = HlistG ; hptr != (DLINK1PTR) NULL ; hptr = hptr->next ) {
    edge = hptr->edge ;
    if( edgeListG[edge].UorR < 0 ) {
	continue ;
    }
    if( edgeListG[edge].loc > last ) {
	last = edgeListG[edge].loc ;
	if( ++count % EXPECTNUMDLINKS == 0 ) {
	    HRptrsG = (DLINK1PTR *) Ysafe_realloc( HRptrsG ,
		    (count + EXPECTNUMDLINKS) * sizeof(DLINK1PTR) ) ;
	}
	HRptrsG[count] = hptr ;
	tinsert( &HRrootG , last , count ) ;
    }
}

return ;
}
