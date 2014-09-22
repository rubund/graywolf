/* ----------------------------------------------------------------- 
FILE:	    ygraph.c                                       
DESCRIPTION:This file contains ygraph necessary for the polar graph.
CONTENTS:   ygraph()
DATE:	    Mar  3, 1990 
REVISIONS:  Mar  3, 1990 - Now a standalone program.
	    Apr 20, 1990 - fixed redundant edges when cells touch.
	    Wed Jun 12 22:10:09 CDT 1991 - fixed problem with add2eindex.
		Now add all the channels to xgraph and ygraph properly.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) ygraph.c version 1.3 6/12/91" ;
#endif

#include <geo.h>
#include <yalecad/debug.h>
#define HOWMANY 0

ygraph()
{

int i , hedge , location , eindex , er , el , r , l , cell , count ;
int beginEdge , iedge , inode , oedge , onode , lastE ;
WCPTR saveptr, yptr ;
DLINK1PTR hptr ;
PLISTPTR plptr ;
FLAREPTR fptr , ifptr , ofptr ;
TNODEPTR tnode ;


yNodulesG = (NODPTR) Ysafe_malloc( (2 + numberCellsG) * sizeof(NODBOX));
for( i = 0 ; i <= numberCellsG + 1 ; i++ ) {
    yNodulesG[i].inList = (FLAREPTR) NULL ;
    yNodulesG[i].outList = (FLAREPTR) NULL ;
    yNodulesG[i].done = 0 ;
}
hptr = HlistG ;
hedge = hptr->edge ;
location = edgeListG[hedge].loc ;
er = edgeListG[hedge].end ;
el = edgeListG[hedge].start ;
plptr = tplist( hChanBeginRootG , location ) ;
for( ; plptr != (PLISTPTR) NULL ; plptr = plptr->pnext ) {
    eindex = plptr->property ;
    r = eArrayG[eindex].hiend ;
    l = eArrayG[eindex].loend ;
    if( ! (l >= el && r <= er) ) {
	continue ;
    }
    fptr = yNodulesG[0].outList ;
    yNodulesG[0].outList = (FLAREPTR) Ysafe_malloc( sizeof(FLARE) ) ;
    yNodulesG[0].outList->next = fptr ;
    yNodulesG[0].outList->fixEdge = hedge ;
    yNodulesG[0].outList->eindex = (INT *) Ysafe_malloc( 2 * sizeof(INT) ) ; 
    yNodulesG[0].outList->eindex[HOWMANY] = 1 ; 
    yNodulesG[0].outList->eindex[1] = eindex ; 
    yNodulesG[0].done = 1 ; 
}
hptr = HendG ;
hedge = hptr->edge ;
location = edgeListG[hedge].loc ;
er = edgeListG[hedge].end ;
el = edgeListG[hedge].start ;
plptr = tplist( hChanEndRootG , location ) ;
for( ; plptr != (PLISTPTR) NULL ; plptr = plptr->pnext ) {
    eindex = plptr->property ;
    r = eArrayG[eindex].hiend ;
    l = eArrayG[eindex].loend ;
    if( ! (l >= el && r <= er) ) {
	continue ;
    }
    fptr = yNodulesG[numberCellsG + 1].inList ;
    yNodulesG[numberCellsG + 1].inList = (FLAREPTR) Ysafe_malloc(sizeof(FLARE));
    yNodulesG[numberCellsG + 1].inList->next = fptr ;
    yNodulesG[numberCellsG + 1].inList->fixEdge = hedge ;
    yNodulesG[numberCellsG + 1].inList->eindex = (INT *) 
						Ysafe_malloc(2 * sizeof(INT)); 
    yNodulesG[numberCellsG + 1].inList->eindex[HOWMANY] = 1 ; 
    yNodulesG[numberCellsG + 1].inList->eindex[1] = eindex ; 
    yNodulesG[numberCellsG + 1].done = 1 ; 
}

hptr = hFixedListG->next ;
for( ; hptr->next != (DLINK1PTR) NULL ; hptr = hptr->next ) {
    hedge = hptr->edge ;
    cell = edgeListG[hedge].cell ;
    if( yNodulesG[cell].done == 1 ) {
	continue ;
    }
    yNodulesG[cell].done = 1 ;
    beginEdge = hedge ;
    do {
	if( edgeListG[hedge].UorR < 0 ) {
	    location = edgeListG[hedge].loc ;
	    plptr = tplist( hChanEndRootG , location ) ;
	    for( ; plptr != (PLISTPTR) NULL ; plptr = plptr->pnext ) {
		eindex = plptr->property ;
		if( eArrayG[eindex].edge2 != hedge ) {
		    continue ;
		}
		fptr = yNodulesG[cell].inList ;
		yNodulesG[cell].inList = (FLAREPTR) Ysafe_malloc(sizeof(FLARE));
		yNodulesG[cell].inList->next = fptr ;
		yNodulesG[cell].inList->fixEdge = hedge ;
		yNodulesG[cell].inList->eindex = (INT *) 
						Ysafe_malloc(2 * sizeof(INT)); 
		yNodulesG[cell].inList->eindex[HOWMANY] = 1 ; 
		yNodulesG[cell].inList->eindex[1] = eindex ; 
	    }
	} else {
	    location = edgeListG[hedge].loc ;
	    plptr = tplist( hChanBeginRootG , location ) ;
	    for( ; plptr != (PLISTPTR) NULL ; plptr = plptr->pnext ) {
		eindex = plptr->property ;
		if( eArrayG[eindex].edge1 != hedge ) {
		    continue ;
		}
		fptr = yNodulesG[cell].outList ;
		yNodulesG[cell].outList = (FLAREPTR) Ysafe_malloc(sizeof(FLARE));
		yNodulesG[cell].outList->next = fptr ;
		yNodulesG[cell].outList->fixEdge = hedge ;
		yNodulesG[cell].outList->eindex = (INT *) 
						Ysafe_malloc(2 * sizeof(INT)); 
		yNodulesG[cell].outList->eindex[HOWMANY] = 1 ; 
		yNodulesG[cell].outList->eindex[1] = eindex ; 
	    }
	}
	hedge = edgeListG[hedge].nextEdge ;
	hedge = edgeListG[hedge].nextEdge ;
    } while( hedge != beginEdge ) ;
}

for( cell = 0 ; cell <= numberCellsG + 1 ; cell++ ) {
    fptr = yNodulesG[cell].inList ;
    for( ; fptr != (FLAREPTR) NULL ; fptr = fptr->next ) {
	for( count = fptr->eindex[HOWMANY]; count <= fptr->eindex[HOWMANY]; count++){
	    eindex = fptr->eindex[count] ;
	    location = eArrayG[eindex].lbside ;
	    er = eArrayG[eindex].hiend ;
	    el = eArrayG[eindex].loend ;
	    plptr = tplist( hChanEndRootG , location ) ;
	    for( ; plptr != (PLISTPTR) NULL ; plptr = plptr->pnext ) {
		eindex = plptr->property ;
		r = eArrayG[eindex].hiend ;
		l = eArrayG[eindex].loend ;
		if( ! (l >= el && r <= er) ) {
		    continue ;
		}
		tdelete( &hChanEndRootG , location , eindex ) ;
		if( add2eindex( fptr, eindex )){
		    count-- ;
		}
		break ;
	    }
	}
    }
    fptr = yNodulesG[cell].outList ;
    for( ; fptr != (FLAREPTR) NULL ; fptr = fptr->next ) {
	for( count = fptr->eindex[HOWMANY]; count <= fptr->eindex[HOWMANY]; count++){
	    eindex = fptr->eindex[count] ;
	    location = eArrayG[eindex].rtside ;
	    er = eArrayG[eindex].hiend ;
	    el = eArrayG[eindex].loend ;
	    plptr = tplist( hChanBeginRootG , location ) ;
	    for( ; plptr != (PLISTPTR) NULL ; plptr = plptr->pnext ) {
		eindex = plptr->property ;
		r = eArrayG[eindex].hiend ;
		l = eArrayG[eindex].loend ;
		if( ! (l >= el && r <= er) ) {
		    continue ;
		}
		tdelete( &hChanBeginRootG , location , eindex ) ;
		if( add2eindex( fptr, eindex )){
		    count-- ;
		}
		break ;
	    }
	}
    }
}

numYnodesG = 0 ;
for( hptr = hFixedListG; hptr != (DLINK1PTR) NULL; hptr = hptr->next ){
    edgeListG[ hptr->edge ].node = ++numYnodesG ;
}
yNodeArrayG = (WCPTR *) Ysafe_malloc( (1 + numYnodesG) * sizeof(WCPTR) );
for( i = 1 ; i <= numYnodesG ; i++ ) {
    yNodeArrayG[i] = (WCPTR) NULL ;
}
for( cell = 1 ; cell <= numberCellsG ; cell++ ) {
    ifptr = yNodulesG[cell].inList ;
    for( ; ifptr != (FLAREPTR) NULL ; ifptr = ifptr->next ) {
	iedge = ifptr->fixEdge ;
	inode = edgeListG[iedge].node ;
	ofptr = yNodulesG[cell].outList ;
	for( ; ofptr != (FLAREPTR) NULL ; ofptr = ofptr->next ) {
	    oedge = ofptr->fixEdge ;
	    onode = edgeListG[oedge].node ;
	    saveptr = yNodeArrayG[inode] ;
	    yptr = yNodeArrayG[inode] = (WCPTR) Ysafe_malloc( sizeof(WCBOX));
	    yptr->next = saveptr ;
	    yptr->fixedWidth = 1 ;
	    yptr->node = onode ;
	    yptr->channels = (INT *) NULL ;
	    yptr->length = edgeListG[oedge].loc - edgeListG[iedge].loc ;
	}
    }
}
for( cell = 0 ; cell <= numberCellsG + 1 ; cell++ ) {
    ofptr = yNodulesG[cell].outList ;
    for( ; ofptr != (FLAREPTR) NULL ; ofptr = ofptr->next ) {
	iedge = ofptr->fixEdge ;
	inode = edgeListG[iedge].node ;

	lastE = ofptr->eindex[ ofptr->eindex[HOWMANY] ] ;
	onode = edgeListG[ eArrayG[lastE].edge2 ].node ;

	saveptr = yNodeArrayG[inode] ;
	yptr = yNodeArrayG[inode] = (WCPTR) Ysafe_malloc( sizeof(WCBOX));
	yptr->next = saveptr ;
	yptr->fixedWidth = 0 ;
	yptr->length = 0 ;
	yptr->node = onode ;
	yptr->channels = (INT *) Ysafe_malloc( (1 + 
				ofptr->eindex[HOWMANY]) * sizeof(INT) );
	for( count = 0 ; count <= ofptr->eindex[HOWMANY] ; count++ ) {
	    yptr->channels[count] = ofptr->eindex[count] ;
	}
    }
}

if( hChanBeginRootG != (TNODEPTR) NULL ) {
    for( ; ; ) {
	tpop( &hChanBeginRootG , &tnode , &location , &eindex ) ;
	if( tnode == (TNODEPTR) NULL ) {
	    break ;
	}
    }
}
if( hChanEndRootG != (TNODEPTR) NULL ) {
    for( ; ; ) {
	tpop( &hChanEndRootG , &tnode , &location , &eindex ) ;
	if( tnode == (TNODEPTR) NULL ) {
	    break ;
	}
    }
}

return ;
}

/* add to the flare pointer only if we don't find a match */
/* degree of graph <= 4 brute force is simplest and fastest */
/* returns true if edge is redundant */
BOOL add2eindex( fptr, eindex )
FLAREPTR fptr ;
int eindex ;
{
    int i ;                   /* counter */
    int howmany ;             /* howmany edges so far */

    howmany = fptr->eindex[HOWMANY] ;
    for( i = 1; i <= howmany; i++ ){
	if( fptr->eindex[i] == eindex ){
	    /* it is already in the list - avoid adding a redundant one*/
	    return( TRUE ) ;
	}
    }
    /* if we get to this point we haven't found a redundant edge */
    /* so add a new one */
    fptr->eindex[HOWMANY] = ++howmany ;
    if( howmany % 2 == 0 ) {
	fptr->eindex = (INT *) Ysafe_realloc( fptr->eindex ,
			(2 + howmany) * sizeof(INT) ) ;
    }
    fptr->eindex[ howmany ] = eindex ;
    return( FALSE ) ; /* new edge */
} /* end add2eindex */
