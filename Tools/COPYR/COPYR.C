#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dos.h>

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
  int i, n;
  long int lb, siz, neededSpace;
  char buf[16384];
  char str[256], file[64], *filePtr;
  struct fcb blk;

  if (argc != 2) {
	printf("Usage: copyr <prom-file>\n");
	printf("       appends space and copyright notice at end of file\n");
	printf("       space is enough to fill up 512k\n");
	exit(1);
  }

  if ((fpIn = fopen(strcpy(file, argv[1]), "ab")) == 0) {
	printf("cannot open '%s'\n", argv[1]);
	exit(1);
  }
  siz = filesize(fpIn);
/*
!	and append space...
*/
  neededSpace = 524288L - siz - 48;
  printf("File is %ld bytes in size\n", siz);
  printf("Needed space is %ld bytes\n", neededSpace);
  printf("and total storage is %ld bytes.\n", 524288L);
  if (neededSpace < 0) {
    printf("Error two files larger than 512k!\n");
    exit(1);
  }
  for (n = 0; n < 512; n++)
    buf[n] = 255;

  while (neededSpace) {
    if (neededSpace > 512)
      n = 512;
    else
      n = neededSpace;
    if (n != fwrite(buf, 1, n, fpIn)) {
	printf("Error writing space to file, errno = %d\n", errno);
    }
    neededSpace -= n;
    lb += n;
  }
//  printf("After space has been appended file size is %ld bytes\n", lb);
/*
! 	and copy the second file.
*/
//  strcpy("Version 1.896  Copyright IVT Electronic AB, 1994");

/* put file name in fcb */
  filePtr = &file[strlen(file) - 8];
/*
  if (parsfnm(file, &blk, 2) == NULL)
     printf("Error in parsfm call\n");
*/
  strcat(strncat(strcpy(buf, "Version "), filePtr-1, 5),
	"  Copyright IVT Electronic AB, 1994");
  buf[8] = *filePtr;
  buf[9] = '.';

  printf("%s\n", buf);
  if (48 != fwrite(buf, 1, 48, fpIn)) {
	printf("Error writing file, errno = %d\n", errno);
  }
  lb += 48;
  fclose(fpIn);
//  printf("%ld bytes\n", lb);
#if 0
  if ((fpIn = fopen(argv[1], "rb")) == 0) {
	printf("cannot open '%s'\n", argv[1]);
	exit(1);
  }
  if ((fpOut = fopen(argv[3], "wb")) == 0) {
	printf("cannot create '%s'\n", argv[2]);
	exit(1);
  }
/*
!	Copy the first file...
*/
  lb = 0;
  printf("Copying file '%s'...", argv[1]); fflush(stdout);
  while (n = fread(buf, 1, 16384, fpIn)) {
    if (n != fwrite(buf, 1, n, fpOut)) {
	printf("Error writing file, errno = %d\n", errno);
    }
    lb += n;
  }
  fclose(fpIn);
  printf("%ld bytes\n", lb);
/*
!	and open second file to calculate file size...
*/
  if ((fpIn = fopen(argv[2], "rb")) == 0) {
	printf("cannot open '%s'\n", argv[2]);
	exit(1);
  }
  siz = filesize(fpIn);
/*
!	and append space...
*/
  neededSpace = 524288L - siz - lb;
  printf("Second file is %d bytes in size\n", siz);
  printf("Needed space is %ld bytes\n", neededSpace);
  printf("since first file occupied %ld bytes\n", lb);
  printf("and total storage is %ld bytes.\n", 524288L);
  if (neededSpace < 0) {
    printf("Error two files larger than 512k!\n");
    exit(1);
  }
  for (n = 0; n < 512; n++)
    buf[n] = 255;

  while (neededSpace) {
    if (neededSpace > 512)
      n = 512;
    else
      n = neededSpace;
    if (n != fwrite(buf, 1, n, fpOut)) {
	printf("Error writing space to file, errno = %d\n", errno);
    }
    neededSpace -= n;
    lb += n;
  }
  printf("After space has been appended file size is %ld bytes\n", lb);
/*
! 	and copy the second file.
*/
  printf("Copying file '%s'...", argv[2]); fflush(stdout);
  while (n = fread(buf, 1, 512, fpIn)) {
    if (n != fwrite(buf, 1, n, fpOut)) {
	printf("Error writing file, errno = %d\n", errno);
    }
    lb += n;
  }
  fclose(fpIn);
  fclose(fpOut);
  printf("%ld bytes\n", lb);
#endif
}

