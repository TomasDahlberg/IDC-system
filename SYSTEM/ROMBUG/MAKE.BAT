r68 -q -aDEBUGGER boot.a -o=RELS\boot.r
r68 -q -aDEBUGGER sysinit.a -o=RELS\sysinit.r
r68 -q -aDEBUGGER sysboot.a -o=RELS\sysboot.r
r68 -q -aDEBUGGER vectors.a -o=RELS\vectors.r
r68 -q -aDEBUGGER rom3.a -o=RELS\rom.r
r68 -q -aDEBUGGER x.a -o=RELS\x.r
cd RELS
rem l68 -l=..\..\lib\sys.l -l=disasm.l vectors.r boot.r rom.r sysinit.r sysboot.r x.r dbgdown.l -O=rombug -r=380000
l68 -l=..\..\lib\sys.l -l=disasm.l vectors.r boot.r rom.r sysinit.r sysboot.r dbgdown.l -O=rombug -r=380000

