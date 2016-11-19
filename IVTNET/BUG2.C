#include <stdio.h>
#include <errno.h>

static int netpath = 0, netnode = -1;

int netopen()
{
   return ((netpath = open("/nb", 0x03)) == -1);
}

int netwrite(node, cptr, sz)
int node;
char *cptr;
int sz;
{
	int n;
	if (node == netnode)
	   return(write(netpath, cptr, sz));
	_ss_dcon(netpath, node);
	netnode = node;
        return(write(netpath, cptr, sz));
}

int netread(node, cptr, sz)
int node;
char *cptr;
int sz;
{
  if (node != netnode) {
    _ss_dcon(netpath, node);
    netnode = node;
  }
  return(read(netpath, cptr, sz));
}

int netpoll(node)
int node;
{
  if (node != netnode) {
    _ss_dcon(netpath, node);
    netnode = node;
  }
  return(_gs_rdy(netpath));
}

main(argc, argv)
int argc;
char *argv[];
{
  char buf[256];
  int bt, node, preFixedSize = 0;
  
  if (netopen() == -1)  {
    fprintf(stderr, "cannot open network !\n");
    exit(0);
  }

  preFixedSize = atoi(argv[1]);
  printf("Fixed size is %d bytes\n", preFixedSize);

  node = 2;
  printf("0. node %d has %d bytes", node, netpoll(node));
                           printf(", errno = %d\n", errno);
  while (netpoll(node) < 1) {
    netpoll(100);
    printf("waiting.. %d\n", netpoll(node));
  }
  
  while (1) {
    if ((bt = netpoll(node)) > 0) {
      int rest, bt2, bt3, bt4, nr;


      bt2 = netpoll(node);
      bt3 = netpoll(node);
      bt4 = netpoll(node);
      printf("---------------\nPoll: %d, %d, %d, %d\n", bt, bt2, bt3, bt4); 


      if (preFixedSize) {
        if (preFixedSize == 1) {
          unsigned char antal;
          int nr;
          
          printf("netread = %d, ", nr = netread(node, &antal, 1));
          printf("[0] = %d\n", antal);
          bt = antal - 1;
          
          if (nr == -1) {
            printf("Empty-ing buffer....\n");
            
            printf("b. poll = %d\n", netpoll(node));
            printf("b. poll = %d\n", netpoll(node));
            printf("b. poll = %d\n", netpoll(node));
            netpoll(100);
            printf("a. poll = %d\n", netpoll(node));
            printf("a. poll = %d\n", netpoll(node));
            printf("a. poll = %d\n", netpoll(node));
            continue;
          }   

      
        } else
          bt = preFixedSize;
      }
      printf("3. Netread returns %d", nr = netread(node, &buf[0], bt));
      printf(", errno = %d\n", errno);
      if (nr > 0)
        dump(buf, bt);
      rest = buf[0] - bt;
      printf("rest = %d, poll = %d\n", rest, netpoll(node));
    } 
  }
}

dump(buf, siz)
unsigned char *buf;
int siz;
{
  int i;
  for (i = 0; i < siz; i++)
    printf("%02x, ", buf[i]);
  printf("\n");
}
