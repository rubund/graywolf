/*
 *   Copyright (C) 1990-1992 Yale University
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
   FILE:	    yreadpar.c                                       
   DESCRIPTION:     Read a TimberWolf parameter file.
   CONTENTS:   
   DATE:	    Oct 24, 1990 
   REVISION:        Nov 23, 1990 - Fixed for tab spaces in USER file.
		    Dec 21, 1990 - Changed buffer to static so it 
			won't disappear upon return.
		    Sat Jan 26 16:33:03 PST 1991 - added genrows
			to list of programs.
		    Wed Jan 30 18:35:01 EST 1991 - fixed problem
			with numtokens access violation on empty lines.
		    Fri Feb 22 23:41:39 EST 1991 - added wildcard
			boolean to Yreadpar_next and renamed for Tomus.
		    Wed Apr 17 23:32:00 EDT 1991 - Added design rule
			information into the .par file.
		    Wed Jun  5 16:35:41 CDT 1991 - added overhang and
			aspect limit to design rules.  Also fixed
			Yreadpar_next problems.
		    Mon Aug 12 16:02:33 CDT 1991 - modified for new
			Yrbtree_init routine.
		    Sat Apr 18 11:29:50 EDT 1992 - added compactor
			program entry - CMPT.
   ----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) yreadpar.c (Yale) version 1.6 10/1/91" ;
#endif

#include <string.h>
#include <yalecad/file.h>
#include <yalecad/rbtree.h>
#include <yalecad/message.h>
#include <yalecad/string.h>
#include <yalecad/yreadpar.h>

#define COMMENT		'#'
#define WILDCARD	'*'
#define END(v) (v-1 + sizeof(v)/sizeof(v[0]) ) /* for table lookup */
#define CHARACTERISTIC	(DOUBLE) 1000.0 ;
#define DEFAULT_LAYERS   3
#define DEFAULT_VIAS     2

/* the rule type definitions */
#define CAPACITANCE_T    'c'
#define RESISTANCE_T     'r'
#define DIRECTION_T      'd'
#define LAYER_T          'l'
#define WIDTH_T          'w'
#define VIA_T            'v'
#define SPACING_T        's'
#define OVERHANG_T       'o'
#define ASPECTLIM_T      'a'

#define ERROR_CHECK(type)    \
if(!(layerArrayS )){     \
    M(ERRMSG,"Yreadpar_errorcheck","No rules found in parameter file\n");\
    return( (type) 0 ) ; \
}

typedef struct {
    char *rule ;
    union {
	BOOL bvalue ;
	INT ivalue ;
	DOUBLE fvalue ;
	char *svalue ;
    } value ;
} RULEBOX, *RULEPTR ;

static YTREEPTR rule_treeS ;
static INT numlayS = 0 ;
static INT numviaS = 0 ;
static INT alloc_layS = DEFAULT_LAYERS ;
static INT alloc_viaS = DEFAULT_VIAS ;
static char **layerArrayS = NIL(char **) ;
static char **viaArrayS = NIL(char **) ;

/* reserved programs binary search */
/* ----------------------------------------------------------------- 
    The following is table of the reserved words - Table must be in
    alphabetical order for binary search to work properly.
----------------------------------------------------------------- */
static struct ptable_rec {  /* reserved program table */
    char *program ;      /* program name */
    INT  program_id ;    /* program id */
    char *suffix ;       /* program suffix */
} pgmtableS[] = {
    "CMPT",	CMPT,	"cpar",
    "GENR",	GENR,	"genpar",
    "MICK",	MICK,	"gpar",
    "MINC",	MINC,	"mtpar",
    "PART",	PART,	"ppar",
    "SGGR",	SGGR,	"sgpar",
    "TWAR",	TWAR,	"apar",
    "TWMC",	TWMC,	"mpar",
    "TWSC",	TWSC,	"spar",
    "USER",	USER,	"par"
} ;

