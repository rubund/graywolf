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

typedef struct feed_assgn_box {
    PINBOXPTR netptr ;
    PINBOXPTR refer  ;
    SEGBOXPTR segptr ;
}
*FEED_SEG_PTR ,
FEED_SEG ;

#endif /* FEEDS_H */
