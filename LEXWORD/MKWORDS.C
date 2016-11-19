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
  char word[80], wordBuf[80];
  char *wordPtr, *wptr;
  int wordLen, i;

  while (*p && *p != '\"') {
    while (*p == ' ')
      p++;
    wordPtr = word;
    wordLen = 0;
    wptr = wordPtr;
    while (*p && *p != ' ' && *p != '\"' && *p != '\\' && *p != '%') {
      if (wordLen++ < WORD_LENGTH)
	*wordPtr++ = *p++;

      if (*p == ' ' && *(p+1) != ' ') {

	*wordPtr = 0;  /* temp */
	strcpy(wordBuf, wptr);	/* added */
	if (strlen(wordBuf) > 1)
	  insert(wordBuf);
	wptr = wordPtr;

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

   for (i = 0; i < NO_OF_LINKS; i++)
     wordLink[i] = (WORD *) 0;

  if ((fp = fopen(argv[1], "r")) == NULL) {
    printf("cannot open file '%s'\n", argv[1]);
    exit(1);
  }
  while (fgets(buf, 256, fp)) {
/*
    pc2intern(buf);
*/
    if (p = checkUp(buf, "display(\""))
      takeWords(p);
    if (p = checkUp(buf, "enter(\""))
      takeWords(p);
  }
  fclose(fp);

  if ((fp = fopen("sentence.lis", "w")) == NULL) {
    printf("cannot create file '%s'\n", "sentence.lis");
    exit(1);
  }
  for (alpha = i = currentOffset = 0; alpha < NO_OF_LINKS; alpha++) {
    WORD *tmp;
    if (tmp = wordLink[alpha])
      while (tmp) {   
	fprintf(fp, "%3d, '%s'\n", tmp->antal, tmp->word);
	tmp = tmp->next;
      }
  }
  fclose(fp);
}