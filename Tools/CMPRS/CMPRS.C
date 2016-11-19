#include <stdio.h>

unsigned short int opcodeArray[256] =
{
  0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007, 
  0x0008, 0x0009, 0x000a, 0x000b, 0x000c, 0x000d, 0x000e, 0x000f, 
  0x0010, 0x0012, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017, 0x0018, 
  0x0019, 0x001c, 0x001e, 0x001f, 0x0020, 0x0021, 0x0024, 0x002c, 
  0x0030, 0x0040, 0x0044, 0x0047, 0x004c, 0x0050, 0x0053, 0x0054, 
  0x0056, 0x0058, 0x006c, 0x0072, 0x00a2, 0x00a8, 0x0100, 0x0102, 
  0x0476, 0x05ea, 0x05ee, 0x066e, 0x0800, 0x0c00, 0x0d00, 0x0f43, 
  0x2008, 0x2017, 0x2020, 0x2025, 0x2028, 0x202e, 0x202f, 0x203a, 
  0x203c, 0x203d, 0x2040, 0x2041, 0x2047, 0x2054, 0x2057, 0x2069, 
  0x206d, 0x206e, 0x206f, 0x2073, 0x2074, 0x2140, 0x217c, 0x2200, 
  0x2208, 0x222e, 0x226e, 0x2348, 0x2368, 0x243c, 0x2533, 0x2a2a, 
  0x2d40, 0x2e31, 0x2f00, 0x2f28, 0x2f2f, 0x3020, 0x305f, 0x3066, 
  0x3100, 0x3120, 0x3131, 0x315f, 0x3166, 0x3200, 0x325f, 0x332e, 
  0x342e, 0x352e, 0x3530, 0x3532, 0x3635, 0x3a20, 0x3d20, 0x3d25, 
  0x4175, 0x41ee, 0x41ef, 0x41fa, 0x4228, 0x4236, 0x42a7, 0x42a8, 
  0x42ae, 0x4631, 0x4754, 0x486e, 0x4878, 0x4880, 0x48c0, 0x48d7, 
  0x48e7, 0x48e8, 0x4a28, 0x4a80, 0x4aa8, 0x4aae, 0x4c42, 0x4cd7, 
  0x4cdf, 0x4ce8, 0x4ce9, 0x4ced, 0x4d61, 0x4e40, 0x4e45, 0x4e46, 
  0x4e4d, 0x4e4f, 0x4e55, 0x4e5d, 0x4e75, 0x4fef, 0x508f, 0x518f, 
  0x5265, 0x5280, 0x5356, 0x5380, 0x5435, 0x5469, 0x562e, 0x5635, 
  0x5649, 0x588f, 0x5f31, 0x5f36, 0x6000, 0x6022, 0x6080, 0x6100, 
  0x616c, 0x616e, 0x6172, 0x6174, 0x6420, 0x6465, 0x6500, 0x6520, 
  0x6532, 0x6567, 0x656c, 0x656d, 0x6572, 0x6574, 0x6600, 0x6604, 
  0x6616, 0x6620, 0x6626, 0x6665, 0x6674, 0x6700, 0x6708, 0x670c, 
  0x671c, 0x6720, 0x673a, 0x676c, 0x6772, 0x6875, 0x6964, 0x6966, 
  0x696c, 0x696e, 0x6c00, 0x6c20, 0x6c61, 0x6c65, 0x6c6c, 0x6d69, 
  0x6d70, 0x6e67, 0x6e69, 0x6e73, 0x6e74, 0x6e75, 0x7000, 0x7001, 
  0x7002, 0x7200, 0x7201, 0x7206, 0x7220, 0x723c, 0x7261, 0x7264, 
  0x7265, 0x7269, 0x726d, 0x7270, 0x7275, 0x7365, 0x7374, 0x7400, 
  0x7420, 0x7465, 0x7469, 0x746f, 0x756c, 0x7574, 0x7600, 0x7669, 
  0x801e, 0x802a, 0x802e, 0x8080, 0x8620, 0x8fee, 0xb0a8, 0xc000, 
  0xc080, 0xff9c, 0xffa4, 0xffb0, 0xffbc, 0xfff8, 0xfffc, 0xffff 
};

#include <stdlib.h>

int sort_function( const void *a, const void *b);

