/* setstat.c  1993-10-25 TD,  version 1.3 */
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
! setstat.c
! Copyright (C) 1992-1993 IVT Electronic AB.
*/


/*
!   Setstat - utility program for statistics
! 
!   History:
!   Date        by   rev   what
!   ----------  ---  ----  ---------------------------------------------
!   1992-??-??  TD   1.00  initial coding
!   1992-10-13  TD   1.10  update
!   1992-11-13  TD   1.20  added initNextSample
!   1993-10-25  TD   1.30  added tries-fields. New meta stat structure
!                          Creates metastat if module 'METASTAT' doesn't exists
!
!   Function:
!   Shows any meta data or stat data
!
*/
@_sysedit: equ 3
@_sysattr: equ $8002

#include <stdio.h>
#include <module.h>
#include <time.h>

/*
#define NEW_STAT_AS_OF_931021
*/

#include "../ivtnet/v1.1/stat.h"

#ifdef NEW_STAT_AS_OF_931021
int BUFF_MAX, ITEM_MAX;
#endif

static struct _metaStatModule *metaStatModule;
static struct _metaStatS *statS;
static struct _statBuff *statPtr;

short int *saveIdx = SAVE_IDX_ADDRESS;
short int *readIdx_PC0 = READ_IDX_ADDRESS_PC0;
short int *readIdx_PC1 = READ_IDX_ADDRESS_PC1;


int showFull = 0;

#ifdef NEW_STAT_AS_OF_931021
createStatModule()
{
  int siz, rev = 1;
  char buff[32];
  int noOfItems, noOfBuffs;

  printf("No of Buffers (8) : ");
  if (strlen(gets(buff)))
    noOfBuffs = atoi(buff);
  else
    noOfBuffs = 8;
    
  printf("No of Items per buffers (30) : ");
  if (strlen(gets(buff)))
    noOfItems = atoi(buff);
  else
    noOfItems = 30;

  siz = 2 + noOfBuffs * (32 + noOfItems * sizeof(struct _metaStatBuff));

  metaStatModule = (struct _metaStatModule *) 
            ((char *) 
                   _mkdata_module("METASTAT", siz, 
                        (short) mkattrevs(MA_REENT, rev), 
                        (short) MP_OWNER_READ | MP_OWNER_WRITE | MP_OWNER_EXEC)
                         + sizeof(struct modhcom));
  metaStatModule->noOfBuffs = noOfBuffs;
  metaStatModule->noOfItemsPerBuff = noOfItems;

  if (initStatPtr() == 0) {
    exit(1);
  }
}
#endif
                         
struct _metaStatS *statBufferPtr(metaStatModule, buf)
struct _metaStatModule *metaStatModule;
int buf;
{
#ifdef NEW_STAT_AS_OF_931021
  int siz, items, sizeOfOneBuffer;
  char *start;
  if (buf > metaStatModule->noOfBuffs)
    return 0;
  items = metaStatModule->noOfItemsPerBuff;
  start = (char *) metaStatModule->metaStatBuff;

  sizeOfOneBuffer = sizeof(struct _metaStatS) +
                                 sizeof(struct _metaStatBuff) * (items - 1);
  start += sizeOfOneBuffer * buf;
  return (struct _metaStatS *) start;
#else
  return &statS[buf];
#endif
}

struct _metaStatBuff *statItemPtr(metaStatModule, buf, item)
struct _metaStatModule *metaStatModule;
int buf, item;
{
  struct _metaStatS *metaPtr;
  metaPtr = statBufferPtr(metaStatModule, buf);
  return &metaPtr->rows[item];
}


initStatPtr()
{
#ifndef NEW_STAT_AS_OF_931021
  statS = METASTAT_ADDRESS;
  statPtr = SAVE_STAT_ADDRESS;
  return 1;
#else
  if ((metaStatModule = 
	(struct _metaStatModule *) modlink("METASTAT", (short) 0 /* any */)) 
            == (struct _metaStatModule *) -1) {
    statS = 0;
    statPtr = 0;
    metaStatModule = 0;
    printf("Cannot link to 'METASTAT', must create new\n");
    return 0;
  } else {
    metaStatModule = (struct _metaStatModule *) 
			(((char *) metaStatModule) + sizeof(struct modhcom));
    statS = (struct _metaStatS *) metaStatModule->metaStatBuff;
    statPtr = SAVE_STAT_ADDRESS;

    BUFF_MAX = metaStatModule->noOfBuffs;	  /* used to be 8 */
    ITEM_MAX = metaStatModule->noOfItemsPerBuff;  /* used to be 30 */
    return 1;
  }
#endif
}

initStatBuf()
{
  int i, row;
  
  struct _metaStatS *metaPtr;
  struct _metaStatBuff *entry;

  for (i = 0; i < BUFF_MAX; i++) {
    metaPtr = statBufferPtr(metaStatModule, i);
    metaPtr->name[0] = 0;
    for (row = 0; row < ITEM_MAX; row++) {
      entry = &metaPtr->rows[row];
      entry->buff = 0;
      entry->var[0] = 0;
    }
  }
}

