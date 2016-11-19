/* reg.c  1992-06-04 TD,  version 1.1 */
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
! reg.c
! Copyright (C) 1991,1992 IVT Electronic AB.
*/

/*
!     reg.c is part of the IVTnet product
!
!     File: reg.c
!     
!     Contains main function for reg process at master site
!
!     History     
!     Date        Revision Who  What
!     
!      ?-jan-1992     1.0  TD   Start of coding
!
!      4-jun-1992     1.1  TD   Changed MAX_NODES from 256 to 64
!                               Changed size of struct _reg from
!                               10*48 = 480 to 40 * 24 = 960 bytes
*/
#include <time.h>
#include "layer2.h"
#include "ivtnet.h"
#include "net.h"

#define DELIVER_DELAY 30
#define DISTR_PACKET_SIZE 8    /* 8*8 = 64 */
#define MAX_NODES 64

extern char *dm, *meta;
extern struct _message *message;
extern int DEBUG2;

#define MAX_REGS 40
static struct _reg {
  int id;			/* our variable identification */
  unsigned char value[8];	/* sent value, holds binary double or long */
  unsigned char subscriber[DISTR_PACKET_SIZE];/* bitmap 8*8=64 nodes of targets */
  long updateTime;
} registration[MAX_REGS];   /* size = 40*24 = 960 */


/*
!	Here comes the routines for registration and send-var-change 
*/
int registrate(id, toNode)	/* returns 1 if ok, otherwise 0 */
int id, toNode;
{
  int i, j = -1, ret = 0;
  
  if (toNode >= MAX_NODES)
    return 0;
  for (i = 0; i < MAX_REGS; i++) {
    if (DEBUG2) {
      int q;
      printf("Slot %d: ", i);
      if (registration[i].id) {
        printf("id %d, age %d, nodes: ", registration[i].id,
                        getRelTime(0) - registration[i].updateTime);
        for (q = 0; q < MAX_NODES; q++)
          if (registration[i].subscriber[q / 8] & (1 << (q % 8)))
            printf("%d, ", q);
        printf("\n");
      } else
        printf("- empty -\n");
    }
    if ((registration[i].id == 0) && (j == -1))
	j = i;
    if (registration[i].id == id)
      break;
  }
  if ((j == -1) && (i >= MAX_REGS)) {
    if (DEBUG2) printf("No empty slot found\n");
    return 0;		/* not found and no empty slot */
  }
  if (i < MAX_REGS) {
    registration[i].subscriber[toNode / 8] |= (1 << (toNode % 8 ));
    ret = 1;
  } else {
    int size;
 
    registration[j].updateTime = 0;
    size = metaSize(meta, id);
    if (size > 8)
	size = 8;
    registration[j].id = id;
    memcpy(registration[j].value, metaValue(dm, meta, id), size);
    registration[j].subscriber[toNode / 8] = (1 << (toNode % 8 ));
    if (DEBUG2) printf("Ok, variable registrated !\n");
    ret = 1;
  }


  for (i = 0; i < MAX_REGS; i++) {
    if (DEBUG2) {
      int q;
      printf("Slot %d: ", i);
      if (registration[i].id) {
        printf("id %d, age %d, nodes: ", registration[i].id,
                        getRelTime(0) - registration[i].updateTime);
        for (q = 0; q < MAX_NODES; q++)
          if (registration[i].subscriber[q / 8] & (1 << (q % 8)))
            printf("%d, ", q);
        printf("\n");
      } else
        printf("- empty -\n");
    }
  }
  
  return ret;
}

int sendNextRegistrated(sBuf, retSiz)	/* returns true if buffer must be sent */
char *sBuf;
int *retSiz;
{
   static int nextId;
   int this, id, siz, size, pos, node;

   if (nextId >= MAX_REGS)
     nextId = 0;
   if (nextId < 0)
     nextId = 0;
   this = nextId;
   nextId++;

if (DEBUG2) printf("sendNextReg: check slot %d\n", this);
   if ((id = registration[this].id) == 0)
     return 0;
if (DEBUG2) printf("check update time\n");
   if (abs(getRelTime() - registration[this].updateTime) < DELIVER_DELAY)
     return 0;
if (DEBUG2) printf("check difference\n");
   siz = metaSize(meta, id);
   if (siz > 8)
     siz = 8;

   if (!memcmp(registration[this].value, metaValue(dm, meta, id), siz))
	return 0;
if (DEBUG2) printf("ok, different!\n");
/*
!   obs ! must send same value as copied !
*/ 
   node = 0;/* send this packet to master who is responsible for distribution*/

   message = (struct _message *) sBuf;
   message->message_type = GET_VAR | REPLY_MASK | REGISTRATE;
   message->error = NET_NOERROR;
   strcpy(message->mix.varRequest.varName, 
                                  (char *) metaName(meta, id));
   size = 2 + strlen(message->mix.varRequest.varName) + 1 + 1;
   pos = strlen(message->mix.varRequest.varName) + 1;

   message->mix.varRequest.varName[pos] = metaType(meta, id);
   memcpy(registration[this].value, metaValue(dm, meta, id), siz);
   memcpy(&message->mix.varRequest.varName[pos + 1], 
				registration[this].value, siz);
   size += siz;
   pos ++;
   pos += siz;
   memcpy(&message->mix.varRequest.varName[pos],
		registration[this].subscriber, DISTR_PACKET_SIZE);
		                                      /* should be first ? */
   size += DISTR_PACKET_SIZE;
   registration[this].updateTime = getRelTime();
   *retSiz = size;
/*   sendPacket(node, sBuf, size);    */
   return 1;
}