typedef struct {
    char *parameter ;       /* the parameter keyword */
    char **tokens ;         /* the values of the parameter */
    INT  numtokens ;        /* number of tokens */
    INT  program ;          /* name of program */
} PARAM, *PARAMPTR ;

static FILE *fpS = NIL(FILE *) ;
static char copy_bufferS[LRECL] ;
static char bufferS[LRECL] ;
static INT  filter_idS = 0 ;
static INT  prog_idS = 0 ;
static INT  lineS = 0 ;
static char filterNameS[5] ;
static BOOL verboseS = TRUE ;

static INT prog2id( program ) 
char *program ;
{
    INT c ;
    struct ptable_rec *low = pgmtableS,          /* ptr to beginning */
		      *mid ,  
		      *high = END(pgmtableS) ;   /* ptr to end */

    /* binary search to look thru table to find pattern match */
    while( low <= high){
	mid = low + (high-low) / 2 ;
	if( (c = strcmp(mid->program, program) ) == STRINGEQ){
	    return( mid->program_id ) ; /* return token number */
	} else if( c < 0 ){
	    low = mid + 1 ;
	} else {
	    high = mid - 1 ;
	}
    }
    /* at this point we haven't found a match so we have an error */
    return( UNKN ) ;
		
} /* end prog2id function */

static char *id2prog( id )
INT id ;
{
    if( id > 0 && id <= MAXID ){
	return( pgmtableS[id-1].program ) ;
    } else {
	return( NIL(char *) ) ; 
    }
} /* end id2prog */

static char *id2suffix( id )
INT id ;
{
    if( id > 0 && id <= MAXID ){
	return( pgmtableS[id-1].suffix ) ;
    } else {
	return( NIL(char *) ) ; 
    }
} /* end id2prog */

/* compare routine for design rule processing */
static INT compare_rule( key1, key2 )
RULEPTR key1, key2 ;
{
    return( (INT)strcmp( key1->rule, key2->rule ) ) ;
} /* end compare */

static BOOL check_layer( layer )
char *layer ;
{
    INT i ;        /* counter */

    verboseS = FALSE ;
    if( Yreadpar_layer2id( layer ) ){
	verboseS = TRUE ;
	return( FALSE ) ;
    } else {
	for( i = 1; i <= numviaS; i++ ){
	    if( strcmp( Yreadpar_viaId2name( i ), layer ) == STRINGEQ ){
		verboseS = TRUE ;
		return( FALSE ) ;
	    }
	}
    }
    sprintf( YmsgG, 
	"Layer or via:%s has not been defined.  Rule ignored.\n", layer) ;
    M( ERRMSG, "check_layer", YmsgG ) ;
    verboseS = TRUE ;
    return( TRUE ) ;
} /* end check_layer */


static char *make_data( rule, value, type )
char *rule ;
char *value ;
char type ;
{
    RULEPTR data ;
    RULEBOX data_buffer ;
    DOUBLE  fvalue ;   
    INT     data_value ;
    char    key[LRECL] ;

    /* make the key */
    sprintf( key, "%s:%c", rule, type ) ;
    data_buffer.rule = key ;

    /* see if key already exists */
    if( Yrbtree_search( rule_treeS, (char *) &(data_buffer) ) ){
	sprintf( YmsgG, "Rule on line:%d already exists. Ignored.\n", lineS ) ;
	M( ERRMSG, "Yreadpar_init", YmsgG ) ;
	return( NIL(char *) ) ;
    }

    data = YMALLOC( 1, RULEBOX ) ;
    data->rule = Ystrclone( key ) ;

    switch( type ){
    case CAPACITANCE_T:
    case RESISTANCE_T:
	data->value.fvalue = atof( value ) ;
	break ;
    case DIRECTION_T:
	/* use the HnotV convention */
	if( strcmp( value, "vertical" ) == STRINGEQ ){
	    data->value.bvalue = FALSE ;
	} else if( strcmp( value, "horizontal" ) == STRINGEQ ){
	    data->value.bvalue = TRUE ;
	} else {
	    M( ERRMSG, "Yreadpar_init", "Unknown layer direction\n" ) ;
	}
	break ;
    case LAYER_T:
	data->value.ivalue = (INT) value ;
	break ;
    case WIDTH_T:
	/* now calculate value */
	fvalue = atof( value ) ;

        /* This code converts the float to an integer
	fvalue *= CHARACTERISTIC ;
	data_value = ROUND( fvalue ) ;
	data->value.ivalue = data_value ;
        */

        data->value.fvalue = fvalue ; 
       
	break ;
    case ASPECTLIM_T:
	data->value.fvalue = atof( value ) ;
	break ;
    } /* end switch on type */

    return( (char *) data ) ;

} /* end make_data */

