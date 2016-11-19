REM Kompilerar RECMOD.C och s„tter revision
REM 910915/da
xcc -I -W=..\LIB -V=..\DEFS -E=1 -BP recmod.c
fixmod -ua=8008 recmod
