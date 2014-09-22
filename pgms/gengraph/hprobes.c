/* ----------------------------------------------------------------- 
FILE:	    hprobes.c                                       
DESCRIPTION:Horizontal probe code.
CONTENTS:   hprobes()
	    doubleBack( dptr ) 
		DLINK1PTR dptr ;
DATE:	    Mar  3, 1990 
REVISIONS:  Mar  3, 1990 - Now a standalone program.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) hprobes.c version 1.1 3/3/90" ;
#endif

#include <geo.h>
#include <yalecad/debug.h>

hprobes()
{

DLINK1PTR downPtr , up1ptr , up2ptr , checkPtr , ptr ;
int dx , dy1 , dy2 , dedge , by1 , by2 , uedge , edge , check ;
int u1y2 , u1y1 , u1x , u2y2 , u2y1 , u2x ;

downPtr = vFixedListG ;
for( ; downPtr != (DLINK1PTR) NULL ; downPtr = downPtr->next ) {
    dedge = downPtr->edge ;
    if( edgeListG[dedge].UorR > 0 ) {
	continue ;
    }
    dx  = edgeListG[dedge].loc   ;
    dy1 = edgeListG[dedge].start ;
    dy2 = edgeListG[dedge].end   ;
    
    up2ptr = VptrsG[ tprop( VrootG , dx ) ] ;
    for(; up2ptr != (DLINK1PTR) NULL ; up2ptr = up2ptr->next){
	uedge = up2ptr->edge ;
	if( edgeListG[uedge].UorR < 0 ) {
	    continue ;
	}
	u2x  = edgeListG[uedge].loc   ;
	u2y1 = edgeListG[uedge].start ;
	u2y2 = edgeListG[uedge].end   ;
	if( u2y2 < dy2 || u2y1 >= dy2 ) {
	    continue ;
	}
	break ;
    }
    
    up1ptr = VptrsG[ tprop( VrootG , dx ) ] ;
    for(; up1ptr != (DLINK1PTR) NULL ; up1ptr = up1ptr->next){
	uedge = up1ptr->edge ;
	if( edgeListG[uedge].UorR < 0 ) {
	    continue ;
	}
	u1x  = edgeListG[uedge].loc   ;
	u1y1 = edgeListG[uedge].start ;
	u1y2 = edgeListG[uedge].end   ;
	if( u1y2 <= dy1 || u1y1 > dy1 ) {
	    continue ;
	}
	break ;
    }

    if( up2ptr != (DLINK1PTR) NULL && up2ptr == up1ptr ) {
	check = 1 ;
	checkPtr = VptrsG[ tprop( VrootG , dx ) ] ;
	for(; checkPtr != (DLINK1PTR) NULL ;
				    checkPtr = checkPtr->next ) {
	    if( edgeListG[ checkPtr->edge ].UorR < 0 ) {
		continue ;
	    }
	    if( edgeListG[ checkPtr->edge ].loc >= u2x ) {
		break ;
	    }
	    if( edgeListG[ checkPtr->edge ].start >= dy2 ||
			edgeListG[ checkPtr->edge ].end <= dy1 ) {
		continue ;
	    }
	    check = 0 ;
	    break ;
	}
	if( check ) {
	    edgeListG[++numProbesG + edgeCountG].start  = dx ;
	    edgeListG[numProbesG + edgeCountG].end      = u2x  ;
	    edgeListG[numProbesG + edgeCountG].loc      = dy2 ;
	    edgeListG[numProbesG + edgeCountG].length   = u2x - dx ;
	    edgeListG[numProbesG + edgeCountG].fixed    = 0 ;
	    edgeListG[numProbesG + edgeCountG].cell     = 0 ;
	    edgeListG[numProbesG + edgeCountG].UorR     = 1 ;
	    tinsert( &hEdgeRootG, dy2 , numProbesG + edgeCountG ) ;
#ifdef DEBUG
	    fprintf(fpdebugG,"hprobe:%d  ", numProbesG ) ;
	    fprintf(fpdebugG,"start:%d  end:%d  loc:%d  UorR:%d\n",
					dx , u2x , dy2 , 1 ) ;
#endif
	    edgeListG[++numProbesG + edgeCountG].start  = dx ;
	    edgeListG[numProbesG + edgeCountG].end      = u2x  ;
	    edgeListG[numProbesG + edgeCountG].loc      = dy1 ;
	    edgeListG[numProbesG + edgeCountG].length   = u2x - dx ;
	    edgeListG[numProbesG + edgeCountG].fixed    = 0 ;
	    edgeListG[numProbesG + edgeCountG].cell     = 0 ;
	    edgeListG[numProbesG + edgeCountG].UorR     = -1 ;
	    tinsert( &hEdgeRootG, dy1 , numProbesG + edgeCountG ) ;
#ifdef DEBUG
	    fprintf(fpdebugG,"hprobe:%d  ", numProbesG ) ;
	    fprintf(fpdebugG,"start:%d  end:%d  loc:%d  UorR:%d\n",
					dx , u2x , dy1 , -1 ) ;
#endif
	} else {
	    doubleBack( downPtr ) ;	
	}
	continue ;
    }

    if( up2ptr != (DLINK1PTR) NULL ) {
	ptr = VptrsG[ tprop( VrootG , u2x ) ] ;
	for( ptr = ptr->next; ptr != (DLINK1PTR) NULL; ptr = ptr->next){
	    if( edgeListG[ptr->edge].loc > u2x ) {
		break ;
	    }
	}
	if( ptr == (DLINK1PTR) NULL ) {
	    ptr = vFixedEndG ;
	} else {
	    ptr = ptr->prev ;
	}
	for( ; ptr != (DLINK1PTR) NULL; ptr = ptr->prev){
	    edge = ptr->edge ;
	    if( edgeListG[edge].UorR > 0 ) {
		continue ;
	    }
	    by1 = edgeListG[edge].start ;
	    by2 = edgeListG[edge].end   ;
	    if( by2 <= u2y1 || by1 > u2y1 ) {
		continue ;
	    }
	    break ;
	}
	if( downPtr == ptr ) {
	    check = 1 ;
	    checkPtr = VptrsG[ tprop( VrootG , dx ) ] ;
	    /* check for fixed edge in between */
	    for(; checkPtr != (DLINK1PTR) NULL ;
				    checkPtr = checkPtr->next ) {
		if( edgeListG[ checkPtr->edge ].UorR < 0 ) {
		    continue ;
		}
		if( edgeListG[ checkPtr->edge ].loc >= u2x ) {
		    break ;
		}
		if( edgeListG[ checkPtr->edge ].start >= dy2 ||
			edgeListG[ checkPtr->edge ].end <= u2y1 ) {
		    continue ;
		}
		check = 0 ;
		break ;
	    }
	    if( check ) {
		edgeListG[++numProbesG + edgeCountG].start  = dx ;
		edgeListG[numProbesG + edgeCountG].end      = u2x  ;
		edgeListG[numProbesG + edgeCountG].loc      = dy2 ;
		edgeListG[numProbesG + edgeCountG].length   = u2x - dx ;
		edgeListG[numProbesG + edgeCountG].fixed    = 0 ;
		edgeListG[numProbesG + edgeCountG].cell     = 0 ;
		edgeListG[numProbesG + edgeCountG].UorR     = 1 ;
		tinsert( &hEdgeRootG, dy2 , numProbesG + edgeCountG ) ;
#ifdef DEBUG
		fprintf(fpdebugG,"hprobe:%d  ", numProbesG ) ;
		fprintf(fpdebugG,"start:%d  end:%d  loc:%d  UorR:%d\n",
					dx , u2x , dy2 , 1 ) ;
#endif
		edgeListG[++numProbesG + edgeCountG].start  = dx ;
		edgeListG[numProbesG + edgeCountG].end      = u2x  ;
		edgeListG[numProbesG + edgeCountG].loc      = u2y1 ;
		edgeListG[numProbesG + edgeCountG].length   = u2x - dx ;
		edgeListG[numProbesG + edgeCountG].fixed    = 0 ;
		edgeListG[numProbesG + edgeCountG].cell     = 0 ;
		edgeListG[numProbesG + edgeCountG].UorR     = -1 ;
		tinsert( &hEdgeRootG, u2y1 , numProbesG + edgeCountG ) ;
#ifdef DEBUG
		fprintf(fpdebugG,"hprobe:%d  ", numProbesG ) ;
		fprintf(fpdebugG,"start:%d  end:%d  loc:%d  UorR:%d\n",
				    dx , u2x , u2y1 , -1 ) ;
#endif
	    }
	}
    }
    if( up1ptr != (DLINK1PTR) NULL ) {
	ptr = VptrsG[ tprop( VrootG , u1x ) ] ;
	for( ptr = ptr->next; ptr != (DLINK1PTR) NULL; ptr = ptr->next){
	    if( edgeListG[ptr->edge].loc > u1x ) {
		break ;
	    }
	}
	if( ptr == (DLINK1PTR) NULL ) {
	    ptr = vFixedEndG ;
	} else {
	    ptr = ptr->prev ;
	}
	for( ; ptr != (DLINK1PTR) NULL; ptr = ptr->prev){
	    edge = ptr->edge ;
	    if( edgeListG[edge].UorR > 0 ) {
		continue ;
	    }
	    by1 = edgeListG[edge].start ;
	    by2 = edgeListG[edge].end   ;
	    if( by2 < u1y2 || by1 >= u1y2 ) {
		continue ;
	    }
	    break ;
	}
	if( downPtr == ptr ) {
	    check = 1 ;
	    checkPtr = VptrsG[ tprop( VrootG , dx ) ] ;
	    for(; checkPtr != (DLINK1PTR) NULL ;
				    checkPtr = checkPtr->next ) {
		if( edgeListG[ checkPtr->edge ].UorR < 0 ) {
		    continue ;
		}
		if( edgeListG[ checkPtr->edge ].loc >= u1x ) {
		    break ;
		}
		if( edgeListG[ checkPtr->edge ].start >= u1y2 ||
			edgeListG[ checkPtr->edge ].end <= dy1 ) {
		    continue ;
		}
		check = 0 ;
		break ;
	    }
	    if( check ) {
		edgeListG[++numProbesG + edgeCountG].start  = dx ;
		edgeListG[numProbesG + edgeCountG].end      = u1x  ;
		edgeListG[numProbesG + edgeCountG].loc      = u1y2 ;
		edgeListG[numProbesG + edgeCountG].length   = u1x - dx ;
		edgeListG[numProbesG + edgeCountG].fixed    = 0 ;
		edgeListG[numProbesG + edgeCountG].cell     = 0 ;
		edgeListG[numProbesG + edgeCountG].UorR     = 1 ;
		tinsert( &hEdgeRootG, u1y2 , numProbesG + edgeCountG ) ;
#ifdef DEBUG
		fprintf(fpdebugG,"hprobe:%d  ", numProbesG ) ;
		fprintf(fpdebugG,"start:%d  end:%d  loc:%d  UorR:%d\n",
					dx , u1x , u1y2 , 1 ) ;
#endif
		edgeListG[++numProbesG + edgeCountG].start  = dx ;
		edgeListG[numProbesG + edgeCountG].end      = u1x  ;
		edgeListG[numProbesG + edgeCountG].loc      = dy1 ;
		edgeListG[numProbesG + edgeCountG].length   = u1x - dx ;
		edgeListG[numProbesG + edgeCountG].fixed    = 0 ;
		edgeListG[numProbesG + edgeCountG].cell     = 0 ;
		edgeListG[numProbesG + edgeCountG].UorR     = -1 ;
		tinsert( &hEdgeRootG, dy1 , numProbesG + edgeCountG ) ;
#ifdef DEBUG
		fprintf(fpdebugG,"hprobe:%d  ", numProbesG ) ;
		fprintf(fpdebugG,"start:%d  end:%d  loc:%d  UorR:%d\n",
					dx , u1x , dy1 , -1 ) ;
#endif
	    }
	}
    }
    doubleBack( downPtr ) ;
}
return ;
}



