#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char *checkUp(char *s, char *p)
{
  char *q;
  if (q = strstr(s, p))
    return q + strlen(p);
  return q;
}

#define WORD_LENGTH 80
/*
#define NO_OF_WORDS 1000
char words[NO_OF_WORDS][WORD_LENGTH];
int antal[NO_OF_WORDS];
int nextFree = 0;
*/

typedef struct _word {
  struct _word *next;
  int antal;
  char word[80];  
} WORD;

/*
  pointer to link of words starting with index
  that is, wordLink['p'] points to word starting with 'p'
*/
#define NO_OF_LINKS 256
WORD *wordLink[NO_OF_LINKS];

void initLink()
{
  int i;
  for (i = 0; i < NO_OF_LINKS; i++) 
    wordLink[i] = (WORD *) 0;
}

WORD *alloc(char *word)
{
  WORD *tmp;
  if ((tmp = (WORD *) malloc(sizeof(WORD))) == (WORD *) NULL) {
    printf("Ran out of memory\n");
    exit(99);
  }
    strcpy(tmp->word, word);
    tmp->antal = 1;
    tmp->next = (WORD *) 0;
  return tmp;
}

void insert(unsigned char *word)
{
  WORD *tmp, *link;

  if ((link = wordLink[*word]) == (WORD *) 0) {
  /* special case */

    wordLink[*word] = alloc(word);
    return ;
  }
  if (strcmp(link->word, word) == 0) {
    link->antal ++;
    return;
  }

  if (strcmp(link->word, word) > 0) { /* replace root link with new */
    tmp = link;
    link = wordLink[*word] = alloc(word);
    link->next = tmp;
    return ;
  }

  while (link->next && (strcmp(link->next->word, word) < 0))
    link = link->next;

  if (link->next && (strcmp(link->next->word, word) == 0)) {
    /* already exists */
    link->next->antal ++;
    return ;
  }
  tmp = alloc(word);
  tmp->next = link->next;
  link->next = tmp;
}

void takeWords(char *p)
{
  char word[80];
  char *wordPtr;
  int wordLen, i;

  while (*p && *p != '\"') {
    while (*p == ' ')
      p++;
    wordPtr = word;
    wordLen = 0;
    while (*p && *p != ' ' && *p != '\"' && *p != '\\' && *p != '%') {
      if (wordLen++ < WORD_LENGTH)
	*wordPtr++ = *p++;

      if (*p == ' ' && *(p+1) != ' ') {
	*wordPtr++ = *p++;
      }

    }
    *wordPtr = 0;
    while (*p && *p != ' ' && *p != '\"')
      p++;

    if (strlen(word) > 1)
      insert(word);

  }
}

int antal[256];
int idxMap[256];

struct {
  unsigned short int offset;
  unsigned char shortCode;
  unsigned char pad;
} table[1024];
int entriesInTable = 0;

void pc2intern(unsigned char *buf)
{
  int i, l;
  l = strlen(buf);
  for (i = 0; i < l; i++) {
    if (*buf == 134)
      *buf = 6;
    else if (*buf == 132)
      *buf = 4;
    else if (*buf == 148)
      *buf = 20;
    else if (*buf == 143)
      *buf = 16;
    else if (*buf == 142)
      *buf = 14;
    else if (*buf == 153)
      *buf = 25;
    else if (*buf == 248)
      *buf = 15;
    buf++;
  }
}

void intern2pc(unsigned char *buf)
{
  int i, l;
  l = strlen(buf);
  for (i = 0; i < l; i++) {
    if (*buf == 6)
      *buf = 134;
    else if (*buf == 4)
      *buf = 132;
    else if (*buf == 20)
      *buf = 148;
    else if (*buf == 16)
      *buf = 143;
    else if (*buf == 14)
      *buf = 142;
    else if (*buf == 25)
      *buf = 153;
    else if (*buf == 15)
      *buf = 248;
    buf++;
  }
}

