/* net.h  1991-09-03 TD,  version 1.0 */
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
! net.h
! Copyright (C) 1991, IVT Electronic AB.
*/

#define NET_EVENT_FREE         "NetFree"
#define NET_EVENT_TASK_ACCOMPLISHED "NetTask"

#define CMD_NO      0x00
#define CMD_GETVAR  0x01
#define CMD_PUTVAR  0x02
#define CMD_SETHOST 0x03
#define CMD_ALARMS  0x04

#define CMD_GETALARM_TEXT 0x05
#define CMD_CONFIRM_ALARM 0x06

#define CMD_GETIDX_VAR    0x07
#define CMD_PUTIDX_VAR    0x08
#define CMD_GET_UPDATED   0x09
#define CMD_SET_REMOTE    0x0a
#define CMD_ACK_ALARM     0x0b

#define CMD_GET_VARIDX    0x0c
#define CMD_GET_CAL       0x0d
#define CMD_SET_CAL       0x0e

#define CMD_GET_N_IDX_VAR 0x0f

#define CMD_CLEAR_ALARM   0x10

#define CMD_ALLOCATE_MEM  0x11
#define CMD_GET_MEM       0x12
#define CMD_PUT_MEM       0x13
#define CMD_LOAD_PROGRAM  0x14
#define CMD_REBOOT        0x15
#define CMD_NEW           0x16

struct _method1Arr {
  short int node, idx; char name[32]; double value;
};


#define ALARM_TEXT_SIZE 100

#if 0
#define ALARM_TEXT_ADDRESS  (unsigned char *) 0x003fc00;    /* 3ff00 -> 3fc00 930225 */

#define COMMAND_ADDRESS     (unsigned char *) 0x003ffc7;    /* d7 -> c7 930225 */
#endif

struct _alarmStrct {
  unsigned short int alarmNo, serialNo, status;
  long dtime;
  char text[80];
};

/*
!   the api struct for GET_MEM and PUT_MEM is 
  struct { long address; long size; char *sourcePtr };
*/

/* unsigned char *currentCommand = COMMAND_ADDRESS; */

/*
static unsigned char *alarmTextBufPtr = ALARM_TEXT_ADDRESS;
*/

#ifdef _WHERE_SHOULD_THIS_BE_
struct _node
{
  unsigned char failCount;
  unsigned char full;               /*  added 920306 */  
  short size;
  unsigned short int rCount, sCount;
  unsigned short noOfAlarms;       /* come, gone, disable, confirm -> 4*100 */

  unsigned char pollCount;        /* incremented for each poll */
  unsigned char failFlag;
/*  added 920306 */
  unsigned long activeABCD;
  unsigned long confirmedABCD;
  unsigned short version;
};    /* size = 22 * MAX_NODES = 1408 */
#endif
