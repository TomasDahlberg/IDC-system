#include <dir.h>
#include <alloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define LEXICON "LEXICON"
char *lexiconHeader;
char *lexicon;
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

void swapLexicon(char *lexicon)
{
  unsigned short q;
  int entriesInTable;
  unsigned short int *idx;

  q = 0x4afc;
  if (*((char *) &q) == 0x4a)
    return;
/*
! 	emit lexicon, starting with byte index table 
*/
  idx = (unsigned short int *) lexicon;
  for (q = 0; q < 64; q++) {
    *idx = swapword(*idx);
    idx ++;
  }
/*
!	next is word index table
*/
  entriesInTable = swapword(*idx) / 4;
  for (q = 0; q < entriesInTable; q++) {
    *idx = swapword(*idx);
    idx ++;
    idx ++;
  }
}

void readLexicon(void)
{
  int siz;
  FILE *fp;

  siz = getFileSize(LEXICON);
  if (!(lexiconHeader = (char *) malloc(siz))) {
    printf("not enough memory\n");
    exit(1);
  }
  if ((fp = fopen(LEXICON, "rb")) <= 0) {
    printf("Error opening '%s'\n", LEXICON);
    exit(1);
  }
  fread(lexiconHeader, siz, 1, fp);
  lexicon = lexiconHeader + sizeof(struct _metaModHeader);
  swapLexicon(lexicon);
  fclose(fp);
}

struct _lexicon {
  unsigned short shortIdxMap[64];
  struct {
    unsigned short offset;
    unsigned char shortCode;		// 0xff if none
    unsigned char pad;
  } map[1024];
/*
  char names[40000];
*/
} *buff;

char *lexiconQuickName(char *lexicon, int idx)
{
  struct _lexicon *buff;
  buff = (struct _lexicon *) lexicon;
  if (idx < 0 || idx > 63)
    return (char *) 0;
  return ((char *) buff->map) + buff->map[buff->shortIdxMap[idx]].offset;
}

char *lexiconName(char *lexicon, int idx)
{
  struct _lexicon *buff;
  int antal;

  buff = (struct _lexicon *) lexicon;
  antal = buff->map[0].offset / sizeof(buff->map[0]);
  if (idx < 0 || idx > antal)
    return (char *) 0;
  return ((char *) buff->map) + buff->map[idx].offset;
}

int lexiconShortCode(char *lexicon, int idx)
{
  struct _lexicon *buff;
  int antal;

  buff = (struct _lexicon *) lexicon;
  antal = buff->map[0].offset / sizeof(buff->map[0]);
  if (idx < 0 || idx > antal)
    return 0;
  return buff->map[idx].shortCode;
}

int lexiconIdx(char *lexicon, char *name)
{
  struct _lexicon *buff;
  int antal;
  int id, a, b, match;

  buff = (struct _lexicon *) lexicon;
  a = 0; /* 1; */
  b = buff->map[0].offset / sizeof(buff->map[0]);
  while (1) {
    id = (a + b) / 2;
    if ((match = strcmp(((char *) buff->map) + buff->map[id].offset, name)) < 0){
      if (a == b)
        break;
      if (a == id) {
        a = b;
        continue;               /* break */
      }
      a = id;                       /* go forward */
    } else if (match > 0) {
      if (a == b)
        break;
      if (b == id) {
        b = a;
        continue;     /* break */
      }
      b = id;                       /* back */
    } else
      break;                        /* ok, found ! */
  }
  printf("lexiconIdx: a = %d, b = %d, %s, id = %d\n", a, b, 
				(match == 0) ? "MATCH" : "NO MATCH", id);
  return (match == 0) ? id : -1;
}

static int matchLen(char *table, char *our)
{
  int match = 0;
  while (*our) {
    if (*table != *our) {
      match = 0;
      break;
    }
    our ++;
    table ++;
    match ++;
  }
  return match;
}

int lexiconCloseIdx(char *lexicon, char **name)
{
  struct _lexicon *buff;
  int antal;
  int id, a, b, match, match1, match2;

  buff = (struct _lexicon *) lexicon;
  a = 0; /* 1; */
  b = buff->map[0].offset / sizeof(buff->map[0]);
  while (1) {
    if (a + 1 == b) {
      if (strcmp(((char *) buff->map) + buff->map[a].offset, *name) == 0)
	return a;
      if (strcmp(((char *) buff->map) + buff->map[b].offset, *name) == 0)
	return b;

      match1 = matchLen(((char *) buff->map) + buff->map[a].offset, *name);
      match2 = matchLen(((char *) buff->map) + buff->map[b].offset, *name);
/*
      printf("match1 = %d, match2 = %d\n", match1, match2);
*/
      if (match1 < 2 && match2 < 2) /* less than 2chars match ! */
	return -1;
      if (match1 > match2) {
	*name += match1;
	return a;
      } else {
	*name += match2;
	return b;
      }
    }
    id = (a + b) / 2;
    if ((match = strcmp(((char *) buff->map) + buff->map[id].offset, *name)) < 0){
      if (a == b)
        break;
      if (a == id) {
        a = b;
        continue;               /* break */
      }
      a = id;                       /* go forward */
    } else if (match > 0) {
      if (a == b)
        break;
      if (b == id) {
        b = a;
        continue;     /* break */
      }
      b = id;                       /* back */
    } else
      break;                        /* ok, found ! */
  }
  printf("lexiconIdx: a = %d, b = %d, %s, id = %d\n", a, b, 
				(match == 0) ? "MATCH" : "NO MATCH", id);
  return (match == 0) ? id : -1;
}

main()
{
  int q, antal;
  char nbuf[80], *n;
  time_t t1, t2;
  int tt;

  readLexicon();
  buff = (struct _lexicon *) lexicon;

/*
  printf("%s -> ");
*/

  n = nbuf;
  strcpy(n, "Anget hud sd hej");
  q = lexiconCloseIdx(lexicon, &n);

  n = nbuf;
  strcpy(n, "Pa");
  q = lexiconCloseIdx(lexicon, &n);

t1 = time(0);
for (tt = 0; tt < 1000; tt++) {

  n = nbuf;
  strcpy(n, "Anger med hur m†nga atomer");
  do {
    q = lexiconCloseIdx(lexicon, &n);
/*
    printf("q = %d, Kvar: '%s'\n", q, n);
*/
  } while (q != -1);

}
t2 = time(0);
printf("Elapsed time : %ld\n", t2 - t1);


  printf("Quick Table:\n");
  for (q = 0; q < 64; q++) {
    printf("%2d: -> '%s'\n", q, lexiconQuickName(lexicon, q));
/*
    printf("%2d: -> %3d '%s'\n", q, buff->shortIdxMap[q], 
      ((char *) buff->map) + buff->map[buff->shortIdxMap[q]].offset);
*/
  }
  printf("Table:\n");

  antal = buff->map[0].offset / sizeof(buff->map[0]);
  printf("Antal = %d\n", antal);
  for (q = 0; q < antal; q++) {
    printf("%3d (%2d) '%s'\n", q, lexiconShortCode(lexicon, q),
					lexiconName(lexicon, q));
/*
    printf("%3d (%2d) '%s'\n", q, buff->map[q].shortCode,
      ((char *) buff->map) + buff->map[q].offset);
*/
  }

  free(lexiconHeader);
  return 0;
}