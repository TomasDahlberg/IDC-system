/* lexicon.c  1992-??-?? TD,  version 1.0 */
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
! lexicon.c
! Copyright (C) 1991, IVT Electronic AB.
*/

/*
!     lexicon.c is part of the IVTnet product
!
!     File: lexicon.c
!     
!     Contains definitions for messages sent on IVTnet
!
!     History     
!     Date        Revision Who  What
!     
!     ??-???-1992     1.0  TD   Start of coding
!
*/
#include <module.h>
#include <stdio.h>

static int QUICKER = 0;

static struct _screenContext {
  unsigned char display[2][40];
  unsigned char map[10];
  unsigned char key, keyDown, keyWasDown, status;
  unsigned char x, y;
  int spawnedScreenPid;
};

#define LEXICON "LEXICON"
static char *lexiconHeader;
static char *lexicon;
static struct _metaModHeader {
    unsigned short sync;
    unsigned short sysrev;
    unsigned long  size;
    unsigned long  owner;
    unsigned long  nameoffset;
    unsigned short access;
    unsigned short typelang;
    unsigned short attrrev;
    unsigned short edition;
    unsigned long  usage;
    unsigned long  symbol;
    unsigned short ident;
    char  spare[12];
    unsigned short parity;
    
    unsigned long  dataptr;
    char  name[12];
};

void readLexicon()
{
  mod_exec *mod, *modlink();
  extern int errno;
  
  if (lexicon)
    return ;
  if ((mod = modlink(LEXICON, 0)) == (mod_exec *) -1) {
    printf("cannot link to '%s'\n", LEXICON);
    printf("errno = %d\n", errno);
    exit(errno);
  }
  lexicon = ((char *) mod) + sizeof(struct _metaModHeader);
}

static struct _lexicon {
  unsigned short shortIdxMap[64];
  struct {
    unsigned short offset;
    unsigned char shortCode;		/* 0xff if none */
    unsigned char pad;
  } map[1024];
/*
  char names[40000];
*/
} *buff;

char *lexiconQuickName(lexicon, idx)
char *lexicon;
int idx;
{
  struct _lexicon *buff;
  buff = (struct _lexicon *) lexicon;
  if (idx < 0 || idx > 63)
    return (char *) 0;
  return ((char *) buff->map) + buff->map[buff->shortIdxMap[idx]].offset;
}

char *lexiconName(lexicon, idx)
char *lexicon;
int idx;
{
  struct _lexicon *buff;
  int antal;

  buff = (struct _lexicon *) lexicon;
  antal = buff->map[0].offset / sizeof(buff->map[0]);
  if (idx < 0 || idx > antal)
    return (char *) 0;
  return ((char *) buff->map) + buff->map[idx].offset;
}

int lexiconShortCode(lexicon, idx)
char *lexicon;
int idx;
{
  struct _lexicon *buff;
  int antal;

  buff = (struct _lexicon *) lexicon;
  antal = buff->map[0].offset / sizeof(buff->map[0]);
  if (idx < 0 || idx > antal)
    return -1;
  if (buff->map[idx].shortCode == 0xff)
    return -1;
  return buff->map[idx].shortCode;
}

int lexiconIdx(lexicon, name)
char *lexicon;
char *name;
{
  struct _lexicon *buff;
  int antal;
  int id, a, b, match;

  buff = (struct _lexicon *) lexicon;
  a = 0; /* 1; */
  b = buff->map[0].offset / sizeof(buff->map[0]);
  while (1) {
    id = (a + b) / 2;
    if ((match = strcmp(((char *) buff->map) + buff->map[id].offset, name)) < 0){
      if (a == b)
        break;
      if (a == id) {
        a = b;
        continue;               /* break */
      }
      a = id;                       /* go forward */
    } else if (match > 0) {
      if (a == b)
        break;
      if (b == id) {
        b = a;
        continue;     /* break */
      }
      b = id;                       /* back */
    } else
      break;                        /* ok, found ! */
  }
/*
  printf("lexiconIdx: a = %d, b = %d, %s, id = %d\n", a, b, 
				(match == 0) ? "MATCH" : "NO MATCH", id);
*/
  return (match == 0) ? id : -1;
}

static int matchLen(table, our)
char *table;
char *our;
{
  int match = 0;
  while (*our) {
    if (*table != *our)
      break;
    our ++;
    table ++;
    match ++;
  }
  if (*table != 0)
    match = 0;
  return match;
}

