#!/bin/bash

rm -f seqdisk.d64
rm -f seqdisk.prg
cl65 -O -t c64 ultimate_lib.c seq.c -o seq.prg

rm -f *.o
rm -f *.inf

c1541 -format "seqdisk,sh" d64 seqdisk.d64

c1541 -attach seqdisk.d64 -write seq.prg seq
c1541 -attach seqdisk.d64 -write u-term.seq u-term,s
