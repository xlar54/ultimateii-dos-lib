/*****************************************************************
Screen Utility for C64 (VIC-II) and C128 (VDC)
Francesco Sblendorio

Part of code (VDC memory save/restore) based on mirkosoft's work:
http://commodore128.mirkosoft.sk/vdc.html
http://archive.fo/0i6MI

Patches and pull requests are welcome
******************************************************************/
#ifndef _SAVE_SCREEN_H
#define _SAVE_SCREEN_H

void cursor_on(void);
void cursor_off(void);
void save_screen(void);
void restore_screen(void);

#endif
