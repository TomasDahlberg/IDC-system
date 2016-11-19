#include <stdio.h>
#include <stdlib.h>

int sort_function( const void *a, const void *b);

#define MAX_CODES 5000
static struct _x {
  unsigned int noOf, code;
} codes[MAX_CODES];


unsigned short int swapWord(unsigned short int x)
{
  return (x >> 8) | ((x & 0xff) << 8);
}


main()
{
  FILE *fp;
  int n, i, j;
  unsigned short int buf[256];
  long bits1 = 0, bits2 = 0, fileBits = 0;
  int code, codeStuff = 0;
  int block;
  long tal, itemsMissed = 0;
  long sum9 = 0;
  long sum10 = 0;
  long sum11 = 0;
  long sum12 = 0;
  int iMax;

  block = 0;
  fp = fopen("duc11.exe", "rb");
  while (n = fread(buf, 2, 128, fp)) {
    printf("\rReading block %d", block++);
    for (j = 0; j < n; j++) {
/*
      tal = buf[j];
*/
      tal = swapWord(buf[j]);
      for (i = 0; i < MAX_CODES; i++) {
	if (codes[i].noOf == 0) {
	  codes[i].code = tal;
	  codes[i].noOf = 1;
	  break;
	}
	if (codes[i].code == tal) {
	  codes[i].noOf ++;
	  break;
	}
      }
      if (i < MAX_CODES) {
	;
      } else
	itemsMissed ++;
    } /* end for j next short */
  }
  fclose(fp);
  for (i = 0; i < MAX_CODES; i++) {
    if (codes[i].noOf == 0)
      break;
    printf("%3d:Code %04x used %d times\n", i, codes[i].code, codes[i].noOf);
  }
  printf("%d items missed\n", itemsMissed);

  iMax = i;
  qsort((void *) codes, iMax, sizeof(codes[0]), sort_function);

  for (i = 0; i < iMax; i++) {
    if (codes[i].noOf == 0)
      break;
    printf(
    "0x%04x, /*%3d:Code %04x used %d times*/\n",
    codes[i].code, i, codes[i].code, codes[i].noOf);

    if (i > 4412) {
      sum9 += codes[i].noOf;
    }
    if (i > 3900) {
      sum10 += codes[i].noOf;
    }
    if (i > 2876) {
      sum11 += codes[i].noOf;
    }
    if (i > 828) {
      sum12 += codes[i].noOf;
    }
  }
  printf("9bit  sum of 4413-4668 is %ld\n", sum9);
  printf("10bit sum of 3901-4668 is %ld\n", sum10);
  printf("11bit sum of 2876-4668 is %ld\n", sum11);
  printf("12bit sum of  828-4668 is %ld\n", sum12);
}


int sort_function( const void *a, const void *b)
{
   if ( ((struct _x *) a)->noOf == ((struct _x *) b)->noOf)
     return 0;
   if ( ((struct _x *) a)->noOf < ((struct _x *) b)->noOf)
     return -1;
   return 1;
}
