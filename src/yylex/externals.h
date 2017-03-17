extern rw_table *rwtable;
extern int yylex(FILE *yyin,FILE *yyout);
extern void reset_yylex(unsigned int sizeof_rwtableS, unsigned int sizeof_yycrankS, unsigned int sizeof_yysvecS);
extern yysvf *yysvec;
extern yywork *yycrank;
extern unsigned long line_countS;
extern int *yyvstop;
extern char *yyextra;
extern char *yytext;
