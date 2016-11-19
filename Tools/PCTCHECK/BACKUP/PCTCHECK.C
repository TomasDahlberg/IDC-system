#include <stdio.h>
#include <dos.h>
#include <dir.h>

extern char *metaMod;
extern char *metaModHeader;
int previousDucNo = -1;

int quiet = 0, verbose = 0;

strcpy2sp(char *s1, char *s2)
{
  while ((*s1 = *s2) && (*s2 != ' '))
   { s1++; s2++; }
  *s1 = 0;
}
char ducPath[30];

readMeta(int duc)
{
  char file[40];
  short hilo;

  if (duc == 0)
    duc = 1;
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
}

inform(char *f1, int ducNo)
{
  printf("%s duc %d\n", f1, ducNo);
}

doFile(char *f1, char *f2)
{
  FILE *fp1, *fp2;
  char buf[150];
  int i, ducNo, masterNo, first = 1;

  fp1 = fopen(f1, "rb");
  fp2 = fopen(f2, "rb");
  if (fp1 == NULL || fp2 == NULL)
    return 0;
  fread(buf, 137, 1, fp2);
  fread(buf, 137, 1, fp2);
  fread(buf, 137, 1, fp2);
  ducNo = buf[0];
  masterNo = buf[1];
  
  if (!quiet)  
    inform(f1, ducNo);

  if (!getMeta(ducNo))
    return 0;

  for (i = 0; i < 200; i++) {
    fread(buf, 54, 1, fp1);
    if (buf[0] == 0)
      continue;
/*					    
   if (buf[1] == 16)
     dump(buf, 54);
*/
   if (buf[1] == 9 || buf[1] == 16) {       /* dyn or cal */
      char var[32];

      buf[19 + buf[18]] = 0;
      strcpy2sp(var, &buf[19]);
      if (verbose) printf("var '%s'", var);
      if (metaId(metaMod, var) > 0) {
	if (verbose) printf("\n");
      }
      else {
	if (quiet) {  
	  if (first == 1)
	    inform(f1, ducNo);
	  first = 0;
	}
	if (!verbose) printf("var '%s'", var);
	printf(" - error - missing var\n");
      }
    }
  }
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

processFile(char *file)
{
  struct fcb blk;

  if (parsfnm(file, &blk, 1) == NULL)
   printf("Error in parsfm call\n");
  else {
   char f1[30], f2[30];
   blk.fcb_name[8] = 0;
   strcat(strcpy(f1, blk.fcb_name), ".pct");
   strcat(strcpy(f2, blk.fcb_name), ".cmd");
   doFile(f1, f2);
  }
}

usage()
{
  printf("Usage: pctcheck [option] <duc-path> <pct-path>\n");
  printf("\n");
  printf("option is one of the following:\n");
  printf("-q	     quiet mode\n");
  printf("-v         verbose mode\n");
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

usageSWD()
{
  printf("Syntax: pctcheck [val] <duc-path> <pct-path>\n");
  printf("\n");
  printf("val „r ett av f”ljande:\n");
  printf("-q	     tyst mod\n");
  printf("-v         pratig mod\n");
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
    int done;

    printCopyright();
    while( argc >= 2  && argv[1][0] == '-' ) {
	while( *++(argv[1]) ) {
	    switch( *argv[1] ) {
		case 'q':
		case 'Q':
		    quiet = 1;
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
	

   done = findfirst(pctPath,&ffblk,0);
   while (!done)
   {
/*
      printf("  %s\n", ffblk.ff_name);
*/
      processFile(ffblk.ff_name);
      done = findnext(&ffblk);
   }
}

