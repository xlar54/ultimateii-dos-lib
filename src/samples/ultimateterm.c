/*****************************************************************
Ultimate II+ UltimateTerm 64 and 128
Scott Hutter, Francesco Sblendorio, Leif Bloomquist

Based on ultimate_dos-1.2.docx and command interface.docx
https://github.com/markusC64/1541ultimate2/tree/master/doc

Disclaimer:  Because of the nature of DOS commands, use this code
soley at your own risk.
Patches and pull requests are welcome

Demo program does not alter any data
******************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <cbm.h>
#include <errno.h>
#include <device.h>

// #define DEBUG_MODE

#ifdef __C128__
#include <c128.h>
#define RESET_MACHINE	asm("jmp $FF3D");
#define SCREEN_WIDTH	80
#define KEYBOARD_BUFFER 208
#define QUOTE_MODE      244
#define DISPLAY_HEADER	printf("%c%c%cUltimateTerm 128 v%s %c", 146, 14, CG_COLOR_WHITE, version, CG_COLOR_CYAN);
#define BORDER(x)       //
void blank_vicII(void);
#endif

#ifdef __C64__
#include <c64.h>
#define RESET_MACHINE 	asm("jmp $FCE2");
#define SCREEN_WIDTH	40
#define KEYBOARD_BUFFER 198
#define QUOTE_MODE      212
#define DISPLAY_HEADER	printf("%c%c%cUltimateTerm v%s %c", 146, 14, CG_COLOR_WHITE, version, CG_COLOR_CYAN);
#define BORDER(x)	    POKE(0xD020,x);
#endif

#include <conio.h>
#include <peekpoke.h>
#include <unistd.h>
#include "../lib/ultimate_lib.h"
#include "screen_utility.h"

// PETSCII Codes

#define RVS_ON			0x12
#define RVS_OFF			0x92

#define LEFT			0x9d
#define RIGHT			0x1d
#define UP				0x91
#define DOWN			0x11

#define DELETE			0x14
#define CLRSCR			0x93
#define HOME			0x13
#define BELL			0x07
#define	CR				0x0d
#define SHIFT_CR		0x8d
#define LF				0x0a
#define PIPE            0xdd

#define CG_COLOR_BLACK      0x90
#define CG_COLOR_WHITE 		0x05
#define CG_COLOR_RED 		0x1c
#define CG_COLOR_CYAN		0x9f
#define CG_COLOR_PURPLE		0x9c
#define CG_COLOR_GREEN		0x1e
#define CG_COLOR_BLUE		0x1f
#define CG_COLOR_YELLOW 	0x9e
#define CG_COLOR_L_GREEN 	0x99
#define CG_COLOR_L_BLUE  	0x9A
#define CG_COLOR_L_RED  	0x96
#define CG_COLOR_L_GRAY  	0x9B
#define SPACE38				"                                      "

#define PB_SIZE 1680

#define SOH  ((char)1)     /* Start Of Header */
#define EOT  ((char)4)     /* End Of Transmission */
#define ACK  ((char)6)     /* ACKnowlege */
#define CAN  ((char)0x18)  /* CANcel */
#define NAK  ((char)0x15)  /* Negative AcKnowlege */

// ANSI Codes

#define ANSI_ESCAPE          0x1b
#define ANSI_SEPARATOR       0x3b // ;
#define ANSI_BRACKET         0x5b // [
#define ANSI_CURSOR_UP       0x41 // A
#define ANSI_CURSOR_DOWN     0x42 // B
#define ANSI_CURSOR_FORWARD  0x43 // C
#define ANSI_CURSOR_BACKWARD 0x44 // D
#define ANSI_CURSOR_HOME     0x48 // H
#define ANSI_CLEAR_SCREEN    0x4A // J
#define ANSI_CLEAR_LINE      0x4B // K
#define ANSI_GRAPHICS_MODE   0x6D // m
#define ANSI_DECSTBM         0x72 // r
#define ANSI_PRIVATE         0x3f // ?
#define ANSI_DEC_h           0x68 // h
#define ANSI_DEC_l           0x68 // h
#define ANSI_HPA	         0x62 // b
#define ANSI_VPA	         0x64 // d

#define ANSI_VALUE_BUFFER_SIZE 10

// Telnet Stuff

#define NVT_SE 240
#define NVT_NOP 241
#define NVT_DATAMARK 242
#define NVT_BRK 243
#define NVT_IP 244
#define NVT_AO 245
#define NVT_AYT 246
#define NVT_EC 247
#define NVT_GA 249
#define NVT_SB 250
#define NVT_WILL 251
#define NVT_WONT 252
#define NVT_DO 253
#define NVT_DONT 254
#define NVT_IAC 255

#define NVT_OPT_TRANSMIT_BINARY 0
#define NVT_OPT_ECHO 1
#define NVT_OPT_SUPPRESS_GO_AHEAD 3
#define NVT_OPT_STATUS 5
#define NVT_OPT_RCTE 7
#define NVT_OPT_TIMING_MARK 6
#define NVT_OPT_NAOCRD 10
#define NVT_OPT_TERMINAL_TYPE 24
#define NVT_OPT_NAWS 31
#define NVT_OPT_TERMINAL_SPEED 32
#define NVT_OPT_LINEMODE 34
#define NVT_OPT_X_DISPLAY_LOCATION 35
#define NVT_OPT_ENVIRON 36
#define NVT_OPT_NEW_ENVIRON 39

void uii_data_print(void);
unsigned char term_bell(void);
unsigned char term_getchars(char* def, char *buf, unsigned char lbound, unsigned char ubound);
void term_displayheader(void);
void putstring_ascii(char* str);
void putstring_ansi(char* str);
void term_hostselect(void);
void term_getconfig(void);
void term_window(unsigned char x, unsigned char y, unsigned char width, unsigned char height, int border);
void detect_uci(void);
void exit_uci_error(void);
unsigned char read_host_and_port(char *prompt_host, char *prompt_port);
void display_phonebook(void);
void update_phonebook(unsigned char new_y);
void delete_phonebook_entry(void);
void add_phonebook_entry(void);
void load_phonebook(void);
void save_phonebook(void);
void help_screen(void);
void quit(void);
void download_xmodem(void);
void dos_commands(void);
void showdir(char *);
void send_dos(char *);
unsigned char handle_telnet_iac();
void send_char_ansi(unsigned char c);

char *version = "2.6";
char host[80];
char portbuff[10];
char strbuff[520];
char chr;
char buff[50];

unsigned int port = 0;
unsigned char socketnr = 0;

#define MODE_PETSCII 0
#define MODE_ASCII   1
#define MODE_ANSI    2

unsigned char character_mode = MODE_PETSCII;
unsigned char first_char = 0;
unsigned char telnet_done = 0;
unsigned char telnet_binary = 0;
unsigned char pb_loaded = 0;
unsigned char phonebookctr = 0;
char phonebook[21][80];
unsigned char dev = 0;
unsigned char pbtopidx = 0;
unsigned char pbselectedidx = 0;
char pb_bytes[PB_SIZE+1];
char hst[80];
unsigned file_index;
unsigned char y = 0;
unsigned char px = 0;
unsigned char py = 0;
#ifdef DEBUG_MODE
unsigned char debug = 0;
#endif
int intbuff = 0;
int datacount;

