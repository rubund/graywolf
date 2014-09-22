/* ----------------------------------------------------------------- 
"@(#) feeds.h (Yale) version 4.2 9/7/90"
FILE:	    feeds.h                                       
DESCRIPTION:TimberwolfSC insert file for feedthrus.
CONTENTS:   
DATE:	    Mar 27, 1989 
REVISIONS:  
----------------------------------------------------------------- */
#ifndef FEEDS_H
#define FEEDS_H

#ifdef FEEDS_VARS
#define EXTERN 
#else
#define EXTERN extern 
#endif

typedef struct feed_assgn_box {
    PINBOXPTR netptr ;
    PINBOXPTR refer  ;
    SEGBOXPTR segptr ;
}
*FEED_SEG_PTR ,
FEED_SEG ;

EXTERN INT *feeds_in_rowG ;
EXTERN INT *FeedInRowG ;
EXTERN INT fdWidthG ;
EXTERN INT *fdcel_addedG ;
EXTERN INT **fdcel_needG ;
EXTERN INT *total_feed_in_the_rowG ;


#undef EXTERN  

#endif /* FEEDS_H */
