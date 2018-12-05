cd target

del *.prg
del *.d64

cd ..\src

cl65 -O -t c64 ultimate_ii.c u-sample.c -o ..\target\u-sample.prg
cl65 -O -t c64 ultimate_ii.c u-shell.c -o ..\target\u-shell.prg
cl65 -O -t c64 ultimate_ii.c ultimateterm.c -o ..\target\u-term64.prg
cl65 -O -t c64 ultimate_ii.c u-chat.c -o ..\target\u-chat64.prg
cl65 -O -t c128 ultimate_ii.c ultimateterm.c -o ..\target\u-term128.prg
cl65 -O -t c128 ultimate_ii.c u-chat.c -o ..\target\u-chat128.prg

del *.o

cd ..\target

c1541 -format "ultimatedemos,sh" d64 ultimatedemos.d64

c1541 -attach ultimatedemos.d64 -write u-shell.prg u-shell
c1541 -attach ultimatedemos.d64 -write u-term64.prg u-term64
c1541 -attach ultimatedemos.d64 -write u-term128.prg u-term128
c1541 -attach ultimatedemos.d64 -write u-term.seq u-term,s
c1541 -attach ultimatedemos.d64 -write u-chat64.prg u-chat64
c1541 -attach ultimatedemos.d64 -write u-chat128.prg u-chat128

cd ..



