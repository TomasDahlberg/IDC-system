/* lcd.c  1993-01-21 TD,  version 1.5 */
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
! lcd.c
! Copyright (C) 1991-1993 IVT Electronic AB.
*/



/* 
!  DT-08 drivrutin (C)
!  1990-10-06
!
!  IVT Electronic has added functions such as;
!
!     - routines for reading keyboard; keyDown(), key() and getKey()
!     - display contents cached and only updated when neseccery
!     - lcdwrite takes care of control characters formfeed and newline
!     - implemented scroll of display
!     - converts swedish characters
!     - implemented user defined characters
!     - keeps control of x and y coordinates
!     - lcdprintf changed to pop arguments from stack, to make floats work ?!?
!     - lcdprintf handles writing bitpatterns using arrows
!     - control of led's
!     - user defined characteristics, WRAP, CACHE and CACHE_CURSOR
!
! 1991-01-24
!   
!     - reorder of keys, added keys:  KEY_CHANGE, KEY_FUNCTION, KEY_PLUSMINUS,
!                                     and KEY_POINT
! 1992-02-18
!     - flushMap added
!
! 1992-06-17
!     - lcdGetScreen and lcdPutScreen added, reads and writes the hole screen
!
! 1992-07-20 TD,  version 1.4       Removed screenContext->status = 0; when
!                                   lcdpos() was used
! 
! 1993-01-21 TD,  version 1.5       Added 'useDisplay', if lcdsetselect(-1), 
!                                   don't use any display
*/

#include <varargs.h>
#include <ctype.h>
#include <time.h>
#include "phyio.h"

/*
void lcdinit();
void lcdwrite(unsigned char c);
void lcdcontrol(unsigned char);
void lcdpos(int r, int c);
*/
void lcdpos();

/*
!   area for keeping display characteristics
*/
static struct {
  int wrap          ;
  int cache         ;
  int cache_cursor  ;
  int flash_led_delay ;
  int timeout_value ;
} characteristics = { 1, 1, 0, 80, 300} ;    /* if timeout check 80 else 1000 */

/*   
! will use HB as alarm led on this prototype since no keyboard leds are mounted
!
! ok, 1991-03-06, will now use keyboard led
!
*/
#define USE_KEYBOARD_LED

/*
!   display, keyboard and led characteristics
*/
#define WRAP              characteristics.wrap
#define CACHE             characteristics.cache
#define CACHE_CURSOR      characteristics.cache_cursor
#define FLASH_LED_DELAY   characteristics.flash_led_delay
#define TIMEOUT_VALUE     characteristics.timeout_value
/*
!     CACHE       CACHE_CURSOR        function
!       0           x               Chars. will be rewritten and cursor moved
!
!       1                           Characters only written at mis-match
!       1           0               Will move cursor at match
!       1           1               Will not move cursor at match
*/

/*
!   Globala pointers till LCD-porten 
*/

unsigned char *LCD_CTRL = 0x308000;
unsigned char *LCD_DATA = 0x308001;
unsigned char *KEYBOARDPOINTER = 0x308002;
unsigned char *LEDBOARDPOINTER = 0x308002;

/**/
struct _screenContext {
  unsigned char display[2][40];
  unsigned char map[10];
  unsigned char keyCode, keyDown, keyWasDown, status;
  unsigned char x, y;
  int spawnedScreenPid;
} *screenContext;
int useScreenContext = 0, useDisplay = 1;
int useDDRAM = 1;

void lcdSetScreenContext(a)
struct _screenContext *a;
{
  screenContext = a;
  useScreenContext = 1;
}

void lcdSetSelect(c)
int c;
{
  if (c < 0) {
    useDisplay = 0;
  } else {
    useDisplay = 1;
    LCD_CTRL = ((unsigned char *) (0x308000 + 4*c));
    LCD_DATA = ((unsigned char *) (0x308001 + 4*c));
    KEYBOARDPOINTER = ((unsigned char *) (0x308002 + 4*c));
  }
/*  useScreenContext = 0;     */
}
/**/
    unsigned char *addressOn = (char *) 0x34001e;
    unsigned char *addressOff = (char *) 0x34001f;

