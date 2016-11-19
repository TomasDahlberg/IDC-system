/* mkvars.c  1993-03-17 TD,  version 1.3 */
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
! mkvars.c
! Copyright (C) 1992,1993 IVT Electronic AB.
*/
/*
!	This program mkvars makes a datamodule named VARS and fills it with
!	values from a logfile (captured from look) according to the metavars
!	found in file metaMod.
*/
/*
!	History
!	When	By Rev 	What
!	------- -- ---  ----------------------------------------------
!	920416	TD 1.0	Initial coding
!	921110	TD 1.1	Changed atoi -> atol in parsing int's
!	930304	TD 1.2	Changed swapdouble->swapdouble2, and %ld -> %d
!	930317	TD 1.3  Added patch of existing module, and options -o -p
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
void emitHeaderAndChunk(char *file, char *chunk, int chunkSize);
void getVarsData(char *file, char *chunk, int chunkSize);
int checkOldVarsData(char *file, int chunkSize);



#define NO_OF_CAL_ENTRIES 10
struct _calendar {
  unsigned short day[NO_OF_CAL_ENTRIES];
  unsigned short stopday[NO_OF_CAL_ENTRIES];
  unsigned char color[NO_OF_CAL_ENTRIES];
  unsigned short start[NO_OF_CAL_ENTRIES];
  unsigned short stop[NO_OF_CAL_ENTRIES];
};

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
/*
struct ffblk {
  char      ff_reserved[21];
  char      ff_attrib;
  unsigned  ff_ftime;
  unsigned  ff_fdate;
  long      ff_fsize;
  char      ff_name[13];
};
*/
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
/*
  if ((siz = getFileSize(METAMOD)) > 60000) {
    printf("To big file to handle, '%s'\n", METAMOD);
    exit(1);
  }
*/
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
/*    printMeta(meta, id);           */
    if (metaRemote(meta, id))
      siz += 4;
    siz += metaSize(meta, id);
    siz ++;
    id ++;
  }
/*  printf("size = %d\n", siz);      */
  return siz;
}

void usage(void)
{
  printf("Usage: mkvars <log-file>\n");
  printf("\n");
  printf("Function:\n");
  printf("       takes the metamod file from the current directory and\n");
  printf("       creates a data module named VARS and fills it with\n");
  printf("       values from the log file.\n");
  exit(1);
}

void usageSWD(void)
{
  printf("Syntax: mkvars [option] <logg-fil>\n");
  printf("\n");
  printf("option:\n");
  printf("	-o   (overwrite) skriv ”ver existerande VARS-modul\n");
  printf("	-p   (patch)	 modifiera existerande VARS-modul\n");
  printf("\n");
  printf("Funktion:\n");
  printf("       Skapar datamodulen VARS och fyller den med v„rden\n");
  printf("       fr†n den angivna logg-filen med hj„lp av metamod-\n");
  printf("       filen fr†n aktuellt bibliotek. Logg-filen skapas\n");
  printf("       l„mpligen genom att f†nga utskriften fr†n programmet\n");
  printf("       look.\n");
  printf("\n");
  printf("       Den skapade modulen VARS laddas sedan vid behov p†\n");
  printf("       adress $20000 mha 'sendmod vars 20000'.\n");
  printf("\n");
  printf("       Observera att den VARS-modul som ska ers„ttas m†ste ha\n");
  printf("       skapats av ett program kompilerat med IDCC ver 1.46\n");
  printf("       eller senare.\n");
  exit(1);
}


/* pseudo code is:
  openLogFile
  openMetaModandReadToMem
  getTotalSizeofVars
  createVARSchunk
  for each entry in log-file
    find var in metamod
    fill entry in VARSchunk with value
  close
  emitChunktoVARSfile
*/

