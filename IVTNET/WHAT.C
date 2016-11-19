/* what.c  1993-09-20 TD,  version 1.1 */
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
 * Heimdalsgatan 4
 * 113 28 Stockholm
 * Sweden
 */

/*
! what.c
! Copyright (C) 1991-1993 IVT Electronic AB.
*/

/*
!     what.c is part of the IVTnet product
!
!     File: what.c
!     
!     Shows network status from master site when Server is not running !
!
!     History     
!     Date        Revision Who  What
!     
!      3-sep-1991     1.0  TD   Start of coding
!     20-sep-1993     1.1  TD   Added options
*/

@_sysedit: equ 2
@_sysattr: equ $8002

#include <stdio.h>
#include <time.h>
#include <modes.h>
#define NETDEVICE "/nb"

static int netpath = 0;

int netopen()
{
   return (netpath = open(NETDEVICE, 0x03));
}

int netpoll(node)
int node;
{
  _ss_dcon(netpath, node);
  return _gs_rdy(netpath);
}

void icp(s)
int s;
{
  gotoxy(0, 21);
  exit(0);
}

print(s)
char *s;
{
  write(1, s, strlen(s));
}

gotoxy(x, y)
int x, y;
{
  char buff[10];
  sprintf(buff, "%c%c%02d;%02d%c", 27, 91, y + 1, x + 1, 72);
  write(1, buff, 8);
}

clearScreen()
{
  int i;
  for (i = 0; i < 24; i++)
    printf("\n");
}

int usage()
{
  fprintf(stderr, "Syntax: what [<opt>] {<node>}\n");
  fprintf(stderr, "Function: show net status when server is NOT running\n");
  fprintf(stderr, "Options:\n");
  fprintf(stderr, "    -a  shows nodes 1 - 255\n");
  fprintf(stderr, "    -6  shows nodes 1 - 63\n");
  fprintf(stderr, "    -t  used with parameter <node> to show timestamps\n");
  fprintf(stderr, "\nParameter <node> is used to poll only this node\n");
}

char *ctid(t)
long t;
{
  static char buf[30];
  strcpy(buf, ctime(&t));
  buf[strlen(buf) - 1] = 0;
  return buf;
}

main(argc, argv)
int argc;
char *argv[];
{
  int x, y, i, j, fails[20], polls, tid, node, maxNode = 20;
  int prevOk[20], changeTime[20], timeStamp = 0;
  char status[80];
  int sts = -1;
  
  while( argc >= 2  && argv[1][0] == '-' ) {
    while( *++(argv[1]) ) {
      switch( *argv[1] ) {
        case '6':
          maxNode = 63;
          break;
        case 'a':
          maxNode = 255;
          break;
        case 't':
          timeStamp = 1;
          break;
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
  } else
    node = -1;
  

  intercept(icp);
  for (i = 0; i < 20; i++)
    fails[i] = prevOk[i] = 0;
  if (netopen() == -1) {
    printf("cannot open network !\n");
    exit(0);
  }
  clearScreen();
  netpoll(1);
  netpoll(100);
  if (maxNode > 20) {
    x = 0; y = 0;
    gotoxy(x,y);
    printf("Available nodes;\n");
    while (1) {           /* for ever */
      x = 0; y = 1;
      gotoxy(x,y);
      for (i = 1; i < 14; i++) {
        node = i;
        if (node > maxNode)
          break;
        for (j = 0; j < 20; j++, node += 13) {
          if (node > maxNode) {
            sprintf(status, "    ");
	    write(1, status, 4);
            continue;
	  }
          polls = netpoll(node);
          if (polls >= 0) 
            sprintf(status, "%4d", node);
          else
            sprintf(status, "    ");
	  write(1, status, 4);
        }
/*
        if (node >= maxNode)
          break;
*/
      }
    }
  }
  while (1) {           /* for ever */
    x = 0; y = 0;
    tid = time(0);
/*    netpoll(100);   */
    for (i = 0; i < 20; i++, y++) {
      gotoxy(x,y);

      polls = netpoll(node >= 0 ? node : i);
      if (polls < 0) {
        fails[i] ++;
        if (prevOk[i] != -1) {
          changeTime[i] = tid;
          prevOk[i] = -1;
        }
      } else {
        fails[i] = 0;
        if (prevOk[i] != 1) {
          changeTime[i] = tid;
          prevOk[i] = 1;
        }
      }
      if ((tid - changeTime[i]) < 10)
        strcpy(status, (prevOk[i] == 1) ? "Up  " : "Down");
      else
        strcpy(status, "          ");
      printf("Node %2d -> poll = %5d,  (fails = %d)  %s\n", 
              node >= 0 ? node : i, polls, fails[i], status);
      if (node >= 0) {
        if (timeStamp) {
          static int yt = 0;
          
          if (sts == -1) sts = (polls < 0) ? 1 : 0;
          
          if (polls < 0 && sts) {  /* going down... ? */
            sts = 0;
            if (!yt)
              yt++;
          } else if (polls >= 0 && !sts) {  /* going up... ? */
            sts = 1;
            yt ++;
          } else 
            break;
          if (yt > 20)
            yt = 1;
          gotoxy(sts ? 0 : 40, yt);
          sprintf(status, "%s %s", sts ? "Up  " : "Down", ctid(tid));
          print(status);
          if (sts == 0) 
            printf("\n");
        }
        break;
      }
    }
  } 
}
