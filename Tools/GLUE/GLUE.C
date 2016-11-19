/*
! 	glue takes output from idcc special format and metavar and
!	generates C definitions.
!
!	Coding started:	8:23:00, and ended: 10:06:00
!
!	In data:
!		welcome:level
!		welcome:GT63
!		welcome:GT58
!		welcome:GT55
!		screen2:GT48
!		screen3:GT47
!		screen3:GT3
!	Algorithm:
!		read entry
!		lookup id in metavar
!		skip illegal variables (functions)
!		make sorted link of 'name',ids1,ids2 etc
!		emit all screen names
!		emit all ids
!		emit array
!
!	Out data:
!
static char x[] = "welcome\0screen2\0screen3\0";
static int ids[] = { 12, 23, 18, 17, -1, 3, -1, 5, 2, -1};

struct {
  char *name;
  int *ids;
} scr[] = {
	{ x+0, ids },
	{ x + 8, ids + 5},
	{ x + 16, ids + 8}
};
int noOfScreens = 3;
!
*/

#include <dir.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *metaName(char *, int);
int metaRemote(char *, int);
int metaSize(char *, int);
char *metaValue(char *, char *, int);
int metaType(char *, int);
int metaId(char *, char *);
int metaLock(char *, char *, int);
void swapMetaMod(char *);

void emitIds(FILE *fp);
void emitOffsets(FILE *fp);
void emitNames(FILE *fp);
void insertLink(char *screen, int id);
int  parse(char *line, char *screen, char *var);
int  lookupId(char *var);
void readMetaMod(char *name);

int noOfScreens = 0;

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

main(int argc, char *argv[])
{
  FILE *fp1, *fp3;
  char line[256], screen[80], var[80];
  int id;

  if (argc != 3) {
	printf("Usage: glue <data.lis> <metavar>\n");
	exit(1);
  }
  if (!(fp1 = fopen(argv[1], "r"))) {
	printf("cannot open file %s\n", argv[1]);
	exit(1);
  }
  if (!(fp3 = fopen("output.c", "w"))) {
	printf("cannot create output file %s\n", "output.c");
	exit(1);
  }
/*
!	Algorithm:
!		read entry
!		lookup id in metavar
!		skip illegal variables (functions)
!		make sorted link of 'name',ids1,ids2 etc
!
!		emit all screen names
!		emit all ids
!		emit array
*/
  readMetaMod(argv[2]);
  hilo = 0x4afc;
  if (*((char *) &hilo) == 0x4a)
    hilo = 1;                             /* like MC68K */
  else
    hilo = 0;                             /* maybe intel */
  if (!hilo)
    swapMetaMod(metaMod);
  while (fgets(line, 255, fp1)) {
    if (!parse(line, screen, var))
      continue;
    if ((id = lookupId(var)) >= 0) {
      insertLink(screen, id);
    }
  }
  fprintf(fp3, "static char sn[] = \n\"");
  emitNames(fp3);
  fprintf(fp3, "\";\n");

  fprintf(fp3, "static int ids[] = {\n");
  emitIds(fp3);
  fprintf(fp3, "\n};\n");

  fprintf(fp3, "struct { char *name; int *ids; } scr[] = {\n");
  emitOffsets(fp3);
  fprintf(fp3, "};\n");
  fprintf(fp3, "int noOfScreens = %d;\n", noOfScreens);

  fclose(fp1);
  free(metaModHeader);
  fclose(fp3);
  return 0;
}

#define MAX_IDS 10
struct _link {
  char name[32];
  int noOfIds;
  int ids[MAX_IDS];
  struct _link *next;
} *rootLink;
typedef struct _link link;

/* static int ids[] = { 12, 23, 18, 17, -1, 3, -1, 5, 2, -1}; */
void emitIds(FILE *fp)
{
  link *tmp;
  int i;
  tmp = rootLink;
  while (tmp) {
    for (i = 0; i < tmp->noOfIds; i++)
      fprintf(fp, "%d, ", tmp->ids[i]);
    fprintf(fp, "-1, \n");
    tmp = tmp->next;
  }
  fprintf(fp, "-1");
}
/*	{ x+0, ids },
	{ x + 8, ids + 5},
	{ x + 16, ids + 8}	*/
