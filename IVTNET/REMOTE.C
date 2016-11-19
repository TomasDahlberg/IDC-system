/* remote.c  1991-09-03 TD,  version 1.0 */
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
! remote.c
! Copyright (C) 1991, IVT Electronic AB.
*/

/*
!     remote.c will become part of the IVTnet product
!
!     File: remote.c
!     
!     Program for starting a remote shell
!
!     History     
!     Date        Revision Who  What
!     
!      3-sep-1991     1.0  TD   Start of coding
!
!
*/

#include <stdio.h>
#include <modes.h>
#include <errno.h>
#include <time.h>
#include <sgstat.h>
#include "ivtnet.h"

@_sysedit: equ 1
@_sysattr: equ $8002

int Abort = 0;

struct _sgs statbuf;    /* 128 byte status buffer */
struct _sgs bufcopy;

int set_com_param(echo)
int echo;
{
  int stat;
  stat= getstat(0, 0, &statbuf);
/*  if (stat) printf("getstat status= %d\n", stat);
  printf("baud=%02x xoff=%02x xon=%02x tab=%02x\n",
    statbuf._sgs_baud,statbuf._sgs_xoff,statbuf._sgs_xon,statbuf._sgs_tabcr);
*/
  memcpy(&bufcopy, &statbuf, 128);   /* save copy to restore */
  statbuf._sgs_echo= echo;
  statbuf._sgs_pause=0;
  statbuf._sgs_bspch=0;
  statbuf._sgs_dlnch=0;
  statbuf._sgs_rlnch=0;
  statbuf._sgs_dulnch=0;
  statbuf._sgs_kbich=0;
  statbuf._sgs_bsech=0;
  statbuf._sgs_xon=0;
  statbuf._sgs_xoff=0;
  statbuf._sgs_tabcr=0;
  statbuf._sgs_delete= 0;
  statbuf._sgs_eorch=13;
  statbuf._sgs_eofch=0;
  statbuf._sgs_psch=0;
  statbuf._sgs_kbach=0;
  statbuf._sgs_bellch=0;

  setstat(0,0,&statbuf);     /* set new parameters */
  return(stat);
}

int restore_param()
{
  int stat;
  stat= setstat(0,0,&bufcopy);
  return(stat);
}

icp(s)
int s;
{
  if (s == 3 || s == 2)
    Abort = 1;
}

static unsigned char *shellError = 0x003ffd0; /* 0 = shell is running 
                                                 1 = shell has terminated ok,
                                                     otherwise error code */
int usage()
{
  fprintf(stderr, "Syntax: remote [<opt>] node\n");
  fprintf(stderr, "Function. starts a remote shell at specified node\n");
  fprintf(stderr, "Options:\n");
  fprintf(stderr, "    -e     local echo disabled\n\n");
}

main(argc, argv)
int argc;
char *argv[];
{
  int node, status, nb, echo = 1;
  int readPath, writePath, dataSent = 0;
  char pipeIn[50], pipeOut[50];
  char buf[256];
  
  
  while( argc >= 2  && argv[1][0] == '-' ) {
    while( *++(argv[1]) ) {
      switch( *argv[1] ) {
        case 'e':
        case 'E':
          echo = 0;
          continue;
        case '?':
          usage();
          exit(0);
	default:
          printf( "illegal option: %c", (char *) *argv[1]);
          exit(0);
      }
    }
    argv++;
    argc--;
  }
  if(argc > 1) {
    node = atoi(argv[1]);
  } else {
    fprintf(stderr, "node not specified\n");
    usage();
    exit(0);
  }

  initidcio();
  intercept(icp);
  fprintf(stderr, "Connecting to remote node %d...\n", node);
  
  if (!(status = netSetRemote(node))) {
    fprintf(stderr, "slave process not available (?)\n");
/*    exit(0);  */

  } else if (status < 0) {
    status = -status;
    if (status == NET_BUSY)
      fprintf(stderr, "node is busy, try again later\n");
    else if (status == NET_SPAWN_ERROR)
      fprintf(stderr, "unknown error during spawn shell at remote site\n");
    else if (status == NET_TIMEOUT)
      fprintf(stderr, "cannot reach node %d, timeout\n", node);
    else {
      fprintf(stderr, "error %d returned from remote site\n", status);
      exit(status);
    }
    exit(0);
  }

  sprintf(pipeIn, "/pipe/from_%d", node);
  sprintf(pipeOut, "/pipe/to_%d", node);

  if ((readPath = open(pipeIn, S_IREAD)) < 0) {
    fprintf(stderr, "cannot open '%s', error %d\n", pipeIn, errno);
    exit(errno);
  }
  if ((writePath = open(pipeOut, S_IWRITE)) < 0) {
    fprintf(stderr, "cannot open '%s', error %d\n", pipeOut, errno);
    exit(errno);
  }
  set_com_param(echo);
  fprintf(stderr, "Connection established\n", node);
  status = 0;
  while (!Abort) {
    if (*shellError)
    {
      if (*shellError == 1)
        break;
      fprintf(stderr, "shell terminated with error %d\n", *shellError);
      status = *shellError;
      break;
    }
    
    if ((nb = _gs_rdy(0)) > 0) {
      if ((time(0) - dataSent) > 1) {
        read(0, buf, nb);
        write(writePath, buf, nb);
        dataSent = time(0);
      }
    }
    if ((nb = _gs_rdy(readPath)) > 0) {
      read(readPath, buf, nb);
      writeScreen(1, buf, nb);
    }
    if (Abort) {
      char ctrl_c = 3;
      
      write(writePath, &ctrl_c, 1);
/*      Abort = 0;   */

/*      write(writePath, "logout", 6);  */
    }
  }
  restore_param();
  fprintf(stderr, "remote connection terminated\n");
  exit(status);
}

dumpBuf(buf, nb)
char *buf;
int nb;
{
  int i;
  for (i = 0; i < nb; i++)
    printf("%02x, ", buf[i]);
  printf("\n");
}

int writeScreen(path, buf, nb)
int path;
char *buf;
int nb;
{
  char lf = 10;
  
  while (nb--) {
    write(path, buf, 1);
    if (*buf == 13)
      write(path, &lf, 1);
    buf++;
  }
}


