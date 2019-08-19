/*****************************************************************
Ultimate 64/II+ Command Library
Scott Hutter, Francesco Sblendorio

Based on ultimate_dos-1.2.docx and command interface.docx
https://github.com/markusC64/1541ultimate2/tree/master/doc

Disclaimer:  Because of the nature of DOS commands, use this code
soley at your own risk.

Patches and pull requests are welcome
******************************************************************/
#include <string.h>
#include "ultimate_lib.h"

static unsigned char *cmddatareg = (unsigned char *)CMD_DATA_REG;
static unsigned char *controlreg = (unsigned char *)CONTROL_REG;
static unsigned char *statusreg = (unsigned char *)STATUS_REG;
//static unsigned char *idreg = (unsigned char *)ID_REG;
static unsigned char *respdatareg = (unsigned char *)RESP_DATA_REG;
static unsigned char *statusdatareg = (unsigned char *)STATUS_DATA_REG;

unsigned char uii_status[STATUS_QUEUE_SZ];
unsigned char uii_data[DATA_QUEUE_SZ*2];
unsigned char temp_string_onechar[2];
int uii_data_index;
int uii_data_len;

unsigned char uii_target = TARGET_DOS1;

void uii_logtext(char *text)
{
#ifdef DEBUG
	printf("%s", text);
#else
	text = 0;  // to eliminate the warning in cc65
#endif
}

void uii_logstatusreg(void)
{
#ifdef DEBUG
	printf("\nstatus reg %p = %d",statusreg, *statusreg);
#endif
}

void uii_settarget(unsigned char id)
{
	uii_target = id;
}

void uii_freeze(void)
{
	unsigned char cmd[] = {0x00,0x05};
	
	uii_settarget(TARGET_CONTROL);
	
	uii_sendcommand(cmd, 2);
	uii_readdata();
	uii_readstatus();
	uii_accept();
	
}

void uii_identify(void)
{
	unsigned char cmd[] = {0x00,DOS_CMD_IDENTIFY};
	uii_settarget(TARGET_DOS1);
	uii_sendcommand(cmd, 2);
	uii_readdata();
	uii_readstatus();
	uii_accept();
}

void uii_get_path(void)
{
	unsigned char cmd[] = {0x00,DOS_CMD_GET_PATH};	
	uii_settarget(TARGET_DOS1);
	uii_sendcommand(cmd, 2);
	uii_readdata();
	uii_readstatus();
	uii_accept();
}

void uii_open_dir(void)
{
	unsigned char cmd[] = {0x00,DOS_CMD_OPEN_DIR};
	int len = 0;
	uii_settarget(TARGET_DOS1);
	uii_sendcommand(cmd, 2);
	uii_readstatus();
	uii_accept();
}

void uii_get_dir(void)
{
	unsigned char cmd[] = {0x00,DOS_CMD_READ_DIR};
	int count = 0;
	uii_settarget(TARGET_DOS1);
	uii_sendcommand(cmd, 2);
}

void uii_change_dir(char* directory)
{
	unsigned char cmd[] = {0x00,DOS_CMD_CHANGE_DIR};
	
	int len = 0;
	int x = 0;
	unsigned char* fullcmd;
	
	fullcmd = (unsigned char *)malloc(strlen(directory)+2);
	fullcmd[0] = cmd[0];
	fullcmd[1] = cmd[1];
	
	for(x=0;x<strlen(directory);x++)
		fullcmd[x+2] = directory[x];

	uii_settarget(TARGET_DOS1);
	uii_sendcommand(fullcmd, strlen(directory)+2);
	
	free(fullcmd);
	
	uii_readstatus();
	uii_accept();
}

void uii_create_dir(char *directory)
{
	unsigned char cmd[] = {0x00,DOS_CMD_CREATE_DIR};
	int len = 0;
	int x = 0;
	unsigned char* fullcmd;
	
	fullcmd = (unsigned char *)malloc(strlen(directory)+2);
	fullcmd[0] = cmd[0];
	fullcmd[1] = cmd[1];
	
	for(x=0;x<strlen(directory);x++)
		fullcmd[x+2] = directory[x];
	
	uii_settarget(TARGET_DOS1);
	uii_sendcommand(fullcmd, strlen(directory)+2);
	
	free(fullcmd);
	
	uii_readdata();
	uii_readstatus();
	uii_accept();
}

