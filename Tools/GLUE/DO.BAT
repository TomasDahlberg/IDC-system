echo Tar ut.lis och metamod och skapar output.c
glue ut.lis metamod
bcc -c driver.c
bcc -c func.c
echo
echo l„nkar: 	driver	- generell drivrutin
echo		output	- denna tabell f”r globala variabler vs screen-namn
echo		main_scr- aktuella screen-block
echo		func	- generella funktioner s†som display, enter etc
echo
bcc -DPC -w- driver.obj output.c main_scr.c func.obj