doubleBack( dptr ) 
DLINK1PTR dptr ;
{

int dx , dy1 , dy2 , ux , uy1 , uy2 , check , edge ;
DLINK1PTR checkPtr , ptr ;

dx  = edgeListG[ dptr->edge ].loc   ;
dy2 = edgeListG[ dptr->edge ].end   ;
dy1 = edgeListG[ dptr->edge ].start ;

ptr = VptrsG[ tprop( VrootG , dx ) ] ;
for( ; ptr != (DLINK1PTR) NULL ; ptr = ptr->next ) {
    edge = ptr->edge ;
    if( edgeListG[edge].UorR < 0 ) {
	continue ;
    }
    ux  = edgeListG[edge].loc   ;
    uy1 = edgeListG[edge].start ;
    uy2 = edgeListG[edge].end   ;
    if( ! ( uy2 < dy2 && uy1 > dy1 )  ) {
	continue ;
    }
    
    check = 1 ;
    checkPtr = VptrsG[ tprop( VrootG , dx ) ] ;
    for(; checkPtr != (DLINK1PTR) NULL ; checkPtr = checkPtr->next){
	if( checkPtr == dptr ) {
	    continue ;
	}
	if( edgeListG[ checkPtr->edge ].UorR > 0 ) {
	    continue ;
	}
	if( edgeListG[ checkPtr->edge ].loc > ux ) {
	    break ;
	}
	if( edgeListG[ checkPtr->edge ].start >= uy2 ||
		    edgeListG[ checkPtr->edge ].end <= uy1 ) {
	    continue ;
	}
	check = 0 ;
	break ;
    }
    if( check ) {
	edgeListG[++numProbesG + edgeCountG].start  = dx ;
	edgeListG[numProbesG + edgeCountG].end      = ux ;
	edgeListG[numProbesG + edgeCountG].loc      = uy2 ;
	edgeListG[numProbesG + edgeCountG].length   = ux - dx ;
	edgeListG[numProbesG + edgeCountG].fixed    = 0 ;
	edgeListG[numProbesG + edgeCountG].cell     = 0 ;
	edgeListG[numProbesG + edgeCountG].UorR     = 1 ;
	tinsert( &hEdgeRootG, uy2 , numProbesG + edgeCountG ) ;
#ifdef DEBUG
	fprintf(fpdebugG,"hprobe:%d  start:%d  end:%d  loc:%d  UorR:%d\n",
		    numProbesG, dx , ux , uy2 , 1 ) ;
#endif
	edgeListG[++numProbesG + edgeCountG].start  = dx ;
	edgeListG[numProbesG + edgeCountG].end      = ux ;
	edgeListG[numProbesG + edgeCountG].loc      = uy1 ;
	edgeListG[numProbesG + edgeCountG].length   = ux - dx ;
	edgeListG[numProbesG + edgeCountG].fixed    = 0 ;
	edgeListG[numProbesG + edgeCountG].cell     = 0 ;
	edgeListG[numProbesG + edgeCountG].UorR     = -1 ;
	tinsert( &hEdgeRootG, uy1 , numProbesG + edgeCountG ) ;
#ifdef DEBUG
	fprintf(fpdebugG,"hprobe:%d  start:%d  end:%d  loc:%d  UorR:%d\n",
		    numProbesG, dx , ux , uy1 , -1 ) ;
#endif
    }
}
return ;
}
