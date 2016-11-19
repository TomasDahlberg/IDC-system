#include <varargs.h>
#include <time.h>
#include "phyio.h"
#include <ctype.h>
#include <math.h>

#define TIMEOUT_VALUE 300

double ptrprintf(ap)
va_list *ap;
{
  char *stf, *p, line[80], *fmtPek, fmt[40], *sval;
  int *ival, offset, width;
  double *dval;
  double v1 = 0.0;
  int first = 0;
  
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
        ival = va_arg(*ap, int*);
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
        (*ival) >>= offset;
        line[width] = '\0';
        while (width ) {
          line[--width] = ((*ival) & 1) ? '\2' : '\1';
          (*ival) >>= 1;
        }
        lcdputs(line);
        break;
      case 'c':
        ival = va_arg(*ap, int*);
        sprintf(line, fmt, *ival);
        lcdputs(line);
        break;
      case 'd':
        ival = va_arg(*ap, int*);
        sprintf(line, fmt, *ival);
        lcdputs(line);
        if (!first++) {
          v1 = (double) *ival;
        }
        break;
      case 'f':
        dval = va_arg(*ap, double*);
        sprintf(line, fmt, *dval);
        lcdputs(line);
        if (!first++)
          v1 = *dval;
        break;
      case 'g':
        dval = va_arg(*ap, double*);
        sprintf(line, fmt, *dval);
        lcdputs(line);
        if (!first++)
          v1 = *dval;
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
  return v1;
} 

int kalle(va_alist)
va_dcl
{
  va_list ap;

  while (1) {  
    va_start(ap);
    ptrprintf(&ap);
  }  
}

/*
double enter(s, v1, v2, v3, v4, v5)
char *s;
double *v1, *v2, *v3, *v4, *v5;
*/

double enter(va_alist)
va_dcl
{
  va_list ap;
  int pos, keyCode, point = 0, changed = 0;
  char buff[40], backup[40];
  double newValue = 0;
  time_t timer;
  double v1;

  /* bug !! cio cannot handle SMALL numbers e.g 3.8241e-112 !!! */
/*  if (abs(*v1) < 0.000010) {    */            /* changed 91-09-09 */

/*
  if ((*v1 < 0.000010) && (*v1 > -0.000010)) {
    *v1 = 0.0;
  }
*/

  lcdsetCacheCursor(1);     /* don't want cursor sweeping over display */
  timer = time(0);
  do {
    lcdhome();
    va_start(ap);
    v1 = ptrprintf(&ap);

/*
    lcdprintf(s, *v1, ((int) v2 > 0x3ffff) ? 0 : *v2,
                      ((int) v3 > 0x3ffff) ? 0 : *v3,
                      ((int) v4 > 0x3ffff) ? 0 : *v4,
                      ((int) v5 > 0x3ffff) ? 0 : *v5);
*/

    if ((time(0) - timer) > TIMEOUT_VALUE)
      return v1;
  } while (!keyDown());
  if (key() != KEY_CHANGE)
    return v1;

  printf("Enter: ptrprint returned %g\n", v1);
      
  if (5 /* getLevel()*/ < 3) {
    while (keyDown())
      ;
    lcdpos(1, 0);
/*    lcdprintf("F\024r l\06g beh\024righetsniv\06 !"); */
    lcdprintf("F\024r l\06g beh\024righetsniv\06 !             ");
/*
    sleeptight(2); 
*/
    sleep(2);

    lcdprintf("\f");
    return v1;
  }
    
  lcdpos(1, 0);
  lcdprintf("                                      ");
  lcdpos(1, 1);
  lcdcursorOn();
  buff[0] = ' '; buff[1] = '\0';
  pos = 1;
  while (1) {
    switch (keyCode = getKey()) {
      case NO_KEY:                  /* timeout, return old value */
        lcdcursorOff();
        lcdpos(1, 0);
        lcdprintf("                                      ");
        return v1;
      case KEY_CHANGE:              /* no change, return old value */
        if (changed) {
          lcdcursorOff();
          lcdpos(1, 0);
          lcdprintf("                                      ");
          return v1;
        }
        break;
      case KEY_ENTER:               /* ok, enter finished */
        lcdcursorOff();
        lcdpos(1, 0);
        lcdprintf("                                      ");
        if (buff[1])
          return atof(buff);
        return v1;
      case KEY_LEFT:                /* dismis last digit entered, if: */
        if (pos > 1) {              /* any has been entered */
          pos --;
          if (buff[pos] == '.')
            point = 0;
          buff[pos] = '\0';
          lcdpositRel(-1, 0);
          lcdwrite(' ');
          lcdpositRel(-1, 0);
          if (pos == 1) {           /* if no more digits reset any sign */
            lcdpositRel(-1, 0);
            lcdwrite(' ');
          }
        }
        break;
      case KEY_PLUS_MINUS:        /* change sign */
        if (pos > 1) {
          lcdpos(1, 0);
          if (buff[0] == '-')
            lcdwrite(buff[0] = ' ');
          else
            lcdwrite(buff[0] = '-');
          lcdpos(1, pos);
        }
        break;
      case KEY_POINT:
        if (!point) {
          point = 1;
          if (pos == 1) {
            lcdpos(1, pos);
            buff[pos++] = '0';
            lcdwrite('0');
          }
          lcdpos(1, pos);
          buff[pos++] = '.';
          buff[pos] = '\0';
          lcdwrite('.');
        }
        break;
      case KEY_RIGHT:
      case KEY_UP:
      case KEY_DOWN:
      case KEY_ALARM:
      case KEY_HELP:
        break;
      default:
        lcdpos(1, pos);
        buff[pos++] = '0' + keyCode;
        buff[pos] = '\0';
        lcdwrite('0' + keyCode);
        break;
    }
    changed = 1;
  }
}


main()
{
  int x;
  double y, z;
 
  initphyio();
  
  x = 0;
  y = 0;
  
  lcdprintf("\f");
  while (1) {
    printf("x = %d\ny = %g\n", x, y);
    
    
    z = enter("x = %g\3\ny = %d", &y, &x);
    
   
    printf("Retur z = %g\n", z);
    
    y = z;
    x = y * 2;
  }
}
