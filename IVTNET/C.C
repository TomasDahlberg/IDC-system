main(argc, argv)
int argc;
char **argv;
{
  int p[20], i, n, n2;
  for (i = 0; i < 20; i++) p[i] = 0;
  
  while( argc >= 2  && argv[1][0] == '-' ) {
    while( *++(argv[1]) ) {
      switch( *argv[1] ) {
        case 'n':
        case 'N':     /* -n=1-3,7-12,14,15,17-22  */
          if (!*++argv[1])
            continue;
          while( *++(argv[1]) ) {
            n = *argv[1]++ - '0';
            if (*argv[1] >= '0')
              n = 10*n + *argv[1]++ - '0';
            if (*argv[1] == '-') {
              argv[1]++;
              n2 = *argv[1]++ - '0';
              if (*argv[1] >= '0')
                n2 = 10*n2 + *argv[1]++ - '0';
              for (i = n; i <= n2; i++)
                p[i] = 1;
              if (!*argv[1]) {
                --argv[1];            /* new */
                break;
              }
              continue;
            } else {
              p[n] = 1;
              if (*argv[1] == ',')
                continue;
              if (!*argv[1]) {
                --argv[1];            /* new */
                break;
              }
            }
	  }
          continue;
        default:
          printf("illegal option: %d\n", *argv[1]);
      }
    }
    argv++;
    argc--;
  }

  for (i = 0; i < 20; i++)
    printf("%2d: %d\n", i, p[i]);
}