static char *make_data2( object1, object2, value, type )
char *object1 ;
char *object2 ;
char *value ;
char type ;
{
    RULEPTR data ;
    RULEBOX data_buffer ;
    DOUBLE  fvalue ;   
    INT     data_value ;
    char    key[LRECL] ;


    /* make the key order the key alphabetically */
    if( strcmp( object1, object2 ) <= STRINGEQ ){
	sprintf( key, "%s/%s:%c", object1, object2, type ) ;
    } else {
	sprintf( key, "%s/%s:%c", object2, object1, type ) ;
    }
    data_buffer.rule = key ;

    /* see if key already exists */
    if( Yrbtree_search( rule_treeS, (char *) &(data_buffer) ) ){
	sprintf( YmsgG, "Rule on line:%d already exists. Ignored.\n", lineS ) ;
	M( ERRMSG, "Yreadpar_init", YmsgG ) ;
	return( NIL(char *) ) ;
    }

    data = YMALLOC( 1, RULEBOX ) ;
    data->rule = Ystrclone( key ) ;

    switch( type ){
    case VIA_T:
	data->value.svalue = Ystrclone( value ) ;
	break ;
    case SPACING_T:
	/* now calculate value */
	fvalue = atof( value ) ;

        /* This code converts the float to an integer
	fvalue *= CHARACTERISTIC ;
	data_value = ROUND( fvalue ) ;
	data->value.ivalue = data_value ;
        */

        data->value.fvalue = fvalue ;

	break ;
    }
    return( (char *) data ) ;
} /* end make_data_spacing */


