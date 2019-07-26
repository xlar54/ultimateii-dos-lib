#!/bin/bash
rm -rf target/
mkdir target

cd src/samples

cl65 -O -t c64 ../lib/ultimate_lib.c u-sample.c -o ../../target/u-sample.prg
cl65 -O -t c64 ../lib/ultimate_lib.c u-menu.c -o ../../target/u-menu.prg
cl65 -O -t c64 ../lib/ultimate_lib.c u-shell.c -o ../../target/u-shell.prg
cl65 -O -t c64 ../lib/ultimate_lib.c u-echoserver.c -o    ../../target/u-echoserver.prg
cl65 -O -t c64 ../lib/ultimate_lib.c screen_utility.c ultimateterm.c -o ../../target/u-term64.prg
cl65 -O -t c64 ../lib/ultimate_lib.c u-chat.c -o ../../target/u-chat64.prg
cl65 -O -t c128 ../lib/ultimate_lib.c screen_utility.c ultimateterm.c -o ../../target/u-term128.prg
cl65 -O -t c128 ../lib/ultimate_lib.c u-chat.c -o ../../target/u-chat128.prg
cl65 -t geos-cbm -O -o ../../target/geouterm.cvt geouterm-res.grc ../lib/ultimate_lib.c geouterm.c

petcat -w2 -o ../../target/loader.prg loader.bas

rm -f *.o

cd ../../target

DISK_IMAGE=UltimateTerm-and-demos.d64

c1541 -format "ultimatedemos,sh" d64 $DISK_IMAGE \
-write loader.prg loader \
-write u-term64.prg u-term64 \
-write u-term128.prg u-term128 \
-write u-shell.prg u-shell \
-write u-sample.prg u-sample \
-write u-menu.prg u-menu \
-write u-chat64.prg u-chat64 \
-write u-chat128.prg u-chat128 \
-write u-echoserver.prg u-echoserver \
-write geouterm.cvt geoterm.cvt \
-write ../src/samples/cbm40.cvt cbm40.cvt \
-write ../src/samples/cbm80.cvt cbm80.cvt \
-write ../src/samples/u-term.seq u-term,s

rm -f geouterm.cvt loader.prg

cd ..
