/* ----------------------------------------------------------------- 
FILE:	    okmalloc.h                                       
CONTENTS:   macro definitions for memory manager.
DATE:	    Tue Mar  3 16:05:51 EST 1992
REVISIONS:  
----------------------------------------------------------------- */
#ifndef YOKMALLOC_H
#define YOKMALLOC_H

#ifndef lint
static char Yokmalloc_HId[] = "@(#) okmalloc.h version 1.1 3/5/92" ;
#endif

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
extern void Ysafe_free( void *ptr ) ;
extern void Ysafe_cfree( void *ptr ) ;
extern char *Ysafe_malloc( int bytes ) ;
extern char *Ysafe_calloc( int num_entries, int bytes ) ;
extern char *Ysafe_realloc( void *ptr, int bytes ) ;
extern char *Yvector_alloc( int lo, int hi, int size ) ;
extern char *Yvector_calloc( int lo, int hi, int size ) ;
extern char *Yvector_realloc( void* array,int lo,int hi,int size) ;
extern void Yvector_free( void* array, int lo, int size ) ;

/* In both cases, define these */
extern int YgetCurMemUse(void) ;
extern int YgetMaxMemUse(void) ;
extern int YcheckMemObj(char *ptr) ;
extern void YdebugMemory(int flag) ;
extern int YcheckDebug(void* where) ;
extern void Yinit_memsize(int memsize) ;
extern void Ydump_mem(void) ;
extern void Ypmemerror(char *message) ;

#endif /* YOKMALLOC_H */
