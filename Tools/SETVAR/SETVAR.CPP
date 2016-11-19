/* sendvar.cpp */
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
! Copyright (C) 1994, IVT Electronic AB.
*/

/*
!
!   History:
!   Date        by   rev   what
!   ----------  ---  ----  ---------------------------------------------
!   1994-02-28  MS   1.00  initial coding
!
!   Function:
!      Program to get and set vars in IVT16
!
*/


#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
/*
#include <bprint.h>
*/
#include "asynch_1.h"
#include "support.h"	       /* Support functions	  */
#include "dtproto.h"

#define BREAKCH  0x03   /* ^C */
#define OS9_PROMPT '$'
#define MAX_TRIAL 10

#define  BLANK    32
#define  LINEFEED 10
#define  ENTER	  13
#define  BACKSP    8
#define  ESCAPE   27
#define  F1	  59
#define  F2	  60
#define  F10      68
#define  BAR	 205
#define  NULBYTE '\0'
#define  ETX	 '\003'
#define  EOT	 '\004'
#define  PROMPT  '>'
#define  NORMAL    7

#define  TRUE	 -1
#define  FALSE	 0

struct _readData {
  varNamn[32];
};

struct _readData readData;

/*  Prototypes  */
int init_com(int);
void belldn(void);
void bellup(void);
void disp_help(void);
void disp_desc(void);
void disp_use(void);
int connect_dt08(int);
int start_recmod();

void main(int, char **);
int wr_ch(char);
int rd_ch(void);
void init_ports(void);
void errmsg(char *, int, unsigned);

void openReadFile();
void parse(char *, struct _readData *);
void skipWhiteSpace(char **);

/*int prinit(int);     /* macros */
/*int prchar(int, char);         */
/*int prstatus(int);             */

char versionst[] = "SENDVAR Version 1.00";  /* versionstring */
char line[81];			       /* Text line to be output  */
char st[81];
char *ibuffer,*obuffer;                /* Communication queues    */
int  in_row  = 6;		       /* Current row number of   */
int  in_col  = 3;		       /* the incoming messages.  */
int  out_row = 16;
int  out_col = 3;
int  in_port, out_port, port;
int baudopt;
FILE *fin;

FILE *fpReadfile;
char readfile[80];
int noOfLinesInReadfile;
_readData *pReadfile;

#include "dtproto.c"

void bellup()
  {
  int i;
  for (i=400; i< 1600; i+=100)
    {
    sound(i);
    delay(30);
    }
  nosound();
  }

void belldn()
  {
  int i;
  for (i=1600; i>100; i-=100)
    {
    sound(i);
    delay(20);
    }
  delay(100);
  nosound();
  }

#if 0
void prs(port,st)
int port;    /* LPT port number 0 or 1 */
char *st;
  {
  while (*st)
    prchar(port, *st++);
  }
#endif

/* WR_CH  Write a character to the output queue        */
/* returns 0 if OK, otherwise ER_WRITE */
int wr_ch(char ch)
{
  int ercode;

  while( (ercode = wrtch_a1(out_port, ch)) == OUT_Q_FULL)
    ;
  if( ercode != 0 ) {
    errmsg("Cannot write to the output queue (wrtch_a1).",ercode,0);
    return(ER_WRITE);
  }
  else return(0);
}

/* WR_ST  Write a string to the output queue        */
/* returns 0 if OK, otherwise ER_WRITE */
int wr_st(char *str)
{
  int stat;
  stat = 0;
  while( *str && (stat==0) )
    stat = wr_ch(*str++);
  return(stat);
}

/* returns 0 if OK, otherwise ER_WRITE */
int wr_blk(nof)
int nof;
  {
  int stat;
  unsigned char *pblk;
  pblk= &wblk.start;
  stat=0;
  while ((nof-- > 0) && (stat==0))
    stat= wr_ch(*pblk++);
  return(stat);
  }

/* assemble and send block, don't wait for respons */
/* type: packet type */
/* pstr: pointer to null terminated string */
/* returns error from wr_blk() */
int as_blk(type, pstr)
char type;
char *pstr;
  {
  wblk.type= type;
  strcpy(&wblk.data[0], pstr);
  return(wr_blk(asm_blk(strlen(&wblk.type))));
  }


/* RD_CH  Read a character from the input queue.   */
/* Character is returned, if empty or error -1 is returned  */
/* the character (or instruction) is echoed in the receive window.*/

int rd_ch()
  {
  int ercode,iqsize;
  unsigned status;
  unsigned char ch;

  ercode = rdch_a1(in_port, &ch, &iqsize, &status);
  if (status & 0x00ff)
      errmsg("Error accessing the input queue (rdch_a1).",0,status);
  if (ercode == IN_Q_EMPTY)          /* Nothing is in queue	  */
      return(-1);
  switch (ercode)
    {
    case 0:			       /* Character received	  */
      return(ch);
    default:
      errmsg("Error reading input port (rdbk_a1).",ercode,status);
      return (ER_READ);
    }
  }

