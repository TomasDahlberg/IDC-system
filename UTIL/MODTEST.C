/* modtest.c  1992-05-05 TD,  version 1.0 */
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
! modtest.c
! Copyright (C) 1992, IVT Electronic AB.
*/


/*
!   modtest  - a facility to test an arbitrary module configuration
! 
!   History:
!   Date        by   rev   ed#  what
!   ----------  ---  ----  ---  ---------------------------------------------
!   1992-05-05  TD   1.00  1    initial coding
!
!   Function: 
!   Presentates a continously update the told configuration
!
*/
@_sysedit: equ 1
@_sysattr: equ $8003

#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <math.h>
#include <ctype.h>
#include <module.h>
#include <sgstat.h>

char *dm;
char *meta;
char *headerPtr1, *headerPtr2, *headerPtr3, *headerPtr4;

double ANAIN_1();
double TEMP_1();
double Ni1000LG();
double Ni1000DIN();

#define TYPE_INT    7
#define TYPE_FLOAT  8

#define NAMEOFDATAMODULE  "VARS"

#define TYP_DIGIN_1   1
#define TYP_DIGOUT_1  2
#define TYP_ANAIN_1   3
#define TYP_ANAOUT_1  4
#define TYP_TEMP_1    5
#define TYP_PULS_2    6

#define FILT_NI1000LG  1
#define FILT_NI1000DIN 2
#define FILT_NI100LG   3
#define FILT_NI100DIN  4
#define FILT_PT1000LG  5
#define FILT_PT1000DIN 6
#define FILT_PT100LG   7
#define FILT_PT100DIN  8

struct { 
  char typ;      /* 0 = ai, 1 = au etc */
  struct _ch {
    union {
      float ai;
      char  di;
      float temp;
      long puls;
    } value;
    char typ, changed;
  } ch[8];
} moddef[16];

int Abort()
{
  unlinkDataModule(headerPtr1);
  unlinkDataModule(headerPtr2);
  enableXonXoff(0);
  exit(0);
}

