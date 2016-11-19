/* lookalarm.c  1992-12-08 TD,  version 1.5 */
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
! lookalarm.c
! Copyright (C) 1991,1992 IVT Electronic AB.
*/


/*
!   lookalarm        - shows current alarm stack
! 
!   History:
!   Date        by   rev   what
!   ----------  ---  ----  ---------------------------------------------
!   1991-02-19  TD   1.00  initial coding
!   1991-05-06  TD   1.01  code shaped to fit new alarm routines
!   1991-07-30  TD   1.02  recompiled after new entry added in alarmModule2
!   1991-09-02  TD   1.03  option -e for extended format added
!   1992-05-18  TD   1.30  Added return-time and confirm-time in extended mode
!   1992-09-17  TD   1.40  Bugfix in globalAlarmMask system
!   1992-12-08  TD   1.50  Added statusEvents
!
!   Function: 
!   Presentates a list of the alarm stack
!
*/
@_sysedit: equ 3
@_sysattr: equ $8002

#define _V920914_1_40

#define NO_OF_ALARMS        1

#include <time.h>
#include "alarm.h"

struct _system *sysVars = SYSTEM_AREA; /* 0x003ffe0; */

char *short_time(tid)
long *tid;
{
  static char buf[25];
  struct tm tmbuff;

  if (!*tid) {
    strcpy(buf, "               ");
  } else {
    memcpy(&tmbuff, localtime(tid), sizeof(struct tm));
    sprintf(buf, "%02d%02d%02d %02d:%02d:%02d", 
      tmbuff.tm_year,
      tmbuff.tm_mon + 1,
      tmbuff.tm_mday,
      tmbuff.tm_hour,
      tmbuff.tm_min,
      tmbuff.tm_sec);
  }
  return buf;
}                   

int noOfBits(n)
int n;
{
  int i, one_bits = 0;
  for (i = 0; i < 8; i++) {
    if (n & 1)
      one_bits++;
    n >>= 1;
  }
  return one_bits;
}

char *statusEvents(status)
int status;
{
  static char str[80];
  str[0] = 0;
  if (status & ALARM_SEND_ASSERT)
    strcat(str, "assert+");
  if (status & ALARM_SEND_NEGATE)
    strcat(str, "negate+");
  if (status & ALARM_SEND_CONFIRM)
    strcat(str, "confirm+");
  if (status & ALARM_SEND_DISABLE)
    strcat(str, "disable+");
  if (status & ALARM_SEND_ENABLE)
    strcat(str, "enable+");
  if (status & ALARM_SEND_INIT)
    strcat(str, "(init)");
  return str;
}

int alarmStatus(entry)
struct _alarmEntry *entry;
{
  int as, ns, cs, ds, es, status;
  
  as = (entry->sendStatus & ALARM_SEND_ASSERT) ? 
        entry->sendMask ^ entry->assertSent & entry->sendMask : 0;
  ns = (entry->sendStatus & ALARM_SEND_NEGATE) ? 
        entry->sendMask ^ entry->negateSent & entry->sendMask : 0;
  cs = (entry->sendStatus & ALARM_SEND_CONFIRM) ? 
        entry->sendMask ^ entry->confirmSent & entry->sendMask : 0;
  ds = (entry->sendStatus & ALARM_SEND_DISABLE) ? 
        entry->sendMask ^ entry->disableSent & entry->sendMask : 0;
  es = (entry->sendStatus & ALARM_SEND_ENABLE) ? 
        entry->sendMask ^ entry->enableSent & entry->sendMask : 0;

  status = as | ns | cs | ds | es;
  
  return status;
}

