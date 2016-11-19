/* shownet.c  1993-05-17 TD,  version 1.3 */
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
 * Heimdalsgatan 4
 * 113 28 Stockholm
 * Sweden
 */

/*
! shownet.c
! Copyright (C) 1991-1993 IVT Electronic AB.
*/

/*
!     shownet.c is part of the IVTnet product
!
!     File: shownet.c
!     
!     Shows network status from master site
!
!     History     
!     Date        Revision Who  What
!     
!      3-sep-1991     1.0  TD   Start of coding
!      5-feb-1992     1.1  TD   Updated to HEX for alarmfield (ABCD-alarms)
!      6-may-1992     1.2  TD   Changed name from show to shownet
!                               since we have showipl and shownode also
!     17-may-1993     1.3  TD   Added -3,-4 options, quick fix printf->write
*/

@_sysedit: equ 3
@_sysattr: equ $8009

#include <stdio.h>
#include <time.h>
#include <module.h>
#include "../../sysvars.h"

#ifdef NEW_930928
#include "nodes.h"
#endif

static struct _system *sysVars = SYSTEM_AREA;

#define MAX_NODE_NO 64

/* #define NEW_930928 */
#ifndef NEW_930928
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
  unsigned char A,B,C,D,cA,cB,cC,cD;
/*
  unsigned long activeABCD;
  unsigned long confirmedABCD;
*/
  unsigned short version;
};    /* size = 22 * MAX_NODES = 1408 */
struct _node *nodeMap = (struct _node *) 0x0003f000;
#else
struct _node *nodeMap;
#endif

struct _node *n;
struct _ivtnet *ivtnet;

char *ivtnet_module;

void icp(s)
int s;
{
  munlink(ivtnet_module);
  gotoxy(0, 21);
  exit(0);
}

print(s)
char *s;
{
  write(1, s, strlen(s));
}

gotoxy(x, y)
int x, y;
{
  char buff[10];
  sprintf(buff, "%c%c%02d;%02d%c", 27, 91, y + 1, x + 1, 72);
  write(1, buff, 8);
}

clearScreen()
{
  static char buf[4] = { 27, 91, '2', 'J' };
  write(1, buf, 4);
}

static unsigned char *map2Node = (unsigned char *) 0x0003f600;
static int nextFree_inMap2Node;

int usage()
{
  fprintf(stderr, "Syntax: shownet [<opt>]\n");
  fprintf(stderr, "Function. slave process interface to IVTnet\n");
  fprintf(stderr, "Options:\n");
  fprintf(stderr, "    -u  show alive nodes only\n");
  fprintf(stderr, "    -n  show nodemap\n");
  fprintf(stderr, "    -3  show nodes 0-59\n");
  fprintf(stderr, "    -4  show nodes 0-79\n");
}

static char *cmdTexts[] = { "GET_VAR", "PUT_VAR", "ALARM_NO", 
                  "ALARM_TEXT_1","ALARM_TEXT_2","CONFIRM_ALARM",
                  "SET_TIME","SET_HOST","KEY_DISPLAY",
                  "GETIDX_VAR_1","GET_UPDATED_1","SET_REMOTE",
                  "REMOTE_DATA","GET_VARIDX","GET_CAL",
                  "SET_CAL","GETNIDX_VAR_1", "VERSION",
                  "GETIDX_VAR_2", "GET_UPDATED_2", "GETNIDX_VAR2",
                  "CLEAR_ALARM", "GET_STAT_VAR",
                  "ALLOCATE_MEM", "GET_MEM", "PUT_MEM", "LOAD_PROGRAM",
                  "REBOOT", "NEW"
};
static char *netErrors[] = {
"NET_NOERROR",
"NET_NOSUCHVAR","NET_ILLEGALTYPE","NET_CANCEL",
"NET_NOBINDING","NET_NOALARMS","NET_BUSY",
"NET_SPAWN_ERROR","NET_TIMEOUT","NET_LOGOUT",
"NET_CTRL_C","NET_NOSUCHALARM","NET_NOMOREALARMS",
"NET_REGISTRATE",
"NET_NOSUCHNODE",
"NET_WRONG_CRC"
};

