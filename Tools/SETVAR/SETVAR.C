/* setvar.c */
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
!      Program to get vars in IVT16
!
*/

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <time.h>
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

#define LF 0x0A
#define TIMEOUT_TIME 10   // tid mellan f”rfr†gningar

char VERSION[] = "Setvar version 1.00";  /* versionstring */

/*  Prototypes  */
int init_com(int);
void belldn(void);
void bellup(void);
void disp_help(void);
void disp_desc(void);
void disp_use(void);
/*
int connect_dt08(int);
int start_recmod();
*/
void main(int, char **);
int wr_ch(unsigned char);
int rd_ch(void);
void init_ports(void);
void errmsg(char *, int, unsigned);

void skipWhiteSpace(unsigned char **);
void copyUntil(unsigned char *, unsigned char **, unsigned char);
int  readInputString();
void writeString(unsigned char *str);

/*int prinit(int);     /* macros */
/*int prchar(int, char);         */
/*int prstatus(int);             */

//char line[81];			       /* Text line to be output  */
//char st[81];
char *ibuffer,*obuffer;                /* Communication queues    */
//int  in_row  = 6;		       /* Current row number of   */
//int  in_col  = 3;		       /* the incoming messages.  */
//int  out_row = 16;
//int  out_col = 3;
int  in_port, out_port, port;
int baudopt;
//FILE *fin;

FILE *fpReadfile;
char readfile[80];
int DEBUG;
char var[80];

#include "dtproto.c"

/* WR_CH  Write a character to the output queue        */
/* returns 0 if OK, otherwise ER_WRITE */
int wr_ch(unsigned char ch)
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
int wr_st(unsigned char *str)
{
  int stat;
  stat = 0;
  while( *str && (stat==0) )
    stat = wr_ch(*str++);
  return(stat);
}
/*
 rd_ch Read a character from the input queue.
 Character is returned, if empty or error -1 is returned
 the character (or instruction) is echoed in the receive window.
*/
int rd_ch()
{
  int ercode,iqsize;
  unsigned status;
  unsigned char ch;

  ercode = rdch_a1(in_port, &ch, &iqsize, &status);
  if (status & 0x00ff)
      errmsg("Error accessing the input queue (rdch_a1).", 0, status);
  if (ercode == IN_Q_EMPTY)          /* Nothing is in queue	  */
      return(-1);
  switch( ercode ) {
    case 0:			       /* Character received	  */
      return( ch );
    default:
      errmsg("Error reading input port (rdbk_a1).", ercode, status);
      return (ER_READ);
  }
}

void printCopyright()
{
  static char *str = "Copyright 1994 IVT Electronic AB";
  static char text[] = {
  0x20, 0x52, 0x3d, 0x4d, 0x34, 0x46, 0x2f, 0x48, 0x20, 0x54, 0x74, 0x45,
  0x7c, 0x45, 0x71, 0x51, 0x18, 0x4e, 0x1a, 0x3a, 0x7f, 0x13, 0x76, 0x15,
  0x61, 0x13, 0x7c, 0x12, 0x7b, 0x18, 0x38, 0x79, 0x3b
};

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
//  printf("Setvar\n");
}

void disp_use()
{
  printf("Use: Setvar readfile [-option]\n");
  printf("         readfile: filname for variablelist\n");
  printf("Variablelist should look like this:\n");
  printf("level = 1\n");
  printf("level1 = 1111\n");
  printf("level2 = 2222\n");
  printf("and so on...\n");
  printf("If you want nodenumber, put the nodenumber before variablename.\n");
  printf("Ex.\n");
  printf("2level3 = 3333\n");
  printf("Sets level3 to 3333 in node 2.\n");
  printf("Options:\n");
  printf("         [-2]    : Use COM2 for transfer, default is COM1\n");
  printf("         [-bn]   : Set baudrate n=4(1200)..8(19200), default is 9600\n");
  printf("         [-?]    : Show this list\n");
}

int get_ch()
{
  int ret;
  long li;
  time_t now;

  now = time(NULL);
  while( 1 ) {
    if( (ret= rd_ch()) >= 0)
      return( ret );
    if( ret== ER_READ )
      return( ER_READ );       // ERROR in read
    if( difftime(time(NULL), now) > TIMEOUT_TIME )
      return(ER_TIMOUT);       // timeout
  }
/*
  for (li=0; li<50000; li++)  {
    if( (ret= rd_ch()) >= 0)
      return( ret );
    if( ret== ER_READ )
      return( ER_READ );   // ERROR in read
  }
  return(ER_TIMOUT);     // timeout
*/
}

void pexit()
{
  close_a1( port );
  fclose( fpReadfile );
  exit(17);
}

void perr_exit()
{
  printf("Fatal error! Terminating process!\n\n");
  belldn();
  pexit();
}

