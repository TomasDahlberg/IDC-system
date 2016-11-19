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

  node = atoi(argv[1]);
  
  if (netopen() == -1)  {
    fprintf(stderr, "cannot open network !\n");
    exit(0);
  }

  netpoll(100);
  while (1) {
    printf("waiting.. node %d has %d bytes\n", node, netpoll(node));
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