main(argc, argv)
int argc;
char *argv[];
{
  int cnt, i, count;
  struct _alarmModule *aldm;
  struct _alarmModule2 *aldm2;
  char *headerPtr = 0;
  char buf[25];
  int extended = 0;
  
  if (argv[1][0] == '-')
  {
    if ((argv[1][1] | 0x20) == 'e')
      extended = 1;
    if (argv[1][1] == '?')
      printf("Usage: \n  -e  extended, shows status about send/sent\n");
  }
  initidcio();
  aldm = (struct _alarmModule *) linkDataModule("ALARM", &headerPtr);
  if (!aldm) {
    printf("cannot link to datamodule '%s'\n", "ALARM");
    printf("check if process 'scan' is running\n");
    return 0;
  }

  aldm2 = (struct _alarmModule2 *)
            (((char *) aldm) +
              (aldm->noOfAlarmEntries * sizeof(struct _alarmEntry) +
               sizeof(short) + sizeof(long)));

  printf("pkt  aktiv  disable  init-tid         serienr\n");
  printf("---------------------------------------------\n");
  for (i = 0; i < aldm2->noOfAlarmPts; i++) {
    strcpy(buf, short_time(&aldm2->alarmPts[i].initTime));
    printf("%3d  %5d  %7d  %s  %7d\n", aldm2->alarmPts[i].alarmNo,
       aldm2->alarmPts[i].active, aldm2->alarmPts[i].disable,
       buf, aldm2->alarmPts[i].serialNo);
  }
  printf("\n");

  if (!aldm->alarmListPtr) {
    printf("Inga alarm i listan\n");
    exit(0);
  }
  
  cnt = aldm->alarmListPtr;
  count = 0;
  for (i = 0; i < cnt; i++) {
    if (!aldm->alarmList[i].confirm)
      count++;
  }
  printf("%d alarm i listan, next serialNo = %d\n", count, sysVars->serialNo);
/*  
          *addressOfSerialNo);
*/
          
  for (i = 0; i < cnt; i++) {
    char buff[255];

    convert(aldm->alarmList[i].string, buff);
#ifdef _V920914_1_40
    printf("entry %3d: %s, alarmindex %d, %s", i, 
/*
a c
0 0       okvittat + oaktivt    wait for kvittens 
0 1       kvittat  + oaktivt    -> borta !
1 0       okvittat+ aktivt      just kommit
1 1       kvittat + aktivt      
*/
         aldm->alarmList[i].active ? 
           (aldm->alarmList[i].confirm ? "larmtillst}nd kvar" : "nytt larm") :
           (aldm->alarmList[i].confirm ? "(kvittat+inaktivt)" : "okvittat"),
              aldm->alarmList[i].alarmIndex,
              ctime(&aldm->alarmList[i].initTime));
#else
    printf("entry %3d: %s, alarmindex %d, %s", i, 
              aldm->alarmList[i].confirm ? "larmtillst}nd kvar" : "okvittat",
              aldm->alarmList[i].alarmIndex,
              ctime(&aldm->alarmList[i].initTime));
#endif
    {
      int n;
      char stat[20];
      
      n = noOfBits(alarmStatus(&aldm->alarmList[i]));
      if (aldm->alarmList[i].sendStatus & ALARM_SEND_INIT) {
        if (n)
          sprintf(stat, "%d pc kvar", n);
        else
          strcpy(stat, "sent");
      } else
        strcpy(stat, "not init");
      printf("%c, confirm %d, disable %d, active %d, alarmNo %d, (%d), %s\n", 
                    aldm->alarmList[i].class + 'A',
                    aldm->alarmList[i].confirm,
                    aldm->alarmList[i].disable,
                    aldm->alarmList[i].active,
                    aldm->alarmList[i].alarmNo,
                    aldm->alarmList[i].serialNo,
                    stat);
    }
    if (extended) {
#ifdef _V920914_1_40
      printf("sendMask $%02x, (Sent: assert $%02x, negate $%02x, confirm $%02x, disable $%02x, en $%02x)\n",
        aldm->alarmList[i].sendMask,
        aldm->alarmList[i].assertSent,
        aldm->alarmList[i].negateSent,
        aldm->alarmList[i].confirmSent,
        aldm->alarmList[i].disableSent,
        aldm->alarmList[i].enableSent);
      printf("sendStatus $%02x (Events: %s)\n", aldm->alarmList[i].sendStatus, 
                  statusEvents(aldm->alarmList[i].sendStatus));
      if (!aldm->alarmList[i].active)
        printf("Aatergick: %s", ctime(&aldm->alarmList[i].offTime));
      if (aldm->alarmList[i].confirm)
        printf("Kvitterat: %s", ctime(&aldm->alarmList[i].confirmTime));
      if (aldm->alarmList[i].disableTime)
        printf("Blockerat: %s", ctime(&aldm->alarmList[i].disableTime));
      if (aldm->alarmList[i].enableTime)
        printf("Avblocker: %s", ctime(&aldm->alarmList[i].enableTime));
#else
      printf("assert %d(%d), negate %d(%d), confirm %d(%d), disable %d(%d)\n",
        aldm->alarmList[i].assertSent,
        aldm->alarmList[i].sendAssert,
        aldm->alarmList[i].negateSent,
        aldm->alarmList[i].sendNegate,
        aldm->alarmList[i].confirmSent,
        aldm->alarmList[i].sendConfirm,
        aldm->alarmList[i].disableSent,
        aldm->alarmList[i].sendDisable
        );
      if (aldm->alarmList[i].sendNegate)    /* added 920518 */
        printf("Aatergick: %s", ctime(&aldm->alarmList[i].offTime));
      if (aldm->alarmList[i].sendConfirm)   /* added 920518 */
        printf("Kvitterat: %s", ctime(&aldm->alarmList[i].confirmTime));
#endif
    }
    printf("%s\n", buff);
    if (aldm->alarmList[i].string[strlen(aldm->alarmList[i].string)-1] != '\n')
      printf("\n");
  }
  unlinkDataModule(headerPtr);
}

convert(codestring, s)
char *codestring, *s;
{
  int i, len;

  strcpy(s, codestring);
  len = strlen(s);
  for (i = 0; i < len; i++) {
    if (s[i] == '\17') 
      s[i] = 'o';                 /* degree sign */
    else if (s[i] == '\06')
      s[i] = 'a';
    else if (s[i] == '\04')
      s[i] = 'a';
    else if (s[i] == '\24')
      s[i] = 'o';
    else if (s[i] == '\00')        /* ??? code = ??? */
      s[i] = 'A';
    else if (s[i] == '\16')
      s[i] = 'A';
    else if (s[i] == '\31')
      s[i] = 'O';
  }
}