/*
!   size of display
*/
#define MAX_COLUMNS   38            /* 40 ************ */
#define MAX_LINES     2
/*
!   statics to keep control of display contents and position
*/
static int xPos, yPos;      /* 0..39,  0..1 */
static unsigned char line[MAX_LINES][MAX_COLUMNS];
static unsigned checkMap[MAX_LINES][MAX_COLUMNS];

static int changed = 0;


/*
!   maps the keyboard scan code to internal code
*/

/*
    KEY_PLUS_MINUS            KEY_PLUS ,  KEY_MINUS
    KEY_POINT                 
    KEY_CHANGE                KEY_MAN
    KEY_ALARM                 KEY_READ_ALARM
*/

struct _tid {     /* size is (8?)  2+2 + 6 + 2 = 12bytes */
  short year;
  char  month, day, hour, minute, second, dummy;
};

struct _system {
  char flashBits,
       currentBits;
  char settime, flashBits2;
  struct _tid tid;
  long serialNo;
  long reboots;
  char newFlex;
} *sysVars = 0x003ffe0;


/*
    To use for new flex-layout
*/
char keyMapFlex[20] = { KEY_ENTER, KEY_LEFT, KEY_RIGHT, KEY_ALARM,
                    KEY_HELP,  KEY_DOWN,  KEY_UP,   KEY_CHANGE,
                    KEY_POINT, 9,         6,        3,
                    0,         8,         5,        2,
                    KEY_PLUS_MINUS,
                               7,         4,        1 };

/*
!   bit maps for pre-defined characters, upArrow, downArrow and swedish 'aring'
*/

/* bit map for } */

int aring[8] = {
                /* 00100  */    0x04,
                /* 00000  */    0x00,
                /* 01110  */    0x0e,
                /* 00001  */    0x01,
                /* 01111  */    0x0f,
                /* 10001  */    0x11,
                /* 01111  */    0x0f,
                /* 00000  */    0x00
  };

int upArrow[8] = {
                /* 00100  */    0x04,
                /* 01110  */    0x0e,
                /* 10101  */    0x15,
                /* 00100  */    0x04,
                /* 00100  */    0x04,
                /* 00100  */    0x04,
                /* 00100  */    0x04,
                /* 00000  */    0x00
  };

int downArrow[8] = {
                /* 00100  */    0x04,
                /* 00100  */    0x04,
                /* 00100  */    0x04,
                /* 00100  */    0x04,
                /* 10101  */    0x15,
                /* 01110  */    0x0e,
                /* 00100  */    0x04,
                /* 00000  */    0x00
  };
  
int leftArrow[8] = {
                /* 00010  */    0x02,
                /* 00100  */    0x04,
                /* 01000  */    0x08,
                /* 11111  */    0x1f,
                /* 01000  */    0x08,
                /* 00100  */    0x04,
                /* 00010  */    0x02,
                /* 00000  */    0x00
  };

int rightArrow[8] = {
                /* 01000  */    0x08,
                /* 00100  */    0x04,
                /* 00010  */    0x02,
                /* 11111  */    0x1f,
                /* 00010  */    0x02,
                /* 00100  */    0x04,
                /* 01000  */    0x08,
                /* 00000  */    0x00
  };

int highLevel[8] = {
                /* 00000  */    0x00,
                /* 01111  */    0x0f,
                /* 01000  */    0x08,
                /* 01000  */    0x08,
                /* 01000  */    0x08,
                /* 01000  */    0x08,
                /* 11000  */    0x18,
                /* 00000  */    0x00
  };
  
int lowLevel[8] = {
                /* 00000  */    0x00,
                /* 01110  */    0x0e,
                /* 01010  */    0x0a,
                /* 01010  */    0x0a,
                /* 01010  */    0x0a,
                /* 01010  */    0x0a,
                /* 11011  */    0x1b,
                /* 00000  */    0x00
  };

