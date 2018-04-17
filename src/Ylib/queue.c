/*
 *   Copyright (C) 1989-1991 Yale University
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
