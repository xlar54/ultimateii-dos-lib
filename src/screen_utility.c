#include <conio.h>
#include <string.h>
#include <peekpoke.h>
#ifdef __C64__
#include <c64.h>
#else
#include <c128.h>
#endif

#include "screen_utility.h"

#ifdef __C64__
#define CHARCOLOR 646
unsigned char vic_chars[1000], vic_color[1000], scr_saved_lowcase;
#else
#define CHARCOLOR 241
unsigned char vdc_saved_screen[4096];
unsigned char *ptr_vdc_screen = vdc_saved_screen;
unsigned char vdc_x_w, vdc_y_w, vdc_x_r, vdc_y_r, vdc_temp;
void vdc_write_reg(void);
#endif
unsigned char scr_saved_x, scr_saved_y, scr_saved_c;

#ifdef __C64__
void save_screen() {
	scr_saved_x = wherex();
	scr_saved_y = wherey();
	scr_saved_c = PEEK(CHARCOLOR);
	scr_saved_lowcase = PEEK(0xD018);
	memcpy(vic_chars, (void*)0x0400, 1000);
	memcpy(vic_color, (void*)0xD800, 1000);
}

void restore_screen() {
	memcpy((void*)0xD800, vic_color, 1000);
	POKE(0xD018, scr_saved_lowcase);
	memcpy((void*)0x0400, vic_chars, 1000);
	gotoxy(scr_saved_x, scr_saved_y);
	POKE(CHARCOLOR, scr_saved_c);
}
#else
#pragma optimize (push,off)
void vdc_prepare(void) {
	asm("lda #$12");
	asm("sta $d600");
	asm("lda %v+1", ptr_vdc_screen);
	asm("sta $d601");
	asm("lda #$13");
	asm("sta $d600");
	asm("lda %v", ptr_vdc_screen);
	asm("sta $d601");
	asm("lda #$1f");
	asm("sta $d600");
}

void vdc_read(void) {
notyet:
	asm("bit $d600");
	asm("bpl %g", notyet);
	asm("lda $d601"); 
}

void vdc_write(void) {
notyet: 
	asm("bit $d600");
	asm("bpl %g", notyet);
	asm("sta $d601");
}

void save_screen(void) {
	scr_saved_x = wherex();
	scr_saved_y = wherey();
	scr_saved_c = PEEK(CHARCOLOR);
	asm("ldx #$00");
	asm("ldy #$10");
	asm("stx %v", vdc_x_w);
	asm("sty %v", vdc_y_w);
	asm("ldx #$00");
	asm("ldy #$00");
loop:
	asm("stx %v", vdc_x_r);
	asm("sty %v", vdc_y_r);
	asm("stx %v", ptr_vdc_screen);
	asm("sty %v+1", ptr_vdc_screen);
	asm("jsr %v", vdc_prepare);
	asm("jsr %v", vdc_read);
	asm("sta %v", vdc_temp);
	asm("ldx %v", vdc_x_w);
	asm("stx %v", ptr_vdc_screen);
	asm("ldy %v", vdc_y_w);
	asm("sty %v+1", ptr_vdc_screen);
	asm("jsr %v", vdc_prepare);
	asm("lda %v", vdc_temp);
	asm("jsr %v", vdc_write);
	asm("inx");
	asm("stx %v", vdc_x_r);
	asm("stx %v", vdc_x_w);
	asm("ldy %v", vdc_y_r);
	asm("cpx #$00");
	asm("bne %g", loop);
	asm("ldy %v", vdc_y_w);
	asm("iny");
	asm("sty %v", vdc_y_w);
	asm("tya");
	asm("and #$0f");
	asm("tay");
	asm("sty %v", vdc_y_r);
	asm("cpy #$00");
	asm("beq %g", done);
	asm("jmp %g", loop);
done:
	asm("rts");
}

void restore_screen(void) {
	asm("ldx #$00");
	asm("ldy #$00");
	asm("stx %v", vdc_x_w);
	asm("sty %v", vdc_y_w);
	asm("ldx #$00");
	asm("ldy #$10");
loop: 
	asm("stx %v", vdc_x_r);
	asm("sty %v", vdc_y_r);
	asm("stx %v", ptr_vdc_screen);
	asm("sty %v+1", ptr_vdc_screen);
	asm("jsr %v", vdc_prepare);
	asm("jsr %v", vdc_read);
	asm("sta %v", vdc_temp);
	asm("ldx %v", vdc_x_w);
	asm("stx %v", ptr_vdc_screen);
	asm("ldy %v", vdc_y_w);
	asm("sty %v+1", ptr_vdc_screen);
	asm("jsr %v", vdc_prepare);
	asm("lda %v", vdc_temp);
	asm("jsr %v", vdc_write);
	asm("inx");
	asm("stx %v", vdc_x_r);
	asm("stx %v", vdc_x_w);
	asm("ldy %v", vdc_y_r);
	asm("cpx #$00");
	asm("bne %g", loop);
	asm("ldy %v", vdc_y_r);
	asm("iny");
	asm("sty %v", vdc_y_r);
	asm("tya");
	asm("and #$0f");
	asm("tay");
	asm("sty %v", vdc_y_w);
	asm("cpy #$00");
	asm("beq %g", done);
	asm("ldy %v", vdc_y_r);
	asm("jmp %g", loop);
done:
	gotoxy(scr_saved_x, scr_saved_y);
	POKE(CHARCOLOR, scr_saved_c);
}
#pragma optimize (pop)
#endif

#pragma optimize (push, off)
void cursor_on(void) {
#ifdef __C64__
	asm("ldy #$00");
	asm("sty $cc");
#else
	asm("ldx #$0a");
	asm("lda #$60");
	asm("jsr %v", vdc_write_reg);
#endif
}
#pragma optimize (pop)

#pragma optimize (push, off)
void cursor_off(void) {
#ifdef __C64__
	asm("ldy $cc");
	asm("bne %g", exitloop);
	asm("ldy #$01");
	asm("sty $cd");
loop:
	asm("ldy $cf");
	asm("bne %g", loop);
exitloop:
	asm("ldy $ff");
	asm("sty $cc");
#else
	asm("ldx #$0a");
	asm("lda #$20");
	asm("jsr %v", vdc_write_reg);
#endif
}
#pragma optimize (pop)

#ifdef __C128__
#pragma optimize (push,off)
void vdc_write_reg(void) {
	asm("stx $d600");
vdc_write_wait:
	asm("ldx $d600");
	asm("bpl %g", vdc_write_wait);
	asm("sta $d601");

}
#pragma optimize (pop)
#endif
