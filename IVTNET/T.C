#include <time.h>

main()
{
  long t1 = 0x29f5d3d0;
  
  printf("t1 = %d\n%s\n", t1, ctime(&t1));
  t1 = 0x29f5e1e0;
  printf("t1 = %d\n%s\n", t1, ctime(&t1));
}
