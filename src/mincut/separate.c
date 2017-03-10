#include <stdio.h>
#include <yalecad/base.h>

int separate_cel_file(cktName)
char *cktName;
{
	char celfile[LRECL];
	char mcelfile[LRECL];
	char scelfile[LRECL];

	FILE *fp, *fpsc, *fpmc;
	char line[LRECL];

	sprintf( celfile, "%s.cel", cktName ) ;
	sprintf( scelfile, "%s.scel", cktName ) ;
	sprintf( mcelfile, "%s.mcel", cktName ) ;

	fp = fopen(celfile,"r");
	fpsc = fopen(scelfile,"a");
	fpmc = fopen(mcelfile,"a");

	int sc = 0;
	char *tmpStr;
	while (fgets(line, sizeof(line), fp)) {
		if(tmpStr = strstr(line, "cell")) {
			sc = 1;
		} else if(tmpStr = strstr(line, "hardcell")) {
			sc = 0;
		} else if(tmpStr = strstr(line, "softcell")) {
			sc = 0;
		} else if(tmpStr = strstr(line, "pad")) {
			sc = 0;
		}

		if(sc) {
			fprintf(fpsc, "%s", line);
		} else {
			fprintf(fpmc, "%s", line);
		}
	}

	fclose(fp);
	fclose(fpsc);
	fclose(fpmc);

	return 0;
}

