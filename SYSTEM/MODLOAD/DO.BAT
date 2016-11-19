r68 -o=modload.r modload.a
l68 ..\lib\scfstat.l modload.r -o=modload -l=\os9c\lib\sys.l
fixmod -uo=0.0 modload
