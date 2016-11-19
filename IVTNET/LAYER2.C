#include "layer2.h"

/*
struct _reply *mkReplyBlock(rec, seqNo, reasonMask)
union _mix *rec;
unsigned char seqNo;
unsigned char reasonMask;
{
  static struct _reply buf;
  
  buf.head.size = sizeof(struct _reply);
  buf.head.seqNo = seqNo;
  buf.head.targetMaster = 0;
  buf.head.targetNode = rec->frame.head.sourceNode;
  buf.head.sourceMaster = 0;
  buf.head.sourceNode = rec->frame.head.targetNode;
  buf.head.cmd = REPLY_PACKET | reasonMask;
  buf.head.hChs = 0;
  buf.head.hChs = headerChs(&buf.head);
  return &buf;
}
*/

int allBlockChs(rec)      /* returns zero if ok ! */
union _mix *rec;
{
  int i;
  unsigned char Lchs = 0;
  char *bufPtr = (char *) rec;

  for (i = 0; i < rec->frame.head.size; i++)
    Lchs ^= *bufPtr++;
  return Lchs;
}

int headerChs(head)      /* returns zero if ok ! */
struct _header *head;
{
  int i;
  unsigned char Lchs = 0;
  char *headPtr = (char *) head;
 
  for (i = 0; i < sizeof(struct _header); i++)
    Lchs ^= *headPtr++;
  return Lchs;
}
