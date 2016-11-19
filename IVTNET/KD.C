/*
!   takes the message blocks and unpack, show information on our display
*/
void showDisplayContext(message)
struct _message *message;
{    /* move bytes for led's into our sys structure */
  int xPos, yPos, i, j, next, ok;
  unsigned char prev, new, mess, newBits;
  
  prev = sysVars->flashBits;
  new = sysVars->flashBits = message->mix.keyDisplay.display.flashLed;
/*
!   prev new      -> currentBits    xor
!
!   0    0        -> copy           0
!   0    1        -> set            1
!   1    0        -> clear          1
!   1    1        ->  --            0
*/
  newBits = 0;
  mess = message->mix.keyDisplay.display.currentLed;
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
  sysVars->currentBits = newBits;
  
  next = yPos = xPos = ok = 0;
  for (i = 0; i < 10; i++) {
    for (j = 1; j < 256; j <<= 1) {
      if (message->mix.keyDisplay.display.map[i] & j) {
        if (!ok) {
          lcdpos(yPos, xPos);
          ok = 1;
        }
        lcdwr(message->mix.keyDisplay.display.buf[next++]);
      } else 
        ok = 0;
      xPos ++;
    } 
    if (xPos >= 40) {
      xPos = 0;
      yPos = 1; 
    }
  }
  lcdpos(message->mix.keyDisplay.display.y, 
                    message->mix.keyDisplay.display.x);
  if (message->mix.keyDisplay.display.status & 1)
    lcdcursorOn();
  else
    lcdcursorOff();
}

/*
!   takes the message blocks and unpack, includes key codes
!   then, pack our display context in the same message
*/
static char cap[2][40];

void captureDisplay(message)
struct _message *message;
{    /* move bytes for led's into our sys structure */
  int xPos, i, j, next, q, p;
  static char yPos = 0;
#define MAX_NEXT 40

  for (j = 0; j < 2; j++) 
    for (i = 0; i < 40; i++) {
      if (cap[j][i] != screenContext->display[j][i]) {
        cap[j][i] = screenContext->display[j][i];
        screenContext->map[(j*40+i)/8] |= (1 << ((j*40+i) % 8));
      }
    }
  
  screenContext->keyCode = message->mix.keyDisplay.key.keyCode;
  screenContext->keyDown = message->mix.keyDisplay.key.keyDown;
  message->mix.keyDisplay.display.flashLed = sysVars->flashBits;
  message->mix.keyDisplay.display.currentLed = sysVars->currentBits;
  
  message->mix.keyDisplay.display.status = screenContext->status;
  message->mix.keyDisplay.display.x = screenContext->x;
  message->mix.keyDisplay.display.y = screenContext->y;
  next = 0;
  for (q = 0; q < 2; q++, yPos ^= 1) {
    xPos = 0;
    for (i = yPos * 5, p = 0; p < 5; i++, p++) {
      message->mix.keyDisplay.display.map[i] = 0;
      for (j = 1; j < 256; j <<= 1, xPos++)
        if ((screenContext->map[i] & j) && (next < MAX_NEXT)) {
          message->mix.keyDisplay.display.buf[next++] = 
                        screenContext->display[yPos][xPos];
          screenContext->map[i] &= ~j;
          message->mix.keyDisplay.display.map[i] |= j;
        }
    }
  }
}


/*
!   starts a new screen process with our buffer as context
*/
int spawnScreen()
{
  int paramSize;
  char param[20];
  screenContext = (struct _screenContext *) screenBuff;

  sprintf(param, "-1 %d", screenContext);
  paramSize = strlen(param) + 1;
  if (DEBUG) printf("param='%s', paramSize=%d\n", param, paramSize);
  if ((screenContext->spawnedScreenPid = 
                    os9fork("screen", paramSize, param, 0, 0, 0, 0)) == -1) {
    return (errno) ? errno : 1; /* cannot fork to screen module */
  }
  return 0;       /* ok ! */
}