int len_diskbuff, idiskbuff;
int nextchar(void);

unsigned char ascToPet[] = {
0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x14,0x20,0x0a,0x11,0x93,0x0d,0x0e,0x0f,
0x10,0x0b,0x12,0x13,0x08,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,
0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,
0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f,
0x40,0xc1,0xc2,0xc3,0xc4,0xc5,0xc6,0xc7,0xc8,0xc9,0xca,0xcb,0xcc,0xcd,0xce,0xcf,
0xd0,0xd1,0xd2,0xd3,0xd4,0xd5,0xd6,0xd7,0xd8,0xd9,0xda,0x5b,0x5c,0x5d,0x5e,0x5f,
0xc0,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4a,0x4b,0x4c,0x4d,0x4e,0x4f,
0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5a,0xdb,0xdd,0xdc,0xde,0xdf,
0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8a,0x8b,0x8c,0x8d,0x8e,0x8f,
0x90,0x91,0x92,0x0c,0x94,0x95,0x96,0x97,0x98,0x99,0x9a,0x9b,0x9c,0x9d,0x9e,0x9f,
0xa0,0xa1,0xa2,0xa3,0xa4,0xa5,0xa6,0xa7,0xa8,0xa9,0xaa,0xab,0xac,0xad,0xae,0xaf,
0xb0,0xb1,0xb2,0xb3,0xb4,0xb5,0xb6,0xb7,0xb8,0xb9,0xba,0xbb,0xbc,0xbd,0xbe,0xbf,
0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,
0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7a,0x7b,0x7d,0x7c,0x7e,0x7f,
0xe0,0xe1,0xe2,0xe3,0xe4,0xe5,0xe6,0xe7,0xe8,0xe9,0xea,0xeb,0xec,0xed,0xee,0xef,
0xf0,0xf1,0xf2,0xf3,0xf4,0xf5,0xf6,0xf7,0xf8,0xf9,0xfa,0xfb,0xfc,0xfd,0xfe,0xff
};

unsigned char petToAsc[] = {
0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,
0x10,0x11,0x12,0x13,0x08,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f, 
0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,
0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f,
0x40,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,
0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7a,0x5b,0x5c,0x5d,0x5e,0x5f,
0xc0,0xc1,0xc2,0xc3,0xc4,0xc5,0xc6,0xc7,0xc8,0xc9,0xca,0xcb,0xcc,0xcd,0xce,0xcf,
0xd0,0xd1,0xd2,0xd3,0xd4,0xd5,0xd6,0xd7,0xd8,0xd9,0xda,0xdb,0xdc,0xdd,0xde,0xdf,
0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8a,0x8b,0x8c,0x8d,0x8e,0x8f,
0x90,0x91,0x92,0x0c,0x14,0x95,0x96,0x97,0x98,0x99,0x9a,0x9b,0x9c,0x9d,0x9e,0x9f,
0xa0,0xa1,0xa2,0xa3,0xa4,0xa5,0xa6,0xa7,0xa8,0xa9,0xaa,0xab,0xac,0xad,0xae,0xaf,
0xb0,0xb1,0xb2,0xb3,0xb4,0xb5,0xb6,0xb7,0xb8,0xb9,0xba,0xbb,0xbc,0xbd,0xbe,0xbf,
0x60,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4a,0x4b,0x4c,0x4d,0x4e,0x4f,
0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5a,0x7b,0x7d,0x7c,0x7e,0x7f,
0xa0,0xa1,0xa2,0xa3,0xa4,0xa5,0xa6,0xa7,0xa8,0xa9,0xaa,0xab,0xac,0xad,0xae,0xaf,
0xb0,0xb1,0xb2,0xb3,0xb4,0xb5,0xb6,0xb7,0xb8,0xb9,0xba,0xbb,0xbc,0xbd,0xbe,0xbf
};

unsigned char term_bell(void) {
	POKE(0xD418, 15);
	POKE(0xD401, 20);
	POKE(0xD405, 0);
	POKE(0xD406, 249);
	POKE(0xD404, 17);
	POKE(0xD404, 16);
	return 7;
}

#define term_gethostname(def, buf) (term_getchars(def, buf, 33, 90))
#define term_getstring(def, buf) (term_getchars(def, buf, 32, 255))

unsigned char term_getchars(char* def, char *buf, unsigned char lbound, unsigned char ubound) {
	unsigned char c,x;
	
	cursor_on();
	for (x=0;x<strlen(def);x++) {
		buf[x] = def[x];
		putchar(def[x]);
	}
	
	POKE(KEYBOARD_BUFFER,0);
	while(1) {
		c = kbhit();
		if (c != 0) {
			c = cgetc();
			switch(c) {
				case CR:
					cursor_off();
					buf[x] = 0;
					return x;

				case DELETE:
					if (x > 0) {
						x--;
						cursor_off();
						putchar(LEFT);
						putchar(' ');
						putchar(LEFT);
						cursor_on();
					}
					break;

				default:
					if (
						((c >= 32 && c <= 127) || (c >= 160)) &&
						(c >= lbound && c <= ubound)
					) {
						buf[x++] = c;
						cursor_off();
						putchar(c);
						POKE(QUOTE_MODE, 0);
						cursor_on();
					}
			}
		}
	}
}

void term_displayheader(void) {
	putchar(CG_COLOR_WHITE);
	clrscr();
	DISPLAY_HEADER
	chlinexy(0,1,SCREEN_WIDTH);
}

void putstring_ascii(char *str) {
	for (; datacount > 0; ++str, --datacount) {
		if (*str != LF) *str==BELL ? term_bell() : putchar(ascToPet[*str]);
		asm("ldx #$00");
		#ifdef __C128__
			asm("stx $f4");
			asm("stx $f5");
		#else
			asm("stx $d4");
			asm("stx $d8");
		#endif
	}
}

void term_window(unsigned char x, unsigned char y, unsigned char width, unsigned char height, int border) {
	unsigned char i;
	char *spaces="                                        ";

	spaces[width-2] = 0;
	for (i=y+1;i<y+height;i++)
		cputsxy(x+1,i,spaces);
	if (!border) return;

	chlinexy(x+1,y,width-2);chlinexy(x+1,y+height,width-2);
	cvlinexy(x,y+1,height-1);cvlinexy(x+width-1,y+1,height-1);
	cputcxy(x,y,176);cputcxy(width-1,y,174);
	cputcxy(x,y+height,173);cputcxy(x+width-1,y+height,189);
}

void delete_phonebook_entry(void) {
	unsigned char ch, ctr, x;
	if (!pbselectedidx || phonebookctr<=0) return;

	for (ctr=pbselectedidx; ctr<phonebookctr; ++ctr)
		strcpy(phonebook[ctr], phonebook[ctr+1]);
	--phonebookctr;
	if (pbtopidx>0 && pbselectedidx>phonebookctr && pbtopidx==pbselectedidx) {
		--pbtopidx;
		--pbselectedidx;
	} else if (pbselectedidx > phonebookctr) {
		--y;
		--pbselectedidx;
	}
	x = 15;
	for (ctr=pbtopidx; ctr<=pbtopidx+8 && ctr<=phonebookctr+1; ++ctr) {
		gotoxy(1,x++); 
		ch = (ctr == pbselectedidx) ? '>' : ' ';
		printf(ctr<=phonebookctr ? "%c %-36s" : SPACE38,ch,phonebook[ctr]);
	}
	cputcxy(1,y,(char) '>');
}

