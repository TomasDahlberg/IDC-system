#define InSize 1024 /* input buffer size */
#define OutSize 1024 /* output buffer size */
#define FrmSize 32 /* frame size */

typedef unsigned char * PTR;
typedef char BYTE;
typedef unsigned short WORD;

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


