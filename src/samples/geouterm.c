/*****************************************************************
Ultimate 64/II/+ Telnet GEOS 64/128 Demo
Scott Hutter

Based on ultimate_dos-1.1.docx and command interface.docx
https://github.com/markusC64/1541ultimate2/tree/master/doc

Disclaimer:  Because of the nature of DOS commands, use this code
soley at your own risk.

Patches and pull requests are welcome

Demo program does not alter any data
******************************************************************/

#include <geos.h>
#include <conio.h>
#include <stdlib.h>
#include <peekpoke.h>
#include "../lib/ultimate_lib.h"
#include "geouterm-res.h"

//#define TESTING

void vdc_write_reg(void);
void vdc_copyline(unsigned char srchi, unsigned char srclo, unsigned char desthi, unsigned char destlo);


struct window *winTerminal;
struct window *winHeader;
struct window *winBottomRow;

struct window vic_winHdr = {0, 15, 200, SC_PIX_WIDTH-1 };
struct window vic_winTerm = {20, SC_PIX_HEIGHT-1, 0, SC_PIX_WIDTH-1};
struct window vic_BtmRow = {184, SC_PIX_HEIGHT-1, 0, SC_PIX_WIDTH-1};

struct window vdc_winHdr = {0, 15, 400, SCREENPIXELWIDTH-1 };
struct window vdc_winTerm = {20, SC_PIX_HEIGHT-1, 0, SCREENPIXELWIDTH-1};
struct window vdc_BtmRow = {179, SC_PIX_HEIGHT-1, 0, SCREENPIXELWIDTH-1};

unsigned char cx,cy;
unsigned char socketnr = 0;
int datacount = 0;
char buff[2] = {0,0};
unsigned char fontbuff40[1000];
unsigned char fontbuff80[600];
char host[80];
char portbuf[9];
unsigned short port;
unsigned int screen_cols;
unsigned short screen_pixel_width;
unsigned int font_width;
char osType = 0;
char vdc = 0;

void_func oldKeyVector;

void loadFonts()
{
	if(OpenRecordFile("cbm40") != 0)
	{
		 DlgBoxOk ("Error accessing font.", "'cbm40' not found.");
		 exit(0);
	}
	PointRecord(8);
	ReadRecord(fontbuff40, 970);
	CloseRecordFile();
	
	if(OpenRecordFile("cbm80") != 0)
	{
		 DlgBoxOk ("Error accessing font.", "'cbm80' not found.");
		 exit(0);
	}
	PointRecord(8);
	ReadRecord(fontbuff80, 594);
	CloseRecordFile();
}

void clearTerminal(unsigned char fontsize)
{	
	unsigned char *s = "  GeoUTerm  ";
	unsigned short hdrY = 0;

	if ((osType & GEOS64) == GEOS64) // c64
	{
		screen_pixel_width = SC_PIX_WIDTH;
		winHeader = &vic_winHdr;
		winTerminal = &vic_winTerm;
		winBottomRow = &vic_BtmRow;
		hdrY = 200;
		vdc = 0;
	}
	
	if ((osType & GEOS128) == GEOS128) // c128
	{
		if((graphMode & 0x80) == 0x00)
		{
			// 40 col mode
			screen_pixel_width = SC_PIX_WIDTH;
			winHeader = &vic_winHdr;
			winTerminal = &vic_winTerm;
			winBottomRow = &vic_BtmRow;
			hdrY = 200;
			vdc = 0;
		}
		else if((graphMode & 0x80) == 0x80)
		{
			// == 0x80 - 80 col mode
			screen_pixel_width = SCREENPIXELWIDTH-1;
			winHeader = &vdc_winHdr;
			winTerminal = &vdc_winTerm;
			winBottomRow = &vdc_BtmRow;
			hdrY = 400;
			vdc = 1;
		}
	}
	
	SetPattern(0);
		
	InitDrawWindow (winTerminal);
	Rectangle();
	HorizontalLine(255, 20, 0, screen_pixel_width-1);

	InitDrawWindow (winHeader);
	Rectangle();
	HorizontalLine(255, 1, hdrY, screen_pixel_width-1);
	HorizontalLine(255, 4, hdrY, screen_pixel_width-1);
	HorizontalLine(255, 6, hdrY, screen_pixel_width-1);
	HorizontalLine(255, 8, hdrY, screen_pixel_width-1);
	HorizontalLine(255, 10, hdrY, screen_pixel_width-1);
	HorizontalLine(255, 13, hdrY, screen_pixel_width-1);
	
	UseSystemFont();
	PutString (s, 9, hdrY+20);
	
	if(fontsize == 40)
	{
		LoadCharSet ((struct fontdesc *)(fontbuff40));
		screen_cols=40;
		
		if ((osType & GEOS128) == GEOS128) // c128
		{
			if((graphMode & 0x80) == 0x80)
			{
				screen_cols = 80;
			}
		}
		
		font_width = 8;
	}
	
	if (fontsize == 80)
	{
		LoadCharSet ((struct fontdesc *)(fontbuff80));
		screen_cols=80;
		font_width = 4;
	}
	
	cx = 0;
    cy = 4;
}

