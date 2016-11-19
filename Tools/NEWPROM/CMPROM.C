#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int parse(char *buf, long *addr, long *siz, long *rev,
			long *ed, long *crc, char *tkn, char *name)
{
  char n[32];
  if (strncmp(buf, "00", 2))
    return 0;
  sscanf(buf, "%08lx %8ld %3ld %3ld %06lx %s\n",
	addr, siz, rev, ed, crc, n);
  if (n[0] == '-' || n[0] == '=' || n[0] == '+' || n[0] == ' ') {
    *tkn = n[0];
    strcpy(name, &n[1]);
  } else {
    *tkn = ' ';
    strcpy(name, n);
  }
  return 1;
}

void main(int noOfArgs, char **theArgs)
{
  FILE *fp1, *fp2;
  int more, more1, prom2;
  char buf1[256], buf2[256];
  long addr1, addr2, siz1, siz2, rev1, rev2, ed1, ed2, crc1, crc2;
  char tkn1, tkn2, name1[32], name2[32];

  if (noOfArgs != 3) {
    printf("Usage: cmprom <new-list> <old-list>\n");
    exit(1);
  }

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
    if (!strncmp(buf1, "================================", 32))
      break;
  }
  while (fgets(buf1, 255, fp2)) {
    if (!strncmp(buf1, "================================", 32))
      break;
  }
  printf("New prom id file is '%s'\n", theArgs[1]);
  printf("Old prom id file is '%s'\n", theArgs[2]);
  printf("\n\n");
/*
! 	read
*/
  more = 1;
  more1 = 1;
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
	printf("%s <-> %s\n", name1, name2);
      } else {
	if (crc1 != crc2) {
	  if (more1) {
	    more1 = 0;
	    printf("            New PROM                                 Old PROM\n");
	    printf("Size     Rev Ed# CRC    Module name      Size     Rev Ed# CRC    Module name\n");
	    printf("====================================     ====================================\n");
	  }
	  printf("%8ld %3ld %3ld %06lx %-12s <-> %8ld %3ld %3ld %06lx %-12s\n",
		siz1, rev1, ed1, crc1, name1,
		siz2, rev2, ed2, crc2, name2);
	}
      }
    } else {
      if (more) {
	printf("\nNew modules added:\n\n");
	printf("            New PROM               \n");
	printf("Size     Rev Ed# CRC    Module name\n");
	printf("====================================\n");
	more = 0;
      }
      printf("%8ld %3ld %3ld %06lx %-12s\n", siz1, rev1, ed1, crc1, name1);
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
      printf("\nOld prom contains additional modules:\n\n");
      printf("            Old PROM               \n");
      printf("Size     Rev Ed# CRC    Module name\n");
      printf("====================================\n");
      more = 0;
    }
    printf("%8ld %3ld %3ld %06lx %-12s\n", siz1, rev1, ed1, crc1, name1);
  }
  fclose(fp2);
  fclose(fp1);
}
