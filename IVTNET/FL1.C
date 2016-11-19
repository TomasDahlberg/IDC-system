main()
{
  double x, y, z;
  int yy;
  
  yy = 3;
  y = 3;
  
  x = 0;
  x = x + y / 10;
  printf("x = %g (0.3)\n", x);
  
  x = 0;
  x = x + y / 10.0;
  printf("x = %g (0.3)\n", x);
  
  x = 0;
  x = x + yy / 10;
  printf("x = %g (0.0)\n", x);

  x = 0;
  x = x + yy / 10.0;
  printf("x = %g (0.3)\n", x);
}
