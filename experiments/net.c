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
    long z;

    for (z=0; z<3000L; z++);
    printf("PROVA UNO\n");
    uii_settarget(TARGET_NETWORK);
	uii_identify();
	uii_getipaddress();

    printf("\nIP Address: %d.%d.%d.%d", uii_data[0], uii_data[1], uii_data[2], uii_data[3]);
    printf("\n   Netmask: %d.%d.%d.%d", uii_data[4], uii_data[5], uii_data[6], uii_data[7]);
    printf("\n   Gateway: %d.%d.%d.%d\n", uii_data[8], uii_data[9], uii_data[10], uii_data[11]);
    

	uii_tcpconnect("cbbsoutpost.servebbs.com", 80);
	socketnr = uii_data[0];
    if (uii_status[0] == '0' && uii_status[1] == '0') {
    	printf("FASE 1-----------------\n");
        // strcpy(buff,"GET /api/exportbbslist/service.php?f=csv HTTP/1.0\n\n");
        strcpy(buff,"GET / HTTP/1.0\012\012");
    	uii_tcpsocketwrite(socketnr, "get /index.php\012\012\012\012");

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
