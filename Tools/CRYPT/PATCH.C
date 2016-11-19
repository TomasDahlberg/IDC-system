#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//	'Licensnr 174711'
static unsigned char old[] = {
  0x0f, 0x5d, 0x34, 0x57, 0x32, 0x5c, 0x2f, 0x41, 0x33, 0x13, 0x22, 0x15,
  0x21, 0x16, 0x27, 0x16
};

char *printCopyright(unsigned char *str)
{
  int seed = 17;
  int len;
  static char buf[80];
  char *bf;
  bf = buf;
  len = *str++;
  for (;len; len--) {
    *bf++ = *str ^ seed;
    seed = *str++;
  }
  *bf = 0;
  return buf;
}

main(argc, argv)
int argc;
char *argv[];
{
  char prev = 0;
  char buf1[80], buf2[80];
  unsigned char buf[256];
  int i, seed;
  char str[50];
  FILE *fp;
  long fptr, nr;

  seed = 17;
//	'Licensnr 174711'
  nr = atol(argv[1]);
  if (nr < 0 || nr > 999999) {
    printf("illegal number\n");
    exit(1);
  }
  sprintf(str, "Licensnr %06ld", nr);
  encode(str, seed, buf1);
//  dump(str, buf1);
  printf("Patching file...");
  if ((fp = fopen("vp.exe", "rb")) == NULL) {
    printf("cannot open file ivt.exe\n");
    exit(1);
  }
  i = 0;
  while (fread(buf, 1, 1, fp)) {
    if (old[i] == buf[0]) {
      if (i == 0)
	fptr = ftell(fp);
      i++;
      if (i >= 10) 	// j„mf”r endast 'Licens '
	break;
    } else if (i) {
      fseek(fp, fptr, SEEK_SET);
      i = 0;
    }
  }
  fclose(fp);
  if (!i) {
    printf("could find, error\n");
    exit(1);
  }
  fptr --;
//  printf("Fptr = %ld\n", fptr);

  if ((fp = fopen("vp.exe", "r+b")) == NULL) {
    printf("cannot open file ivt.exe\n");
    exit(1);
  }
  fseek(fp, fptr, SEEK_SET);
  if (fread(buf, 1, 30, fp) != 30) {
    printf("Error reading file\n");
    exit(1);
  }
  printf("  %s -> ", printCopyright(buf));
  printf("%s\n", printCopyright(buf1));
  fseek(fp, fptr, SEEK_SET);
  if (fwrite(buf1, 1, buf1[0] + 1, fp) != buf1[0] + 1) {
    printf("Error patching file\n");
  }
/*
  for (i = 0; i < 20; i ++) {
    printf("%02x, ", buf[i]);
  }
*/
  fclose(fp);
}

dump(a, b)
unsigned char *a, *b;
{
  int i;
  printf("static unsigned char copyrightNotice[] = {\n  ");
  i = 0;
  while (*a++) {
    printf("0x%02x, ", *b++);
    if (i++ > 10) {
      printf("\n  ");
      i = 0;
    }
  }
  printf("0x%02x", *b++);
  printf("\n};\n");
}

encode(str, seed, out)
char *str, *out;
int seed;
{
  *out++ = strlen(str);
  for (; *str; str++)
    *out++ = seed = *str ^ seed;
}

