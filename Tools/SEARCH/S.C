long *s = 0x380000;

long p = 0x00040000;


main()
{
  int i;
  for (i = 0; 5000; i++, s++) {
    if (*s == p) {
      printf("At %6x we found %x\n", s, p);
    }
  }
}
