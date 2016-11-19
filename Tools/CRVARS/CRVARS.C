/* crvars.c  1994-10-12,  version 1.0 */
/*
 */

/*
! crvars.c
*/

/*
!	This program crvars makes a list of all variables in the metamod module
*/

/*
!	History
!	When	By Rev 	What
!	------- -- ---  ----------------------------------------------
!	941012	   1.0	Initial coding
*/

#define METAMOD "metaMod"

#include <stdio.h>
#include <dir.h>
#include <errno.h>
#include <math.h>
#include <ctype.h>
#include <stdlib.h>
#include <io.h>
#include <string.h>
#include <time.h>

char *metaName(char *, int);
int metaRemote(char *, int);
int metaSize(char *, int);
void printCopyright(void);
char *metaValue(char *, char *, int);
int metaType(char *, int);
int metaId(char *, char *);
int metaLock(char *, char *, int);
void swapMetaMod(char *);
void readMetaMod(void);


int isStartOfVar(char c);
int isVar(char c);
int isValue(char c);
int parse(char *line, char *var, char *value);
int searchVar(char *meta, char *var);
unsigned short swapword(unsigned short);
unsigned long swaplong(unsigned long);
double swapdouble(double);

void swapdouble2(double, char*);

void putValue(char *dm, char *meta, int id, char *value, FILE *fpLog);

char *metaModHeader;
char *metaMod;
short int hilo;

struct _metaModHeader {
    unsigned short sync;
    unsigned short sysrev;
    unsigned long  size;
    unsigned long  owner;
    unsigned long  nameoffset;
    unsigned short access;
    unsigned short typelang;
    unsigned short attrrev;
    unsigned short edition;
    unsigned long  usage;
    unsigned long  symbol;
    unsigned short ident;
    char  spare[12];
    unsigned short parity;
    
    unsigned long  dataptr;
    char  name[12];
};

int getFileSize(char *s)
{
   struct ffblk ffblk1;

   if (findfirst(s,&ffblk1,0)) {
     return 0;
   }
   return ffblk1.ff_fsize;
}

void readMetaMod(void)
{
  int siz;
  FILE *fp;

  if (!(siz = getFileSize(METAMOD))) {
     printf("Sorry, no such file as '%s'\n", METAMOD);
     exit(1);
  }
  if (!(metaModHeader = (char *) malloc(siz))) {
    printf("not enough memory\n");
    exit(1);
  }
  if ((fp = fopen(METAMOD, "rb")) <= 0) {
    printf("Error opening '%s'\n", METAMOD);
    exit(1);
  }
  fread(metaModHeader, siz, 1, fp);
  metaMod = metaModHeader + sizeof(struct _metaModHeader);
  fclose(fp);
}

getTotalSizeOfVars(char *meta)
{
  int id = 1, siz = 0;
  while (metaName(meta, id) > 0) {
    if (metaRemote(meta, id))
      siz += 4;
    siz += metaSize(meta, id);
    siz ++;
    id ++;
  }
  return siz;
}

void usageSWD(void)
{
  printf("Syntax: crvars [-nodnummer] <ut-logg-fil>\n");
  printf("\n");
  printf("Funktion:\n");
  printf("       Skapar en lista ”ver alla variabler som finns i metamod.\n");
  printf("       Listan anv„nds som indata till programmen setvar och getvar.\n");
  printf("       Den metamod som finns i aktuell katalog anv„nds.\n");
  printf("\n");
  exit(1);
}