int bigAring[8] = {
                /* 00100  */    0x04,
                /* 00000  */    0x00,
                /* 11111  */    0x1f,
                /* 10001  */    0x11,
                /* 11111  */    0x1f,
                /* 10001  */    0x11,
                /* 10001  */    0x11,
                /* 00000  */    0x00
  };

int bigUmlaut[8] = {
                /* 01010  */    0x0a,
                /* 00000  */    0x00,
                /* 11111  */    0x1f,
                /* 10001  */    0x11,
                /* 11111  */    0x1f,
                /* 10001  */    0x11,
                /* 10001  */    0x11,
                /* 00000  */    0x00
  };
  
int bigOh[8] = {
                /* 01010  */    0x0a,
                /* 00000  */    0x00,
                /* 11111  */    0x1f,
                /* 10001  */    0x11,
                /* 10001  */    0x11,
                /* 10001  */    0x11,
                /* 11111  */    0x1f,
                /* 00000  */    0x00
  };
  
/* 
! lcdcontrol(c)
! char c;
!
! Skriver till LCD kontroll-register.
! c ar kontroll data. Se manualen for narmare anvisningar.
*/
void lcdcontrol(c)
unsigned char c;
{
  if (useScreenContext)
    parseControl(c);
/*  else    */
  if (useDisplay) 
  {
    while ((128 & *LCD_CTRL) != 0) 
      ;
    (*LCD_CTRL) = c;
  }
}

int lcdwr(c)
char c;
{
  if (useScreenContext)
    parseData(c);
/*  else  */
  if (useDisplay) 
  {
    while ((128 & *LCD_CTRL) != 0) ;
    *LCD_DATA = c;
  }
}

parseData(c)
unsigned char c;
{
  if (useDDRAM) {
    screenContext->display[screenContext->y][screenContext->x] = c;
    screenContext->x++;
  }
}

parseControl(c)
unsigned char c;
{
  if (c == 1)                         /* Clear display */
    screenContext->status = 0;
  else if ((c & 0xfe) == 2) {         /* Return home */
    screenContext->x = screenContext->y = 0;
  }
  else if ((c & 0xf8) == 8) {          /* Display on/off control */
    screenContext->status = ((c & 0x07) == 5) ? 1 : 0;  /* 1 == on, 0 == off */

/*    screenContext->status = c & 0x07;   */
  }
  else if (c & 0x80) {                 /* set DD RAM address */
/*    screenContext->status = 0;    */    /* removed 920720 */
    screenContext->x = c & 0x3f;
    screenContext->y = (c & 0x40) ? 1 : 0;
    useDDRAM = 1;
  } else if (c & 0x40)                 /* set CG RAM address */
    useDDRAM = 0;
}

