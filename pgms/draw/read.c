/* ----------------------------------------------------------------- 
FILE:	    read.c                                       
DESCRIPTION:Reads datafiles.  Modified find_max_xy to become 
	    read_max_xy which is the heart of the file I/O.
CONTENTS:   read_max_xy(count)
		int count ;
	    find_scale_factors(w,scale_factor)
		Window	w;
		float	*scale_factor;
	    min_max(min,max)
		int *min,*max;
	    swap(a,b)
		int *a,*b;
DATE:	    Jan 25, 1989 - major rewrite of version 1.0
REVISIONS:  Oct 14, 1990 - modified to work with new lib routines.
	    Mon May  6 20:36:34 EDT 1991 - rewrote so that damaged
		frames could be ignored and deleted.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) read.c version 3.8 11/23/91" ;
#endif

#include	"globals.h"
#include	<stdio.h>
#include	<yalecad/debug.h>
#include	<yalecad/message.h>
#include	<yalecad/file.h>


EXTERN char *cellFileG;          /* file name for cell file */
EXTERN char *netFileG;           /* file name for net file */
EXTERN char *symbFileG;          /* file name for symb file */
EXTERN int num_cel_recG;         /* number of cell records read in */
EXTERN int num_net_recG;         /* number of net records read in */
EXTERN DATABOX *cell_file_ptrG ; 
EXTERN DATABOX *net_file_ptrG ; 
EXTERN char  *lookupLabelG ;

#define FROMSTART    (int) 0
#define FROMCUR      (int) 1
#define FROMEND      (int) 2
#define FSEEKERROR   (int) -1