void set40col(void)
{
	clearTerminal(40);
	GotoFirstMenu();
}

void set80col(void)
{
	clearTerminal(80);
	GotoFirstMenu();
}

void scroll_down(void)
{
	unsigned char t = 0;
	unsigned short srcmem = 0;
	unsigned short destmem = 0;
	
	if(vdc == 0)
	{
		for(t=3;t<24;t++)
		{				
			MoveData(SCREEN_BASE + screen_pixel_width *t, SCREEN_BASE + screen_pixel_width * (t+1) ,screen_pixel_width-1); // screen
		}
		
		cy=23;
		InitDrawWindow (winBottomRow);
		Rectangle();
	}
	else
	{
		for(t=33;t<192;t++)
		{
			srcmem = 80 * t;
			destmem = 80 * (t-8);
			vdc_copyline(srcmem>>8, srcmem & 0xff, destmem>>8, destmem & 0xff);
			
			//srcmem = (80 * t) + 0x800;
			//destmem = (80 * (t-8)) + 0x800;
			//vdc_copyline(srcmem>>8, srcmem & 0xff, destmem>>8, destmem & 0xff);
		}
		
		cy=23;
		//InitDrawWindow (winBottomRow);
		//Rectangle();
		gotoxy(cx,cy);
		cputs("                                                                                  ");
		gotoxy(cx,cy);
	}
	
	
}

void handle_io(void)
{
	unsigned char ch = 0;
	unsigned char x = 0;
	unsigned char t = 0;
	unsigned char *dat;
	unsigned char buf[3];

	#ifdef TESTING
	return;
	#endif

	InitForIO();
	uii_tcpsocketread(socketnr, 255);	
	DoneWithIO();
	
	buf[0] = uii_status[0];
	buf[1] = uii_status[1];
	buf[2] = 0;
	
	if (CmpString(buf, "00") == 0)
	{	
		datacount = uii_data[0] | (uii_data[1]<<8);

		if(datacount > 0)
		{
			dat = &(uii_data[2]);
			
			for(x=0;x<datacount;x++)
			{
				if (dat[x] == 10)
				{
					continue;
				}
				else if (dat[x] == 13)
				{
					cy += 1;
					cx = 0;
					if(cy == 24)
						scroll_down();
				}
				else if (dat[x] == 8)
				{
					if(cx == 0 && cy > 2)
					{
						cx=screen_cols;
						cy--;
					}
					cx--;
					PutChar(' ', cy*8,cx*font_width);
				}
				else
				{
					if(dat[x] > 31 && dat[x] < 127)
					{
						if(cx == screen_cols)
						{	
							cy += 1;
							cx = 0;
							if(cy == 24)
								scroll_down();

							gotoxy(cx,cy);
						}
						PutChar(dat[x], cy*8,cx*font_width);
						cx++;
					}
				}
			}
		}
	}
	else
	{
		if(CmpString(buf, "02") != 0) //no data
		{	
			cprintf("Error: %s", uii_status);
		}
	}
}

void handle_key(void)
{
	unsigned char ch = 0;
	unsigned char x = 0;
	unsigned char t = 0;

	#ifndef TESTING
	ch = keyData;
	
	while (ch != 0)
	{
		if (ch == 133)
		{
			cursor(0);

			InitForIO();
			uii_tcpclose(socketnr);
			DoneWithIO();
		}
		else
		{
			buff[0] = ch;
			buff[1] = 0;

			InitForIO();
			uii_tcpsocketwrite(socketnr, buff);
			DoneWithIO();
		}
	
		ch = GetNextChar();
	}
	
	#else
	
	ch = keyData;
	
	while (ch != 0)
	{
		if (ch == 13)
		{
			cy += 1;
			cx = 0;
			if(cy == 24)
				scroll_down();
		}
		else if (ch == 29)
		{
			if(cx == 0 && cy > 2)
			{
				cx=screen_cols;
				cy--;
			}
			
			cx--;
			PutChar(' ', cy*8,cx*font_width);
		}
		else
		{
			if(cx == screen_cols)
			{	
				cy += 1;
				cx = 0;
				if(cy == 24)
					scroll_down();
			}
			PutChar(ch, cy*8, cx*font_width);
			cx++;
		}

		ch = GetNextChar();
	}
	
	#endif
}

