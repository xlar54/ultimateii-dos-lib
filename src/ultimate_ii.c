/*****************************************************************
Ultimate II+ DOS Command Library
Scott Hutter

Based on ultimate_dos-1.1.docx and command interface.docx
https://github.com/markusC64/1541ultimate2/tree/master/doc

Disclaimer:  Because of the nature of DOS commands, use this code
soley at your own risk.

Patches and pull requests are welcome
******************************************************************/
#include <string.h>
#include "ultimate_ii.h"

static unsigned char *cmddatareg = (unsigned char *)CMD_DATA_REG;
static unsigned char *controlreg = (unsigned char *)CONTROL_REG;
static unsigned char *statusreg = (unsigned char *)STATUS_REG;
static unsigned char *idreg = (unsigned char *)ID_REG;
static unsigned char *respdatareg = (unsigned char *)RESP_DATA_REG;
static unsigned char *statusdatareg = (unsigned char *)STATUS_DATA_REG;

unsigned char uii_status[STATUS_QUEUE_SZ];
unsigned char uii_data[DATA_QUEUE_SZ*2];

void uii_logtext(char *text)
{
#ifdef DEBUG
	printf("%s", text);
#endif
}

void uii_logstatusreg(void)
{
#ifdef DEBUG
	printf("\nstatus reg %p = %d",statusreg, *statusreg);
#endif
}

void uii_identify(void)
{
	unsigned char cmd[] = {0x01,0x01};
	
	uii_sendcommand(cmd, 2);
	uii_readdata();
	uii_readstatus();
	uii_accept();
}

void uii_get_path(void)
{
	unsigned char cmd[] = {0x01,0x12};	
	
	uii_sendcommand(cmd, 2);
	uii_readdata();
	uii_readstatus();
	uii_accept();
}

void uii_open_dir(void)
{
	unsigned char cmd[] = {0x01,0x13};
	int len = 0;

	uii_sendcommand(cmd, 2);
	uii_readstatus();
	uii_accept();
}

void uii_get_dir(void)
{
	unsigned char cmd[] = {0x01,0x14};
	int count = 0;
	
	uii_sendcommand(cmd, 2);
}

void uii_change_dir(char* directory)
{
	unsigned char cmd[] = {0x01,0x11};
	int len = 0;
	int x = 0;
	unsigned char* fullcmd;
	
	fullcmd = (unsigned char *)malloc(strlen(directory)+2);
	fullcmd[0] = cmd[0];
	fullcmd[1] = cmd[1];
	
	for(x=0;x<strlen(directory);x++)
		fullcmd[x+2] = directory[x];
	
	uii_sendcommand(fullcmd, strlen(directory)+2);
	
	free(fullcmd);
	
	uii_readstatus();
	uii_accept();
}

void uii_create_dir(char *directory)
{
	unsigned char cmd[] = {0x01,0x16};	
	int len = 0;
	int x = 0;
	unsigned char* fullcmd;
	
	fullcmd = (unsigned char *)malloc(strlen(directory)+2);
	fullcmd[0] = cmd[0];
	fullcmd[1] = cmd[1];
	
	for(x=0;x<strlen(directory);x++)
		fullcmd[x+2] = directory[x];
	
	uii_sendcommand(fullcmd, strlen(directory)+2);
	
	free(fullcmd);
	
	uii_readdata();
	uii_readstatus();
	uii_accept();
}

void uii_change_dir_home(void)
{
	unsigned char cmd[] = {0x01,0x17};
	int count = 0;
	
	uii_sendcommand(cmd, 2);
	uii_readstatus();
	uii_accept();
}

void uii_mount_disk(unsigned char id, char *filename)
{
	unsigned char cmd[] = {0x01,0x23};	
	int len = 0;
	int x = 0;
	unsigned char* fullcmd;
	
	fullcmd = (unsigned char *)malloc(strlen(filename)+3);
	fullcmd[0] = cmd[0];
	fullcmd[1] = cmd[1];
	fullcmd[2] = id;
	
	for(x=0;x<strlen(filename);x++)
		fullcmd[x+3] = filename[x];
	
	uii_sendcommand(fullcmd, strlen(filename)+3);
	
	free(fullcmd);
	
	uii_readdata();
	uii_readstatus();
	uii_accept();
}

