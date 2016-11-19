/* netload.c  1992-10-06 TD,  version 1.0 */
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
! netload.c
! Copyright (C) 1991, IVT Electronic AB.
*/


/*
!   netload - loads a memory module across network
! 
!   History:
!   Date        by   rev   what
!   ----------  ---  ----  ---------------------------------------------
!   1992-10-05  TD   1.00  initial coding
!
!   Function:
!   Distributes a memory module to the specified node
!
*/
@_sysedit: equ 1
@_sysattr: equ $8001

#include <stdio.h>

main(argc, argv)
int argc;
char *argv[];
{
  long node, address, size, n, err, buf;
  char *name, *headerPtr1;
  
  if (argc == 4) {
    node = atoi(argv[1]);
    name = argv[2];
    sscanf(argv[3], "%x", &address);
  } else {
    printf("Usage: netload <node> <name> <load-address>\n");
    exit(1);
  }

  initidcio();
  if (!linkDataModule(name, &headerPtr1)) {
    printf("Cannot link to datamodule '%s'\n", name);
    exit(1);
  }
  size = *((long *) (headerPtr1 + 4));
  buf = (long) headerPtr1;

  printf("Loading %d bytes\n", size);  
  for (n = 0; n < size; n += 100) {
    int bsize;
    bsize = size - n;
    if (bsize > 100)
      bsize = 100;
    if ((err = netPutMem(node, address, bsize, buf)) == 0) {
      printf("\r%d (%x, %d -> %x)", n, buf, bsize, address); fflush(stdout);
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
    buf += bsize;
  }
  printf("\n");
  unlinkDataModule(headerPtr1);
}

