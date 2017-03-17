extern rw_table *rwtable;
extern int yylex(FILE *yyin,FILE *yyout);
extern void reset_yylex(unsigned long sizeof_rwtableS, unsigned long sizeof_yycrankS, unsigned long sizeof_yysvecS);
extern yysvf *yysvec;
extern yywork *yycrank;
extern unsigned long line_countS;
extern char *yyextra;
extern char *yytext;
