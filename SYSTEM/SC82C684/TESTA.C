#include <stdio.h>
#include <sgstat.h>
#include <errno.h>

main()
{
  setSpeed(stdin->_fd, 57600);
  setSpeed(stdout->_fd, 57600);  

  system("");
}

setSpeed(path, speed)
int path, speed;
{
  struct sgbuf buffer;
  
  if (_gs_opt(path, &buffer) == -1) {
    fprintf(stderr, "error during _gs_opt: %d\n", errno);
    exit(errno);
  }
  if (speed == 115200)
    buffer._sgm._sgs._sgs_baud   = 0x19;
  else if (speed == 57600)
    buffer._sgm._sgs._sgs_baud   = 0x18;
  else if (speed == 38400)
    buffer._sgm._sgs._sgs_baud   = 0x10;
  if (_ss_opt(path, &buffer) == -1) {
    fprintf(stderr, "error during _ss_opt: %d\n", errno);
    exit(errno);
  }
}
