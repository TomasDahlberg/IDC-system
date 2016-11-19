#include <dir.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>

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

/*
int noOfScreens = 0;
*/

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

typedef void (*PFI)();

extern struct {
  short left, right, up, down, help;
  PFI fcn;
  char *name;
} screens[];

extern struct _datamodule *dm;
extern int _ROOT_SCREEN_POINTER;

char displayBuffer[256];
char oldBuffer[80];

int main(int argc, char *argv[])
{
  FILE *fp1, *fp3;
  char line[256], screen[80], var[80];
  int id, picId, lastPicId, c;
  int varSize;
  char *varChunk;
  char *varChunk2;

  if (argc != 2) {
	printf("Usage: win3208 <metavar>\n");
	exit(1);
  }
/*
!	Algorithm:
!		read metamod
!		create vars structure
!		call first screen
*/
  readMetaMod(argv[1]);
  hilo = 0x4afc;
  if (*((char *) &hilo) == 0x4a)
    hilo = 1;                             /* like MC68K */
  else
    hilo = 0;                             /* maybe intel */
  if (!hilo)
    swapMetaMod(metaMod);
/*
!	now initialize data module vars
*/
  varSize = getTotalSizeOfVars(metaMod);
  if ((varChunk = (char *) calloc(1, varSize)) == NULL) {
    printf("Not enough memory to allocate %d bytes (VARS).\n", varSize);
    exit(errno);
  }
/*
  if ((varChunk2 = (char *) calloc(1, varSize)) == NULL) {
    printf("Not enough memory to allocate %d bytes (VARS2).\n", varSize);
    exit(errno);
  }
*/
  dm = varChunk;

/*  dumpScr();	*/

  doMap();

  picId = _ROOT_SCREEN_POINTER - 1;	/* ??? */
  lastPicId = picId - 1;		/* not the same anyway */
  while (1) {
/*
!	if new pic, get all its global vars
*/
    if (picId != lastPicId) {
	lastPicId = picId;
	printTags(picId);
    }

    for (c = 0; c < 80; c ++)
      displayBuffer[c] = 0;

    (screens[picId].fcn)();
    if (memcmp(displayBuffer, oldBuffer, 80)) {
      memcpy(oldBuffer, displayBuffer, 80);
      printf("+----------------------------------------+\n");
      printf("!%40.40s!\n", displayBuffer);
      printf("!%40.40s!\n", &displayBuffer[40]);
      printf("+----------------------------------------+\n");
      printf("\n");
    }
/*
!	check if any vars have been changed
*/


/*
!	now check if any arrow has been pressed
*/
    if (kbhit()) {
      c = getch();
      if (c == 0) {
	c = getch();
	if (c == 72 && screens[picId].up)
	  picId = screens[picId].up - 1;
	if (c == 77 && screens[picId].right)
	  picId = screens[picId].right - 1;
	if (c == 80 && screens[picId].down)
	  picId = screens[picId].down - 1;
	if (c == 75 && screens[picId].left)
	  picId = screens[picId].left - 1;
      }
    }
  }

  free(metaModHeader);
  return 0;
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
