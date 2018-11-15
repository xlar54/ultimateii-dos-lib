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

char *host="chat.freenode.net";
char *portbuff = "6667";
char *realname = "Bob Anonymous";
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
char *ping = "PING";

#define RVS_ON			0x12
#define RVS_OFF			0x92
#define CURSOR 			0xe4
#define LEFT			0x9d
#define DELETE			0x14
#define CG_COLOR_YELLOW 	0x9e
#define CG_COLOR_CYAN	 	0x9f
#define CG_COLOR_WHITE 	0x05

int getstring(char *buf)
{
	unsigned char c = 0;
	unsigned char x = 0;
	
	printf("%c", CURSOR);
	
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

void irc_login() // Handle IRC login procedures
{
    char NICK_STRING[64]; 
    char USER_STRING[128];

    sprintf(NICK_STRING, "nick %s\r\n", nick); // NICK user
    sprintf(USER_STRING, "user %s * 0 :%s\r\n", nick, strlen(realname) == 0 ? nick : realname); // USER user * 0 :Real name

	uii_tcpsocketwrite(socketnr, NICK_STRING);
	printf("\n%s", NICK_STRING);
	
	uii_tcpsocketwrite(socketnr, USER_STRING);
	printf("\n%s", USER_STRING);
	
	uii_tcpsocketwrite(socketnr, "join #ultimate64\n");
	printf("\njoin #ultimate64\n");

}
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


void irc_print(unsigned char *msgptr, unsigned char leadingNewLineFlag)
{
	int y =0;
	
	if(leadingNewLineFlag == 1 && wherex() != 79)
		printf("\n");
	
	if(wherey() > 21)
	{
		cputsxy(0,22,"                                        ");
		asm("jsr $e8ea");
		chlinexy(0,22,40);
		gotoxy(0,23);
		printf("%s",outbuf);
		cputcxy(wherex(), wherey(), CURSOR);
		gotoxy(0,21);
	}
	
	for(y=0;y<strlen(msgptr);y++)
	{
		if(wherey() > 21)
		{
			cputsxy(0,22,"                                        ");
			asm("jsr $e8ea");
			chlinexy(0,22,40);
			gotoxy(0,23);
			printf("%s",outbuf);
			cputcxy(wherex(), wherey(), CURSOR);
			gotoxy(0,21);
		}
		printf("%c", convertchar(msgptr[y]));
	}
}

void irc_pong(unsigned char *buf)
{
    buf[0] = 'p';
	buf[1] = 'o';
	buf[2] = 'n';
	buf[3] = 'g';
	//irc_print(buf, 1);
	uii_tcpsocketwrite(socketnr, buf);
	uii_tcpsocketwrite(socketnr, "\r\n");
}

int irc_message(char *buf)
{	
    char full_message[100]; 
	unsigned char x = 0;
	
	for(x=0;x<strlen(buf);x++)
		buf[x] = convertchar(buf[x]);
	
    sprintf(full_message, "privmsg %s :%s\n", "#ultimate64", buf); // PRIVMSG <channel> :Message text
	uii_tcpsocketwrite(socketnr, full_message);
	
	irc_print("<",1);
	irc_print(nick,0);
	irc_print("> ",0);
	irc_print(buf,0);
	
    return 0;
}

int StartsWith(const char *a, const char *b)
{
   if(strncmp(a, b, strlen(b)) == 0) return 1;
   return 0;
}

void main(void) 
{
	int count = 0;
	int port = 0;
	int datacount = 0;
	unsigned char c = 0;
	char buff[2] = {0,0};
	int x = 0;
	int y = 0;
	unsigned char i = 0;
	unsigned char curs = 0;
	unsigned char newline = 0;
	unsigned char connected = 0;
	char *msgptr;
	char *ptr;
	unsigned char* sender;

	POKEW(0xD020,0);
	POKEW(0xD021,0);
	
	clrscr();
	chlinexy(0,22,40);
	cputcxy(0,23, CURSOR);
	gotoxy(0,0);
	
	printf("%cUltimateChat v1.0%c", 0x05, 0x9f);
	printf("\nFreenode #ultimate64 channel");
	
	uii_settarget(TARGET_NETWORK);
	uii_identify();
	
	printf("\n\nNetwork interface : %s", uii_data);
	printf("\n           Status : %s", uii_status);
	
	uii_getipaddress();
	printf("\n\nIP Address: %d.%d.%d.%d", uii_data[0], uii_data[1], uii_data[2], uii_data[3]);
	printf("\n   Netmask: %d.%d.%d.%d", uii_data[4], uii_data[5], uii_data[6], uii_data[7]);
	printf("\n   Gateway: %d.%d.%d.%d", uii_data[8], uii_data[9], uii_data[10], uii_data[11]);
	printf("\n    Status: %s", uii_status);
	
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

	printf("\n\nNickname: ");
	getstring(nick);

	for(x=0;x<strlen(nick);x++)
		nick[x] = convertchar(nick[x]);
	
	inbufptr = 0;
	port = atoi(portbuff);
	
	printf("\n\nConnecting to: %s:%u\n", host, port);
	uii_tcpconnect(host, port);
	socketnr = uii_data[0];
	
	printf("\n\n[F1] to disconnect----------------------\n");
	
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
								//unsigned char* dest = (unsigned char*) malloc((41) * sizeof(unsigned char));
								
								i = 0;
								while(inbuf[i] != '!')
								{
									sender[i] = inbuf[i];
									i++;
								}
								
								sender[0] = '<';
								sender[i++] = '>';
								sender[i] = 0;
								
								irc_print(sender,1);
								newline = 0;
								free(sender); 
								//free(dest);
							}
							
							if (strstr(inbuf, " join ") != 0)
							{
								sender = (unsigned char*) malloc(80 * sizeof(unsigned char));
								//unsigned char* dest = (unsigned char*) malloc((41) * sizeof(unsigned char));
								
								i = 0;
								while(inbuf[i] != '!')
								{
									sender[i] = inbuf[i];
									i++;
								}
								
								sender[0] = '*';
								sender[i++] = ' ';
								sender[i++] = 106;
								sender[i++] = 111;
								sender[i++] = 105;
								sender[i++] = 110;
								sender[i++] = 101;
								sender[i++] = 100;
								sender[i++] = 46;
								sender[i] = 0;
								
								irc_print(sender,1);
								inbufptr = 0;
								free(sender); 
								continue;
								//free(dest);
							}
							
							if (strstr(inbuf, " part ") != 0)
							{
								sender = (unsigned char*) malloc(80 * sizeof(unsigned char));
								//unsigned char* dest = (unsigned char*) malloc((41) * sizeof(unsigned char));
								
								i = 0;
								while(inbuf[i] != '!')
								{
									sender[i] = inbuf[i];
									i++;
								}
								
								sender[0] = '*';
								sender[i++] = ' ';
								sender[i++] = 108;
								sender[i++] = 101;
								sender[i++] = 102;
								sender[i++] = 116;
								sender[i++] = 46;
								sender[i] = 0;
								
								irc_print(sender,1);
								inbufptr = 0;
								free(sender); 
								continue;
								//free(dest);
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
				if (c == 133)
				{
					printf("\n\nClosing connection");
					uii_tcpclose(socketnr);
					break;
				}
				else
				{
					if(c == DELETE && outx > 0)
					{
						tempx = wherex();
						tempy = wherey();
						
						cputcxy(outx, outy, ' ');
						
						if (outx < 0 && outy == 24)
						{
							outx = 80;
							outy--;
						}
						
						outx--;
						cputcxy(outx, outy, CURSOR);
						
						outbufptr--;
						outbuf[outbufptr] = 0;
						gotoxy(tempx, tempy);
					}
					else if(c == 13)
					{
						irc_message(outbuf);
						
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
	}
	else
	{
		printf("\nConnect failed: %s", uii_status);
	}

}