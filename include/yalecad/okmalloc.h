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
extern char *Ysafe_malloc( P1(INT bytes) ) ;
extern char *Ysafe_calloc( P2(INT num_entries, INT bytes) ) ;
extern char *Ysafe_realloc( P2(void *ptr, INT bytes) ) ;
extern char *Yvector_alloc( P3(INT lo, INT hi, INT size ) ) ;
extern char *Yvector_calloc( P3(INT lo, INT hi, INT size ) ) ;
extern char *Yvector_realloc( P4(VOIDPTR array,INT lo,INT hi,INT size )) ;
extern VOID Yvector_free( P3( VOIDPTR array, INT lo, INT size ) ) ;

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
extern VOID Ysafe_free( P3(void *ptr,char *file,INT line) ) ;
extern VOID Ysafe_cfree( P3(void *ptr,char *file,INT line) ) ;
extern char *Ysafe_malloc( P3(INT bytes,char *file,INT line) ) ;
extern char *Ysafe_calloc( P4(INT num_entries, INT bytes,char *file,INT line) ) ;
extern char *Ysafe_realloc( P4(void *ptr, INT bytes,char *file,INT line) ) ;
extern char *Yvector_alloc( P5(INT lo, INT hi, INT size,char *file,INT line ) ) ;
extern char *Yvector_calloc( P5(INT lo, INT hi, INT size,char *file,INT line ) ) ;
extern char *Yvector_realloc( P6(VOIDPTR a,INT lo,INT h,INT s,char *f,INT l )) ;
extern VOID Yvector_free( P5( VOIDPTR array, INT lo, INT size,char *f,INT l ) ) ;

#endif /* MEM_DEBUG */

/* In both cases, define these */
extern INT YgetCurMemUse( P1(void) ) ;
extern INT YgetMaxMemUse( P1(void) ) ;
extern INT YcheckMemObj( P1(char *ptr) ) ;
extern VOID YdebugMemory( P1(INT flag ) ) ;
extern INT YcheckDebug( P1(VOIDPTR where ) ) ;
extern VOID Yinit_memsize( P1(INT memsize) ) ;
extern VOID Ydump_mem( P1(void) ) ;
extern VOID Ypmemerror( P1(char *message ) ) ;

#endif /* YOKMALLOC_H */
