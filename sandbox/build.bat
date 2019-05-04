del *.prg
del *.d64
cl65 -O -t c64 ultimate_lib.c net.c -o net.prg
..\c1541 -format "ultimatedemos,sh" d64 net.d64
..\c1541 -attach net.d64 -write net.prg net