void add_phonebook_entry(void) {
	unsigned char ctr;
	if (phonebookctr >= 20) return;

	putchar(CG_COLOR_CYAN);
	cputsxy(8,14,"[Add entry to phonebook]");
	if (read_host_and_port("", "")) {
		sprintf(hst,"%s %u",host,port);
		++phonebookctr;
		for (ctr=phonebookctr-1; ctr>=pbselectedidx+1; --ctr)
			strcpy(phonebook[ctr+1], phonebook[ctr]);
		strcpy(phonebook[ctr+1], hst);
	}
	putchar(CG_COLOR_CYAN);
	chlinexy(8,14,24);
	display_phonebook();
}

void edit_phonebook_entry() {
	unsigned char ctr,x,len;
	if (!pbselectedidx || phonebookctr<=0) return;

	putchar(CG_COLOR_CYAN);
	cputsxy(7,14,"[Edit entry of phonebook]");
	ctr = 0;
	len = strlen(phonebook[pbselectedidx]);
	for (x=0; x<len && phonebook[pbselectedidx][x]!=' '; x++)
		host[ctr++] = phonebook[pbselectedidx][x];
	host[ctr] = 0;
	
	++x;
	ctr = 0;
	for (; x<len; x++)
		portbuff[ctr++] = phonebook[pbselectedidx][x];
	portbuff[ctr] = 0;

	if (read_host_and_port(host, portbuff)) {
		sprintf(hst,"%s %u",host,port);
		strcpy(phonebook[pbselectedidx],hst);
	}
	putchar(CG_COLOR_CYAN);
	chlinexy(7,14,25);
	display_phonebook();
}

void save_phonebook(void) {
	unsigned char ctr = 0;
	int status1, status2;

	if (dev < 8) return;
	putchar(CG_COLOR_WHITE);
	term_window(0, 14, 40, 10, 0);
	cputsxy(9,18,"SAVING: PLEASE WAIT...");

	pb_bytes[0] = 0;
	for (ctr=1; ctr<=phonebookctr; ++ctr) {
		strcat(pb_bytes, phonebook[ctr]);
		strcat(pb_bytes, "\n");
	}
	cbm_close(15); cbm_close(2);
	cbm_open(15, dev, 15,"s:u-term,s"); cbm_close(15);
	status1 = cbm_open(2, dev, CBM_WRITE,"u-term,s");
	status2 = cbm_write(2, pb_bytes, strlen(pb_bytes));
	cbm_close(2);
	if (!status1 && status2==-1) {
		cbm_open(15, dev, 15, "");
		cbm_read(15, pb_bytes, PB_SIZE);
		cbm_close(15);
	} 
	display_phonebook();
}

void quit(void) {
	putchar(CG_COLOR_CYAN);
	cputsxy(7,14,"[Reset and back to BASIC]");
	putchar(CG_COLOR_WHITE);
	term_window(0, 14, 40, 10, 0);
	gotoxy(9,18); printf("ARE YOU SURE (Y/N)? ");
	cursor_on();
	term_gethostname("", hst);
	cursor_off();
	if (hst[0]=='y' || hst[0]=='Y')
		RESET_MACHINE;
	putchar(CG_COLOR_CYAN);
	chlinexy(7,14,25);
	display_phonebook();
}

void load_phonebook(void) {
	char *file = "0:u-term,s";
	unsigned char c, ctr;
	int bytesRead = 0;
	pbtopidx = 0;
	pbselectedidx = 0;

	term_window(0, 14, 40, 10, 0);
	cputsxy(8,14,"[ Loading Phonebook... ]");
	cbm_close(2);
	cbm_close(15);

	if (dev>=8) {
		c = cbm_open(2, dev, CBM_READ, file);
		bytesRead = c ? 0 : cbm_read(2, pb_bytes, PB_SIZE);
	}
	strcpy(phonebook[0], "MANUAL ENTRY");
	if (dev < 8 || bytesRead <= 0) { // No drive or no file
		// Default phonebook
		strcpy(phonebook[1], "bbs.retrocampus.com 6510");
		strcpy(phonebook[2], "afterlife.dynu.com 6400");
		strcpy(phonebook[3], "borderlinebbs.dyndns.org 6400");
		strcpy(phonebook[4], "commodore4everbbs.dynu.net 6400");
		strcpy(phonebook[5], "eagleman.bounceme.net 6464");
		strcpy(phonebook[6], "hurricanebbs.dynu.net 6401");
		strcpy(phonebook[7], "particlesbbs.dyndns.org 6400");
		strcpy(phonebook[8], "bbs.retroacademy.com 6510");
		phonebookctr = 8;
		if (dev >= 8) {
			if (!c && !cbm_open(15, dev, 15, "")) cbm_read(15, pb_bytes, PB_SIZE);
			cbm_close(15);
		}
	} else {
		// read phonebook data
		phonebookctr = 0;
		ctr=0;

		file_index = 0;
		for (file_index=0; file_index<bytesRead; ++file_index) {
			c = pb_bytes[file_index];
			if ((c == CR || c == LF) && ctr) {
				strcpy(phonebook[++phonebookctr], hst);
				ctr=0;
			} else if (c != LF && c != CR) {
				// c to lowercase
				if ((c>=97 && c<=122) || (c>=193 && c<=218)) c &= 95;

				hst[ctr] = c;
				hst[++ctr] = 0;

				// hostname too big
				if (ctr == 78) break;
			}
		}
		cbm_close(2);
	}

	chlinexy(8,14,24);
	y = 15;
	display_phonebook();
	cputcxy(1,y,'>');
	pb_loaded = 1;
}