int lexiconCloseIdx(lexicon, name)
char *lexicon;
char **name;
{
  struct _lexicon *buff;
  int antal;
  int id, a, b, match, match1, match2;

  buff = (struct _lexicon *) lexicon;
  a = 0; /* 1; */
  b = buff->map[0].offset / sizeof(buff->map[0]);
  while (1) {
    if (a + 1 == b) {
      if (strcmp(((char *) buff->map) + buff->map[a].offset, *name) == 0)
	return a;
      if (strcmp(((char *) buff->map) + buff->map[b].offset, *name) == 0)
	return b;

      match1 = matchLen(((char *) buff->map) + buff->map[a].offset, *name);
      match2 = matchLen(((char *) buff->map) + buff->map[b].offset, *name);
/*
      printf("match1 = %d, match2 = %d\n", match1, match2);
*/
      if (match1 < 2 && match2 < 2) /* less than 2chars match ! */
	return -1;
      if (match1 > match2) {
	*name += match1;
	return a;
      } else {
	*name += match2;
	return b;
      }
    }
    id = (a + b) / 2;
    if ((match = strcmp(((char *) buff->map) + buff->map[id].offset, *name)) < 0){
      if (a == b)
        break;
      if (a == id) {
        a = b;
        continue;               /* break */
      }
      a = id;                       /* go forward */
    } else if (match > 0) {
      if (a == b)
        break;
      if (b == id) {
        b = a;
        continue;     /* break */
      }
      b = id;                       /* back */
    } else
      break;                        /* ok, found ! */
  }
/*
  printf("lexiconIdx: a = %d, b = %d, %s, id = %d\n", a, b, 
				(match == 0) ? "MATCH" : "NO MATCH", id);
*/
  return (match == 0) ? id : -1;
}

void decode(buf, len)
unsigned char *buf;
int len;
{
  int i;
  for (i = 0; i < len; i++) {
    if (*buf == 2)
      *buf = 6;
    else if (*buf == 225)
      *buf = 4;
    else if (*buf == 239)
      *buf = 20;
    else if (*buf == 3)
      *buf = 16;
    else if (*buf == 4)
      *buf = 14;
    else if (*buf == 5)
      *buf = 25;
    else if (*buf == 223)
      *buf = 15;
    else if (*buf == 0)
      *buf = 1;
    else if (*buf == 1)
      *buf = 2;
    else if (*buf == 6)
      *buf = 8;
    else if (*buf == 7)
      *buf = 9;
    buf++;
  }
}

void encode(buf, len)
unsigned char *buf;
int len;
{
  int i;
  for (i = 0; i < len; i++) {
    if (*buf == 6)
      *buf = 2;
    else if (*buf == 4)
      *buf = 225;
    else if (*buf == 20)
      *buf = 239;
    else if (*buf == 16)
      *buf = 3;
    else if (*buf == 14)
      *buf = 4;
    else if (*buf == 25)
      *buf = 5;
    else if (*buf == 15)
      *buf = 223;
    else if (*buf == 1)
      *buf = 0;
    else if (*buf == 2)
      *buf = 1;
    else if (*buf == 8)
      *buf = 6;
    else if (*buf == 9)
      *buf = 7;
    buf++;
  }
}

encodeIt(x, y, status, flash, led, up, down, r, l)
int x,          /* 0 - 39 */
    y,          /* 0 - 1  */
    status,     /* 0/1    */
    flash,      /* 3bits, red,green,yellow */
    led,        /* 3bits, red,green,yellow */
    up,         /* 0/1    */
    down,       /* 0/1    */
    r,          /* 0/1    */
    l;          /* 0/1    */
{
  int red, green, yellow, arrows;

  /* 40*2*2*(3*3*3)*2*2*2*2 > 2^16 !! */
  /* 40*2*2*(3*2*3)*2*2*2*2 < 2^16 !! */
  /* i.e. code led green as always lit !! */
  
  red = green = yellow = arrows = 0;
  if (up)
    arrows |= 1;
  if (down)
    arrows |= 2;
  if (l)
    arrows |= 4;
  if (r)
    arrows |= 8;
  if (led & 1)
    red = 1;
  if (led & 2)
    green = 1;
  if (led & 4)
    yellow = 1;
  if (flash & 1)
    red = 2;
  if (flash & 2)
    green = 2;
  if (flash & 4)
    yellow = 2;

  if (green == 2)
    green = 0;


{ extern int DEBUG;
  if (DEBUG) {
    printf("flash = %d, led = %d -->> red=%d,green=%d,yellow=%d\n",
      flash, led, red, green, yellow);
    printf("arrows=%d,status=%d,y=%d,x=%d\n", arrows, status, y, x);
  }
}


  return ((((((arrows*3 + yellow)*2 + green)*3 + red)*2 + status)*2 + y)*40+x);
}

