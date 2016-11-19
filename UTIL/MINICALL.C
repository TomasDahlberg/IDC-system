#include <stdio.h>

extern int pathIn, pathOut;

extern int errno;

char buf[400];			/* ??? */

extern int internalCounter;     /* next alarm text to send after request !!! */

char *mkpassw(pwd)
int pwd[];
{
  static char cpwd[10];
  int i;

  for (i = 0; i < 8; i++)
    cpwd[i] = pwd[i];
  cpwd[8] = 0;
  return cpwd;
}

char message[100];

int itoh(c)  /* 0-15 -> '0'-'9','A'-'F' */
int c;
{
  return (c < 10) ? '0' + c : 'A' + c - 10;
}

int make_chs(buf)
unsigned char *buf;
{
  int i, chs = 0;
  do {
    chs += *buf;
  } while (*buf++ != 0x03) ;		/* until ETX */
  chs &= 0xff;
  *buf++ = itoh(chs >> 4);
  *buf = itoh(chs & 0x0f);		/* CHS fel (+1) */
}

int execute_minicall(send_number, leg, send_password, phone_number, pris, typ,
	error)
int send_number, leg, send_password[], typ, *error;
char *phone_number;
double *pris;
{
  char data[25];
  int tries;
  int ipris;
  int n;

  internalCounter = 0;     /* restart counter */
  sleep(1);
  clear_up();		/* skip hello's */
  write(pathOut, "...\015", 4);
  if (!get_junk(1, 0, 0))
    return 2;
  tries = 5;
  do { 
    if (!makeAlarmText(message, typ)) {
/*      fprintf(stderr, "makeAlarmText error, tries = %d\n", tries); */
      tries --;

      internalCounter = 0;     /* restart counter */
      continue;
    }

    write(pathOut, "ACCESS MINICALL\015", 16);

    if (!get_junk(2, data, 25))		/* get junk and sign on message */
      return 3;
    if (data[1] != 'S' || data[2] != 'O')	/* Sign On */
      return 4;

    sprintf(buf, "%c%c%6d%s%s%03d%s%c**%c", 
			0x02 /* STX */, 'S', 
			send_number, 
			mkpassw(send_password),
			phone_number,
			leg,
			message, 
			0x03 /* ETX */, 
			0x17 /* ETB */);
    make_chs(buf);
    n = write(pathOut, buf, strlen(buf));
/*    write(pathOut, "\015", 1);		*/
/*
    io_error(pathOut);
    fprintf(stderr, "write = %d\n", n);
    fprintf(stderr, "errno = %d\n", errno);
*/
    if (!get_junk(2, data, 25))		/* get kvittens */
      return 5;
    if (data[1] == 'A' && data[2] == 'S') {	/* ACK, Ack Send */
      if (error) *error = 0;
      ipris = data[3] * 10 + data[4] - 528;
      if (pris) *pris += ipris;
      ipris = data[5] * 10 + data[6] - 528;
      if (pris) *pris += ipris / 100.0;
      ackLastAlarmRead();
    } else if (data[1] == 'N' && data[2] == 'A') {	/* NAK */
      if (error) *error = data[3] * 10 + data[4] - 528;
    } else
      return 17;

    if (!get_junk(1, 0, 0))
      return 19;
  } while (tries) ;
  return 1;
/*
  send_hang_up();
*/
}

int clear_up()
{
  char str[16];
  int n;

  while (1) {
    if ((n = _gs_rdy(pathIn)) > 0) {
      if (n > 15) n = 16;
      n = read(pathIn, str, n);
    } else
      break;
  }
  return 0;
}

int get_junk(typ, data, max_len)
int typ, max_len;
char *data;
{
  char str[16];
  int i, n, cnt;
  for (i = 0; i < 5; i ++) {
    while (1) {
      if ((n = _gs_rdy(pathIn)) > 0) {
        if (typ == 1) {
	  n = read(pathIn, str, 1);
          if (n == 1 && str[0] == '*') 
	    return 1;
        } else if (typ == 2) {
	  n = read(pathIn, str, 1);
          if (n == 1 && str[0] == 0x02) { /* STX */
/*  	    fprintf(stderr, "%d: [%d]\n", n, str[0]);	*/
	    typ = 3;
	    cnt = 1;
	    *data++ = str[0];
	  } else if (n == -1) {
/*	    fprintf(stderr, "Error: fel = %d\n", errno);
	    io_error(pathIn);
	    fprintf(stderr, "%d: [%d]\n", n, str[0]);	*/
          } else
		;
/*  	    fprintf(stderr, "%d: [%d]\n", n, str[0]);	*/
        } else if (typ == 3) {
	  n = read(pathIn, data, 1);
/*	  fprintf(stderr, "3:<%3d> ", *data);	*/
          if (n == 1) {
	    if (*data == 0x17) /* ETX */
	      return 1;
	    cnt ++;
	    if (cnt < max_len)
	      data++;
	  }
	}
      } else
	break;
    }
    sleep(1);
  }
  return (i < 5) ? 1 : 0;
}

