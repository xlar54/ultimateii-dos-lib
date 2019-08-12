/*****************************************************************
Ultimate II+ UltimateTerm 64 and 128
Scott Hutter, Francesco Sblendorio

Based on ultimate_dos-1.1.docx and command interface.docx
https://github.com/markusC64/1541ultimate2/tree/master/doc

Disclaimer:  Because of the nature of DOS commands, use this code
soley at your own risk.

Patches and pull requests are welcome

Demo program does not alter any data
******************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <cbm.h>
#include <errno.h>
#include <device.h>

#ifdef __C128__
#include <c128.h>
#define RESET_MACHINE	asm("jmp $FF3D");
#define SCREEN_WIDTH	80
#define KEYBOARD_BUFFER 208
#define QUOTE_MODE      244
#define DISPLAY_HEADER	printf("%c%cUltimateTerm 128 v%s %c", 14, CG_COLOR_WHITE, version, CG_COLOR_CYAN);
void blank_vicII(void);
#endif

#ifdef __C64__
#include <c64.h>
#define RESET_MACHINE 	asm("jmp $FCE2");
#define SCREEN_WIDTH	40
#define KEYBOARD_BUFFER 198
#define QUOTE_MODE      212
#define DISPLAY_HEADER	printf("%c%cUltimateTerm v%s %c", 14, CG_COLOR_WHITE, version, CG_COLOR_CYAN);
#endif

#include <conio.h>
#include <peekpoke.h>
#include <unistd.h>
#include "../lib/ultimate_lib.h"
#include "screen_utility.h"

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
#define	CR				0x0D
#define SHIFT_CR		0x8D
#define LF				0x0A

#define CG_COLOR_YELLOW 	0x9e
#define CG_COLOR_CYAN		0x9f
#define CG_COLOR_WHITE 		0x05
#define CG_COLOR_L_GREEN 	0x99
#define CG_COLOR_L_BLUE  	0x9A
#define CG_COLOR_L_RED  	0x96
#define CG_COLOR_L_GRAY  	0x9B
#define SPACE38				"                                      "

#define PB_SIZE 1640

#define SOH  ((char)1)     /* Start Of Header */
#define EOT  ((char)4)     /* End Of Transmission */
#define ACK  ((char)6)     /* ACKnowlege */
#define NAK  ((char)0x15)  /* Negative AcKnowlege */

void uii_data_print(void);
unsigned char term_bell(void);
unsigned char term_getchars(char* def, char *buf, unsigned char lbound, unsigned char ubound);
void term_displayheader(void);
void putstring_ascii(char* str);
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

