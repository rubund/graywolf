/*
 *   Copyright (C) 1988-1991 Yale University
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

#ifndef lint
static char yysccsid[] = "@(#)yaccpar	1.8 (Berkeley) 01/20/90";
#endif
#define YYBYACC 1
/* ----------------------------------------------------------------- 
FILE:	    readnets.c <- readnets_yacc <- readnets_lex
DESCRIPTION:This file contains the grammar (BNF) for the TimberWolfMC
	    input file parser for nets. The rules for lex are in 
	    readnets_lex.  The grammar for yacc is in readnets_yacc.
	    The output of yacc (y.tab.c) is renamed to readnets.c
CONTENTS:   
	    readnets( fp )
		FILE *fp ;
	    yyerror(s)
		char    *s;
	    yywrap()
	
	    Note:readnets.c will also include the yacc parser.
DATE:	    Aug  7, 1988 
REVISIONS:  Jan 29, 1989 - changed to msgG and added \n's.
	    Mar 01, 1989 - modified argument to YexitPgm.
	    Mar 11, 1989 - added get_total_paths for new print_paths.
	    Mar 16, 1989 - changed data structure of netlist.
	    Thu Dec 20 00:16:51 EST 1990 - added check_nets to 
		make sure constraints are consistent.
	    Sun Jan 20 21:47:52 PST 1991 - ported to AIX.
	    Thu Mar 14 16:39:01 CST 1991 - fixed crash in bad_nets.
	    Thu Apr 18 01:55:39 EDT 1991 - added debug function
		and fixed for new library names.
----------------------------------------------------------------- */
#ifndef VMS
#ifndef lint
static char SccsId[] = "@(#) readnets.y (Yale) version 4.12 4/21/91" ;
#endif
#endif

#include <yalecad/message.h>
#include <yalecad/debug.h>
#include <yalecad/hash.h>
#include "readnets.h"  /* redefine yacc and lex globals */
#include "standard.h"  
#include "main.h"  

#undef  REJECT         /* undefine TimberWolfSC definition */
#define STARTPATH  1   /* flag for start of path */
#define CONTPATH   0   /* flag for continuing path */

#ifdef DEBUG
#define YYDEBUG  1          /* condition compile for yacc debug */
#endif /* DEBUG */

static INT line_countS ;
static INT netS ;      /* current net being processed */
static char bufferS[LRECL] ;
static BOOL abortFlagS = FALSE ;
static INT total_num_pathS = 0 ;
static GLISTPTR netPtrS ;
static YHASHPTR net_hash_tableS ;
static PATHPTR pathPtrS = NULL ;  /* start of path list */
static PATHPTR curPathS ; /* current bottom of path list so list */
			/* is in order given by user debug easier */

