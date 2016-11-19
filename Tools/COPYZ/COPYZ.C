#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
  FILE *fpZ, *fpIn, *fpOut;
  int i, n;
  long int lb;
  char buf[512];
  char str[256];

  if (argc != 3) {
	printf("Usage: copyz -z=txt-file <out-bin-file>\n");
	printf("       txt-file is a directory listing which\n");
	printf("       contains files to copy\n");
	printf("       out-bin-file is the output file\n");
	exit(1);
  }

  if ((fpZ = fopen(&argv[1][3], "r")) == 0) {
	printf("cannot open '%s'\n", &argv[1][3]);
	exit(1);
  }
  if ((fpOut = fopen(argv[2], "wb")) == 0) {
	printf("cannot create '%s'\n", argv[2]);
	exit(1);
  }
  lb = 0;
  while (fgets(str, 256, fpZ)) {
    for (i = 0; i < 40; i++) {
      if (str[i] == ' ')
	break;
    }
    str[i] = 0;
    if ((fpIn = fopen(str, "rb")) == 0) {
	printf("cannot open '%s'\n", str);
	exit(1);
    }
    printf("Adding module '%s'...", str); fflush(stdout);
    while (n = fread(buf, 1, 512, fpIn)) {
      if (n != fwrite(buf, 1, n, fpOut)) {
	printf("Error writing file, errno = %d\n", errno);
      }
      lb += n;
    }
    printf("%ld bytes\n", lb);
    fclose(fpIn);
  }
  fclose(fpOut);
  fclose(fpZ);
}

