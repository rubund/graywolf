/*
 *   Copyright (C) 1990-1991 Yale University
 *
 *   This work is distributed in the hope that it will be useful; you can
 *   redistribute it and/or modify it under the terms of the
 *   GNU General Public License as published by the Free Software Foundation;
 *   either version 2 of the License,
 *   or any later version, on the following conditions:
 *
 *   (a) YALE MAKES NO, AND EXPRESSLY DISCLAIMS
 *   ALL, REPRESENTATIONS OR WARRANTIES THAT THE MANUFACTURE, USE, PRACTICE,
 *   SALE OR
 *   OTHER DISPOSAL OF THE SOFTWARE DOES NOT OR WILL NOT INFRINGE UPON ANY
 *   PATENT OR
 *   OTHER RIGHTS NOT VESTED IN YALE.
 *
 *   (b) YALE MAKES NO, AND EXPRESSLY DISCLAIMS ALL, REPRESENTATIONS AND
 *   WARRANTIES
 *   WHATSOEVER WITH RESPECT TO THE SOFTWARE, EITHER EXPRESS OR IMPLIED,
 *   INCLUDING,
 *   BUT NOT LIMITED TO, WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A
 *   PARTICULAR
 *   PURPOSE.
 *
 *   (c) LICENSEE SHALL MAKE NO STATEMENTS, REPRESENTATION OR WARRANTIES
 *   WHATSOEVER TO
 *   ANY THIRD PARTIES THAT ARE INCONSISTENT WITH THE DISCLAIMERS BY YALE IN
 *   ARTICLE
 *   (a) AND (b) above.
 *
 *   (d) IN NO EVENT SHALL YALE, OR ITS TRUSTEES, DIRECTORS, OFFICERS,
 *   EMPLOYEES AND
 *   AFFILIATES BE LIABLE FOR DAMAGES OF ANY KIND, INCLUDING ECONOMIC DAMAGE OR
 *   INJURY TO PROPERTY AND LOST PROFITS, REGARDLESS OF WHETHER YALE SHALL BE
 *   ADVISED, SHALL HAVE OTHER REASON TO KNOW, OR IN FACT SHALL KNOW OF THE
 *   POSSIBILITY OF THE FOREGOING.
 *
 */

/* ----------------------------------------------------------------- 
FILE:	    initnets.c
DESCRIPTION:Initialize the net information.  Split this from
	    readnets.y.
CONTENTS:   
DATE:	    Dec 13, 1990 
REVISIONS:  Thu Dec 20 00:02:54 EST 1990 - made net cap and res.
		matches work.
----------------------------------------------------------------- */
#include <globals.h>
#include <custom.h>
#include <analog.h>
#include "readnets.y.h"  /* redefine yacc and lex globals */
#include "initnets.h"
#include "paths.h"

#define HOWMANY      0

static YHASHPTR netTableS ;
static BOOL abortFlagS = FALSE ;
static int total_num_pathS = 0 ;
static GLISTPTR netPtrS ;
static PATHPTR pathPtrS = NULL ;  /* start of path list */
/* cur bot of path list so list is in order given by user debug easier */
static PATHPTR curPathS ;
static int numcapmatchS = 0 ;   /* number of cap matches */
static int numresmatchS = 0 ;   /* number of res matches */
static int anetS ;          /* current analog net */
static ANETPTR aptrS ;      /* current analog net information record */
static COMMONPTR commonS ;  /* current common point record */

void build_path_array() ;
void init_path_set() ;
void init_net_set() ;
void add_paths_to_cells() ;

/* initialization before parsing nets */
void init_nets()
{
	YHASHPTR getNetTable() ;
	numpathsG = 0 ;
	netTableS = getNetTable() ;
	net_cap_matchG = (int **) Ysafe_calloc( numnetsG+1,sizeof(int *) ) ;
	net_res_matchG = (int **) Ysafe_calloc( numnetsG+1,sizeof(int *) ) ;
} /* end init_nets */

/* cleanup after parsing nets */
int cleanup_nets()
{
	if( abortFlagS ){
		return 1;
	}
	build_path_array() ;
	init_path_set() ;
	init_net_set() ;
	add_paths_to_cells() ;
} /* end cleanup_nets */

void set_net_error()
{
	abortFlagS = TRUE ;
} /* end set_net_error */

static int find_net( char *netname)
{
	char *data ;
	int  net ;

	if(!(data = Yhash_search( netTableS, netname, NULL, FIND))){
		printf( "The net named: %s  in the .net file ", netname );
		printf( "was not encountered while reading\n");
		printf( "the .cel file --- FATAL error\n");
		Ymessage_error_count() ;
		abortFlagS = TRUE ;
		return( 0 ) ;
	}
	net = * ( (int *) data ) ;
	if( net < 1 || net > numnetsG ){
		sprintf( YmsgG, "net:%s - number:%d out of bounds\n", netname, net ) ;
		M( ERRMSG, "find_net", YmsgG ) ;
		return( 0 ) ;
	} else {
		return( net ) ;
	}
} /* end find_net */

