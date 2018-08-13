# ultimateii-dos-lib
cc65 library for accessing the DOS functions of the Ultimate II+ cartridge

The demo program does not alter any data, but as this is an early release, please use with caution.

Based on ultimate_dos-1.1.docx and command interface.docx
https://github.com/markusC64/1541ultimate2/tree/master/doc

Patches and pull requests are welcome


simple build instructions:

cl65 -O -t c64 ultimate_ii.c main.c -o u2sample.prg