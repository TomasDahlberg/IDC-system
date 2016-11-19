#include <stdio.h>

int parse(char *buf, long *addr, long *siz, long *rev,
			long *ed, long *crc, char *tkn, char *name)
{
  if (strncmp(buf, "00", 2))
    return 0;
  sscanf(buf, "%08x %8d %3d %3d %06x %c%s\n"
	addr, siz, rev, ed, crc, tkn, name);
  return 1;
}

void main(int noOfArgs, char **theArgs)
{
  FILE *fp1, *fp2;
  int more, prom2;
  char buf1[256], buf2[256];
  long addr1, addr2, siz1, siz2, rev1, rev2, ed1, ed2, crc1, crc2;
  char tkn1, tkn2, name1[32], name2[32];

  if ((fp1 = fopen(theArgs[1], "r")) == NULL) {
    printf("Sorry, couldn't open '%s'\n", theArgs[1]);
    exit(1);
  }
  if ((fp2 = fopen(theArgs[2], "r")) == NULL) {
    printf("Sorry, couldn't open '%s'\n", theArgs[2]);
    exit(1);
  }
/*
!      skip header in file1 and file2
*/
  while (fgets(buf1, 255, fp1)) {
    if (!strcmp(buf1, "================================"))
      break;
  }
  while (fgets(buf1, 255, fp2)) {
    if (!strcmp(buf1, "================================"))
      break;
  }
/*
! 	read
*/
  prom2 = 1;
  while (fgets(buf1, 255, fp1)) {
    if (!parse(buf1, &addr1, &siz1, &rev1, &ed1, &crc1, &tkn1, name1))
      break;
    if (prom2 && !fgets(buf2, 255, fp2))
      prom2 = 0;

    if (prom2 &&
		!parse(buf2, &addr2, &siz2, &rev2, &ed2, &crc2, &tkn2, name2))
      prom2 = 0;

    if (prom2) {
      if (strcmp(name1, name2)) {
	printf("oh no!\n");
      } else {
	if (crc1 != crc2) {
	  printf("%06x %s <-> %06x %s\n", crc1, name1, crc2, name2);
	}
      }
    } else {
      if (more) {
	printf("New modules added:\n");
	more = 0;
      }
      printf("%s\n", name1);


    }
  }
/*
!   if any left in buf2, then report these
*/
  more = 1;
  while (fgets(buf1, 255, fp2)) {
    if (!parse(buf1, &addr1, &siz1, &rev1, &ed1, &crc1, &tkn1, name1))
      break;
    if (more) {
      printf("Old prom contains additional modules:\n");
      more = 0;
    }
    printf("%s\n", name1);

  }
  fclose(fp2);
  fclose(fp1);
}