void icp(s)
int s;
{
  if (s == 257)
    return ;
    
  gotoxy(0, 21);
  Abort();
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

visualAttribute(c)
int c;
{
  char buff[10];
  sprintf(buff, "%c[%dm", 27, c);
  write(1, buff, 4);
}

normal() {  visualAttribute(0); }
bold() {  visualAttribute(1); }
blinking() {  visualAttribute(5); }
inverse() {  visualAttribute(7); }

clearScreen()
{
  static char buf[4] = { 27, 91, '2', 'J' };
  write(1, buf, 4);
}


/*
!   gives a hint of usage of this program
*/
void usage()
{
  printf("Syntax: modtest [<opts>] {<type>}\n");
  printf("Function: show module data according to told configuration\n");
  printf("Options:\n");
  printf("     -?           shows this list\n");
  printf("Types are separated by spaces and are the types for I/O slot 1 to 16\n");
  printf("Legal types are:\n");
  printf("0  - empty slot\n");
  printf("1  - DIGIN_1 module\n");
  printf("2  - DIGOUT_1 module\n");
  printf("3  - ANAIN_1 module\n");
  printf("4  - ANAOUT_1 module\n");
  printf("5  - TEMP_1 module\n");
  printf("6  - PULS_1 module\n");
  printf("\n");
  printf("E.g.\n");
  printf("modtest 1 2 3 4 5\n");
  printf("\n");
}

int EXP = 0;

main(argc, argv)
int argc;
char *argv[];
{
  int mod, ch;

  for (mod = 0; mod < 16; mod++) {
    moddef[mod].typ = 0;
    for (ch = 0; ch < 8; ch++) {
      moddef[mod].ch[ch].value.puls = 0;
      moddef[mod].ch[ch].changed = 1;
    }
  }

  mod = 0;
    
  while( argc >= 2 ) {
    if (argv[1][0] == '-' )
    {
        char c;
	while( *++(argv[1]) ) {
	    switch( *argv[1] ) {
                case '?':
                    usage();
                    exit(0);
		default:
		    fprintf(stderr, "illegal option: %c", (char *) *argv[1]);
                    exit(0);
		}
	    }
	argv++;
	argc--;
    } else {
/*      strcpy(pattern[pattCnt++], argv[1]);    */


      moddef[mod].typ = atoi(argv[1]);
      if (moddef[mod].typ == TYP_TEMP_1) {
        int ch;
        for (ch = 0; ch < 8; ch++)
          moddef[mod].ch[ch].typ = FILT_NI1000LG;
      }
      if (mod > 7)
        EXP = 1;
      mod++;

      argv++;
      argc--;
    }
  }

  intercept(icp);
  initidcio();
  initphyio();
/*
  bind(&dm, &meta, &headerPtr1, &headerPtr2);
  if (!(dm && meta)) {
    exit(0);
  }
*/
  disableXonXoff(0);
  clearScreen();
 
  displayLayout();
  while (1) {
    updateScreen();
    if (_gs_rdy(0) > 0) {
      char c;
      read(0, &c, 1);
      if (c == 'S') {
/*
        getConfigure();
*/
      }
    }
  } 
}


getName(mod, buf)
int mod;
char *buf;
{
  if (moddef[mod].typ == TYP_DIGIN_1)
    strcpy(buf, "DIGIN_1");
  else if (moddef[mod].typ == TYP_DIGOUT_1)
    strcpy(buf, "DIGOUT_1");
  else if (moddef[mod].typ == TYP_ANAIN_1)
    strcpy(buf, "ANAIN_1");
  else if (moddef[mod].typ == TYP_ANAOUT_1)
    strcpy(buf, "ANAOUT_1");
  else if (moddef[mod].typ == TYP_TEMP_1)
    strcpy(buf, "TEMP_1");
  else if (moddef[mod].typ == TYP_PULS_2)
    strcpy(buf, "PULS_2");
  else
    strcpy(buf, "N/A");
}

displayLayout()
{
  int mod, ch, x, y;
  char buf[20];
  
  for (mod = 0; mod < (EXP ? 16 : 8); mod++) {
    getName(mod, buf);
    x = 4+9*(mod & 7);
    y = (mod < 8 ? 3 : 14);
    gotoxy(x, y);
    print(buf);
  }
  x = 0;
  for (ch = 0; ch < 8; ch ++) {
    y = 4 + ch;
    sprintf(buf, "ch%d", ch);
    gotoxy(x, y);
    print(buf);
    if (EXP) {
      y += 11;
      gotoxy(x, y);
      print(buf);
    }
  }
}


updateScreen()
{
  int mod, ch, x, y;
  char buf[20];
  
  for (mod = 0; mod < (EXP ? 16 : 8); mod++) {
    if (moddef[mod].typ == 0)
      continue;
    if (moddef[mod].typ == TYP_DIGIN_1)
      readModule(mod, 0);
    for (ch = 0; ch < 8; ch ++) {
      if (moddef[mod].typ == TYP_PULS_2 && ch >= 4)
        break;
      if (moddef[mod].typ != TYP_DIGIN_1)
        readModule(mod, ch);
      if (moddef[mod].ch[ch].changed) {
        writeValue(mod, ch, buf);
        x = 4+9*(mod & 7);
        y = (mod < 8 ? 4 : 15) + ch;
        gotoxy(x, y);
        rAdjustBuf(buf, 8);
        print(buf);
        moddef[mod].ch[ch].changed = 0;
      }
    }
  }
}

rAdjustBuf(buf, p)
char *buf;
int p;
{
  char *lp1, *lp2;
  int l;

  l = strlen(buf) + 1;  
  lp1 = buf + l - 1;
  lp2 = buf + p;
  while (l--)
    *lp2-- = *lp1--;
  while (lp2 >= buf)
    *lp2-- = ' ';
}

writeValue(mod, ch, buf)
int mod, ch;
char *buf;
{
  int i, j;
  float value;
  switch (moddef[mod].typ) {
    case TYP_DIGIN_1:
      value = moddef[mod].ch[ch].value.di;
      break;
    case TYP_DIGOUT_1:
      value = 0;
      break;
    case TYP_ANAIN_1:
      value = moddef[mod].ch[ch].value.ai;
      break;
    case TYP_ANAOUT_1:
      value = 0;
      break;
    case TYP_TEMP_1:
      value = moddef[mod].ch[ch].value.temp;
      break;
    case TYP_PULS_2:
      value = moddef[mod].ch[ch].value.puls;
      break;
    default:
      value = 0;
      break;
  }
  sprintf(buf, "%g", value);
}

readModule(mod, ch)
int mod, ch;
{
  int i, j, value;
  double fvalue;
  
  switch (moddef[mod].typ) {
    case TYP_DIGIN_1:
      value = get_di(mod+1);
      for (i = 0, j = 1; i < 8; i++, j <<= 1)
        if (moddef[mod].ch[i].value.di != ((value & j) ? 1 : 0)) {
          moddef[mod].ch[i].value.di = ((value & j) ? 1 : 0);
          moddef[mod].ch[ch].changed = 1;
        }
      break;
    case TYP_DIGOUT_1:
      break;
    case TYP_ANAIN_1:
      fvalue = ANAIN_1(0.0, mod+1, ch+1, 0);
      if (moddef[mod].ch[ch].value.ai != fvalue) {
        moddef[mod].ch[ch].value.ai = fvalue;
        moddef[mod].ch[ch].changed = 1;
      }
      break;
    case TYP_ANAOUT_1:
      break;
    case TYP_TEMP_1:
      fvalue = TEMP_1(0.0, mod+1, ch+1, 0);
      switch(moddef[mod].ch[ch].typ) {
        case FILT_NI1000LG:
          fvalue = Ni1000LG(0.0, fvalue, 0.0);
          break;
        case FILT_NI1000DIN:
          fvalue = Ni1000DIN(0.0, fvalue, 0.0);
          break;
        default:
          fvalue = 0;
          break;
      }
      if (moddef[mod].ch[ch].value.temp != fvalue) {
        moddef[mod].ch[ch].value.temp = fvalue;
        moddef[mod].ch[ch].changed = 1;
      }
      break;
    case TYP_PULS_2:
      value = PULS_2(moddef[mod].ch[ch].value.puls, mod+1, ch+1, 0, 0);
      if (moddef[mod].ch[ch].value.puls != value) {
        moddef[mod].ch[ch].value.puls = value;
        moddef[mod].ch[ch].changed = 1;
      }
      break;
  }
}


bind(dm, meta, headerPtr1, headerPtr2)
char **dm, **meta, **headerPtr1, **headerPtr2;
{
/*
!   bind to data module VARS, storage location for variables
*/  
  *dm = (char *) linkDataModule(NAMEOFDATAMODULE, headerPtr1);
  if (!*dm) {
    fprintf(stderr, "cannot link to datamodule '%s'\n", NAMEOFDATAMODULE);
    fprintf(stderr, "check if process 'scan' is running\n");
    return 0;
  }
/*
!   bind to data module META, storage for meta description of VARS module
*/
  *meta = (char *) linkDataModule("METAVAR", headerPtr2);
  if (!*meta) {
    fprintf(stderr, "cannot link to datamodule '%s'\n", "METAVAR");
    fprintf(stderr, "check if process 'scan' is running\n");
    return 0;
  }
}

struct sgbuf copy;
enableXonXoff(path)
int path;
{
  if (_ss_opt(path, &copy) == -1) {
    fprintf(stderr, "error during _ss_opt: %d\n", errno);
    exit(errno);
  }
}


disableXonXoff(path)
int path;
{
  struct sgbuf buffer;
  
  if (_gs_opt(path, &buffer) == -1) {
    fprintf(stderr, "error during _gs_opt: %d\n", errno);
    exit(errno);
  }
  memcpy(&copy, &buffer, sizeof(struct sgbuf));
  buffer._sgm._sgs._sgs_xon   = 0;
  buffer._sgm._sgs._sgs_xoff  = 0;
  buffer._sgm._sgs._sgs_echo  = 0;
  buffer._sgm._sgs._sgs_kbach = 0;      /* keyboard abort character */
  
  if (_ss_opt(path, &buffer) == -1) {
    fprintf(stderr, "error during _ss_opt: %d\n", errno);
    exit(errno);
  }
}

