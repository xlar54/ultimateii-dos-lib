/*****************************************************************
Ultimate 64/II+ TCP Network Echo Server Demo
Scott Hutter
July 13, 2019

Based on ultimate_dos-1.2.docx and command interface.docx
https://github.com/markusC64/1541ultimate2/tree/master/doc

Disclaimer:  Because of the nature of DOS commands, use this code
soley at your own risk.

Patches and pull requests are welcome

Demo program does not alter any data
******************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef __C128__
#include <c128.h>
#endif

#ifdef __C64__
#include <c64.h>
#endif

#include <conio.h>
#include <peekpoke.h>
#include <unistd.h>
#include "../lib/ultimate_lib.h"

void waitforconnection(void);

void main(void) 
{
    int port = 6400;
    char *end;
    char buf[40];
        
    printf("Demo chat server (incoming connections)\n");
    printf("Port # for listenr:");

    do {
        if (!fgets(buf, sizeof buf, stdin))
            break;

        // remove \n
        buf[strlen(buf) - 1] = 0;
        port = strtol(buf, &end, 10);

    } while (end != buf + strlen(buf));

    uii_tcplistenstart(port);

    printf("Listener on port %d status:%s\n", port, uii_status);
    
    if(uii_success())
        waitforconnection();
}

void waitforconnection(void)
{
    unsigned char c = 0;
    unsigned char socketnr = 0;
    int datacount = 0;
    char buff[2] = {0,0};
    unsigned char state = 0;

    printf("Waiting...\n");

    do
    {
        state = uii_tcpgetlistenstate();

        if(state != NET_LISTENER_STATE_LISTENING && state != NET_LISTENER_STATE_CONNECTED)
        {
            printf("Listener error: %d", state);
            abort();
        }

    } while(state != NET_LISTENER_STATE_CONNECTED);

    printf("Listener status:%s\n", uii_status);
    
    if(uii_success())
    {
        socketnr =  uii_tcpgetlistensocket();
        uii_tcplistenstop();

        if(socketnr != 0)
        {
            printf("=[Remote Connected - F1 : Exit]======\n");
            
            do 
            {
                datacount = uii_socketread(socketnr, 892);

                if(datacount == 0)
                {
                    uii_socketclose(socketnr);
                    printf("\n\nConnection closed.");
                    break;
                }
                    
                if(datacount > 0)
                {
                    printf("%c", uii_data[2]);
                }

                c = kbhit();

                if(c != 0) {
                    c = cgetc();

                    if(c == 133)
                    {
                        uii_socketclose(socketnr);
                        printf("\n\nConnection closed.");
                        break;
                    }

                    buff[0] = c;
                    uii_socketwrite(socketnr, buff);
                    printf("%c", c);
                }

            } while(1);      
        }
    } 

    uii_tcplistenstop();
       
}
