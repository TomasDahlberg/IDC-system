#include <stdio.h>
#include <stdlib.h>

int sort_function( const void *a, const void *b);

hex(char s)
{
  if ('0' <= s && s <= '9')
    return s - '0';
  return s - 'a' + 10;
}

long hex4(char *s)
{
  long a,b,c,d;
  a = hex(*s++);
  b = hex(*s++);
  c = hex(*s++);
  d = hex(*s++);
  return (a << 12) | (b << 8) | (c << 4) | d;
}

ishex(char s)
{
  return ('0' <= s && s <= '9') || ('a' <= s && s <= 'f');
}

ishex4(char *s)
{
/*
  return hex(*s++) + hex(*s++) + hex(*s++) + hex(*s++);
*/
  int a,b,c,d;
  a = ishex(*s++);
  b = ishex(*s++);
  c = ishex(*s++);
  d = ishex(*s++);
  return a && b && c && d;
}

#define MAX_CODES 2500
static struct _x {
  unsigned int noOf, code;
} codes[MAX_CODES];

main()
{
  FILE *fp;
  char buf[256];
  long tal, itemsMissed = 0;
  int i, iMax;

  fp = fopen("_main.lis", "r");
  while (fgets(buf, 256, fp)) {
    if (!ishex4(&buf[11]))
	continue;
    tal = hex4(&buf[11]);
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
  }
}


int sort_function( const void *a, const void *b)
{
   if ( ((struct _x *) a)->noOf == ((struct _x *) b)->noOf)
     return 0;
   if ( ((struct _x *) a)->noOf < ((struct _x *) b)->noOf)
     return -1;
   return 1;
}
