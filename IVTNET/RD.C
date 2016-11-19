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

  while (1) {
    int nr;
    unsigned char antal;
    
    printf("gsrdy-> %d\n", _gs_rdy(netpath));
    bt = netpoll(0);
    printf("Poll = %d\n", bt);
    if (bt > 0) {
      printf("netread, antal = %d, ", nr = netread(0, &antal, 1));
      printf("antal = %d\n", antal);
      bt --;
      printf("netread = %d\n", nr = netread(0, buf, bt));
      if (nr > 0)
        dump(buf, bt);
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
