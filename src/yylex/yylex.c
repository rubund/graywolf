#include <string.h>
#include <unistd.h>
#include <yalecad/debug.h>
#include <yalecad/cleanup.h>
#include <yalecad/file.h>
#include <general.h>
#include <string.h>
#include <yalecad/string.h>
#define YYU(x) x

/*int yycnprs = 0;
int yydebug;
short *yyssp;
YYSTYPE *yyvsp;
YYSTYPE yyval;
short yyss[YYSTACKSIZE];
YYSTYPE yyvs[YYSTACKSIZE];
struct yysvf *yyestate;
char yytext[YYLMAX];
char yysbuf[YYLMAX];
char *yysptr = yysbuf;
extern struct yysvf *yyestate;
int yylineno =1;
int yyleng;
int yymorfg;
int yytchar;
FILE *yyout;
int *yyvstop;
struct yysvf *yysvec;
short __attribute__((visibility("default"))) *yylhs;
short __attribute__((visibility("default"))) *yydefred;
short __attribute__((visibility("default"))) *yydgoto;
short __attribute__((visibility("default"))) *yysindex;
short __attribute__((visibility("default"))) *yyrindex;
short __attribute__((visibility("default"))) *yygindex;
short __attribute__((visibility("default"))) *yytable;
short __attribute__((visibility("default"))) *yycheck;
short __attribute__((visibility("default"))) *match;
char __attribute__((visibility("default"))) *yymatch;
char __attribute__((visibility("default"))) *cktNameG;*/

char yytext[];
unsigned long line_countS = 0;
YYSTYPE yylval;
char *yyextra;
struct yysvf **yylsp;
struct yysvf **yyolsp;
char *yysptr;
char yysbuf[];
struct yysvf *yyestate;
struct yysvf *yysvec;
struct yysvf *yybgin;
struct yysvf *yybgin;
struct yywork *yytop;
int yyleng;
int yymorfg;
int yytchar;
int yylineno;
int *yyfnd;
const struct yywork { YYTYPE verify, advance; } *yycrank;
int yyprevious = YYNEWLINE;
struct yysvf *yylstate [YYLMAX];
rw_table __attribute__((visibility("default"))) *rwtable;

int
yylook(fp)
FILE *fp;
{
	struct yysvf *yystate, **lsp;
	struct yywork *yyt;
	struct yysvf *yyz;
	int yych;
	struct yywork *yyr;
	char *yylastch;
	/* start off machines */
	if (!yymorfg)
		yylastch = yytext;
	else {
		yymorfg=0;
		yylastch = yytext+yyleng;
	}
	while(1) {
		lsp = yylstate;
		yyestate = yystate = yybgin;
		if (yyprevious==YYNEWLINE)
			yystate++;
		while(1) {
			yyt = yystate->yystoff;
			if(yyt == yycrank) {		/* may not be any transitions */
				printf("miau1\n");
				yyz = yystate->yyother;
				if(yyz == 0) {
					printf("miau2\n");
					break;
				}
				if(yyz->yystoff == yycrank) {
					printf("miau3\n");
					break;
				}
			}
			yych = getc(fp);
			printf("yych: %s\n",yych);
			(*yylastch++) = yych;
			printf("yych: %s\n",yych);
			printf("yylastch: %s\n",yylastch);
tryagain:
			yyr = yyt;
			if ( (long)yyt > (long)yycrank) {
				yyt = yyr + yych;
				if (yyt <= yytop && yyt->verify+yysvec == yystate) {
					if(yyt->advance+yysvec == YYLERR) {	/* error transitions */
						unput(*--yylastch);
						printf("yylastch: %s\n",yylastch);
						break;
					}
					*lsp++ = yystate = yyt->advance+yysvec;
					goto contin;
				}
			} else {
				printf("yylastch: %s\n",yylastch);
				unput(*--yylastch);
				break;
			}
contin:
			;
		}
		while (lsp-- > yylstate){
			*yylastch-- = 0;
			if (*lsp != 0 && (yyfnd= (*lsp)->yystops) && *yyfnd > 0){
				yyolsp = lsp;
				if(yyextra[*yyfnd]){		/* must backup */
					while(yyback((*lsp)->yystops,-*yyfnd) != 1 && lsp > yylstate){
						lsp--;
						unput(*yylastch--);
					}
				}
				yyprevious = YYU(*yylastch);
				yylsp = lsp;
				yyleng = yylastch-yytext+1;
				yytext[yyleng] = 0;
				return(*yyfnd++);
			}
			unput(*yylastch);
		}
		if (yytext[0] == 0  /* && feof(yyin) */)
		{
			yysptr=yysbuf;
			return(0);
		}
		yyprevious = yytext[0] = getc(fp);
		if (yyprevious>0)
			output(yyprevious);
		yylastch=yytext;
	}
}

int 
__attribute__((visibility("default")))
yylex(fp)
FILE *fp;
{
	int nstr;
	int yyprevious;
	while((nstr = yylook(fp)) >= 0) {
		yyfussy:
			switch(nstr)  {
				case 0:
					if(yywrap())
						return(0);
					break;
				case 1:
					/* C-style comments over multiple lines */
					check_line_count( yytext );
					break;
				case 2:
					/* convert to an integer */
					yylval.ival = atoi( yytext );
					return (INTEGER); 
					break;
				case 3:
					/* convert to an integer */
					yylval.fval = atof( yytext );
					return (FLOAT); 
					break;
				case 4:
					/* convert to an integer */
					yylval.fval = atof( yytext ) ;
					return (FLOAT); 
					break;
				case 5:
					return(screen()) ;
					break;
				case 6:
					 line_countS++;
					break;
				case 7:
					;
					break;
				case 8:
					return(token(yytext[0]));
					break;
				case -1:
					break;
				default:
					fprintf("bad switch yylook %d",nstr);
					break;
			}
	}
	return 0;
}

/* end of yylex */

yyback(p, m)
int *p;
int m;
{
	if (p==0) return(0);
	while (*p)
	{
		if (*p++ == m)
			return(1);
	}
	return(0);
}

int check_line_count( s ) 
char *s ;
{
	printf("Miau \n");
	if( s ){
		if( strlen(s) >= YYLMAX ){
			printf("comment beginning at line %d ", line_countS );
			printf("exceeds maximum allowed length:%d chars.\n", YYLMAX );
		}
		for( ;*s;s++ ){
			if( *s == '\n'){
				line_countS++;
			}
		}
	}
} /* end check_line_count */

yywrap()
{
	return(1);
}

int screen() 
{
	INT c ;
	rw_table  *low = rwtable,        /* ptr to beginning */
			 *mid ,  
			 *high = END(rwtable) ;   /* ptr to end */

	/* binary search to look thru table to find pattern match */
	while( low <= high){
		mid = low + (high-low) / 2 ;
		if( (c = strcmp(mid->rw_name, yytext) ) == STRINGEQ){
			return( mid->rw_yylex ) ; /* return token number */
		} else if( c < 0 ){
			low = mid + 1 ;
		} else {
			high = mid - 1 ;
		}
	}
	/* at this point we haven't found a match so we have a string */
	/* save the string by making copy */
	yylval.string = (char *) Ystrclone( yytext ) ;
	return (STRING); 
} /* end screen function */

output( fp )
FILE *fp ;
{
}