typedef union {
    INT ival ;
    char *string ;
    double fval ;
} YYSTYPE;
#define INTEGER 257
#define STRING 258
#define FLOAT 259
#define ALLNETS 260
#define COLON 261
#define HVWEIGHTS 262
#define PATH 263
#define IGNORE 264
#define DONTGLOBALROUTE 265
#define NET 266
#define YYERRCODE 256
short yylhs[] = {                                        -1,
    0,    0,    2,    2,    3,    3,    6,    6,    7,    7,
    5,    4,    4,    8,    9,    9,    1,    1,    1,
};
short yylen[] = {                                         2,
    0,    1,    1,    2,    1,    2,    1,    2,    1,    1,
    2,    5,    4,    2,    1,    2,    1,    1,    1,
};
short yydefred[] = {                                      0,
    0,    0,    0,    0,    3,    5,    0,    0,   18,   17,
   19,   15,    0,   11,    4,    9,   10,    0,    7,    0,
   16,    8,    0,    0,   12,
};
short yydgoto[] = {                                       3,
   12,    4,    5,    6,    7,   18,   19,    8,   13,
};
short yysindex[] = {                                   -263,
 -253, -249,    0, -263,    0,    0, -257, -251,    0,    0,
    0,    0, -253,    0,    0,    0,    0, -257,    0, -246,
    0,    0, -245, -244,    0,
};
short yyrindex[] = {                                     14,
    0,    0,    0,   15,    0,    0,    0,    0,    0,    0,
    0,    0, -243,    0,    0,    0,    0,    1,    0,    0,
    0,    0,    0,    2,    0,
};
short yygindex[] = {                                      0,
    3,    0,   13,    0,    0,    0,    4,    0,    0,
};
#define YYTABLESIZE 268
short yytable[] = {                                       1,
    6,   13,    2,    9,   10,   11,   16,   17,   14,   20,
   23,   24,   25,    1,    2,   21,   15,   14,    0,    0,
    0,   22,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    6,   13,    0,    6,   13,
};
short yycheck[] = {                                     263,
    0,    0,  266,  257,  258,  259,  264,  265,  258,  261,
  257,  257,  257,    0,    0,   13,    4,  261,   -1,   -1,
   -1,   18,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,  263,  263,   -1,  266,  266,
};
#define YYFINAL 3
#ifndef YYDEBUG
#define YYDEBUG 0
#endif
#define YYMAXTOKEN 266
#if YYDEBUG
char *yyname[] = {
"end-of-file",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"INTEGER","STRING","FLOAT",
"ALLNETS","COLON","HVWEIGHTS","PATH","IGNORE","DONTGLOBALROUTE","NET",
};
char *yyrule[] = {
"$accept : start_nets",
"start_nets :",
"start_nets : list_of_paths_n_nets",
"list_of_paths_n_nets : path_or_net",
"list_of_paths_n_nets : list_of_paths_n_nets path_or_net",
"path_or_net : single_path",
"path_or_net : net_record net_options",
"net_options : list_of_options",
"net_options : net_options list_of_options",
"list_of_options : IGNORE",
"list_of_options : DONTGLOBALROUTE",
"net_record : NET STRING",
"single_path : pathlist COLON INTEGER INTEGER INTEGER",
"single_path : pathlist COLON INTEGER INTEGER",
"pathlist : PATH netlist",
"netlist : string",
"netlist : netlist string",
"string : STRING",
"string : INTEGER",
"string : FLOAT",
};
#endif
#define yyclearin (yychar=(-1))
#define yyerrok (yyerrflag=0)
#ifdef YYSTACKSIZE
#ifndef YYMAXDEPTH
#define YYMAXDEPTH YYSTACKSIZE
#endif
#else
#ifdef YYMAXDEPTH
#define YYSTACKSIZE YYMAXDEPTH
#else
#define YYSTACKSIZE 500
#define YYMAXDEPTH 500
#endif
#endif
int yydebug;
int yynerrs;
int yyerrflag;
int yychar;
short *yyssp;
YYSTYPE *yyvsp;
YYSTYPE yyval;
YYSTYPE yylval;
short yyss[YYSTACKSIZE];
YYSTYPE yyvs[YYSTACKSIZE];
#define yystacksize YYSTACKSIZE

#include "readnets_l.h"
/* add readnets_l.h for debug purposes */
/* ********************* #include "readnets_l.h" *******************/
/* ********************* #include "readnets_l.h" *******************/


static free_net_data();
static bad_net();
char *Ystrclone();


readnets( fp )
FILE *fp ;
{ 
    /* static free_net_data() ; */
    YHASHPTR get_net_table() ;
#ifdef YYDEBUG
    extern int yydebug ;
    yydebug = FALSE ;
    D( "twsc/readnets",
	yydebug = TRUE ;
    ) ;
#endif

    yyin = fp ;
    yyout = stdout ;
    line_countS = 0 ;
    numpathsG = 0 ;
    net_hash_tableS = get_net_table() ;
    /* parse input file using yacc */
    if( fp ) {
	yyparse();  
    }
    check_paths() ;
    if( abortFlagS ){
	YexitPgm( PGMFAIL ) ;
    }
    build_path_array() ;
    init_path_set() ;
    init_net_set() ;
    add_paths_to_cells() ;
    /* free hash table */
    Yhash_table_delete( net_hash_tableS , free_net_data ) ;

} /* end readnets */