/* 
! lcdwrite(c)
! char c;
!
!  Skriver tecknet c till lcd data-register.
*/
void lcdwrite(c)
unsigned char c;
{
/*
!   take care of control characters, such as formfeed and newline 
*/
  if (c == '\f') {
    lcdcld();         /* clear display */
    changed = 1;
    if (!CACHE_CURSOR)
      lcdpos(yPos, xPos);
    return;
  }
  if (c == '\n') {
    xPos = 0;
    yPos ++;
    changed = 1;
    if (yPos < MAX_LINES) {
      lcdpos(yPos, xPos);
      return;
    }
  }
  if (c == '\03') {
    lcdclr();
    return;
  }
/*
! retrench characters outside right margin if no wrap 
! otherwise continue at next line
*/

/*
  printf("\n(%d, %d) WRAP = %d, CACHE = %d, C = %d\n", xPos, yPos, WRAP, CACHE,
      CACHE_CURSOR);
*/

  if (xPos >= MAX_COLUMNS) {
    if (WRAP) {
     
/*  printf("wrap will now be performed:\n");  */
     
      xPos = 0;
      yPos ++;
      changed = 1;
      if ((yPos < MAX_LINES) && (!CACHE_CURSOR))
        lcdpos(yPos, xPos);
    } else
      return ;
  }
/*
!   scroll !!
*/
  if (yPos >= MAX_LINES) {
    for (yPos = 0, xPos = 0; xPos < MAX_COLUMNS; xPos++)
      if (line[0][xPos] != line[1][xPos]) {
        if (changed) {
          lcdpos(yPos, xPos);
        }
        lcdwr(line[0][xPos] = line[1][xPos]);
      } else
        changed = 1;
    for (yPos = 1, xPos = 0; xPos < MAX_COLUMNS; xPos++)
      if (line[1][xPos] != ' ') {
        if (changed) {
          lcdpos(yPos, xPos);
        }
        lcdwr(line[1][xPos] = ' '); xPos--;
      } else
        changed = 1;
    xPos = 0;
    lcdpos(yPos, xPos);
    if (c == '\n')
      return ;
  }
/*
! change strange character codes
*/
  if (c == '\06')
    c = 2;
  else if (c == '\04')
    c = 225;
  else if (c == '\24')
    c = 239;
  else if (c == '\20')
    c = 3;
  else if (c == '\16')
    c = 4;
  else if (c == '\31')
    c = 5;
  else if (c == '\17')
    c = 223;
  else if (c == '\1')
    c = 0;
  else if (c == '\2')
    c = 1;
  else if (c == '\x08')
    c = 6;
  else if (c == '\x09')
    c = 7;

/*
! if CACHE is being used, check if contents has changed
*/
  if (CACHE) {
    if (line[yPos][xPos] == c) {
      checkMap[yPos][xPos] = c;
      
      xPos ++;
      changed = 1;
      if (!CACHE_CURSOR && xPos < MAX_COLUMNS)
        lcdpos(yPos, xPos);
      return;
    }
    if (changed) {
      lcdpos(yPos, xPos);
    }
  }
  line[yPos][xPos] = c;           checkMap[yPos][xPos] = c;
  lcdwr(c);
  xPos ++;
  if (xPos >= MAX_COLUMNS) {
    xPos --;
    lcdpos(yPos, xPos);
    xPos ++;
  }
}

lcdclearCheckMap()
{
  int x;
  for (x = 0; x < MAX_COLUMNS; x++)
    checkMap[0][x] = checkMap[1][x] = ' ';
}

lcdflushCheckMap()
{
  int x, y, xp=-1, yp=-1, c;
  
  if (CACHE)
    for (y = 0; y < MAX_LINES; y++) 
      for(x = 0; x < MAX_COLUMNS; x++, xp++) {
        if ((line[y][x] != (c = checkMap[y][x])) && (c == ' ')) {
          if (xp != x || yp != y)
            lcdpos(yp = y, xp = x);
          line[y][x] = c;           /* added 920218 */
          lcdwr(c);
        }
      }
}

/*
!   relative position of cursor
*/
void lcdpositRel(dx, dy)
int dx, dy;
{
  xPos += dx;
  yPos += dy;
  lcdpos(yPos, xPos);
}

/* 
! lcdputs(s)
! char *s;
!
! Skriver string till LCD
*/
void lcdputs(str)
char *str;
{
  int cnt;
  cnt=0;
  while (*str && (cnt<128))
  {
    lcdwrite(*str++);
    cnt++;
  }
}

void lcdcursorOff()
{
  lcdcontrol(0x0C);     /* display on, cursor off, blink off */
}

void lcdcursorOn()
{
  lcdcontrol(0x0D);     /* display on, cursor off, blink on */
}

void lcdinit()
{
  int i;
  
  lcdcontrol(0x01);           /* clear display */
/*  lcdcontrol(0x0D);  */         /* display on, cursor off, blink on */
  lcdcontrol(0x0C);           /* display on, cursor off, blink off */
  lcdcontrol(0x38);           /* 8bit, 2lines, 5x7 dots  */
  lcdcontrol(0x06);           /* increment pos, no shift */

  lcddefine(0, downArrow);
  lcddefine(1, upArrow);
  lcddefine(2, aring);

  lcddefine(3, bigAring);
  lcddefine(4, bigUmlaut);
  lcddefine(5, bigOh);
  lcddefine(6, highLevel);
  lcddefine(7, lowLevel);

  lcdcld();     /* obs! lcdcld() must not use cache ! */
}

