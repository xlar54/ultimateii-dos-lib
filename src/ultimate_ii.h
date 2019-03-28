/*****************************************************************
Ultimate II+ DOS Command Library
Scott Hutter, Francesco Sblendorio

Based on ultimate_dos-1.2.docx and command interface.docx
https://github.com/markusC64/1541ultimate2/tree/master/doc

Disclaimer:  Because of the nature of DOS commands, use this code
soley at your own risk.

Patches and pull requests are welcome
******************************************************************/

#ifndef _ULTIMATE_II_DOS_H_
#define _ULTIMATE_II_DOS_H_

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

//#define DEBUG
#define DISPLAY_READ

#define uii_tcpconnect_success() (uii_status[0] == '0' && uii_status[1] == '0')

extern unsigned char uii_status[STATUS_QUEUE_SZ];
extern unsigned char uii_data[DATA_QUEUE_SZ*2];

extern char uii_date_str[11];
extern char uii_time_str[9];
extern int uii_year;
extern int uii_month;
extern int uii_day;
extern int uii_hour;
extern int uii_minute;
extern int uii_second;


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

void uii_get_time(void);

#endif