void term_hostselect(void) {
	unsigned char ctr;
	unsigned char x;
	unsigned char c = 0;
	unsigned len;
	
startover:
	POKE(KEYBOARD_BUFFER,0);
	term_window(0, 14, 40, 10, 1);
	if (pb_loaded == 0)
		load_phonebook();
	else {
		display_phonebook();
		cputcxy(1,y,'>');
	}

	
	POKE(KEYBOARD_BUFFER,0);
	while(1) {
		c = kbhit();
		if (c != 0) {
			c = cgetc();
			if ((c>=97 && c<=122) || (c>=193 && c<=218)) c &= 95; // c to lowercase

			if (c == 'd') 
				delete_phonebook_entry();

			else if (c == 'a')
				add_phonebook_entry();

			else if (c == 'e')
				edit_phonebook_entry();

			else if (c == 'l') 
				load_phonebook();

			else if (c == 's')
				save_phonebook();

			else if (c == 'q')
				quit();

			else if (c == 133) { // KEY F1: help
				help_screen();
				cursor_off();
			}

			else if (c == 135) { // KEY F5: send DOS command
				px = wherex(); py = wherey();
				dos_commands();
				cursor_off();
				gotoxy(37, 9); printf("%c%2d%c", CG_COLOR_YELLOW, dev, CG_COLOR_CYAN);
				gotoxy(px, py);
			}

			else if (c == '+') {
				px = wherex(); py = wherey();
				if (dev < 15) ++dev;
				gotoxy(37, 9); printf("%c%2d%c", CG_COLOR_YELLOW, dev, CG_COLOR_CYAN);
				gotoxy(px, py);
			}

			else if (c == '-') {
				px = wherex(); py = wherey();
				if (dev > 8) --dev;
				gotoxy(37, 9); printf("%c%2d%c", CG_COLOR_YELLOW, dev, CG_COLOR_CYAN);
				gotoxy(px, py);
			}

			else if (c == DOWN && wherey() < 23 && (pbselectedidx + 1 <= phonebookctr)) {
				cputcxy(1,y++,' ');
				cputcxy(1,y,'>');
				pbselectedidx++;
			}

			else if (c == DOWN && wherey() == 23 && (pbselectedidx + 1 <= phonebookctr)) {
				if (phonebookctr >= pbtopidx+8) {
					pbtopidx++;
					update_phonebook(23);
					pbselectedidx++;
				}
			}

			else if (c == UP && wherey() > 15) {
				cputcxy(1,y--,' ');
				cputcxy(1,y,'>');
				pbselectedidx--;
			}

			else if (c == UP && wherey() == 15) {
				if (pbtopidx > 0) {
					pbtopidx--;
					update_phonebook(15);
					pbselectedidx--;
				}
			}

			else if (c == CR) {
				if (pbselectedidx == 0) {
					// MANUAL ENTRY
					if (read_host_and_port("", "")) return;
					goto startover;
				} else {
					ctr = 0;
					len = strlen(phonebook[pbselectedidx]);
					for (x=0; x<len && phonebook[pbselectedidx][x]!=' '; x++)
						host[ctr++] = phonebook[pbselectedidx][x];
					host[ctr] = 0;
					
					ctr = 0;
					for (; x<len; x++)
						portbuff[ctr++] = phonebook[pbselectedidx][x];
					portbuff[ctr] = 0;

					port = atoi(portbuff);
					return;
				}
			}
		}
	}
}

void term_getconfig(void) {
	printf("%cby %cScott Hutter%c & %cFrancesco Sblendorio%c", CG_COLOR_CYAN, CG_COLOR_WHITE, CG_COLOR_CYAN, CG_COLOR_WHITE, CG_COLOR_CYAN);
	printf("\n\n%cPlease ensure the following:%c",CG_COLOR_YELLOW,CG_COLOR_CYAN);
	printf("\n - Network link is in 'Link Up' state");
	printf("\n - %cPress F1%c to get help in session", CG_COLOR_WHITE, CG_COLOR_CYAN);
	printf("\n - %cPress F5%c to send DOS commands", CG_COLOR_WHITE, CG_COLOR_CYAN);

	uii_identify();
	printf("\n\nNIC Status: %c%s%c", CG_COLOR_WHITE, uii_status, CG_COLOR_CYAN);

	uii_getipaddress();
	printf("\nIP Address: %c%d.%d.%d.%d%c", CG_COLOR_WHITE,uii_data[0], uii_data[1], uii_data[2], uii_data[3],CG_COLOR_CYAN);
	printf("\n   Netmask: %c%d.%d.%d.%d%c", CG_COLOR_WHITE,uii_data[4], uii_data[5], uii_data[6], uii_data[7],CG_COLOR_CYAN);
	printf("\n   Gateway: %c%d.%d.%d.%d%c", CG_COLOR_WHITE,uii_data[8], uii_data[9], uii_data[10], uii_data[11],CG_COLOR_CYAN);

	gotoxy(29, 9); printf("\005+-%c Drive%c%2d", CG_COLOR_CYAN, CG_COLOR_YELLOW, dev);
	gotoxy(29,10); printf("\005A%cdd   \005S%cave", CG_COLOR_CYAN, CG_COLOR_CYAN);
	gotoxy(29,11); printf("\005D%cel   \005L%coad", CG_COLOR_CYAN, CG_COLOR_CYAN);
	gotoxy(29,12); printf("\005E%cdit  \005Q%cuit", CG_COLOR_CYAN, CG_COLOR_CYAN);
	gotoxy(0,2);
}

void send_char(unsigned char c) {	
	buff[0] = c;
	buff[1] = 0;
	uii_socketwrite(socketnr, buff);	
}

void send3chars(unsigned char c1, unsigned char c2, unsigned char c3) {
	buff[0] = c1;
	buff[1] = c2;
	buff[2] = c3;
	buff[3] = 0;
	uii_socketwrite(socketnr, buff);
}

