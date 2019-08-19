/*****************************************************************
Ultimate 64/II+ Command Library
Scott Hutter, Francesco Sblendorio

Based on ultimate_dos-1.2.docx and command interface.docx
https://github.com/markusC64/1541ultimate2/tree/master/doc

Disclaimer:  Because of the nature of DOS commands, use this code
soley at your own risk.

Patches and pull requests are welcome
******************************************************************/

#ifndef _ULTIMATE_LIB_DOS_H_
#define _ULTIMATE_LIB_DOS_H_

#include <stdlib.h>
#include <stdio.h>

								// bit 7    bit 6    bit 5    bit 4    bit 3    bit 2    bit 1    bit 0
#define CONTROL_REG		0xDF1C	// --------reserved------ ---------   CLR_ERR   ABORT   DATA_ACC  PUSH_CMD
#define STATUS_REG		0xDF1C  //DATA_AV   STAT_AV [    STATE    ]    ERROR    ABORT_P DATA_ACC  CMD_BUSY
#define CMD_DATA_REG	0xDF1D
#define ID_REG			0xDF1D
#define RESP_DATA_REG   0xDF1E
#define STATUS_DATA_REG	0xDF1F

#define DATA_QUEUE_SZ		896
#define STATUS_QUEUE_SZ		256

#define TARGET_DOS1		0x01
#define TARGET_DOS2		0x02
#define TARGET_NETWORK	0x03
#define TARGET_CONTROL	0x04

#define DOS_CMD_IDENTIFY		0x01
#define DOS_CMD_OPEN_FILE		0x02
#define DOS_CMD_CLOSE_FILE		0x03
#define DOS_CMD_READ_DATA		0x04
#define DOS_CMD_WRITE_DATA		0x05
#define DOS_CMD_FILE_SEEK		0x06
#define DOS_CMD_FILE_INFO		0x07
#define DOS_CMD_FILE_STAT		0x08
#define DOS_CMD_DELETE_FILE		0x09
#define DOS_CMD_RENAME_FILE		0x0a
#define DOS_CMD_COPY_FILE		0x0b
#define DOS_CMD_CHANGE_DIR		0x11
#define DOS_CMD_GET_PATH		0x12
#define DOS_CMD_OPEN_DIR		0x13
#define DOS_CMD_READ_DIR		0x14
#define DOS_CMD_COPY_UI_PATH	0x15
#define DOS_CMD_CREATE_DIR		0x16
#define DOS_CMD_COPY_HOME_PATH	0x17
#define DOS_CMD_LOAD_REU		0x21
#define DOS_CMD_SAVE_REU		0x22
#define DOS_CMD_MOUNT_DISK		0x23
#define DOS_CMD_UMOUNT_DISK		0x24
#define DOS_CMD_SWAP_DISK		0x25
#define DOS_CMD_GET_TIME		0x26
#define DOS_CMD_SET_TIME		0x27
#define CTRL_CMD_ENABLE_DISK_A	0x30
#define CTRL_CMD_DISABLE_DISK_A	0x31
#define CTRL_CMD_ENABLE_DISK_B	0x32
#define CTRL_CMD_DISABLE_DISK_B	0x33
#define CTRL_CMD_DRIVE_A_POWER 	0x34
#define CTRL_CMD_DRIVE_B_POWER	0x35
#define DOS_CMD_ECHO			0xf0

#define NET_CMD_GET_INTERFACE_COUNT	0x02
#define NET_CMD_GET_IP_ADDRESS		0x05
#define NET_CMD_TCP_SOCKET_CONNECT	0x07
#define NET_CMD_TCP_SOCKET_CLOSE	0x09
#define NET_CMD_TCP_SOCKET_READ		0x10
#define NET_CMD_TCP_SOCKET_WRITE	0x11
#define NET_CMD_TCP_LISTENER_START	0x12
#define NET_CMD_TCP_LISTENER_STOP	0x13
#define NET_CMD_GET_LISTENER_STATE	0x14
#define NET_CMD_GET_LISTENER_SOCKET	0x15

#define NET_LISTENER_STATE_NOT_LISTENEING	0x00
#define NET_LISTENER_STATE_LISTENING		0x01
#define NET_LISTENER_STATE_CONNECTED		0x02
#define NET_LISTENER_STATE_BIND_ERROR		0x03
#define NET_LISTENER_STATE_PORT_IN_USE		0x04

//#define DEBUG
#define DISPLAY_READ

#define uii_success() (uii_status[0] == '0' && uii_status[1] == '0')

extern unsigned char uii_status[STATUS_QUEUE_SZ];
extern unsigned char uii_data[DATA_QUEUE_SZ*2];

struct DirectoryEntry {
	unsigned char attributes;
	unsigned char *filename;
};


// prototypes
void uii_settarget(unsigned char id);
void uii_freeze(void);
void uii_identify(void);
void uii_get_path(void);
void uii_open_dir(void);
void uii_get_dir(void);
void uii_change_dir(char* directory);
void uii_create_dir(char *directory);
void uii_change_dir_home(void);
void uii_mount_disk(unsigned char id, char *filename);
void uii_unmount_disk(unsigned char id);
void uii_swap_disk(unsigned char id);
void uii_open_file(unsigned char attrib, char *filename);
void uii_close_file(void);
void uii_read_file(unsigned char length);
void uii_write_file(unsigned char* data, int length);
void uii_delete_file(char* filename);
void uii_rename_file(char* filename, char* newname);
void uii_copy_file(char* sourcefile, char* destfile);
void uii_echo(void);

void uii_getinterfacecount(void);
void uii_getipaddress(void);
unsigned char uii_tcpconnect(char* host, unsigned short port);
void uii_tcpclose(unsigned char socketid);
int uii_tcpsocketread(unsigned char socketid, unsigned short length);
void uii_tcpsocketwrite(unsigned char socketid, char *data);
void uii_tcpsocketwritechar(unsigned char socketid, char one_char);
void uii_tcpsocketwrite_ascii(unsigned char socketid, char *data);

int uii_tcplistenstart(unsigned short port);
int uii_tcplistenstop(void);
int uii_tcpgetlistenstate(void);
unsigned char uii_tcpgetlistensocket(void);

void uii_logtext(char *text);
void uii_logstatusreg(void);
void uii_sendcommand(unsigned char *bytes, int count);
int uii_readdata(void);
int uii_readstatus(void);
void uii_accept(void);
void uii_abort(void);
int uii_isdataavailable(void);
int uii_isstatusdataavailable(void);

char uii_tcp_nextchar(unsigned char socketid);
int uii_tcp_nextline(unsigned char socketid, char*);
int uii_tcp_nextline_ascii(unsigned char socketid, char*);
void uii_tcp_emptybuffer(void);
void uii_reset_uiidata(void);

void uii_enable_drive_a(void);
void uii_disable_drive_a(void);
void uii_enable_drive_b(void);
void uii_disable_drive_b(void);
void uii_get_drive_a_power(void);
void uii_get_drive_b_power(void);

#endif
