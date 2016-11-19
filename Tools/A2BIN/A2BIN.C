#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*
static unsigned short swapword(w)
unsigned short w;
{
  return ((w & 0xff) << 8) | ((w >> 8) & 0xff);
}

static unsigned long swaplong(l)
unsigned long l;
{
  unsigned short w1, w2;
  w1 = (l >> 16) & 0xffff;
  w2 = l & 0xffff;
  return (((unsigned long) swapword(w2)) << 16) | swapword(w1);
}
  */

int hex(unsigned char a, unsigned char b)
{
   int c, d;
   if (a >= 'A')
	c = a - 'A' + 10;
   else
	c = a - '0';
   if (b >= 'A')
	d = b - 'A' + 10;
   else
	d = b - '0';
   return (c << 4) + d;
}

/*
void poke(char *inbuf, long base, long address, int i, int c)
{
  *(inbuf + (address - base) + i) = c;
}
*/

void main(argc, argv)
int argc;
char *argv[];
{
  FILE *fpIn, *fpOut;
  long base, address;
  int i, q, c;
  unsigned char buf[256];
//  unsigned char *inbuf;
  unsigned char outBuf[256];

//  inbuf = (char *) &netbuf;
  base = 0;

  if (argc != 3) {
	printf("Usage: a2bin <ascii-file> <bin-file>\n");
	printf("       ascii-file is captured from debug\n");
	printf("       and bin-file is created by us\n");
	exit(1);
  }
  if ((fpIn = fopen(argv[1], "r")) == 0) {
	printf("cannot open '%s'\n", argv[1]);
	exit(1);
  }
  if ((fpOut = fopen(argv[2], "wb")) == 0) {
	printf("cannot open '%s'\n", argv[2]);
	exit(1);
  }
/*
0038C5EA - 4AFC 0001 0000 0658 0000 0000 0000 064A   J|.....X.......J
*/
  while (fgets(buf, 255, fpIn)) {
    if (strlen(buf) < 12)
	continue;
    if (buf[9] != '-')
      continue;
    if (buf[8] != ' ')
      continue;
    if (buf[10] != ' ')
      continue;

    sscanf(buf, "%lx", &address);
    if (base == 0)
      base = address;

/*    if ((address - base) > sizeof(netbuf))
	break;
  */

//    printf("%s", buf);

    for (i = 0; i < 16; i++) {
	q = 11 + 5*(i/2);   if (i & 1) q += 2;
	if (buf[q] == ' ')
	  break;
	c = hex(buf[q], buf[q+1]);
	outBuf[i] =c;
/*	poke(inbuf, base, address, i, c); */
    }
    if (i != fwrite(outBuf, 1, i, fpOut)) {
      printf("Error writing binary file\n");
    }
  }
  fclose(fpOut);
  fclose(fpIn);
}