int sort_function( const void *a, const void *b)
{
  if (*((unsigned short int *) a) > *((unsigned short int *) b))
    return 1;

  if (*((unsigned short int *) a) < *((unsigned short int *) b))
    return -1;
  return 0;
}


int sort_dictionary()
{
  return 0;
/*
  qsort((void *) opcodeArray, 256, sizeof(opcodeArray[0]), sort_function);
*/
}

int quick_lookup(unsigned short int op)
{
  int i, a, b, c;
  unsigned short int *opPtr;
  opPtr = opcodeArray;

  a = 0;
  b = 255;
  while (1) {
    c = (a + b) / 2;
    if (!(i = (opcodeArray[c] - op)))
      return 256 + c;
    if (a == b)
      return 0;
    if (i < 0) {
      if (a == c)
	return 0;
      a = c;
    }
    else {
      if (b == c)
	return 0;
      b = c;
    }
  }
  return 0;
}

int lookup(unsigned short int op)
{
  int i;
  unsigned short int *opPtr;
  opPtr = opcodeArray;

  for (i = 0; i < 256; i++, opPtr++)
    if (op == *opPtr)
      return 256 + i;
  return 0;
}

unsigned short int swapWord(unsigned short int x)
{
  return (x >> 8) | ((x & 0xff) << 8);
}

main(int argc, char *argv[])
{
  FILE *fp1, *fp2;
  int n, i;
  unsigned short int buf[256];
  long bits1 = 0, bits2 = 0, fileBits = 0;
  int code, codeStuff = 0;
  long t1, t2;
  long saved;
  int elapsed;

  if (argc != 3) {
    printf("usage: decmprs in-file out-file\n");
    exit(1);
  }
  fp1 = fopen(argv[1], "rb");
  fp2 = fopen(argv[2], "wb");
  if (fp1 == 0 || fp2 == 0) {
    printf("Error\n");
    exit(1);
  }
  sort_dictionary();
  t1 = time(0);
  while (n = fread(buf, 2, 128, fp1)) {
    for (i = 0; i < n; i++) {
      fileBits += 16;

/* if pc, swap word */

	buf[i] = swapWord(buf[i]);

      if (code = quick_lookup(buf[i])) {
	bits1 += 9;
	codeStuff++;
	emit9bit(fp2, code);
      } else {
	bits1 += 18;
	emit9bit(fp2, buf[i] >> 8);
	emit9bit(fp2, buf[i] & 0xff);
      }
    } /* for */
  }  /* while */
  t2 = time(0);
  flush9bitOutput(fp2);
  printf("File size  %ld\n", fileBits >> 3);
  printf("Compressed %ld\n", bits1 >> 3);
  printf("Ratio %d%%\n", bits1 * 100 / fileBits);
  printf("Code stuff is %d\n", codeStuff);
  printf("Elapsed time %d seconds\n", elapsed = (int) (t2 - t1));
  saved = (fileBits >> 3) - (bits1 >> 3);
  printf("Saved %ld bytes\n", saved);
  printf("Needs a transfer rate less than %d bps !!\n",
	(int) (saved * 10 / elapsed));
}

/*
!   routines to emit 9 bits numbers into stream, and to flush stream
*/
static unsigned char outBuf[256];
static int outBytePtr,
		outBitPtr = 1;
static unsigned char outNofull;

int flush9bitOutput(FILE *fp2)
{
  if (outBytePtr || (outBitPtr != 1)) {
    outBuf[outBytePtr++] = outNofull;
    fwrite(outBuf, 1, outBytePtr, fp2);
    outBytePtr = 0;
    outBitPtr = 1;
  }
}

int emit9bit(FILE *fp2, int tal)
{
  unsigned char rest;

  outNofull |= tal >> outBitPtr;
  rest = tal & ((1 << outBitPtr) - 1);
  outBuf[outBytePtr++] = outNofull;
  outNofull = rest << 8 - outBitPtr;
  outBitPtr ++;
  if (outBitPtr > 8) {
    outBitPtr = 1;
    if (outBytePtr > 255) {
      fwrite(outBuf, 1, 256, fp2);
      outBytePtr = 0;
    }
    outBuf[outBytePtr++] = outNofull;
    outNofull = 0;
  }
  if (outBytePtr > 255) {
    fwrite(outBuf, 1, 256, fp2);
    outBytePtr = 0;
  }
}