void uii_change_dir_home(void)
{
	unsigned char cmd[] = {0x00,DOS_CMD_COPY_HOME_PATH};
	int count = 0;
	
	uii_settarget(TARGET_DOS1);
	uii_sendcommand(cmd, 2);
	uii_readstatus();
	uii_accept();
}

void uii_mount_disk(unsigned char id, char *filename)
{
	unsigned char cmd[] = {0x00,DOS_CMD_MOUNT_DISK};
	int len = 0;
	int x = 0;
	unsigned char* fullcmd;
	
	fullcmd = (unsigned char *)malloc(strlen(filename)+3);
	fullcmd[0] = cmd[0];
	fullcmd[1] = cmd[1];
	fullcmd[2] = id;
	
	for(x=0;x<strlen(filename);x++)
		fullcmd[x+3] = filename[x];
	
	uii_settarget(TARGET_DOS1);
	uii_sendcommand(fullcmd, strlen(filename)+3);
	
	free(fullcmd);
	
	uii_readdata();
	uii_readstatus();
	uii_accept();
}

void uii_unmount_disk(unsigned char id)
{
	unsigned char cmd[] = {0x00,DOS_CMD_UMOUNT_DISK, 0x00};

	cmd[2] = id;
	
	uii_settarget(TARGET_DOS1);
	uii_sendcommand(cmd, 3);

	uii_readdata();
	uii_readstatus();
	uii_accept();
}

void uii_swap_disk(unsigned char id)
{
	unsigned char cmd[] = {0x00,DOS_CMD_SWAP_DISK, 0x00};	
	
	cmd[2] = id;
	
	uii_settarget(TARGET_DOS1);
	uii_sendcommand(cmd, 3);

	uii_readdata();
	uii_readstatus();
	uii_accept();
}

void uii_open_file(unsigned char attrib, char *filename)
{
	unsigned char cmd[] = {0x00,DOS_CMD_OPEN_FILE, 0x00};
	
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
	
	uii_settarget(TARGET_DOS1);
	uii_sendcommand(fullcmd, strlen(filename)+3);
	
	free(fullcmd);
	
	uii_readdata();
	uii_readstatus();
	uii_accept();
}

void uii_close_file(void)
{
	unsigned char cmd[] = {0x00,DOS_CMD_CLOSE_FILE};
	
	uii_settarget(TARGET_DOS1);
	uii_sendcommand(cmd, 2);
	
	uii_readdata();
	uii_readstatus();
	uii_accept();
}

void uii_read_file(unsigned char length)
{
	unsigned char cmd[] = {0x00,DOS_CMD_READ_DATA, 0x00, 0x00};
	
	cmd[2] = length & 0xFF;
	cmd[3] = length >> 8;
	
	uii_settarget(TARGET_DOS1);
	uii_sendcommand(cmd, 2);
	
	// As with _get_dir(), read this in a loop, and _accept() the data
	// in order to get the next packet
	//
	// each data packet is 512 bytes each
}

void uii_write_file(unsigned char* data, int length)
{
	unsigned char cmd[] = {0x00,DOS_CMD_WRITE_DATA, 0x00, 0x00};
	
	unsigned char *fullcmd;
	int x = 0;
	
	fullcmd = (unsigned char *)malloc(length+4);
	fullcmd[0] = cmd[0];
	fullcmd[1] = cmd[1];
	fullcmd[2] = cmd[2];
	fullcmd[3] = cmd[3];
	
	for(x=0;x<length;x++)
		fullcmd[x+4] = data[x];
	
	uii_settarget(TARGET_DOS1);
	uii_sendcommand(fullcmd, length+4);
	
	uii_readdata();
	uii_readstatus();
	uii_accept();
}

