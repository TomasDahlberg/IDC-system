/* new.c  1992-10-06 TD,  version 1.1 */
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
! new.c
! Copyright (C) 1992, IVT Electronic AB.
*/


/*
!   new
! 
!   History:
!   Date        by   rev   what
!   ----------  ---  ----  ---------------------------------------------
!   1992-09-24  MS   1.00  initial coding
!   1992-10-06  TD   1.10  added net support
!
!   Function:
!   Kills scan, screen & main. Clears the sync byte of these modules and
!   metavar, then issues the clear command.
!
*/
@_sysedit: equ 2
@_sysattr: equ $8001

#include <stdio.h>
#include <sgstat.h>
#include <setsys.h>
#include <procid.h>
#include <signal.h>


#asm
getDsc:
    lea      buff(a6),a0
    OS9       F$GPrDsc
    move.l    d1,d0
    rts
#endasm

procid buff;

char * pid2Name(pid)
int pid;
{
  if( getDsc(pid,sizeof(buff)) == 224)
    return 0;
  return ((char *) buff._pmodul) + buff._pmodul->_mh._mname;
}

int delModule(c)
char *c;
{
 mod_exec *i;
  
 i = (mod_exec *) modlink(c, (short) 0);
 if( i != (mod_exec *) -1)
   i->_mh._msync = 0; 
} 

char scan[]     = "scan";
char screen[]   = "screen";
char Main[]     = "main";
char checkIpl[] = "checkIpl";
char METAVAR[]  = "METAVAR";

main(argc, argv)
int argc;
char *argv[];
{
  int pid;
  int i, node = 0;
  
  if (argc == 2) {
    node = atoi(argv[1]);
  }
  initidcio();
  
  if (node < 0) {
    int i;
    node = -node;
    node ++;
    for (i = 1; i < node; i++) {
      newAtNode(i);
    }
    exit(0);
  } else if (node > 0) {
    newAtNode(node);
    exit(0);
  }

  system("pulse");
  for(pid = 1; pid < 20; pid++) {
    if( strcmp(checkIpl,pid2Name(pid)) == 0 )
      kill(pid,0);
  }
  for(pid = 1; pid < 20; pid++) {
    if( strcmp(scan,pid2Name(pid)) == 0 )
      kill(pid,0);
    if( strcmp(Main,pid2Name(pid)) == 0 )
      kill(pid,0);
    if( strcmp(screen,pid2Name(pid)) == 0 )
      kill(pid,0);
  }
  delModule(scan);
  delModule(screen);
  delModule(Main);
  delModule(METAVAR);
  system("clear");
}

newAtNode(node)
int node;
{
  int err;
  printf("New at node %d...", node); fflush(stdout);
  if ((err = netNew(node)) == 0) 
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

