/*****************************************************************
Ultimate II+ TCP Network IRC Demo
Scott Hutter

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
#include "../lib/ultimate_lib.h"

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

#ifdef __C128__
#define SCREEN_WIDTH	80
#define MAX_OUTBUFFER	160
#else
#define SCREEN_WIDTH	40
#define MAX_OUTBUFFER	80
#endif

#define VDC_REG	0xd600

unsigned char convertchar(unsigned char c);
int getstring(char* def, char *buf);
void irc_updateheader(char *chan);
void irc_login(void);
void irc_refreshscreen(void);
void irc_print(char *buf, int newlineflg);
void irc_pong(char *buf);
void irc_help(void);
void irc_handleinput(char *buf);
void getconfig(void);

#ifdef __C128__
void vdc_write_reg(void);
void vdc_copyline(unsigned char srchi, unsigned char srclo, unsigned char desthi, unsigned char destlo);
#endif

char *version = "1.3";
char host[25];
char portbuff[10];
char *realname = "Bob Anonymous";
char *nochan = "No Channel";
char channel[50];
char nick[255];
char inbuf[400];
char outbuf[MAX_OUTBUFFER];
unsigned char outbufptr = 0;
unsigned char inbufptr = 0;
unsigned char socketnr = 0;
unsigned char outx = 0;
unsigned char outy = 23;
unsigned char tempx = 0;
unsigned char tempy = 0;
unsigned char rvs_vid = 0;


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
			
			switch(c) 
			{
				case 0x0D:
				{
					buf[x] = 0;
					printf("%c ", LEFT);
				
					for(x=0;x<strlen(buf);x++)
						buf[x] = convertchar(buf[x]);
									
					return x;
				}
				case DELETE:
				{
					if(x > 0)
					{
						x--;
						printf("%c%c  %c%c%c",LEFT,LEFT,LEFT,LEFT,CURSOR);
						
					}
					break;
				}
				default:
				{
					buf[x++] = c;
					printf("%c %c%c%c",LEFT,LEFT,c,CURSOR);
					break;
				}
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
#ifdef __C64__
	printf("%cUltimateChat 64 v%s                    %c",  CG_COLOR_WHITE, version, CG_COLOR_CYAN);
#endif

#ifdef __C128__
	printf("%cUltimateChat 128 v%s                                                         %c",  CG_COLOR_WHITE, version, CG_COLOR_CYAN);
#endif
	for(i=strlen(chan); i>0; i--)
	{
		cputcxy((SCREEN_WIDTH-1)-t,0, chan[i-1]);
		t++;
	}
	gotoxy(x,y);
}

void irc_login(void) // Handle IRC login procedures
{
    char NICK_STRING[64]; 
    char USER_STRING[128];
	char *chan;

	chan = (char*) malloc(50 * sizeof(unsigned char));
    
	sprintf(NICK_STRING, "nick %s\r\n", nick); // NICK user
    sprintf(USER_STRING, "user %s * 0 :%s\r\n", nick, strlen(realname) == 0 ? nick : realname); // USER user * 0 :Real name
	sprintf(chan, "join %s\n", channel);
	
	uii_socketwrite(socketnr, NICK_STRING);
	uii_socketwrite(socketnr, USER_STRING);
	uii_socketwrite(socketnr, chan);
	
	irc_updateheader(channel);
	free(chan);
}


void irc_refreshscreen(void)
{
	unsigned char curx = 0;
	unsigned char cury = 0;
	
	// store current cursor position
	curx = wherex();
	cury = wherey();
	
	// header
	gotoxy(0,0);
	
#ifdef __C64__
	printf("%cUltimateChat 64 v%s                    %c",  CG_COLOR_WHITE, version, CG_COLOR_CYAN);
#endif

#ifdef __C128__
	printf("%cUltimateChat 128 v%s                   %c",  CG_COLOR_WHITE, version, CG_COLOR_CYAN);
#endif

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
	unsigned short srcmem = 0;
	unsigned short destmem = 0;
	
	curx = wherex();
	cury = wherey();
	
	gotoxy(curx,21);
	
	for(x=0;x<strlen(buf);x++)
	{	
		if(curx == SCREEN_WIDTH || newlineflg == 1 || buf[x] == '\r' || buf[x] == '\n')
		{
#ifdef __C64__
			//Scroll up		
			for(t=0;t<19;t++)
			{				
				memcpy((unsigned short*)(0x0450+SCREEN_WIDTH*t), (unsigned short*)(0x0450+SCREEN_WIDTH*(t+1)),SCREEN_WIDTH); // screen
				memcpy((unsigned short*)(0xD850+SCREEN_WIDTH*t), (unsigned short*)(0xD850+SCREEN_WIDTH*(t+1)),SCREEN_WIDTH); // color
			}
#endif

#ifdef __C128__
			//Scroll up
			for(t=3;t<22;t++)
			{
				srcmem = 80 * t;
				destmem = 80 * (t-1);
				vdc_copyline(srcmem>>8, srcmem & 0xff, destmem>>8, destmem & 0xff);
				
				srcmem = (80 * t) + 0x800;
				destmem = (80 * (t-1)) + 0x800;
				vdc_copyline(srcmem>>8, srcmem & 0xff, destmem>>8, destmem & 0xff);
			}
#endif
			
			curx = 0; cury = 21;
			
#ifdef __C64__
			cputsxy(curx, cury, "                                        ");
#endif

#ifdef __C128__
			cputsxy(curx, cury, "                                                                                ");
#endif

			gotoxy(curx,cury);
			
			newlineflg = 0;
		}
		
		if(buf[x] != '\r' && buf[x] != '\n')
		{
			if(buf[x] == 0x02 || buf[x] == 0x1D || buf[x] == 0x1F)
			{
				// eat bold, italic and underline text
				continue;
			}
			else if(buf[x] == 0x16)
			{
				if(rvs_vid == 0)
				{
					printf("%c", 0x12);
					rvs_vid = 1;
				}
				else
				{
					printf("%c", 0x92);
					rvs_vid = 0;
				}
			}
			else if(buf[x] == 0x03)
			{
				printf("%c", CG_COLOR_L_GREEN);
				if(buf[x+1] == '0') { printf("%c", 0x05); x++; }
				if(buf[x+1] == '1') { printf("%c", 0x90); x++; }
				if(buf[x+1] == '2') { printf("%c", 0x1F); x++; }
				if(buf[x+1] == '3') { printf("%c", 0x1E); x++; }
				if(buf[x+1] == '4') { printf("%c", 0x1C); x++; }
				if(buf[x+1] == '5') { printf("%c", 0x95); x++; }
				if(buf[x+1] == '6') { printf("%c", 0x9C); x++; }
				if(buf[x+1] == '7') { printf("%c", 0x81); x++; }
				if(buf[x+1] == '8') { printf("%c", 0x9E); x++; }
				if(buf[x+1] == '9') { printf("%c", 0x99); x++; }
				if(buf[x+1] == '1' && buf[x+2] == '0') { printf("%c", 0x9F); x+=2; }
				if(buf[x+1] == '1' && buf[x+2] == '1') { printf("%c", 0x9F); x+=2; }
				if(buf[x+1] == '1' && buf[x+2] == '2') { printf("%c", 0x9A); x+=2; }
				if(buf[x+1] == '1' && buf[x+2] == '3') { printf("%c", 0x96); x+=2; }
				if(buf[x+1] == '1' && buf[x+2] == '4') { printf("%c", 0x98); x+=2; }
				if(buf[x+1] == '1' && buf[x+2] == '5') { printf("%c", 0x9B); x+=2; }
			}
			else
			{
				cputcxy(curx,cury, convertchar(buf[x]));
				curx++;
			}

		}
	}
	
}

void irc_pong(char *buf)
{
    buf[0] = 'p';
	buf[1] = 'o';
	buf[2] = 'n';
	buf[3] = 'g';

	uii_socketwrite(socketnr, buf);
	uii_socketwrite(socketnr, "\r\n");
}

void irc_help()
{
	printf("%c", CG_COLOR_WHITE);
	irc_print("",1);
	irc_print("cOMMANDS",1);
	
	irc_print("/JOIN #CHANNEL - JOIN CHANNEL",1);
	irc_print("/PART          - LEAVE CURRENT CHANNEL",1);
	irc_print("/NICK          - CHANGE YOUR NICKNAME",1);
	irc_print("/ME ACTION     - PERFORM AN ACTION",1);
	irc_print("/QUIT          - QUIT PROGRAM",1);
	irc_print("/HELP          - THIS LIST",1);
	irc_print("",1);
	printf("%c", CG_COLOR_CYAN);
}

void irc_handleinput(char *buf)
{	
    char full_message[MAX_OUTBUFFER+80]; 
	unsigned char x = 0;
	unsigned char s = 0;

	if (strlen(buf) == 0)
		return;
	
	if(strstr(buf,"/join") == buf)
	{
		if(channel[0] != 0)
		{
			irc_print("** yOU ARE ALREADY IN A CHANNEL.",1);
			irc_print("** uSE /part TO LEAVE FIRST.",1);
			return;
		}
		
		strcpy(channel, &buf[5]);
		irc_updateheader(channel);
		
		sprintf(full_message, "join %s\n", channel);
		uii_socketwrite(socketnr, full_message);
	}
	else if(strstr(buf,"/part") == buf)
	{
		if(channel[0] == 0)
		{
			irc_print("** yOU ARE NOT IN A CHANNEL.",1);
			return;
		}
		
		irc_updateheader(nochan);
		
		sprintf(full_message, "part %s\n", channel); // Leave current channel
		uii_socketwrite(socketnr, full_message);
		channel[0] = 0;
	}
	else if(strstr(buf,"/quit") == buf)
	{
		uii_socketclose(socketnr);
		RESET_MACHINE
	}
	else if(strstr(buf,"/help") == buf)
	{
		irc_help();
	}
	else if(strstr(buf,"/nick ") == buf)
	{
		for(x=0;x<strlen(buf);x++)
			buf[x] = convertchar(buf[x]);
		
		strcpy(nick, &buf[6]);
		
		sprintf(full_message, "nick %s\n", nick);
		uii_socketwrite(socketnr, full_message);
	}
	else
	{
		if(channel[0] == 0)
		{
			irc_print("** yOU ARE NOT IN A CHANNEL.",1);
			return;
		}
		
		if(strstr(buf,"/me ") == buf)
		{
			buf = &buf[4];
			
			for(x=0;x<strlen(buf);x++)
				buf[x] = convertchar(buf[x]);

			sprintf(full_message, "privmsg %s :%caction %s%c\n", channel, 0x01, buf, 0x01);
			uii_socketwrite(socketnr, full_message);
			
			printf("%c", CG_COLOR_YELLOW);
			irc_print(" * ",1);
			irc_print(nick,0);
			irc_print(" ",0);
			irc_print(buf,0);
			printf("%c", CG_COLOR_L_GRAY);
		}
		else
		{
			for(x=0;x<strlen(buf);x++)
				buf[x] = convertchar(buf[x]);

			sprintf(full_message, "privmsg %s :%s\n", channel, buf); // PRIVMSG <channel> :Message text
			uii_socketwrite(socketnr, full_message);
			
			printf("%c", CG_COLOR_CYAN);
			irc_print("<",1);
			printf("%c", CG_COLOR_L_RED);
			irc_print(nick,0);
			printf("%c", CG_COLOR_CYAN);
			irc_print("> ",0);
			printf("%c", CG_COLOR_L_RED);
			irc_print(buf,0);
		}
	}
	
    return;
}

void getconfig(void)
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
	char* sender;
	char* tmpPtr;
	//unsigned char* tmpPtr2;

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
			datacount = uii_socketread(socketnr, 400);

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
								sender = (char*) malloc(40 * sizeof(char));
							
								i = 1;
								while(inbuf[i] != '!')
								{
									sender[i-1] = inbuf[i];
									i++;
								}
								sender[i-1] = 0;
								
								
								if (strstr(inbuf, "action ") != 0)
								{
									tmpPtr = (char*) malloc(80 * sizeof(char));
									strcpy(tmpPtr," * ");
									strcat(tmpPtr,sender);
									strcat(tmpPtr," ");
									strcat(tmpPtr, (strstr(inbuf, "action ")+7));
									
									i = 0;
									// strip the 0x01 
									while (tmpPtr[i] != 0)
									{
										if(tmpPtr[i] == '\x01')
										{
											tmpPtr[i] = ' ';
										}
										i++;
									}
									
									printf("%c", CG_COLOR_YELLOW);
									irc_print(tmpPtr,1);
									printf("%c", CG_COLOR_L_GRAY);									
									inbufptr = 0;
									newline = 0;
									free(tmpPtr);
									free(sender);
									continue;
								}
								else
								{							
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
							}
							else if (strstr(inbuf, " join ") != 0)
							{
								sender = (char*) malloc(80 * sizeof(char));
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
								sender = (char*) malloc(80 * sizeof(char));
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
							else if (strstr(inbuf, " quit ") != 0)
							{
								sender = (char*) malloc(80 * sizeof(char));
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
								irc_print(" HAS QUIT IRC.",0);
								printf("%c", CG_COLOR_L_GRAY);
								
								inbufptr = 0;
								free(sender); 
								continue;
							}
							else if (strstr(inbuf, " nick ") != 0)
							{
								sender = (char*) malloc(80 * sizeof(char));
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
								free(sender);
								
								irc_print(" IS NOW KNOWN AS ",0);
								
								sender = strstr(inbuf, " nick ") + 7;
								
								irc_print(sender,0);
								printf("%c", CG_COLOR_L_GRAY);
								
								inbufptr = 0;								
								continue;
							}
							else if (strstr(inbuf, " quit ") != 0)
							{
								sender = (char*) malloc(80 * sizeof(char));
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
								irc_print(" HAS QUIT IRC. ",0);
								
								irc_print(tmpPtr,0);
								
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
						outx = SCREEN_WIDTH-1;
						outy--;
					}
					else
					{
						outx--;
					}
					
					printf("%c", CG_COLOR_WHITE);
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
					
					#ifdef __C64__
					cputsxy(0,23,"                                        ");
					cputsxy(0,24,"                                        ");
					#endif
					
					#ifdef __C128__
					cputsxy(0,23,"                                                                                ");
					cputsxy(0,24,"                                                                                ");
					#endif
					
					outx = 0;
					outy = 23;
					cputcxy(outx, outy, CURSOR);
					gotoxy(tempx, tempy);
					outbufptr = 0;
					outbuf[outbufptr]=0;
				}
				else
				{
					if(outbufptr < MAX_OUTBUFFER)
					{
						tempx = wherex();
						tempy = wherey();
						
						printf("%c", CG_COLOR_WHITE);
						cputcxy(outx++, outy, c);

						if(outx > SCREEN_WIDTH-1)
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

#ifdef __C128__

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

#endif