VOID Yreadpar_init( design_name, parfile, filter, abortFlag )
char *design_name ;
INT  parfile ;
INT  filter ;
BOOL abortFlag ;
{
    char *suffix ;              /* parameter file suffix */
    char *pname ;               /* program name of filter */
    char filename[LRECL] ;      /* the file name */
    char *bufferptr ;
    char **tokens ;
    INT  i ;                    /* counter */
    INT  numtokens ;            /* number of tokens on a line */
    BOOL rule_section ;         /* true if rule section exists */
    char *data ;                /* rule data to be stored */
    static BOOL rules_unreadL=TRUE; /* false after the rules have been read */

    if( suffix = id2suffix( parfile ) ){
	prog_idS = parfile ;
	sprintf( filename, "%s.%s", design_name, suffix ) ;
	if( pname = id2prog( filter )){
	    filter_idS = filter ;
	    strcpy( filterNameS, pname ) ;
	} else {
	    M( ERRMSG, "Yreadpar_init", "Unknown filter program\n" ) ;
	}
    } else {
	M( ERRMSG, "Yreadpar_init", "Unknown parameter file\n" ) ;
	return ;
    } /* end switch on program */

    /* now that we have the file name open the par file */
    if(fpS){
	M( ERRMSG, "Yreadpar_init", 
	    "Only one par file may be read at the same time\n" ) ;
	return ;
    }
    fpS = TWOPEN( filename, "r", abortFlag ) ;
    if(!(fpS) ){
	return ;
    }
    lineS = 0 ;
    rule_section = FALSE ;


    /* TRY to read the RULES SECTION of the parameter file */
    while( bufferptr = fgets( bufferS, LRECL, fpS ) ){
	lineS++ ;
	/* remove leading blanks */
	bufferptr = Yremove_lblanks( bufferptr ) ;
	if( *bufferptr == '#' ){
	    /* skip over comments */
	    continue ;
	}
	tokens = Ystrparser( bufferptr, " \t\n", &numtokens ) ;
	if( numtokens == 0 ){
	    continue ;
	}
	if( strcmp( tokens[0], "RULES" ) == STRINGEQ ){
	    rule_section = TRUE ;
	    /* initialize tree of rules */
	    rule_treeS = Yrbtree_init( compare_rule ) ;
	    /* initialize the number of layers */
	    layerArrayS = YVECTOR_CALLOC( 1,alloc_layS,char *) ;
	    viaArrayS =   YVECTOR_CALLOC( 1,alloc_viaS,char *) ;
	    continue ;
	} else if( strcmp( tokens[0], "ENDRULES" ) == STRINGEQ ){
	    rules_unreadL = FALSE ;
	    break ;
	}
	if( rule_section && rules_unreadL ){
	    if( strcmp( tokens[0], "width" ) == STRINGEQ &&
		numtokens == 3 ){
		if( check_layer( tokens[1] ) ){
		    continue ;
		}
		data = make_data( tokens[1], tokens[2], WIDTH_T ) ;
		if( data ){
		    Yrbtree_insert( rule_treeS, data ) ;
		}
	    } else if( strcmp( tokens[0], "spacing" ) == STRINGEQ &&
		numtokens == 4 ){
		if( check_layer( tokens[1] ) ){
		    continue ;
		}
		if( check_layer( tokens[2] ) ){
		    continue ;
		}
		data = make_data2( tokens[1], tokens[2], tokens[3], SPACING_T ) ;
		if( data ){
		    Yrbtree_insert( rule_treeS, data ) ;
		}
	    } else if( strcmp( tokens[0], "via" ) == STRINGEQ &&
		(numtokens == 4 || numtokens == 5) ){
		if( ++numviaS >= alloc_viaS ){
		    /* need to expand the number of vias allocated */
		    alloc_viaS = numviaS + 3 ;
		    viaArrayS = YVECTOR_REALLOC( viaArrayS,1,alloc_viaS,char * ) ;
		    for( i = numviaS; i <= alloc_viaS; i++ ){
			viaArrayS[i] = NIL(char *) ;
		    }

		}
		/* at this point safe to use vianame */
		/* we need two mechanisms : viaid to vianame - use array */
		/* and vianame to viaid - use balanced tree */
		viaArrayS[numviaS] = Ystrclone( tokens[1] ) ;
		data = make_data2( tokens[2], tokens[3], tokens[1], VIA_T ) ;
		if( data ){
		    Yrbtree_insert( rule_treeS, data ) ;
		}
		if( numtokens == 5 ){
		    data = make_data( tokens[1], tokens[4], ASPECTLIM_T ) ;
		} else {
		    /* set the default to 1.0 for aspect ratio limit */
		    data = make_data( tokens[1], "1.0", ASPECTLIM_T ) ;
		}
		if( data ) Yrbtree_insert( rule_treeS, data ) ;

	    } else if( strncmp( tokens[0], "layer",5 ) == STRINGEQ &&
		numtokens == 5 ){
		if( ++numlayS >= alloc_layS ){
		    /* need to expand the number of layers allocated */
		    alloc_layS = numlayS + 3 ;
		    layerArrayS = YVECTOR_REALLOC(layerArrayS,1,alloc_layS,char*);
		    for( i = numlayS; i <= alloc_layS; i++ ){
			layerArrayS[i] = NIL(char *) ;
		    }

		}
		/* at this point safe to use layer */
		/* we need two mechanisms : layerid to layername - use array */
		/* and layername to layerid - use balanced tree */
		layerArrayS[numlayS] = Ystrclone( tokens[1] ) ;
		data = make_data( tokens[1], (char *) numlayS, LAYER_T ) ;
		if( data ) Yrbtree_insert( rule_treeS, data ) ;
		/* store the values of res, cap, and routing preference */
		data = make_data( tokens[1], tokens[2], RESISTANCE_T ) ;
		if( data ) Yrbtree_insert( rule_treeS, data ) ;
		data = make_data( tokens[1], tokens[3], CAPACITANCE_T ) ;
		if( data ) Yrbtree_insert( rule_treeS, data ) ;
		data = make_data( tokens[1], tokens[4], DIRECTION_T ) ;
		if( data ) Yrbtree_insert( rule_treeS, data ) ;

	    } else if( strncmp( tokens[0], "overhang",8 ) == STRINGEQ &&
		numtokens == 4 ){
		data = make_data2( tokens[1], tokens[2], tokens[3], OVERHANG_T ) ;
		if( data ) Yrbtree_insert( rule_treeS, data ) ;

	    } else {
		sprintf( YmsgG, "Unknown rule at line:%d in filename:%s\n", 
		    lineS, filename ) ;
		M( ERRMSG, "Yreadpar_init", YmsgG ) ;
	    }

	}/* end processing of rule section */ 

    } /* end while */

    if(!(rule_section)){
	rewind(fpS) ;
	lineS = 0 ;
    }

} /* end Yreadpar_init */