main(int argc, char *argv[])
{
  FILE *fp;
  char buf[256], *p;
  int mx1, idx, q, i, alpha;
  WORD *pek, *sort[256];
  void emitFile(char *file, int chunkSize);
  int currentOffset, chunkSize;

  if ((fp = fopen(argv[1], "r")) == NULL) {
    printf("cannot open file '%s'\n", argv[1]);
    exit(1);
  }
  while (fgets(buf, 256, fp)) {
    pc2intern(buf);
    if (p = checkUp(buf, "display(\""))
      takeWords(p);
    if (p = checkUp(buf, "enter(\""))
      takeWords(p);
  }

  for (alpha = i = currentOffset = 0; alpha < NO_OF_LINKS; alpha++) {
    WORD *tmp;
    if (tmp = wordLink[alpha])
      while (tmp) {   
	tmp = tmp->next;
	i++;
      }
  }
  currentOffset = i * sizeof(table[0]);
  for (alpha = i = 0; alpha < NO_OF_LINKS; alpha++) {
    WORD *tmp;
    if (tmp = wordLink[alpha])
      while (tmp) {   
	printf("%3d: antal %3d, '%s'\n", i, tmp->antal, tmp->word);
	table[i].offset = currentOffset;
	currentOffset += strlen(tmp->word) + 1;
	table[i].shortCode = 0xff;		/* no short code yet */
	tmp = tmp->next;
	i++;
	entriesInTable = i;
      }
  }
  chunkSize = 2*64 + currentOffset;
  printf("Sorting...");
  for (q = 0; q < 64; q++) {
   sort[q] = 0;
   antal[q] = 0;
   mx1 = 0;

   for (alpha = i = 0; alpha < NO_OF_LINKS; alpha++) {
    WORD *tmp;
    if (tmp = wordLink[alpha])
      while (tmp) {   
/*
	printf("%3d: antal %3d, '%s'\n", i, tmp->antal, tmp->word);
*/
	if (tmp->antal > mx1) {
	  pek = tmp;
	  mx1 = tmp->antal;
	  idx = i;
	}
	tmp = tmp->next;
	i++;
      }
   }
   if (mx1 > 0) {
     sort[q] = pek;
     antal[q] = mx1;
     idxMap[q] = idx;
     pek->antal = 0;
     table[idx].shortCode = q;
   }
  }
  printf("ok\n");
  for (q = 0; q < 64; q++) {
    if (antal[q])
      printf("%3d: idxMap = %d, antal %3d, '%s'\n", q, idxMap[q], antal[q], sort[q]->word);
  }
  fclose(fp);
/*
    emit data module table
*/
  emitFile("lexicon", chunkSize);
}

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

struct {
  unsigned short shortIdxMap[64];
  struct {
    unsigned short offset;
    unsigned char shortCode;		// 0xff if none
    unsigned char pad;
  } map[1024];
/*
  char names[40000];
*/
} buff;

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

void emitFile(char *file, int chunkSize)
{
  int totsize, odd, align, q, alpha;
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
  strcpy(buff.name, "LEXICON");
  buff.name[7] = 0;
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
/*
! 	emit lexicon, starting with byte index table 
*/

  for (q = 0; q < 64; q++) {
    unsigned short int idx;
    idx = (antal[q]) ? idxMap[q] : 0xffff;
    if (!hilo) 
      idx = swapword(idx);
    if (fwrite(&idx, sizeof(unsigned short int), 1, fp) != 1)
      printf("cannot write datamodule\n");
  }
/*
!	next is word index table
*/
  for (q = 0; q < entriesInTable; q++) {
    unsigned short int offset;
    unsigned char shortCode;

    offset = (hilo) ? table[q].offset : swapword(table[q].offset);
    shortCode = table[q].shortCode;
    if (fwrite(&offset, sizeof(offset), 1, fp) != 1)
      printf("cannot write datamodule\n");
    if (fwrite(&shortCode, sizeof(shortCode), 1, fp) != 1)
      printf("cannot write datamodule\n");
    shortCode = 0;	/* dummy, to align next offset word */
    if (fwrite(&shortCode, sizeof(shortCode), 1, fp) != 1)
      printf("cannot write datamodule\n");
  }
/*
! 	then, it's the names
*/
  for (alpha = 0; alpha < NO_OF_LINKS; alpha++) {
    WORD *tmp;
    if (tmp = wordLink[alpha])
      while (tmp) {   
	if (fwrite(tmp->word, strlen(tmp->word) + 1, 1, fp) != 1)
	  printf("cannot write datamodule\n");
	tmp = tmp->next;
      }
  }
  if (odd)
    if (fwrite(&odd, 1, 1, fp) != 1)
      printf("cannot write datamodule\n");

  if (align)
    if (fwrite(&align, 2, 1, fp) != 1)
      printf("cannot write datamodule\n");

  if (fwrite(&crc, sizeof(long), 1, fp) != 1)
    printf("cannot write datamodule\n");

  fclose(fp);
}
 