lcdshowdirections(pos, yes)
int pos;
char *yes;
{
  int x, y, pil;

#if 1
/*  if (sysVars->newFlex) {   */
    if (pos == 1)      { pil = 1; x = 38; y = 0; }    /* up */
    else if (pos == 4) { pil = 0; x = 38; y = 1; }    /* down  */
    else if (pos == 3) { pil = 126; x = 39; y = 0; }    /* right */
    else if (pos == 2) { pil = 127; x = 39; y = 1; }    /* left */
/*  } else {   
    if (pos == 1)      { pil = 1; x = 38; y = 0; }    
    else if (pos == 4) { pil = 0; x = 38; y = 1; }    
    else if (pos == 3) { pil = 126; x = 39; y = 1; }  
    else if (pos == 2) { pil = 127; x = 39; y = 0; }  
  }
*/
#else
  if (pos == 1)      { pil = 1; x = 38; y = 0; }    /* up */
  else if (pos == 4) { pil = 0; x = 38; y = 1; }    /* down  */
  else if (pos == 3) { pil = 126/*4*/; x = 39; y = 0; }    /* right */
  else if (pos == 2) { pil = 127/*3*/; x = 39; y = 1; }    /* left */
#endif

  lcdcontrol(0x80 | (y*0x40) | x);
  
  lcdwr( (yes) ? pil : ' ');

  lcdcontrol(0x80 | (yPos*0x40) | (xPos  /* + 1 */));
}

/*
void lcdprintf(stf,p1,p2,p3,p4,p5,p6,p7,p8)
char *stf;
unsigned long p1,p2,p3,p4,p5,p6,p7,p8;
  {
  char s[100];
  sprintf(s,stf,p1,p2,p3,p4,p5,p6,p7,p8);
  lcdputs(s);
  }
*/

#ifdef SUCK 
int ptrprintf(ap)
va_list *ap;
{
  char *stf, *p, line[80], *fmtPek, fmt[40], *sval;
  int ival, offset, width;
  double dval;

  stf = va_arg(*ap, char *);
  for (p = stf; *p; p++) {
    if (*p != '%') {
      lcdwrite(*p);
      continue;
    }
    ++p;
    fmtPek = fmt;
    *fmtPek++ = '%';
    while (isdigit(*p) || *p == '.' || *p == '-')
      *fmtPek++ = *p++;
    *fmtPek++ = *p;
    *fmtPek = '\0';   
   
    switch (*p) {
      case 'a':
        ival = va_arg(*ap, int);
        --fmtPek;
        *fmtPek = '\0';  
        fmtPek = fmt; fmtPek ++;
        offset = 0;
        width = 8;
        if (*fmtPek) {
          width = atoi(fmtPek);
          while (*fmtPek && *fmtPek != '.')
            fmtPek++;
          if (*fmtPek) {
            offset = width;
            fmtPek++;
            width = atoi(fmtPek);
          }
        }
        line[0] = '\0';
        ival >>= offset;
        line[width] = '\0';
        while (width ) {
          line[--width] = (ival & 1) ? '\2' : '\1';
          ival >>= 1;
        }
        lcdputs(line);
        break;
      case 'c':
        ival = va_arg(*ap, int);
        sprintf(line, fmt, ival);
        lcdputs(line);
        break;
      case 'd':
        ival = va_arg(*ap, int);
        sprintf(line, fmt, ival);
        lcdputs(line);
        break;
      case 'x':
        ival = va_arg(*ap, int);
        sprintf(line, fmt, ival);
        lcdputs(line);
        break;
      case 'f':
        dval = va_arg(*ap, double);
        sprintf(line, fmt, dval);
        lcdputs(line);
        break;
      case 'g':
        dval = va_arg(*ap, double);
        sprintf(line, fmt, dval);
        lcdputs(line);
        break;
      case 's':
        sval = va_arg(*ap, char *);
        sprintf(line, fmt, sval);
        lcdputs(line);
        break;
      default:
        lcdwrite(*p);
        break;
    }
  }
/*  va_end(*ap);    */
} 
#endif

