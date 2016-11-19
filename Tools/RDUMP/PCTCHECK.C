/* pctcheck.c  1992-04-07 TD,  version 1.0 */
/*
 * This file contains proprietary information of IVT Electronic AB.
 * Copying or reproduction without prior written approval is prohibited.
 *
 * This file is furnished under a license agreement or nondisclosure
 * agreement. The software may be used or copied only in accordance 
 * with the terms of the agreement.
 *
 * In no event will IVT Electronic AB, be liable for any lost revenue or
 * profits or other special, indirect and consequential damages, even if
 * IVT has been advised of the possibility of such damages.
 *
 * IVT Electronic AB
 * Box 996
 * 191 29 Sollentuna
 * Sweden
 */

/*
! pctcheck.c
! Copyright (C) 1992, IVT Electronic AB.
*/
#include <stdio.h>
#include <dos.h>
#include <dir.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

extern char *metaMod;
extern char *metaModHeader;
int previousDucNo = -1;

int quiet = 0, verbose = 0;
int totFiles = 0, totErrors = 0, totErrFiles = 0;
int duc0isduc1 = 0;
int SWD = 1;

int metaId(char *meta, char *name);
void printCopyright(void);
int readMetaMod(char *);
void swapMetaMod(char *);


char *strcpy2sp(char *s1, char *s2)
{
  char *p;
  p = s1;
  while ((*s1 = *s2) && (*s2 != ' '))
   { s1++; s2++; }
  *s1 = 0;
  return p;
}
char ducPath[30];

readMeta(int duc)
{
  char file[40];
  short hilo;

  if (duc0isduc1) {
    if (duc == 0)
      duc = 1;
  }
  sprintf(file, ducPath, duc);
  strcat(file, "\\metamod");
/*
  sprintf(file, "%s%d\\metamod", ducPath, duc);
*/
/*
  sprintf(file, "\\huvud\\duc%d\\metamod", duc);
*/
  if (!readMetaMod(file))
    return 0;

  hilo = 0x4afc;
  if (*((char *) &hilo) == 0x4a)
    hilo = 1;                             /* like MC68K */
  else
    hilo = 0;                             /* maybe intel */
  if (!hilo)
    swapMetaMod(metaMod);
  return 1;
}

/*
#define MAX 5
static char ducs[256], *ducs[MAX];
*/
getMeta(int ducNo)
{
/*
  if (ducs[ducNo & 255]) {
    metaModHeader = ducs[ducNo & 255];
    metaMod = metaModHeader + 64;
    return 0;
  }
*/
  if (ducNo != previousDucNo) {
    if (previousDucNo != -1)
      free(metaModHeader);
    if (!readMeta(ducNo))
      return 0;
    previousDucNo = ducNo;
  }
  return 1;
}

inform(char *f1, int ducNo)
{
  printf("%s duc %d\n", f1, ducNo);
}

doFile(char *f1, char *f2)
{
  FILE *fp1, *fp2;
  char buf[150];
  int i, ducNo, masterNo, first = 1, badFile = 0;

  if ((fp1 = fopen(f1, "rb")) == NULL) {
    printf("Error opening '%s'\n", f1);
    printf("errno = %d\n", errno);
    printf("_doserrno = %d\n", _doserrno);
    perror(f1);
    return 0;
  }
  if ((fp2 = fopen(f2, "rb")) == NULL) {
    printf("Error opening '%s'\n", f2);
    printf("errno = %d\n", errno);
    printf("_doserrno = %d\n", _doserrno);
    perror(f2);
    return 0;
  }
  fread(buf, 137, 1, fp2);
  fread(buf, 137, 1, fp2);
  fread(buf, 137, 1, fp2);
  ducNo = buf[0];
  masterNo = buf[1];
  
  if (!quiet)  
    inform(f1, ducNo);

  if (!getMeta(ducNo)) {
    fclose(fp1);
    fclose(fp2);
    return 0;
  }

  totFiles ++;
  for (i = 0; i < 200; i++) {
    fread(buf, 54, 1, fp1);
    if (buf[0] == 0)
      continue;
/*				    
   if (buf[1] == 9)
     dump(buf, 54);
*/
   if (buf[1] == 9 || buf[1] == 16) {       /* dyn or cal */
      char var[32];

      buf[19 + buf[18]] = 0;
      strcpy2sp(var, &buf[19]);
      if (metaId(metaMod, var) > 0) {
	if (verbose) printf("   var '%s'\n", var);
      }
      else {
	if (quiet) {  
	  if (first == 1)
	    inform(f1, ducNo);
	  first = 0;
	}
	if (SWD)
	  printf("    - fel - variabeln saknas; '%s'\n", var);
	else
	  printf("    - error - missing variable '%s'\n", var);
	totErrors ++;
	badFile = 1;
      }
    }
  }
  if (badFile) 
    totErrFiles ++;
  fclose(fp1);
  fclose(fp2);
}

