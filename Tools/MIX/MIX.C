#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void padString(char *s, int l)
{
  int q;
  char *p;
  s[l] = 0;
  p = strchr(s, 0);
  q = (p - s);
  for (; q < l; q++)
    s[q] = ' ';
}

main(int argc, char *argv[])
{
  FILE *fp1, *fp2;
  char buf1[256], buf2[256];
  int fil1 = 1, fil2 = 1;

  if (argc != 3) {
    printf("Usage: mix <file1> <file2>\n");
    exit(1);
  }

  if (!(fp1 = fopen(argv[1], "r"))) {
	printf("cannot open %s\n", argv[1]);
	exit(1);
  }
  if (!(fp2 = fopen(argv[2], "r"))) {
	printf("cannot open %s\n", argv[2]);
	exit(1);
  }
  while (fil1 || fil2) {
    if (fil1) {
      fil1 = (fgets(buf1, 255, fp1) != NULL);
    }
    buf1[strlen(buf1) - 1] = 0;
    buf1[25] = 0;
    printf("%s\n", buf1);

    if (!strncmp(buf1, "ACK", 3))
	continue;

    if (fil2) {
	fil2 = (fgets(buf2, 255, fp2) != NULL);
    } else
	buf2[0] = 0;

    buf2[strlen(buf2) - 1] = 0;
    buf2[65] = 0;
    printf("                         %s\n", buf2);
  }

}