/* COMPORT_INIT  Initialize the COM port(s).             */

int init_com(int port)
{
  int ercode, baud_rate, data_bits, parity, stop_bits;
  static char *pbaud[]   = {"110","150","300","600","1200","2400","4800","9600","19200"};
  static char *pparity[] = {"No","Odd","Even"};
  static char *pdata[]   = {"Five","Six","Seven","Eight"};
  static char *pstop[]   = {"One","Two"};

/*   scdspmsg(2,26,15,0,"I N I T I A L I Z A T I O N");
*/   ercode = -1;
   while (ercode)
   {
/*      scscroll(4,0,24,79,0,NORMAL);
      sccurset(4,0);
      sccursor(0,12,13);  */
/*      in_port   = aprdnum("    Receiving Port");
      out_port  = aprdnum(" Transmission Port");  */
      in_port= port;
      out_port= port;
      if ((ibuffer = malloc(16384)) == NIL)
	 errmsg("Cannot allocate space for input buffer.",0,0);
      if ((ercode = open_a1(in_port,15250,1024,0,0,ibuffer)) != 0)
	 errmsg("Cannot open input port.",ercode,0);
      if (in_port != out_port)
      {
	 if ((obuffer = malloc(260)) == NIL)
	    errmsg("Cannot allocate space for output buffer.",0,0);
	 if ((ercode = open_a1(out_port,128,128,0,0,obuffer)) != 0)
	    errmsg("Cannot open output port.",ercode,0);
      }
   }
   ercode = -1;
   while (ercode)
   {
/*      printf(" BAUD Rate   110  150  300  600 1200 2400 4800 9600 19200\n");
      printf(" Code          0    1    2    3    4    5    6    7     8\n");
      baud_rate = aprdnum("         BAUD Rate");
*/
      if (baudopt)
	  baud_rate= baudopt;
	else
	  baud_rate = 7;    /* default 9600 */
      if ((ercode = setop_a1(in_port,1,baud_rate)) != 0)
	 errmsg("Cannot set BAUD rate for the input port.",ercode,0);
/*      else if ((ercode = setop_a1(out_port,1,baud_rate)) != 0)
	 errmsg("Cannot set BAUD rate for the output port.",ercode,0);
*/   }
   ercode = -1;
   while (ercode)
   {
/*      printf(" Parity      None  Odd   Even\n");
      printf(" Code          0    1      2\n");
      parity	= aprdnum("            Parity");
*/     parity= 0;
     if ((ercode = setop_a1(in_port,2,parity)) != 0)
	 errmsg("Cannot set Parity for the input port.",ercode,0);
/*      else if ((ercode = setop_a1(out_port,2,parity)) != 0)
	 errmsg("Cannot set Parity for the output port.",ercode,0);
*/   }
   ercode = -1;
   while (ercode)
   {
/*      printf(" Data bits     5    6    7    8\n");
      printf(" Code          0    1    2    3\n");
      data_bits = aprdnum("         Data bits");
*/   data_bits = 3;
   if ((ercode = setop_a1(in_port,3,data_bits)) != 0)
	 errmsg("Cannot set Data Bits for the input port.",ercode,0);
/*      else if ((ercode = setop_a1(out_port,3,data_bits)) != 0)
	 errmsg("Cannot set Data Bits for the output port.",ercode,0);
*/   }
   ercode = -1;
   while (ercode)
   {
/*      printf(" Stop bits     1    2\n");
      printf(" Code          0    1\n");
       stop_bits = aprdnum("         Stop bits");
*/   stop_bits= 0;
      if ((ercode = setop_a1(in_port,4,stop_bits)) != 0)
	 errmsg("Cannot set Stop Bits for the input port.",
						       ercode,0);
/*      else if ((ercode = setop_a1(out_port,4,stop_bits)) != 0)
	 errmsg("Cannot set Stop Bits for the output port.",
						       ercode,0);
*/   }
   if ((ercode = setop_a1(out_port,9,0)) != 0)
      errmsg("Cannot remove requirement for CTS",0,0);
/*   if ((ercode = setop_a1(out_port,5,1)) != 0)
      errmsg("Cannot enable remote Flow Control (XON/XOFF)",0,0);
*/
   printf(" Communication port COM%d\n", out_port);
   printf(" BAUD rate is %s    %s parity    %s data bits    %s stop bit(s)\n",
			    pbaud[baud_rate],pparity[parity],
			    pdata[data_bits],pstop[stop_bits]);
/*   printf(" Transmit flow control with XON/XOFF.   "); */
   printf(" CTS is not required.\n");

   return(0);

}

