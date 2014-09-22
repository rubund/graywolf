/* ----------------------------------------------------------------- 
FILE:	    queue.c                                       
DESCRIPTION:This file contains the routines to handle queues
CONTENTS:  YinitQueue( queue, node ) 
		YQUEUE *queue ;
		char *node ;
	    char *YtopQueue( queue ) 
		YQUEUE *queue ;
	    Yadd2Queue( queue, node ) 
		YQUEUE *queue ;
		char *node ;
	    YQUEUEPTR YqueueNotEmpty( queue ) 
		YQUEUE *queue ;
	    YdumpQueue( queue ) 
		YQUEUE *queue ;
DATE:	    Mar 16, 1989 
REVISIONS:  Sep 16, 1989 - all debug directed to stderr.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) queue.c (Yale) version 3.4 12/15/91" ;
#endif

#include <stdio.h>
#include <yalecad/queue.h>

/* initialize queue */
YinitQueue( queue, node ) 
YQUEUE *queue ;
char *node ;
{   
    YQUEUEPTR temp ; 
    queue->top_of_queue = temp = YMALLOC( 1, YQUEUEBOX ) ; 
    temp->next = NULL ; 
    queue->bot_of_queue = temp ;   
    temp->data = node ; 
} /* end YinitQueue */

/* top queue return the element at top of the queue */
char *YtopQueue( queue ) 
YQUEUE *queue ;
{
    YQUEUEPTR temp ; 
    char *data ; 

    if( temp = queue->top_of_queue ){
	data = temp->data ;
	queue->top_of_queue = temp->next ; 
	YFREE( temp ) ;  
	return( data ) ;
    } else { /* queue is empty */
	return( NULL ) ;
    }
} /* end YtopQueue */

/* add a vertex to the end of the queue */
Yadd2Queue( queue, node ) 
YQUEUE *queue ;
char *node ;
{  
    if( queue->top_of_queue ){ 
	queue->bot_of_queue->next= YMALLOC( 1, YQUEUEBOX ) ;
	queue->bot_of_queue = queue->bot_of_queue->next ; 
	queue->bot_of_queue->next = NULL ; 
	queue->bot_of_queue->data = node ; 
    } else { 
	YinitQueue( queue, node ) ; 
    } 
} /* end Yadd2Queue */

/* check status of the queue */
YQUEUEPTR YqueueNotEmpty( queue ) 
YQUEUE *queue ;
{  
    return( queue->top_of_queue ) ;
}

/* debug function to dump the contents of the queue */
YdumpQueue( queue ) 
YQUEUE *queue ;
{
    YQUEUEPTR temp ; 

    fprintf( stderr,"Queue:" ) ;
    if( temp = queue->top_of_queue ){
	for( ;temp; temp=temp->next ){
#if SIZEOF_VOID_P == 64
	    fprintf( stderr,"%ld ", (INT)temp->data ) ;
#else
	    fprintf( stderr,"%d ", (INT)temp->data ) ;
#endif
	}
	fprintf( stderr,"\n" ) ;
    } else { /* queue is empty */
	fprintf( stderr,"empty.\n" ) ;
    }
} /* end YdumpQueue */