static free_net_data( data )
INT *data ;
{
    Ysafe_free( data ) ;
} /* free_swap_data */

process_net_rec( netname ) 
char *netname ;
{
    INT *data ;

    if(!(data = (INT *) Yhash_search( net_hash_tableS, netname, NULL, FIND ))){
	bad_net( netname, FALSE ) ; /* not fatal */
	netS = 0 ;
    } else {
	netS = *data ;
    }
    
} /* end process_net_rec */

ignore_net()
{
    if( netS ){
	netarrayG[netS]->ignore = 1 ;
    }
} /* end ignore_net */

ignore_route()
{
    if( netS ){
	netarrayG[netS]->ignore = -1 ;
    }
} /* end ignore_route */

yyerror(s)
char    *s;
{
    sprintf(YmsgG,"problem reading %s.net:", cktNameG );
    M( ERRMSG, "yacc", YmsgG ) ;
    sprintf(YmsgG, "  line %d near '%s' : %s\n" ,
	line_countS+1, yytext, s );
    M( ERRMSG,NULL, YmsgG ) ;
    Ymessage_error_count() ;
    abortFlagS = TRUE ;
}

yywrap()
{
    return(1);
}                      

add_path( pathFlag, net )
BOOL pathFlag ;
char *net ;
{
    INT *data ;
    GLISTPTR tempNetPtr ;
    
    if( pathFlag == STARTPATH ){
	/* see if pathptr exists */
	if( pathPtrS ){
	    curPathS->next = (PATHPTR) Ysafe_malloc( sizeof(PATHBOX) ) ;
	    curPathS = curPathS->next ;
	} else { /* first path - start list */
	    curPathS = pathPtrS = (PATHPTR) Ysafe_malloc( sizeof(PATHBOX) ) ;
	}
	curPathS->next = NULL ;
	netPtrS = curPathS->nets = 
	    (GLISTPTR) Ysafe_malloc( sizeof(GLISTBOX) ) ;
	netPtrS->next = NULL ;
    } else {  /* continuing a path */
	tempNetPtr = netPtrS ;
	ASSERT( netPtrS, "add_path", "net pointer should be non-NULL" ) ;
	netPtrS = curPathS->nets = 
	    (GLISTPTR) Ysafe_malloc( sizeof(GLISTBOX) ) ;
	netPtrS->next = tempNetPtr ;
    }
    if( data = (INT *) Yhash_search( net_hash_tableS , net, NULL, FIND)){
	/* get data from field */
	netPtrS->p.net = *data ;
    } else {
	bad_net( net, TRUE ) ; /* Fatal abort */
    }
    /* string bufferS is allocated in lex */
    if( net ){
	Ysafe_free( net ) ;
    }

} /* end add_path */

end_path(lower_bound, upper_bound, priority )
INT lower_bound, upper_bound, priority ;
{
    GLISTPTR nets, path_ptr, tempPath ;
    DBOXPTR dimptr ;
    INT net_number ;

    if( abortFlagS ){
	return ;
    }

    curPathS->lower_bound = lower_bound ;
    curPathS->upper_bound = upper_bound ;
    curPathS->priority = priority ;
    total_num_pathS++ ;  /* total number of paths given */
    if( priority ){
	numpathsG++ ;  /* increment number of active paths */
	/* add path to path list in netarrayG */
	for( nets = curPathS->nets; nets ; nets = nets->next ){
	    net_number = nets->p.net ;
	    dimptr = netarrayG[net_number] ;

	    if( tempPath = dimptr->paths ){
		path_ptr = dimptr->paths = 
		(GLISTPTR) Ysafe_malloc( sizeof(GLISTBOX) ) ;
		path_ptr->next = tempPath ;
	    } else {  /* start a new list */
		path_ptr = dimptr->paths = 
		(GLISTPTR) Ysafe_malloc( sizeof(GLISTBOX) ) ;
		path_ptr->next = NULL ;
	    }
	    /* use total_num_pathS so we can index patharrayG */
	    path_ptr->p.path = total_num_pathS ;

	}
    }
	
} /* end function end_path */

