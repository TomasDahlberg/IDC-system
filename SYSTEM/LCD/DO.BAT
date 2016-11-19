r68 -o=lcd.r lcd.a
l68 ..\lib\scfstat.l lcd.r -o=lcd -l=\os9c\lib\sys.l
fixmod -uo=0.0 lcd
