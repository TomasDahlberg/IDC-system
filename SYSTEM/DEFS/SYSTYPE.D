* System Definitions for Motorola VME110 system
 opt -l
* File: SYSTYPE.D
********************************
* Edition History
*   date    comments                                            by
* -------- ---------------------------------------------------- ---
* 11-01-84 Returned I/O port addresses to systype file.         rfd
*          Added configuration module strings.                  rfd
* 11-05-84 Changed names of A_Term, Term_Par to match boot.a    wgp
* 07-10-85 Changed serial macros for new Type byte standards    res
* 10-24-86 Changed configuration parameters for use with V2.0   djl
* 05-22-87 Added CPUTyp definition                              wwb
* 06-01-87 Added CPUType, ClocFreq (sc8x30) definition          wwb
*          <<<---- OS-9/68000 V2.1 Release ---->>>
*          <<<---- OS-9/68000 V2.2 Release ---->>>
* 03-20-89 removed labels that were removed from "oskdefs.d".   wwb
* 03-21-89 defined DevCon for scf descriptors (new scfdesc.a).  wwb
* 03-22-89 added U0 disk macro.                                 wwb
*          <<<---- OS-9/68000 V2.3 Release ---->>>

* 90-11-20 added M2 modemport macro, 2400 baud /da
* 90-12-09 added N2, as M2 but no echo /da
* 91-01-06 added M248, echo and 4800 baud /da
* 91-01-06 added N248, no echo and 4800 baud /da
* 91-01-27 added N296, no echo and 9600 baud /da
* 91-01-27 added N224, no echo and 2400 baud /da
* 91-01-27 added M212, echo and 1200 baud /da
* 91-01-27 added M224, echo and 2400 baud, as M2 for consistency /da
* 91-11-16 added NT14, NT19, NT28, NT57, NT115 new baudrates for network /da

********************************
* Hardware type definitions

VME140 equ 1
VME110 equ 110 default value: MVME110 cpu
ClocFreq equ 100 10MHz clock (used by sc8x30.a only)
SerType  equ VME400
IOType   equ 1 value from "odd" in oskdefs.d
IRQ_used equ 1 define the label
CPUTyp set 68000 mpu definition
CPUType set VME110 define cpu module in use

ClkPort equ $00340000 clock port address
ClkVect set 30 clock vector number (level 6 autovector)
ClkMask set $600 clock irq mask level
ClkPrior set 1 high polling table priority
TicksSec set 100 ticks per second

*******************************
* Configuration module constants
*    used only by init module

CONFIG macro
MainFram dc.b "DT08",0
SysStart dc.b "sysgon",0 name of initial module to execute
SysParam dc.b C$CR,0 parameter to SysStart
SysDev   dc.b 0 initial system disk pathlist
ConsolNm dc.b "/term",0  console terminal pathlist
ClockNm  dc.b "ck82c684",0 clock module name
 endm
* (Other default values may be reset here)

********************************
* SCF device descriptor definitions
*    used only by scf device descriptor modules

* SCFDesc: Port,Vector,IRQlevel,Priority,Parity,BaudRate,DriverName

TERM macro
* default descriptor values can be changed here
 SCFDesc $00340000,30,6,5,0,14,sc82c684
pagpause equ OFF
DevCon set 0 no device specific constants
 endm

LCDTERM macro
 SCFDesc $00308000,30,6,255,0,15,lcd
pagpause equ OFF
DevCon set 0 no device specific constants
 endm

T0 macro
 SCFDesc $00340000,30,6,5,0,14,sc82c684
DevCon set 0 no device specific constants
 endm

T1 macro
 SCFDesc $00340008,30,6,240,0,14,sc82c684
pagpause equ OFF
DevCon set 0 no device specific constants
 endm

T2 macro
 SCFDesc $00340010,30,6,249,0,14,sc82c684
pagpause equ OFF
DevCon set 0 no device specific constants
 endm

NT96 macro
 SCFDesc $00340018,30,6,250,2,14,nt82c684
pagpause equ OFF
DevCon set 0 no device specific constants
 endm

NT14 macro
 SCFDesc $00340018,30,6,250,2,0,nt82c684
pagpause equ OFF
DevCon set 0 no device specific constants
 endm

NT19 macro
 SCFDesc $00340018,30,6,250,2,15,nt82c684
pagpause equ OFF
DevCon set 0 no device specific constants
 endm

NT28 macro
 SCFDesc $00340018,30,6,250,2,1,nt82c684
pagpause equ OFF
DevCon set 0 no device specific constants
 endm

NT57 macro
 SCFDesc $00340018,30,6,250,2,2,nt82c684
pagpause equ OFF
DevCon set 0 no device specific constants
 endm

NT115 macro
 SCFDesc $00340018,30,6,250,2,3,nt82c684
pagpause equ OFF
DevCon set 0 no device specific constants
 endm


M2 macro
 SCFDesc $00340010,30,6,249,0,10,sc82c684
pagpause equ OFF
DevCon set 0 no device specific constants
 endm

N2 macro
 SCFDesc $00340010,30,6,249,0,10,sc82c684
pagpause equ OFF
autoecho equ OFF
DevCon set 0 no device specific constants
 endm

M212 macro
 SCFDesc $00340010,30,6,249,0,7,sc82c684
pagpause equ OFF
DevCon set 0 no device specific constants
 endm

M224 macro
 SCFDesc $00340010,30,6,249,0,10,sc82c684