void lcdprintf(va_alist)
va_dcl
{
  char *stf, *p, line[80], *fmtPek, fmt[40], *sval;
  int ival, offset, width;
  double dval;
  va_list ap;
  va_start(ap);
  
  
  stf = va_arg(ap, char *);
  for (p = stf; *p; p++) {
    if (*p != '%') {
      lcdwrite(*p);
      continue;
    }
    ++p;
    fmtPek = fmt;
    *fmtPek++ = '%';
    while (isdigit(*p) || *p == '.' || *p == '-')
      *fmtPek++ = *p++;
    *fmtPek++ = *p;
    *fmtPek = '\0';   
   
    switch (*p) {
      case 'a':
        ival = va_arg(ap, int);
        --fmtPek;
        *fmtPek = '\0';  
        fmtPek = fmt; fmtPek ++;
        offset = 0;
        width = 8;
        if (*fmtPek) {
          width = atoi(fmtPek);
          while (*fmtPek && *fmtPek != '.')
            fmtPek++;
          if (*fmtPek) {
            offset = width;
            fmtPek++;
            width = atoi(fmtPek);
          }
        }
        line[0] = '\0';
        ival >>= offset;
        line[width] = '\0';
        while (width ) {
          line[--width] = (ival & 1) ? '\2' : '\1';
          ival >>= 1;
        }
        lcdputs(line);
        break;
      case 'c':
        ival = va_arg(ap, int);
        sprintf(line, fmt, ival);
        lcdputs(line);
        break;
      case 'd':
        ival = va_arg(ap, int);
        sprintf(line, fmt, ival);
        lcdputs(line);
        break;
      case 'x':
        ival = va_arg(ap, int);
        sprintf(line, fmt, ival);
        lcdputs(line);
        break;
      case 'f':
        dval = va_arg(ap, double);
        sprintf(line, fmt, dval);
        lcdputs(line);
        break;
      case 'g':
        dval = va_arg(ap, double);
        sprintf(line, fmt, dval);
        lcdputs(line);
        break;
      case 's':
        sval = va_arg(ap, char *);
        sprintf(line, fmt, sval);
        lcdputs(line);
        break;
      default:
        lcdwrite(*p);
        break;
    }
  }
  va_end(ap);
} 

void lcdhome()
{
  xPos = 0;
  yPos = 0;
  changed = 1;                /* ??? 920115 */
}

void lcdpos(r,c)
int r,c;
{
  xPos = c;
  yPos = r;
  
/*  c++;   */                         /* *************** */ 
 
  lcdcontrol(0x80 | (r*0x40) | c);
  changed = 0;
}

static char ledInfo[6] = {0, 0, 0, 0, 0, 0};    /* flash info */
static int ledCurrent = 0;

void led(a, b)      /* led a = b; */
int a, b;
{
  ledInfo[a] = 0;
  ledCurrent &= ~(1 << a);
  if (b) 
    ledCurrent |= (1 << a);

#ifdef USE_KEYBOARD_LED
    *LEDBOARDPOINTER = ~ledCurrent;
#else
  if (a == 0)           /* only alarm led */
  {
    if (b)      /* light it */
      *addressOn = (char) 0x04;
    else
      *addressOff = (char) 0x04;
  }
#endif
}

void flashLed(a)
int a;
{
  led(a, 1);      /* start with lightning it and let updateLed() do the rest */
  ledInfo[a] = 1;
}

void updateLed()
{
  int i, sv;
  static int prev = 0;
  
  prev++;
/*
  if (prev < FLASH_LED_DELAY)
    return ;
*/    
  prev = 0;
  for (i = 0; i < 6; i++) {
    if (sv = ledInfo[i]) {
      led(i, ((ledCurrent >> i) & 1) ^ 1);
      ledInfo[i] = sv;
    }
  }
}
/*
!   checks if any key is being pressed
*/
static unsigned char *wdst = 0x00348000;

