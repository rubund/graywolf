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
#ifndef MEM_DEBUG

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
extern VOID Ysafe_free( P1(void *ptr) ) ;
extern VOID Ysafe_cfree( P1(void *ptr) ) ;
extern char *Ysafe_malloc( P1(int bytes) ) ;
extern char *Ysafe_calloc( P2(int num_entries, int bytes) ) ;
extern char *Ysafe_realloc( P2(void *ptr, int bytes) ) ;
extern char *Yvector_alloc( P3(int lo, int hi, int size ) ) ;
extern char *Yvector_calloc( P3(int lo, int hi, int size ) ) ;
extern char *Yvector_realloc( P4(VOIDPTR array,int lo,int hi,int size )) ;
extern VOID Yvector_free( P3( VOIDPTR array, int lo, int size ) ) ;

#else /* MEM_DEBUG - memory debug functions */

#define NIL( type )		     (type) NULL
#define YMALLOC(n, els) \
    (els *) Ysafe_malloc((n)*sizeof(els),__FILE__,__LINE__)
#define YCALLOC(n, els) \
    (els *) Ysafe_calloc(n, sizeof(els),__FILE__,__LINE__)
#define YREALLOC(ar,n,els) \
    (els *) Ysafe_realloc(ar,(n)*sizeof(els),__FILE__,__LINE__)
#define YFREE(els) \
    Ysafe_free(els,__FILE__,__LINE__)

#define YVECTOR_MALLOC(lo, hi, els) \
    (els *) Yvector_alloc(lo,hi,sizeof(els),__FILE__,__LINE__)
#define YVECTOR_CALLOC(lo, hi, els)  \
    (els *) Yvector_calloc(lo,hi,sizeof(els), __FILE__,__LINE__)
#define YVECTOR_REALLOC(ar,lo,hi,els) \
    (els *) Yvector_realloc( ar,lo,hi,sizeof(els),__FILE__,__LINE__)
#define YVECTOR_FREE(ar,lo) \
    Yvector_free( ar,lo,sizeof(* ar),__FILE__,__LINE__)

/* memory manager definitions - safety net between memory manager */
extern VOID Ysafe_free( P3(void *ptr,char *file,int line) ) ;
extern VOID Ysafe_cfree( P3(void *ptr,char *file,int line) ) ;
extern char *Ysafe_malloc( P3(int bytes,char *file,int line) ) ;
extern char *Ysafe_calloc( P4(int num_entries, int bytes,char *file,int line) ) ;
extern char *Ysafe_realloc( P4(void *ptr, int bytes,char *file,int line) ) ;
extern char *Yvector_alloc( P5(int lo, int hi, int size,char *file,int line ) ) ;
extern char *Yvector_calloc( P5(int lo, int hi, int size,char *file,int line ) ) ;
extern char *Yvector_realloc( P6(VOIDPTR a,int lo,int h,int s,char *f,int l )) ;
extern VOID Yvector_free( P5( VOIDPTR array, int lo, int size,char *f,int l ) ) ;

#endif /* MEM_DEBUG */

/* In both cases, define these */
int YgetCurMemUse(void) ;
int YgetMaxMemUse(void) ;
int YcheckMemObj(char *ptr) ;
void YdebugMemory(int flag) ;
int YcheckDebug(void* where) ;
void Ydump_mem(void) ;
void Ypmemerror(char *message) ;

#endif /* YOKMALLOC_H */