void add_path( BOOL pathFlag, char *net )
{
	int net_num ;
	GLISTPTR tempNetPtr ;
	
	/* first make sure that data is good */
	if(!(net_num = find_net( net ))){
		return ;
	}
	if( pathFlag == STARTPATH ){
		/* see if pathptr exists */
		if( pathPtrS ){
			curPathS->next = (PATHPTR) Ysafe_malloc( sizeof(PATHBOX) ) ;
			curPathS = curPathS->next ;
		} else { /* first path - start list */
			curPathS = pathPtrS = (PATHPTR) Ysafe_malloc( sizeof(PATHBOX) ) ;
		}
		curPathS->next = NULL ;
		netPtrS = curPathS->nets = 
		(GLISTPTR) Ysafe_malloc( sizeof(GLISTBOX) ) ;
		netPtrS->next = NULL ;
	} else {  /* continuing a path */
		tempNetPtr = netPtrS ;
		ASSERT( netPtrS, "add_path", "net pointer should be non-NULL" ) ;
		netPtrS = curPathS->nets = 
		(GLISTPTR) Ysafe_malloc( sizeof(GLISTBOX) ) ;
		netPtrS->next = tempNetPtr ;
	}
	/* get data from field */
	netPtrS->p.net = net_num ;
	/* string bufferS is allocated in lex */
	if( net ){
		Ysafe_free( net ) ;
	}

} /* end add_path */

void end_path(int lower_bound, int upper_bound, int priority )
{
	GLISTPTR nets, path_ptr, tempPath ;
	NETBOXPTR dimptr ;
	int net_number ;

	if( abortFlagS ){
		/* report as many errors as possible without crashing */
		return ;
	}
	curPathS->lower_bound = lower_bound ;
	curPathS->upper_bound = upper_bound ;
	curPathS->priority = priority ;
	total_num_pathS++ ;  /* total number of paths given */
	if( priority ){
		numpathsG++ ;  /* increment number of active paths */
		/* add path to path list in netarray */
		for( nets = curPathS->nets; nets ; nets = nets->next ){
			net_number = nets->p.net ;
			dimptr = netarrayG[net_number] ;
			if( tempPath = dimptr->paths ){
				path_ptr = dimptr->paths = 
				(GLISTPTR) Ysafe_malloc( sizeof(GLISTBOX) ) ;
				path_ptr->next = tempPath ;
			} else {  /* start a new list */
				path_ptr = dimptr->paths = 
				(GLISTPTR) Ysafe_malloc( sizeof(GLISTBOX) ) ;
				path_ptr->next = NULL ;
			}
			/* use total_num_pathS so we can index patharrayG */
			path_ptr->p.path = total_num_pathS ;
		}
	}
} /* end function end_path */

void build_path_array()
{
	int i =1;
	PATHPTR curPtr ;
	
	patharrayG = (PATHPTR *) Ysafe_malloc( (total_num_pathS+1)*sizeof(PATHPTR) ) ;
	for( curPtr = pathPtrS; curPtr ; curPtr = curPtr->next ){
		patharrayG[i++] = curPtr ;
	}
} /* end build_path_array */

PATHPTR get_path_list()
{
	return( pathPtrS ) ;
} /* end get_path_list */

int get_total_paths()
{
	return( total_num_pathS ) ;
} /* end get_total_paths */

void add_paths_to_cells()
{
	int howmany ;
	int net_number ;
	PSETPTR pathlist, enum_path_set() ;
	CELLBOXPTR ptr ;
	GLISTPTR  path_ptr, tempPath ;
	NETBOXPTR dimptr ;
	PINBOXPTR pinptr ;

	for( int i=1; i<=endsuperG; i++ ){
		ptr = cellarrayG[i] ;
		clear_path_set() ;
		/* look for the UNIQUE paths that connects to this cell */
		for( pinptr = ptr->pinptr ; pinptr ; pinptr = pinptr->nextpin ) {
			net_number = pinptr->net ;
			/* now go to net array */
			dimptr = netarrayG[net_number] ;
			/* look at all paths that use this net */
			for( path_ptr=dimptr->paths;path_ptr;path_ptr=path_ptr->next){
				add2path_set( path_ptr->p.path ) ;
			}
		}
		/* now add UNIQUE list of paths to this cell */
		for( pathlist=enum_path_set(); pathlist; pathlist=pathlist->next) {
			if( tempPath = ptr->paths ){
				path_ptr = ptr->paths = 
				(GLISTPTR) Ysafe_malloc( sizeof(GLISTBOX) ) ;
				path_ptr->next = tempPath ;
			} else {  /* start a new list */
				path_ptr = ptr->paths = 
				(GLISTPTR) Ysafe_malloc( sizeof(GLISTBOX) ) ;
				path_ptr->next = NULL ;
			}
			/* copy path to cell list of paths */
			path_ptr->p.path = pathlist->path ;
		}
	}
} /* end add_paths_to_cells */

