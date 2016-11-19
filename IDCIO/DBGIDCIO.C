static char *debugPtr = 0x3ffb0;
static char *debugRelease = 0x3ffb1;
/*
  if (*debugPtr) {
    printf("get_ad: module %d, chn %d, value = $%xmV\n", module, chn, bvolt);
    *debugRelease = 0;
    while (*debugRelease == 0)
      ;
  }
*/

void icp(s)
int s;
{
  *debugPtr = 0;
  *debugRelease = 1;
  
  exit(0);
}

main()
{
  int go;
  
  intercept(icp);
  printf("dbgIdcio:\ng    - go mode\ns    - step mode\nn    - next\n\n");
  go = 0;
  *debugPtr = 1;
  while (1) {
    if (_gs_rdy(0) > 0) {
      char c;
      read(0, &c, 1);
     
      if (c == 'g')             /* go */
        go = 1;
      else if (c == 's')        /* step */
        go = 0;
      else if (c == 'n')        /* next */
        *debugRelease = 1;
    }
    if (go) {
      *debugRelease = 1;
    }
  }
}

