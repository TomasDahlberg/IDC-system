/*
!	This program takes a list file and encapsulates it in a data module
!	with header and trailing crc. So it will be loadable for an OS-9
!	target system.
*/

#include <stdio.h>
#include <errno.h>
#include <math.h>
#include <ctype.h>

char *metaModHeader;
char *metaMod;
short int hilo;

struct _header {
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

main(argc, argv)
int argc;
char *argv[];
{
  FILE *fpLog, *fpOut;
  char *logFile, line[512], file[63];
  long totSize;

  printf("encaps, version 1.0\n");
  printf("Copyright 1992, IVT Electronic AB\n");
  if (argc != 2) {
    printf("Usage: encaps <list-file>\n");
    printf("\n");
    printf("Function:\n");
    printf("       takes the specified list file and encapsulates it with\n");
    printf("       a header and trailing crc to form a loadable module\n");
    printf("       for an OS-9 target system\n");
    exit(1);
  }
  hilo = 0x4afc;
  if (*((char *) &hilo) == 0x4a)
    hilo = 1;                             /* like MC68K */
  else
    hilo = 0;                             /* maybe intel */
  logFile = argv[1];
  if ((fpLog = fopen(logFile, "r")) <= 0) {
    printf("Cannot open log file '%s'\n", logFile);
    exit(1);
  }
  strcpy(file, "lisMod");
#ifdef OSK
  if ((fpOut = fopen(file, "w")) == NULL) {
#else
  if ((fpOut = fopen(file, "wb")) == NULL) {
#endif
    printf("cannot create file '%s'\n", file);
    printf("error %d\n", errno);
#ifndef OSK
    perror("unable to create file");
    printf("doserror %d\n", _doserrno);
#endif    
    exit(errno);
  }

  emitHeader(fpOut);
  totSize = sizeof(struct _header);
  printf("Copying list file...\n");
  while (fgets(line, 512, fpLog) != NULL) {
    int i;
    i = strlen(line);
    totSize += i;
    fputs(line, fpOut);
  }
  {
    short int endMark = 0xffff;
    if (fwrite(&endMark, 2, 1, fpOut) != 1)
      printf("cannot write datamodule");
    totSize += 2;
  }
  if (totSize & 1) {
    totSize ++;
    if (fwrite(&totSize, 1, 1, fpOut) != 1)
      printf("cannot write datamodule");
  }
  totSize += sizeof(long);
  emitTrailer(fpOut);
  updateSize(fpOut, totSize);
  fclose(fpOut);
  fclose(fpLog);
  system("fixmod -u lisMod");
  system("fixmod -u lisMod");
  exit(0);
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

updateSize(fp, siz)
FILE *fp;
long siz;
{
  fseek(fp, 4, 0 /* SEEK_SET */);

  if (!hilo)
    siz = swaplong(siz);
  if (fwrite(&siz, sizeof(long), 1, fp) != 1)
    printf("cannot write datamodule");
}


emitHeader(fp)
FILE *fp;
{
  int totsize, odd;
  long crc;
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

  totsize = 0x0123;
  buff.sync = 0x4afc;
  if (*((char *) &buff.sync) == 0x4a)
    hilo = 1;                             /* like MC68K */
  else
    hilo = 0;                             /* maybe intel */
  buff.sysrev = 1;
/*
  if (totsize & 1) {
    odd = 1;
    totsize ++;
  } else
    odd = 0;
*/
  buff.size = sizeof(buff) + totsize + 4;                 /* 4 = size of crc */
  buff.owner = 0;
  buff.nameoffset = (int) ( buff.name  - ((int) &buff));
  buff.access = 0x0111;
  buff.typelang = 0x0400;
  buff.attrrev =  0x8001;
  buff.edition = 0;
  buff.dataptr = sizeof(buff);
  strcpy(buff.name, "LISMOD");
  buff.name[6] = buff.name[7] = 0;
  buff.name[8] = buff.name[9] = buff.name[10] = buff.name[11] = 0;
  buff.usage = 0;
  buff.symbol = 0;
  buff.ident = 0;
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
    printf("cannot write datamodule");
}
/*
  if (fwrite(chunk, chunkSize, 1, fp) != 1)
    printf("cannot write datamodule");
  fclose(fp);
}
*/

emitTrailer(fp)
FILE *fp;
{
  long crc;
  if (fwrite(&crc, sizeof(long), 1, fp) != 1)
    printf("cannot write datamodule");
}

/*
dump(char *s)
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
*/