decodeIt(code, x, y, status, flash, led, up, down, r, l)
int code;
unsigned char *x,          /* 0 - 39 */
    *y,          /* 0 - 1  */
    *status;     /* 0/1    */
int *flash,      /* 3bits, red,green,yellow */
    *led,        /* 3bits, red,green,yellow */
    *up,         /* 0/1    */
    *down,       /* 0/1    */
    *r,          /* 0/1    */
    *l;          /* 0/1    */
{
  int red, green, yellow, arrows;
/* colors: 0 = off, 1 = on, 2 = flash */
  
  *x = code % 40;      code /= 40;
  *y = code & 1;       code >>= 1;
  *status = code & 1;  code >>= 1;
  red = code % 3;     code /= 3;
  green = code & 1;   code >>= 1;
  yellow = code % 3;  code /= 3;
  arrows = code & 15;
  *led = 0;
  *flash = 0;
  if (green == 0)     /* always on ! */
    green = 2;

  if (red)
    *led |= 0x01;
  if (green)
    *led |= 0x02;
  if (yellow)
    *led |= 0x04;
  if (red == 2)
    *flash |= 0x01;
  if (green == 2)
    *flash |= 0x02;
  if (yellow == 2)
    *flash |= 0x04;

  *up   = (arrows & 0x01) ? 1 : 0;    
  *down = (arrows & 0x02) ? 1 : 0;    
  *l    = (arrows & 0x04) ? 1 : 0;    
  *r    = (arrows & 0x08) ? 1 : 0;    
}

#define SPACE_THE_REST 23
#define SPACE_16 31
#define SPACE_8 30
/* #define SPACE_7 30   */
#define SPACE_6 29
#define SPACE_5 28
#define SPACE_4 27
#define SPACE_3 26
#define SPACE_2 24        /* obs 24 ! */
#define SPACE_1 ' ' 

/*
!   takes the message buffer and decodes it, creating a fresh display
*/
decodeScreenContext(sc, flash, led, in, len)
struct _screenContext *sc;
int *flash, *led;
unsigned char *in;
int len;
{
  int code, up, down, r, l, q, c, i, x, y, sp, chr;
  unsigned char *p;
  
  code = (*in++) << 8;
  code |= *in++;
  len -= 2;
  decodeIt(code, &sc->x, &sc->y, &sc->status, flash, led, &up, &down, &r, &l);
  sc->display[0][38] = up ? 2 /* 1 */ : 32;
  sc->display[1][38] = down ? 1 /* 0 */ : 32;
  sc->display[0][39] = r ? 126 : 32;
  sc->display[1][39] = l ? 127 : 32;

  x = 0;
  y = 0;
  while (len--) {
    if (x >= 38) {
      if (y == 1) {
       printf("Fault, framing error ! x = %d, y = %d and %d more bytes to go !\n",
                   x, y, len + 1);
       return 0;
      }
      x = 0;
      y = 1;
    }
    q = *in++;
    if (q < 127) {
      sp = 1;
      chr = ' ';
      if (q == SPACE_16)
        sp = 16;
      else if (q == SPACE_8)
        sp = 8;
      else if (q == SPACE_6)
        sp = 6;
      else if (q == SPACE_5)
        sp = 5;
      else if (q == SPACE_4)
        sp = 4;
      else if (q == SPACE_3)
        sp = 3;
      else if (q == SPACE_2)
        sp = 2;
      else if (q == SPACE_THE_REST)
        sp = 40;
      else
        chr = q;
      while (sp-- && x < 38) {
        sc->display[y][x] = chr;
        x++;
      }
    } else {    /* quick or long code */
      if (q & 0x40) {    /* quick code */
        p = (unsigned char *) lexiconQuickName(lexicon, q & 0x3f);
      } else {                  /* long code */
        c = ((q & 0x3f) << 8) | *in++;
        p = (unsigned char *) lexiconName(lexicon, c);
        len --;
      }
      while (x < 38 && *p) {
        sc->display[y][x] = *p++;
        x++;
      }
    }
  }
}

