/*****************************************************************
Ultimate II+ UltimateTerm 64 and 128
Scott Hutter

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
#endif

#ifdef __C64__
#include <c64.h>
#define RESET_MACHINE 	asm("jmp $FCE2");
#endif

#include <conio.h>
#include <peekpoke.h>
#include <unistd.h>
#include "ultimate_ii.h"

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

#ifdef __C128__
#define SCREEN_WIDTH	80
#define DISPLAY_HEADER	printf("%cUltimateTerm 128 v%s %c",  CG_COLOR_WHITE, version, CG_COLOR_CYAN);
void vdc_write_reg(void);
void blank_vicII(void);
#else
#define SCREEN_WIDTH	40
#define DISPLAY_HEADER	printf("%cUltimateTerm v%s %c",  CG_COLOR_WHITE, version, CG_COLOR_CYAN);
#endif

int file_exists(char *name, unsigned char dev);
int term_getstring(char* def, char *buf);
void term_displayheader(void);
int putchar_ascii(int c);
int (*term_print)(int c) = putchar;
void term_getconfig(void);
void term_bell(void);
void term_window(unsigned char x, unsigned char y, unsigned char width, unsigned char height, int border);

void cursorOn(void);
void cursorOff(void);

char *version = "1.46";
char host[80];
char portbuff[10];
int port = 0;
unsigned char socketnr = 0;
unsigned char asciimode = 0;
unsigned char phonebookctr = 0;
unsigned char phonebook[20][80];
unsigned char dev = 0;
unsigned char pbtopidx = 0;
unsigned char pbselectedidx = 0;

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

int term_getstring(char* def, char *buf)
{
	unsigned char c = 0;
	unsigned char x = 0;
	
	cursorOn();
	for(x=0;x<strlen(def);x++)
	{
		buf[x] = def[x];
		putchar(def[x]);
	}
	
#ifdef __C64__
	POKE(198,0);
#else
	POKE(208,0);
#endif
	
	while(1)
	{
		c = kbhit();
		if(c != 0)
		{
			c = cgetc();
			switch(c) 
			{
				case 0x0D:
				{
					cursorOff();
					buf[x] = 0;
					return x;
				}
				case DELETE:
				{
					if(x > 0)
					{
						x--;
						cursorOff();
						putchar(LEFT);
						putchar(' ');
						putchar(LEFT);
						cursorOn();
					}
					break;
				}
				default:
				{
					if(c > 32 && c < 91)
					{
						buf[x++] = c;
						cursorOff();
						putchar(c);
						cursorOn();
					}
					break;
				}
			}
		}
	}
}

void term_displayheader(void)
{
	clrscr();
	DISPLAY_HEADER
	chlinexy(0,1,SCREEN_WIDTH);
}

int putchar_ascii(int c)
{
	c = ascToPet[(unsigned char) c];

	if (c == BELL)
		term_bell();
	else
		putchar(c);

	return c;
}

void term_window(unsigned char x, unsigned char y, unsigned char width, unsigned char height, int border)
{
	unsigned char i;
	char *spaces="                                        ";

	spaces[width-2] = 0;
	for(i=y+1;i<y+height;i++)
		cputsxy(x+1,i,spaces);
	if (!border) return;

	chlinexy(x+1,y,width-2);chlinexy(x+1,y+height,width-2);
	cvlinexy(x,y+1,height-1);cvlinexy(x+width-1,y+1,height-1);
	cputcxy(x,y,176);cputcxy(width-1,y,174);cputcxy(x,y+height,173);cputcxy(x+width-1,y+height,189);
}

void term_hostselect(void)
{
	unsigned char b[2];
	unsigned char hst[80];
	unsigned char ctr = 0;
	unsigned char x = 0;
	unsigned char y = 0;
	unsigned char *file = "0:u-term,s";
	unsigned char c = 0;
	int bytesRead = 0;
	
startover:
	strcpy(phonebook[0], "MANUAL ENTRY");
	strcpy(phonebook[1], "afterlife.dynu.org 6400");
	strcpy(phonebook[2], "bbs.jammingsignal.com 23");
	strcpy(phonebook[3], "borderlinebbs.dyndns.org 6400");
	strcpy(phonebook[4], "commodore4everbbs.dynu.net 6400");
	strcpy(phonebook[5], "eagleman.bounceme.net 6464");
	strcpy(phonebook[6], "hurricanebbs.dynu.net 6401");
	strcpy(phonebook[7], "particlesbbs.dyndns.org 6400");
	strcpy(phonebook[8], "bbs.retroacademy.it 6510");
	phonebookctr = 8;
	
	term_window(0, 14, 40, 10, 1);
	
	y = 15;
	if(dev < 8 || !file_exists(file, dev))
	{
		// cant get a device number or cant find phonebook file. use default hardcoded hosts
		cputsxy(9,14,"[  Default Phonebook  ]");

		for(ctr=0;ctr<=phonebookctr;ctr++)
			cputsxy(3,y + ctr, phonebook[ctr]);
	}
	else
	{
		cbm_open(2, dev, CBM_READ, file);
		// clear existing
		for(ctr=1;ctr<=phonebookctr;ctr++)
			phonebook[ctr][0] = 0;
		
		// load phonebook data
		cputsxy(9,14,"[ Loading Phonebook... ]");
		bytesRead = cbm_read(2, b, 1);	
		
		phonebookctr = 0;
		ctr=0;
		
		while(bytesRead > 0)
		{
			c = b[0];
			if(c == CR)
			{
				phonebookctr++;
				strcpy(phonebook[phonebookctr], hst);
				cputsxy(10,18,"                      ");
				cputsxy(10,18,"Entries found.....");
				cprintf("%d",phonebookctr);
				ctr=0;
			}
			else
			{
				if(c != 0x0A)
				{	
					// c to lowercase
					if ((c >= 97 && c <= 122) || (c >= 193 && c <= 218))
						c &= 95;
					
					hst[ctr] = c;
					ctr++;
					hst[ctr] = 0;
					
					// hostname too big
					if(ctr == 78)
						break;
				}
			}
			
			bytesRead = cbm_read(2, b, 1);
			
			// load any remaining items
			if(bytesRead == 0 && ctr != 0)
			{
				phonebookctr++;
				strcpy(phonebook[phonebookctr], hst);
				ctr=0;
			}
		};

		// handle error
		if(bytesRead == -1)
		{
			gotoxy(9,14);
			cprintf("[ Read Error: %d       ]", _oserror);
		}
		
		cbm_close(2);
		chlinexy(9,14,24);
		
		y = 15;
		pbtopidx = 0;
		
		// display 1st 8
		for(ctr=pbtopidx;ctr<=phonebookctr;ctr++)
		{
			gotoxy(3,y);
			cprintf("%s",phonebook[ctr]);
			y++;
			
			if(ctr == 8)
				break;
		}
	}
	
	y = 15;
	cputsxy(1,y,">");
	gotoxy(1,y);
	pbselectedidx = 0;
	
#ifdef __C64__
	POKE(198,0);
#else
	POKE(208,0);
#endif
	
	while(1)
	{
		c = kbhit();

		if(c != 0)
		{
			c = cgetc();
			
			if(c == DOWN && wherey() < 23 && (pbselectedidx + 1 <= phonebookctr))
			{
				cputsxy(1,y," ");
				y++;
				cputsxy(1,y,">");
				pbselectedidx++;
			}
			else if(c == DOWN && wherey() == 23 && (pbselectedidx + 1 <= phonebookctr))
			{
				if(phonebookctr >= pbtopidx+8)
				{
					term_window(0, 14, 40, 10, 0);
					pbtopidx++;
					y = 15;
					for(ctr=pbtopidx;ctr<=phonebookctr;ctr++)
					{
						gotoxy(3,y);
						cprintf("%s",phonebook[ctr]);
						y++;
						if(ctr == pbtopidx + 8)
							break;
					}
					y=23;
					cputsxy(1,y,">");
					pbselectedidx++;
				}
			}
			else if(c == UP && wherey() > 15)
			{
				cputsxy(1,y," ");
				y--;
				cputsxy(1,y,">");
				pbselectedidx--;
			}
			else if(c == UP && wherey() == 15)
			{
				if(pbtopidx > 0)
				{
					term_window(0, 14, 40, 10, 0);
					pbtopidx--;
					y = 15;
					for(ctr=pbtopidx;ctr<=phonebookctr;ctr++)
					{
						gotoxy(3,y);
						cprintf("%s",phonebook[ctr]);
						y++;
						if(ctr == pbtopidx + 8)
							break;
					}
					y=15;
					cputsxy(1,y,">");
					pbselectedidx--;
				}
			}
			else if(c == CR)
			{
				if(pbselectedidx == 0)
				{
					term_window(0, 14, 40, 10, 0);
					
					gotoxy(5,16);
					printf("%c", CG_COLOR_CYAN);
					cprintf("Host: ");
					printf("%c", CG_COLOR_WHITE);
					term_getstring("", host);
					
					gotoxy(5,18);
					printf("%c", CG_COLOR_CYAN);
					cprintf("Port: "); 
					printf("%c", CG_COLOR_WHITE);
					term_getstring("", portbuff);
					printf("%c", CG_COLOR_CYAN);
					
					if(host[0] == 0 || portbuff[0] == 0)
						goto startover;
					else
					{
						port = atoi(portbuff);
						return;
					}
				}
				else
				{
					ctr = 0;
					for(x=0;x<strlen(phonebook[pbselectedidx]);x++)
					{
						if(phonebook[pbselectedidx][x] != ' ')
						{
							host[ctr] = phonebook[pbselectedidx][x];
							ctr++;
							host[ctr] = 0;
						}
						else
						{
							x++;
							break;
						}
					}
					
					ctr = 0;
					for(;x<strlen(phonebook[pbselectedidx]);x++)
					{
						portbuff[ctr] = phonebook[pbselectedidx][x];
						ctr++;
						portbuff[ctr] = 0;
					}
					port = atoi(portbuff);
					return;
				}
			}
		}
	}
	
}

void term_getconfig(void)
{
	gotoxy(0,2);
	printf("Bug reports to: scott.hutter@gmail.com");
	printf("\n\n%cPlease ensure the following:%c",CG_COLOR_YELLOW,CG_COLOR_CYAN);
	printf("\n - Command Interface is enabled");
	printf("\n - Network link is in 'Link Up' state");
	printf("\n - Disable any emulated cartridges");

	uii_identify();
	printf("\n\nNIC Status: %c%s%c", CG_COLOR_WHITE, uii_status, CG_COLOR_CYAN);

	uii_getipaddress();
	printf("\nIP Address: %c%d.%d.%d.%d%c", CG_COLOR_WHITE,uii_data[0], uii_data[1], uii_data[2], uii_data[3],CG_COLOR_CYAN);
	printf("\n   Netmask: %c%d.%d.%d.%d%c", CG_COLOR_WHITE,uii_data[4], uii_data[5], uii_data[6], uii_data[7],CG_COLOR_CYAN);
	printf("\n   Gateway: %c%d.%d.%d.%d%c", CG_COLOR_WHITE,uii_data[8], uii_data[9], uii_data[10], uii_data[11],CG_COLOR_CYAN);
}

void term_bell(void)
{
	int x = 0;
	
	POKE(0XD400 + 24, 15);
	for(x=0; x< 2000; x++);
	POKE(0XD400 + 24, 0);
}

void main(void) 
{
	int datacount = 0;
	unsigned char c = 0;
	char buff[2] = {0,0};
	int x = 0;
	term_print = putchar;

	dev = getcurrentdevice();
	
	POKEW(0xD020,0);
	POKEW(0xD021,0);
	
#ifdef __C128__
	videomode(VIDEOMODE_80COL);
	putchar(14);
	blank_vicII();
	fast();
#endif

	// set up bell sound
	POKE(0XD400 + 5, 68);
	POKE(0XD400 + 6, 70);
	POKE(0XD400 + 4, 17);
	POKE(0XD400 + 1, 45);
	POKE(0XD400 + 0, 255);
	POKE(0XD400 + 24, 0);

	printf("Accessing network target...(if no response, perhaps connection was not closed?");
	
	uii_settarget(TARGET_NETWORK);
	cursorOff();
	while(1)
	{
		term_displayheader();
		term_getconfig();
		term_hostselect();
		cursorOff();

		term_displayheader();
		gotoxy(0,2);
		printf("%c\n[F1] to close the connection when done\n", CG_COLOR_YELLOW);

#ifdef __C128__
		printf("\n * Connecting to %s:%u\n\n",host, port);
#else
		printf("\n * Connecting to\n   %s:%u\n\n",host, port);
#endif
		
		uii_tcpconnect(host, port);
		socketnr = uii_data[0];
		
		if (uii_status[0] == '0' && uii_status[1] == '0')
		{
			putchar(CG_COLOR_CYAN);
			cursorOn();
			while(1)
			{
				uii_tcpsocketread(socketnr, 832);
				datacount = uii_data[0] | (uii_data[1]<<8);

				if(datacount > -1)
				{
					cursorOff();
					for(x=2;x<datacount+2;x++)
						#ifdef __C128__
						if (uii_data[x] != LF)
						#endif
						term_print(uii_data[x]);
					cursorOn();
				}

				c = kbhit();

				if(c != 0)
				{
					c = cgetc();
					if (c == 133)
					{
						printf("%c\n\nClosing connection", 14);
						uii_tcpclose(socketnr);
						break;
					}
					else if (c == 134)
					{
						asciimode = (asciimode == 1 ? 0 : 1);
						term_print = (asciimode ? putchar_ascii : putchar);
					}
					else
					{
						buff[0] = c;
						buff[1] = 0;
						uii_tcpsocketwrite(socketnr, buff);
					}
				}
			}
			cursorOff();
		}
		else
		{
			printf("\n%c * Connect failed:\n   %s", CG_COLOR_L_RED, uii_status);
			printf("\n\n * Press any key");
			
			c = 0;
			while(c==0)
			{
				c=kbhit();
			}
		}
	}
}

int file_exists(char *name, unsigned char dev) {
	int bytesRead;
	unsigned char b[2];
	if (cbm_open(127, dev, CBM_READ, name) != 0) return 0;
	bytesRead = cbm_read(127, b, 1);
	cbm_close(127);
	return bytesRead;
}

#pragma optimize (push, off)
void cursorOn(void) {
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
void cursorOff(void) {
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
void vdc_write_reg(void)
{
	asm("stx $d600");
vdc_write_wait:
	asm("ldx $d600");
	asm("bpl %g", vdc_write_wait);
	asm("sta $d601");

}
#pragma optimize (pop)

#pragma optimize (push,off)
void blank_vicII(void)
{
	asm("lda $d011");
	asm("and #$ef");
	asm("sta $d011");
}
#pragma optimize (pop)
#endif
