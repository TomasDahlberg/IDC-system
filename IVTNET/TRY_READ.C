static unsigned char bigBuf[1024];
static int last, currentNode = -1;

emptyBuffer()
{
  printf("emptyBuffer: node = %d, buffer size %d bytes!!\n", currentNode, last);
  last = 0;
  netpoll(100);
}

myNetpoll(node)
int node;
{
  if (currentNode == node) {
    if (last) 
      return last;
    else
      return netpoll(node);
  } else {
    if (last)
      printf("myNetPoll: Error, switching node %d->%d while buffer not empty!\n", currentNode, node);
    currentNode = node;
    last = 0;
    return netpoll(node);
  }
}

doNetRead(node)
int node;
{
  int nb, nr;

  printf("       doNetRead node %d ", node);  
  nb = netpoll(node);
  if (nb > 0) {
    if ((nb + last) < 1024) {
      nr = netread(node, &bigBuf[last], nb);
      printf("adding %d bytes (%d)\n", nb, nr);
      if (nr == -1) {
        printf("doNetRead: error ...... -1 returned from netread()\n");
        return -1;
      }
      last += nr;
    } else 
      printf("bigBuf full\n");
  } else
    printf("no additional bytes read\n");
  return nb;
}

fetchNodeData(node)
int node;
{
  int nb;
  
  nb = 0;
  printf("fetchNodeData: node %d\n", node);
  while ((nb += doNetRead(node)) > 0)
    ;
  return nb;
}

allNetRead(node, buf, size)
int node;
char *buf;
int size;
{
  int mn, rest, nb;
  
  printf("allNetRead: read %d bytes.\n", size);
/*
  while ((nb = doNetRead(node)) > 0)
    ;
*/
  if (nb == -1)
    return -1;
  if (last == 0) {
    printf("allNetRead: No data in buffer !! returns 0\n");
    return 0;
  }
  mn = (last > size) ? size : last;
  memcpy(buf, bigBuf, mn);
  rest = last - mn;
  last -= mn;
  if (last)
    memcpy(bigBuf, &bigBuf[mn], rest);
  return mn;
}


/*
int ntread(node, buf, size)
int node;
unsigned char *buf;
int size;
{
  int bt, pt, i;
  pt = 0;
  
  while ((bt = allNetRead(node, &buf[pt], size)) > 0) {
    size -= bt;
    pt += bt;
  }
  return pt;
}
*/