void uii_unmount_disk(unsigned char id)
{
	unsigned char cmd[] = {0x01,0x24, 0x00};	

	cmd[2] = id;
	
	uii_sendcommand(cmd, 3);

	uii_readdata();
	uii_readstatus();
	uii_accept();
}

void uii_swap_disk(unsigned char id)
{
	unsigned char cmd[] = {0x01,0x25, 0x00};	

	cmd[2] = id;
	
	uii_sendcommand(cmd, 3);

	uii_readdata();
	uii_readstatus();
	uii_accept();
}

void uii_open_file(unsigned char attrib, char *filename)
{
	unsigned char cmd[] = {0x01,0x23, 0x00};	
	int len = 0;
	int x = 0;
	unsigned char* fullcmd;
	
	// Attrib will be:
	// 0x01 = Read
	// 0x02 = Write
	// 0x06 = Create new file
	// 0x0E = Create (overwriting an existing file)
	
	fullcmd = (unsigned char *)malloc(strlen(filename)+3);
	fullcmd[0] = cmd[0];
	fullcmd[1] = cmd[1];
	fullcmd[2] = attrib;
	
	for(x=0;x<strlen(filename);x++)
		fullcmd[x+3] = filename[x];
	
	uii_sendcommand(fullcmd, strlen(filename)+3);
	
	free(fullcmd);
	
	uii_readdata();
	uii_readstatus();
	uii_accept();
}

void uii_close_file(void)
{
	unsigned char cmd[] = {0x01,0x03};
	
	uii_sendcommand(cmd, 2);
	
	uii_readdata();
	uii_readstatus();
	uii_accept();
}

void uii_read_file(unsigned char length)
{
	unsigned char cmd[] = {0x01,0x04, 0x00, 0x00};
	
	cmd[2] = length & 0xFF;
	cmd[3] = length >> 8;
	
	uii_sendcommand(cmd, 2);
	
	// As with _get_dir(), read this in a loop, and _accept() the data
	// in order to get the next packet
	//
	// each data packet is 512 bytes each
}

void uii_write_file(unsigned char* data, int length)
{
	unsigned char cmd[] = {0x01,0x04, 0x00, 0x00};
	unsigned char *fullcmd;
	int x = 0;
	
	fullcmd = (unsigned char *)malloc(length+4);
	fullcmd[0] = cmd[0];
	fullcmd[1] = cmd[1];
	fullcmd[2] = cmd[2];
	fullcmd[3] = cmd[3];
	
	for(x=0;x<length;x++)
		fullcmd[x+4] = data[x];
	
	uii_sendcommand(fullcmd, length+4);
	
	uii_readdata();
	uii_readstatus();
	uii_accept();
}

void uii_delete_file(char* filename)
{
	unsigned char cmd[] = {0x01,0x09};
	int len = 0;
	int x = 0;
	unsigned char* fullcmd;
	
	fullcmd = (unsigned char *)malloc(strlen(filename)+2);
	fullcmd[0] = cmd[0];
	fullcmd[1] = cmd[1];
	
	for(x=0;x<strlen(filename);x++)
		fullcmd[x+2] = filename[x];
	
	uii_sendcommand(fullcmd, strlen(filename)+2);
	
	free(fullcmd);
	
	uii_readstatus();
	uii_accept();
}

void uii_rename_file(char* filename, char* newname)
{
	unsigned char cmd[] = {0x01,0x0a};
	int len = 0;
	int x = 0;
	int y = 0;
	unsigned char* fullcmd;
	
	fullcmd = (unsigned char *)malloc(strlen(filename)+ 3 + strlen(newname));
	fullcmd[0] = cmd[0];
	fullcmd[1] = cmd[1];
	
	for(x=0;x<strlen(filename);x++)
		fullcmd[x+2] = filename[x];
	
	fullcmd[x++] = 0x00;
	
	for(;x<strlen(newname);x++)
		fullcmd[x] = newname[y++];
	
	uii_sendcommand(fullcmd, strlen(filename)+ 3 + strlen(newname));
	
	free(fullcmd);
	
	uii_readstatus();
	uii_accept();
}

