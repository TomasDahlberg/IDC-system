#include <stdio.h>

#define InSize 1024 /* input buffer size */
#define OutSize 1024 /* output buffer size */
#define FrmSize 32 /* frame size */

#define OFF 0
#define ON 1

typedef unsigned char * PTR;
typedef char BYTE;
typedef unsigned short WORD;

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
 char cmds[512];
 long FIX; /* testvalue */
 long NrofReads; /* ds.l 1 nr of good blocks read */
 long NrofWrites; /* ds.l 1 nr of good blocks written */
 long NrofErrors; /* ds.l 1 nr of errors */
 long NrofResets; /* ds.l 1 nr of resets */
 long TxStatus; /* ds.l 1 transmit status */
 } netstatic;

static netstatic *netstat, netbuf;

static unsigned short swapword(w)
unsigned short w;
{
  return ((w & 0xff) << 8) | ((w >> 8) & 0xff);
}

static unsigned long swaplong(l)
unsigned long l;
{
  unsigned short w1, w2;
  w1 = (l >> 16) & 0xffff;
  w2 = l & 0xffff;
  return (((unsigned long) swapword(w2)) << 16) | swapword(w1);
}

void prstat()
  {
  printf("Shadow =$%lx\n", swaplong(netstat->Shadow));
  printf("Handler=$%lx\n", swaplong(netstat->Handler));
  printf("InRead =$%lx\n", swaplong(netstat->InRead));
  printf("InWrite=$%lx\n", swaplong(netstat->InWrite));
  printf("InEnd  =$%lx\n", swaplong(netstat->InEnd));
  printf("OutRead =$%lx\n", swaplong(netstat->OutRead));
  printf("OutWrite=$%lx\n", swaplong(netstat->OutWrite));
  printf("OutEnd  =$%lx\n", swaplong(netstat->OutEnd));
  printf("DevEven =$%lx\n", swaplong(netstat->DevEven));
  printf("\n");
  printf("InCount  = %d\n", swapword(netstat->InCount));
  printf("OutCount = %d\n", swapword(netstat->OutCount));
  printf("IRQMask  =$%x\n", swapword(netstat->IRQMask));
  printf("Station  = %d\n", swapword(netstat->Station));
  printf("MsbReg   = %d\n", swapword(netstat->MsbReg));
  printf("LastAccs = %d\n", swapword(netstat->LastAccs));
  printf("CmdAccs  = %d\n", swapword(netstat->CmdAccs));
  printf("NextAccs = %d\n", swapword(netstat->NextAccs));
  printf("PkSize   = %d\n", swapword(netstat->PkSize));
  printf("PkRdy    = %d\n", swapword(netstat->PkRdy));
  printf("PkIn     = %d\n", swapword(netstat->PkIn));
  printf("PkOut    = %d\n", swapword(netstat->PkOut));
  printf("PkCmd    = %d\n", netstat->PkCmd);
  printf("PkMode   = %d\n", netstat->PkMode);
  printf("PkType   = %d\n", netstat->PkType);
  printf("PkFlag   = %d\n", netstat->PkFlag);
  printf("PkError  = %d\n", netstat->PkError);
  printf("Select   = %d\n", netstat->Select);
  printf("MsgByte  = %d\n", netstat->MsgByte);
  printf("Parity   = %d\n", netstat->Parity);
  printf("BaudRate = %d\n", netstat->BaudRate);
  printf("IrqFlags = %d\n", netstat->IrqFlags);
  printf("RtsBit   = %d\n", netstat->RtsBit);

  printf("\n");
  printf("FIX      = %lx\n", swaplong(netstat->FIX));

  printf("NrofReads = %ld\n", swaplong(netstat->NrofReads));
  printf("NrofWrites= %ld\n", swaplong(netstat->NrofWrites));
  printf("NrofErrors= %ld\n", swaplong(netstat->NrofErrors));
  printf("NrofResets= %ld\n", swaplong(netstat->NrofResets));
  printf("TxStatus  = %ld\n", swaplong(netstat->TxStatus));

/*
  printf("TxOn= %ld\nR= %ld W= %ld E= %ld Res= %ld PkError=%d\n",
  swaplong(netstat->TxStatus),
  swaplong(netstat->NrofReads),
  swaplong(netstat->NrofWrites),
  swaplong(netstat->NrofErrors),
  swaplong(netstat->NrofResets),
  netstat->PkError);
*/

}

void tststat()
  {
  if ((oldstat==ON) && (netstat->TxStatus==ON) && (netstat->NrofWrites==oldnowr))
      ;
  oldstat= netstat->TxStatus;
  oldnowr= netstat->NrofWrites;
  }

int hex(unsigned char a, unsigned char b)
{
   int c, d;
   if (a >= 'A')
	c = a - 'A' + 10;
   else
	c = a - '0';
   if (b >= 'A')
	d = b - 'A' + 10;
   else
	d = b - '0';
   return (c << 4) + d;
}

void poke(char *inbuf, long base, long address, int i, int c)
{
  *(inbuf + (address - base) + i) = c;
}

main(argc, argv)
int argc;
char *argv[];
{
  FILE *fp;
  long base, address;
  int i, q, c;
  unsigned char buf[256];
  unsigned char *inbuf;

  netstat = (netstatic *) &netbuf;
  inbuf = (char *) &netbuf;
  base = 0;

  if (argc != 2) {
	printf("Usage: test <file>\n");
	exit(1);
  }
  if ((fp = fopen(argv[1], "r")) == 0) {
	printf("cannot open '%s'\n", argv[1]);
	exit(1);
  }

  while (fgets(buf, 255, fp)) {
    if (strlen(buf) < 12)
	continue;
    if (buf[9] != '-')
      continue;
    if (buf[8] != ' ')
      continue;
    if (buf[10] != ' ')
      continue;

    sscanf(buf, "%lx", &address);
    if (base == 0)
      base = address;

    if ((address - base) > sizeof(netbuf))
	break;

    printf("%s", buf);

    for (i = 0; i < 16; i++) {
	q = 11 + 5*(i/2);   if (i & 1) q += 2;
/*		printf("i=%d, q=%d\n", i, q);	*/
	c = hex(buf[q], buf[q+1]);
	poke(inbuf, base, address, i, c);
    }

  }
  prstat();
  fclose(fp);
}
