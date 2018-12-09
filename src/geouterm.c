#include <geos.h>
#include <conio.h>
#include <stdlib.h>
#include "ultimate_ii.h"
#include "geouterm-res.h"

//#define TESTING

struct window winHdr = {0, 15, 200, SC_PIX_WIDTH-1 };
struct window winTerm = {20, SC_PIX_HEIGHT-1, 0, SC_PIX_WIDTH-1};
struct window btmRow = {184, SC_PIX_HEIGHT-1, 0, SC_PIX_WIDTH-1};

unsigned char cx,cy;
unsigned char socketnr = 0;
int datacount = 0;
char buff[2] = {0,0};
unsigned char fontbuff40[1000];
unsigned char fontbuff80[600];
char host[80];
char portbuf[9];
unsigned short port;
unsigned int screen_width;
unsigned int font_width;

//void_func oldAppMainVector;

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

void change40col(void)
{
	InitDrawWindow (&winTerm);
	Rectangle();
	HorizontalLine(255, 20, 0, SC_PIX_WIDTH-1);
	
	cx = 0;
    cy = 4;
	
	LoadCharSet ((struct fontdesc *)(fontbuff40));
	screen_width=40;
	font_width = 8;
	GotoFirstMenu();
}

void change80col(void)
{
	InitDrawWindow (&winTerm);
	Rectangle();
	HorizontalLine(255, 20, 0, SC_PIX_WIDTH-1);
	
	cx = 0;
    cy = 4;

	LoadCharSet ((struct fontdesc *)(fontbuff80));
	screen_width=80;
	font_width = 4;
	cx=0;
	GotoFirstMenu();
}

void scroll_down(void)
{
	unsigned char t = 0;
	
	for(t=3;t<24;t++)
	{				
		MoveData(SCREEN_BASE + 320 *t, SCREEN_BASE + 320 * (t+1) ,319); // screen
	}
	cy=23;
	InitDrawWindow (&btmRow);
	Rectangle();
}

void handle_io_test(void)
{
	unsigned char ch = 0;
	unsigned char x = 0;
	unsigned char t = 0;

	ch = GetNextChar();

	if(ch != 0)
	{
		if (ch == 133)
		{
			cursor(0);
		}
		else
		{
			if (ch == 13)
			{
				cy += 1;
				cx = 0;
				if(cy == 24)
					scroll_down();
				
				//gotoxy(cx,cy);
			}
			else if (ch == 29)
			{
				if(cx == 0 && cy > 2)
				{
					cx=screen_width;
					cy--;
				}
				
				cx--;
				//gotoxy(cx,cy);
				//cputc(' ');
				//gotoxy(cx,cy);
				PutChar(' ', cy*8,cx*font_width);
			}
			else
			{
				if(cx == screen_width)
				{	
					cy += 1;
					cx = 0;
					if(cy == 24)
						scroll_down();

					//gotoxy(cx,cy);
				}
				//cputc(ch);
				PutChar(ch, cy*8, cx*font_width);
				cx++;
			}
		}
	}
}

void handle_io(void)
{
	unsigned char ch = 0;
	unsigned char x = 0;
	unsigned char t = 0;
	unsigned char *dat;


	InitForIO();
	uii_tcpsocketread(socketnr, 255);
	DoneWithIO();
	
	if (uii_status[0] == '0' && uii_status[1] == '0')
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
					
					//gotoxy(cx,cy);
				}
				else if (dat[x] == 8)
				{
					if(cx == 0 && cy > 2)
					{
						cx=screen_width;
						cy--;
					}
					
					cx--;
					//gotoxy(cx,cy);
					//cputc(' ');
					//gotoxy(cx,cy);
					PutChar(' ', cy*8,cx*font_width);
				}
				else
				{
					if(dat[x] > 31 && dat[x] < 127)
					{
						if(cx == screen_width)
						{	
							cy += 1;
							cx = 0;
							if(cy == 24)
								scroll_down();

							gotoxy(cx,cy);
						}
						//cputc(dat[x]);
						PutChar(dat[x], cy*8,cx*font_width);
						cx++;
					}
				}
			}
		}
	}
	else
	{
		if(uii_status[0] == '0' && uii_status[1] == '2') //no data
		{	
			ch = GetNextChar();

			if(ch != 0)
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
			}
		}
		else
		{
			cprintf("Error: %s", uii_status);
		}
	}
}

void hook_into_system(void)
{
    // hook into system vectors - preserve old value
    //oldMouseVector = mouseVector;
    //mouseVector = foo1;
    //oldKeyVector = keyVector;
    //keyVector = foo2;
#ifndef TESTING
	appMain = handle_io;
#else
	appMain = handle_io_test;
#endif
}

void remove_hooks(void)
{
    //mouseVector = oldMouseVector;
    //keyVector = oldKeyVector;
	appMain = 0;
}

void main(void)
{
	unsigned char *s = "GeoUTerm 64";
	unsigned char ch = 0;
	unsigned char x = 0;
	
	loadFonts();
	
	atexit(&remove_hooks);
	
	DoMenu(&mainMenu);

	SetPattern(0);
	
	InitDrawWindow (&winHdr);
	Rectangle();
	HorizontalLine(255, 1, 200, SC_PIX_WIDTH-1);
	HorizontalLine(255, 4, 200, SC_PIX_WIDTH-1);
	HorizontalLine(255, 6, 200, SC_PIX_WIDTH-1);
	HorizontalLine(255, 8, 200, SC_PIX_WIDTH-1);
	HorizontalLine(255, 10, 200, SC_PIX_WIDTH-1);
	HorizontalLine(255, 13, 200, SC_PIX_WIDTH-1);
	PutString (s, 9, 220);
	
	change40col();		

	//gotoxy(cx,++cy);

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
	
	if (uii_status[0] == '0' && uii_status[1] == '0')
	{
#endif
		hook_into_system();

#ifndef TESTING	
	}
#endif

	MainLoop();
}