main(int argc, char *argv[])
{
  FILE *fpLog;
  int varSize, id, varsFound, sts, option_H = 0, size, i;
  char *logFile, line[80], var[32], value[32], *p;
  int emitNodeNumber = 0, nodeNumber;

#define PROGRAM_NAME	"crvars, version 1.1"
  fprintf(stderr, "%s\n", PROGRAM_NAME);

  while( argc >= 2  && argv[1][0] == '-' ) {
    while( *++(argv[1]) ) {
      if (isdigit(*argv[1])) {
	nodeNumber = atoi(argv[1]);
	emitNodeNumber = 1;
	break;
      }
      switch( *argv[1] ) {
	case 'h':
	case 'H':
	  option_H = 1;
	  continue;
	case '?':
	  usageSWD();
	  exit(0);
	default:
	  fprintf(stderr, "ogiltigt val: %c\n", (char *) *argv[1]);
	  exit(0);
      }
    }
    argv++;
    argc--;
  }
  if (argc < 2) {
    usageSWD();
    exit(1);
  }

  logFile = argv[1];
  if ((fpLog = fopen(logFile, "w")) <= 0) {
    printf("Cannot create log file '%s'\n", logFile);
    exit(1);
  }
  readMetaMod();
  hilo = 0x4afc;
  if (*((char *) &hilo) == 0x4a)
    hilo = 1;                             /* like MC68K */
  else
    hilo = 0;                             /* maybe intel */
  if (!hilo)
    swapMetaMod(metaMod);
  varSize = getTotalSizeOfVars(metaMod);

  fprintf(stderr, "Skapar loggfilen...\n");
  if (option_H) {
        time_t t;
	fprintf(fpLog, "* Filnamn: %s\n", logFile);
	time(&t);
	fprintf(fpLog, "* Skapat av %s, %s", PROGRAM_NAME, ctime(&t));
  }
  varsFound = 0;
  id = 0;
  while (p = metaName(metaMod, ++id)) {
	int typ;

	typ = metaType(metaMod, id);
	if (typ == 15) {	/* calendar */
		if (emitNodeNumber)
			fprintf(fpLog, "calendar %d%s\n", nodeNumber, p);
		else
			fprintf(fpLog, "calendar %s\n", p);
		varsFound ++;
	} else if (typ == 7) {	/* int */
		if (emitNodeNumber) fprintf(fpLog, "%d", nodeNumber);
		fprintf(fpLog, "%s\n", p);
		varsFound ++;
	} else if (typ == 8) {	/* float */
		if (emitNodeNumber) fprintf(fpLog, "%d", nodeNumber);
		fprintf(fpLog, "%s\n", p);
		varsFound ++;
	} else if (typ == 4) {	/* int vec */
		size = metaSize(metaMod, id) / sizeof(long);
		for (i = 0; i < size; i++) {
			if (emitNodeNumber) fprintf(fpLog, "%d", nodeNumber);
			fprintf(fpLog, "%s[%d]\n", p, i);
		}
		varsFound ++;
	} else if (typ == 5) {	/* float vec */
		size = metaSize(metaMod, id) / sizeof(double);
		for (i = 0; i < size; i++) {
			if (emitNodeNumber) fprintf(fpLog, "%d", nodeNumber);
			fprintf(fpLog, "%s[%d]\n", p, i);
		}
		varsFound ++;
	}
	fprintf(stderr, "%d variabler hittade\r", varsFound);
  }
  fprintf(stderr, "%d variabler hittade\n", varsFound);
  free(metaModHeader);
  fclose(stdout);
  exit(0);
  return 0;
}

unsigned short swapword(w)
unsigned short w;
{
  return ((w & 0xff) << 8) | ((w >> 8) & 0xff);
}

unsigned long swaplong(l)
unsigned long l;
{
  unsigned short w1, w2;
  w1 = (l >> 16) & 0xffff;
  w2 = l & 0xffff;
  return (((unsigned long) swapword(w2)) << 16) | swapword(w1);
}

double swapdouble(d)
double d;
{
  union {
    double dd;
    struct { unsigned long l1, l2; } ll;
  } u1, u2;

  u1.dd = d;
  u2.ll.l1 = swaplong(u1.ll.l2);
  u2.ll.l2 = swaplong(u1.ll.l1);
  return u2.dd;
}

void swapdouble2(d, buf)
double d;
char *buf;
{
  union {
    double dd;
    struct { unsigned long l1, l2; } ll;
  } u1, u2;

  u1.dd = d;
  u2.ll.l1 = swaplong(u1.ll.l2);
  u2.ll.l2 = swaplong(u1.ll.l1);

  memcpy(buf, &u2.dd, sizeof(double));
}

void swapMetaMod(char *metaMod)
{
  int id, max;
  struct _metaEntry {
     unsigned short nameOffset, size, offset, lockOffset, type;
  } *metaEntry;

  metaEntry = (struct _metaEntry *) metaMod;
  max = metaEntry->nameOffset = swapword(metaEntry->nameOffset);
  id = 1;
  while (id++ <= max) {
    metaEntry++;

    metaEntry->nameOffset = swapword(metaEntry->nameOffset);
    metaEntry->size       = swapword(metaEntry->size);
    metaEntry->offset     = swapword(metaEntry->offset);
    metaEntry->lockOffset = swapword(metaEntry->lockOffset);
    metaEntry->type       = swapword(metaEntry->type);
  }
}


