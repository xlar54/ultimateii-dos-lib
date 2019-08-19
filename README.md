# ultimate-lib
cc65 library for accessing the DOS & network functions of the Ultimate 64 and II/+ cartridge

The demo program does not alter any data, but as this is an early release, please use with caution.

Based on ultimate_dos-1.2.docx and command interface.docx
https://github.com/markusC64/1541ultimate2/tree/master/doc

Patches and pull requests are welcome

I have created a simple (dos) batch file to create the
demo programs.  Please have c1541.exe and petcat.exe in your execution
path (they both come with the VICE emulator).

The build will automatically create the prg files as well
as a D64 disk image consisting of the examples.

Example programs:

 * u-sample.prg is a simple demo of making some dos calls
 * u-shell.prg is a simple command shell
 * u-term64.prg is a simple terminal program using the built in ethernet connection
 * u-chat64.prg is an irc client for the 64
 * geouterm is a basic ASCII terminal for GEOS (for both GEOS 64 and 128)

Some programs also have an 80 column C128 version (u-term and u-chat).

Note: these are really just demo applications. You are encouraged to utilize,
improve on, and and build even better apps than I have.  Enjoy!
