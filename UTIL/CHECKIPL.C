/* checkIpl.c  1992-11-16 TD,  version 1.1 */
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
! checkIpl.c
! Copyright (C) 1992 IVT Electronic AB.
*/

/*   history:
*   date       by  rev  ed#  what
*   ---------- --- ---  ---  ------------------------------------
*   1992-09-11 td  1.00   1  initial coding
*   1992-11-16 td  1.10   1  if scan exists check for screen
*
*/
@_sysedit: equ 1
@_sysattr: equ $8001

#include <procid.h>
#include <setsys.h>
/*
#include <ctype.h>
*/

/* procid buff; */
char buff[100];

/*
!   Purpose:  Parses the ipl-string for programs and checks 
!             if they are running
!
!   Added:  Every 60 second, check ipl-string
!           Every 3times a second, update hardware watchdog strobe
!                 light the red/green led depending on battery voltage
*/

char busy[21];
static short int *bvPtr = 0x003ffd4;
unsigned char *qsop2= 0x34001e;       /* set bit (low) in output port 2 */
unsigned char *qrop2= 0x34001f;       /* reset bit (high) in output port 2 */
#define BATTLED 0x08       /* in op2 */

int main(argc, argv)
int argc;
char *argv[];
{
  char *ipl = (char *) 0x442;
  static unsigned char *wdst = 0x00348000;
  int cnt = 0, bCnt = 0, bLevel = 0;
  float bv;
  
  setpr( getpid(), (short) 900);
  initHardware_WD();
  while (1) {
    *wdst = 0;
    tsleep(30);             /* sleep 0.3 seconds */
    if (bCnt > bLevel) {
      *qrop2 = BATTLED;       /* red */
    }
    if (bCnt > 30)        /* 10 seconds */ 
    {
      bv = (*bvPtr);
      bv = (bv - 3) / 0.8;
      if (bv < 0)
        bv = 0;
      else if (bv > 1)
        bv = 1;

      bLevel = 28 * bv + 1;      /* 1 - 29 */
      bCnt = 0;
      *qsop2 = BATTLED;       /* green */
    } 
    bCnt ++;
    if (cnt > 180) {
      cnt = 0;
      if (argc == 2) {
        check(argv[1]);
      } else {
        if ( *((short int *) 0x440) != (short int) 0xc0de )
          continue;
        check(ipl);
      }
    }
    cnt ++;
  }
/*    
  while (sleep(60) || 1) {
    if (argc == 2) {
      check(argv[1]);
    } else {
      if ( *((short int *) 0x440) != (short int) 0xc0de )
        continue;
      check(ipl);
    }
  }
*/  
}

check(ipl)
char *ipl;
{
  char *namePtr;
  char name[128];
  int i;

  for (i = 0; i < 21; i++)
    busy[i] = 0;  
  while (*ipl != 0x0d && *ipl) {
    while (*ipl == ' ')
      ipl ++;
    if (*ipl == 0x0d || *ipl == 0) continue;
    namePtr = name;
    while (*ipl != 0x0d && *ipl && *ipl != ' ' && *ipl != ';' && *ipl != '&')
      *namePtr++ = *ipl++;
    *namePtr = 0;
/*    if (*ipl == 0x0d || *ipl == 0) continue;  */
    while (*ipl != 0x0d && *ipl && *ipl != ';' && *ipl != '&')    /* skip args */
      ipl ++;
    if (*ipl == ';')
      ; /* ex 'verify;' printf("no need to check '%s'\n", name); */
    else if (*ipl == '&') {
        /* printf("check '%s'\n", name); */
      checkName(name);
      if (!jmfrnamn(name, "scan")) {       /* added 921116 */
        checkName("screen");
      }
    } else {
      /* ex prog x in 'verify;q&k&p;qr;x' */
    }
    
    if (*ipl == 0x0d || *ipl == 0) continue;
    ipl ++;
  }
  return 1;  
}

char *pid2name(pid)
int pid;
{
  char *c;

  if (_get_process_desc(pid, sizeof(buff), buff) == -1)
    return 0;
    
  c = (char *) (((procid *) buff)->_pmodul);
  return c + *((long *) (c + 12));
/*
  if (_get_process_desc(pid, sizeof(buff), &buff) == -1)
    return 0;
  return ((char *) buff._pmodul) + buff._pmodul->_mh._mname;
*/
}

checkName(n)
char *n;
{
  int pid, ok;

  pid = ok = 0;
  while (1) {
    char *name;
    pid ++;
    if (pid > 20)
      break;
    if ((name = pid2name(pid)) == 0)
      continue;
/*    printf("%3d : '%s'\n", pid, name);    */
    if (busy[pid])
      continue;
    if (!jmfrnamn(n, name)) {
      ok = 1;
      busy[pid] = 1;
      break;
    }
  }
  if (ok == 0) {
/*    printf("'%s' finns inte !\n", n);   */
    reboot();
  }
}

int isalpha(c)
char c;
{
  if (c < 'A')
    return 0;
  if (c > 'z')
    return 0;
  return 1;
}

int jmfrnamn(n, name)
char *n, *name;
{
  while (*n && *name) {
    if (isalpha(*n) && isalpha(*name)) {
      if ((*n | 0x20) != (*name | 0x20))
        break;
    } else if (*n != *name)
      break;
    n++;  name++;
  }
  if (*n || *name)
    return 1;
  return 0;
}

reboot()
{
  os9fork("reboot", 0, 0, 0, 0, 0, 0);
}
/*
reboot:  movea.l  #0,a0
         movea.l  0(a0),a7
         movea.l  4(a0),a1
         jmp      (a1)
*/

unsigned char *rtc  = 0x00310000;
unsigned char *wdst = 0x00348000;

initHardware_WD()
{
  *wdst = 0;
  while ((*rtc & 0x0f) != 0)
    ;
#asm
  movea.l rtc(a6),a0
  move.b  #15,(a0)
  move.b  #6,(a0)
  move.b  #1,(a0)
#endasm
}