char **Yreadpar_next( lineptr, line, numtokens, onNotOff, wildcard )
char **lineptr ;
INT *line ;
INT  *numtokens ;
BOOL *onNotOff ;
BOOL *wildcard ;
{
    PARAMPTR data ;
    char *bufferptr ;
    char **tokens ;
    BOOL rule_section ;


    if( filter_idS == UNKN || !(fpS) ){
	/* don't read an unknown format */
	return( NIL(char **) ) ;
    }
    /* assume that every option is on */
    *onNotOff = TRUE ; 
    rule_section = FALSE ;
    while( bufferptr = fgets( bufferS, LRECL, fpS )){
	/* parse file */
	lineS++ ;
	*line = lineS ;
	/* remove leading blanks */
	bufferptr = Yremove_lblanks( bufferptr ) ;
	/* skip comments */
	if( *bufferptr == COMMENT ){
	    continue ;
	}
	if( strncmp( bufferptr, "RULES", 5 ) == STRINGEQ ){
	    rule_section = TRUE ;
	    /* skip over rules section */
	    continue ;
	} else if( strncmp( bufferptr, "ENDRULES", 8 ) == STRINGEQ ){
	    rule_section = FALSE ;
	    /* skip over rules section */
	    continue ;
	} else if( rule_section ){
	    continue ;
	}
	*wildcard = FALSE ;
	strcpy( copy_bufferS, bufferptr ) ;
	*lineptr = copy_bufferS ;
	if( prog_idS == USER ){
	    bufferptr = Yremove_lblanks( bufferptr ) ;
	    if( *bufferptr != WILDCARD ){
		tokens = Ystrparser( bufferptr, "*\n", numtokens ) ;
		if( *numtokens == 2 ){
		    /* look for match for this program */
		    if( strcmp( tokens[0], filterNameS ) != STRINGEQ ){
			/* go on to the next line */
			if( prog_idS == USER ){
			    continue ;
			}
		    }
		    bufferptr = tokens[1] ;
		} else if( *numtokens == 0 ){
		    continue ;
		} else {
		    sprintf( YmsgG,
			"Trouble parsing line:%d :\n\t%s\n", lineS,
			copy_bufferS ) ;
		    M( ERRMSG, "Yreadpar_next", YmsgG ) ;
		    continue ;
		}
	    } else {
		/* WILDCARD skip over it */
		*wildcard = TRUE ;
		bufferptr++ ;
	    }
	}
	tokens = Ystrparser( bufferptr, " :\t\n", numtokens ) ;
	if( *numtokens > 1 ){
	    if( strcmp( tokens[*numtokens-1], "off" ) == STRINGEQ ){
		/* we have turned this value off */
		*onNotOff = FALSE ;
	    }
	}
	return( tokens ) ;
    }
    TWCLOSE( fpS ) ;
    fpS = NIL( FILE * ) ;
    return( NIL(char **) ) ;

} /* end Yreadpar_next */

