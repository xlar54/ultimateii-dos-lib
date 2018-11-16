/*****************************************************************
Ultimate II+ TCP Network IRC Demo
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

#ifdef __C128__
#include <c128.h>
#endif

#ifdef __C64__
#include <c64.h>
#endif

#include <conio.h>
#include <peekpoke.h>
#include <unistd.h>
#include "ultimate_ii.h"

int irc_handleinput(char *buf);

char *version = "1.2";
char host[25];
char portbuff[10];
char *realname = "Bob Anonymous";
char *nochan = "No Channel";
char channel[50];
char nick[255];
char inbuf[400];
char outbuf[160];
unsigned char outbufptr = 0;
unsigned char inbufptr = 0;
unsigned char socketnr = 0;
unsigned char outx = 0;
unsigned char outy = 23;
unsigned char tempx = 0;
unsigned char tempy = 0;


#define RVS_ON			0x12
#define RVS_OFF			0x92
#define CURSOR 			0xe4
#define LEFT			0x9d
#define DELETE			0x14
#define CG_COLOR_YELLOW 0x9e
#define CG_COLOR_CYAN	0x9f
#define CG_COLOR_WHITE 	0x05
#define CG_COLOR_L_GREEN 0x99
#define CG_COLOR_L_BLUE  0x9A
#define CG_COLOR_L_RED  0x96
#define CG_COLOR_L_GRAY  0x9B

#define SCREEN_WIDTH	40

unsigned char convertchar(unsigned char c)
{
	if(c > 64 && c < 91) 
	{
		c += 32;
	}
	else if (c > 96 && c < 123)
	{
		c -= 32;
	}
	else if (c >192 && c < 219)
	{
		c -= 128;
	}
	return c;
}

int getstring(char* def, char *buf)
{
	unsigned char c = 0;
	unsigned char x = 0;
	
	for(x=0;x<strlen(def);x++)
		buf[x] = def[x];
		
	printf("%s%c", buf, CURSOR);
	
	while(1)
	{
		c = kbhit();
		
		if(c != 0)
		{
			c = cgetc();
			if (c == 13)
			{
				buf[x] = 0;
				printf("%c ", LEFT);
				
				for(x=0;x<strlen(buf);x++)
					buf[x] = convertchar(buf[x]);
								
				return x;
			}
			else if (c == DELETE)
			{
				if(x > 0)
				{
					x--;
					printf("%c%c  %c%c%c",LEFT,LEFT,LEFT,LEFT,CURSOR);
				}
			}
			else
			{
				buf[x++] = c;
				printf("%c %c%c%c",LEFT,LEFT,c,CURSOR);
			}
		}
	}
}

void irc_updateheader(char *chan)
{
	unsigned char x = 0;
	unsigned char y = 0;
	unsigned char t = 0;
	unsigned char i = 0;
	
	x=wherex();
	y=wherey();
	t = 0;
	
	gotoxy(0,0);
	printf("%cUltimateChat v%s                       %c",  CG_COLOR_WHITE, version, CG_COLOR_CYAN);
	
	for(i=strlen(chan); i>0; i--)
	{
		cputcxy(39-t,0, chan[i-1]);
		t++;
	}
	gotoxy(x,y);
}

void irc_login() // Handle IRC login procedures
{
    char NICK_STRING[64]; 
    char USER_STRING[128];
	char *chan;

	chan = (unsigned char*) malloc(50 * sizeof(unsigned char));
    
	sprintf(NICK_STRING, "nick %s\r\n", nick); // NICK user
    sprintf(USER_STRING, "user %s * 0 :%s\r\n", nick, strlen(realname) == 0 ? nick : realname); // USER user * 0 :Real name
	sprintf(chan, "join %s\n", channel);
	
	uii_tcpsocketwrite(socketnr, NICK_STRING);
	uii_tcpsocketwrite(socketnr, USER_STRING);
	uii_tcpsocketwrite(socketnr, chan);
	
	irc_updateheader(channel);
	free(chan);
}


void irc_refreshscreen()
{
	unsigned char curx = 0;
	unsigned char cury = 0;
	
	// store current cursor position
	curx = wherex();
	cury = wherey();
	
	// header
	gotoxy(0,0);
	printf("%cUltimateChat v%s                       %c",  CG_COLOR_WHITE, version, CG_COLOR_CYAN);
	chlinexy(0,1,SCREEN_WIDTH);
	
	// footer
	chlinexy(0,22,SCREEN_WIDTH);
	gotoxy(0,23);
	printf("%s",outbuf);
	cputcxy(outx, outy, CURSOR);
	
	// get back where we were
	gotoxy(curx,cury);
}

void irc_print(char *buf, int newlineflg)
{
	unsigned char x = 0;
	unsigned char t = 0;
	unsigned char curx = 0;
	unsigned char cury = 0;
	
	
	curx = wherex();
	cury = wherey();
	
	gotoxy(curx,21);
	
	for(x=0;x<strlen(buf);x++)
	{	
		if(curx == SCREEN_WIDTH || newlineflg == 1 || buf[x] == '\r' || buf[x] == '\n')
		{
			//Scroll up		
			for(t=0;t<19;t++)
			{				
				memcpy(1104+SCREEN_WIDTH*t, 1104+SCREEN_WIDTH*(t+1),SCREEN_WIDTH); // screen
				memcpy(55376+SCREEN_WIDTH*t, 55376+SCREEN_WIDTH*(t+1),SCREEN_WIDTH); // color
			}
			
			curx = 0; cury = 21;
			cputsxy(curx, cury, "                                        ");
			gotoxy(curx,cury);
			
			newlineflg = 0;
		}
		
		if(buf[x] != '\r' && buf[x] != '\n')
		{
			cputcxy(curx,cury, convertchar(buf[x]));
			curx++;

		}
	}
	
}

void irc_pong(unsigned char *buf)
{
    buf[0] = 'p';
	buf[1] = 'o';
	buf[2] = 'n';
	buf[3] = 'g';

	uii_tcpsocketwrite(socketnr, buf);
	uii_tcpsocketwrite(socketnr, "\r\n");
}

void irc_help()
{
	printf("%c", CG_COLOR_WHITE);
	irc_print("",1);
	irc_print("cOMMANDS",1);
	
	irc_print("/JOIN #CHANNEL - JOIN CHANNEL",1);
	irc_print("/PART          - LEAVE CURRENT CHANNEL",1);
	irc_print("/QUIT          - QUIT PROGRAM",1);
	irc_print("/HELP          - THIS LIST",1);
	irc_print("",1);
	printf("%c", CG_COLOR_CYAN);
}

int irc_handleinput(char *buf)
{	
    char full_message[100]; 
	unsigned char x = 0;
	unsigned char s = 0;

	if (strlen(buf) == 0)
		return 0;
	
	if(strstr(buf,"/join") == buf)
	{
		if(channel[0] != 0)
		{
			irc_print("** yOU ARE ALREADY IN A CHANNEL.",1);
			irc_print("** uSE /part TO LEAVE FIRST.",1);
			return 0;
		}
		
		strcpy(channel, &buf[5]);
		irc_updateheader(channel);
		
		sprintf(full_message, "join %s\n", channel);
		uii_tcpsocketwrite(socketnr, full_message);
	}
	else if(strstr(buf,"/part") == buf)
	{
		if(channel[0] == 0)
		{
			irc_print("** yOU ARE NOT IN A CHANNEL.",1);
			return 0;
		}
		
		irc_updateheader(nochan);
		
		sprintf(full_message, "part %s\n", channel); // Leave current channel
		uii_tcpsocketwrite(socketnr, full_message);
		channel[0] = 0;
	}
	else if(strstr(buf,"/quit") == buf)
	{
		uii_tcpclose(socketnr);
		asm("jmp $FCE2");
	}
	else if(strstr(buf,"/help") == buf)
	{
		irc_help();
	}
	else
	{
		if(channel[0] == 0)
		{
			irc_print("** yOU ARE NOT IN A CHANNEL.",1);
			return 0;
		}
		
		for(x=0;x<strlen(buf);x++)
			buf[x] = convertchar(buf[x]);
		
		sprintf(full_message, "privmsg %s :%s\n", channel, buf); // PRIVMSG <channel> :Message text
		uii_tcpsocketwrite(socketnr, full_message);
		
		printf("%c", CG_COLOR_CYAN);
		irc_print("<",1);
		printf("%c", CG_COLOR_L_RED);
		irc_print(nick,0);
		printf("%c", CG_COLOR_CYAN);
		irc_print("> ",0);
		printf("%c", CG_COLOR_L_RED);
		irc_print(buf,0);
	}
	
    return 0;
}

void getconfig()
{
	uii_identify();
	printf("\n\nNetwork Interface Status : %s", uii_status);

	uii_getipaddress();
	printf("\n\nIP Address: %d.%d.%d.%d", uii_data[0], uii_data[1], uii_data[2], uii_data[3]);
	printf("\n   Netmask: %d.%d.%d.%d", uii_data[4], uii_data[5], uii_data[6], uii_data[7]);
	printf("\n   Gateway: %d.%d.%d.%d", uii_data[8], uii_data[9], uii_data[10], uii_data[11]);
	
	if(uii_data[0] == 0)
	{
		printf("\n\nUnable to access network interface.");
		printf("\nPlease ensure the following:");
		printf("\n - Command Interface is enabled");
		printf("\n - Network link is in 'Link Up' state");
		printf("\n - Disable any cartridges like");
		printf("\n	 the Action Replay or FC III");
		return;
	}
	
	do
	{
		gotoxy(0,9);
		printf("                                      ");
		printf("\n                                      ");
		printf("\n                                      ");
		printf("\n                                      ");
		
		gotoxy(0,9);
		printf("  Server: "); getstring("chat.freenode.net", host);
		printf("\n    Port: "); getstring("6667", portbuff);
		printf("\n Channel: ");	getstring("#c64friends", channel);
		printf("\nNickname: ");	getstring("", nick);
	
		if(host[0] == 0) printf("\n\n* You must provide a server!");
		if(portbuff[0] == 0) printf("\n\n* You must provide a port!");
		if(channel[0] == 0) printf("\n\n* You must provide a channel!");
		if(nick[0] == 0) printf("\n\n* You must provide a nickname!");
	
	}while(host[0] == 0 || portbuff[0] == 0 || channel[0] == 0 || nick[0] == 0);
	
}

void main(void) 
{
	int count = 0;
	int port = 0;
	int datacount = 0;
	int num = 0;
	unsigned char c = 0;
	char buff[2] = {0,0};
	int x = 0;
	int y = 0;
	unsigned char i = 0;
	unsigned char curs = 0;
	unsigned char newline = 0;
	unsigned char connected = 0;
	char *msgptr;
	unsigned char* sender;

	POKEW(0xD020,0);
	POKEW(0xD021,0);
	
	clrscr();
	irc_refreshscreen();
		
	uii_settarget(TARGET_NETWORK);
	
	getconfig();
	
	inbufptr = 0;
	port = atoi(portbuff);

	//irc_clearwindow();
	
	printf("\n\nConnecting to: %s:%u\n", host, port);
	uii_tcpconnect(host, port);
	socketnr = uii_data[0];
	
	printf("\n\n/help for options--------------------\n");
	
	gotoxy(0,21);
	
	if (uii_status[0] == '0' && uii_status[1] == '0')
	{
		while(1)
		{
			uii_tcpsocketread(socketnr, 400);
			datacount = uii_data[0] | (uii_data[1]<<8);

			for(x=2;x<datacount+2;x++)
			{
				if(datacount > -1)
				{	
					if (uii_data[x] == 10) 
						continue;
					
					if (uii_data[x] == 13)
					{
						inbuf[inbufptr] = 0;
						msgptr = &inbuf[0];
						newline = 1;
						
						if(strstr(inbuf, "ping") == inbuf)
						{
							irc_pong(inbuf);
							inbufptr = 0;
							continue;
						}
						else if(inbuf[0] == ':')
						{
							if (strstr(inbuf, " privmsg ") != 0)
							{
								sender = (unsigned char*) malloc(41 * sizeof(unsigned char));
							
								i = 1;
								while(inbuf[i] != '!')
								{
									sender[i-1] = inbuf[i];
									i++;
								}
								sender[i-1] = 0;
								
								printf("%c", CG_COLOR_CYAN);
								irc_print("<",1);
								printf("%c", CG_COLOR_L_GREEN);
								irc_print(sender,0);
								printf("%c", CG_COLOR_CYAN);
								irc_print("> ",0);
								printf("%c", CG_COLOR_L_GREEN);
								
								newline = 0;
								free(sender); 
							}
							else if (strstr(inbuf, " join ") != 0)
							{
								sender = (unsigned char*) malloc(80 * sizeof(unsigned char));
								i = 1;
								while(inbuf[i] != '!')
								{
									sender[i-1] = inbuf[i];
									i++;
								}
								sender[i-1] = 0;
								
								printf("%c", CG_COLOR_YELLOW);
								irc_print(" * ",1);
								irc_print(sender,0);
								irc_print(" JOINED THE ROOM.",0);
								printf("%c", CG_COLOR_L_GRAY);
								
								inbufptr = 0;
								free(sender); 
								continue;
							} 
							else if (strstr(inbuf, " part ") != 0)
							{
								sender = (unsigned char*) malloc(80 * sizeof(unsigned char));
								i = 1;
								while(inbuf[i] != '!')
								{
									sender[i-1] = inbuf[i];
									i++;
								}
								sender[i-1] = 0;
								
								printf("%c", CG_COLOR_YELLOW);
								irc_print(" * ",1);
								irc_print(sender,0);
								irc_print(" LEFT THE ROOM.",0);
								printf("%c", CG_COLOR_L_GRAY);
								
								inbufptr = 0;
								free(sender); 
								continue;
							}
							
							for(y=1;y<inbufptr;y++)
							{
								if(inbuf[y] == ':')
								{
									msgptr = &inbuf[y+1];
									break;
								}
							}
						}
						
						irc_print(msgptr,newline);											
						inbufptr = 0;
						
						if(connected == 0)
						{
							connected = 1;
							sleep(20);
							irc_login();
						}
					}
					else
					{
						inbuf[inbufptr] = uii_data[x];
						inbufptr++;
					}					

				}
			}

			c = kbhit();

			if(c != 0)
			{
				c = cgetc();

				if(c == DELETE)
				{
					if(outbufptr == 0)
						continue;

					tempx = wherex();
					tempy = wherey();
					
					cputcxy(outx, outy, ' ');
					
					if (outx == 0 && outy == 24)
					{
						outx = 39;
						outy--;
					}
					else
					{
						outx--;
					}
					
					cputcxy(outx, outy, CURSOR);
					outbufptr--;
					outbuf[outbufptr] = 0;
					gotoxy(tempx, tempy);

				}
				else if(c == 13)
				{
					irc_handleinput(outbuf);
					
					tempx = wherex();
					tempy = wherey();
					
					cputsxy(0,23,"                                        ");
					cputsxy(0,24,"                                        ");

					outx = 0;
					outy = 23;
					cputcxy(outx, outy, CURSOR);
					gotoxy(tempx, tempy);
					outbufptr = 0;
					outbuf[outbufptr]=0;
				}
				else
				{
					if(outbufptr < 79)
					{
						tempx = wherex();
						tempy = wherey();
						
						cputcxy(outx++, outy, c);

						if(outx > 39)
						{
							outx = 0;
							outy++;
						}
						
						cputcxy(outx, outy, CURSOR);
						
						outbuf[outbufptr] = c;
						outbufptr++;
						outbuf[outbufptr] = 0;
						gotoxy(tempx, tempy);
					}
				}
					

			}
		}
	}
	else
	{
		printf("\nConnect failed: %s", uii_status);
	}

}