/*****************************************************************
Ultimate II+ TCP Network Demo Terminal
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
#include "ultimate_ii.h"

struct bbslist {
	char host[255];
	char port[20];
};

char host[255] = "";
char portbuff[20];
char inbuf[255];

#define LISTMAX 12
struct bbslist myList[LISTMAX] = {
	{"the-hidden.hopto.org", "64128"},
	{"rapidfire.hopto.org", "64128"},
	{"reflections.servebbs.com", "64128"},
	{"bbs.retrohack.se", "64128"},
	{"fastline.nu", "1541"},
	{"dzbbs.hopto.org", "64128"},
	{"antidote.triad.se", "64128"},
	{"raveolution.hopto.org", "64128"},
	{"ssbbs.hopto.org", "64128"},
	{"afterlife.dynu.com", "6400"},
	{"centronian.com", "6400"},
	{"hhbbs.servebeer.com", "6400"}
};

#define RVS_ON	0x12
#define RVS_OFF	0x92
#define CURSOR 	0xe4
#define LEFT	0x9d
#define DELETE	0x14

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



void main(void) 
{
	int count = 0;
	
	int port = 0;
	unsigned char socketnr = 0;
	int datacount = 0;
	unsigned char c = 0;
	char buff[2] = {0,0};

	char *ptr;
	long ret;

	int x = 0;
	unsigned char curs = 0;

	POKEW(0xD020,0);
	POKEW(0xD021,0);
	printf("%c%cUltimateTerm 64 v1.22%c", 0x05, 147, 0x9f);
		
	uii_settarget(TARGET_NETWORK);
	
	uii_identify();
	
	printf("\n\nNetwork interface : %s", uii_data);
	printf("\n           Status : %s", uii_status);
	
	//uii_getinterfacecount();  //needed?
	
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
	
	while (1)
	{
		x = strlen(host);
		while ( x == 0 ) {
			for (x = 0; x < LISTMAX; ++x) {
				printf("\n%2d %s:%s",x,myList[x].host,myList[x].port);
			}
			printf("\n c custom address\n");
			x = 0;

			getstring(inbuf);

			ret = strtol(inbuf, &ptr, 10);

			if ( ret == 0 ) {
				printf("\n\nHost Address: ");
				getstring(host);
		
				printf("\n        Port: ");
				getstring(portbuff);
		
				port = atoi(portbuff);

			} else if ( ret < LISTMAX ) {
				strcpy(host,myList[ret].host);
				port = atoi(myList[ret].port);
			}
			x = strlen(host);
		}
		x = 0;
		printf("\n\nConnecting to: %s:%u\n", host, port);
		uii_tcpconnect(host, port);
		socketnr = uii_data[0];
		
		printf("\n\n[F1] to disconnect----------------------\n");
		printf("%c",CURSOR);
		curs = 1;
		
		if (uii_status[0] == '0' && uii_status[1] == '0')
		{
			
			while(1)
			{
				uii_tcpsocketread(socketnr, 1024);
				datacount = uii_data[0] | (uii_data[1]<<8);

				if(datacount > -1)
				{
					printf("%c",LEFT);
					
					for(x=2;x<datacount+2;x++)
					{
						if(curs == 1 && 
							(uii_data[x] == 13 || uii_data[x] == 29 || uii_data[x] == 157 
								|| uii_data[x] == 17 || uii_data[x] == 145))
						{
							printf(" %c", LEFT);
							curs = 0;
						}

						printf("%c", uii_data[x]);	// data byte						
					}
					printf("%c",CURSOR);
					curs = 1;
				}

				c = kbhit();

				if(c != 0)
				{
					c = cgetc();
					if (c == 133)
					{
						printf("\n\nClosing connection");
						uii_tcpclose(socketnr);
						strcpy(host,"");
						break;
					}
					else
					{
						buff[0] = c;
						buff[1] = 0;
						uii_tcpsocketwrite(socketnr, buff);
					}
				}
			}
		}
		else
		{
			printf("\nConnect failed: %s", uii_status);
			strcpy(host,"");
		}
	}
}