static INT compare_parameter( key1, key2 )
PARAMPTR key1, key2 ;
{
    return( (INT)strcmp( key1->parameter, key2->parameter ) ) ;
} /* end compare */

YPARPTR Yreadpar_file()
{
    PARAMPTR data ;
    YPARPTR  store ;              /* a tree with all the parameters */
    char *lineptr ;
    char **tokens ;
    INT  numtokens ;
    INT  line ;
    INT  i ;                     /* counter */
    BOOL onNotOff ;
    BOOL wildcard ;

    store = (YPARPTR) NULL ;

    if( prog_idS == UNKN || !(fpS) ){
	/* don't read an unknown format */
	return( store ) ;
    }

    /* initialize a tree which store the data found in the *par file*/
    YRBTREE_INIT( store, compare_parameter );

    /* assume that every option is on */
    while( tokens = Yreadpar_next( &lineptr, &line, &numtokens,
	&onNotOff, &wildcard )){
	if( numtokens == 0 ){
	    continue ;
	}
	if( numtokens >= 2 ){
	    /* copy tokens for storing in a tree */
	    data = YMALLOC( 1, PARAM ) ;
	    data->parameter = Ystrclone( tokens[0] ) ;
	    data->program = filter_idS ;
	    data->tokens = YMALLOC( numtokens-1, char * ) ;
	    for( i = 1; i < numtokens ; i++ ){
		data->tokens[i-1] = Ystrclone( tokens[i] ) ;
	    }
	    data->numtokens = numtokens - 1 ; /* save rest of line */
	    /* now write the tokens */
	    Yrbtree_insert( store, data ) ;
	} else {
	    sprintf( YmsgG,
		"Trouble parsing line:%d :\n\t%s\n", line,
		lineptr ) ;
	    M( ERRMSG, "Yreadpar_file", YmsgG ) ;
	    continue ;
	}
    } /* end while loop */

    TWCLOSE( fpS ) ;
    fpS = NIL( FILE * ) ;
    return( store ) ;

} /* end Yreadpar_file */

char **Yreadpar_lookup( par_object, param, program, numtokens )
YPARPTR par_object ;          /* parameter tree */
char *param ;                 /* parameter */
INT program ;                 /* program id */
INT *numtokens ;              /* returns the number of tokens for parameter */
{
    PARAMPTR data ;           /* store this in the tree */
    PARAM key ;

    *numtokens = 0 ;
    key.parameter = param ;
    data = (PARAMPTR) Yrbtree_interval(par_object,&key,&key,TRUE) ;
    while( data ){
	if( data->program == program ){
	    *numtokens = data->numtokens ;
	    return( data->tokens ) ;
	}
	data = (PARAMPTR) Yrbtree_interval(par_object,&key,&key,FALSE) ;
    }
    /* if we get to this point we didn't find a match */
    return( NIL(char **) ) ;
} /* end Yreadpar_lookup */

DOUBLE Yreadpar_spacing( object1, object2 )
char *object1, *object2 ;
{
    char  key[LRECL] ;
    RULEPTR data ;
    RULEBOX data_buffer ;

    ERROR_CHECK(DOUBLE) ;
    /* first build the key */
    sprintf( key, "%s/%s:%c", object1, object2, SPACING_T ) ;
    data_buffer.rule = key ;

    if( data = (RULEPTR) Yrbtree_search( rule_treeS, (char *) &(data_buffer) ) ){
	return( data->value.fvalue ) ;
    } else if( verboseS ){
	sprintf( YmsgG, 
	    "Spacing for %s to %s not defined in the given design rules.\n",
	    object1, object2 );
	M( ERRMSG, "Yreadpar_spacing", YmsgG ) ;
	M( ERRMSG, NULL, "\tDefaulting to zero.\n\n" ) ;
	return( 0 ) ;
    }

} /* end Yreadpar_spacing */

