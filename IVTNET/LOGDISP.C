/*
  IVTnet message

      1. KEY_DISPLAY | REQUEST

            Ok, we are running the screen process
            entry point is captureDisplay(message);


      2. KEY_DISPLAY | REPLY

            Ok, we have received a display context, show it !
            entry point is showDisplayContext(message);

    union _keyDisplay
    {
      struct _display
      {
        unsigned char map[10];
        unsigned char buf[40];
        unsigned char flashLed, currentLed, status, x, y;
      } display;
      struct _key
      {
        unsigned char keyCode, keyDown, keyWasDown;
      } key;
    } keyDisplay;
*/
#include <time.h>
#include "ivtnet.h"
#define NO_OF_ALARMS 1
#ifndef DOS /* OSK */
#include "../../alarm.h"
#include "../../meta.h"
#else
#include "sysvars.h"
/* #include "alarm.h"	*/
#include "meta.h"
#endif

extern struct _system *sysVars;

struct _screenContext {
  unsigned char display[2][40];
  unsigned char map[10];
  unsigned char keyCode, keyDown, keyWasDown, status;
  unsigned char x, y;
  int spawnedScreenPid;
};

extern struct _screenContext *screenContext;
extern int DEBUG;

disp(scr)
struct _screenContext *scr;
{
  int j, i, c;

  printf("+");  
  for (i = 0; i < 40; i++)
    printf("-");
  printf("+\n");
  for (j = 0; j < 2; j++) {
    printf("!");
    for (i = 0; i < 40; i++) {
      c = scr->display[j][i];
      if (c > 32 && c < 128)
        printf("%c", c);
      else
        printf(".");
    }
    printf("!\n");
  }
  printf("+");
  for (i = 0; i < 40; i++)
    printf("-");
  printf("+\n");
  if (scr->display[0][38] == 1) printf("up, ");
  if (scr->display[1][38] == 0) printf("down, ");
  if (scr->display[0][39] == 126) printf("right, ");
  if (scr->display[1][39] == 127) printf("left ");
  printf("\n");
  printf("Key=%d,keydown=%d\n", scr->keyCode, scr->keyDown);
  printf("x=%2d,y=%d\n", scr->x, scr->y);
  printf("\n");
}
/*
!   takes the message blocks and unpack, show information on our display
*/
void showDisplayContext(message, len)
struct _message *message;
int len;
{
  int xPos, yPos, i, j, next, ok;
  unsigned char prev, new, mess, newBits;

  int flash, led;
  struct _screenContext scr;
  
  decodeScreenContext(&scr, &flash, &led, 
            message->mix.keyDisplay.display.buf, len - 2); /* msg -> internal */
  encode(scr.display, 80);  /* internal -> disp */
  if (DEBUG) {
    printf("flash = %d, led = %d\n", flash, led);
    disp(&scr);
  } 
  
    /* move bytes for led's into our sys structure */
  
  prev = sysVars->flashBitsB;
  new = sysVars->flashBitsB = flash;
/*
!   prev new      -> currentBits    xor
!
!   0    0        -> copy           0
!   0    1        -> set            1
!   1    0        -> clear          1
!   1    1        ->  --            0
*/
  newBits = 0;
  mess = led;
  for (j = 1; j < 256; j <<= 1)
  {
    if (((prev | new) & j) == 0)
      newBits |= mess & j;
    else if ((new ^ prev) & new & j)
      newBits |= j; 
    else if ((new ^ prev) & prev & j)
      newBits &= ~j; 
    else if (new & prev & j)
      ; 
  }
  sysVars->currentBitsB = newBits;
  
  for (yPos = 0; yPos < 2; yPos++) {
    lcdpos(yPos, xPos = 0);
    if (DEBUG) {
      printf("lcdpos(y=%d,x=%d)\n", yPos, xPos);
    }
    for (; xPos < 40; xPos++) {
/*
      if (DEBUG) {
        printf("lcdwr(%02x = disp[%d][%d])\n", scr.display[yPos][xPos], yPos, xPos);
      }
*/
      lcdwr(scr.display[yPos][xPos]);
    } 
  }
  if (DEBUG) {
    printf("[0][38] = %d, [0][39] = %d\n", scr.display[0][38], scr.display[0][39]);
    printf("[1][38] = %d, [1][39] = %d\n", scr.display[1][38], scr.display[1][39]);
  }
  if (scr.y < 2 && scr.x < 40)
    lcdpos(scr.y, scr.x);
  if (scr.status & 1)
    lcdcursorOn();
  else
    lcdcursorOff();
}


/*
!   takes the message blocks and unpack, includes key codes
!   then, pack our display context in the same message
*/
static unsigned char cap[2][40];
static unsigned char prevLed, prevFlash;

int unpackKeyCodes(message)
struct _message *message;
{
  screenContext->keyCode = message->mix.keyDisplay.key.keyCode;
  screenContext->keyDown = message->mix.keyDisplay.key.keyDown;
/*
  screenContext->keyWasDown = message->mix.keyDisplay.key.keyWasDown;
*/
  if (DEBUG) {
    printf("key = %d, down = %d, wasDown = %d\n",
        screenContext->keyCode,
        screenContext->keyDown,
        screenContext->keyWasDown);
  }
}

int captureDisplay(message)
struct _message *message;
{    /* move bytes for led's into our sys structure */
  int flash, led, x, y, ch;
  struct _screenContext scr2;
  unsigned char *utPtr;
   
/*    has been move to the unpackKeyCodes routine 
  screenContext->keyCode = message->mix.keyDisplay.key.keyCode;
  screenContext->keyDown = message->mix.keyDisplay.key.keyDown;
  screenContext->keyWasDown = message->mix.keyDisplay.key.keyWasDown;
*/
  flash = sysVars->flashBits;
  led = sysVars->currentBits;
  if (DEBUG)
    printf("flash = %d, led = %d\n", flash, led);
  utPtr = message->mix.keyDisplay.display.buf;
  memcpy(&scr2, screenContext, sizeof(struct _screenContext));
  for (ch = y = 0; y < 2; y++)
    for (x = 0; x < 40; x++) {
      if (scr2.display[y][x] != cap[y][x]) {
        ch = 1;
/*
        if (DEBUG) {
          printf("Diff x=%d,y=%d,  %02x <-> %02x\n", x, y, cap[y][x], 
                      scr2.display[y][x]);
        }
*/
        cap[y][x] = scr2.display[y][x];
      }
    }
  if (ch == 0) {
    if (flash == prevFlash && led == prevLed)
      ;     /* nothing has changed */
    else
      ch = 1;
  }
  prevLed = led;
  prevFlash = flash;
  decode(scr2.display, 80);   /* disp -> internal */
  encodeScreenContext(&scr2, flash, led, &utPtr); /* internal -> msg */
  return ((ch == 0) ? -1 : 1) * (utPtr - message->mix.keyDisplay.display.buf);
}

