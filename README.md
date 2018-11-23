# ultimateii-dos-lib
cc65 library for accessing the DOS functions of the Ultimate II+ cartridge

The demo program does not alter any data, but as this is an early release, please use with caution.

Based on ultimate_dos-1.1.docx and command interface.docx
https://github.com/markusC64/1541ultimate2/tree/master/doc

Patches and pull requests are welcome


simple build instructions:

cl65 -O -t c64 ultimate_ii.c main.c -o u2sample.prg

cl65 -O -t c64 ultimate_ii.c shell.c -o u2sample.prg

cl65 -O -t c64 ultimate_ii.c term.c -o u2term.prg

cl65 -O -t c64 ultimate_ii.c uchat.c -o u2chat.prg

for C128:
cl65 -O -t c128 ultimate_ii.c term.c -o u2term128.prg

cl65 -O -t c128 ultimate_ii.c uchat.c -o u2chat128.prg

Example programs:

u2sample.prg is a simple demo of making some calls

u2shell.prg is a simple command shell

u2term.prg is a simple terminal program using the built in ethernet connection

u2chat.prg is an irc client for the 64