void uii_delete_file(char* filename)
{
	unsigned char cmd[] = {0x00,DOS_CMD_DELETE_FILE};
	
	int len = 0;
	int x = 0;
	unsigned char* fullcmd;
	
	fullcmd = (unsigned char *)malloc(strlen(filename)+2);
	fullcmd[0] = cmd[0];
	fullcmd[1] = cmd[1];
	
	for(x=0;x<strlen(filename);x++)
		fullcmd[x+2] = filename[x];
	
	uii_settarget(TARGET_DOS1);
	uii_sendcommand(fullcmd, strlen(filename)+2);
	
	free(fullcmd);
	
	uii_readstatus();
	uii_accept();
}

void uii_rename_file(char* filename, char* newname)
{
	unsigned char cmd[] = {0x00,DOS_CMD_RENAME_FILE};
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
	
	uii_settarget(TARGET_DOS1);
	uii_sendcommand(fullcmd, strlen(filename)+ 3 + strlen(newname));
	
	free(fullcmd);
	
	uii_readstatus();
	uii_accept();
}

void uii_copy_file(char* sourcefile, char* destfile)
{
	unsigned char cmd[] = {0x00,DOS_CMD_COPY_FILE};
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
	
	uii_settarget(TARGET_DOS1);
	uii_sendcommand(fullcmd, strlen(sourcefile)+ 3 + strlen(destfile));
	
	free(fullcmd);
	
	uii_readstatus();
	uii_accept();
}

void uii_echo(void)
{
	unsigned char cmd[] = {0x00,DOS_CMD_ECHO};
	uii_settarget(TARGET_DOS1);
	uii_sendcommand(cmd, 2);

	uii_readdata();
	uii_readstatus();
	uii_accept();
}

void uii_enable_drive_a(void)
{
	unsigned char cmd[] = {0x00,CTRL_CMD_ENABLE_DISK_A};

	uii_settarget(TARGET_CONTROL);
	uii_sendcommand(cmd, 2);

	uii_readdata();
	uii_readstatus();
	uii_accept();
}

void uii_disable_drive_a(void)
{
#define CTRL_CMD_DISABLE_DISK_A	0x31
	unsigned char cmd[] = {0x00,CTRL_CMD_DISABLE_DISK_A};

	uii_settarget(TARGET_CONTROL);
	uii_sendcommand(cmd, 2);

	uii_readdata();
	uii_readstatus();
	uii_accept();
}

void uii_enable_drive_b(void)
{
	unsigned char cmd[] = {0x00,CTRL_CMD_ENABLE_DISK_B};

	uii_settarget(TARGET_CONTROL);
	uii_sendcommand(cmd, 2);

	uii_readdata();
	uii_readstatus();
	uii_accept();
}

void uii_disable_drive_b(void)
{
	unsigned char cmd[] = {0x00,CTRL_CMD_DISABLE_DISK_B};

	uii_settarget(TARGET_CONTROL);
	uii_sendcommand(cmd, 2);

	uii_readdata();
	uii_readstatus();
	uii_accept();
}

void uii_get_drive_a_power(void) 
{
	unsigned char cmd[] = {0x00,CTRL_CMD_DRIVE_A_POWER};

	uii_settarget(TARGET_CONTROL);
	uii_sendcommand(cmd, 2);

	uii_readdata();
	uii_readstatus();
	uii_accept();
}

void uii_get_drive_b_power(void) 
{
	unsigned char cmd[] = {0x00,CTRL_CMD_DRIVE_B_POWER};
	
	uii_settarget(TARGET_CONTROL);
	uii_sendcommand(cmd, 2);

	uii_readdata();
	uii_readstatus();
	uii_accept();
}