main(argc, argv)
int argc;
char *argv[];
{
  int x, y, i, ups = 0, mx = 19, nds = 0;
  int option_3 = 0, option_4 = 0;
  int specialNode = -1;
  union {
    struct {
       unsigned char A,B,C,D,cA,cB,cC,cD;
    } a;
    struct {
      unsigned long activeABCD;
      unsigned long confirmedABCD;
    } b;
  } u;
  
  intercept(icp);

/*
!   create data module ivtnet, added 930928
*/
#ifdef NEW_930928
  if ((ivtnet_module = ivtnet = 
              (struct _ivtnet *) modlink("IVTnet", (short) 0 /* any */)) 
                            == (struct _ivtnet *) -1) {
    printf("Cannot link to data module 'IVTnet'\n");
    printf("Make sure server is running\n");
    exit(1);
  } else {
    ivtnet = (struct _ivtnet *) (((char *) ivtnet) + sizeof(struct modhcom));
  }

  nodeMap = (struct _node *) ivtnet->nodeMap;
  map2Node = ivtnet->map2Node;
#endif

  mx = 64; /* testa */

  if (argc == 2 && argv[1][0] == '-') {
    if ((argv[1][1] | 0x20) == 'u') {
      ups = 1;
      mx = 64;
    } else if ((argv[1][1] | 0x20) == 'n') {
      nds = 1;
    } else if (argv[1][1] == '3') {
      option_3 = 1;
    } else if (argv[1][1] == '4') {
      option_4 = 1;
    } else if (argv[1][1] == '?') {
      usage();
      exit(0);
    }
  } else if (argc == 2) {
    specialNode = atoi(argv[1]);
  }
  clearScreen();
#define NEW 1
#if NEW
      gotoxy(0,0);
if (option_3)
  print("Nod siz poll vers kv/okv Nod siz poll vers kv/okv Nod siz poll vers kv/okv \n");
else if (option_4)
  print("Nod siz polls vers Nod siz polls vers Nod siz polls vers Nod siz polls vers \n");
else
  print("Nod try siz   count     alrms polls dwn vers Full  A, B, C, D  cA, cB, cC, cD\n");
#endif
  while (1) {           /* for ever */
    x = 0; y = 1;
    for (i = 0; i < mx; i++, y++) {
      if (i && (specialNode != -1)) {
        i = specialNode;
      }
      if (y > 20) {
        if (option_3) {
          if (x >= 50)
            break;
          x += 25;
          y = 1;
        } else if (option_4) {
          if (x >= 57)
            break;
          x += 20;
          y = 1;
        } else
          break;
      }
      gotoxy(x,y);
      if (ups && (nodeMap[i].size < 0)) {
        y--;
        continue;
      }
#if NEW
      n = &nodeMap[i];
#ifdef NEW_930928
      u.b.activeABCD = n->activeABCD;
      u.b.confirmedABCD = n->confirmedABCD;
#endif
/*
Node xx fail xxx size xxx, count (xxxxx, xxxxx), alarms xxxx, polls xxx, (x)
Nod try siz   count     alrms polls dwn vers Full  A, B, C, D  cA, cB, cC, cD
 xx xxx xxx xxxxx,xxxxx  xxxx  xxx  (x) 1.671  x  93 91 00 12  99  000 000 000
 xx xxx xxx xxxxx,xxxxx  xxxx  xxx  (x) 1.671  x  93 91 0  12  000 000 000 000

+17   (show extended information about node 17)
      
      Node: 17
      Last packet:  MSG_NO_OF_ALARMS
      packet info:  -
      
-2    (upp till 40 st)
////////////////////////////////////////

Nod try siz   count     alrms polls dwn vers Full  A, B, C, D  cA, cB, cC, cD
 xx xxx xxx xxxxx,xxxxx  xxxx  xxx  (x) 1.671  x  93 91 00 12  99  000 000 000
 xx xxx xxx xxxxx,xxxxx  xxxx  xxx  (x) 1.671  x  93 91 0  12  000 000 000 000

-3    (upp till 60st)
Nod siz poll vers kv/okv Nod siz poll vers kv/okv Nod siz poll vers kv/okv 
 xx xxx xxx 1.850 99/99 | xx xxx xxx 1.850 99/99 | xx xxx xxx 1.850 99/99  
 xx xxx xxx 1.850 99/99 | xx xxx xxx 1.850 99/99 | xx xxx xxx 1.850 99/99  
 xx xxx xxx 1.850 99/99 | xx xxx xxx 1.850 99/99 | xx xxx xxx 1.850 99/99  

-4    (upp till 80st)
Nod siz polls vers Nod siz polls vers Nod siz polls vers Nod siz polls vers 
 xx xxx  xxx  1.850|xx xxx  xxx  1.850|xx xxx  xxx  1.850|xx xxx  xxx  1.850
 xx xxx  xxx  1.850|xx xxx  xxx  1.850|xx xxx  xxx  1.850|xx xxx  xxx  1.850
 xx xxx  xxx  1.850|xx xxx  xxx  1.850|xx xxx  xxx  1.850|xx xxx  xxx  1.850
 xx xxx  xxx  1.850|xx xxx  xxx  1.850|xx xxx  xxx  1.850|xx xxx  xxx  1.850
 xx xxx  xxx  1.850|xx xxx  xxx  1.850|xx xxx  xxx  1.850|xx xxx  xxx  1.850
*/
if (option_3)
  printf(" %2d %3d %3d %01d.%03d %02d/%02d |\n", i, n->size, 
    (n->size >= 0) ? n->pollCount : n->failCount, 
    n->version >> 10, n->version & 1023,
#ifdef NEW_930928
    u.a.A + u.a.B + u.a.C + u.a.D, u.a.cA + u.a.cB + u.a.cC + u.a.cD);
#else
    n->A + n->B + n->C + n->D, n->cA + n->cB + n->cC + n->cD);
#endif
else if (option_4)
  printf("%2d %3d  %3d  %01d.%03d|\n", i, n->size, 
    (n->size >= 0) ? n->pollCount : n->failCount, 
    n->version >> 10, n->version & 1023);
else
  printf(" %2d %3d %3d %5d,%5d  %04x  %3d  (%1d) %1d.%03d %1d   %2d %2d %2d %2d  %2d  %2d  %2d  %2d\n",
    i, n->failCount, n->size, n->rCount, n->sCount, n->noOfAlarms, 
    n->pollCount, n->failFlag,
    n->version >> 10, n->version & 1023, 
    n->full, 
#ifdef NEW_930928
    u.a.A, u.a.B, u.a.C, u.a.D, u.a.cA, u.a.cB, u.a.cC, u.a.cD);
#else
    n->A, n->B, n->C, n->D, n->cA, n->cB, n->cC, n->cD);
#endif

#else
      printf(
"Node %2d fail %3d size %3d, count (%5d, %5d), alarms %04x, polls %d, (%d)\n",
          i, nodeMap[i].failCount, nodeMap[i].size,
             nodeMap[i].rCount, nodeMap[i].sCount,
             nodeMap[i].noOfAlarms, nodeMap[i].pollCount, nodeMap[i].failFlag);
#endif             
      if (specialNode != -1) {
        if (!specialNode)
          break;
        if (i)
          break;
      }
    }

    x = 0;
    if (i >= 20) 
      y = 21;

    if (specialNode != -1) {
      y = 4;
    }
    gotoxy(x,y);
    printf("Server up since %s", ctime(&sysVars->reboots));
    if (specialNode != -1) {
      y ++;
      y ++;
      gotoxy(x,y);


#ifdef NEW_930928
      printf("Received:\n");
      printf("Cmd = $%02x, from node = %3d, error =  %3d, %s", 
                    (unsigned char ) n->rec.command, (unsigned) n->rec.fromNode, 
                    (unsigned) n->rec.error, 
                    ctime(&n->recTid));
      if (n->rec.command & 0x80)
        printf("REQUEST for ");
      else
        printf("REPLY to ");
      if (n->rec.command & 0x40)
        printf("REGISTRATE ");

      if ((n->rec.command & 0x3f) <= (sizeof(cmdTexts) / sizeof(cmdTexts[0])))
        printf("Command: '%s'                  \n", cmdTexts[(n->rec.command & 0x3f) - 1]);
      else
        printf("Command: UNKNOWN COMMAND !!, %d\n", n->rec.command & 0x3f);
      printf("Error: %s                       \n", netErrors[n->rec.error & 0x3f]);
/*  if (cmd < 2) {  printf("Variable: '%s'\n", &buf[9]);  } */

      printf("\nTransmitted:\n");
      printf("Cmd = $%02x, to   node = %3d, error =  %3d, %s", 
                    (unsigned char ) n->trans.command, (unsigned) n->trans.toNode,
                     (unsigned) n->trans.error,
                    ctime(&n->transTid));
      if (n->trans.command & 0x80)
        printf("REQUEST for ");
      else
        printf("REPLY to ");
      if (n->trans.command & 0x40)
        printf("REGISTRATE ");

      if ((n->trans.command & 0x3f) <= (sizeof(cmdTexts) / sizeof(cmdTexts[0])))
        printf("Command: '%s'                  \n", cmdTexts[(n->trans.command & 0x3f) - 1]);
      else
        printf("Command: UNKNOWN COMMAND !!, %d\n", n->trans.command & 0x3f);
      printf("Error: %s                        \n", netErrors[n->trans.error & 0x3f]);
/*  if (cmd < 2) {  printf("Variable: '%s'\n", &buf[9]);  } */

      printf("\nOutermost Counter = %6d\n", ivtnet->outerMostCounter);
      printf("Outer     Counter = %6d\n", ivtnet->outerCounter);
#endif
/*
Received:
Cmd = %3d, from node = %3d, error = %3d, 
Transmitted:
Cmd = %3d, to nod    = %3d, error = %3d, 
*/
    }
    if (nds) {
      int i;
      printf("The following nodes are in poll-list;\n");
      for (i = 0; i < 64; i++) {
        if (map2Node[i] == 0)
          break;
        printf("%d, ", map2Node[i]);
      }
      printf("        ");
      fflush(stdout);
/*      write(1, "          ", 10);     /* changed from printf, 930517 */
/*      printf(  "          \n");   */
    }
/*
    printf("Total %d kB sent across net\n", nodeMap[0].pollCount, nodeMap[0].
    printf("Average effective net speed is %d bps\n", nodeMap[0].pollCount * 9600 / 100);
*/
  } 
}
