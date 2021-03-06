#include <stdio.h>
#include <stdlib.h>
#include <string.h>

long int getNextWord(FILE *fp)
{
  static unsigned int buf[128], w;
  static int n = -1;
  static int mx = 128;

  if (n < 0 || mx <= n) {
    mx = fread(buf, 2, 128, fp);
    if (mx == 0)
      return -1;
    n = 0;
  }
  w = buf[n];
  n++;
  return w;
}

unsigned int swap(unsigned int w)
{
  return ((w & 0x00ff) << 8) | ((w & 0xff00) >> 8);
}

long filesize(FILE *stream)
{
   long curpos, length;

   curpos = ftell(stream);
   fseek(stream, 0L, SEEK_END);
   length = ftell(stream);
   fseek(stream, curpos, SEEK_SET);
   return length;
}

void main(argc, argv)
int argc;
char *argv[];
{
  FILE *fpIn, *fpOut;
  int i, state;

#define SEARCH_HEADER		0x01
#define EXTRACT_MODULE		0x02
#define EXTRACT_HEADER		0x03
#define EXIT			0x04
#define RESTORE			0x05

  int np;
  union {
    char name[60];
    unsigned int wname[30];
  } u;
  long int w, wCnt, nameCnt, hsum, fpos, fkomihog, fsize;
  unsigned int head[24];
  unsigned int head2[24];
  char buf[60];

/*
  unsigned char buf[256];
  unsigned char outBuf[256];
*/

  if (argc < 2 || argc > 3) {
	printf("Usage: extract <bin-file> [module]\n");
	printf("       bin-file contains OS-9 modules\n");
	printf("       module is a specific module to extract\n");
	printf("\n       if no [module] is specified, all modules\n");
	printf("       will be extracted\n");
	exit(1);
  }
  if (argc == 3) {
    printf("Not implemented yet\n");
    exit(1);
  }

  if ((fpIn = fopen(argv[1], "rb")) == 0) {
	printf("cannot open '%s'\n", argv[1]);
	exit(1);
  }
  fsize = filesize(fpIn) >> 1;

  state = SEARCH_HEADER;
  while (1) {
    fpos = ftell(fpIn);
    w = getNextWord(fpIn);
    fsize --;
    if (w == -1)
      state = EXIT;
    switch (state) {
      case SEARCH_HEADER:
	if (w == 0xfc4a) {
	  state = EXTRACT_HEADER;
	  printf("Extract module... "); fflush(stdout);
	}
	break;
      case EXTRACT_HEADER:
	fkomihog = fpos;
	head[0] = 0xfc4a;                   /* sync word */
	head[1] = w;
	for (i = 2; i < 24; i++) {
	  w = getNextWord(fpIn);
	  fsize --;
	  if (w == -1) {
	    printf("file ended in module header\n");
	    state = EXIT;
	    break;
	  }
	  head[i] = w;
	}
	wCnt = (swap(head[3]) >> 1) - 24;	/* remaining no of words in file */
	nameCnt = (swap(head[7]) >> 1) - 23;
	if (wCnt > fsize || nameCnt > fsize) {
	  state = RESTORE;
	  break;
	}
	np = 0;

	for (hsum = i = 0; i < 24; i++) {
	  hsum ^= head[i];
	}
	if (hsum != 0xffff) {
	  state = RESTORE;
	  break;
	}
	if ((fpOut = fopen("temp.mod", "wb")) == 0) {
	  printf("cannot open temporary file 'temp.mod'\n");
	  exit(1);
	}
	fwrite(&head[0], 2, 24, fpOut);	/* emit to file */
	state = EXTRACT_MODULE;
	break;
      case EXTRACT_MODULE:
	fwrite(&w, 2, 1, fpOut);	/* emit to file */
	wCnt --;
	if (nameCnt > 0)
	  nameCnt --;
	if (nameCnt == 0) {
	  u.wname[np++] = w;
	  if ((w & 0xff) == 0 || (w & 0xff00) == 0)
	    nameCnt = -1;
	}
	if (wCnt <= 0) {
	  state = SEARCH_HEADER;
	  fclose(fpOut);
	  fpOut = 0;
	  printf("%s\n", u.name);
/*
! 	since we now should know the real module name, rename the file
*/
	  i = 1;
	  if ((fpOut = fopen(u.name, "rb")) != NULL) {
	    if (24 == fread(head2, 2, 24, fpOut)) {
	      if ((swap(head[10]) & 0xff) <= (swap(head2[10]) & 0xff)) {
		printf("Overwrite '%s', rev %02x with revision %02x ? ",
			u.name,
			swap(head2[10]) & 0xff,
			swap(head[10]) & 0xff
			);
		if (strlen(gets(buf))) {
		  if (buf[0] == 'n' || buf[0] == 'N')
		    i = 0;
		}
	      }  /* if old rev > new rev */
	      if (i) {
		printf("Overwriting... %s\n", u.name);
		remove(u.name);
	      }
	    }	/* if fread ok */
	  }  	/* if able to open old file */
	  if (i) {
	    if (-1 == rename("temp.mod", u.name))
	      printf("Error renaming file, errno = %d\n", errno);
	  }
	}
	break;
      case EXIT:	/* close file if any open */
	if (fpOut)
	  fclose(fpOut);
	fpOut = 0;
	break;
    }
    if (state == RESTORE) {
      state = SEARCH_HEADER;
      fseek(fpIn, fkomihog, SEEK_SET);
      printf("Restoring filepointer...\n");
    }
    if (state == EXIT)
	break;
  }
  fclose(fpIn);
}
