#include <stdio.h>

static unsigned short int opcodeArray[256] =
{
0x6e69, /*4413:Code 6e69 used 20 times*/
0x2054, /*4414:Code 2054 used 20 times*/
0x2348, /*4415:Code 2348 used 20 times*/
0x7201, /*4416:Code 7201 used 20 times*/
0x2073, /*4417:Code 2073 used 20 times*/
0x562e, /*4418:Code 562e used 20 times*/
0x0021, /*4419:Code 0021 used 20 times*/
0x6d70, /*4420:Code 6d70 used 20 times*/
0x2200, /*4421:Code 2200 used 20 times*/
0x0058, /*4422:Code 0058 used 20 times*/
0x00a8, /*4423:Code 00a8 used 21 times*/
0x00a2, /*4424:Code 00a2 used 21 times*/
0xc000, /*4425:Code c000 used 21 times*/
0x0015, /*4426:Code 0015 used 21 times*/
0x2074, /*4427:Code 2074 used 21 times*/
0x4175, /*4428:Code 4175 used 21 times*/
0x673a, /*4429:Code 673a used 21 times*/
0x001f, /*4430:Code 001f used 21 times*/
0x7206, /*4431:Code 7206 used 21 times*/
0x7365, /*4432:Code 7365 used 22 times*/
0x6772, /*4433:Code 6772 used 22 times*/
0x3200, /*4434:Code 3200 used 22 times*/
0x4c42, /*4435:Code 4c42 used 22 times*/
0x0012, /*4436:Code 0012 used 22 times*/
0x2533, /*4437:Code 2533 used 22 times*/
0x0019, /*4438:Code 0019 used 22 times*/
0x6500, /*4439:Code 6500 used 22 times*/
0x7264, /*4440:Code 7264 used 22 times*/
0x6875, /*4441:Code 6875 used 22 times*/
0x342e, /*4442:Code 342e used 22 times*/
0x0030, /*4443:Code 0030 used 23 times*/
0x4236, /*4444:Code 4236 used 23 times*/
0x4cdf, /*4445:Code 4cdf used 23 times*/
0x5635, /*4446:Code 5635 used 23 times*/
0x5380, /*4447:Code 5380 used 23 times*/
0x5f36, /*4448:Code 5f36 used 23 times*/
0x6c00, /*4449:Code 6c00 used 23 times*/
0x0009, /*4450:Code 0009 used 23 times*/
0x3131, /*4451:Code 3131 used 23 times*/
0x41ef, /*4452:Code 41ef used 24 times*/
0x2d40, /*4453:Code 2d40 used 24 times*/
0x7265, /*4454:Code 7265 used 24 times*/
0x670c, /*4455:Code 670c used 24 times*/
0x066e, /*4456:Code 066e used 24 times*/
0x5649, /*4457:Code 5649 used 24 times*/
0x004c, /*4458:Code 004c used 24 times*/
0x7669, /*4459:Code 7669 used 24 times*/
0x6e75, /*4460:Code 6e75 used 24 times*/
0x5265, /*4461:Code 5265 used 24 times*/
0x2069, /*4462:Code 2069 used 25 times*/
0x2047, /*4463:Code 2047 used 25 times*/
0x6574, /*4464:Code 6574 used 25 times*/
0x6e74, /*4465:Code 6e74 used 25 times*/
0x2041, /*4466:Code 2041 used 25 times*/
0x000d, /*4467:Code 000d used 25 times*/
0x332e, /*4468:Code 332e used 25 times*/
0x3635, /*4469:Code 3635 used 25 times*/
0x6964, /*4470:Code 6964 used 25 times*/
0x000a, /*4471:Code 000a used 25 times*/
0x5469, /*4472:Code 5469 used 26 times*/
0x0040, /*4473:Code 0040 used 26 times*/
0x656d, /*4474:Code 656d used 26 times*/
0x4ce9, /*4475:Code 4ce9 used 26 times*/
0x6c20, /*4476:Code 6c20 used 26 times*/
0x4d61, /*4477:Code 4d61 used 26 times*/
0x2040, /*4478:Code 2040 used 27 times*/
0x6e73, /*4479:Code 6e73 used 27 times*/
0x0050, /*4480:Code 0050 used 27 times*/
0x3d25, /*4481:Code 3d25 used 27 times*/
0x0f43, /*4482:Code 0f43 used 27 times*/
0x3530, /*4483:Code 3530 used 27 times*/
0x6616, /*4484:Code 6616 used 27 times*/
0x0013, /*4485:Code 0013 used 27 times*/
0x6600, /*4486:Code 6600 used 27 times*/
0x0476, /*4487:Code 0476 used 27 times*/
0x7465, /*4488:Code 7465 used 28 times*/
0x676c, /*4489:Code 676c used 28 times*/
0x3532, /*4490:Code 3532 used 28 times*/
0x486e, /*4491:Code 486e used 28 times*/
0x5356, /*4492:Code 5356 used 28 times*/
0x0044, /*4493:Code 0044 used 28 times*/
0x48c0, /*4494:Code 48c0 used 28 times*/
0x3120, /*4495:Code 3120 used 29 times*/
0x746f, /*4496:Code 746f used 29 times*/
0x001e, /*4497:Code 001e used 29 times*/
0x6d69, /*4498:Code 6d69 used 29 times*/
0x6720, /*4499:Code 6720 used 29 times*/
0x4631, /*4500:Code 4631 used 29 times*/
0x7269, /*4501:Code 7269 used 29 times*/
0x671c, /*4502:Code 671c used 29 times*/
0x6966, /*4503:Code 6966 used 29 times*/
0x0056, /*4504:Code 0056 used 30 times*/
0x8fee, /*4505:Code 8fee used 30 times*/
0x6665, /*4506:Code 6665 used 30 times*/
0x6626, /*4507:Code 6626 used 30 times*/
0x6e67, /*4508:Code 6e67 used 30 times*/
0x5280, /*4509:Code 5280 used 30 times*/
0x6022, /*4510:Code 6022 used 30 times*/
0x5435, /*4511:Code 5435 used 30 times*/
0x352e, /*4512:Code 352e used 30 times*/
0x7469, /*4513:Code 7469 used 30 times*/
0x7002, /*4514:Code 7002 used 30 times*/
0x006c, /*4515:Code 006c used 31 times*/
0x6420, /*4516:Code 6420 used 31 times*/
0x2208, /*4517:Code 2208 used 31 times*/
0x726d, /*4518:Code 726d used 31 times*/
0x000e, /*4519:Code 000e used 31 times*/
0x7420, /*4520:Code 7420 used 31 times*/
0x723c, /*4521:Code 723c used 32 times*/
0x2e31, /*4522:Code 2e31 used 32 times*/
0x7574, /*4523:Code 7574 used 33 times*/
0x000f, /*4524:Code 000f used 33 times*/
0x616c, /*4525:Code 616c used 33 times*/
0x0047, /*4526:Code 0047 used 34 times*/
0x3a20, /*4527:Code 3a20 used 34 times*/
0x6465, /*4528:Code 6465 used 34 times*/
0xb0a8, /*4529:Code b0a8 used 34 times*/
0xffb0, /*4530:Code ffb0 used 35 times*/
0x588f, /*4531:Code 588f used 35 times*/
0x0006, /*4532:Code 0006 used 36 times*/
0x0017, /*4533:Code 0017 used 36 times*/
0x6080, /*4534:Code 6080 used 36 times*/
0x3066, /*4535:Code 3066 used 36 times*/
0x3100, /*4536:Code 3100 used 36 times*/
0x6520, /*4537:Code 6520 used 37 times*/
0x001c, /*4538:Code 001c used 37 times*/
0x42ae, /*4539:Code 42ae used 37 times*/
0x3020, /*4540:Code 3020 used 37 times*/
0x7261, /*4541:Code 7261 used 37 times*/
0x0d00, /*4542:Code 0d00 used 38 times*/
0x3166, /*4543:Code 3166 used 38 times*/
0x305f, /*4544:Code 305f used 38 times*/
0x6708, /*4545:Code 6708 used 38 times*/
0x6174, /*4546:Code 6174 used 38 times*/
0x0053, /*4547:Code 0053 used 38 times*/
0x6700, /*4548:Code 6700 used 39 times*/
0x41ee, /*4549:Code 41ee used 39 times*/
0x4880, /*4550:Code 4880 used 39 times*/
0x7270, /*4551:Code 7270 used 40 times*/
0x4aae, /*4552:Code 4aae used 40 times*/
0x2057, /*4553:Code 2057 used 40 times*/
0x2f00, /*4554:Code 2f00 used 41 times*/
0x7400, /*4555:Code 7400 used 41 times*/
0x0800, /*4556:Code 0800 used 41 times*/
0x0054, /*4557:Code 0054 used 41 times*/
0x6604, /*4558:Code 6604 used 42 times*/
0x206d, /*4559:Code 206d used 42 times*/
0x6567, /*4560:Code 6567 used 42 times*/
0x002c, /*4561:Code 002c used 43 times*/
0x2017, /*4562:Code 2017 used 43 times*/
0x6620, /*4563:Code 6620 used 44 times*/
0x000b, /*4564:Code 000b used 44 times*/
0x243c, /*4565:Code 243c used 44 times*/
0x6674, /*4566:Code 6674 used 45 times*/
0x4cd7, /*4567:Code 4cd7 used 45 times*/
0x6c61, /*4568:Code 6c61 used 46 times*/
0x2368, /*4569:Code 2368 used 46 times*/
0x7220, /*4570:Code 7220 used 46 times*/
0x696c, /*4571:Code 696c used 48 times*/
0x0016, /*4572:Code 0016 used 48 times*/
0xff9c, /*4573:Code ff9c used 49 times*/
0x0c00, /*4574:Code 0c00 used 50 times*/
0x518f, /*4575:Code 518f used 51 times*/
0x8080, /*4576:Code 8080 used 51 times*/
0x616e, /*4577:Code 616e used 51 times*/
0xffbc, /*4578:Code ffbc used 52 times*/
0x7600, /*4579:Code 7600 used 52 times*/
0x7200, /*4580:Code 7200 used 53 times*/
0x7374, /*4581:Code 7374 used 55 times*/
0x2a2a, /*4582:Code 2a2a used 55 times*/
0x2025, /*4583:Code 2025 used 56 times*/
0x0005, /*4584:Code 0005 used 56 times*/
0x656c, /*4585:Code 656c used 57 times*/
0x203d, /*4586:Code 203d used 58 times*/
0x4754, /*4587:Code 4754 used 58 times*/
0x203a, /*4588:Code 203a used 59 times*/
0x42a7, /*4589:Code 42a7 used 59 times*/
0x7001, /*4590:Code 7001 used 59 times*/
0xffa4, /*4591:Code ffa4 used 62 times*/
0x4228, /*4592:Code 4228 used 62 times*/
0x0024, /*4593:Code 0024 used 63 times*/
0x6532, /*4594:Code 6532 used 64 times*/
0x8620, /*4595:Code 8620 used 64 times*/
0x2140, /*4596:Code 2140 used 64 times*/
0x222e, /*4597:Code 222e used 65 times*/
0x6000, /*4598:Code 6000 used 67 times*/
0x315f, /*4599:Code 315f used 67 times*/
0x4e45, /*4600:Code 4e45 used 68 times*/
0x3d20, /*4601:Code 3d20 used 69 times*/
0x4e40, /*4602:Code 4e40 used 71 times*/
0xfffc, /*4603:Code fffc used 71 times*/
0x7275, /*4604:Code 7275 used 72 times*/
0x802a, /*4605:Code 802a used 72 times*/
0x5f31, /*4606:Code 5f31 used 75 times*/
0x226e, /*4607:Code 226e used 76 times*/
0x0100, /*4608:Code 0100 used 76 times*/
0x756c, /*4609:Code 756c used 79 times*/
0x7000, /*4610:Code 7000 used 82 times*/
0x6172, /*4611:Code 6172 used 82 times*/
0x0020, /*4612:Code 0020 used 83 times*/
0x202f, /*4613:Code 202f used 84 times*/
0x6572, /*4614:Code 6572 used 85 times*/
0x0072, /*4615:Code 0072 used 85 times*/
0x696e, /*4616:Code 696e used 85 times*/
0x325f, /*4617:Code 325f used 86 times*/
0x4a28, /*4618:Code 4a28 used 91 times*/
0x48d7, /*4619:Code 48d7 used 92 times*/
0x0010, /*4620:Code 0010 used 93 times*/
0x0002, /*4621:Code 0002 used 100 times*/
0x206f, /*4622:Code 206f used 100 times*/
0x48e8, /*4623:Code 48e8 used 104 times*/
0x202e, /*4624:Code 202e used 107 times*/
0x4ce8, /*4625:Code 4ce8 used 109 times*/
0x6c65, /*4626:Code 6c65 used 110 times*/
0x6c6c, /*4627:Code 6c6c used 114 times*/
0x2020, /*4628:Code 2020 used 121 times*/
0x508f, /*4629:Code 508f used 123 times*/
0x2028, /*4630:Code 2028 used 124 times*/
0x4a80, /*4631:Code 4a80 used 128 times*/
0x0102, /*4632:Code 0102 used 131 times*/
0x802e, /*4633:Code 802e used 132 times*/
0xfff8, /*4634:Code fff8 used 136 times*/
0xc080, /*4635:Code c080 used 137 times*/
0x2f2f, /*4636:Code 2f2f used 141 times*/
0x0018, /*4637:Code 0018 used 149 times*/
0x4aa8, /*4638:Code 4aa8 used 179 times*/
0x4e4f, /*4639:Code 4e4f used 179 times*/
0x217c, /*4640:Code 217c used 187 times*/
0x0007, /*4641:Code 0007 used 201 times*/
0x4e4d, /*4642:Code 4e4d used 210 times*/
0x000c, /*4643:Code 000c used 216 times*/
0x0014, /*4644:Code 0014 used 223 times*/
0x4ced, /*4645:Code 4ced used 232 times*/
0x0001, /*4646:Code 0001 used 242 times*/
0x05ee, /*4647:Code 05ee used 242 times*/
0x4e5d, /*4648:Code 4e5d used 243 times*/
0x203c, /*4649:Code 203c used 243 times*/
0x05ea, /*4650:Code 05ea used 243 times*/
0x42a8, /*4651:Code 42a8 used 251 times*/
0x4e46, /*4652:Code 4e46 used 262 times*/
0x4fef, /*4653:Code 4fef used 264 times*/
0x2008, /*4654:Code 2008 used 275 times*/
0xffff, /*4655:Code ffff used 290 times*/
0x4e55, /*4656:Code 4e55 used 290 times*/
0x4e75, /*4657:Code 4e75 used 303 times*/
0x48e7, /*4658:Code 48e7 used 309 times*/
0x4878, /*4659:Code 4878 used 324 times*/
0x0004, /*4660:Code 0004 used 325 times*/
0x41fa, /*4661:Code 41fa used 342 times*/
0x0008, /*4662:Code 0008 used 370 times*/
0x0003, /*4663:Code 0003 used 388 times*/
0x2f28, /*4664:Code 2f28 used 858 times*/
0x6100, /*4665:Code 6100 used 993 times*/
0x801e, /*4666:Code 801e used 1581 times*/
0x206e, /*4667:Code 206e used 1600 times*/
0x0000 /*4668:Code 0000 used 1618 times*/
/* Sum of 4413-4668 is 23543 */
};
#ifdef TEST
main(int argc, char *argv[])
{
  FILE *fp1, *fp2;
  int n, utlen;
  unsigned char buf[256], utbuf[512];
  long t1, t2;

  if (argc != 3) {
    printf("usage: decmprs in-file out-file\n");
    exit(1);
  }
  fp1 = fopen(argv[1], "rb");
  fp2 = fopen(argv[2], "wb");
  if (fp1 == 0 || fp2 == 0) {
    printf("Error\n");
    exit(1);
  }
  t1 = time(0);            /* 9, 243, */
  while (n = fread(buf, 1, 243, fp1)) {
    utlen = unpack(buf, n, utbuf);
    fwrite(utbuf, 1, utlen, fp2);
  }  /* while */
  t2 = time(0);
  fclose(fp2);
  printf("Elapsed time is %d seconds\n", (int) (t2 - t1));
}
#endif
static int get9bitsBuf(s, l, bitNo)
unsigned char *s;
long l;
long bitNo;
{
  unsigned short int q, byte, sh;
  if ((bitNo + 9) > (l << 3))		/* does all 9bits exist ? */
    return -1;

  byte = bitNo / 8;
  q = (s[byte] << 8) | s[byte+1];

  sh = 7 - (bitNo & 7);
  return (q >> sh) & 0x1ff;
}

int unpack(inbuf, inlen, utbuf)
char *inbuf;
int inlen;
char *utbuf;
{
  int utlen, code;
  long bitNo;
  unsigned short int tal;

  utlen = 0;
  bitNo = 0;
  while ((code = get9bitsBuf(inbuf, inlen, bitNo)) >= 0) {
    bitNo += 9;
    if (code < 256)
      utbuf[utlen++] = code;
    else {
      tal = opcodeArray[code - 256];
      utbuf[utlen++] = tal >> 8;
      utbuf[utlen++] = tal & 0xff;
    }
  }  /* while */
  return utlen;
}