void hook_into_system(void)
{
    // hook into system vectors - preserve old value
    //oldMouseVector = mouseVector;
    //mouseVector = foo1;
    //oldKeyVector = keyVector;
    //keyVector = foo2;

	appMain = handle_io;

	oldKeyVector = keyVector;
    keyVector = handle_key;

}

void remove_hooks(void)
{
    //mouseVector = oldMouseVector;
    keyVector = oldKeyVector;
	appMain = 0;
}

void switch4080(void)
{	
	//asm("lda $3f");
	//asm("eor #$80");
	//asm("sta $3f");
	//asm("jsr $c2dd"); // SetNewMode

	SetNewMode();
	
	set40col();
	DoMenu(&mainMenu);
}

void main(void)
{
	unsigned char ch = 0;
	unsigned char x = 0;
	
	osType = get_ostype();
	loadFonts();
	atexit(&remove_hooks);

	if((osType & GEOS128) == GEOS128)
	{	
		if((graphMode & 0x80) == 0x00)
		{
			DlgBoxOk ("GeoUTerm v1.0", "Commodore 128 40 column mode");
			set40col();
			DoMenu(&mainMenu);
		}
		else
		{
			DlgBoxOk ("GeoUTerm v1.0", "Commodore 128 80 column mode");
			set40col();
			DoMenu(&mainMenu);
		}	
	}
	else
	{
		DlgBoxOk ("GeoUTerm v1.0", "For the Commodore 64");
		set40col();	
		DoMenu(&mainMenu);
	}

#ifndef TESTING	
	InitForIO();
	uii_settarget(TARGET_NETWORK);
	DoneWithIO();
	
	InitForIO();
	uii_identify();
	DoneWithIO();
#endif

	gotoxy(cx,++cy);
	cprintf("NIC Status: %s", uii_status);

#ifndef TESTING	
	InitForIO();
	uii_getipaddress();
	DoneWithIO();
#endif

	gotoxy(cx,++cy);
	cprintf("IP Address: %d.%d.%d.%d", uii_data[0], uii_data[1], uii_data[2], uii_data[3]);
	gotoxy(cx,++cy);
	cprintf("   Netmask: %d.%d.%d.%d", uii_data[4], uii_data[5], uii_data[6], uii_data[7]);
	gotoxy(cx,++cy);
	cprintf("   Gateway: %d.%d.%d.%d", uii_data[8], uii_data[9], uii_data[10], uii_data[11]);
	gotoxy(cx,++cy);

	if (DlgBoxGetString (host, 80, "Host name:", "") == CANCEL)
		exit(0);

	if(DlgBoxGetString (portbuf, 80, "Port:", "") == CANCEL)
		exit(0);
		
	port = atoi(portbuf);
	
	gotoxy(cx,++cy);
	cprintf(" * Connecting to %s:%u...", host,port);
	gotoxy(cx,++cy);
	gotoxy(cx,++cy);
	
#ifndef TESTING	
	InitForIO();
	uii_tcpconnect(host, port);
	DoneWithIO();
#endif
	
	cursor(1);
	
#ifndef TESTING	
	socketnr = uii_data[0];
	
	if (uii_success())
	{
#endif
		hook_into_system();

#ifndef TESTING	
	}
#endif

	MainLoop();
}

#pragma optimize (push,off)
void vdc_copyline(unsigned char srchi, unsigned char srclo, unsigned char desthi, unsigned char destlo)
{
	// Set src line
	asm("ldx #$20");
	asm("ldy #%o", srchi);
	asm("lda (sp),y");
	asm("jsr %v", vdc_write_reg);
	
	asm("ldx #$21");
	asm("ldy #%o", srclo);
	asm("lda (sp),y");
	asm("jsr %v", vdc_write_reg);
	
	// Set dest line
	asm("ldx #$12");
	asm("ldy #%o", desthi);
	asm("lda (sp),y");
	asm("jsr %v", vdc_write_reg);
	
	asm("ldx #$13");
	asm("ldy #%o", destlo);
	asm("lda (sp),y");
	asm("jsr %v", vdc_write_reg);
	
	// set copy mode
	asm("ldx #$18");
	asm("lda #$80");				// set bit 7 = copy
	asm("jsr %v", vdc_write_reg);
	
	// Set byte count (initates copy operation)
	asm("ldx #$1E");
	asm("lda #$50");				// 80 chars - 1
	asm("jsr %v", vdc_write_reg);
}
#pragma optimize (pop)

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
