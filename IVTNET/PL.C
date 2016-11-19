main()
{
  int bt, r, bt2;
  char buf[256];
  
  sleep(5);
  
  bt = _gs_rdy(0);
  r = read(0, &buf[0], 1);
  
  bt2 = _gs_rdy(0);
  
  printf("r = %d\n", r);
  printf("bt = %d, bt2 = %d, buf[0] = %d\n", bt, bt2, buf[0]);
}
