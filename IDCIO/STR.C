struct _remote { long timeStamp; };

struct {
  int (*display)();
  double (*enter)();
  int level;
  double p;
  struct _remote _remoteVar;
  int x;

} k;



main()
{
  printf("k = %d\n", &k);
  printf("k.1 = %d\n", &k.display);
  printf("k.2 = %d\n", &k.enter);
  printf("k.3 = %d\n", &k.level);
  printf("k.4 = %d\n", &k.p);
  printf("k.5 = %d\n", &k._remoteVar);
  printf("k.6 = %d\n", &k.x);
}
