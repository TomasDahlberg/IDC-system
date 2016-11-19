#include <stdio.h>
/*
! The string 
'Copyright 1992, IVT Electronic AB'
! is encoded as;
*/
static unsigned char copyrightNotice[] = {
  33, 10, 101, 21, 108, 30, 119, 16, 120, 12, 44, 29, 
  36, 29, 47, 3, 35, 106, 60, 104, 72, 13, 97, 4, 
  103, 19, 97, 14, 96, 9, 106, 74, 11, 73
};
/*
! Use the following procedure to decode it;
*/
void printCopyright()
{
  int seed = 73;
  int len;
  unsigned char *str;
  str = copyrightNotice;
  len = *str++;
  for (;len; len--) {
    fprintf(stderr, "%c", *str ^ seed);
    seed = *str++;
  }
  fprintf(stderr, "\n");
}
