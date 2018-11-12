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
#ifdef __C128__
#include <c128.h>
#endif

#ifdef __C64__
#include <c64.h>
#endif

#include <conio.h>
#include <peekpoke.h>
#include "ultimate_ii.h"

char host[255];
char portbuff[20];
char inbuf[255];

int getstring(char *buf)
{
	unsigned char c = 0;
	unsigned char x = 0;
	
	while(1)
	{
		c = kbhit();
		
		if(c != 0)
		{
			c = cgetc();
			if (c == 13)
			{
				buf[x] = 0;
				return x;
			}
			else
			{
				buf[x++] = c;
				printf("%c",c);
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
	int x = 0;
	
	
	POKEW(0xD020,0);
	POKEW(0xD021,0);
	printf("%cUltimateTerm 64 v1.2", 147);
	
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
		printf("\n\nUnable to access network interface.  Please ensure the Command Interface is enabled and your network link is connected and in 'Link Up' state.");
		return;
	}
	
	while (1)
	{
		printf("\n\nHost Address: ");
		getstring(host);
		
		printf("\n        Port: ");
		getstring(portbuff);
		
		port = atoi(portbuff);
		
		printf("\n\nConnecting to: %s:%u\n", host, port);
		uii_tcpconnect(host, port);
		socketnr = uii_data[0];
		
		printf("\n\n[F1] to disconnect----------------------");
		if (uii_status[0] == '0' && uii_status[1] == '0')
		{
			while(1)
			{
				uii_tcpsocketread(socketnr, 1024);
				datacount = uii_data[0] | (uii_data[1]<<8);

				if(datacount > -1)
				{
					for(x=2;x<datacount+2;x++)
					{
						printf("%c", uii_data[x]);	// data byte
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
		}
	}
}