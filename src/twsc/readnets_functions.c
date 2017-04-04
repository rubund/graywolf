#include "standard.h"
#include "main.h"
#include "parser_defines.h"
#include "readnets.h"
#include <string.h>
#include <pads.h>
#include <yalecad/buster.h>
#include <yalecad/hash.h>
#include <yalecad/debug.h>
#include <yalecad/string.h>
#include <yalecad/message.h>

static YHASHPTR net_hash_tableS ;
static PATHPTR pathPtrS = NULL ;  /* start of path list */
static int total_num_pathS = 0 ;
static BOOL abortFlagS;
static int netS ; /* current net being processed */
static PATHPTR curPathS ; /* current bottom of path list so list */
static GLISTPTR netPtrS ;

YHASHPTR get_net_table() ;
static void bad_net( char *net, BOOL fatal );
void init_path_set() ;
void init_net_set() ;

static void free_net_data( int *data )
{
	Ysafe_free( data ) ;
} /* free_swap_data */

void process_net_rec( char *netname ) 
{
	int *data ;
	if(!(data = (int*) Yhash_search( net_hash_tableS, netname, NULL, FIND ))){
		bad_net( netname, FALSE ) ; /* not fatal */
		netS = 0 ;
	} else {
		netS = *data ;
	}
} /* end process_net_rec */

void ignore_net()
{
	if( netS ){
		netarrayG[netS]->ignore = 1 ;
	}
} /* end ignore_net */

void ignore_route()
{
	if( netS ){
		netarrayG[netS]->ignore = -1 ;
	}
} /* end ignore_route */

void add_path( BOOL pathFlag, char *net )
{
	int *data ;
	GLISTPTR tempNetPtr ;
	
	if( pathFlag == STARTPATH ){
		/* see if pathptr exists */
		if( pathPtrS ){
			curPathS->next = (PATHPTR) Ysafe_malloc( sizeof(PATHBOX) ) ;
			curPathS = curPathS->next ;
		} else { /* first path - start list */
			curPathS = pathPtrS = (PATHPTR) Ysafe_malloc( sizeof(PATHBOX) ) ;
		}
		curPathS->next = NULL ;
		netPtrS = curPathS->nets = (GLISTPTR) Ysafe_malloc( sizeof(GLISTBOX) ) ;
		netPtrS->next = NULL ;
	} else {  /* continuing a path */
		tempNetPtr = netPtrS ;
		ASSERT( netPtrS, "add_path", "net pointer should be non-NULL" ) ;
		netPtrS = curPathS->nets = (GLISTPTR) Ysafe_malloc( sizeof(GLISTBOX) ) ;
		netPtrS->next = tempNetPtr ;
	}
	data = (int*) Yhash_search( net_hash_tableS , net, NULL, FIND);
	if(data){
		/* get data from field */
		netPtrS->p.net = *data ;
	} else {
		bad_net( net, TRUE ) ; /* Fatal abort */
	}
	/* string bufferS is allocated in lex */
	if( net ){
		Ysafe_free( net ) ;
	}
} /* end add_path */

void end_path(int lower_bound, int upper_bound, int priority )
{
	GLISTPTR nets, path_ptr, tempPath ;
	DBOXPTR dimptr ;
	int net_number ;

	if( abortFlagS ){
		return ;
	}

	curPathS->lower_bound = lower_bound ;
	curPathS->upper_bound = upper_bound ;
	curPathS->priority = priority ;
	total_num_pathS++ ;  /* total number of paths given */
	if( priority ){
		numpathsG++ ;  /* increment number of active paths */
		/* add path to path list in netarrayG */
		for( nets = curPathS->nets; nets ; nets = nets->next ){
			net_number = nets->p.net ;
			dimptr = netarrayG[net_number] ;
			if( tempPath = dimptr->paths ){
				path_ptr = dimptr->paths = (GLISTPTR) Ysafe_malloc( sizeof(GLISTBOX) ) ;
				path_ptr->next = tempPath ;
			} else {  /* start a new list */
				path_ptr = dimptr->paths = (GLISTPTR) Ysafe_malloc( sizeof(GLISTBOX) ) ;
				path_ptr->next = NULL ;
			}
			/* use total_num_pathS so we can index patharrayG */
			path_ptr->p.path = total_num_pathS ;
		}
	}
} /* end function end_path */