void uii_sendcommand(unsigned char *bytes, int count)
{
	int x =0;
	int success = 0;
	
	bytes[0] = uii_target;
	
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

void uii_getinterfacecount(void)
{
	unsigned char tempTarget = uii_target;
	unsigned char cmd[] = {0x00,NET_CMD_GET_INTERFACE_COUNT};
	
	uii_settarget(TARGET_NETWORK);
	uii_sendcommand(cmd, 0x02);

	uii_readdata();
	uii_readstatus();
	uii_accept();
	
	uii_target = tempTarget;
}

void uii_getipaddress(void)
{
	unsigned char tempTarget = uii_target;
	unsigned char cmd[] = {0x00,NET_CMD_GET_IP_ADDRESS, 0x00}; // interface 0 (theres only one)
	
	uii_settarget(TARGET_NETWORK);
	uii_sendcommand(cmd, 0x03);

	uii_readdata();
	uii_readstatus();
	uii_accept();
	
	uii_target = tempTarget;
}

unsigned char uii_tcpconnect(char* host, unsigned short port)
{
	unsigned char tempTarget = uii_target;
	unsigned char cmd[] = {0x00,NET_CMD_TCP_SOCKET_CONNECT, 0x00, 0x00};
	int x=0;
	unsigned char* fullcmd;
	
	fullcmd = (unsigned char *)malloc(4 + strlen(host)+ 1);
	fullcmd[0] = cmd[0];
	fullcmd[1] = cmd[1];
	fullcmd[2] = port & 0xff;
	fullcmd[3] = (port>>8) & 0xff;
	
	for(x=0;x<strlen(host);x++)
		fullcmd[x+4] = host[x];
	
	fullcmd[4+strlen(host)] = 0x00;
	
	uii_settarget(TARGET_NETWORK);
	uii_sendcommand(fullcmd, 4+strlen(host)+1);

	uii_readdata();
	uii_readstatus();
	uii_accept();
	
	uii_target = tempTarget;

	uii_data_index = 0;
	uii_data_len = 0;
	return uii_data[0];
}

void uii_tcpclose(unsigned char socketid)
{
	unsigned char tempTarget = uii_target;
	unsigned char cmd[] = {0x00,NET_CMD_TCP_SOCKET_CLOSE, 0x00};
	cmd[2] = socketid;
	
	uii_settarget(TARGET_NETWORK);
	uii_sendcommand(cmd, 0x03);

	uii_readdata();
	uii_readstatus();
	uii_accept();
	
	uii_target = tempTarget;
}

int uii_tcpsocketread(unsigned char socketid, unsigned short length)
{
	unsigned char tempTarget = uii_target;
	unsigned char cmd[] = {0x00,NET_CMD_TCP_SOCKET_READ, 0x00, 0x00, 0x00};

	cmd[0] = cmd[0];
	cmd[1] = cmd[1];
	cmd[2] = socketid;
	cmd[3] = length & 0xff;
	cmd[4] = (length>>8) & 0xff;
	
	uii_settarget(TARGET_NETWORK);
	uii_sendcommand(cmd, 0x05);

	uii_readdata();
	uii_readstatus();
	uii_accept();
	
	uii_target = tempTarget;
	return uii_data[0] | (uii_data[1]<<8);
}

int uii_tcplistenstart(unsigned short port)
{
	unsigned char tempTarget = uii_target;
	unsigned char cmd[] = {0x00,NET_CMD_TCP_LISTENER_START, 0x00, 0x00};
	cmd[2] = port & 0xff;
	cmd[3] = (port>>8) & 0xff;
	
	uii_settarget(TARGET_NETWORK);
	uii_sendcommand(cmd, 0x04);

	uii_readdata();
	uii_readstatus();
	uii_accept();
	
	uii_target = tempTarget;
	return uii_data[0] | (uii_data[1]<<8);
}

int uii_tcplistenstop()
{
	unsigned char tempTarget = uii_target;
	unsigned char cmd[] = {0x00,NET_CMD_TCP_LISTENER_STOP};
	
	uii_settarget(TARGET_NETWORK);
	uii_sendcommand(cmd, 0x02);

	uii_readdata();
	uii_readstatus();
	uii_accept();
	
	uii_target = tempTarget;
	return uii_data[0] | (uii_data[1]<<8);
}

int uii_tcpgetlistenstate()
{
	unsigned char tempTarget = uii_target;
	unsigned char cmd[] = {0x00,NET_CMD_GET_LISTENER_STATE};
	
	uii_settarget(TARGET_NETWORK);
	uii_sendcommand(cmd, 0x02);

	uii_readdata();
	uii_readstatus();
	uii_accept();
	
	uii_target = tempTarget;
	return uii_data[0] | (uii_data[1]<<8);
}

unsigned char uii_tcpgetlistensocket()
{
	unsigned char tempTarget = uii_target;
	unsigned char cmd[] = {0x00,NET_CMD_GET_LISTENER_SOCKET};
	
	uii_settarget(TARGET_NETWORK);
	uii_sendcommand(cmd, 0x02);

	uii_readdata();
	uii_readstatus();
	uii_accept();
	
	uii_target = tempTarget;
	return uii_data[0] | (uii_data[1]<<8);
}

void uii_tcpsocketwrite_convert_parameter(unsigned char socketid, char *data, int ascii)
{
	unsigned char tempTarget = uii_target;
	unsigned char cmd[] = {0x00,NET_CMD_TCP_SOCKET_WRITE, 0x00};
	int x=0;
	unsigned char* fullcmd;
	char c;
	
	fullcmd = (unsigned char *)malloc(3 + strlen(data));
	fullcmd[0] = cmd[0];
	fullcmd[1] = cmd[1];
	fullcmd[2] = socketid;
	
	for(x=0;x<strlen(data);x++){
		c = data[x];
		if (ascii) {
			if ((c>=97 && c<=122) || (c>=193 && c<=218)) c &= 95;
            else if (c>=65 && c<=90) c |= 32;
            else if (c==13) c=10;
		}
		fullcmd[x+3] = c;
	}
	
	fullcmd[3+strlen(data)+1] = 0;
	
	uii_settarget(TARGET_NETWORK);
	uii_sendcommand(fullcmd, 3+strlen(data));

	uii_readdata();
	uii_readstatus();
	uii_accept();

	uii_target = tempTarget;
	
	uii_data_index = 0;
	uii_data_len = 0;
}

void uii_tcpsocketwritechar(unsigned char socketid, char one_char) {
	temp_string_onechar[0] = one_char;
	temp_string_onechar[1] = 0;

	uii_tcpsocketwrite(socketid, temp_string_onechar);
}

void uii_tcpsocketwrite(unsigned char socketid, char *data) {
	uii_tcpsocketwrite_convert_parameter(socketid, data, 0);
}

void uii_tcpsocketwrite_ascii(unsigned char socketid, char *data) {
	uii_tcpsocketwrite_convert_parameter(socketid, data, 1);
}

char uii_tcp_nextchar(unsigned char socketid) {
    char result;
    if (uii_data_index < uii_data_len) {
        result = uii_data[uii_data_index+2];
        uii_data_index++;
    } else {
        do {
            uii_data_len = uii_tcpsocketread(socketid, DATA_QUEUE_SZ-4);
            if (uii_data_len == 0) return 0; // EOF
        } while (uii_data_len == -1);
        result = uii_data[2];
        uii_data_index = 1;
    }
    return result;
}

int uii_tcp_nextline_convert_parameter(unsigned char socketid, char *result, int swapCase) {
    int c, count = 0;
    *result = 0;
    while ((c = uii_tcp_nextchar(socketid)) != 0 && c != 0x0A) {
    	if (c == 0x0D){
    		continue;
    	} else if (swapCase) {
            if ((c>=97 && c<=122) || (c>=193 && c<=218)) c &= 95;
            else if (c>=65 && c<=90) c |= 32;
        }
        result[count++] = c;
    }
    result[count] = 0;
    return c != 0 || count > 0;
}

int uii_tcp_nextline(unsigned char socketid, char *result) {
	return uii_tcp_nextline_convert_parameter(socketid, result, 0);
}

int uii_tcp_nextline_ascii(unsigned char socketid, char *result) {
	return uii_tcp_nextline_convert_parameter(socketid, result, 1);
}

void uii_reset_uiidata() {
	uii_data_len = 0;
	uii_data_index = 0;
	memset(uii_data, 0, DATA_QUEUE_SZ*2);
	memset(uii_status, 0, STATUS_QUEUE_SZ);
}

void uii_tcp_emptybuffer() {
	uii_data_index = 0;
}
