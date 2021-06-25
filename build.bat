@echo off
cls
del /S/Q target
rd target
md target

cd src\samples

rem cl65 -O -t c64 ..\lib\ultimate_lib.c u-sample.c -o  ..\..\target\u-sample.prg
rem cl65 -O -t c64 ..\lib\ultimate_lib.c u-menu.c -o    ..\..\target\u-menu.prg
rem cl65 -O -t c64 ..\lib\ultimate_lib.c u-shell.c -o   ..\..\target\u-shell.prg
rem cl65 -O -t c64 ..\lib\ultimate_lib.c u-chat.c -o    ..\..\target\u-chat64.prg
rem cl65 -O -t c64 ..\lib\ultimate_lib.c u-echoserver.c -o    ..\..\target\u-echoserver.prg
cl65 -O -t c64 ..\lib\ultimate_lib.c screen_utility.c ultimateterm.c -o  ..\..\target\u-term64.prg
rem cl65 -O -t c128 ..\lib\ultimate_lib.c screen_utility.c ultimateterm.c -o ..\..\target\u-term128.prg
rem cl65 -O -t c128 ..\lib\ultimate_lib.c u-chat.c -o ..\..\target\u-chat128.prg
rem cl65 -t geos-cbm -O -o ..\..\target\geouterm.cvt geouterm-res.grc ..\lib\ultimate_lib.c geouterm.c

cd ..\..

choice /c yn /n /m "Run on Ultimate64 Yes, No?"
if %ERRORLEVEL% == 2 exit /b

u64remote 192.168.7.64 reset
u64remote 192.168.7.64 run target\u-term64.prg


exit /b

del *.o
del ..\lib\*.o

petcat -w2 -o ..\..\target\loader.prg loader.bas

cd ..\..\target

c1541 -format "ultimatedemos,sh" d64 UltimateTerm-and-demos.d64

c1541 -attach UltimateTerm-and-demos.d64 -write loader.prg loader
c1541 -attach UltimateTerm-and-demos.d64 -write u-term64.prg u-term64
c1541 -attach UltimateTerm-and-demos.d64 -write u-term128.prg u-term128
c1541 -attach UltimateTerm-and-demos.d64 -write u-shell.prg u-shell
c1541 -attach UltimateTerm-and-demos.d64 -write u-sample.prg u-sample
c1541 -attach UltimateTerm-and-demos.d64 -write u-menu.prg u-menu
c1541 -attach UltimateTerm-and-demos.d64 -write u-chat64.prg u-chat64
c1541 -attach UltimateTerm-and-demos.d64 -write u-chat128.prg u-chat128
c1541 -attach UltimateTerm-and-demos.d64 -write u-echoserver.prg u-echoserver
c1541 -attach UltimateTerm-and-demos.d64 -write geouterm.cvt geoterm.cvt
c1541 -attach UltimateTerm-and-demos.d64 -write ..\src\samples\cbm40.cvt cbm40.cvt
c1541 -attach UltimateTerm-and-demos.d64 -write ..\src\samples\cbm80.cvt cbm80.cvt
c1541 -attach UltimateTerm-and-demos.d64 -write ..\src\samples\u-term.seq u-term,s

del /Q geouterm.cvt
del /Q loader.prg


