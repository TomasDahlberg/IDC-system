#include <stdio.h>
#include <errno.h>
#include <sgstat.h>

disableXonXoff(path)
int path;
{
  struct sgbuf buffer;
  
  if (_gs_opt(path, &buffer) == -1) {
    fprintf(stderr, "error during _gs_opt: %d\n", errno);
    exit(errno);
  }
  buffer._sgm._sgs._sgs_xon   = 0;
  buffer._sgm._sgs._sgs_xoff  = 0;
  buffer._sgm._sgs._sgs_kbach = 0;      /* keyboard abort character */
  buffer._sgm._sgs._sgs_parity = 1;
  
  if (_ss_opt(path, &buffer) == -1) {
    fprintf(stderr, "error during _ss_opt: %d\n", errno);
    exit(errno);
  }
}

int nextChar()
{
  int b;
  unsigned char buf;
  
  if ((b = _gs_rdy(0)) > 0)
    read(0, &buf, 1);
  return (b < 1) ? -1 : (int) buf;
}

int wait4NextChar()
{
  int b;
  while ((b = nextChar()) < 0)
    ;
  return b;
}
#define SOM 0x03c
#define SOA 0x03d
#define EOM 0x03e

main()
{
  int b, pla, ela;

/*   disableXonXoff(0); */

  while (1)
  {
    b = wait4NextChar();  
    if (b == SOM) {
      printf("Start found\n");

      pla = wait4NextChar();
      ela = wait4NextChar();

      printf("Start found: pla = %d, ela = %d\n", pla, ela);
      
      while ((b = wait4NextChar()) != EOM)
        printf("b = %02x\n", b);
    } else if (b == SOA) {
      ;
      
    }
  }
}