void uii_copy_file(char* sourcefile, char* destfile)
{
	unsigned char cmd[] = {0x01,0x0b};
	int len = 0;
	int x = 0;
	int y = 0;
	unsigned char* fullcmd;
	
	fullcmd = (unsigned char *)malloc(strlen(sourcefile)+ 3 + strlen(destfile));
	fullcmd[0] = cmd[0];
	fullcmd[1] = cmd[1];
	
	for(x=0;x<strlen(sourcefile);x++)
		fullcmd[x+2] = sourcefile[x];
	
	fullcmd[x++] = 0x00;
	
	for(;x<strlen(destfile);x++)
		fullcmd[x] = destfile[y++];
	
	uii_sendcommand(fullcmd, strlen(sourcefile)+ 3 + strlen(destfile));
	
	free(fullcmd);
	
	uii_readstatus();
	uii_accept();
}

void uii_echo(void)
{
	unsigned char cmd[] = {0x01,0xf0};	

	uii_sendcommand(cmd, 2);

	uii_readdata();
	uii_readstatus();
	uii_accept();
}

void uii_sendcommand(unsigned char *bytes, int count)
{
	int x =0;
	int success = 0;
	
	while(success == 0)
	{
		// Wait for idle state
		uii_logtext("\nwaiting for cmd_busy to clear...");
		uii_logstatusreg();
		
		while ( !(((*statusreg & 32) == 0) && ((*statusreg & 16) == 0)))  {
			uii_logtext("\nwaiting...");
			uii_logstatusreg();
		};
		
		// Write byte by byte to data register
		uii_logtext("\nwriting command...");
		while(x<count)
			*cmddatareg = bytes[x++];
		
		// Send PUSH_CMD
		uii_logtext("\npushing command...");
		*controlreg |= 0x01;
		
		uii_logstatusreg();
		
		// check ERROR bit.  If set, clear it via ctrl reg, and try again
		if ((*statusreg & 4) == 4)
		{
			uii_logtext("\nerror was set. trying again");
			*controlreg |= 0x08;
		}
		else
		{
			uii_logstatusreg();
			
			// check for cmd busy
			while ( ((*statusreg & 32) == 0) && ((*statusreg & 16) == 16) )
			{
				uii_logtext("\nstate is busy");
			}
			success = 1;
		}
	}
	
	uii_logstatusreg();
	uii_logtext("\ncommand sent");
	
}

void uii_accept(void)
{
	// Acknowledge the data
	uii_logstatusreg();
	uii_logtext("\nsending ack");
	*controlreg |= 0x02;
}

int uii_isdataavailable(void)
{
	if ( ((*statusreg & 128) == 128 ) )
		return 1;
	else
		return 0;
}

int uii_isstatusdataavailable(void)
{
	if ( ((*statusreg & 64) == 64 ) )
		return 1;
	else
		return 0;
}

void uii_abort(void)
{
	// abort the command
	uii_logstatusreg();
	uii_logtext("\nsending abort");
	*controlreg |= 0x04;
}
int uii_readdata(void) 
{
	int count = 0;
	int z = 0;
	
	uii_data[0] = 0;
	
	uii_logtext("\n\nreading data...");
	uii_logstatusreg();

	// If there is data to read
	while (uii_isdataavailable())
	{
		uii_data[z] = *respdatareg;
		z++;
		count++;		
	}
	uii_data[z] = 0;
	return count;
}

int uii_readstatus(void) 
{
	int count = 0;
	int z = 0;
	
	uii_status[0] = 0;
	
	uii_logtext("\n\nreading status...");
	uii_logstatusreg();

	while(uii_isstatusdataavailable())
	{
		uii_status[z++] = *statusdatareg;
		count++;
	}
	
	uii_status[z] = 0;
	return count;
}


