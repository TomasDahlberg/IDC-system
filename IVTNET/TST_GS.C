main(argc, argv)
int argc;
char *argv[];
{
  int nb,nb2,nb3,l, buf[64], m2;
  if (argv[1][0] == '-' && argv[1][1] == 'e')
    m2 =1;
  while (1)
  {
    if ((nb = _gs_rdy(0)) > 0) {
      sleep(2);
      if (m2) {
        nb2 = _gs_rdy(0);
      }
      l = read(0, buf, nb);
      nb3 = _gs_rdy(0);
      printf("nb=%d, nb2=%d, nb3=%d, l=%d\n", nb,nb2,nb3,l);
    }
  }
}