void emitOffsets(FILE *fp)
{
  link *tmp;
  int nameOffset = 0, idsOffset = 0;
  tmp = rootLink;
  while (tmp) {
    fprintf(fp, "{ sn+%d, ids+%d },\n", nameOffset, idsOffset);
    nameOffset += strlen(tmp->name) + 1;
    idsOffset += tmp->noOfIds + 1;	/* +1 f”r -1 */
    tmp = tmp->next;
  }
  fprintf(fp, "{ 0, 0 }\n");
}

/* static char x[] = "welcome\0screen2\0screen3\0"; */
void emitNames(FILE *fp)
{
  link *tmp;
  tmp = rootLink;
  while (tmp) {
    fprintf(fp, "%s\\0\\\n", tmp->name);
    tmp = tmp->next;
    noOfScreens ++;
  }
}

/* make sorted link of 'name',ids1,ids2 etc */
void insertLink(char *screen, int id)
{
  link *tmp, *tmpLink, *prevLink;
  int f;
  prevLink = tmp = rootLink;
  while (tmp) {
    if (!(f = strcmp(tmp->name, screen))) {
      if (tmp->noOfIds >= MAX_IDS) {
	printf("too many ids in screen block\n");
      } else {
	for (f = 0; f < tmp->noOfIds; f++)
	  if (tmp->ids[f] == id)
	    return;
	tmp->ids[tmp->noOfIds++] = id;
      }
      return ;
    } else if (f > 0)
	break;
    prevLink = tmp;
    tmp = tmp->next;
  }
  tmpLink = (struct _link *) malloc(sizeof(struct _link));
  strcpy(tmpLink->name, screen);
  tmpLink->noOfIds = 1;
  tmpLink->ids[0] = id;
  tmpLink->next = tmp;
  if (tmp == rootLink)
    rootLink = tmpLink;
  else
    prevLink->next = tmpLink;
}

int parse(char *line, char *screen, char *var)
{
  char *p;
  if (!(p = strchr(line, ':'))) {
    printf("%s\nerror parsing input, : not found\n", line);
    return 0;
  }
  *p = 0;
  strcpy(screen, line);
  strcpy(var, p + 1);
  if ((p = strchr(var, 0x0a)))
    *p = 0;
  return 1;
}

int  lookupId(char *var)
{
  int id, type;
  id = metaId(metaMod, var);
  if (id >= 0) {
      type = metaType(metaMod, id);
      if (type == 11 || type == 12)
	return -1;
  }
  return id;
}

static int getFileSize(char *s)
{
   struct ffblk ffblk1;
   if (findfirst(s,&ffblk1,0)) {
     return 0;
   }
   return ffblk1.ff_fsize;
}

void readMetaMod(char *name)
{
  int siz;
  FILE *fp;
  if (!(siz = getFileSize(name))) {
     printf("Sorry, no such file as '%s'\n", name);
     exit(1);
  }
  if (!(metaModHeader = (char *) malloc(siz))) {
    printf("not enough memory\n");
    exit(1);
  }
  if ((fp = fopen(name, "rb")) <= 0) {
    printf("Error opening '%s'\n", name);
    exit(1);
  }
  fread(metaModHeader, siz, 1, fp);
  metaMod = metaModHeader + sizeof(struct _metaModHeader);
  fclose(fp);
}

unsigned short swapword(unsigned short w)
{
  return ((w & 0xff) << 8) | ((w >> 8) & 0xff);
}

unsigned long swaplong(unsigned long l)
{
  unsigned short w1, w2;
  w1 = (l >> 16) & 0xffff;
  w2 = l & 0xffff;
  return (((unsigned long) swapword(w2)) << 16) | swapword(w1);
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
