/* reboot.c  1993-05-11 TD,  version 1.1 */
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
! reboot.c
! Copyright (C) 1991-1993, IVT Electronic AB.
*/


/*
!   reboot       - reboots local or remote node
! 
!   History:
!   Date        by   rev   what
!   ----------  ---  ----  ---------------------------------------------
!   1992-10-06  TD   1.00  initial coding
!   1993-05-11  TD   1.10  Added check if initidcio() ok
!
!   Function:
!   Reboots node 
!
*/
@_sysedit: equ 2
@_sysattr: equ $8001

#include <stdio.h>

main(argc, argv)
int argc;
char *argv[];
{
  char *headerPtr1;
  int i, node = 0;
  
  if (argc == 2) {
    node = atoi(argv[1]);
  }
  if (i = initidcio()) {
    if (i == 207)
      printf("Error 207, Memory Full\n");
    else if (i == 237)
      printf("Error 237, No RAM Available\n");
    else
      printf("Cannot link to trap module 'idcio', error %d\n", i);
  }
  
  if (node < 0) {
    int i;
    node = -node;
    node ++;
    for (i = 1; i < node; i++) {
      rebootNode(i);
    }
    exit(0);
  } else if (node > 0) {
    rebootNode(node);
    exit(0);
  }
  os9fork("localReboot", 0, 0, 0, 0, 0, 0);
}

rebootNode(node)
int node;
{
  int err;
  printf("Rebooting node %d...", node); fflush(stdout);
  if ((err = netRebootNode(node)) == 0) 
    printf("ok !\n");
  else {
    if (err == 8)
      printf("Timeout, node not responding\n");
    else if (err == -1)
      printf("server process not found\n");
    else if (err == 14)
      printf("no such node available\n");
    else
      printf("error %d\n", err);
  }
}