void skipWhiteSpace(unsigned char **s)
{
  while (isspace(**s))
	(*s) ++;
}

void copyUntil(unsigned char *namn, unsigned char **s, unsigned char sep)
{
  char c;
  while ((c = *namn = **s) && (c != sep)) {
    if (c == '\t')
      *namn = ' ';
    (*s)++;
    namn++;
  }
  *namn = 0;
  if( **s != NULL )
    (*s)++;
}

void main(int argc, char *argv[])
{
  unsigned char buf[80], *str[80], *p;
  static unsigned char outputString[256];
  int i, status;
  static int cnt;

  port = 1;

  printCopyright();
  printf("%s\n", VERSION);
  if( (argc < 2) || (argc > 3) ) {
    disp_help();
    exit(2);
  }

  baudopt= 0;
  DEBUG = 0;
  while( argc >= 2 ) {
    if( argv[1][0] != '-' ) {
      strcpy(readfile, argv[1]);
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
	  case 'd':
	  case 'D':
	    DEBUG = 1;
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
	    exit(3);
	} /* end switch */
      } /* end while */
    } /* end if '-' */
    argv++;
    argc--;
  }

  if( init_com(port) ) {
    printf("COM%d not available\n", port);
    close_a1(port);
    exit(4);
  }

  if( (fpReadfile = fopen(readfile, "r") ) == NULL ) {
    printf("cannot open file '%s'", readfile);
    pexit(0);
  }

  str[0] = (unsigned char *) malloc(80);
  if( str[0] == NULL ) {
    printf("Not enough memory to allocate buffer\n");
    pexit();
  }
  while( 1 ) {
    strcpy(outputString, "0 " );              // nollst„ll str„ng
    if( fgets(str[0], 80, fpReadfile) ) {
      skipWhiteSpace(&str);
      copyUntil(var, &str, ' ');
      (*str)--;
      copyUntil(buf, &str, LF);
      strcat(outputString, var);
      strcat(outputString, buf);
      writeString(outputString);
      if( DEBUG ) printf("%s\n", outputString);
      cnt = 0;
      while( (status = readInputString()) < 1) {
	if( status == ER_TIMOUT ) {
	  if( DEBUG ) printf("%s\n", outputString);
	  writeString(outputString);
	  cnt++;
	  if( cnt > 4 ) {
	    free(str[0]);
	    pexit();
	  }
	  continue;
	} else if( status == ER_READ ) {
	  printf("error reading port %d\nterminating program", port);
	  free(str[0]);
	  pexit();
	}
      }
    } else
      break;
  } // end while
  free(str[0]);
  pexit();
}

void writeString(unsigned char *str)
{
  wr_st(str);
  wr_ch(CR);
  wr_ch(LF);
}

int readInputString()
{
  int ch;
  unsigned char inputString[256], buf[80];

  strcpy(inputString, "\0");           // nollst„ll str„ng

  while( (ch = get_ch()) != LF ) {
    if( ch == ER_TIMOUT) {
      if( DEBUG ) printf("timeout\n");
      return ER_TIMOUT;
    } else if( ch == ER_READ ) {
      if( DEBUG ) printf("error read\n");
      return ER_READ;
    } else if( ch > 0 ) {
      if( ch != CR ) {
	sprintf(buf, "%c", ch);
	strcat(inputString, buf);
      }
    }
  }
  if( DEBUG ) printf("inputString= '%s'\n\n", inputString);
  if(strncmp(inputString, "ACK", 3)  == 0) {
    if( !DEBUG )  printf(".");
    return 1;
  }
  if(strncmp(inputString, "NAK", 3) == 0) {
    if( !DEBUG )  printf("\n'%s' is not update\n", var);
    return 2;
  }
  return 3;
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
   printf("Communication port COM%d\n", out_port);
   printf("BAUD rate is %s    %s parity    %s data bits    %s stop bit(s)\n",
			    pbaud[baud_rate],pparity[parity],
			    pdata[data_bits],pstop[stop_bits]);
/*   printf(" Transmit flow control with XON/XOFF.   "); */
   printf("CTS is not required.\n");
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
void errmsg(char *pmsg,int code,unsigned status)
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
void bellup()
{
  int i;
  for (i=400; i< 1600; i+=100) {
    sound(i);
    delay(30);
  }
  nosound();
}
void belldn()
{
  int i;
  for (i=1600; i>100; i-=100) {
    sound(i);
    delay(20);
  }
  delay(100);
  nosound();
}
/* returns 0 if OK, otherwise ER_WRITE */
int wr_blk(int nof)
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
int as_blk(char type, char *pstr)
{
  wblk.type= type;
  strcpy(&wblk.data[0], pstr);
  return(wr_blk(asm_blk(strlen(&wblk.type))));
}

