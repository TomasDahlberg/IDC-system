#include <stdio.h>
#include <dir.h>

#define METAMOD "metaMod"

char *metaModHeader = 0;
char *metaMod;

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

getFileSize(char *s)
{
   struct ffblk ffblk1;

   if (findfirst(s,&ffblk1,0)) {
     printf("Sorry, no such file as '%s'\n", s);
     exit(1);
   }
   return ffblk1.ff_fsize;
}

readMetaMod(char *file)
{
  int siz, err;
  FILE *fp;

  siz = getFileSize(file);
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
  if ((fp = fopen(file, "rb")) <= 0) {
    printf("Error opening '%s'\n", METAMOD);
    return 0;
  }
  err = fread(metaModHeader, siz, 1, fp);
  metaMod = metaModHeader + sizeof(struct _metaModHeader);
  fclose(fp);
  return 1;
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



swapMetaMod(char *metaMod)
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