DOUBLE Yreadpar_width( object )
char *object ;
{
    char  key[LRECL] ;
    RULEPTR data ;
    RULEBOX data_buffer ;

    ERROR_CHECK(DOUBLE) ;
    /* first build the key */
    sprintf( key, "%s:%c", object, WIDTH_T ) ;
    data_buffer.rule = key ;
    if( data = (RULEPTR) Yrbtree_search( rule_treeS, (char *) &(data_buffer) ) ){
	return( data->value.fvalue ) ;
    } else {
	sprintf( YmsgG, 
	    "Width for %s not defined in the given design rules.\n",
	    object );
	M( ERRMSG, "Yreadpar_width", YmsgG ) ;
	M( ERRMSG, NULL, "\tDefaulting to zero.\n\n" ) ;
	return( 0 ) ;
    }

} /* end Yreadpar_spacing */

DOUBLE Yreadpar_pitch( object )
char *object ;
{
    DOUBLE spacing ;
    DOUBLE width ;

    /* pitch for a given layer is spacing plus width */

    ERROR_CHECK(DOUBLE) ;
    spacing = Yreadpar_spacing( object, object ) ;
    width = Yreadpar_width( object ) ;
    return( spacing + width ) ;
} /* end Yreadpar_pitch */

DOUBLE Yreadpar_layer_res( object )
char *object ;
{
    char  key[LRECL] ;
    RULEPTR data ;
    RULEBOX data_buffer ;

    ERROR_CHECK(DOUBLE) ;
    /* first build the key */
    sprintf( key, "%s:%c", object, RESISTANCE_T ) ;
    data_buffer.rule = key ;
    if( data = (RULEPTR) Yrbtree_search( rule_treeS, (char *) &(data_buffer) ) ){
	return( data->value.fvalue ) ;
    } else if( verboseS ){
	sprintf( YmsgG, 
	    "Resistance for layer %s not defined in the given design rules.\n",
	    object );
	M( ERRMSG, "Yreadpar_layer_res", YmsgG ) ;
	M( ERRMSG, NULL, "\tDefaulting to zero.\n\n" ) ;
	return( 0 ) ;
    }

} /* end Yreadpar_layer_res */

DOUBLE Yreadpar_layer_cap( object )
char *object ;
{
    char  key[LRECL] ;
    RULEPTR data ;
    RULEBOX data_buffer ;

    ERROR_CHECK(DOUBLE) ;
    /* first build the key */
    sprintf( key, "%s:%c", object, CAPACITANCE_T ) ;
    data_buffer.rule = key ;
    if( data = (RULEPTR) Yrbtree_search( rule_treeS, (char *) &(data_buffer) ) ){
	return( data->value.fvalue ) ;
    } else if( verboseS ){
	sprintf( YmsgG, 
	    "Capacitance for layer %s defined in the given design rules.\n",
	    object );
	M( ERRMSG, "Yreadpar_layer_cap", YmsgG ) ;
	M( ERRMSG, NULL, "\tDefaulting to zero.\n\n" ) ;
	return( (DOUBLE) 0.0 ) ;
    }

} /* end Yreadpar_layer_cap */

