
int ntread(node, buf, size)
int node;
char *buf;
int size;
{
  int bt, pt;
  pt = 0;
  while ((bt = netread(node, &buf[pt], size)) > 0) {
/*
    printf("ntread: size = %d, bt = %d, -> more = %d\n", size, bt, size - bt);
*/    
    size -= bt;
    pt += bt;
  }
  return pt;
}

int getPacket(buf, size)
unsigned char *buf;               /* new 920313 */
int size;
{
  int pt, more, bt, sz;

  ntread(0, &buf[0], 1);
  size --;
  more = buf[0] - 1;
  pt = 1;
  while (more > 0) {
    while (size && more) {
      sz = (more > size) ? size : more;
      bt = ntread(0, &buf[pt], sz);
/*
    printf("gP: sz = %d, bt = %d, -> more = %d\n", sz, bt, sz - bt);
*/
      pt += bt; more -= bt; size -= bt;
    }
    if (more && ((size = netpoll(0)) == -1)) {
/*      return 0;   */
      if (DEBUG)
        printf("getPacket: ??? error ?? (bt = %d)\n", bt);
      size = 0;
    }
  }
  if (DEBUG)
    printf("getPacket: bt was %d\n", bt);
  return 1;
}

int dumpPacket(buf)
unsigned char *buf;
{
  int i;

  printf("Packet size %d bytes, protocol version %d\n", buf[0], buf[1]);
  printf("To node %d, from node %d, command %d\n", buf[3], buf[5], buf[6]);

  printf("Packet data:\n");
  for (i = 7; i < buf[0]; i++)
    printf("%02x, ", buf[i]);
  printf("\n");
}

int layer2()
{
  int i, bt, nb;
  int cnt = 0;
  
  bt = 0;


  if ((nb = netpoll(0)) > 0) {          /* removed new 920313 */
/*
    nb = netpoll(0);                    
    nb = netpoll(0);
*/
    getPacket(&receiver, nb);
    if (DEBUG)
      printf("Efter: nb = %d\n", nb);
  }
  else
    return _idle;

  if (DEBUG)
    dumpPacket(&receiver);

    if (allBlockChs(&receiver))        /* should be zero */
    {  /* wrong checksum, reply with previously correct packet received */

      if (DEBUG) printf("wrong checksum !\n");
      return _idle;
    }


  return _packetReceived;
   
} /* end of layer2 */