int keyWasDown()
{
  if (useScreenContext)
    return screenContext->keyWasDown;
  else
    return (*KEYBOARDPOINTER) & 0x80;
}

int keyDown()
{
  *wdst = 0;                  /* a try for 1.624 since some DCU's reboot */
  if (useScreenContext) {
    int dwn;
    if (dwn = screenContext->keyDown) {
      /* set up a timer which expires in 100ms, the icp resets the keyDown */
      /* or ... */
      screenContext->keyWasDown = 17;

/*  screenContext->keyDown = 0;       /* another try 920721 */
    }
    return dwn;
  } else
    return (*KEYBOARDPOINTER) & 0x40;
}
/*
! returns key code
*/
int key()
{
  if (useScreenContext)
    return screenContext->keyCode;
  else
    return keyMapFlex[*KEYBOARDPOINTER & 0x1f];
}
/*
! waits for a key being pressed and return key code
*/
int getKey()
{
  time_t timer;
  
  timer = time(0);
  while (!keyDown()) {
/*    updateLed();    */
    if ((time(0) - timer) > TIMEOUT_VALUE)
      return NO_KEY;
  }
  timer = time(0);
  while (keyDown()) {
/*    updateLed();  */
    if ((time(0) - timer) > TIMEOUT_VALUE)
      return NO_KEY;
  }
  return key();
}
/*
! defines a user character, 
! c is character ascii code of user character 0..7
! and a is a pointer to bitmap
*/
void lcddefine(c, a)
int c;
int a[];
{
  int i;
  lcdcontrol(0x40 | (c << 3));     /* init pos in CG RAM to 0 */
  for (i = 0; i < 8; i++)
    lcdwr(a[i]);
}

lcdGetScreen(buf)
char *buf;
{
  memcpy(buf, line, 80);
}

lcdPutScreen(buf)
char *buf;
{
  for (yPos = 0; yPos < MAX_LINES; yPos++) {
    lcdpos(yPos, xPos = 0);
    for ( ; xPos < MAX_COLUMNS; xPos++)
      lcdwr(line[yPos][xPos] = checkMap[yPos][xPos] = *buf++);
  }
  lcdpos(0,0);
}

/*
! clear contents of display and positions cursor at (0,0)
*/
int lcdcld()
{
  for (yPos = 0; yPos < MAX_LINES; yPos++) {
    lcdpos(yPos, xPos = 0);
    for ( ; xPos < MAX_COLUMNS; xPos++)
      lcdwr(line[yPos][xPos] = checkMap[yPos][xPos] = ' ');
  }
  lcdpos(0,0);
}

/*
! clear rest of display and repositions cursor at current position !
*/
int lcdclr()
{
  int currentX, currentY;
  
  currentX = xPos;
  currentY = yPos;
  for (; yPos < MAX_LINES; yPos++) {
    lcdpos(yPos, xPos);
    for ( ; xPos < MAX_COLUMNS; xPos++)
      lcdwr(line[yPos][xPos] = ' ');
  }
  lcdpos(currentY, currentX);
}

/*
! clear contents of display through cache and positions cursor at (0,0)
*/
int lcdcld_through_cache()
{
  int a, b;
  lcdpos(0,0);
  for (a = 0; a < MAX_LINES; a++) {
    for (b = 0; b < MAX_COLUMNS; b++) {
      if ((CACHE == 0) || (line[a][b] != ' ')) {
        lcdpos(a, b);
        lcdwr(line[a][b] = ' ');
      }
    }
  }
  lcdpos(0,0);
}


int lcdsetWrap(c)
int c;
{
  WRAP = c;
}
int lcdsetCache(c)
int c;
{
  CACHE = c;
}
int lcdsetCacheCursor(c)
int c;
{
  CACHE_CURSOR = c;
}