void main(void) 
{
	unsigned char c = 0;
	int x = 0;

	detect_uci();
	uii_abort();
	dev = getcurrentdevice();
	dev = (dev < 8 ? 8 : dev);

#ifdef __C128__
	videomode(VIDEOMODE_80COL);
	putchar(14);
	blank_vicII();
	fast();
	POKE(808,107);   // Disable RUN/STOP + RESTORE on C128
#else
	POKEW(0xD020,0); // Border + background = black
	POKE(808,239);   // Disable RUN/STOP on C64
	POKE(792,193);   // Disable RESTORE  on C64
#endif

	for (x=0; x<6000; ++x); // Initial pause for handling direct "RUN" from u2(+)
	for (c=0; c<25; ++c) POKE(0xD400 + c, 0); // Initialize SID
	printf("Accessing network target...\n(if no response, perhaps connection was\nnot closed?");

	uii_settarget(TARGET_NETWORK);
	cursor_off();

	// Main Program Loop
	while(1) {
		BORDER(0)
		character_mode = MODE_PETSCII;
		first_char = 1;
		telnet_binary = 0;
		telnet_done = 0;

		term_displayheader();
		term_getconfig();
		term_hostselect();
		cursor_off();
		term_displayheader();
		gotoxy(0,2);
		printf("%c\n[F7] to close the connection when done\n", CG_COLOR_YELLOW);
		printf("\n * Connecting to\n   %s:%u\n", host, port);
		
		socketnr = uii_tcpconnect(host, port);
		
		if (uii_success()) {
			printf("\n * Connected");
			cursor_on();

			// Main Terminal Loop
			while (1) {

				// Read from remote

				if (telnet_done) {
					datacount = uii_socketread(socketnr, 892);
			    } else {
					datacount = uii_socketread(socketnr, 1);	  // Special handling during telnet negotiation - read single character at a time
				}

				if (datacount == 0) { // datacount == 0 means "disconnected"
					break;
				} else if (datacount > 0) {

					c = uii_data[2];

					// Special handling for first char.   If first character back from remote side is NVT_IAC, we have a telnet connection.
					if (first_char) {
						if (c == NVT_IAC) {
							BORDER(2)     // red
							handle_telnet_iac();
							character_mode = MODE_ANSI;
							first_char = 0;
							BORDER(12)    // light
							cursor_off();
							putchar(14); // lowercase
							putchar(8);  // lock lowercase
							printf("%c (Telnet)%c\n\n", CG_COLOR_L_GREEN, CG_COLOR_L_GRAY);
							cursor_on();
							continue;     // Top of terminal loop
						}
						else
						{
							first_char = 0;
							telnet_done = 1;
							cursor_off();
							printf("%c\n\n", CG_COLOR_CYAN);
							cursor_on();
						}
					}

					// Connection already established, but may be more telnet control characters
					if ((c == NVT_IAC) && (character_mode == MODE_ANSI))
					{
						if (handle_telnet_iac())  // Returns true if a second NVT_IAC (255) was received
						{
							c = NVT_IAC;  // TODO - fix - this character is not displayed
						}
					}
					else   //  Finally regular data - just display
					{
						telnet_done = 1;
						cursor_off();

						switch (character_mode)
						{
							case MODE_PETSCII:
								uii_data_print();
								break;

							case MODE_ASCII:
								putstring_ascii(uii_data + 2);
								break;

							case MODE_ANSI:
								putstring_ansi(uii_data + 2);
								break;

							default:
								uii_data_print();
								break;
						}

						cursor_on();
					}
				} // datacount == -1 means "wait state"

				// Handle keyboard

				c = kbhit();
				if (c != 0) {
					c = cgetc();
					if (c == 133) // KEY F1: HELP
						help_screen();
					else if (c == 134) // KEY F3: download (xmodem protocol)
						download_xmodem();
					else if (c == 135) // KEY F5: send DOS commands to disk
						dos_commands();
					else if (c == 139) { // KEY F6: switch petscii/ascii  (not allowed in telnet/ansi mode)
						if (character_mode != MODE_ANSI) {
							character_mode = !character_mode;
						}
					}
					else if (c == 136) // KEY F7: close connection
						break;
				#ifdef DEBUG_MODE
					else if (c == 140) { // KEY F8: Debugging
						debug = !debug;
					}
				#endif
					else
					{
						switch (character_mode)
						{
							case MODE_PETSCII:
								send_char(c);
								break;

							case MODE_ASCII:
								send_char(petToAsc[c]);
								break;

							case MODE_ANSI:
								send_char_ansi(c);
								break;

							default:
								send_char(c);
								break;
						}
					}
				}
			}

			uii_socketclose(socketnr);
			cursor_off();
			if (c != 136) { // NOT KEY F7
				printf("\n%cconnection closed, hit any key", CG_COLOR_WHITE);
				c = 0; while(c==0) c=kbhit();
			}
			putchar(14);
		}
		else
		{
			cursor_off();
			printf("\n%c * Connect failed:\n   %s", CG_COLOR_L_RED, uii_status);
			printf("\n\n * Press any key");
			
			c = 0;
			while(c==0) c=kbhit();
		}
	}
}

void update_phonebook(unsigned char new_y) {
	unsigned char ctr;
	y = 15;
	for (ctr=pbtopidx; ctr<=pbtopidx+8 && ctr<=phonebookctr; ++ctr) {
		gotoxy(3,y++);
		cprintf("%-36s",phonebook[ctr]);
	}
	y=new_y;
	cputcxy(1,y,'>');
}

void display_phonebook(void) {
	unsigned char ctr, x = 15;
	putchar(CG_COLOR_CYAN);
	term_window(0, 14, 40, 10, 0);
	for (ctr=pbtopidx; ctr<=pbtopidx+8 && ctr<=phonebookctr; ++ctr)
		cputsxy(3,x++,phonebook[ctr]);
	cputcxy(1,y,'>');
}

unsigned char read_host_and_port(char *prompt_host, char *prompt_port) {
	putchar(CG_COLOR_WHITE);
	term_window(0, 14, 40, 10, 0);
	gotoxy(5,16);
	printf("%cHost: %c", CG_COLOR_CYAN, CG_COLOR_WHITE);
	term_gethostname(prompt_host, host);
	putchar(CG_COLOR_CYAN);
	if (host[0]==0) return 0;
	gotoxy(5,18);
	printf("%cPort: %c", CG_COLOR_CYAN, CG_COLOR_WHITE); 
	term_gethostname(prompt_port, portbuff);
	putchar(CG_COLOR_CYAN);
	if (portbuff[0] == 0) return 0;
	port = atoi(portbuff);
	return 1;
}

#ifdef __C128__
#pragma optimize (push,off)
void blank_vicII(void) {
	asm("lda $d011");
	asm("and #$6f");        /* make sure to mask bit 7 else we may end up waiting for a non existing rasterline! */
	asm("sta $d011");
}
#pragma optimize (pop)
#endif

#pragma optimize (push,off)
void detect_uci(void) {
	asm("lda $df1d");
	asm("cmp #$c9");
	asm("bne %g", nointerface);
	asm("rts");
nointerface:
	asm("jmp %v", exit_uci_error);
}
#pragma optimize (pop)

#pragma optimize (push,off)
void uii_data_print(void) {
	static char *string_ptr = uii_data;
	asm("lda %v", string_ptr);
	asm("clc");
	asm("adc #$02");
	asm("sta $fb");
	asm("lda %v+1", string_ptr);
	asm("adc #$00");
	asm("sta $fc");
	asm("lda #$00");
	asm("sta $fd"); // keeps track of hibyte of number of chars printed

	asm("ldy #$00");
loop:
	asm("lda ($fb),y");
#ifdef __C128__
	asm("cmp #$0a");
	asm("beq %g", skipline);
#endif
	// check for bell character and perform sound if so
	asm("cmp #$07");
	asm("bne %g", skipbell);
	asm("jsr %v", term_bell);
	asm("jmp %g", nextch);
skipbell:
	asm("jsr $ffd2");
	asm("ldx #$00");
#ifdef __C128__
	asm("stx $f4");
	asm("stx $f5");
#else
	asm("stx $d4");
	asm("stx $d8");
#endif
#ifdef __C128__
skipline:
#endif
nextch:
	asm("iny");
	asm("bne %g", skp);
	asm("inc $fc");
	asm("inc $fd");
skp:
	asm("cpy %v", datacount);
	asm("bne %g", loop);
	asm("lda $fd");
	asm("cmp %v+1", datacount);
	asm("bne %g", loop);
}
#pragma optimize (pop)

void exit_uci_error(void) {
	POKEW(0xD020,0);
	printf(
		"%c"
		"\005WARNING:\233 Turn on \005Command Interface\233\n" 
		"\243\243\243\243\243\243\243\243\243\243\243\243\243"
		"\243\243\243\243\243\243\243\243\243\243\243\243\243"
		"\243\243\243\243\243\243\243\243\n"
		"\n"
		"1. Enter Ultimate's menu, press F2,\n"
		"   go to \005C64 and cartridge settings\233\n"
		"\n"
		"2. Go to \005Cartridge\233 and set\n"
		"   it to \005None\233 using Cursor Keys\n"
		"\n"
		"3. Go to \005Command Interface\233 and set\n"
		"   it to \005Enabled\233 using Cursor Keys\n"
		"\n"
		"4. Press \005RUN/STOP\233 twice and \005reboot\233\n"
		"   your computer (turn it off and on)\n\n"
		"\n"
		"You have to do these steps \005once\233. Then\n"
		"\005reload UltimateTerm\233 and run it.\n"
		"\243\243\243\243\243\243\243\243\243\243"
		"\243\243\243\243\243\243\243\243\243\n"
		,147
	);
	exit(1);
}

