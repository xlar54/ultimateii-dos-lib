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
#include "ultimate_ii.h"

void main(void) 
{
	int count = 0;
	
	printf("Demo of the cc65 Ultimate II+\nDOS Library\n");
	
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

}