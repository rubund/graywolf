#include <string.h>
#include <unistd.h>
#include <yalecad/debug.h>
#include <yalecad/cleanup.h>
#include <yalecad/file.h>
#include <general.h>
#include <string.h>
#include <yalecad/string.h>
#define YYU(x) x

yysvf **yylsp;
yysvf **yyolsp;

char yysbuf[YYLMAX];
char *yysptr  = yysbuf;
yysvf *yyestate;
YYSTYPE yylval;

int yyleng = 0;
int yymorfg = 0;
int yytchar = 0;
int yylineno = 1;
int *yyfnd = 0;

int yyprevious = 0;
yysvf *yylstate [YYLMAX];
rw_table __attribute__((visibility("default"))) *rwtable;
yywork __attribute__((visibility("default"))) *yycrank;
yywork *yytop;
yysvf  __attribute__((visibility("default"))) *yysvec;
yysvf  *yybgin;

unsigned long __attribute__((visibility("default"))) line_countS = 0;
int  __attribute__((visibility("default"))) *yyvstop;
char __attribute__((visibility("default"))) *yyextra;

// char yytext[YYLMAX];
char __attribute__((visibility("default"))) yytext[YYLMAX];

int input( FILE* yyin );
int output( int out ) ;
int screen();
int yyback(int *p, int m);
int check_line_count(char *s) ;
int unput(char c);
int yylook(FILE *yyin);

int yylook(yyin)
FILE *yyin;
{
	yysvf *yystate;
	yysvf **lsp;
	yywork *yyt;
	yywork *yyr;
	yysvf *yyz;
	int yych;
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
		yystate = yybgin;
		yyestate = yystate;

		if (yyprevious==YYNEWLINE) {
			yystate+=1;
			printf("Value of yystate->yystops:\t%d\n", *(yystate->yystops));
			printf("Pointer of yystate:\t%p\n",yystate);
		}

		while(1) {
			yyt = yystate->yystoff;
			if(yyt == yycrank) { // may not be any transitions
// 				printf("Pointer of yybgin->yystoff:\t%p\n",yybgin->yystoff);
				yyz = yystate->yyother;
				if(yyz == 0) {
					break;
				}
				if(yyz == 0x18) {
					break;
				}
				if(yyz->yystoff == yycrank) {
					break;
				}
			}
			*yylastch++ = yych = input(yyin);
		tryagain:
			yyr = yyt;
			if ( (long)yyt > (long)yycrank){
				yyt = yyr + yych;
				if (yyt <= yytop && yyt->verify+yysvec == yystate){
					if(yyt->advance+yysvec == YYLERR) {	/* error transitions */
						unput(*--yylastch);
						break;
					}
					*lsp++ = yystate = yyt->advance+yysvec;
					goto contin;
				}
			} else {
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
		printf("yytext: %s\n",yytext);
		if ( (yytext[0] == 0)  && (feof(yyin)) ) {
			yysptr=yysbuf;
			return(0);
		}
		yyprevious = yytext[0] = input(yyin);
		if (yyprevious>0)
			output(yyprevious);
		yylastch=yytext;
	}
}

int __attribute__((visibility("default"))) yylex(yyin,yyout)
FILE *yyin;
FILE *yyout;
{
	yytop = yycrank+440;
// 	yybgin = yysvec+sizeof(yysvf);
 	yybgin = yysvec+1;
	printf("Pointer of yycrank:\t%p\n", yycrank);
	printf("Pointer of yysvec:\t%p\n", yysvec);
	printf("Pointer of yybgin:\t%p\n", yybgin);
	printf("Pointer of yybgin->yystoff:\t%p\n",yybgin->yystoff);
	
	printf("Value of yybgin->yystoff->advance:\t%d\n",yybgin->yystoff->advance);
	printf("Value of yybgin->yystoff->verify:\t%d\n",yybgin->yystoff->verify);

	int nstr;
	while((nstr = yylook(yyin)) >= 0)
		yyfussy:
			switch(nstr){
		case 0:
				return(0);
				break;
		case 1:
				{
					/* C-style comments over multiple lines */
					check_line_count(yytext) ;
				}
				break;
		case 2:
				{
					/* convert to an integer */
					yylval.ival = atoi( yytext ) ;
					return (INTEGER); 
				}
				break;
		case 3:
				{
					/* convert to an integer */
					yylval.fval = atof( yytext ) ;
					return (FLOAT); 
				}
			break;
		case 4:
				{
					/* convert to an integer */
					yylval.fval = atof( yytext ) ;
					return (FLOAT); 
				}
				break;
		case 5:
				{
					return( COLON ) ;
				}
				break;
		case 6:
				{
					return( screen() ) ;
				}
				break;
		case 7:
				{
					line_countS++;
				}
				break;
		case 8:
				break;
		case 9:
				{
					return( token(yytext[0]) ) ;
				}
				break;

		case -1:
				break;

		default:
			fprintf(yyout,"bad switch yylook %d",nstr);
	} 
	return(0);
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
	//printf("output: %d \n",out);
}

int input( yyin )
FILE *yyin;
{
	int ret = (((yytchar=yysptr>yysbuf?U(*--yysptr):fgetc(yyin))==10?(yylineno++,yytchar):yytchar)==EOF?0:yytchar);
	//printf("char %i\n",ret);
	return ret;
}

int unput(c)
char c;
 {
	yytchar=(c);
	if(yytchar=='\n')
		 yylineno--;
	*yysptr++=yytchar;
}
