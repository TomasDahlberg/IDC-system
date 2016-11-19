/*
!  IDC code examples
  clearalarm("Press enter to clear all alarms");
    or
  clearalarm("Time is %s, do really wanna clear all alarms\nso press enter", swdctime());
*/
double clearalarm(va_alist)
va_dcl
{
  va_list ap;
  int status, pid;
  
  int pos, keyCode, point = 0, changed = 0;
  char buff[40], backup[40];
  time_t timer;
  double v1;

  lcdsetCacheCursor(1);     /* don't want cursor sweeping over display */
  timer = time(0);

  lcdhome();
  va_start(ap);
  v1 = ptrprintf(&ap);

  if ((time(0) - timer) > TIMEOUT_VALUE)      /* ? */
    return v1;

  if (!keyDown() || key() != KEY_ENTER)
    return v1;


  if (!checkAccessLevel(PRIV_M_CLEARALARM))
    return v1;
    
  while (keyDown())     /* wait for release of enter-key */
    ;

  os9fork("clearalarm", 0, 0, 0, 0, 0, 0);
  display("Clearing...");
  while (!(pid = wait(&status)))
      ;
  if (status == 0)
      display("Ok !");
  else
      display("Err %d !", status);
}