int setBps(int port, int baud_rate)
{
  int ercode;

  if ((ercode = setop_a1(port,1,baud_rate)) != 0)
    errmsg("Cannot set BAUD rate for the input port.",ercode,0);
  return ercode;
}

/* ERRMSG  Display an error message, error code and port status   */
/* if they are nonzero.  The bottom four lines of the transmit	  */
/* window are used to display the error messages.		  */

void errmsg(pmsg,code,status)
char	 *pmsg;
int	 code;
unsigned status;
{

   int row,col,tcur,bcur;
   static char *pcodemsg[] =
	{"Successful",
	 "Reserved for future use",
	 "Invalid COM port number",
	 "COM port is not opened",
	 "Invalid parameter or function value",
	 "Reserved for future use",
	 "No serial port found",
	 "Output queue is full",
	 "Reserved for future use",
	 "COM port is already open",
	 "Input queue is empty"};

   sccurst(&row,&col,&tcur,&bcur);     /* Current cursor position */
   sccursor(1,0,0);		       /* Turn the cursor off	  */
   sccurset(21,0);		       /* Position the cursor,	  */
   printf(" %s\n",pmsg);               /* display the message,    */
   if (code != 0)		       /* and possibly the code.  */
      printf(" Error code - %02d  %s",code,pcodemsg[code]);
   if (status != 0)
      printf(" Port status - %04x",status);
   printf("\n");
/*   utpause("");                        /* Wait for a key to be    */
   sccursor(0,12,13);              /* pressed. Turn the cursor*/
   scscroll(21,0,24,79,0,NORMAL);      /* back on and clear the   */
   sccurset(row,col);		       /* window and go back.	  */

   return;
  }

void printCopyright()
{
  static char *str = "Copyright 1991-1993 IVT Electronic AB";
  static char text[] = {
	  37, 82, 61, 77, 52, 70, 47, 72, 32, 84, 116,
	  69, 124, 69, 116, 89, 104, 81, 104, 91, 123,
	  50, 100, 48, 16, 85, 57, 92, 63, 75, 57, 86,
	  56, 81, 50, 18, 83, 17 };

  int seed = 17;
  int len;

  str = text;
  len = *str++;
  for (;len; len--) {
    printf("%c", *str ^ seed);
    seed = *str++;
  }
  printf("\n");
}

void disp_help()
{
  disp_desc();
  disp_use();
}

void disp_desc()
{
  printf("SENDMOD loads program to IVT16\n");
  printf("  Binary data is transfered over the serial line\n");
  printf("  SENDMOD set the lineparameters to 9600,8,1,n and no RTS required\n");
  printf("  Program RECMOD must reside in IVT16 module directory\n");
  printf("  and a shell must be running on the IVT16 communication port\n");
  printf("\n");
}

void disp_use()
{
  printCopyright();
  printf("Use: SENDVAR readfile writefile [-option]\n");
  printf("         readfile: filname for varlist\n");
  printf("         writefile: filename for backupfile)\n");
  printf("Options:\n");
  printf("         [-2]    : Use COM2 for transfer, default is COM1\n");
  printf("         [-bn]   : Set baudrate n=4(1200)..8(19200), default is 9600\n");
  printf("         [-?]    : Show this list\n");
}

/* returns 1 if connected otherwise 0 */

int connect_dt08(port)
int port;
  {
  int connected, i, ercode, size;
  unsigned int status;
  char ch;
  connected= 0;
  iflsh_a1(port);            /* flush input queue */
/*  wrtch_a1(port, BREAKCH);   /* send ^C */
  wrtch_a1(port, CR);   /* send CR */
  for (i=0; i< MAX_TRIAL; i++)
    {
    do
      {
      delay(100);    /* sleep .1 seconds */
      ercode= rdch_a1(port, &ch, &size, &status);
      if ((ercode==0) && (ch== OS9_PROMPT))
	  {
	  connected= 1;
	  break;
	  }
      } while (size && !ercode);
    if (connected) break;
    wrtch_a1(port, CR);   /* send a new CR */
    }
  iflsh_a1(port);            /* flush input queue */
  return(connected);
  }


/* returns >0 if RECMOD started, 0 otherwise */
#if 0
int start_recmod()
{
  int stat;
  if (optionN)
     wr_st("new\r");
  wr_st("RECMOD\r");
  stat= get_blk(rblk);      /* wait for a packet from RECMOD */
  if (stat> 0)
      printf("%s\n", rblk[1]);   /* show message from RECMOD */
  return (stat);
}
#endif

int get_ch()
  {
  int ret;
  long li;
  for (li=0; li<50000; li++)
    {
    if ((ret= rd_ch())>= 0) return (ret);
    if (ret== ER_READ) return (ER_READ);   /* ERROR in read */
/*    delay(1);  */
    }
  return(ER_TIMOUT);    /* timeout */
  }

