static unsigned char bigBuf[2048];      /* 1024 -> 2048 920331 */
static int last, currentNode = -1;
extern int verbose;

emptyNode(node)
int node;
{
  int nb, nr, tot;

  tot = 0;
  while (tot < 10000) {
    nb = netpoll(node);
    if (nb > 0) {
      tot += nb;
      if (nb > 1000)
        nb = 1000;
      nr = netread(node, &bigBuf[0], nb);
    } else 
      break;
  }
}

emptyBuffer()
{
  if (verbose) printf("emptyBuffer: node = %d, buffer size %d bytes!!\n", currentNode, last);
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
      if (verbose) printf("myNetPoll: Error, switching node %d->%d while buffer not empty!\n", currentNode, node);
    currentNode = node;
    last = 0;
    return netpoll(node);
  }
}

bufPoll(node)
int node;
{
  return last;
}

doNetRead(node)
int node;
{
  int nb, nr;

  if (verbose) printf("       doNetRead node %d ", node);  
  nb = netpoll(node);
  if (nb > 0) {
    if ((nb + last) < 1024) {
      nr = netread(node, &bigBuf[last], nb);
      if (verbose) printf("adding %d bytes (%d)\n", nb, nr);
      if (nr == -1) {
        if (verbose) printf("doNetRead: error ...... -1 returned from netread()\n");
        return -1;
      }
      last += nr;
    } else {
      if (verbose) printf("bigBuf full\n");
      return 0;
    }
  } else
    if (verbose) printf("no additional bytes read\n");
  return nb;
}

fetchNodeData(node)
int node;
{
  int tot = 0, nb;
  
  while ((nb = doNetRead(node)) > 0)
    tot += nb;
  return tot;
}

allNetRead(node, buf, size)
int node;
char *buf;
int size;
{
  int mn, rest, nb;
  
  if (verbose) printf("allNetRead: read %d bytes.\n", size);

  if (last == 0) {
    if (verbose) printf("allNetRead: No data in buffer !! returns 0\n");
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

