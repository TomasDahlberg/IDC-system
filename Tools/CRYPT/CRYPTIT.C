/* cryptIt.c  1992-02-19 TD,  version 1.0 */
/*
 * This file contains proprietary information of IVT Electronic AB.
 * Copying or reproduction without prior written approval is prohibited.
 *
 * This file is furnished under a license agreement or nondisclosure
 * agreement. The software may be used or copied only in accordance 
 * with the terms of the agreement.
 *
 * In no event will IVT Electronic AB, be liable for any lost revenue or
 * profits or other special, indirect and consequential damages, even if
 * IVT has been advised of the possibility of such damages.
 *
 * IVT Electronic AB
 * Box 996
 * 191 29 Sollentuna
 * Sweden
 */

/*
! cryptIt.c
! Copyright (C) 1992, IVT Electronic AB.
*/


/*
!   Usage:
!     cryptIt <seed> <string enclosed in quotes>
!
!   E.g.
!
!     cryptIt 17 "Copyright Tomas"
!
!   A C-source code is produced to stdout
!
!     History     
!     Date        Revision Who  What
!     
!     19-feb-1992   1.0    TD   Start of coding
!
*/
main(argc, argv)
int argc;
char *argv[];
{
  char prev = 0;
  char buf1[80], buf2[80];
  int i, seed;
  char *str;

  seed = atoi(argv[1]);
  str = argv[2];                                 
  printf("/*\n! The string \n");
  printf("'%s'\n", str);
  printf("! is encoded as;\n*/\n");
  encode(str, seed, buf1);
  dump(str, buf1);
  printf("/*\n! Use the following procedure to decode it;\n*/\n");
  emitUsage(seed);
}

emitUsage(seed)
int seed;
{
  printf("void printCopyright()\n");
  printf("{\n");
  printf("  int seed = %d;\n", seed);
  printf("  int len;\n");
  printf("  unsigned char *str;\n");
  printf("  str = copyrightNotice;\n");
  printf("  len = *str++;\n");
  printf("  for (;len; len--) {\n");
  printf("%s", "    printf(\"%c\", *str ^ seed);\n");
  printf("    seed = *str++;\n");
  printf("  }\n");
  printf("  printf(\"\\n\");\n");
  printf("}\n");
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

