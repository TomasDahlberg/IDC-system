/* showipl.c  1992-10-13 TD,  version 1.1 */
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
! showipl.c
! Copyright (C) 1992, IVT Electronic AB.
*/


/*
!   Showipl - shows the start string for this or any node
! 
!   History:
!   Date        by   rev   what
!   ----------  ---  ----  ---------------------------------------------
!   1992-04-08  TD   1.00  initial coding, in assembler, file: showipl.org
!   1992-10-13  TD   1.10  rewrote in c and added node support
!
!   Function:
!   Shows the IPL-string for local or remote node
!
*/
@_sysedit: equ 2
@_sysattr: equ $8001

#include <stdio.h>
#include <ctype.h>

main(argc, argv)
int argc;
char *argv[];
{
  long node;
  
  if (argc > 2) {
    printf("Usage: showipl [node]\n");
    exit(1);
  }
  if (argc == 2)
    node = atoi(argv[1]);
  else
    node = 0;
    
  initidcio();
  if (node == 0) 
    showIplString(0x440);
  else {
    if (node < 0) {
      int i;
      node = -node;
      node ++;
      for (i = 1; i < node; i++) {
        doIplString(i);
      }
      exit(0);
    } else if (node > 0) {
      doIplString(node);
      exit(0);
    }
  }
}

doIplString(node)
int node;
{
  long err;
  unsigned char buf[256];

  printf("Receiving ipl string from node %d...", node); fflush(stdout);
  if ((err = netGetMem(node, 0x440, 200, buf)) == 0) {
    printf(" [ok]\n");
    showIplString(buf);
  } else {
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

showIplString(b)
char *b;
{
  if (*((unsigned short int *) b) != (unsigned short int) 0xc0de) {
    printf("no-ipl-string!\n");
    return 0;
  }
  b += 2;
  do {
    write(1, b, 1);
  } while (*b++ != 0x0d);
}

/*
* Shows text from address 0x442 if 0xcode is at address 0x440
*
type     set      (1<<8)+1 
revs     set      (128<<8)+1
         psect    v,type,revs,1,256,main

main:    movea.l  #$0440,a0
         cmpi.w   #$c0de,(a0)+
         beq.s    next
         lea.l    str(pc),a0
next     moveq    #1,d1
         moveq    #1,d0
         os9      I$Write
         cmpi.b   #13,(a0)+
         bne.s    next
         moveq    #0,d1             no error
         os9      F$Exit

str      dc.b     "no-ipl-string",13
*/
