#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <cbm.h>
#include <errno.h>
#include <device.h>
#include <c64.h>
#include <conio.h>
#include <unistd.h>
#include "ultimate_ii.h"

unsigned char socketnr = 0;
int datacount;
long z;
int l = 0;
int index = 0;
unsigned char *file = "0:u-term,s";
int c, ctr;
char buff[1024];
int len_buff = 0;
int ibuff;
char line[100];
int cline;
int dev;
char nextchar(void);
#define LF 10
#define CR 13

void main(void) {
    //for (z=0;z<6000;z++) ;
    dev = 8;
    cbm_open(2, dev, CBM_READ, file);
    ibuff = 0;
    len_buff = 0;

    cline = 0;
    while (1) {
        c = nextchar();
        if (c == 0) {
            break;
        } else if ((c == LF || c == CR) && cline) {
            line[cline] = 0;
            cline = 0;
            printf("> %s\n", line);
        } else if (c != LF && c != CR) {
            line[cline] = c;
            line[++cline] = 0;
        }

    }
    cbm_close(2);
    cbm_close(15);
}

char nextchar(void) {
    char result;
    if (ibuff < len_buff) {
        result = buff[ibuff++];
    } else {
        do {
            len_buff = cbm_read(2, buff, 100);
            if (len_buff == 0) return 0; /// if len = -1 o 0 ?
        } while (len_buff == 0);
        result = buff[0];
        ibuff = 1;
    }
    return result;
}
