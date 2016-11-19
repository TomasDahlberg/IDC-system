/**************************************************************************/
/* File:        DTPROTO.H      Type: I  (mainProgram/Module/Include)      */
/* By:          Dan Axelsson                                              */
/* Description:	Constants, structs, prototypes and variables              */
/*                common to SENDMOD and RECMOD                            */
/* Use:         #include "dtproto.h"                                      */
/* ---------------------  History  -------------------------------------  */
/*   Date     /   What                                                    */
/* ...................................................................... */
/* 91-03-16   /   started                                                 */
/* 91-09-15   /   Sequence number variables now unsigned char (prev int)  */
/* 91-09-15   /   updated                                                 */
/**************************************************************************/

#define ER_GENERAL -1
#define ER_READ    -2
#define ER_TIMOUT  -3
#define ER_WRITE   -4
#define ER_SEQ     -5
#define ER_CHKSUM  -6
#define ER_LENGTH  -7

/* states for master and slave processes */
#define INIT 1
#define IDLE 2
#define RECFILE 3
#define RECDATA 4
#define SENDINIT 5
#define SENDFILE 6
#define SENDDATA 7
#define SENDEOF 8
#define SENDCMD 9
#define SENDQUIT 10
#define STOP 11
#define RECINIT 12

/* data types in protocol */
/* each packet may or may not include additional data */
#define PACK 'Y'       /* ACKnowledge message */
#define PNAK 'N'       /* NotAcKnowledge message */
#define PINIT 'I'      /* system init */
#define PERR 'E'       /* system error */
#define PSSEQ 'X'      /* syncronize sequence numbers (reset to 0) */
#define PSEND 'S'      /* send init */
#define PREC 'R'       /* receive init */
#define PFILEH 'F'     /* file header */
#define PEOF 'Z'       /* end of file */
#define PDATA 'D'      /* data */
#define PQUIT 'Q'      /* terminate server */
#define PCMD 'C'       /* command */
#define P_CMPR_DATA 'G'

#define CR 0x0d
#define SOH 0x01
#define BLK_DATA_MAX 250
#define MAXERR 5
#define MAXNAK 5
#define MIN_LOAD_ADDR 0x20000L

/* OS9 C compiler dosn't like these prototypes */
#ifndef DT08
int asm_blk(int);
int send_blk(int);
int get_blk(unsigned char *);
#endif

struct comblk
  {
  unsigned char start,
       length,
       seqnr,
       type;
  unsigned char data[BLK_DATA_MAX];
  unsigned char checksum;  /* ATTENTION: don't access chechsum by name */
  } wblk;

unsigned char rblk[BLK_DATA_MAX+2];
unsigned char  in_seq_nr, out_seq_nr;

char *perrtab[]= {
		 "No error",
		 "General error",
		 "Read error",
		 "Read timeout",
		 "Write error",
		 "Packet sequence number error",
		 "Packet checksum error",
		 "Packet length error" };