void pexit()
{
  close_a1(port);
  fclose(fin);
  exit(0);
}

void perr_exit()
{
  printf("Fatal error! Terminating process!\n\n");
  belldn();
  pexit();
}

void openReadFile()
{
  int lines;
  _readData *p;
  char str[256];

  if( (fpReadfile = fopen(readfile, "r") ) == NULL ) {
    printf("cannot create file '%s'", readfile);
    pexit(0);
  }
  noOfLinesInReadfile = 0;
  while( fgets(str, 255, fpReadfile) ) {
    if( str[0] == '*' )
      continue;
    noOfLinesInReadfile++;
  }
  pReadfile = (_readData *) malloc( noOfLinesInReadfile * sizeof(_readData) );
  if( pReadfile == 0 ) {
      printf("cannot allocate %d bytes", noOfLinesInReadfile * sizeof(_readData));
      pexit(0);
  }
  fclose( fpReadfile );

  if( (fpReadfile = fopen(readfile, "r") ) == NULL ) {
    printf("cannot create file '%s'", readfile);
    pexit(0);
  }
  lines = 0;
  while( fgets(str, 255, fpReadfile) ) {
    if( str[0] == '*' )
      continue;
    if( pReadfile != 0 ) {
      p = &pReadfile[lines];
      parseVp(str, p);
      lines++;
    }
  }
}

void parse(char *str, _readData *p)
{
  skipWhiteSpace(&str);
  strcpy(p->varNamn, str);
}

void skipWhiteSpace(char **s)
{
  while (isspace(**s))
	(*s) ++;
}
/*
void copyUntil(char *namn, char **s, char sep)
{
  char c;
  while ((c = *namn = **s) && (c != sep)) {
    if (c == '\t')
      *namn = ' ';
    (*s)++;
    namn++;
  }
  *namn = 0;
  (*s)++;
}
*/

// sendvar readfile writefile comport baudrate

void main(int argc, char *argv[])
{
  int i, status, ich, errcnt, type;
  int state, mlen, nakcnt, bcnt, ready, receive = 0, highSpeed = 0;
  unsigned char scan;
  long lcnt, laddr;
  char filn[40];
  int c1, node = 0;
  int optionN = 0, optionR = 0, optionC = 0;
  int optionF = 0;
  int optionFF = 0;
  int optionI = 0;
  int optionOldMode = 0;
  int optionState, blkDataMax;
  char iplBuf[100];

  port = 1;
  lcnt= laddr= 0L;
  errcnt= ready= 0;

  printCopyright();
  printf("%s\n", versionst);
  if( (argc < 2) || (argc > 5) ) {
    disp_help();
    pexit();
  }

  baudopt= 0;
  while( argc >= 2 ) {
    if( *argv[1] != '-' ) {
      strcpy(readfile, argv[1]);
      strcpy(writefile, argv[2]);
    } else {
      while( *++(argv[1]) ) {
	switch( *argv[1] ) {
	  case '?':
	    disp_use();
	    pexit();
	  case '1':
	    port = 1;
	    continue;
	  case '2':
	    port = 2;
	    continue;
	  case 'b':
	  case 'B':
	    baudopt = 999;
	    if( argv[1][1]> '0' && argv[1][1]<= '9') {
	      baudopt= argv[1][1] - 0x30;
	      argv[1]++;
	      if( argv[1][1]>= '0' && argv[1][1]<= '9') {
		baudopt= 10*baudopt + argv[1][1] - 0x30;
		argv[1]++;
	      }
	    }
	    if (baudopt > 8) {
	      disp_use();
	      pexit(0);
	    }
	    continue;
	  default:
	    printf("illegal option: %c\n", (char *) *argv[1]);
	    disp_help();
	    pexit();
	} /* end switch */
      } /* end while */
    } /* end if '-' */
    argv++;
    argc--;
  }

  if( init_com(port) ) {
    printf("COM%d not available\n", port);
    pexit(0);
  }

  openReadFile();

  if( (fpWritefile = fopen(writefile, "w") ) == NULL ) {
    printf(buf, "cannot create file '%s'", writefile);
    pexit(0);
  }
  while(1) {
    int ch;
    lines = 0;
    for(i=0; i<noOfLinesInReadfile; i++ ) {
      p = &readData[lines];
      wr_st("000");        // nodenummer
      wr_st(" ");          // space
      wr_st("001");
      wr_st(" ");          // space
      wr_st(p->varNamn);
      wr_ch(0xd);
      wr_ch(0xa);
      while( (ch = get_ch()) != 0xa )
	printf("%d", ch)
      printf("\n");
    }
  }
}