read_max_xy(count)
int count ;
/* read in data and find max x and y values of the cell and net files */
/* count is the current frame we are on */
{
    FILE	*fp ;
    char    fileName[LRECL] ;
    char    color[LRECL] ;
    int     offset ;
    int     i ;
    DATABOX ccell, pcell, ncell ;
    unsigned nitems ;

    /* first see if we need to perform frees */
    if( cell_file_ptrG ){
	Ysafe_free( cell_file_ptrG ) ;
    }
    if( net_file_ptrG ){
	Ysafe_free( net_file_ptrG ) ;
    }
    if( lookupLabelG ){
	Ysafe_free( lookupLabelG ) ;
    }
    /* end check */

    /**************** READ STRING TABLE FIRST ****************/
    sprintf(fileName,"%s.%d",symbFileG,count) ;
    fp = TWOPEN(fileName, "r", ABORT );

    /* find size of file stored at end of file */
    offset = - 4 ;  /* four bytes from end */
    if( fseek(fp, offset, FROMEND) == FSEEKERROR){
	fprintf( stderr, "Couldn't move to end of symbol binary file\n");
	TWCLOSE( fp ) ;
	delete_files( count ) ;
	read_max_xy(count-1) ;
	return ;
    }
    nitems = (unsigned) 1 ;
    fread( &num_charG, sizeof(unsigned int), nitems, fp ) ;
    if( num_charG <= 0){
	fprintf( stderr, "Warning[read]:no labels found\n" ) ;
	/* allocate one to show that we processed data previously */    
	lookupLabelG = (char *) Ysafe_malloc(sizeof (char) ) ;
    } else {
	lookupLabelG = (char *) Ysafe_malloc((num_charG+1) * sizeof (char) ) ;

	/* now read from beginning */
	offset = 0 ; 
	if( fseek(fp, offset, FROMSTART) == FSEEKERROR){
	    fprintf( stderr, "Couldn't move to beginning of symbol binary file\n");
	    TWCLOSE( fp ) ;
	    delete_files( count ) ;
	    read_max_xy( count-1 ) ;
	    return ;
	}
	fread( lookupLabelG, sizeof(char), num_charG, fp ) ;
    }
    TWCLOSE( fp ) ;
    /**************** END READ STRING FILE ****************/

    /* find max and min of window */
    maxNetnoG = maxxG = maxyG = INT_MIN;
    minxG = minyG = INT_MAX;


    /**************** READ CELL FILE FOR SPAN ****************/
    sprintf(fileName,"%s.%d",cellFileG,count) ;
    fp = TWOPEN(fileName, "r", ABORT );

    /* find size of file stored at end of file */
    offset = - 4 ;  /* four bytes from end */
    if( fseek(fp, offset, FROMEND) == FSEEKERROR){
	fprintf( stderr, "Couldn't move to end of cell binary file\n");
	TWCLOSE( fp ) ;
	delete_files( count ) ;
	read_max_xy(count-1) ;
	return ;
    }
    nitems = (unsigned) 1 ;
    fread( &num_cel_recG, sizeof(unsigned int), nitems, fp ) ;
    if( num_cel_recG <= 0){
	fprintf( stderr, "Warning[read]:no cells found\n" ) ;
	/* allocate one to show that we processed data previously */    
	cell_file_ptrG = (DATAPTR) 
	    Ysafe_malloc( sizeof(DATABOX) ) ;
    } else {
	cell_file_ptrG = (DATAPTR) 
	    Ysafe_malloc( (num_cel_recG)* sizeof(DATABOX) ) ;

	/* now read from beginning */
	offset = 0 ; 
	if( fseek(fp, offset, FROMSTART) == FSEEKERROR){
	    fprintf( stderr, "Couldn't move to beginning of cell binary file\n");
	    TWCLOSE( fp ) ;
	    delete_files( count ) ;
	    read_max_xy( count-1 ) ;
	    return ;
	}
	/* read all at once */
	fread( cell_file_ptrG, sizeof(DATABOX), num_cel_recG, fp ) ;
	for( i=0;i<num_cel_recG ; i++ ){
	    /* for speed */
	    ccell = cell_file_ptrG[i] ;
	    maxxG = MAX( maxxG, ccell.x1 ) ;
	    maxxG = MAX( maxxG, ccell.x2 ) ;
	    minxG = MIN( minxG, ccell.x1 ) ;
	    minxG = MIN( minxG, ccell.x2 ) ;

	    maxyG = MAX( maxyG, ccell.y1 ) ;
	    maxyG = MAX( maxyG, ccell.y2 ) ;
	    minyG = MIN( minyG, ccell.y1 ) ;
	    minyG = MIN( minyG, ccell.y2 ) ;
	}
    }
    TWCLOSE( fp ) ;
    /**************** END READ CELL FILE ****************/

    /**************** READ NET FILE FOR SPAN ****************/
    sprintf(fileName,"%s.%d",netFileG,count) ;
    fp = TWOPEN(fileName, "r", ABORT );

    /* find size of file stored at end of file */
    offset = - 4 ;  /* four bytes from end */
    if( fseek(fp, offset, FROMEND) == FSEEKERROR){
	fprintf( stderr, "Couldn't move to end of net binary file\n");
	TWCLOSE( fp ) ;
	delete_files( count ) ;
	read_max_xy( count-1 ) ;
	return ;
    }
    nitems = (unsigned) 1 ;
    fread( &num_net_recG, sizeof(unsigned int), nitems, fp ) ;
    if( num_net_recG <= 0){
	fprintf( stderr, "Warning[read]:no nets found\n" ) ;
	/* allocate one to show that we processed data previously */    
	net_file_ptrG = (DATAPTR) 
	    Ysafe_malloc( sizeof(DATABOX) ) ;
    } else {
	net_file_ptrG = (DATAPTR) 
	    Ysafe_malloc( (num_net_recG)* sizeof(DATABOX) ) ;

	/* now read from beginning */
	offset = 0 ; 
	if( fseek(fp, offset, FROMSTART) == FSEEKERROR){
	    fprintf( stderr, "Couldn't move to beginning of net binary file\n");
	    TWCLOSE( fp ) ;
	    delete_files( count ) ;
	    read_max_xy( count-1 ) ;
	    return ;
	}
	/* read everything at once */
	fread( net_file_ptrG, sizeof(DATABOX), num_net_recG, fp ) ;
	for( i=0;i<num_net_recG ; i++ ){
	    /* for speed */
	    ncell = net_file_ptrG[i] ;
	    maxxG = MAX( maxxG, ncell.x1 ) ;
	    maxxG = MAX( maxxG, ncell.x2 ) ;
	    minxG = MIN( minxG, ncell.x1 ) ;
	    minxG = MIN( minxG, ncell.x2 ) ;

	    maxyG = MAX( maxyG, ncell.y1 ) ;
	    maxyG = MAX( maxyG, ncell.y2 ) ;
	    minyG = MIN( minyG, ncell.y1 ) ;
	    minyG = MIN( minyG, ncell.y2 ) ;
	    maxNetnoG = MIN( maxNetnoG, ncell.ref ) ;
	}
    }
    TWCLOSE( fp ) ;
    /*************** END READ NET FILE *************************/


    D( "Yread_max_count", printf("[read bounding box]:minX=%d minY=%d maxX=%d maxY=%d maxNetno=%d\n",
	    minxG, minyG,maxxG,maxyG,maxNetnoG) ) ;


} /* end function find_max_xy */


