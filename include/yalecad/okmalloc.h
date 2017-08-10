/* ----------------------------------------------------------------- 
FILE:	    okmalloc.h                                       
CONTENTS:   macro definitions for memory manager.
DATE:	    Tue Mar  3 16:05:51 EST 1992
REVISIONS:  
----------------------------------------------------------------- */
#ifndef YOKMALLOC_H
#define YOKMALLOC_H

/* memory definitions - for portability and ease of use */
#define NIL( type )		     (type) NULL
#define YMALLOC(n, els)              (els *) Ysafe_malloc((n)*sizeof(els))
#define YCALLOC(n, els)              (els *) Ysafe_calloc(n, sizeof(els))
#define YREALLOC(ar,n,els)           (els *) Ysafe_realloc(ar,(n)*sizeof(els))
#define YFREE(els)                   Ysafe_free(els)

#define YVECTOR_MALLOC(lo, hi, els)   (els *) Yvector_alloc(lo,hi,sizeof(els))
#define YVECTOR_CALLOC(lo, hi, els)   (els *) Yvector_calloc(lo,hi,sizeof(els))
#define YVECTOR_FREE(ar,lo)           Yvector_free( ar,lo,sizeof(* ar))
#define YVECTOR_REALLOC(ar,lo,hi,els) (els *) Yvector_realloc( ar,lo,hi,sizeof(els))

/* memory manager definitions - safety net between memory manager */
void Ysafe_free( void *ptr ) ;
void Ysafe_cfree( void *ptr ) ;
char *Ysafe_malloc( int bytes ) ;
char *Ysafe_calloc( int num_entries, int bytes ) ;
char *Ysafe_realloc( void *ptr, int bytes ) ;
char *Yvector_alloc( int lo, int hi, int size ) ;
char *Yvector_calloc( int lo, int hi, int size ) ;
char *Yvector_realloc( void* array,int lo,int hi,int size) ;
void Yvector_free( void* array, int lo, int size ) ;
int YgetListSize(char *ptr, char *offsetPtr);

/* In both cases, define these */
int YgetCurMemUse(void) ;
int YgetMaxMemUse(void) ;
int YcheckMemObj(char *ptr) ;
void YdebugMemory(int flag) ;
int YcheckDebug(void* where) ;
void Ydump_mem(void) ;
void Ypmemerror(char *message) ;

#endif /* YOKMALLOC_H */