check_paths()
{
    INT i ;             /* counter */
    DBOXPTR nptr ;      /* traverse the nets */

    /* first make sure that the nets in paths are not ignored */
    for( i = 1; i <= numnetsG; i++ ){
	nptr = netarrayG[i] ;
	if( nptr->paths && nptr->ignore ){
	    sprintf( YmsgG, 
		"Net:%s is specified in a path and ignored\n",
		    nptr->name ) ;
	    M( ERRMSG, "check_paths", YmsgG ) ;
	    abortFlagS = TRUE ;
	}
    }
} /* check_paths */


build_path_array()
{
    INT i ;
    PATHPTR curPtr ;

    
    patharrayG = (PATHPTR *) 
	Ysafe_malloc( (total_num_pathS+1)*sizeof(PATHPTR) ) ;
    i = 1 ;
    for( curPtr = pathPtrS; curPtr ; curPtr = curPtr->next ){
	patharrayG[i++] = curPtr ;
    }
} /* end build_path_array */

PATHPTR get_path_list()
{
    return( pathPtrS ) ;
} /* end get_path_list */

INT get_total_paths()
{
    return( total_num_pathS ) ;
} /* end get_total_paths */

add_paths_to_cells()
{
    INT i ;
    INT net_number ;
    INT total_cells ;
    PSETPTR pathlist, enum_path_set() ;
    CBOXPTR ptr ;
    GLISTPTR  path_ptr, tempPath ;
    DBOXPTR dimptr ;
    PINBOXPTR pinptr ;

    for( i=1;i<=lastpadG; i++ ){
	
	ptr = carrayG[i] ;
	clear_path_set() ;
	/* look for the UNIQUE paths that connects to this cell */
	for(pinptr=ptr->pins;pinptr;pinptr=pinptr->nextpin){
	    net_number = pinptr->net ;

	    /* now go to net array */
	    dimptr = netarrayG[net_number] ;

	    /* look at all paths that use this net */
	    for( path_ptr=dimptr->paths;path_ptr;path_ptr=path_ptr->next){
		add2path_set( path_ptr->p.path ) ;
	    }
	}
	/* now add UNIQUE list of paths to this cell */
	for( pathlist=enum_path_set(); pathlist; pathlist=pathlist->next){
	    if( tempPath = ptr->paths ){
		path_ptr = ptr->paths = 
		    (GLISTPTR) Ysafe_malloc( sizeof(GLISTBOX) ) ;
		path_ptr->next = tempPath ;
	    } else {  /* start a new list */
		path_ptr = ptr->paths = 
		    (GLISTPTR) Ysafe_malloc( sizeof(GLISTBOX) ) ;
		path_ptr->next = NULL ;
	    }
	    /* copy path to cell list of paths */
	    path_ptr->p.path = pathlist->path ;
	}
    }
}

