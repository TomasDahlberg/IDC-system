/**********************************************************************/
/* File:	NETBEAT.C             Type: M  (Main/Module/Include)  */
/* By:		Dan Axelsson, Christer Ohlman                         */
/* Description:	DT-08 HartBeat LED and NetPort Control                */
/* Use:		NETBEAT&  to put process in background                */
/*           or NETBEAT t  where t is a number for beat time (def=15) */
/*                      and status is displayed                       */
/* ---------------------  History  ---------------------------------- */
/*   Date     /   What                                                */
/* .................................................................. */
/* 91-10-28   /   started                                             */
/* 91-10-29   /   updated                                             */
/**********************************************************************/

#include <stdio.h>
#include "dt08io.h"

#define InSize 1024 /* input buffer size */
#define OutSize 1024 /* output buffer size */
#define FrmSize 32 /* frame size */

#define OFF 0
#define ON 1

typedef unsigned char * PTR;
typedef char BYTE;

long oldstat, oldnowr;

/* Device static storage */
typedef struct {
 PTR Shadow; /* ds.l 1 pointer to shadow register */
 PTR Handler; /* ds.l 1 current input interrupt handler */
 PTR InRead; /* ds.l 1 input-buffer read pointer */
 PTR InWrite; /* ds.l 1 input-buffer write pointer */
 PTR InEnd; /* ds.l 1 input-buffer end pointer */
 PTR OutRead; /* ds.l 1 output-buffer read pointer */
 PTR OutWrite; /* ds.l 1 output-buffer write pointer */
 PTR OutEnd; /* ds.l 1 output-buffer end pointer */
 PTR DevEven; /* ds.l 1 pointer to even device half */
 WORD InCount; /* ds.w 1 input-buffer level */
 WORD OutCount; /* ds.w 1 output-buffer level */
 WORD IRQMask; /* ds.w 1 device irq level mask */
 WORD Station; /* ds.w 1 netnode station id code */
 WORD MsbReg; /* ds.w 1 current address modifier */
 WORD LastAccs; /* ds.w 1 last accessed node */
 WORD CmdAccs; /* ds.w 1 node accessed during this command transfer */
 WORD NextAccs; /* ds.w 1 the next node to access */
 WORD PkSize; /* ds.w 1 current blocking size */
 WORD PkRdy; /* ds.w 1 current ready size */
 WORD PkIn; /* ds.w 1 current input block size */
 WORD PkOut; /* ds.w 1 current output block size */
 BYTE PkCmd; /* ds.b 1 the last transmitted command byte */
 BYTE PkMode; /* ds.b 1 current packet transfer mode */
 BYTE PkType; /* ds.b 1 current packet type (0= data, $80= commands) */
 BYTE PkFlag; /* ds.b 1 remote station accept flag */
 BYTE PkError; /* ds.b 1 accumulated errors */
 BYTE Select; /* ds.b 1 node select (0 = not selected) */
 BYTE MsgByte; /* ds.b 1 the last message byte sent to the other node */
 BYTE Parity; /* ds.b 1 parity code value */
 BYTE BaudRate; /* ds.b 1 baudrate code value */
 BYTE IrqFlags; /* ds.b 1 enabled irqs for this device */
 BYTE RtsBit; /* ds.b 1 enable for RTS output */
 BYTE InBuf[InSize]; /* ds.b InSize input data buffer */
 BYTE OutBuf[OutSize]; /* ds.b OutSize output data buffer */
 long FIX; /* testvalue */
 long NrofReads; /* ds.l 1 nr of good blocks read */
 long NrofWrites; /* ds.l 1 nr of good blocks written */
 long NrofErrors; /* ds.l 1 nr of errors */
 long NrofResets; /* ds.l 1 nr of resets */
 long TxStatus; /* ds.l 1 transmit status */
 } netstatic;

static netstatic *netstat;

void prstat()
  {
  printf("TxOn= %ld R= %ld W= %ld E= %ld Res= %ld PkError=%d\n",
  netstat->TxStatus,
  netstat->NrofReads,
  netstat->NrofWrites,
  netstat->NrofErrors,
  netstat->NrofResets,
  netstat->PkError);
  }

void clean_net()
  {
  int f;
  f = open("/nt", 0x01);
  _ss_dcoff(f, -1);
  close(f);
  }

void tststat()
  {
  if ((oldstat==ON) && (netstat->TxStatus==ON) && (netstat->NrofWrites==oldnowr))
      clean_net();
  oldstat= netstat->TxStatus;
  oldnowr= netstat->NrofWrites;
  }


main(argc, argv)
int argc;
char *argv[];
  {
  int delayt;
  long *p = (long *) 0x40C;
  netstat = (netstatic *) (*p);

  if (argc>1)
      delayt= atoi(argv[1]);
    else delayt= 30;
  while(1)
    {
    *qsop2= HBLED;  /* HBled on */
    tsleep(delayt);
    *qrop2= HBLED;  /* HBled off */
    tsleep(2*delayt);
    if (argc>1) prstat();
    tststat();
    }
  }