void help_screen(void) {
	#ifdef __C128__
	#define LINE1 34 
	#define LINE2 28
	#define LINE3 27
	#define LINE4 58
	#else
	#define LINE1 15
	#define LINE2 8
	#define LINE3 7
	#define LINE4 16
	#endif
	cursor_off();
	save_screen();
	POKE(QUOTE_MODE, 0);

	putchar(CG_COLOR_WHITE);
	clrscr();
	putchar(14);
	gotoxy(LINE1,1);  printf("\222HELP SCREEN");
	gotoxy(LINE1,2);  printf("\243\243\243\243\243\243\243\243\243\243\243");
	gotoxy(LINE2,16); printf("Press any key to go back");
	gotoxy(LINE3,5);  printf("\022 F1 \222  This HELP screen");
	gotoxy(LINE3,7);  printf("\022 F3 \222  Download with Xmodem");
	gotoxy(LINE3,9);  printf("\022 F5 \222  DOS commands to disk");
	gotoxy(LINE3,11); printf("\022 F6 \222  Switch PETSCII/ASCII");
	gotoxy(LINE3,13); printf("\022 F7 \222  Disconnect");
	gotoxy(LINE4,21); printf("%cPlease report issues:", CG_COLOR_L_GRAY);
	gotoxy(LINE4,22); printf("%chttps://git.io/fjyUe", CG_COLOR_L_GRAY);
	gotoxy(LINE4,23); printf("%c\243\243\243\243\243\243\243\243\243\243\243\243\243\243\243\243\243\243\243\243", CG_COLOR_WHITE);
	putchar(CG_COLOR_WHITE);
	POKE(KEYBOARD_BUFFER,0);
	cgetc();
	POKE(KEYBOARD_BUFFER,0);
	restore_screen();
	cursor_on();
}

void dos_commands(void) {
	cursor_off();
	save_screen();
	POKE(QUOTE_MODE, 0);
	putchar(CG_COLOR_WHITE);
	clrscr();
	putchar(14);

	printf("Enter a DOS command, or:\n"
	       "\243\243\243\243\243\243\243\243\243\243\243\243\243\243\243\243\243\243\243\243\243\243\243\243\n"
	       "%c$ %c to get directory\n"
	       "%c#8%c to set drive number to 8\n"
	       "%c#9%c to set drive number to 9\n"
	       "%c@ %c to read status\n"
	       "%c. %c to go back\n\n"
	       , CG_COLOR_WHITE, CG_COLOR_L_GRAY
	       , CG_COLOR_WHITE, CG_COLOR_L_GRAY
	       , CG_COLOR_WHITE, CG_COLOR_L_GRAY
	       , CG_COLOR_WHITE, CG_COLOR_L_GRAY
	       , CG_COLOR_WHITE, CG_COLOR_L_GRAY
	       );
	putchar(CG_COLOR_WHITE);

	for (;;)  {
		printf("#%d> ", dev);
		term_getstring("", strbuff);
		putchar('\n');
		if (!strbuff[0] || !strcmp(".", strbuff))
			break;

		if (strbuff[0] == '#') {
			intbuff = atoi(strbuff+1);
			if (intbuff >= 8 && intbuff <= 15) dev = intbuff;
		} else if (strbuff[0] == '$') {
			showdir(strbuff);
		} else if (strbuff[0] == '@' && strbuff[1] == '$') {
			showdir(strbuff+1);
		} else if (strbuff[0] == '@') {
			send_dos(strbuff+1);
		} else {
			send_dos(strbuff);
		}
	}
	
	POKE(KEYBOARD_BUFFER,0);
	restore_screen();
	cursor_on();
}

void send_dos(char *s) {
	cbm_close(15);
	intbuff = cbm_open(15, dev, 15, s);
	if (intbuff) {
		cbm_close(15);
		printf("I/O error\n");
		return;
	}
	pb_bytes[0] = 0;
	intbuff = cbm_read(15, pb_bytes, PB_SIZE);
	if (intbuff > 0)
		pb_bytes[intbuff] = 0;
	else {
		cbm_close(15);
		printf("I/O error\n");
		return;
	}
	cbm_close(15);
	printf("%s\n", pb_bytes);
}

void showdir(char *s) {
	int a,b;
	cbm_close(2); cbm_close(15);
	intbuff = cbm_open(2, dev, 0, s);
	if (intbuff) {
		cbm_close(2); cbm_close(15);
		printf("I/O error\n");
		return;
	}

	pb_bytes[0] = 0;
	cbm_open(15, dev, 15, "");
	intbuff = cbm_read(15, pb_bytes, PB_SIZE);
	if (intbuff > 0)
		pb_bytes[intbuff] = 0;
	else {
		cbm_close(2); cbm_close(15);
		printf("I/O error\n");
		return;
	}
	if (atoi(pb_bytes)) {
		cbm_close(2); cbm_close(15);
		printf("%s\n", pb_bytes);
		return;
	}
	
	len_diskbuff = 0; idiskbuff = 0;
	
	nextchar();
	nextchar();

	for (;;) {
		a = nextchar();
		b = nextchar();
		if ((!a && !b) || a<0 || b<0) break;
		a = nextchar();
		b = nextchar();
		if (a<0 || b<0) break;
		printf("%d ", b*256 + a);
		do {
			if (kbhit()) {
				b = cgetc();
				if (b == 3 || b == 95) { 
					cbm_close(2); cbm_close(15); 
					putchar('\n'); 
					return;
				}
			}
			a = nextchar();
			putchar(a > 0 ? a : '\n');
		} while (a > 0);
		if (a < 0) break;
	}
	putchar('\n');
	cbm_close(2);
	cbm_close(15);
}

int nextchar(void) {
    char result;
    if (idiskbuff < len_diskbuff) {
        result = strbuff[idiskbuff++];
    } else {
        do {
            len_diskbuff = cbm_read(2, strbuff, 512);
            if (len_diskbuff == 0) return -1;
        } while (len_diskbuff == 0);
        result = strbuff[0];
        idiskbuff = 1;
    }
    return result;
}

void process_xmodem_sector(char sector[], char is_eot) {
	int len = 128;
	unsigned char status;

	if (is_eot) {
		chr = sector[len-1];
		while (len > 0 && sector[len-1] == 26)
			--len;
	}

	if (len > 0)
		status = cbm_write(2, sector, len);
}