pagpause equ OFF
DevCon set 0 no device specific constants
 endm

M248 macro
 SCFDesc $00340010,30,6,249,0,12,sc82c684
pagpause equ OFF
DevCon set 0 no device specific constants
 endm

N212 macro
 SCFDesc $00340010,30,6,249,0,7,sc82c684
pagpause equ OFF
autoecho equ OFF
DevCon set 0 no device specific constants
 endm

N224 macro
 SCFDesc $00340010,30,6,249,0,10,sc82c684
pagpause equ OFF
autoecho equ OFF
DevCon set 0 no device specific constants
 endm

N248 macro
 SCFDesc $00340010,30,6,249,0,12,sc82c684
pagpause equ OFF
autoecho equ OFF
DevCon set 0 no device specific constants
 endm

N296 macro
 SCFDesc $00340010,30,6,249,0,14,sc82c684
pagpause equ OFF
autoecho equ OFF
DevCon set 0 no device specific constants
 endm

P1 macro
 SCFDesc $00340008,30,6,250,0,14,sc82c684
DevCon set 0 no device specific constants
 endm

MD macro
 SCFDesc $00340000,30,6,254,0,0,modload
DevCon set 0 no device specific constants
 endm

****************************************************
* RBF device descriptor definitions

* RBFDesc: Port,Vector,IRQLevel,Priority,DriverName


DiskD0 macro
 RBFDesc $00FF7FF1,64,2,2,rb765,dd580
* Default disk values may be changed here
BitDns set Double
SectTrk0 set 10
DnsTrk0 set Single
StepRate set 3
DevCon set 0
 endm

DiskU0 macro
 RBFDesc $00FF7FF1,64,2,2,rb765,uv580
* Default disk values may be changed here
StepRate set 3
DevCon set 0
 endm

DiskR0 macro
 RBFDesc 0,0,0,0,ram,ramdisk
SectTrk set 16
Cylnders set 16
 endm

DiskR1 macro
 RBFDesc $30000,0,0,0,nvram,nvramdisk
SectTrk set 16
Cylnders set 16
 endm

DiskR2 macro
 RBFDesc 0,0,0,0,ncram,ramdisk
SectTrk set 16
Cylnders set 16
 endm

DiskNBF macro
 RBFDesc $00340018,30,2,10,nbf,nvramdisk
SectTrk set 16
Cylnders set 16
 endm

DiskDU macro
 RBFDesc $00FF7FF1,64,2,2,rb765,dd580
* Default disk values may be changed here
BitDns set Double
SectTrk0 set 16
DnsTrk0 set Double
SectOffs set 1
StepRate set 3
DevCon set 0
 endm

DiskD1 macro
 RBFDesc $00FF7FF1,64,2,2,rb765,dd580
* Default disk values may be changed here
BitDns set Double
SectTrk0 set 16
DnsTrk0 set Double
StepRate set 3
DevCon set 0
 endm

DiskH0 macro
 RBFDesc $00FFBFF1,67,2,2,rb8500
DiskKind  set Hard
BitDns    set Single
Heads     set 4
StepRate  set 7 allow enhanced seek speed
TrkDns    set Single
Cylnders  set 615
Heads     set 4
NoVerify  set OFF
SectTrk   set 32
SectTrk0  set 32 
Intrleav  set 3
DnsTrk0   set Single
Density   set BitDns+(TrkDns<<1)
DiskType  set DiskKind+(DnsTrk0<<5)
Control   set MultEnabl+FmtDsabl

DevCon dc.w PrmSize
 dc.b $01 # of cylinders (high byte)
 dc.b $32 # of cylinders (low byte)
 dc.b Heads # of heads
 dc.b $01 cylinder # to begin reduced write (high byte)
 dc.b $40 cylinder # to begin reduced write (low byte)
 dc.b $00 cylinder to begin write precomp (high byte)
 dc.b $C8 cylinder to begin write precomp (low byte)
 dc.b $0B ecc burst length
PrmSize equ *-DevCon
 endm


********************************
* These definitions are only used by the bootstrap ROM source
* This configuration is for a VME110 cpu and a VME400 serial card.

* inclusion of debugger code is controlled by defining the label
* DEBUGGER.  For maximum flexibility, this can be done from the
* assembler cmd line (e.g. -aDEBUGGER).

ConsType equ MC68681
CommType equ MC68681

RAMVects equ 1 exception vectors are RAM
PARITY   equ 0 1 Memory with Parity may be used

Mem.Beg  equ $1000 start of normal memory area to search
Mem.End  equ $20000 end of normal memory area to search
Spc.Beg  equ $20000 beginning of special area to search
Spc.End  equ $40000 end of special area to search
Cons_Adr equ $00340000 console device address
Cons_Par equ 0 console device parity iniz
Comm_Adr equ $00340010 auxilliary device address
Comm_Par equ 0 auxilliary device parity iniz
FDPort    equ  $00FF7FF1
FD_Vct    equ 64
SysDisk equ FDPort this equate is to make boot.a generic
FDsk_Vct equ FD_Vct
ModCntl  equ $FE8021 address of module control register

MemDefs macro  
 dc.l Mem.Beg,Mem.End the normal memory search list
 dc.l $40000,$300000 the normal expansion memory search list
 dc.l 0
 dc.l Spc.Beg,Spc.End the special memory search list
 dc.l $380000,$400000
 dc.l 0,0,0,0,0,0 free bytes for patching
 endm

 opt l

