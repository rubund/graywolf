#include <stdio.h>

#define YYSTACKSIZE 500
#define YYMAXDEPTH 500
#define yystacksize YYSTACKSIZE
#define U(x) ((x)&0377)
#define NLSTATE yyprevious=YYNEWLINE
#define BEGIN yybgin = yysvec + 1 +
#define INITIAL 0
#define YYLERR yysvec
#define token(x) x /* makes it look like regular lex */

#define YYNEWLINE 10
#define YYLMAX 2000       /* comments may be at most 2000 characters */
# define YYTYPE unsigned char

typedef struct { YYTYPE verify, advance; } yywork;

typedef struct  { 
	yywork *yystoff;
	struct yysvf *yyother;
	int *yystops;
} yysvf;

typedef union {
	INT ival ;
	char *string ;
	DOUBLE fval ;
} YYSTYPE;

/*	ncform	4.1	83/08/11	*/

typedef struct {  /* reserved word table */
	char *rw_name ;      /* pattern */
	INT rw_yylex  ;      /* lex token number */
} rw_table;

#define END(v) (v-1 + sizeof(v) / sizeof( v[0] ) ) /* for table lookup */
#define INTEGER 257
#define STRING 258
#define FLOAT 259
#define COLON 262
