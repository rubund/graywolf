/* ----------------------------------------------------------------- 
"@(#) queue.h version 1.4 10/9/90"
FILE:	    queue.h                                       
DESCRIPTION:Insert file for queue library routines.
DATE:	    Mar 16, 1989 
REVISIONS:  
----------------------------------------------------------------- */
#ifndef QUEUE_H
#define QUEUE_H

#include <yalecad/base.h>

/* *********** data structures *************** */
typedef struct yqueue_info {
    char              *data ;
    struct yqueue_info *next ;
} YQUEUEBOX, *YQUEUEPTR  ;

typedef struct {
    YQUEUEPTR  top_of_queue ;
    YQUEUEPTR  bot_of_queue ;
} YQUEUE ;

/* *********** FIFO routines *************** */
extern YinitQueue( P2(YQUEUE *queue, char *node ) ) ;
/* 
Arguments:
    YQUEUE *queue ;
    char *node ;
Function:
    Initialization of the queue handler (FIFO) for a given queue.  User
    supplies a pointer to the data that should be stored in queue.
*/

extern char *YtopQueue( P1(YQUEUE *queue ) ) ;
/* 
Arguments:
    YQUEUE *queue ;
Function:
    Returns the users pointer to the first element in the FIFO.
*/

extern Yadd2Queue( P2(YQUEUE *queue, char *node ) ) ;
/* 
Arguments:
    YQUEUE *queue ;
    char *node ;
Function:
    Add a new element to the end of the queue.
*/

/* check status of the queue */
extern YQUEUEPTR YqueueNotEmpty( P1(YQUEUE *queue ) ) ;
/* 
Arguments:
    YQUEUE *queue ;
Function:
    Returns the QUEUEPTR if queue is not empty. NULL otherwise.
*/

/* debug function to dump the contents of the queue */
extern YdumpQueue( P1(YQUEUE *queue ) ) ;
/* 
Arguments:
    YQUEUE *queue ;
Function:
    Debug function for queue handler.
*/

#endif /* QUEUE_H */