main(int argc, char *argv[])
{
  FILE *fpLog;
  int varSize, id, varsFound, sts, option_O = 0, option_P = 0;
  char *logFile, *varChunk, line[80], var[32], value[32];


  fprintf(stderr, "mkvars, version 1.3\n");
  if (1 == 2) printf("Copyright 1992-1993, IVT Electronic AB\n");  /* luring ! */
  printCopyright();

  while( argc >= 2  && argv[1][0] == '-' ) {
    while( *++(argv[1]) ) {
      switch( *argv[1] ) {
	case 'o':
	case 'O':
	  option_O = 1;
	  continue;
	case 'p':
	case 'P':
	  option_P = 1;
	  continue;
	case '?':
	  usageSWD();
	  exit(0);
	default:
	  printf( "ogiltigt val: %c", (char *) *argv[1]);
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
  if ((fpLog = fopen(logFile, "r")) <= 0) {
    printf("Cannot open log file '%s'\n", logFile);
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
  if ((varChunk = (char *) calloc(1, varSize)) == NULL) {
    printf("F”r lite minne f”r att allokera %d bytes.\n", varSize);
    exit(errno);
  }
  if (sts = checkOldVarsData("VARS", varSize)) {
    char buf[10];
    if (sts == -1) {		/* another old module */
      if (!option_O)
	fprintf(stderr, "Skriv ”ver befintlig VARS-modul ? (j) ");
      if (option_O || strlen(gets(buf))) {
	if (!option_O && (buf[0] | 0x20) != 'y' && buf[0] | 0x20 !=  'j') {
	  fprintf(stderr, "Avslutar programmet\n");
	  free(varChunk);
	  free(metaModHeader);
	  fclose(fpLog);
	  exit(1);
	}
      }
    } else if (sts == 1) {     /* correct size */
      if (!option_P && !option_O)
	fprintf(stderr, "Patcha befintlig VARS-modul ? (j/n/a) : ");
      if (option_P || option_O || strlen(gets(buf))) {
	if (option_P || (buf[0] | 0x20) == 'y' || (buf[0] | 0x20) ==  'j') {
	  fprintf(stderr, "L„ser VARS-filen..."); fflush(stderr);
	  getVarsData("VARS", varChunk, varSize);
	  fprintf(stderr, "[ok]\n");
	} else if (!option_O && (
			(buf[0] | 0x20) == 'q' || (buf[0] | 0x20) ==  'a')) {
	  fprintf(stderr, "Avslutar programmet\n");
	  free(varChunk);
	  free(metaModHeader);
	  fclose(fpLog);
	  exit(1);
	} else {
	  fprintf(stderr, "Skriver ”ver filen\n");
	}
      }		/* if any reply on input */
    }           /* if correct size of vars */
  }		/* if vars already exists ... */
  fprintf(stderr, "Parsar loggfilen...\n");
  varsFound = 0;
  while (fgets(line, 80, fpLog) != NULL) {
    if (parse(line, var, value)) {
      if ((id = searchVar(metaMod, var)) <= 0)
	continue;
/*
  printf("%d, '%s' = '%s', %4x\n", id, var, value, metaValue(0, metaMod, id)+52);
*/
      varsFound ++;
      putValue(varChunk, metaMod, id, value, fpLog);
    }
  }
/*
  dunkaRam(varChunk, varSize);
*/
  emitHeaderAndChunk("VARS", varChunk, varSize);
  fprintf(stderr, "%d variabler hittade\n", varsFound);
  free(varChunk);
  free(metaModHeader);
  fclose(stdout);
  dup(2);
  system("fixmod -u VARS");
  if (system("fixmod -u VARS") == -1)
    perror("Kan inte k”ra fixmod");
  exit(0);
  return 0;
}

int checkOldVarsData(char *file, int chunkSize)
{
  int siz;
  struct {
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
  } buff;

  if (!(siz = getFileSize(file)))
    return 0;

  if (chunkSize & 1)
    chunkSize ++;
  chunkSize += sizeof(buff) + 4;               /* 4 = size of crc */

  return ((chunkSize == siz) << 1) - 1;	     /*	1 ok, -1 fel */
}

void getVarsData(char *file, char *chunk, int chunkSize)
{
  FILE *fp;
  struct {
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
  } buff;

#ifdef OSK
  if ((fp = fopen(file, "r")) == NULL) {
#else
  if ((fp = fopen(file, "rb")) == NULL) {
#endif
    printf("cannot open file %s\n", file);
    printf("error %d\n", errno);
#ifndef OSK
    perror("unable to open file VARS");
    printf("doserror %d\n", _doserrno);
#endif
    exit(errno);
  }
  if (fread(&buff, sizeof(buff), 1, fp) != 1)
    printf("Error reading datamodule\n");

  if (fread(chunk, chunkSize, 1, fp) != 1)
    printf("Error reading datamodule\n");

  fclose(fp);
}

void dunkaRam(char *s, int siz)
{
  int offset = 0x400, i;
  short int *p;
  unsigned short swapword();
  char *q;
  siz -= offset;
  q = s + offset - 52;
  p = (short int *) q;
  siz += 320;
  while ((siz -= 16) > 0) {
    fprintf(stderr, "0002%04x - ", offset);  offset += 16;
    q = (char *) p;
    for (i = 0; i < 8; i++) {
      fprintf(stderr, "%04x ", swapword(*p++));
    }
    fprintf(stderr, "   ");
    for (i = 0; i < 16; i++, q++) {
      fprintf(stderr, "%c", (isprint(*q) ? *q : '.'));
    }
    fprintf(stderr, "\n");
  }
}

int isStartOfVar(char c)
{
  return isalpha(c) || (c == '_');
}

int isVar(char c)
{
  return isalnum(c) || (c == '_');
}

int isValue(char c)
{
  return isdigit(c) || c=='.' || c=='-' || c=='+' || c=='E' || c=='e';
}

int parse(char *line, char *var, char *value)
{
  if (line[3] != ':' || line[0] == '\n')
    return 0;
  line += 4;
  if (!isStartOfVar(*line))
    return 0;
  while (isVar(*line)) {
    *var ++ = *line ++;
  }
  *var = 0;
  while (*line && !isValue(*line))
    line ++;
  while (*line && isValue(*line))
    *value ++ = *line ++;
  *value = 0;
  return 1;
}


int searchVar(char *meta, char *var)
{
  return metaId(meta, var);
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

void parseCalendar(char *line, struct _calendar *cal, int i)
{
  int w, weekDay = 0, color, stopDay = 0, startTime, stopTime;
  char day[] = { 'M', 'T', 'W', 'T', 'F', 'S', 'S' };

  for (w = 0; w < 7; w++) 
    if (line[w*3] == day[w])
	weekDay |= (1 << w);
  if (weekDay == 0) {
    weekDay = atoi(&line[0]);
    stopDay = atoi(&line[7]);
  } else
    weekDay |= 2048;

  if (line[23] == 'B') color = 1;
  else if (line[23] == 'O') color = 2;
  else if (line[23] == 'R') color = 4;
  else color = 0;

  if (line[32] == 'H') {
    startTime = 0;
    stopTime = 2400;
  } else {
    double atof(), d;
    d = 100.0*atof(&line[32]);
    startTime = d;
    d = 100.0*atof(&line[40]);
    stopTime = d;
  }
  cal->day[i] = (hilo) ? weekDay : swapword(weekDay);
  cal->stopday[i] = (hilo) ? stopDay : swapword(stopDay);
  cal->color[i] = color;
  cal->start[i] = (hilo) ? startTime : swapword(startTime);
  cal->stop[i] = (hilo) ? stopTime : swapword(stopTime);
}

void putValue(char *dm, char *meta, int id, char *value, FILE *fpLog)
{
  int typ, i;
  char line[80];

  /*
  void *p1, *p2;

  p1 = (char *) metaValue(dm, meta, id);
  p2 = (char *) metaLock(dm, meta, id);
  */

 /*

  *((char *) metaLock(dm, meta, 305)) = 0x55;
  *((char *) metaLock(dm, meta, 306)) = 0x66;
  *((char *) metaLock(dm, meta, 307)) = 0x77;

   */



  typ = metaType(meta, id);
  if (typ == 7) {
    long qq;

    qq = atol(value); 		/* changed from atoi(value); 921110 */
    if (!hilo)
      *((long *) metaValue(dm, meta, id)) = swaplong(qq);
    else
      *((long *) metaValue(dm, meta, id)) = qq;
    *((char *) metaLock(dm, meta, id)) = 0;
    printf("int %s = %ld;\n", (char *) metaName(meta, id), qq);
  } else if (typ == 8) {
    double d;
    d = atof(value);
    if (!hilo)
/*      *((double *) metaValue(dm, meta, id)) = swapdouble(d);		*/
	swapdouble2(d, (char*) metaValue(dm, meta, id));
    else
      *((double *) metaValue(dm, meta, id)) = d;
    *((char *) metaLock(dm, meta, id)) = 0;
    printf("float %s = %g;\n", (char *) metaName(meta, id), d);
  } else if (typ == 4) {	/* int vec */
    long *vec, siz;
    char *p;
    siz = metaSize(meta, id) / sizeof(long);
    vec = (long *) metaValue(dm, meta, id);
    printf("int %s[%ld] = {", (char *) metaName(meta, id), siz);
    for (i = 0; i < siz; i++) {
      fgets(line, 80, fpLog);
      p = line;
      while (*p && *p != '=')
	p++;
      p++;
      if (!hilo)
	*vec++ = swaplong(atol(p));
      else
	*vec++ = atol(p);
      printf("%ld", atol(p));
      if (i < (siz - 1)) printf(",");
    }
    printf("}\n");
  } else if (typ == 5) {	/* float vec */
    double *vec;
    int siz;
    char *p;
    siz = metaSize(meta, id) / sizeof(double);
    vec = (double *) metaValue(dm, meta, id);
    printf("int %s[%d] = {", (char *) metaName(meta, id), siz);
    for (i = 0; i < siz; i++) {
      fgets(line, 80, fpLog);
      p = line;
      while (*p && *p != '=')
	p++;
      p++;
      if (!hilo) {
	swapdouble2(atof(p), (char*) vec);
	vec ++;
      } else
	*vec++ = atof(p);
      printf("%g", atof(p));
      if (i < (siz - 1)) printf(",");
    }
    printf("}\n");
  } else if (typ == 15) {
    struct _calendar *cal;
    cal = (struct _calendar *) metaValue(dm, meta, id);
    printf("calendar %s;\n", (char *) metaName(meta, id));
    for (i = 0; i < 10; i++) {
      fgets(line, 80, fpLog);
      parseCalendar(line, cal, i);
    }
  } else {
    printf("This type (%d) not implemented yet, var '%s'\n", typ,
	metaName(meta, id));
  }
  *((char *) metaLock(dm, meta, id)) = 0;

}

void emitHeaderAndChunk(char *file, char *chunk, int chunkSize)
{
  int totsize, odd, align;
  long crc;
  FILE *fp;
  struct {
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
  } buff;
/*
  char name[12];
*/

#ifdef OSK
  if ((fp = fopen(file, "w")) == NULL) {
#else
  if ((fp = fopen(file, "wb")) == NULL) {
#endif
    printf("cannot create file metaMod\n");
    printf("error %d\n", errno);
#ifndef OSK
    perror("unable to create file metaMod");
    printf("doserror %d\n", _doserrno);
#endif    
    exit(errno);
  }
  totsize = chunkSize;
  buff.sync = 0x4afc;
  if (*((char *) &buff.sync) == 0x4a)
    hilo = 1;                             /* like MC68K */
  else
    hilo = 0;                             /* maybe intel */
  buff.sysrev = 1;
  if (totsize & 1) {      /* add pad */
    odd = 1;
    totsize ++;
  } else
    odd = 0;
/*
  if (totsize & 3) {
    align = 1;
    totsize += 2;
  } else
*/
  align = 0;


  buff.size = sizeof(buff) + totsize + 4;               /* 4 = size of crc */
  buff.owner = 0;
  buff.nameoffset = sizeof(buff) - 12;
  buff.access = 0x0777;
  buff.typelang = 0x0400;
  buff.attrrev =  0x8000;
  buff.edition = 1;
  buff.usage = 0;
  buff.symbol = 0;
  buff.ident = 0;
  buff.dataptr = sizeof(buff);
  strcpy(buff.name, "VARS");
  buff.name[4] = buff.name[5] = buff.name[6] = buff.name[7] = 0;
  buff.name[8] = buff.name[9] = buff.name[10] = buff.name[11] = 0;
  buff.parity = 0;
  *((long *) &buff.spare[0]) = 0;
  *((long *) &buff.spare[4]) = 0;
  *((long *) &buff.spare[8]) = 0;

  if (!hilo) {
    buff.sync   = swapword(buff.sync);
    buff.sysrev = swapword(buff.sysrev);  
    buff.size   = swaplong(buff.size);
    buff.owner  = swaplong(buff.owner);
    buff.nameoffset = swaplong(buff.nameoffset);
    buff.access = swapword(buff.access);
    buff.typelang = swapword(buff.typelang);
    buff.attrrev = swapword(buff.attrrev);
    buff.edition = swapword(buff.edition);
    buff.usage   = swaplong(buff.usage);
    buff.symbol  = swaplong(buff.symbol);
    buff.ident   = swapword(buff.ident);
    buff.parity  = swapword(buff.parity);
    buff.dataptr = swaplong(buff.dataptr);
  }
  if (fwrite(&buff, sizeof(buff), 1, fp) != 1)
    printf("cannot write datamodule\n");

  if (fwrite(chunk, chunkSize, 1, fp) != 1)
    printf("cannot write datamodule\n");

  if (odd)
    if (fwrite(chunk, 1, 1, fp) != 1)
      printf("cannot write datamodule\n");

  if (align)
    if (fwrite(chunk, 2, 1, fp) != 1)
      printf("cannot write datamodule\n");

/*
  if (fwrite(name, sizeof(name), 1, fp) != 1)
    printf("cannot write datamodule\n");
*/
  if (fwrite(&crc, sizeof(long), 1, fp) != 1)
    printf("cannot write datamodule\n");

  fclose(fp);
}

void dump(char *s)
{
 int i, j;
 char *d;

 printf("- - - - - - - - - - - -\n");
 for (j = 0; j < 5; j++) {
   d = s;
   for (i = 0; i < 16; i++)
     printf("%02x,", (unsigned char) *s++);
   printf("  ");
   for (i = 0; i < 16; i++, d++)
     printf("%c", isalpha(*d) ? *d : '.');
   printf("\n");
 }
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


