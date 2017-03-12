#include <string.h>
#include <unistd.h>
#include <yalecad/debug.h>
#include <yalecad/cleanup.h>
#include <yalecad/file.h>
#include <general.h>
#include <string.h>
#include <yalecad/string.h>
#define YYU(x) x

char *yyextra;
struct yysvf **yylsp;
struct yysvf **yyolsp;
char *yysptr;
char *yysbuf;
struct yysvf *yyestate;
YYSTYPE yylval;

int yyleng = 0;
int yymorfg = 0;
int yytchar = 0;
int yylineno = 1;
int *yyfnd = 0;

int yyprevious = YYNEWLINE;
struct yysvf *yylstate [YYLMAX];
rw_table __attribute__((visibility("default"))) *rwtable;
struct yywork __attribute__((visibility("default"))) *yycrank;
struct yywork __attribute__((visibility("default"))) *yytop;
struct yysvf  __attribute__((visibility("default"))) *yysvec;
struct yysvf  __attribute__((visibility("default"))) *yybgin;
char __attribute__((visibility("default"))) *yytext;
unsigned long __attribute__((visibility("default"))) line_countS = 0;

char input( FILE* yyin );
int output( int out ) ;
int screen();
int yyback(int *p, int m);
int check_line_count(char *s) ;
void unput(char c);

int
yylook(fp)
FILE *fp;
{
	if(!fp) {
		printf("File not open!\n");
		return -1;
	}

	struct yysvf *yystate;
	struct yysvf **lsp;
	struct yywork *yyt;
	struct yysvf *yyz;
	struct yywork *yyr;
	char yych;
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
		yyestate = yybgin;
		yystate = yybgin;
		if (yyprevious==YYNEWLINE)
			yystate++;
		while(1) {
			yyt = yystate->yystoff;
			if(yyt == yycrank) {		/* may not be any transitions */
				yyz = yystate->yyother;
				if(yyz == 0) {
					break;
				}
				if(yyz->yystoff == yycrank) {
					break;
				}
			}
			*yylastch++ = yych =  input(fp);
//  			printf("yych: %c \n",yych);
//   			printf("yylastch: %s \n",yylastch);
tryagain:
			yyr = yyt;
			if ( (long)yyt > (long)yycrank) {
				yyt = yyr + yych;
				if (yyt <= yytop && yyt->verify+yysvec == yystate) {
					if(yyt->advance+yysvec == YYLERR) {	// error transitions
						unput(*--yylastch);
// 						printf("yylastch: %s\n",yylastch);
						break;
					}
					*lsp++ = yystate = yyt->advance+yysvec;
					goto contin;
				}
			} else {
//  				printf("yylastch: %s\n",yylastch);
  				unput(*(--yylastch));
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
		yyprevious = yytext[0] = input(fp);
		
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
					printf("bad switch yylook %d",nstr);
					break;
			}
	}
	return 0;
}

/* end of yylex */

int yyback(p, m)
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

int output( out )
int out;
{
}

char input( yyin )
FILE *yyin;
{
	//char ret = (((yytchar=yysptr>yysbuf?U(*--yysptr):getc(yyin))==10?(yylineno++,yytchar):yytchar)==EOF?0:yytchar);
	//char ret = fgetc(yyin);
	//return ret;
	return 42;
}

void unput(c)
char c;
 {
	printf("char %c\n",c);
	 /* yytchar= (c);
	 if(yytchar=='\n')
		 yylineno--;
	 *yysptr++=yytchar;*/
}