void check_paths()
{
	DBOXPTR nptr ;      /* traverse the nets */

	/* first make sure that the nets in paths are not ignored */
	for( int i = 1; i <= numnetsG; i++ ){
		nptr = netarrayG[i] ;
		if( nptr->paths && nptr->ignore ){
			sprintf( YmsgG, "Net:%s is specified in a path and ignored\n", nptr->name ) ;
			M( ERRMSG, "check_paths", YmsgG ) ;
			abortFlagS = TRUE ;
		}
	}
} /* check_paths */

void build_path_array()
{
	PATHPTR curPtr ;
	patharrayG = (PATHPTR *) Ysafe_malloc( (total_num_pathS+1)*sizeof(PATHPTR) ) ;
	int i = 1 ;
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
	int net_number ;
	int total_cells ;
	PSETPTR pathlist, enum_path_set() ;
	CBOXPTR ptr ;
	GLISTPTR  path_ptr, tempPath ;
	DBOXPTR dimptr ;
	PINBOXPTR pinptr ;

	for( int i=1;i<=lastpadG; i++ ){
		ptr = carrayG[i] ;
		clear_path_set() ;
		/* look for the UNIQUE paths that connects to this cell */
		for(pinptr=ptr->pins;pinptr;pinptr=pinptr->nextpin){
			net_number = pinptr->net ;
			/* now go to net array */
			dimptr = netarrayG[net_number] ;
			/* look at all paths that use this net */
			for( path_ptr=dimptr->paths;path_ptr;path_ptr=path_ptr->next){
				add2path_set( path_ptr->p.path ) ;
			}
		}
		/* now add UNIQUE list of paths to this cell */
		for( pathlist=enum_path_set(); pathlist; pathlist=pathlist->next){
			if( tempPath = ptr->paths ){
				path_ptr = ptr->paths = (GLISTPTR) Ysafe_malloc( sizeof(GLISTBOX) ) ;
				path_ptr->next = tempPath ;
			} else {  /* start a new list */
				path_ptr = ptr->paths = (GLISTPTR) Ysafe_malloc( sizeof(GLISTBOX) ) ;
				path_ptr->next = NULL ;
			}
			/* copy path to cell list of paths */
			path_ptr->p.path = pathlist->path ;
		}
	}
}

static void bad_net( char *net, BOOL fatal )
{
	sprintf( YmsgG, "The net named: %s  in the .net file ", net );
	strcat( YmsgG, "was not encountered\n" ) ;
	fprintf( fpoG, "%s ", YmsgG ) ;
	if( fatal ){
		M( ERRMSG, "add_path", YmsgG ) ;
		strcpy( YmsgG,"\twhile reading the .cel file --- FATAL error\n") ;
		abortFlagS = TRUE ;
		M( ERRMSG, NULL, YmsgG ) ;
	} else {
		M( WARNMSG, "add_path", YmsgG ) ;
		strcpy( YmsgG, "\twhile reading the .cel file --- net constraint ignored.\n") ;
		M( WARNMSG, NULL, YmsgG ) ;
	}
	fprintf( fpoG, "%s ", YmsgG ) ;
} /* end bad_net */

void init_read_nets() {
	net_hash_tableS = get_net_table() ;
}

void finish_read_nets() {
	init_net_set();
	init_path_set();
	check_paths() ;
	build_path_array() ;
	add_paths_to_cells() ;
	/* free hash table */
	Yhash_table_delete( net_hash_tableS , free_net_data ) ;
}
