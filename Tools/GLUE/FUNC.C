#include <stdarg.h>
#include <stdio.h>
#include <time.h>

char buffer[80];
extern char *displayBuffer;

void convert(char *s)
{
  char c;
  while (c = *s) {
    if (c == '\017')
	c = '�';
    if (c == '\024')
	c = '�';

    *s++ = c;
  }
}

int display(char *fmt, ...)
{
   va_list argptr;
   int cnt;

   va_start(argptr, fmt);
   cnt = vsprintf(buffer, fmt, argptr);
   va_end(argptr);
   convert(buffer);
   strcat(displayBuffer, buffer);
/*   printf("%s", buffer);		*/
   return(cnt);
}


float enter()
{
  return 17.3;
}

int enter_calendar()
{
}

char *xxxswdctime()
{
  static char buf[80];
  time_t t;

  time(&t);
  sprintf(buf, "%s", ctime(&t));
  return buf;
}

int password()
{
  printf("Ange l�senord:\n");
}

int set_time()
{
}