void download_xmodem(void) {
	#ifdef __C128__
	#define LINEP1 27 
	#define LINEP2 27
	#define LINEP3 27
	#else
	#define LINEP1 7
	#define LINEP2 7
	#define LINEP3 7
	#endif

	#define MAXERRORS 10
	#define SECSIZE   128

	char filename[80];
	char scratch_cmd[80];
	char c;
	char b;
	char not_b;
	char blocknumber;
	char checksum;
	unsigned char i, status;
	unsigned char errorcount, errorfound;
	char sector[SECSIZE];
	char firstread;

	errorcount = 0;
	blocknumber = 1;
	cursor_off();
	save_screen();
	POKE(QUOTE_MODE, 0);

	putchar(CG_COLOR_WHITE);
	clrscr();
	putchar(14);
	gotoxy(LINEP1,1); printf("\222DOWNLOAD - XMODEM PROTOCOL");
	gotoxy(LINEP1,2); printf("\243\243\243\243\243\243\243\243\243\243\243\243\243"
							  "\243\243\243\243\243\243\243\243\243\243\243\243\243");
	gotoxy(LINEP1,7); printf("\243\243\243\243\243\243\243\243\243\243\243\243\243"
							  "\243\243\243\243\243\243\243\243\243\243\243\243\243\243");
	putchar(CG_COLOR_L_GRAY);
	gotoxy(LINEP3,5); printf("Enter destination filename:");
	gotoxy(LINEP3,6); printf("                            ");
	gotoxy(LINEP3,6); term_getstring("", filename);
	if (filename[0] == 0 || dev < 8) {
		restore_screen();
		cursor_on();
		return;
	}
	for (i=0; i<strlen(filename); ++i) {
		c = filename[i];
		if ((c>=97 && c<=122) || (c>=193 && c<=218)) c &= 95;
		switch (c) {
			case ':':
			case ',':
			case '?':
			case '*':
			case '@':
			case '$':
				c = '.';
		}
		filename[i] = c;
	}
	gotoxy(LINEP3,8); printf("\022P\222RG, \022S\222EQ or \022U\222SR? ");
	POKE(KEYBOARD_BUFFER, 0);
	cursor_off();
	do {
		c = cgetc();
		if (c == 3 || c == 95) {
			restore_screen();
			cursor_on();
			return;
		}
		if ((c>=97 && c<=122) || (c>=193 && c<=218)) c &= 95;
	} while (c != 'p' && c !='s' && c != 'u');
	strcpy(scratch_cmd, "s:");
	strcat(scratch_cmd, filename);
	if (c == 'p') strcat(filename, ",p");
		else if (c == 's') strcat(filename, ",s");
		else strcat(filename, ",u");

	cursor_off();
	POKE(KEYBOARD_BUFFER, 0);
	putchar(c);
	gotoxy(0,10); printf("PLEASE WAIT (RUN/STOP to abort).");
	cursor_off();

	// Open file in write mode
	cbm_close(15); cbm_close(2);
	cbm_open(15, dev, 15, scratch_cmd); cbm_close(15);
	status = cbm_open(2, dev, CBM_WRITE, filename);
	if (status) {
		printf("\n\nI/O ERROR. Download aborted.");
		cgetc();
		restore_screen();
		cursor_on();
		return;
	}

	// Start Xmodem transfer
	cursor_off();
	uii_reset_uiidata();
	uii_socketwritechar(socketnr, NAK);

	firstread = 1;
	do {
		errorfound = 0;
		c = uii_tcp_nextchar(socketnr);
		if (!firstread) {
			putchar('.');
			process_xmodem_sector(sector, c == EOT);
		}
		if (c != EOT) {
			if (c != SOH) {
				printf("\nERROR: expected SOH, received: %d\n", c);
				if (++errorcount < MAXERRORS) {
					errorfound = 1;
					continue;
				} else {
					printf("\nFATAL: too may errors\n");
					break;
				}
			}
			b = uii_tcp_nextchar(socketnr);
			not_b = ~uii_tcp_nextchar(socketnr);
			if (b != not_b) {
				printf("\nERROR during checking blocknumber parity");
				errorfound = 1;
				++errorcount;
				continue;
			}
			if (b != blocknumber) {
				printf("\nERROR: Wrong blocknumber");
				errorfound = 1;
				++errorcount;
				continue;
			}
			checksum = 0;
			for (i=0; i<SECSIZE; ++i) {
				sector[i] = uii_tcp_nextchar(socketnr);
				checksum += sector[i];
			}
			if (checksum != uii_tcp_nextchar(socketnr)) {
				printf("ERROR: bad checksum");
				errorfound = 1;
				++errorcount;
			}

			if (kbhit()) {
				i = cgetc();
				if (i == 3 || i == 95) {
					uii_socketwritechar(socketnr, CAN);
					printf("\nCanceling download...\n");
					cbm_close(2);
					uii_reset_uiidata();
					cbm_open(15, dev, 15, scratch_cmd);
					cbm_read(15, pb_bytes, PB_SIZE);
					cbm_close(15);
					printf("\nBREAK: press any key");
					POKE(KEYBOARD_BUFFER,0);
					cgetc();
					POKE(KEYBOARD_BUFFER,0);
					restore_screen();
					cursor_on();
					return;
				}
			}

			if (errorfound != 0) {
				uii_socketwritechar(socketnr, NAK);
			} else {
				uii_socketwritechar(socketnr, ACK);
				++blocknumber;
			}

			firstread = 0;
		}
	} while (c != EOT);

	//close file
	cbm_close(2);
	cbm_open(15, dev, 15, "");
	cbm_read(15, pb_bytes, PB_SIZE);
	cbm_close(15);

	uii_socketwritechar(socketnr, ACK);

	uii_reset_uiidata();

	printf("\n\nDownload finished: press any key");
	POKE(KEYBOARD_BUFFER,0);
	cgetc();
	POKE(KEYBOARD_BUFFER,0);
	restore_screen();
	cursor_on();
}

void SendTelnetDoWill(unsigned char verb, unsigned char opt)
{
	send_char(NVT_IAC);                               // send character 255 (start negotiation)
	send_char(verb == NVT_DO ? NVT_DO : NVT_WILL);    // send character 253  (do) if negotiation verb character was 253 (do) else send character 251 (will)
	send_char(opt);
}

void SendTelnetDontWont(unsigned char verb, unsigned char opt)
{
	send_char(NVT_IAC);                               // send character 255   (start negotiation)
	send_char(verb == NVT_DO ? NVT_WONT : NVT_DONT);  // send character 252   (wont) if negotiation verb character was 253 (do) else send character 254 (dont)
	send_char(opt);
}

void SendTelnetParameters()
{
	send_char(NVT_IAC);                               // send character 255 (start negotiation) 
	send_char(NVT_DONT);                              // send character 254 (dont)
	send_char(34);                                    // linemode

	send_char(NVT_IAC);                               // send character 255 (start negotiation)
	send_char(NVT_DONT);                              // send character 253 (do)
	send_char(1);                                     // echo
}

