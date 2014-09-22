/* ----------------------------------------------------------------- 
FILE:	    vprobes.c                                       
DESCRIPTION:Vertical probe code.
CONTENTS:   vprobes()
	    doubleDown( rptr ) 
		DLINK1PTR rptr ;
DATE:	    Mar  3, 1990 
REVISIONS:  Mar  3, 1990 - Now a standalone program.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) vprobes.c version 1.1 3/3/90" ;
#endif

#include <geo.h>
#include <yalecad/debug.h>

vprobes()
{

DLINK1PTR ritePtr , left1ptr , left2ptr , checkPtr , ptr ;
int ry , rx1 , rx2 , redge , dx1 , dx2 , ledge , edge , check ;
int l1x2 , l1x1 , l1y , l2x2 , l2x1 , l2y ;

ritePtr = hFixedListG ;
for( ; ritePtr != (DLINK1PTR) NULL ; ritePtr = ritePtr->next ) {
    redge = ritePtr->edge ;
    if( edgeListG[redge].UorR < 0 ) {
	continue ;
    }
    ry  = edgeListG[redge].loc   ;
    rx1 = edgeListG[redge].start ;
    rx2 = edgeListG[redge].end   ;
    
    left1ptr = HptrsG[ tprop( HrootG , ry ) ] ;
    for(; left1ptr != (DLINK1PTR)NULL; left1ptr = left1ptr->next){
	ledge = left1ptr->edge ;
	if( edgeListG[ledge].UorR > 0 ) {
	    continue ;
	}
	l1y  = edgeListG[ledge].loc   ;
	l1x1 = edgeListG[ledge].start ;
	l1x2 = edgeListG[ledge].end   ;
	if( l1x2 <= rx1 || l1x1 > rx1 ) {
	    continue ;
	}
	break ;
    }
    
    left2ptr = HptrsG[ tprop( HrootG , ry ) ] ;
    for(; left2ptr != (DLINK1PTR) NULL ; left2ptr = left2ptr->next){
	ledge = left2ptr->edge ;
	if( edgeListG[ledge].UorR > 0 ) {
	    continue ;
	}
	l2y  = edgeListG[ledge].loc   ;
	l2x1 = edgeListG[ledge].start ;
	l2x2 = edgeListG[ledge].end   ;
	if( l2x2 < rx2 || l2x1 >= rx2 ) {
	    continue ;
	}
	break ;
    }

    if( left1ptr != (DLINK1PTR) NULL && left1ptr == left2ptr ) {
	check = 1 ;
	checkPtr = HptrsG[ tprop( HrootG , ry ) ] ;
	for(; checkPtr != (DLINK1PTR)NULL; checkPtr = checkPtr->next){
	    if( edgeListG[ checkPtr->edge ].UorR > 0 ) {
		continue ;
	    }
	    if( edgeListG[ checkPtr->edge ].loc >= l2y ) {
		break ;
	    }
	    if( edgeListG[ checkPtr->edge ].start >= rx2 ||
			edgeListG[ checkPtr->edge ].end <= rx1 ) {
		continue ;
	    }
	    check = 0 ;
	    break ;
	}
	if( check ) {
	    edgeListG[++numProbesG + edgeCountG].start  = ry ;
	    edgeListG[numProbesG + edgeCountG].end      = l2y  ;
	    edgeListG[numProbesG + edgeCountG].loc      = rx1 ;
	    edgeListG[numProbesG + edgeCountG].length   = l2y - ry ;
	    edgeListG[numProbesG + edgeCountG].fixed    = 0 ;
	    edgeListG[numProbesG + edgeCountG].cell     = 0 ;
	    edgeListG[numProbesG + edgeCountG].UorR     = 1 ;
	    tinsert( &vEdgeRootG, rx1 , numProbesG + edgeCountG ) ;
#ifdef DEBUG
	    fprintf(fpdebugG,"vprobe:%d  ", numProbesG ) ;
	    fprintf(fpdebugG,"start:%d  end:%d  loc:%d  UorR:%d\n",
					ry , l2y , rx1 , 1 ) ;
#endif
	    edgeListG[++numProbesG + edgeCountG].start  = ry ;
	    edgeListG[numProbesG + edgeCountG].end      = l2y  ;
	    edgeListG[numProbesG + edgeCountG].loc      = rx2 ;
	    edgeListG[numProbesG + edgeCountG].length   = l2y - ry ;
	    edgeListG[numProbesG + edgeCountG].fixed    = 0 ;
	    edgeListG[numProbesG + edgeCountG].cell     = 0 ;
	    edgeListG[numProbesG + edgeCountG].UorR     = -1 ;
	    tinsert( &vEdgeRootG, rx2 , numProbesG + edgeCountG ) ;
#ifdef DEBUG
	    fprintf(fpdebugG,"vprobe:%d  ", numProbesG ) ;
	    fprintf(fpdebugG,"start:%d  end:%d  loc:%d  UorR:%d\n",
					ry , l2y , rx2 , -1 ) ;
#endif
	} else {
	    doubleDown( ritePtr ) ;	
	}
	continue ;
    }

    if( left1ptr != (DLINK1PTR) NULL ) {
	ptr = HptrsG[ tprop( HrootG , l1y ) ] ;
	for( ptr = ptr->next; ptr != (DLINK1PTR) NULL; ptr = ptr->next){
	    if( edgeListG[ptr->edge].loc > l1y ) {
		break ;
	    }
	}
	if( ptr == (DLINK1PTR) NULL ) {
	    ptr = hFixedEndG ;
	} else {
	    ptr = ptr->prev ;
	}
	for( ; ptr != (DLINK1PTR) NULL; ptr = ptr->prev){
	    edge = ptr->edge ;
	    if( edgeListG[edge].UorR < 0 ) {
		continue ;
	    }
	    dx1 = edgeListG[edge].start ;
	    dx2 = edgeListG[edge].end   ;
	    if( dx2 < l1x2 || dx1 >= l1x2 ) {
		continue ;
	    }
	    break ;
	}
	if( ritePtr == ptr ) {
	    check = 1 ;
	    checkPtr = HptrsG[ tprop( HrootG , ry ) ] ;
	    for(; checkPtr != (DLINK1PTR) NULL ;
				checkPtr = checkPtr->next ) {
		if( edgeListG[ checkPtr->edge ].UorR > 0 ) {
		    continue ;
		}
		if( edgeListG[ checkPtr->edge ].loc >= l1y ) {
		    break ;
		}
		if( edgeListG[ checkPtr->edge ].start >= l1x2 ||
			edgeListG[ checkPtr->edge ].end <= rx1 ) {
		    continue ;
		}
		check = 0 ;
		break ;
	    }
	    if( check ) {
		edgeListG[++numProbesG + edgeCountG].start  = ry ;
		edgeListG[numProbesG + edgeCountG].end      = l1y ;
		edgeListG[numProbesG + edgeCountG].loc      = rx1 ;
		edgeListG[numProbesG + edgeCountG].length   = l1y - ry ;
		edgeListG[numProbesG + edgeCountG].fixed    = 0 ;
		edgeListG[numProbesG + edgeCountG].cell     = 0 ;
		edgeListG[numProbesG + edgeCountG].UorR     = 1 ;
		tinsert( &vEdgeRootG, rx1 , numProbesG + edgeCountG ) ;
#ifdef DEBUG
		fprintf(fpdebugG,"vprobe:%d  ", numProbesG ) ;
		fprintf(fpdebugG,"start:%d  end:%d  loc:%d  UorR:%d\n",
					ry , l1y , rx1 , 1 ) ;
#endif
		edgeListG[++numProbesG + edgeCountG].start  = ry ;
		edgeListG[numProbesG + edgeCountG].end      = l1y  ;
		edgeListG[numProbesG + edgeCountG].loc      = l1x2 ;
		edgeListG[numProbesG + edgeCountG].length   = l1y - ry ;
		edgeListG[numProbesG + edgeCountG].fixed    = 0 ;
		edgeListG[numProbesG + edgeCountG].cell     = 0 ;
		edgeListG[numProbesG + edgeCountG].UorR     = -1 ;
		tinsert( &vEdgeRootG, l1x2 , numProbesG + edgeCountG ) ;
#ifdef DEBUG
		fprintf(fpdebugG,"vprobe:%d  ", numProbesG ) ;
		fprintf(fpdebugG,"start:%d  end:%d  loc:%d  UorR:%d\n",
					ry , l1y , l1x2 , -1 ) ;
#endif
	    }
	}
    }

    if( left2ptr != (DLINK1PTR) NULL ) {
	ptr = HptrsG[ tprop( HrootG , l2y ) ] ;
	for( ptr = ptr->next; ptr != (DLINK1PTR) NULL; ptr = ptr->next){
	    if( edgeListG[ptr->edge].loc > l2y ) {
		break ;
	    }
	}
	if( ptr == (DLINK1PTR) NULL ) {
	    ptr = hFixedEndG ;
	} else {
	    ptr = ptr->prev ;
	}
	for( ; ptr != (DLINK1PTR) NULL; ptr = ptr->prev){
	    edge = ptr->edge ;
	    if( edgeListG[edge].UorR < 0 ) {
		continue ;
	    }
	    dx1 = edgeListG[edge].start ;
	    dx2 = edgeListG[edge].end   ;
	    if( dx2 <= l2x1 || dx1 > l2x1 ) {
		continue ;
	    }
	    break ;
	}
	if( ritePtr == ptr ) {
	    check = 1 ;
	    checkPtr = HptrsG[ tprop( HrootG , ry ) ] ;
	    for(; checkPtr != (DLINK1PTR) NULL ;
				    checkPtr = checkPtr->next ) {
		if( edgeListG[ checkPtr->edge ].UorR > 0 ) {
		    continue ;
		}
		if( edgeListG[ checkPtr->edge ].loc >= l2y ) {
		    break ;
		}
		if( edgeListG[ checkPtr->edge ].start >= rx2 ||
			edgeListG[ checkPtr->edge ].end <= l2x1 ) {
		    continue ;
		}
		check = 0 ;
		break ;
	    }
	    if( check ) {
		edgeListG[++numProbesG + edgeCountG].start  = ry ;
		edgeListG[numProbesG + edgeCountG].end      = l2y ;
		edgeListG[numProbesG + edgeCountG].loc      = l2x1 ;
		edgeListG[numProbesG + edgeCountG].length   = l2y - ry ;
		edgeListG[numProbesG + edgeCountG].fixed    = 0 ;
		edgeListG[numProbesG + edgeCountG].cell     = 0 ;
		edgeListG[numProbesG + edgeCountG].UorR     = 1 ;
		tinsert( &vEdgeRootG, l2x1 , numProbesG + edgeCountG ) ;
#ifdef DEBUG
		fprintf(fpdebugG,"vprobe:%d  ", numProbesG ) ;
		fprintf(fpdebugG,"start:%d  end:%d  loc:%d  UorR:%d\n",
					ry , l2y , l2x1 , 1 ) ;
#endif
		edgeListG[++numProbesG + edgeCountG].start  = ry ;
		edgeListG[numProbesG + edgeCountG].end      = l2y  ;
		edgeListG[numProbesG + edgeCountG].loc      = rx2 ;
		edgeListG[numProbesG + edgeCountG].length   = l2y - ry ;
		edgeListG[numProbesG + edgeCountG].fixed    = 0 ;
		edgeListG[numProbesG + edgeCountG].cell     = 0 ;
		edgeListG[numProbesG + edgeCountG].UorR     = -1 ;
		tinsert( &vEdgeRootG, rx2 , numProbesG + edgeCountG ) ;
#ifdef DEBUG
		fprintf(fpdebugG,"vprobe:%d  ", numProbesG ) ;
		fprintf(fpdebugG,"start:%d  end:%d  loc:%d  UorR:%d\n",
					ry , l2y , rx2 , -1 ) ;
#endif
	    }
	}
    }
    doubleDown( ritePtr ) ;
}
return ;
}



doubleDown( rptr ) 
DLINK1PTR rptr ;
{

int ry , rx1 , rx2 , ly , lx1 , lx2 , check , edge ;
DLINK1PTR checkPtr , ptr ;

ry  = edgeListG[ rptr->edge ].loc   ;
rx2 = edgeListG[ rptr->edge ].end   ;
rx1 = edgeListG[ rptr->edge ].start ;

ptr = HptrsG[ tprop( HrootG , ry ) ] ;
for( ; ptr != (DLINK1PTR) NULL ; ptr = ptr->next ) {
    edge = ptr->edge ;
    if( edgeListG[edge].UorR > 0 ) {
	continue ;
    }
    ly  = edgeListG[edge].loc   ;
    lx1 = edgeListG[edge].start ;
    lx2 = edgeListG[edge].end   ;
    if( ! ( lx2 < rx2 && lx1 > rx1 )  ) {
	continue ;
    }
    
    check = 1 ;
    checkPtr = HptrsG[ tprop( HrootG , ry ) ] ;
    for(; checkPtr != (DLINK1PTR) NULL ; checkPtr = checkPtr->next){
	if( checkPtr == rptr ) {
	    continue ;
	}
	if( edgeListG[ checkPtr->edge ].UorR < 0 ) {
	    continue ;
	}
	if( edgeListG[ checkPtr->edge ].loc > ly ) {
	    break ;
	}
	if( edgeListG[ checkPtr->edge ].start >= lx2 ||
		    edgeListG[ checkPtr->edge ].end <= lx1 ) {
	    continue ;
	}
	check = 0 ;
	break ;
    }
    if( check ) {
	edgeListG[++numProbesG + edgeCountG].start  = ry ;
	edgeListG[numProbesG + edgeCountG].end      = ly ;
	edgeListG[numProbesG + edgeCountG].loc      = lx1 ;
	edgeListG[numProbesG + edgeCountG].length   = ly - ry ;
	edgeListG[numProbesG + edgeCountG].fixed    = 0 ;
	edgeListG[numProbesG + edgeCountG].cell     = 0 ;
	edgeListG[numProbesG + edgeCountG].UorR     = 1 ;
	tinsert( &vEdgeRootG, lx1 , numProbesG + edgeCountG ) ;
#ifdef DEBUG
	fprintf(fpdebugG,"vprobe:%d  start:%d  end:%d  loc:%d  UorR:%d\n",
		    numProbesG, ry , ly , lx1 , 1 ) ;
#endif
	edgeListG[++numProbesG + edgeCountG].start  = ry ;
	edgeListG[numProbesG + edgeCountG].end      = ly ;
	edgeListG[numProbesG + edgeCountG].loc      = lx2 ;
	edgeListG[numProbesG + edgeCountG].length   = ly - ry ;
	edgeListG[numProbesG + edgeCountG].fixed    = 0 ;
	edgeListG[numProbesG + edgeCountG].cell     = 0 ;
	edgeListG[numProbesG + edgeCountG].UorR     = -1 ;
	tinsert( &vEdgeRootG, lx2 , numProbesG + edgeCountG ) ;
#ifdef DEBUG
	fprintf(fpdebugG,"vprobe:%d  start:%d  end:%d  loc:%d  UorR:%d\n",
		    numProbesG, ry , ly , lx2 , -1 ) ;
#endif
    }
}
return ;
}
