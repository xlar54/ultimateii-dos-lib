#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <c64.h>
#include <conio.h>
#include "../lib/ultimate_lib.h"

#define MAXINPUT 	160
#define MAXPARAM_SZ	20

char buf[MAXINPUT];

char params[4][MAXPARAM_SZ];
unsigned char paramcount = 0;
unsigned char running = 1;

void parsecmd()
{
	int t=0;
	int p=0;
	int q=0;
	int ctr = 0;
	
	
	for(t=0;t<4;t++)
		params[t][0] = 0;
	
	for(t=0; t<strlen(buf); t++)
	{
		if(buf[t]==' ' && q==0)
		{
			params[p][ctr] = 0;
			p++;
			ctr=0;
			
			if(p > 4)
				break;
		}
		else {
			
			if(buf[t]=='"' && q==0)
			{
				q = 1;
			}
			else if(buf[t]=='"' && q==1)
			{
				q = 0;
			}
			else if(ctr < MAXPARAM_SZ)
			{
				params[p][ctr++] = buf[t];
				params[p][ctr]=0;
			}
		}
	}
	
	for(t=0;t<4;t++)
		if(params[t][0] != 0)
			paramcount++;
	
	//for(t=0; t<4; t++)
	//	printf("\nparam %d: %s", t, params[t]);
}

void docmd()
{
	if(strcmp(params[0],"help") == 0)
	{
		printf("\n\nlist of available commands:");
		printf("\nexit             - return to basic");
		printf("\nid               - identify");
		printf("\npath             - get current path");
		printf("\ncd <directory>   - change directory");
		printf("\ndir              - read directory");
		printf("\nmount <d64 file> - to drive 8");
		printf("\nunmount          - what it says");
		printf("\nmore <file>      - read a file");
		printf("\ndriveoff         - disable drive 8");
		printf("\ndriveon          - enable drive 8");
		printf("\ndrivestat        - drive power status");
	}
	else if(strcmp(params[0],"exit") == 0)
	{
		running = 0;
	}
	else if(strcmp(params[0],"driveoff") == 0)
	{
		uii_disable_drive_a();
		printf("\nDrive 8 disabled.");
	}
	else if(strcmp(params[0],"driveon") == 0)
	{
		uii_enable_drive_a();
		printf("\nDrive 8 enabled.");
	}
	else if(strcmp(params[0],"drivestat") == 0)
	{
		uii_get_drive_a_power();
		printf("\nDrive A power:%s", uii_data);
	}
	else if(strcmp(params[0],"id") == 0)
	{
		uii_identify();
		printf("\n%s", uii_data);
	}
	else if(strcmp(params[0],"path") == 0)
	{
		uii_get_path();
		printf("\nCurrent path: %s", uii_data);
	}
	else if(strcmp(params[0],"cd") == 0)
	{
		if(paramcount == 2)
		{
			uii_change_dir(params[1]);
			printf("\n%s", uii_status);	
		}
	}
	else if(strcmp(params[0],"dir") == 0)
	{
		uii_open_dir();
		
		printf("\n\nFiles:");
		uii_get_dir();

		while(uii_isdataavailable())
		{
			uii_readdata();
			uii_accept();
			printf("\n%s", uii_data);
		}
	}
	else if(strcmp(params[0],"mount") == 0)
	{
		if(paramcount == 2)
		{
			uii_mount_disk(8,params[1]);
			printf("\n%s", uii_status);	
		}
	}
	else if(strcmp(params[0],"unmount") == 0)
	{
		if(paramcount == 1)
		{
			uii_unmount_disk(8);
			printf("\n%s", uii_status);	
		}
	}
	else if(strcmp(params[0],"more") == 0)
	{
		if(paramcount == 2)
		{
			uii_open_file(0x01, params[1]);
			printf("\nstatus = %s", uii_status);	
			
			uii_read_file(255);
			printf("\n%s", uii_status);	

			while(uii_isdataavailable())
			{
				uii_readdata();
				uii_accept();
				printf("\n%s", uii_data);
			}
			
			uii_close_file();
			
			printf("\n%s", uii_status);	
		}
	}
	else
	{
		printf("\n\nunknown command. try 'help'");
	}
	
	paramcount = 0;
}

void main(void)
{
	unsigned char c;
	int ctr = 0;

	printf("\n\nsimple u2+ dos shell\n");
	printf("\n>");
	
	buf[0] = 0;
	
	while(running)
	{		
		if(kbhit() != 0)
		{
			c = cgetc();
			
			if(c == 13)
			{
				buf[ctr] = 0;
				
				parsecmd();
				docmd();
				
				printf("\n\n>");
				ctr=0;
				buf[ctr]=0;
			}
			else if(c == 8)
			{
				if(ctr > 0)
				{
					ctr--;
					buf[ctr] = 0;
					printf("%c",c);
				}
			}
			else if(ctr < 160)
			{
				buf[ctr++] = c;
				printf("%c",c);
			}
		}
	}
}