void init_analog( char *net )
{
	/* first make sure that data is good */
	if(!(anetS = find_net( net ) )){
		return ;
	}
	aptrS = netarrayG[anetS]->analog_info = (ANETPTR) Ysafe_malloc( sizeof(ANETBOX) ) ;
	aptrS->num_common_points = 0 ;
	aptrS->net_type = 0 ;
	aptrS->common_pts = NIL(COMMONPTR *) ;
	aptrS->max_drop = INIT_DROP ;
	aptrS->cap_upper_bound = INIT_CAP ;
	aptrS->res_upper_bound = INIT_RES ;

} /* end init_analog */

void set_cap_upper_bound( double cap )
{
	aptrS->cap_upper_bound = cap ;
} /* end set_cap_upper_bound */

void set_res_upper_bound( double res )
{
	aptrS->res_upper_bound = res ;
} /* end set_res_upper_bound */

void set_net_type( int net_type )
{
	switch( net_type ){
		case SHIELDING_NET:
			aptrS->net_type |= SHIELDMASK ;
			break ;
		case NOISY_NET:
			aptrS->net_type |= NOISYMASK ;
			break ;
		case SENSITIVE_NET:
			aptrS->net_type |= SENSITIVEMASK ;
			break ;
	}
} /* end set_net_type */

void set_max_voltage_drop( double drop )
{
	aptrS->max_drop = drop ;
} /* end set_max_voltage_drop */

void add_common_pt()
{
	int pt ;         /* current number of common pts */

	pt = ++aptrS->num_common_points ;	
	if( aptrS->num_common_points == 1 ){
		/* first time */
		aptrS->common_pts = 
		(COMMONPTR *) Ysafe_malloc( sizeof(COMMONPTR) ) ;
	} else {
		aptrS->common_pts = 
		(COMMONPTR *) Ysafe_realloc( aptrS->common_pts,
			pt * sizeof(COMMONPTR) ) ;
	}
	commonS = aptrS->common_pts[pt-1] = (COMMONPTR) Ysafe_calloc( 1, sizeof(COMMONBOX) ) ;
	commonS->common_set = NIL(int) ;
	commonS->cap_match = NIL(int) ;
	commonS->res_match = NIL(int) ;
	commonS->num_pins = 0 ;

} /* end common_pt */

static int find_pin( char *cell, char *pin )
{
	PINBOXPTR nptr ;        /* traverse pins on a net */

	/* cell and pin must match */
	for( nptr = netarrayG[anetS]->pins; nptr; nptr = nptr->next ){
		if( strcmp( pin, nptr->pinname ) == STRINGEQ && strcmp( cell, cellarrayG[nptr->cell]->cname ) == STRINGEQ ) {
			return( nptr->pin ) ;
		}
	}
	sprintf( YmsgG, "Pin:%s on cell:%s could not be found in net:%s\n", pin, cell, netarrayG[anetS]->nname ) ;
	M( ERRMSG, "find_pin", YmsgG ) ;
	return( 0 ) ;
}

void add2common( char *cell, char *pin )
{
	int pinnum ;      /* index in netarray */
	int numpins ;     /* number of pins in common point */

	if(!(pinnum = find_pin( cell, pin ) )){
		return ;
	}
	numpins = ++commonS->num_pins ;
	if( numpins == 1 ){
		/* first time */
		commonS->common_set = (int *) Ysafe_malloc( sizeof(int) ) ;
	} else {
		commonS->common_set = (int *) 
		Ysafe_realloc( commonS->common_set, numpins*sizeof(int) ) ;
		/* now store the pin in the common pt */
		/* check to see if pin is name more than once */
		for( int i = 0 ; i < numpins - 1 ; i++ ){  /* counter for pins in common point */
			if( commonS->common_set[i] == pinnum ){
				sprintf( YmsgG, "Pin:%s include twice in common pt\n", pin ) ;
				M( ERRMSG, "add2common", YmsgG ) ;
				return ;
			}
		}
	}
	commonS->common_set[numpins-1] = pinnum ;

} /* add2common */