static bad_net( net, fatal )
char *net ;
BOOL fatal ;
{
    sprintf( YmsgG, "The net named: %s  in the .net file ", net );
    strcat( YmsgG, "was not encountered\n" ) ;
    fprintf( fpoG, "%s ", YmsgG ) ;
    if( fatal ){
	M( ERRMSG, "add_path", YmsgG ) ;
	strcpy( YmsgG,"\twhile reading the .cel file --- FATAL error\n") ;
	abortFlagS = TRUE ;
	M( ERRMSG, NULL, YmsgG ) ;
    } else {
	M( WARNMSG, "add_path", YmsgG ) ;
	strcpy( YmsgG, 
	"\twhile reading the .cel file --- net constraint ignored.\n") ;
	M( WARNMSG, NULL, YmsgG ) ;
    }
    fprintf( fpoG, "%s ", YmsgG ) ;
} /* end bad_net */
#define YYABORT goto yyabort
#define YYACCEPT goto yyaccept
#define YYERROR goto yyerrlab
int
yyparse()
{
    register int yym, yyn, yystate;
#if YYDEBUG
    register char *yys;
    extern char *getenv();

    if (yys = getenv("YYDEBUG"))
    {
        yyn = *yys;
        if (yyn >= '0' && yyn <= '9')
            yydebug = yyn - '0';
    }
#endif

    yynerrs = 0;
    yyerrflag = 0;
    yychar = (-1);

    yyssp = yyss;
    yyvsp = yyvs;
    *yyssp = yystate = 0;

yyloop:
    if (yyn = yydefred[yystate]) goto yyreduce;
    if (yychar < 0)
    {
        if ((yychar = yylex()) < 0) yychar = 0;
#if YYDEBUG
        if (yydebug)
        {
            yys = 0;
            if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
            if (!yys) yys = "illegal-symbol";
            printf("yydebug: state %d, reading %d (%s)\n", yystate,
                    yychar, yys);
        }
#endif
    }
    if ((yyn = yysindex[yystate]) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)
    {
#if YYDEBUG
        if (yydebug)
            printf("yydebug: state %d, shifting to state %d\n",
                    yystate, yytable[yyn]);
#endif
        if (yyssp >= yyss + yystacksize - 1)
        {
            goto yyoverflow;
        }
        *++yyssp = yystate = yytable[yyn];
        *++yyvsp = yylval;
        yychar = (-1);
        if (yyerrflag > 0)  --yyerrflag;
        goto yyloop;
    }
    if ((yyn = yyrindex[yystate]) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)
    {
        yyn = yytable[yyn];
        goto yyreduce;
    }
    if (yyerrflag) goto yyinrecovery;
#ifdef lint
    goto yynewerror;
#endif
yynewerror:
#if YYDEBUG
    {
        int test_state, i, expect, two_or_more ;
        char err_msg[BUFSIZ] ;
        if( yyname[yychar] ){
            sprintf( err_msg, "\nsyntax error - found:%s expected:",
                yyname[yychar] ) ;
            two_or_more = 0 ;
            if( test_state = yysindex[yystate] ){
                for( i = YYERRCODE+1; i <= YYMAXTOKEN; i++ ){
                    expect = test_state + i ;
                    if( expect <= YYTABLESIZE && yycheck[expect] == i ){
                        if( two_or_more ){
                            strcat( err_msg, " | " ) ;
                        } else {
                            two_or_more = 1 ;
                        }
                        strcat( err_msg, yyname[i] ) ;
                     }
                 }
             }
            if( test_state = yyrindex[yystate] ){
                for( i = YYERRCODE+1; i <= YYMAXTOKEN; i++ ){
                    expect = test_state + i ;
                    if( expect <= YYTABLESIZE && yycheck[expect] == i ){
                        if( two_or_more ){
                            strcat( err_msg, " | " ) ;
                        } else {
                            two_or_more = 1 ;
                        }
                        strcat( err_msg, yyname[i] ) ;
                     }
                 }
             }
             strcat( err_msg, "\n" ) ;
             yyerror( err_msg ) ;
        } else {
            yyerror("syntax error");
        }
     }
#else
     yyerror("syntax error");
#endif
#ifdef lint
    goto yyerrlab;
#endif
yyerrlab:
    ++yynerrs;
yyinrecovery:
    if (yyerrflag < 3)
    {
        yyerrflag = 3;
        for (;;)
        {
            if ((yyn = yysindex[*yyssp]) && (yyn += YYERRCODE) >= 0 &&
                    yyn <= YYTABLESIZE && yycheck[yyn] == YYERRCODE)
            {
#if YYDEBUG
                if (yydebug)
                    printf("yydebug: state %d, error recovery shifting\
 to state %d\n", *yyssp, yytable[yyn]);
#endif
                if (yyssp >= yyss + yystacksize - 1)
                {
                    goto yyoverflow;
                }
                *++yyssp = yystate = yytable[yyn];
                *++yyvsp = yylval;
                goto yyloop;
            }
            else
            {
#if YYDEBUG
                if (yydebug)
                    printf("yydebug: error recovery discarding state %d\n",
                            *yyssp);
#endif
                if (yyssp <= yyss) goto yyabort;
                --yyssp;
                --yyvsp;
            }
        }
    }
    else
    {
        if (yychar == 0) goto yyabort;
#if YYDEBUG
        if (yydebug)
        {
            yys = 0;
            if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
            if (!yys) yys = "illegal-symbol";
            printf("yydebug: state %d, error recovery discards token %d (%s)\n",
                    yystate, yychar, yys);
        }
#endif
        yychar = (-1);
        goto yyloop;
    }
yyreduce:
#if YYDEBUG
    if (yydebug)
        printf("yydebug: state %d, reducing by rule %d (%s)\n",
                yystate, yyn, yyrule[yyn]);
#endif
    yym = yylen[yyn];
    yyval = yyvsp[1-yym];
    switch (yyn)
    {
case 9:
{
			ignore_net() ;
		    }
break;
case 10:
{
			ignore_route() ;
		    }
break;
case 11:
{
			process_net_rec( yyvsp[0].string ) ;
		    }
break;
case 12:
{
			/* pathlist COLON lowerBound upperBound priority*/
			end_path( yyvsp[-2].ival, yyvsp[-1].ival, yyvsp[0].ival ) ;
		    }
break;
case 13:
{
			end_path( yyvsp[-1].ival, yyvsp[0].ival, 1 ) ;
		    }
break;
case 15:
{
			add_path( STARTPATH, yyvsp[0].string ) ; 
		    }
break;
case 16:
{
			add_path( CONTPATH, yyvsp[0].string ) ; 
		    }
break;
case 17:
{ 
			yyval.string = yyvsp[0].string ;	
		    }
break;
case 18:
{
			/* convert integer to string */
			/* this allows integers to be used as strings */
			/* a kluge but timberwolf's old parser supported it */
			sprintf( bufferS,"%d", yyvsp[0].ival ) ;
			/* now clone string */
			yyval.string = (char *) Ystrclone( bufferS ) ;
		    }
break;
case 19:
{
			/* convert float to string */
			/* this allows floats to be used as strings */
			/* a kluge but timberwolf's old parser supported it */
			sprintf( bufferS,"%f", yyvsp[0].fval ) ;
			/* now clone string */
			yyval.string = (char *) Ystrclone( bufferS ) ;
		    }
break;
    }
    yyssp -= yym;
    yystate = *yyssp;
    yyvsp -= yym;
    yym = yylhs[yyn];
    if (yystate == 0 && yym == 0)
    {
#if YYDEBUG
        if (yydebug)
            printf("yydebug: after reduction, shifting from state 0 to\
 state %d\n", YYFINAL);
#endif
        yystate = YYFINAL;
        *++yyssp = YYFINAL;
        *++yyvsp = yyval;
        if (yychar < 0)
        {
            if ((yychar = yylex()) < 0) yychar = 0;
#if YYDEBUG
            if (yydebug)
            {
                yys = 0;
                if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
                if (!yys) yys = "illegal-symbol";
                printf("yydebug: state %d, reading %d (%s)\n",
                        YYFINAL, yychar, yys);
            }
#endif
        }
        if (yychar == 0) goto yyaccept;
        goto yyloop;
    }
    if ((yyn = yygindex[yym]) && (yyn += yystate) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yystate)
        yystate = yytable[yyn];
    else
        yystate = yydgoto[yym];
#if YYDEBUG
    if (yydebug)
        printf("yydebug: after reduction, shifting from state %d \
to state %d\n", *yyssp, yystate);
#endif
    if (yyssp >= yyss + yystacksize - 1)
    {
        goto yyoverflow;
    }
    *++yyssp = yystate;
    *++yyvsp = yyval;
    goto yyloop;
yyoverflow:
    yyerror("yacc stack overflow");
yyabort:
    return (1);
yyaccept:
    return (0);
}
