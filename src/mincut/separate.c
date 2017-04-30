#include <stdio.h>
#include <string.h>
#include <yalecad/base.h>

int separate_cel_file(char *cktName)
{
	char celfile[LRECL];
	char mcelfile[LRECL];
	char scelfile[LRECL];

	FILE *fp, *fpsc, *fpmc;
	char line[LRECL];
	char cmpbuf[LRECL];

	sprintf( celfile, "%s.cel", cktName ) ;
	sprintf( scelfile, "%s.scel", cktName ) ;
	sprintf( mcelfile, "%s.mcel", cktName ) ;

	fp = fopen(celfile,"r");
	fpsc = fopen(scelfile,"w");
	fpmc = fopen(mcelfile,"a");

	int sc = 0, both = 0, end_both=0;
	while (fgets(line, sizeof(line), fp)) {
		strcpy(cmpbuf, "cell");
		if(!strncmp(line, cmpbuf, strlen(cmpbuf))) {
			sc = 1;
			both = 0;
		}
		if(sc) {
			strcpy(cmpbuf, "hardcell");
			if(!strncmp(line, cmpbuf, strlen(cmpbuf))) {
				sc = 0;
				both = 0;
			}
			strcpy(cmpbuf, "softcell");
			if(!strncmp(line, cmpbuf, strlen(cmpbuf))) {
				sc = 0;
				both = 0;
			}
			strcpy(cmpbuf, "pad");
			if(!strncmp(line, cmpbuf, strlen(cmpbuf))) {
				sc = 0;
				both = 1;
			}
			strcpy(cmpbuf, "pin");
			if(!strncmp(line, cmpbuf, strlen(cmpbuf))) {
				if(both)
					end_both=1;
			}
		}

		if(sc) {
			fprintf(fpsc, "%s", line);
		} else {
			fprintf(fpmc, "%s", line);
			if(both) {
				fprintf(fpsc, "%s", line);
				if(end_both) {
					both=0;
					end_both=0;
				}
			}
		}
	}

	fclose(fp);
	fclose(fpsc);
	fclose(fpmc);

	return 0;
}