char *version = "2.3";
char host[80];
char portbuff[10];
char strbuff[520];
unsigned int port = 0;
unsigned char socketnr = 0;
unsigned char asciimode;
unsigned char pb_loaded = 0;
unsigned char phonebookctr = 0;
unsigned char phonebook[20][80];
unsigned char dev = 0;
unsigned char cur_dev = 0;
unsigned char pbtopidx = 0;
unsigned char pbselectedidx = 0;
unsigned char pb_bytes[PB_SIZE+1];
unsigned char hst[80];
unsigned file_index;
unsigned char y = 0;
unsigned char px = 0;
unsigned char py = 0;
int intbuff = 0;

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
0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5a,0xdb,0xdc,0xdd,0xde,0xdf,
0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8a,0x8b,0x8c,0x8d,0x8e,0x8f,
0x90,0x91,0x92,0x0c,0x94,0x95,0x96,0x97,0x98,0x99,0x9a,0x9b,0x9c,0x9d,0x9e,0x9f,
0xa0,0xa1,0xa2,0xa3,0xa4,0xa5,0xa6,0xa7,0xa8,0xa9,0xaa,0xab,0xac,0xad,0xae,0xaf,
0xb0,0xb1,0xb2,0xb3,0xb4,0xb5,0xb6,0xb7,0xb8,0xb9,0xba,0xbb,0xbc,0xbd,0xbe,0xbf,
0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,
0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7a,0x7b,0x7c,0x7d,0x7e,0x7f,
0xe0,0xe1,0xe2,0xe3,0xe4,0xe5,0xe6,0xe7,0xe8,0xe9,0xea,0xeb,0xec,0xed,0xee,0xef,
0xf0,0xf1,0xf2,0xf3,0xf4,0xf5,0xf6,0xf7,0xf8,0xf9,0xfa,0xfb,0xfc,0xfd,0xfe,0xff
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
	for (; *str; str++)
		if (*str != LF) *str==BELL ? term_bell() : putchar(ascToPet[*str]);
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
	cputcxy(1,y,'>');
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
	unsigned char *file = "0:u-term,s";
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
		strcpy(phonebook[1], "afterlife.dynu.com 6400");
		strcpy(phonebook[2], "bbs.jammingsignal.com 23");
		strcpy(phonebook[3], "borderlinebbs.dyndns.org 6400");
		strcpy(phonebook[4], "commodore4everbbs.dynu.net 6400");
		strcpy(phonebook[5], "eagleman.bounceme.net 6464");
		strcpy(phonebook[6], "hurricanebbs.dynu.net 6401");
		strcpy(phonebook[7], "particlesbbs.dyndns.org 6400");
		strcpy(phonebook[8], "bbs.retroacademy.it 6510");
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

			else if (c == 133) { // KEY F1
				help_screen();
				cursor_off();
			}

			else if (c == 139) { // KEY F6
				px = wherex(); py = wherey();
				dos_commands();
				cursor_off();
				gotoxy(37, 9); printf("%c%2d%c", CG_COLOR_YELLOW, cur_dev, CG_COLOR_CYAN);
				gotoxy(px, py);
			}

			else if (c == '+') {
				px = wherex(); py = wherey();
				if (cur_dev < 15) ++cur_dev;
				gotoxy(37, 9); printf("%c%2d%c", CG_COLOR_YELLOW, cur_dev, CG_COLOR_CYAN);
				gotoxy(px, py);
			}

			else if (c == '-') {
				px = wherex(); py = wherey();
				if (cur_dev > 8) --cur_dev;
				gotoxy(37, 9); printf("%c%2d%c", CG_COLOR_YELLOW, cur_dev, CG_COLOR_CYAN);
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
	printf("\n - Disable any emulated cartridges");
	printf("\n - %cPress F1%c to get help in session", CG_COLOR_WHITE, CG_COLOR_CYAN);

	uii_identify();
	printf("\n\nNIC Status: %c%s%c", CG_COLOR_WHITE, uii_status, CG_COLOR_CYAN);

	uii_getipaddress();
	printf("\nIP Address: %c%d.%d.%d.%d%c", CG_COLOR_WHITE,uii_data[0], uii_data[1], uii_data[2], uii_data[3],CG_COLOR_CYAN);
	printf("\n   Netmask: %c%d.%d.%d.%d%c", CG_COLOR_WHITE,uii_data[4], uii_data[5], uii_data[6], uii_data[7],CG_COLOR_CYAN);
	printf("\n   Gateway: %c%d.%d.%d.%d%c", CG_COLOR_WHITE,uii_data[8], uii_data[9], uii_data[10], uii_data[11],CG_COLOR_CYAN);

	gotoxy(29, 9); printf("\005+-%c Drive%c%2d", CG_COLOR_CYAN, CG_COLOR_YELLOW, cur_dev);
	gotoxy(29,10); printf("\005A%cdd   \005S%cave", CG_COLOR_CYAN, CG_COLOR_CYAN);
	gotoxy(29,11); printf("\005D%cel   \005L%coad", CG_COLOR_CYAN, CG_COLOR_CYAN);
	gotoxy(29,12); printf("\005E%cdit  \005Q%cuit", CG_COLOR_CYAN, CG_COLOR_CYAN);
	gotoxy(0,2);
}

