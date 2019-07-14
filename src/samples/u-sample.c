/*****************************************************************
Ultimate II+ DOS Command Library
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
#include <c64.h>
#include "../lib/ultimate_lib.h"

void main(void) 
{
	int count = 0;
	char *host = "192.168.100.50";
	unsigned char socketnr = 0;
	int datacount = 0;
	
	printf("Demo of the cc65 Ultimate II+\nDOS Library\n");
	
	// Set the target to DOS1.  Its the default, so its ok if you dont.
	uii_settarget(TARGET_DOS1);
	
	if(uii_isdataavailable())
	{
		printf("\naborting a previous command...");
		uii_abort();
	}
	
	uii_identify();
	printf("\n\nIdentify: %s\nStatus: %s", uii_data, uii_status);
	
	uii_change_dir("/usb0");
	printf("\n\nDir changed\nStatus: %s", uii_status);	
	
	uii_get_path();
	printf("\n\nPath: %s\nStatus: %s", uii_data, uii_status);
	
	uii_open_dir();
	printf("\n\nDir Opened\nStatus: %s", uii_status);
		
	printf("\n\nFiles:");
	uii_get_dir();

	while(uii_isdataavailable())
	{
		uii_readdata();
		uii_accept();
		printf("\n%s", uii_data);
	}
	
	// -----------------------------------------------------------
	// Network interface target
	// -----------------------------------------------------------
	uii_settarget(TARGET_NETWORK);
	
	uii_identify();
	printf("\n\nIdentify: %s\nStatus: %s", uii_data, uii_status);
	
	uii_getinterfacecount();
	printf("\n\nInterface count: %d\nStatus: %s", atoi(uii_data), uii_status);
	
	uii_getipaddress();
	printf("\n\nIP Address: %d.%d.%d.%d", uii_data[0], uii_data[1], uii_data[2], uii_data[3]);
	printf("\n   Netmask: %d.%d.%d.%d", uii_data[4], uii_data[5], uii_data[6], uii_data[7]);
	printf("\n   Gateway: %d.%d.%d.%d", uii_data[8], uii_data[9], uii_data[10], uii_data[11]);
	printf("\n    Status: %s", uii_status);
	
	printf("\n\nConnecting to: %s", host);
	socketnr = uii_tcpconnect(host, 23);
	printf("\n    Status: %s  (Socket #%d)", uii_status, socketnr);
	
	printf("\n\nReading data...\n");
	
	while(uii_success())
	{
		datacount = uii_tcpsocketread(socketnr, 1);
		printf("%c", uii_data[2]);	// data byte
	}
	printf("\n    Status: %s", uii_status);
	
	
	printf("\n\nWriting data...\n");
	uii_tcpsocketwrite(socketnr, "hello ultimate 64 / uii plus!");
	printf("\n    Status: %s", uii_status);
	
	printf("\n\nClosing connection");
	uii_tcpclose(socketnr);
	printf("\n    Status: %s", uii_status);
	
	// -----------------------------------------------------------
	// Control target
	// -----------------------------------------------------------
	// no need to reset the target - the uii_freeze does it automatically. Just be sure to set it back
	// for more dos commands to TARGET_DOS1 or TARGET_DOS2
	uii_freeze();
	printf("\n\nGoing into Ultimate menu: %s", uii_status);
}