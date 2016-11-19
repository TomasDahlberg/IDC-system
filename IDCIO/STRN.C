main()
{
  char a[10], b[10];
  
  a[0] = 65;
  a[1] = 66;
  a[2] = 67;
  
  b[0] = 1;
  b[1] = 2;
  b[2] = 3;
  strncpy(a, b, 2);

  printf("%d, %d, %d\n", a[0], a[1], a[2]);  
}
