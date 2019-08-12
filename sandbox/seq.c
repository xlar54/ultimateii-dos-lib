#include <stdio.h>
#include <cbm.h>
#include <device.h>

long z;
unsigned char *file = "0:u-term,s";
char buff[1024];
int len_buff, ibuff;
int dev;

char line[100];
char nextchar(void);
int nextline(char*);

void main(void) {
    //for (z=0;z<6000;z++);
    int ok;
    dev = getcurrentdevice();
    cbm_open(2, dev, CBM_READ, file);

    while (ok = nextline(line)) {
        printf("* %s\n", line);
    }

    cbm_close(2);
}

int nextline(char *result) {
    int c, count = 0;
    *result = 0;
    while ((c = nextchar()) != 0 && c != 0x0A && c != 0x0D)
        result[count++] = c;
    result[count] = 0;
    return c != 0 || count > 0;
}

char nextchar(void) {
    char result;
    if (ibuff < len_buff) {
        result = buff[ibuff++];
    } else {
        do {
            len_buff = cbm_read(2, buff, 1024);
            if (len_buff == 0) return 0; /// if len = -1 o 0 ?
        } while (len_buff == 0);
        result = buff[0];
        ibuff = 1;
    }
    return result;
}