dump(unsigned char *s, int l)
{
  int i;
  for (i = 0; i < l; i++) {
    printf("%02x, ", *s++);
    if ((i & 15) == 15)
      printf("\n");
  }
  printf("\n");
}

void getPath(char *path, char *realPath)
{
  char *pk;

  strcpy(realPath, path);
  pk = realPath + strlen(realPath);
  pk --;
  while (*pk != '\\') {
    if (pk < realPath)
      break;
    *pk-- = '\0';
  }
}

processFile(char *file, char *path)
{
  struct fcb blk, blk2;

  if (parsfnm(file, &blk, 1) == NULL)
   printf("Error in parsfm call\n");
  else {
   char f1[30], f2[30];
   if (parsfnm(path, &blk2, 1) == NULL)
     printf("Error in parsfm call2\n");
   else {
     int i, q;
     char realPath[60];

     blk.fcb_name[8] = 0;

     getPath(path, realPath);

     i = strlen(strcpy2sp(blk.fcb_name, blk.fcb_name));
     q = strlen(strcat(strcat(strcpy(f1, realPath), blk.fcb_name), ".pct"));
     strcat(f1, "        ");
     f1[q+8-i] = '\0';
     strcat(strcat(strcpy(f2, realPath), blk.fcb_name), ".cmd");
     doFile(f1, f2);
   }
  }
}

usage(void)
{
  printf("Usage: pctcheck [option] <duc-path> <pct-path>\n");
  printf("\n");
  printf("option is one of the following:\n");
  printf("-q         quiet mode\n");
  printf("-v         verbose mode\n");
  printf("-0         treat node 0 as node 1\n");
  printf("\n");
  printf("<duc-path> is the path to the metamod file(s).\n");
  printf("           An optional %%d will be replaced by \n");
  printf("           the current node number.\n");
  printf("\n");
  printf("<pct-file> is the files to be processed. \n");
  printf("           Wildcards such as * and ? can be used.\n"); 
  printf("\n");
  printf("Example:   pctcheck \\huvud\\duc%%d \\ivtnew\\*.pct\n");
  printf("\n");
}

usageSWD(void)
{
  printf("Syntax: pctcheck [val] <duc-path> <pct-path>\n");
  printf("\n");
  printf("val „r ett av f”ljande:\n");
  printf("-q         tyst mod\n");
  printf("-v         pratig mod\n");
  printf("-0         behandla duc 0 som duc 1\n");
  printf("\n");
  printf("<duc-path> „r s”kv„gen till metamod-fil(er).\n");
  printf("           Ett valfritt %%d kommer att ers„ttas\n");
  printf("           med aktuellt nodnummer.\n");
  printf("\n");
  printf("<pct-file> „r de files som ska behandlas. \n");
  printf("           s”ktecknen * and ? kan anv„ndas.\n"); 
  printf("\n");
  printf("Exempel:   pctcheck \\huvud\\duc%%d \\ivtnew\\*.pct\n");
  printf("\n");
}

char pctPath[50];

main(int argc, char *argv[])
{
    struct ffblk ffblk;
    int done, noOfFiles;

    printf("pctcheck, version 1.0\n");
    printCopyright();
    while( argc >= 2  && argv[1][0] == '-' ) {
	while( *++(argv[1]) ) {
	    switch( *argv[1] ) {
		case 'q':
		case 'Q':
		    quiet = 1;
		    continue;
		case '0':
		    duc0isduc1 = 1;
		    continue;
		case 'v':
		case 'V':
		    verbose = 1;
		    continue;
		case '?':
		    usage();
		    exit(0);
		default:
		    printf( "illegal option: %c", (char *) *argv[1]);
		}
	    }
	argv++;
	argc--;
    }
    if( argc < 3) {
	usageSWD();
	exit(1);
    }
    strcpy(ducPath, argv[1]);
    strcpy(pctPath, argv[2]);
	

    noOfFiles = 0;
   done = findfirst(pctPath,&ffblk,0);
   while (!done) {
      processFile(ffblk.ff_name, pctPath);
      done = findnext(&ffblk);
   }
   if (noOfFiles == 0) {
     strcat(pctPath, ".pct");
     done = findfirst(pctPath,&ffblk,0);
     while (!done) {
       processFile(ffblk.ff_name, pctPath);
       done = findnext(&ffblk);
     }
   }
   printf("\n");
   if (SWD) {
     printf("Totalt %d filer behandlade.\n", totFiles);
     if (totErrors) {
       printf("Sammanlagt %d variabler saknades f”rdelade p† %d filer.\n",
		totErrors, totErrFiles);
/*
       printf("I %d filer var det totalt %d variabler som saknades.\n",
			totErrFiles, totErrors);
*/
     } else 
       printf("Inga variabler saknades.\n");
   } else {
     printf("Total %d files processed.\n", totFiles);
     printf("In %d files totaly %d variables were not found.\n", 
			totErrFiles, totErrors);
   }
}

