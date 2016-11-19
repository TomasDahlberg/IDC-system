int m=1;
int oldBit[8];
int dp[8];

double Ni(a)
int a;
{
  double Ni1000LG(), uin;
  
  uin = a*10000.0 / 1023.0;
  return Ni1000LG(0.0, uin, 0.0);
}

main(a,b)
int a;
char *b[];
{
  int i;

  initphyio();
  initidcio();
  for (i = 0; i < 8; i++) {
      oldBit[i]  = get_ad(m, i) * 1023 / 10000;
      dp[i] = 0;
  }
  while(1) {
    for (i = 0; i < 8; i++) {
      int fel;
      int bit;

      bit  = get_ad(m, i) * 1023 / 10000;
      fel = bit - oldBit[i];
      if (abs(fel) <= 3) {
	dp[i] = (dp[i]*15 >> 4) + fel;
	bit = oldBit[i] + (dp[i] >> 4);
      } else {
	oldBit[i] = bit;
	dp[i] = 0;
      }
      printf("%6.2f ", Ni(bit));
    }
    printf("\n");
  }
}  
