#include <stdio.h>

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
  char buf[1024];
  int size, i;
  
  size = atoi(argv[1]);
  for (i = 0; i < size; i++)
    buf[i] = i + 32;


  if (netopen() == -1) {
    fprintf(stderr, "cannot open network !\n");
    exit(0);
  }
  sleep(2);

  buf[0] = (char) size;
  printf("netwrite = %d (%d)\n", netwrite(0, buf, size), size);
  printf("netwrite = %d (%d)\n", netwrite(0, buf, size), size);
  sleep(10);

  while (1) {  
    buf[0] = (char) size;
    printf("netwrite = %d (%d)\n", netwrite(0, buf, size), size);
    sleep(5); 
  }

  sleep(0);
}
