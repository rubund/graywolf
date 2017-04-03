#include "standard.h"
#include "main.h"

static YHASHPTR net_hash_tableS ;
static PATHPTR pathPtrS = NULL ;  /* start of path list */

YHASHPTR get_net_table() ;

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

static void free_net_data( int *data )
{
	Ysafe_free( data ) ;
} /* free_swap_data */

PATHPTR get_path_list()
{
	return( pathPtrS ) ;
} /* end get_path_list */

void init_read_nets() {
	net_hash_tableS = get_net_table() ;
}

void finish_read_nets() {
	check_paths() ;
	build_path_array() ;
	init_path_set() ;
	init_net_set() ;
	add_paths_to_cells() ;
	/* free hash table */
	Yhash_table_delete( net_hash_tableS , free_net_data ) ;
}
