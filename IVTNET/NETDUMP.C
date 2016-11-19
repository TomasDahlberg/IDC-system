/* netdump.c  1992-10-06 TD,  version 1.0 */
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
! netdump.c
! Copyright (C) 1991, IVT Electronic AB.
*/


/*
!   netdump - dumps memory from remote node
! 
!   History:
!   Date        by   rev   what
!   ----------  ---  ----  ---------------------------------------------
!   1992-10-05  TD   1.00  initial coding
!
!   Function:
!   Gets memory blocks from remote module
!
*/
@_sysedit: equ 1
@_sysattr: equ $8001

#include <stdio.h>
#include <ctype.h>

main(argc, argv)
int argc;
char *argv[];
{
  long node, address, size, n, err, addressBase;
  unsigned char buf[256];
  
  if (argc == 3 || argc == 4) {
    node = atoi(argv[1]);
    sscanf(argv[2], "%x", &address);
    if (argc == 4)
      size = atoi(argv[3]);
    else
      size = 128;
  } else {
    printf("Usage: netdump <node> <load-address> [<size>]\n");
    printf("\n");
    printf("       address in hex, size in decimal\n");
    exit(1);
  }
    
  initidcio();
  addressBase = address;
  for (n = 0; n < size; n += 128) {
    int bsize;
    bsize = size - n;
    if (bsize > 128)
      bsize = 128;
    if ((err = netGetMem(node, address, bsize, buf)) == 0) {
      printf("%d (%x, %d)\n", n, address, bsize); fflush(stdout);
      dump(buf, bsize, address - addressBase);
    } else {
      if (err == 8)
        printf("Timeout, node not responding\n");
      else if (err == -1)
        printf("server process not found\n");
      else if (err == 14)
        printf("no such node available\n");
      else
        printf("error %d\n", err);
      break;
    }
    address += bsize;
  }
  printf("\n");
}

dump(s, l, offset)
unsigned char *s;
int l, offset;
{
  int i, p, p2;
  
  printf("Addr   0 1  2 3  4 5  6 7  8 9  A B  C D  E F 0 2 4 6 8 A C E\n");
  printf("----  ---- ---- ---- ---- ---- ---- ---- ---- ----------------\n");
  for (p = 0; p < l; ) {
    printf("%04x  ", p + offset);
    p2 = p;
    for (i = 0; i < 8; i++) {
      if (p2 < l)
        printf("%02x", s[p2++]);
      else 
        printf("  ");
      if (p2 < l)
        printf("%02x ", s[p2++]);
      else
        printf("   ");
    }
    for (i = 0; i < 16 && p < l; i++, p++)
      printf("%c", (s[p] < 128 && isprint(s[p])) ? s[p] : '.');
    printf("\n");
  }
  printf("\n");
/*  
0000  dead face 0000 0000 0000 0009 5c0a 0609 ^-zN........\...
*/
}