void common_cap( char *cell, char *pin )
{
	int pinnum ;      /* index in termarray */
	int howmany ;     /* number of pins in cap match */

	if(!(pinnum = find_pin( cell, pin ) )){
		return ;
	}
	if(!(commonS->cap_match)){
		commonS->cap_match = (int *) Ysafe_malloc( 2 * sizeof(int) ) ;
		commonS->cap_match[HOWMANY] = howmany = 1 ;
	} else {
		howmany = ++commonS->cap_match[HOWMANY] ;
		commonS->cap_match = (int *) 
		Ysafe_realloc( commonS->cap_match, (howmany+1)*sizeof(int) ) ;
		/* check to see if pin is name more than once */
		for( int i = 0 ; i < howmany - 1 ; i++ ){  /* counter for pins in cap match */
			if( commonS->cap_match[i] == pinnum ){
				sprintf( YmsgG, "Pin:%s include twice in common pt cap. match\n", pin ) ;
				M( ERRMSG, "common_cap", YmsgG ) ;
				return ;
			}
		}
	}
	commonS->cap_match[howmany] = pinnum ;
} /* end common_cap */

void common_res( char *cell, char *pin )
{
	int pinnum ;      /* index in termarray */
	int howmany ;     /* number of pins in res match */

	if(!(pinnum = find_pin( cell, pin ) )){
		return ;
	}
	if(!(commonS->res_match)){
		commonS->res_match = (int *) Ysafe_malloc( 2 * sizeof(int) ) ;
		commonS->res_match[HOWMANY] = howmany = 1 ;
	} else {
		howmany = ++commonS->res_match[HOWMANY] ;
		commonS->res_match = (int *) 
		Ysafe_realloc( commonS->res_match, (howmany+1)*sizeof(int) ) ;
		/* check to see if pin is name more than once */
		for( int i = 0 ; i < howmany - 1 ; i++ ){  /* counter for pins in res match */
			if( commonS->res_match[i] == pinnum ){
				sprintf( YmsgG, 
				"Pin:%s include twice in common pt res. match\n",
				pin ) ;
				M( ERRMSG, "common_res", YmsgG ) ;
				return ;
			}
		}
	}
	commonS->res_match[howmany] = pinnum ;
} /* end common_res */

void start_net_capmatch( char *netname )
{
	int net ;         /* index in netarray */
	int howmany ;     /* howmany net cap. matches already */
	int *match ;      /* current match array */

	/* first make sure that data is good */
	if(!(net = find_net( netname ) )){
		return ;
	}
	howmany = (int) net_cap_matchG[HOWMANY] ;
	net_cap_matchG[HOWMANY] = (int *) ++howmany ;
	match = net_cap_matchG[++numcapmatchS] = (int *) Ysafe_malloc( 2 * sizeof(int) ) ;
	match[HOWMANY] = 1 ;
	match[1] = net ;
} /* end start_net_capmatch */

void add_net_capmatch( char *netname )
{
	int net ;         /* index in netarray */
	int howmany ;     /* howmany net cap. matches already */

	/* first make sure that data is good */
	if(!(net = find_net( netname ) )){
		return ;
	}
	howmany = ++net_cap_matchG[numcapmatchS][HOWMANY] ;
	net_cap_matchG[numcapmatchS] = (int *) Ysafe_realloc( net_cap_matchG[numcapmatchS], (howmany+1) * sizeof(int) ) ;
	net_cap_matchG[numcapmatchS][howmany] = net ;
} /* end add_netcapmatch */

void start_net_resmatch( char *netname )
{
	int net ;         /* index in netarray */
	int howmany ;     /* howmany net res. matches already */
	int *match ;      /* current match array */

	/* first make sure that data is good */
	if(!(net = find_net( netname ) )){
		return ;
	}
	howmany = (int) net_res_matchG[HOWMANY] ;
	net_res_matchG[HOWMANY] = (int *) ++howmany ;
	match = net_res_matchG[++numresmatchS] = (int *) Ysafe_malloc( 2 * sizeof(int) ) ;
	match[HOWMANY] = 1 ;
	match[1] = net ;
} /* end start_net_resmatch */

void add_net_resmatch( char *netname )
{
	int net ;         /* index in netarray */
	int howmany ;     /* howmany net res. matches already */

	/* first make sure that data is good */
	if(!(net = find_net( netname ) )){
		return ;
	}
	howmany = ++net_res_matchG[numresmatchS][HOWMANY] ;
	net_res_matchG[numresmatchS] = (int *) Ysafe_realloc( net_res_matchG[numresmatchS], (howmany+1) * sizeof(int) ) ;
	net_res_matchG[numresmatchS][howmany] = net ;
} /* end add_netresmatch */