BOOL Yreadpar_layer_HnotV( object )
char *object ;
{
    char  key[LRECL] ;
    char  *keyptr ;
    RULEPTR data ;
    RULEBOX data_buffer ;

    ERROR_CHECK(BOOL) ;
    /* first build the key */
    sprintf( key, "%s:%c", object, DIRECTION_T ) ;
    data_buffer.rule = key ;
    if( data = (RULEPTR) Yrbtree_search( rule_treeS, (char *) &(data_buffer) ) ){
	return( data->value.bvalue ) ;
    } else if( verboseS ){
	sprintf( YmsgG, 
	    "Routing direction for layer %s defined in the given design rules.\n",
	    object );
	M( ERRMSG, "Yreadpar_layer_HnotV", YmsgG ) ;
	M( ERRMSG, NULL, "\tDefaulting to horizontal.\n\n" ) ;
	return( TRUE ) ;
    }

} /* end Yreadpar_layer_HnotV */

INT Yreadpar_layer2id( object )
char *object ;
{
    char  key[LRECL] ;
    RULEPTR data ;
    RULEBOX data_buffer ;

    ERROR_CHECK(INT) ;
    /* first build the key */
    sprintf( key, "%s:%c", object, LAYER_T ) ;
    data_buffer.rule = key ;
    if( data = (RULEPTR) Yrbtree_search( rule_treeS, (char *) &(data_buffer) ) ){
	return( data->value.ivalue ) ;
    } else if( verboseS ){
	sprintf( YmsgG, 
	    "Routing layer %s not defined in the given design rules.\n",
	    object );
	M( ERRMSG, "Yreadpar_layer2id", YmsgG ) ;
	M( ERRMSG, NULL, "\tDefaulting to horizontal.\n\n" ) ;
	return( TRUE ) ;
    }

} /* end Yreadpar_layer2id */

char *Yreadpar_id2layer( layerid )
INT layerid ;
{
    ERROR_CHECK(char*) ;
    if( layerid > 0 && layerid <= numlayS ){
	return( layerArrayS[layerid] ) ;
    } else {
	sprintf( YmsgG, 
	    "Layer id:%d is out of bounds. Numlayers = %d\n",
	    layerid, numlayS ) ;
	M( ERRMSG, "Yreadpar_id2layer", YmsgG ) ;
	M( ERRMSG, NULL, "\tDefaulting to layer 1.\n\n" ) ;
	return( layerArrayS[1] ) ;
    }

} /* end Yreadpar_id2layer */

INT Yreadpar_numlayers()
{
    return( numlayS ) ;
} /* end Yreadpar_numlayers */

char *Yreadpar_vianame( object1, object2 )
char *object1, *object2 ;
{
    char  key[LRECL] ;
    RULEPTR data ;
    RULEBOX data_buffer ;

    ERROR_CHECK(char*) ;
    /* first build the key */
    data_buffer.rule = key ;
    /* make the key order the key alphabetically */
    if( strcmp( object1, object2 ) <= STRINGEQ ){
	sprintf( key, "%s/%s:%c", object1, object2, VIA_T ) ;
    } else {
	sprintf( key, "%s/%s:%c", object2, object1, VIA_T ) ;
    }
    if( data = (RULEPTR) Yrbtree_search( rule_treeS, (char *) &(data_buffer) ) ){
	return( data->value.svalue ) ;
    } else if( verboseS ){
	sprintf( YmsgG, 
	    "Could not find a via between layers %s and %s.\n",
	    object1, object2 );
	M( ERRMSG, "Yreadpar_vianame", YmsgG ) ;
	return( NIL(char *) ) ;
    }

} /* end Yreadpar_vianame */

char *Yreadpar_viaId2name( viaid )
INT viaid ;
{
    ERROR_CHECK(char*) ;
    if( viaid > 0 && viaid <= numviaS ){
	return( viaArrayS[viaid] ) ;
    } else {
	sprintf( YmsgG, 
	    "Via id:%d is out of bounds. Numvias = %d\n",
	    viaid, numviaS ) ;
	M( ERRMSG, "Yreadpar_viaId2name", YmsgG ) ;
	M( ERRMSG, NULL, "\tDefaulting to via id 1.\n\n" ) ;
	return( viaArrayS[1] ) ;
    }

} /* end Yreadpar_viaId2name */
