#include <stdio.h>
#include <dos.h>
#include <dir.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

struct {
  long 		ROFsync;
  short int 	typeLang,
		attrRev,
		assValid,
		series;
  char 		dateTime[6];
  short int	edition;
  long 		sizeStatic,
		sizeInit,
		sizeObject,
		sizeStack,
		offsetEntry,
		offsetTrapEntry,
		sizeRemoteStatic,
		sizeRemoteInit;
} buf;

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

void processFile(char *name, char *path)
{
  FILE *fp;

  if ((fp = fopen(name, "rb")) == NULL) {
    printf("Error opening '%s'\n", name);
    printf("errno = %d\n", errno);
    printf("_doserrno = %d\n", _doserrno);
    perror(name);
    return 0;
  }
  fread(&buf, sizeof(buf), 1, fp);
  fclose(fp);
  printf("Static size = %6lx, Init size = %6lx\n",
	swaplong(buf.sizeStatic), swaplong(buf.sizeInit));
}

int main(argc, argv)
int argc;
char **argv;
{
  struct ffblk ffblk;
  char path[80];
  int done;

  strcpy(path, argv[1]);

   done = findfirst(path,&ffblk,0);
   while (!done) {
      processFile(ffblk.ff_name, path);
      done = findnext(&ffblk);
   }
}