void main(void) 
{
	int datacount;
	unsigned char c = 0;
	char buff[2];
	int x = 0;

	detect_uci();
	dev = getcurrentdevice();
	cur_dev = (dev < 8 ? 8 : dev);

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
	while(1) {
		asciimode = 0;
		term_displayheader();
		term_getconfig();
		term_hostselect();
		cursor_off();

		term_displayheader();
		gotoxy(0,2);
		printf("%c\n[F7] to close the connection when done\n", CG_COLOR_YELLOW);
		printf("\n * Connecting to\n   %s:%u\n\n", host, port);
		
		socketnr = uii_tcpconnect(host, port);
		
		if (uii_success()) {
			putchar(CG_COLOR_CYAN);
			cursor_on();
			while(1) {
				datacount = uii_tcpsocketread(socketnr, 892);

				if (datacount == 0) { // datacount == 0 means "disconnected"
					break;
				} else if (datacount > 0) {
					cursor_off();
					if (asciimode) putstring_ascii(uii_data+2); else uii_data_print();
					cursor_on();
				} // datacount == -1 means "wait state"

				c = kbhit();
				if (c != 0) {
					c = cgetc();
					buff[0] = c;
					buff[1] = 0;
					if (c == 133) // KEY F1: HELP
						help_screen();
					else if (c == 134) // KEY F3: switch petscii/ascii
						asciimode = !asciimode;
					else if (c == 135) // KEY F5: download (xmodem protocol)
						download_xmodem();
					else if (c == 139) // KEY F6: send DOS commands to disk
						dos_commands();
					else if (c == 136) // KEY F7: close connection
						break;
					else
						uii_tcpsocketwrite(socketnr, buff);
				}
			}
			uii_tcpclose(socketnr);
			cursor_off();
			if (c != 136) { // NOT KEY F7
				printf("\n\n%cconnection closed, hit any key", CG_COLOR_WHITE);
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
	asm("and #$ef");
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
	asm("ldy #$00");
loop:
	asm("lda ($fb),y");
	asm("beq %g", done);
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
#ifdef __C128__
skipline:
#endif
nextch:
	asm("iny");
	asm("bne %g", loop);
	asm("inc $fc");
	asm("jmp %g", loop);
done:
	asm("rts");
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
	gotoxy(LINE3,7);  printf("\022 F3 \222  Switch PETSCII/ASCII");
	gotoxy(LINE3,9);  printf("\022 F5 \222  Download with Xmodem");
	gotoxy(LINE3,11); printf("\022 F6 \222  DOS commands to disk");
	gotoxy(LINE3,13); printf("\022 F7 \222  Exit BBS");
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

void process_xmodem_sector(char sector[], char is_eot) {
	int len = 128;
	unsigned char status;

	if (is_eot)
		while (len > 0 && sector[len-1] == 26)
			--len;

	if (len > 0)
		status = cbm_write(2, sector, len);
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
		printf("#%d> ", cur_dev);
		term_getstring("", strbuff);
		putchar('\n');
		if (!strbuff[0] || !strcmp(".", strbuff))
			break;

		if (strbuff[0] == '#') {
			intbuff = atoi(strbuff+1);
			if (intbuff >= 8 && intbuff <= 15) cur_dev = intbuff;
		} else if (strbuff[0] == '$') {
			showdir(strbuff);
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
	intbuff = cbm_open(15, cur_dev, 15, s);
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
	intbuff = cbm_open(2, cur_dev, 0, s);
	if (intbuff) {
		cbm_close(2); cbm_close(15);
		printf("I/O error\n");
		return;
	}

	pb_bytes[0] = 0;
	cbm_open(15, cur_dev, 15, "");
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
            if (len_diskbuff == 0) return -1; /// if len = -1 o 0 ?
        } while (len_diskbuff == 0);
        result = strbuff[0];
        idiskbuff = 1;
    }
    return result;
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
	unsigned char errorcount;
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
	if (filename[0] == 0 || cur_dev < 8) {
		restore_screen();
		cursor_on();
		return;
	}
	for (i=0; i<strlen(filename); ++i) {
		c = filename[i];
		if ((c>=97 && c<=122) || (c>=193 && c<=218)) c &= 95;
		filename[i] = c;
	}
	gotoxy(LINEP3,8); printf("\022P\222RG or \022S\222EQ? ");
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
	} while (c != 'p' && c !='s');
	if (c == 'p') strcat(filename, ",p"); else strcat(filename, ",s");
	strcpy(scratch_cmd, "s:");
	strcat(scratch_cmd, filename);

	cursor_off();
	POKE(KEYBOARD_BUFFER, 0);
	putchar(c);
	gotoxy(0,10); printf("PLEASE WAIT.");
	cursor_off();
	// Open file in write mode
	cbm_close(15); cbm_close(2);
	cbm_open(15, cur_dev, 15, scratch_cmd); cbm_close(15);
	status = cbm_open(2, cur_dev, CBM_WRITE, filename);
	if (status) {
		printf("\n\nI/O ERROR. Download aborted.");
		cgetc();
		restore_screen();
		cursor_on();
		return;
	}

	// Start Xmodem transfer
	cursor_off();
	uii_tcpsocketwritechar(socketnr, NAK);

	firstread = 1;
	do {
		c = uii_tcp_nextchar(socketnr);
		if (!firstread) {
			putchar('.');
			process_xmodem_sector(sector, c == EOT);
		}
		if (c != EOT) {
			if (c != SOH) {
				printf("\nERROR: expected SOH, received: %d\n", c);
				if (++errorcount < MAXERRORS)
					continue;
				else {
					printf("\nFATAL: too may errors\n");
					break;
				}
			}
			b = uii_tcp_nextchar(socketnr);
			not_b = ~uii_tcp_nextchar(socketnr);
			if (b != not_b) {
				printf("\nERROR during checking blocknumber parity");
				++errorcount;
				continue;
			}
			if (b != blocknumber) {
				printf("\nERROR: Wrong blocknumber");
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
				++errorcount;
				continue;
			}
			uii_tcpsocketwritechar(socketnr, ACK);
			++blocknumber;

			if (errorcount != 0)
				uii_tcpsocketwritechar(socketnr, NAK);

			firstread = 0;
		}
	} while (c != EOT);

	//close file
	cbm_close(2);
	cbm_open(15, cur_dev, 15, "");
	cbm_read(15, pb_bytes, PB_SIZE);
	cbm_close(15);

	uii_tcpsocketwritechar(socketnr, ACK);

	uii_reset_uiidata();

	printf("\n\nDownload finished: press any key");
	POKE(KEYBOARD_BUFFER,0);
	cgetc();
	POKE(KEYBOARD_BUFFER,0);
	restore_screen();
	cursor_on();
}
