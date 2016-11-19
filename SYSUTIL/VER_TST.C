main(a,b)
int a;
char **b;
{
  int sts, us;
  
  initsysutil();
  if (a == 1) {
    sts = verify();
    printf("verify returns sts= %d\n", sts);
  } else if (a == 2) {
    us = atoi(b[1]);
    printf("Waiting %d us\n", us);
    wait_us(us);
  } else if (a == 5) {
    printf("Calling rs232_DTR function\n");
/*
!   ch  1 - port 1
!       2 - port 2
!
!   length, length to assert in us
!
!   typ 0 - no address, normal data
!       1 - no address, month data
!       2 - emit address
!
!   address
*/
    printf("rs232_DTR(%d, %d, %d, %d)\n", 
                  atoi(b[1]), atoi(b[2]), atoi(b[3]), atoi(b[4]));
    rs232_DTR(atoi(b[1]), atoi(b[2]), atoi(b[3]), atoi(b[4]));
    printf("Ok\n");
  } else {
    printf("Doing a reboot\n");
    reboot();
    printf("nooo... we never reach here, do we ?\n");
  }
}
