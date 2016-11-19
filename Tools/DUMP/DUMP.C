#include <stdio.h>
#include <dir.h>
#include <errno.h>
#include <math.h>
#include <ctype.h>


main(int argc, char *argv[])
{
   struct ffblk ffblk1;
   int done;

   if (done = findfirst(argv[1],&ffblk1,0)) {
     printf("Sorry, no such file as '%s'\n", argv[1]);
     exit(1);
   }
   while (!done) {
     dumpFile(ffblk1.ff_name);
     done = findnext(&ffblk1);
   }
   return ffblk1.ff_fsize;
}

dumpFile(char *fileName)
{
  FILE *fp;
  char buff[256];
  int noOfItems;

  if ((fp = fopen(fileName, "rb")) <= 0) {
    printf("Error opening '%s'\n", fileName);
    exit(1);
  }
  do {
    noOfItems = fread(buff, 1, 16, fp);
    dump(buff, noOfItems);
  } while (noOfItems > 0);
  fclose(fp);
}


dump(char *s, int len)
{
 int i, j;
 char *d;

   d = s;
   for (i = 0; i < len; i++)
     printf("%02x ", (unsigned char) *s++);
   printf("  ");
   for (i = 0; i < len; i++, d++)
     printf("%c", isprint(*d) ? *d : '.');
   printf("\n");
}


