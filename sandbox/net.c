#include <stdio.h>
#include <string.h>
#include "ultimate_lib.h"

unsigned char socket = 0;
char buff[1024];
char name[100];
char host[100];
char port[10];
char *p1, *p2, *p3;
int count = 0;

void main(void) {
    uii_settarget(TARGET_NETWORK);
    uii_identify();
    uii_getipaddress();

    printf("\nIP Address: %d.%d.%d.%d", uii_data[0], uii_data[1], uii_data[2], uii_data[3]);
    printf("\n   Netmask: %d.%d.%d.%d", uii_data[4], uii_data[5], uii_data[6], uii_data[7]);
    printf("\n   Gateway: %d.%d.%d.%d\n\n", uii_data[8], uii_data[9], uii_data[10], uii_data[11]);

    socket = uii_tcpconnect("cbbsoutpost.servebbs.com", 80);

    if (uii_success()) {
        uii_tcpsocketwrite_ascii(socket, "GET /api/exportbbslist/service.php?f=csv HTTP/1.0\n\n");

        // Skip HTTP header
        while (uii_tcp_nextline_ascii(socket, buff) && *buff);

        //Skip first line (header)
        uii_tcp_nextline_ascii(socket, buff);

        // Read single lines
        while (uii_tcp_nextline_ascii(socket, buff) && *buff) {
            p1 = strchr(buff+1, ',');
            p2 = strchr(p1+1, ',');
            p3 = strchr(p2+1, ',');

            strncpy(name, buff+1, (p1-buff-2));
            name[p1-buff-2]=0;

            strncpy(host, p1+2, p2-p1-3);
            host[p2-p1-3]=0;

            strncpy(port, p2+2, p3-p2-3);
            port[p3-p2-3]=0;

            printf("%d) %s:%s\n", ++count, host, port);
        }
        uii_tcpclose(socket);

    } else {
        printf("ERRORE\n");
    }
}
