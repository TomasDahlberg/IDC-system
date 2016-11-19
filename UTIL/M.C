#include <stdio.h>
#include <sgstat.h>
extern int errno;

int pathIn = 0, pathOut = 1;

int send_number, leg, send_password[8], phone_number;

main()
{
  FILE *fpOut, *fpIn;
  int sts;
  double pris = 0;

  if ((fpOut = fopen("/n124", "w")) == NULL) {
    fprintf(stderr, "cannot open '%s' for write access\n", "/n124");
    exit(0);
  }
  if ((fpIn = fopen("/n124", "r")) == NULL) {
    fprintf(stderr, "cannot open '%s' for read access\n", "/n124");
    exit(0);
  }
  pathIn  = fpIn->_fd;
  pathOut = fpOut->_fd;

  disableXonXoff(pathIn);
  disableXonXoff(pathOut);
  switch8_7_no_even(pathIn, 2);
  switch8_7_no_even(pathOut, 1);

  send_number = 901952;
  leg = 0;
  send_password[0] = ' ';
  send_password[1] = ' ';
  send_password[2] = 'B';
  send_password[3] = 'V';
  send_password[4] = 'G';
  send_password[5] = 'D';
  send_password[6] = 'U';
  send_password[7] = 'C';
  phone_number = 482086;
  sts = execute_minicall(send_number, leg, send_password, phone_number, &pris);
  fprintf(stderr, "errno = %d\n", errno);
  fprintf(stderr, "sts = %d\n", sts);
  fprintf(stderr, "pris = %g\n", pris);
}

io_error(path)
int path;
{
  struct sgbuf buffer;
  
  if (_gs_opt(path, &buffer) == -1) {
    fprintf(stderr, "err io_opt: %d\n", errno);
    exit(errno);
  }
  fprintf(stderr, "I/O error %d\n", buffer._sgm._sgs._sgs_err);
}

disableXonXoff(path)
int path;
{
  struct sgbuf buffer;
  
  if (_gs_opt(path, &buffer) == -1) {
    fprintf(stderr, "errGs_opt: %d\n", errno);
    exit(errno);
  }
  buffer._sgm._sgs._sgs_xon   = 0;
  buffer._sgm._sgs._sgs_xoff  = 0;
  buffer._sgm._sgs._sgs_kbich = 0;      /* keyboard interrupt character */
  buffer._sgm._sgs._sgs_kbach = 0;      /* keyboard abort character */

  buffer._sgm._sgs._sgs_echo= 0;
  buffer._sgm._sgs._sgs_pause=0;
  buffer._sgm._sgs._sgs_bspch=0;
  buffer._sgm._sgs._sgs_dlnch=0;
  buffer._sgm._sgs._sgs_rlnch=0;
  buffer._sgm._sgs._sgs_dulnch=0;
  buffer._sgm._sgs._sgs_kbich=0;
  buffer._sgm._sgs._sgs_bsech=0;
  buffer._sgm._sgs._sgs_xon=0;
  buffer._sgm._sgs._sgs_xoff=0;
  buffer._sgm._sgs._sgs_tabcr=0;
  buffer._sgm._sgs._sgs_delete= 0;
/*  buffer._sgm._sgs._sgs_eorch=13;	*/
/*  buffer._sgm._sgs._sgs_eofch=0;	*/
  buffer._sgm._sgs._sgs_psch=0;
  buffer._sgm._sgs._sgs_kbach=0;
  buffer._sgm._sgs._sgs_bellch=0;
  
  if (_ss_opt(path, &buffer) == -1) {
    fprintf(stderr, "errSs_opt: %d\n", errno);
    exit(errno);
  }
}

switch8_7_no_even(path, sw)	/* 1-> even/7, 0-> none/8 */
int path, sw;
{
  struct sgbuf buffer;
  
  if (_gs_opt(path, &buffer) == -1) {
    fprintf(stderr, "errGs_opt: %d\n", errno);
    exit(errno);
  }
  if (sw == 2) {
    buffer._sgm._sgs._sgs_parity = 0 | 4 |  	/* no parity, 7 bits/char */
    		(buffer._sgm._sgs._sgs_parity & 0xf0);
  } else if (sw == 1) {
    buffer._sgm._sgs._sgs_parity = 3 | 4 |  	/* even parity, 7 bits/char */
    		(buffer._sgm._sgs._sgs_parity & 0xf0);
  } else {
    buffer._sgm._sgs._sgs_parity = 0 | 0 |  	/* no parity, 8 bits/char */
    		(buffer._sgm._sgs._sgs_parity & 0xf0);
  }
  
  if (_ss_opt(path, &buffer) == -1) {
    fprintf(stderr, "errSs_opt: %d\n", errno);
    exit(errno);
  }
}
