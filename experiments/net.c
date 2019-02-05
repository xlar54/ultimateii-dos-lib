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
char buff[400];

void main(void) {
	//int i;
	uii_identify();
	uii_getipaddress();
	uii_settarget(TARGET_NETWORK);
	uii_tcpconnect("cbbsoutpost.servebbs.com", 80);
	socketnr = uii_data[0];
    if (uii_status[0] == '0' && uii_status[1] == '0') {
    	printf("FASE 1-----------------\n");
        // strcpy(buff,"GET /api/exportbbslist/service.php?f=csv HTTP/1.0\n\n");
        strcpy(buff,"get /INDEX.PHP http/1.0\n\n");
    	uii_tcpsocketwrite(socketnr, buff);

    	printf("FASE 2-----------------\n");
        while(1) {
            uii_tcpsocketread(socketnr, 892);
            datacount = uii_data[0] | (uii_data[1]<<8);
            if (datacount > 0)
                printf("%s",uii_data+2);
        }
    } else {
        printf("ERRORE\n");
    }
}
