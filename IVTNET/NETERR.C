extern int errno;

main()
{
  int netpath, i, sz;
  
  if ((netpath = open("/nb", 0x03)) == -1)
    exit(errno);
    
  while (1) {
    for (i = 1; i <= 2; i++) {
      _ss_dcon(netpath, i);
      sz = _gs_rdy(netpath);
      printf("node %d, size %d error %d\n", i, sz, errno);
      errno = 0;
    }
  }
}
