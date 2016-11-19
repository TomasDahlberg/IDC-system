copy merge/b+pid.r+mstime.r+datamodule.r+redday.r+runtime_scan.r util1.r
copy findmeta.r/b+trap_screen.r+var.r+rand.r util2.r
copy counter.r/b+enter.r+timer.r+kurvab.r+tvpump.r+week.r+net.r util3.r
copy util1.r/b+util2.r+util3.r util.r
copy \idc\lib\phyio.l/b+\os9c\lib\cio.l+\os9c\lib\clib.l+\os9c\lib\sys.l lib.l
l68 traphand.r util.r lib.l

rem l68 traphand.r util1.r util2.r util3.r -l=/idc/lib/phyio.l -l=/os9c/lib/cio.l -l=/os9c/lib/clib.l -l=/os9c/lib/sys.l -O=idcio
