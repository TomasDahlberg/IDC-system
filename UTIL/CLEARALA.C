/* clearalarm.c  1991-08-03 TD,  version 1.1 */
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
! clearalarm.c
! Copyright (C) 1991, IVT Electronic AB.
*/


/*
!   clearalarm       - clears all alarms from the alarm stack
! 
!   History:
!   Date        by   rev   what
!   ----------  ---  ----  ---------------------------------------------
!   1991-02-19  TD   1.00  initial coding
!   1991-05-07  TD   1.01  re-compiled code to fit new alarm routines
!
!   Function:
!   The alarm stack will be cleared from all entries
!
*/
@_sysedit: equ 2
@_sysattr: equ $8001

#include <stdio.h>
#define NO_OF_ALARMS 1
#include "alarm.h"

struct _alarmModule *aldm;
struct _alarmModule2 *aldm2;

main(argc, argv)
int argc;
char *argv[];
{
  char *headerPtr1;
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
      clearAtNode(i);
    }
    exit(0);
  } else if (node > 0) {
    clearAtNode(node);
    exit(0);
  }

/*
!   bind to data module ALARM, storage for alarm texts
*/
  aldm = (struct _alarmModule *) linkDataModule("ALARM", &headerPtr1);
  if (!aldm) {
    fprintf(stderr, "cannot link to datamodule '%s'\n", "ALARM");
    fprintf(stderr, "check if process 'scan' is running\n");
    return 0;
  }
  aldm2 = (struct _alarmModule2 *)
            (((char *) aldm) +
              (aldm->noOfAlarmEntries * sizeof(struct _alarmEntry) +
               sizeof(short) + sizeof(long)));


  aldm->alarmListPtr = 0;

  for (i = 0; i < aldm2->noOfAlarmPts; i++)
  {
    aldm2->alarmPts[i].active = 0;
    aldm2->alarmPts[i].disable = 0;
    aldm2->alarmPts[i].initTime = 0;
    aldm2->alarmPts[i].serialNo = 0;
  }
  unlinkDataModule(headerPtr1);
}

clearAtNode(node)
int node;
{
  int err;
  printf("Clearing alarms at node %d...", node); fflush(stdout);
  if ((err = netClearAlarm(node)) == 0) 
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