/*
!   takes a display buffer and encodes it into the message buffer
*/
encodeScreenContext(sc, flash, led, utPtr)
struct _screenContext *sc;
int flash, led;
unsigned char **utPtr;
{
  int code, up, down, r, l, q, c, i;
  unsigned char *p, *ut;
  
  ut = *utPtr;
  up   = (sc->display[0][38] == 2 /* 1 */);
  down = (sc->display[1][38] == 1 /* 0 */);
  r    = (sc->display[0][39] == 126);
  l    = (sc->display[1][39] == 127);
  code = encodeIt(sc->x, sc->y, sc->status ? 1 : 0, flash, led, up, down, r, l);
  *ut++ = (code & 0xff00) >> 8;
  *ut++ = code & 0xff;
  for (i = 0; i < 2; i++) {
    p = sc->display[i];
    while (p < &sc->display[i][38]) {
      q = lexiconCloseIdx(lexicon, &p);
      if (q >= 0) {
        if ((c = lexiconShortCode(lexicon, q)) >= 0) 
         *ut++ = 128 | 64 | c;
        else {
          *ut++ = 128 | ((q & 0x3f00) >> 8);
          *ut++ = q & 0xff;  
        }
      } else if (*p == ' ') {
        int cnt = 0;
        
        while (p < &sc->display[i][38]) {
          if (*p != ' ')
            break;
          cnt ++;
          p ++;
        }
        if (p >= &sc->display[i][38]) {
          *ut++ = SPACE_THE_REST;
          cnt = 0;
        }
        while (cnt >= 1) {
          if (cnt >= 16) { cnt -= 16; *ut++ = SPACE_16; }
          else if (cnt >= 8) { cnt -= 8; *ut++ = SPACE_8; }
          else if (cnt >= 6) { cnt -= 6; *ut++ = SPACE_6; }
          else if (cnt >= 5) { cnt -= 5; *ut++ = SPACE_5; }
          else if (cnt >= 4) { cnt -= 4; *ut++ = SPACE_4; }
          else if (cnt >= 3) { cnt -= 3; *ut++ = SPACE_3; }
          else if (cnt >= 2) { cnt -= 2; *ut++ = SPACE_2; }
          else if (cnt >= 1) { cnt -= 1; *ut++ = SPACE_1; }
        }
      } else
if (QUICKER) {
      while (p < &sc->display[i][38] && *p != ' ') {
        *ut++ = *p++;
      }
} else {
        *ut++ = *p++;
}
    }
  }
  *utPtr = ut;
}

#ifdef DEBUG
struct _screenContext scr, scr2, scr3;

main(argc,argv)
int argc;
char *argv[];
{
  int ps, screenPid, t1;
  char pp[50];
  
  readLexicon();

  scr.key = scr.keyDown = scr.keyWasDown = 0;

  sprintf(pp, "-1 %d", &scr);
  printf("Test: param = '%s'\n", pp);
  ps = strlen(pp) + 1;
  if ((screenPid = os9fork("screen", ps, pp, 0, 0, 0, 0)) == -1) {
    printf("cannot fork to screen module\n");
  }
  t1 = time(0);
  while (1) {
    if (_gs_rdy(0) > 0) {
      char k;
      read(0, &k, 1);
      if (k > 96)
        scr.key = k - 96;
      else if (k > 64)
        scr.key = k - 64;
      else
        scr.key = k;
        
      scr.keyDown = 1;
    }
    if (time(0) - t1 > 1) {
      t1 = time(0);
      disp(&scr);
      scr.keyDown = 0;

      {
        int flash, led, i, l, j;
        unsigned char ut[200], *utPtr;
        long tstart, tstop;
        

#if 0
 tstart = time(0);
 for (i = 0; i < 1000; i++) {
#endif 
        flash = 0;
        led = 0;
        utPtr = ut;
        memcpy(&scr2, &scr, sizeof(struct _screenContext));
        decode(scr2.display, 80);   /* disp -> internal */
        encodeScreenContext(&scr2, flash, led, &utPtr); /* internal -> msg */
#if 0
 } 
 tstop = time(0);
 printf("Time for %d iterations of encode is %d sec\n", i, tstop - tstart);
#endif 
       
        printf("%d chars in ut-buf\n", l = utPtr - ut);
        for (i = 0; i < l; i++) {
          printf("%02x, ", ut[i]);
          if ((i & 15) == 15) printf("\n");
        }
        
        for (j = 0; j < 2; j++)
        for (i = 0; i < 40; i++)
          scr3.display[j][i] = '?'; 
       
        printf("\n");
        printf("Re-decode it: \n");
#if 0
 tstart = time(0);
 for (i = 0; i < 1000; i++) {
#endif
        decodeScreenContext(&scr3, &flash, &led, ut, l); /* msg -> internal */
        encode(scr3.display, 80);  /* internal -> disp */
#if 0
 } 
 tstop = time(0);
 printf("Time for %d iterations of decode is %d sec\n", i, tstop - tstart);
#endif 
        disp(&scr3);
        printf("\n");
      }
    }
  }
}


#endif
