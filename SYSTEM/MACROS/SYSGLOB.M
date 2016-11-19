********************************
* sysglob macro
*   Get system global storage ptr.
*
 ifndef sysglob
sysglob macro
 ifne \#-1
 fail sysglob: requires one (A-reg) argument
 endc
 ifne \L1-2
 fail sysglob: argument must be an A register"
 endc
 ifeq (CPUTyp-68020)*(CPUTyp-68030)
 movec vbr,\1
 movea.l (\1),\1
 else
 movea.l 0.w,\1
 endc 68020/68030 CPUTyp
 endm
 endc sysglob

