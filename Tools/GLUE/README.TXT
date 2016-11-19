Displaydriver-DLL
-----------------

IDCC genererar en utfil med formatet <screen-namn>:<variabelnamn>
welcome:level
TA1_temp_1:TA1_GT31
TA1_temp_1_1:TA1_GT31_korr
TA1_temp_2:TA1_GT32
TA1_temp_2_1:TA1_GT32_korr
etc

Denna fil tillsammans med metamod anv„nds som indata till programmet GLUE.

C:\> glue utfil.lis metamod 

genererar en fil output.c som l„nkas tillsammans med main_screen.c och 
en generell fil drivern.obj

C:\> bcc <options> driver.obj output.c main_scr.c

Resultatet blir en windows-dll.

Allts†: fr†n IDCC genereras en ny fil, denna tillsammans med main_screen.c
och main_symtable.h kompileras till en dll. Detta sker automatiskt
genom att till CCOMPILE.BAT skicka med n†gon option exempelvis

C:\> ccompile main main dll

N„r alla ducar „r klara k”rs en bat-fil, MAKEWIN, som tar varje dll och metamod
fil i varje underkatalog med namnet duc<nr>\ och d”per om dessa till

metamod.<nr>
duc<nr>.dll

exempelvis duc2 och duc3 genererar filerna

metamod.2
duc2.dll
metamod.3
duc3.dll

Kommunikations-dll
------------------
Win3208.DLL kommunicerar med visioncom i ducen och hanterar 
variabelf”rfr†gningar, s„tter variabler, h„mtar och s„tter kalendrar,
l„ser larm, kvitterar larm. Dvs motsvarar DDE-servern till InTouch eller
DLL-drivern till Citect. (Anv„nda Citects DLL ?)

Windowsprogrammet 
-----------------
Programmet Win3208.EXE anv„nder Win3208.DLL och filer med namnet
Metamod.1, Duc1.DLL, Metamod.2, Duc2.DLL etc.

Se f”ljande exempel:	\borlandc\owl\examples\paint 