time_t initNextSample(inter)      /* added 921113 */
long inter;
{
  struct tm t1;
  time_t c_time, this;
/*
!   Every hour; sample next 'xx:58:00'
!   Every day;  sample next '23:58:00'
!   Every week; sample next 'Sun 23:58:00'
!   Every month;sample next '31/30/29/28 23:58:00'
*/
  if (inter < 3600)
    return 0;

  c_time = time(0);  
  memcpy(&t1, localtime(&c_time), sizeof(struct tm));
  if (inter >= 86400)     /* ??? */
    t1.tm_hour = 23;                    /* 0 */
  t1.tm_min = 58;                       /* 0 */
  t1.tm_sec = 0;
  if (inter == 2678400) {
    if (t1.tm_mon == 1)       /* feb */
      t1.tm_mday = 28 + (t1.tm_year == 100 /* year 2000 leap or not (?) */ ? 1 :
           ((t1.tm_year % 4) == 0 ? 1 : 0));
    else 
      t1.tm_mday = 31 - ((t1.tm_mon < 7) ? 
                         (t1.tm_mon & 1) :               /* 1 if odd */
                         (1 - (t1.tm_mon & 1)));         /* 1 if even */
  }
  this = mktime(&t1);
/*    t1.tm_wday  0 == sun, 1 = mon, 6 = sat */
  if (inter == 604800)
    this += 86400 * (t1.tm_wday ? (7 - t1.tm_wday) : 0);
  return this - inter;
  
/*  return now - (c_time - this); */
}

setStat(name, row, buff, intervall, duc, var)
char *name;
int row, buff, intervall, duc;
char *var;
{
  struct _metaStatS *metaPtr;
  struct _metaStatBuff *entry;
  int i, free = -1;
  for (i = 0; i < BUFF_MAX; i++) {
    metaPtr = statBufferPtr(metaStatModule, i);
    if (metaPtr->name[0] == 0)
      if (free == -1)
        free = i;
    if (!strcmp(name, metaPtr->name))
      break;
  }
  
  if (i >= BUFF_MAX && free == -1) {
    printf("Ingen ledig plats, ej hittat !\n");
  } else {
    if (i >= BUFF_MAX) {
      i = free;
      metaPtr = statBufferPtr(metaStatModule, i);
      strcpy(metaPtr->name, name, 31);
    }
    entry = &metaPtr->rows[row];
    entry->buff = buff;
    entry->intervall = intervall;
    entry->sample = initNextSample(intervall);
    entry->duc = duc;
    strncpy(entry->var, var, 31);
    printf("Ok!\n");
  }
  return 1;
}

showStat(name, thisRow)
char *name;
int thisRow;
{
  struct _metaStatS *metaPtr;
  struct _metaStatBuff *entry;
  int i, row;

  for (i = 0; i < BUFF_MAX; i++) {
    metaPtr = statBufferPtr(metaStatModule, i);

    if (name != 0 && strcmp(name, metaPtr->name))
      continue;
    if (metaPtr->name[0] == 0) {
      printf("Stat %d empty\n", i);
      continue;
    }
    
    printf("Stat '%s':\n", metaPtr->name);
    for (row = 0; row < ITEM_MAX; row++) {
      if (name != 0 && row != thisRow)
        continue;

      entry = &metaPtr->rows[row];
      if (entry->var[0] == 0)
        printf("  %d: - empty -\n", row);
      else {
        printf("%3d: buff %d, intervall %d, duc %d, var %s\n", row,
            entry->buff, entry->intervall,
            entry->duc,  entry->var);
        if (showFull) {
          long s;
          s = entry->sample + entry->intervall;
#ifdef NEW_STAT_AS_OF_931021
          printf("  Tries=%2d, Next sample: %s", entry->tries, ctime(&s));
#else
          printf("  Next sample: %s", ctime(&s));
#endif
        }
      }
    }     /* next row */
  }   /* next buff */
}

/*
static struct _statBuff *statPtr = SAVE_STAT_ADDRESS;
short int *saveIdx = SAVE_IDX_ADDRESS;
*/
showData(buff, start)
int buff, start;
{
  struct _metaStatS *metaPtr;
  struct _metaStatBuff *entry;
  long i, b, t, s, q;
  double v;
  char *vp, unknown[32];
  
  strcpy(unknown, "Unknown var");
  for (i = start; i <= 4095; i++) {
    b = statPtr[i].bufItm;
    t = statPtr[i].Itm;
    s = statPtr[i].sampleTime;
    v = statPtr[i].value;
    metaPtr = statBufferPtr(metaStatModule, b);
    if (metaPtr) {
      q = metaPtr->rows[t].buff;
      vp = metaPtr->rows[t].var;
    } else {
      q = -1;
      vp = unknown;
    }

    if ((buff >= 0) && (q != buff))
      continue;
    printf("%4d: Buff %02d %s %g %s", i, q, 
                  vp,
                  v,
                  ctime(&s));
  }
}