unsigned char handle_telnet_iac() {
	
	int datacount;
	unsigned char verb;           // telnet parameters
	unsigned char opt;

	// First time through
	if (first_char)
	{
		SendTelnetParameters();                         // Start off with negotiating preferred parameters
	}

	datacount = uii_socketread(socketnr, 1);   // TODO check for closed connection
	verb = uii_data[2];                                // receive negotiation verb character

	if ((verb == NVT_IAC) && telnet_binary)
	{
		return 1;                                  // Received two NVT_IACs in a row so treat as single 255 data in calling function
	}

	datacount = uii_socketread(socketnr, 1);   // TODO check for closed connection
	opt = uii_data[2];                               // receive negotiation option character

	switch (verb) {                                  // evaluate negotiation verb character
		case NVT_WILL:                                      // if negotiation verb character is 251 (will)or
		case NVT_DO:                                        // if negotiation verb character is 253 (do) or

			switch (opt) {

				case NVT_OPT_SUPPRESS_GO_AHEAD:                 // if negotiation option character is 3 (suppress - go - ahead)
					SendTelnetDoWill(verb, opt);
					break;

				case NVT_OPT_TRANSMIT_BINARY:                   // if negotiation option character is 0 (binary data)
					SendTelnetDoWill(verb, opt);
					telnet_binary = 1;
					break;

				default:                                        // if negotiation option character is none of the above(all others), just say no
					SendTelnetDontWont(verb, opt);
					break;                                      //  break the routine
			}
			break;

		case NVT_WONT:                                      // if negotiation verb character is 252 (wont)or
		case NVT_DONT:                                      // if negotiation verb character is 254 (dont)

			switch (opt) {

				case NVT_OPT_TRANSMIT_BINARY:                   // if negotiation option character is 0 (binary data)
					SendTelnetDontWont(verb, opt);
					telnet_binary = 0;
					break;

				default:                                        // if negotiation option character is none of the above(all others)
					SendTelnetDontWont(verb, opt);
					break;                                      //  break the routine
				}
				break;

		case NVT_IAC:                                       // Ignore second IAC/255 if we are in BINARY mode
		default:
			printf(">> Unknown IAC Verb  %d\n", opt);
			break;
	}
	return 0;
}

void send_char_ansi(unsigned char c)
{
	switch (c)
	{
		case UP:  // Cursor up
			send3chars(ANSI_ESCAPE, ANSI_BRACKET, ANSI_CURSOR_UP);
			break;

		case DOWN:  // Cursor down
			send3chars(ANSI_ESCAPE, ANSI_BRACKET, ANSI_CURSOR_DOWN);
			break;

		case LEFT:  // Cursor left
			send3chars(ANSI_ESCAPE, ANSI_BRACKET, ANSI_CURSOR_BACKWARD);
			break;

		case RIGHT:  // Cursor right
			send3chars(ANSI_ESCAPE, ANSI_BRACKET, ANSI_CURSOR_FORWARD);
			break;

		case HOME:  // Intercept this, since it is seen as CTRL-S (XOFF) and there is no way to send CTRL-Q (XON) since that is also cursor down on the C64 keyboard...
			send_char(0x01);  // Send CTRL-A instead (Home, beginning of line)
			break;

		default:
			send_char(petToAsc[c]);
			break;
	}
}

void convert_ansi_color(int values[], int valuecount) {

	int i = 0;
	unsigned char bold = 0;

	if (valuecount == 0) {
		putchar(CG_COLOR_L_GRAY);   // Reset all attributes
		putchar(RVS_OFF);
		return;
	}
	else {  // Foreground colors
		for (i = 0; i < valuecount; i++) {

			switch (values[i]) {

				case 0:
					putchar(CG_COLOR_L_GRAY);   // Reset all attributes
					putchar(RVS_OFF);
					break;

				case 1:
					bold = 1;
					break;

				case 7:
					putchar(RVS_ON);
					break;

				case 30:   // Black
					putchar(CG_COLOR_BLACK);
					break;

				case 31:   // Red
					putchar(bold ? CG_COLOR_L_RED : CG_COLOR_RED);
					break;

				case 32:   // Green
					putchar(bold ? CG_COLOR_L_GREEN : CG_COLOR_GREEN);
					break;

				case 33:   // Yellow
					putchar(CG_COLOR_YELLOW);
					break;

				case 34:   // Blue
					putchar(bold ? CG_COLOR_L_BLUE : CG_COLOR_BLUE);
					break;

				case 35:   // Magenta
					putchar(CG_COLOR_PURPLE);
					break;

				case 36:   // Cyan
					putchar(CG_COLOR_CYAN);

				case 37:   // White
					putchar(bold ? CG_COLOR_L_GRAY : CG_COLOR_WHITE);
					break;
				
				default: // Unhandled, ignore
					break;
			}
		}
	}
}

int parse_ansi_value(char* buf) {
	int value = atoi(buf);
	memset(buf, 0, ANSI_VALUE_BUFFER_SIZE); // Clear in case later strings are shorter
	return value;
}

char* parse_ansi_escape(char* str) {

	char valbuffer[ANSI_VALUE_BUFFER_SIZE] = { 0 };
	int vb = 0;

	int values[2] = { 0 };
	int vi = 0;

	str++;
	datacount--;

	if (*str != ANSI_BRACKET) {  // Invalid escape sequence, ignore and move to next char
		return str;
	}

	while (1) {

		str++;
		datacount--;

		// Numeric values
		if (isdigit(*str)) {
			valbuffer[vb++] = *str;
			continue;
		}

		switch (*str)
		{
			case ANSI_CURSOR_HOME:
				putchar(HOME);
				break;

			case ANSI_CLEAR_SCREEN:
				putchar(CLRSCR);
				break;

			case ANSI_CLEAR_LINE:
				// Not handled yet
				break;

			case ANSI_DECSTBM:
				// Not handled yet
				break;

			case ANSI_PRIVATE:  // Skip to next character
				continue;

			case ANSI_DEC_h: 
				// Not handled yet
				continue;

			case ANSI_SEPARATOR:
				values[vi++] = parse_ansi_value(valbuffer);
				vb = 0;
				continue;

			case ANSI_GRAPHICS_MODE:
				values[vi++] = parse_ansi_value(valbuffer);
				vb = 0;
				convert_ansi_color(values, vi);
				break;

			case ANSI_CURSOR_UP:
				putchar(UP);
				break;

			case ANSI_CURSOR_DOWN:
				putchar(DOWN);
				break;

			case ANSI_CURSOR_BACKWARD:
				putchar(LEFT);
				break;

			case ANSI_CURSOR_FORWARD:
				putchar(RIGHT);
				break;

			case ANSI_HPA:
				values[vi++] = parse_ansi_value(valbuffer);
				vb = 0;
				gotox(values[0]);
				break;

			case ANSI_VPA:
				values[vi++] = parse_ansi_value(valbuffer);
				vb = 0;
				gotoy(values[0]);
				break;

			default:
				printf("Unknown ANSI escape %d [%c]", *str, ascToPet[*str]);
				return str;
				break;
		}

		return str;
	}
}

void putstring_ansi(char* str) {
	for (; datacount > 0; ++str, --datacount) 
	{
		switch (*str) 
		{
			case LF:     // Ignore linefeeds
				break;

			case BELL:   // Ding!
				term_bell();
				break;

			case ANSI_ESCAPE:
				str = parse_ansi_escape(str++);
				break;

			default:
				#ifdef DEBUG_MODE
				if (debug) {
					printf("\nRECV ASC:[%d=%c]\n", *str, *str);
					printf("RECV PET:[%d=%c]\n", ascToPet[*str], ascToPet[*str]);
				}
				else
				#endif
				{
					putchar(ascToPet[*str]);
					asm("ldx #$00");
					#ifdef __C128__
						asm("stx $f4");
						asm("stx $f5");
					#else
						asm("stx $d4");
						asm("stx $d8");
					#endif
				}
				break;
		}
	}
}