/* *********************************************************************** */
/*		Misc utilities	
/* *********************************************************************** */
min_max(min,max)
int	*min,*max;
{
	if (*min>*max){
	    swap(min,max);
	}
}

swap(a,b)
int 	*a,*b;
{	int c;
	
	c = *a;	*a = *b;	*b = c;
}


delete_files( count )
INT count ;
{
    char fileName[LRECL] ;

    if( count == 0 ){
	/* no files to delete */
	M(ERRMSG, "delete_files", "Can no longer rewind\n" ) ;
	YexitPgm( FAILBINARY ) ;
    }
    sprintf(fileName,"%s.%d",symbFileG,count) ;
    Yrm_files( fileName ) ;
    sprintf(fileName,"%s.%d",cellFileG,count) ;
    Yrm_files( fileName ) ;
    sprintf(fileName,"%s.%d",netFileG,count) ;
    Yrm_files( fileName ) ;
    cell_file_ptrG = NIL(DATABOX *) ;
    net_file_ptrG = NIL(DATABOX *) ;
    lookupLabelG = NIL(char *) ;
    if( count == maxCountG ){
	maxCountG-- ;
    }
    
} /* end delete_files */

renumber_files( oldcount, newcount )
INT oldcount ;
INT newcount ;
{
    char oldfile[LRECL] ;
    char newfile[LRECL] ;

    sprintf(oldfile,"%s.%d",symbFileG,oldcount) ;
    sprintf(newfile,"%s.%d",symbFileG,newcount) ;
    YmoveFile( oldfile, newfile ) ;

    sprintf(oldfile,"%s.%d",cellFileG,oldcount) ;
    sprintf(newfile,"%s.%d",cellFileG,newcount) ;
    YmoveFile( oldfile, newfile ) ;

    sprintf(oldfile,"%s.%d",netFileG,oldcount) ;
    sprintf(newfile,"%s.%d",netFileG,newcount) ;
    YmoveFile( oldfile, newfile ) ;

} /* end renumber_files */

BOOL frame_exists( frame )
INT frame ;
{
    char filename[LRECL] ;
    sprintf(filename,"%s.%d",symbFileG,frame) ;
    if(!(YfileExists(filename) )){
	return( FALSE ) ;
    }
    sprintf(filename,"%s.%d",cellFileG,frame) ;
    if(!(YfileExists(filename) )){
	return( FALSE ) ;
    }
    sprintf(filename,"%s.%d",netFileG,frame) ;
    if(!(YfileExists(filename) )){
	return( FALSE ) ;
    }
    /* if we get here all three files exist */
    return( TRUE ) ;
} /* end frame_exists */

renumber_frames()
{
    INT i ;
    INT valid ;

    for( valid = 0, i = 1 ; i <= maxCountG ; i++ ){
	if(!(frame_exists( i ))){
	    continue ;
	}
	/* if we get here all three files exist */
	if( ++valid != i ){
	    /* we need to renumber since some frames don't exist */
	    renumber_files( i, valid ) ;
	}
    }
    maxCountG = valid ;

} /* end renumber_frames */
