#ifndef H_BUILDIMP
#define H_BUILDIMP

extern IPBOXPTR *impFeedsG ;
extern int *FeedInRowG ;
extern int gxstartG ;
extern int gxstopG ;

void addfeed( int row , int pos , int feednum );
void buildimp( );
void fixwolf( );
void link_imptr( );
void decide_boundary( );

#endif