clearData()
{
  int i;
  
  for (i = 0; i < 4095; i++) {
    statPtr[i].bufItm = 0;
    statPtr[i].Itm = 0;
    statPtr[i].sampleTime = 0;
    statPtr[i].value = 0;
  }
}

main()
{
  char buf[25];
  
  if (initStatPtr() == 0) {
#ifdef NEW_STAT_AS_OF_931021
    createStatModule();
#endif
  }

  while (1) {
    printf("SaveIdx     @ %x = %d\n", saveIdx, *saveIdx);
/*
    printf("ReadIdx PC0 @ %x = %d\n", readIdx_PC0, *readIdx_PC0);
    printf("ReadIdx PC1 @ %x = %d\n", readIdx_PC1, *readIdx_PC1);
*/
    printf("ReadIdx PC0 @ %x = %d %s\n", readIdx_PC0, (*readIdx_PC0) & 0x7fff,
                (*readIdx_PC0 & 0x8000) ? "(disable)" : "(enable)");
    printf("ReadIdx PC1 @ %x = %d %s\n", readIdx_PC1, (*readIdx_PC1) & 0x7fff,
                (*readIdx_PC1 & 0x8000) ? "(disable)" : "(enable)");

    printf("1. Initiera metadata och pekare\n");
    printf("2. Lagra/modifera metadata\n");
    printf("3. Visa samtlig metadata\n");
    printf("4. Visa insamlad data\n");
    printf("5. Radera insamlad data\n");
    printf("6. Disable/enable statistik till en viss pc\n");
    printf("7. Modifiera pekare\n");
    printf("0. Avsluta\n");

  switch (atoi(gets(buf))) {
    case 0:
      exit(0);
      break;
    case 1:
      initStatBuf();
      *saveIdx = 0;
      *readIdx_PC0 = 0;
      *readIdx_PC1 = 0;
      break;
    case 2:
      {
        char name[32], var[32];
        int row, buff, intervall, duc;
        printf("Buffer name: "); gets(name);
        printf("Row        : "); row = atoi(gets(var));
        showStat(name, row);
        printf("Buff       : "); buff= atoi(gets(var));
        printf("Intervall  : "); intervall = atoi(gets(var));
        printf("Duc        : "); duc = atoi(gets(var));
        printf("Var        : "); gets(var);
        setStat(name, row, buff, intervall, duc, var);
        showStat(name, row);
      }
      break;
    case 3:
      showFull = 0;
      {
        char buff[20];
        printf("Show next sample ?: (n) ");
        if (strlen(gets(buff))) {
          buff[0] |= 0x20;
          if (buff[0] == 'j' || buff[0] == 'y')
            showFull = 1;
        }
      }
      showStat(0,0);
      break;
    case 4:
      {
        char buff[20];
        int no, start;
        printf("Buff no (-1 = all) :");
        if (strlen(gets(buff))) 
          no = atoi(buff);
        else
          no = -1;
        printf("Start (0-4095)     : ");
        if (strlen(gets(buff))) 
          start = atoi(buff);
        else
          start = 0;
        showData(no, start);
      }
      break;
    case 5:
      {
        char buff[20];
        printf("Radera insamlad data ? : (n) ");
        if (strlen(gets(buff))) {
          buff[0] |= 0x20;
          if (buff[0] == 'j' || buff[0] == 'y') {
            printf("Raderar data..."); fflush(stdout);
            clearData();  
            printf("ok\n");
          }
        }
      }
      break;
    case 6:
    {
      int pc;
      char buff[20];
      
      printf("PC #  ? : ");
      pc = atoi(gets(buff));
      if (pc == 0)
        *readIdx_PC0 ^= 0x8000;
      else if (pc == 1)
        *readIdx_PC1 ^= 0x8000;
      else
        printf("Fel PC# (0 eller 1) !\n");
    }   
    break;
    case 7:
    {
      int pc, value;
      char buff[20];
      
      printf("PC #  ? : ");
      pc = atoi(gets(buff));
      if (pc != 0 && pc != 1) {
        printf("Fel PC# (0 eller 1) !\n");
        break;
      }
      if (pc == 0)
        printf("ReadIdx PC0 @ %x = %d %s\n", readIdx_PC0, (*readIdx_PC0) & 0x7fff,
                (*readIdx_PC0 & 0x8000) ? "(disable)" : "(enable)");
      else if (pc == 1)
        printf("ReadIdx PC1 @ %x = %d %s\n", readIdx_PC1, (*readIdx_PC1) & 0x7fff,
                (*readIdx_PC1 & 0x8000) ? "(disable)" : "(enable)");

      printf("Nytt varde : ");
      value = atoi(gets(buff));
      if (pc == 0) 
        value |= (*readIdx_PC0 & 0x8000);
      else if (pc == 1)
        value |= (*readIdx_PC1 & 0x8000);

      if (pc == 0) 
        *readIdx_PC0 = value;
      else if (pc == 1)
        *readIdx_PC1 = value;
    }   
    break;
    default:
      printf("No such choice\n");
      break;
  }
  }
}
