/*****************************************************************
Ultimate 64/II+ TCP Network Echo Server Demo
Scott Hutter
July 13, 2019

Based on ultimate_dos-1.1.docx and command interface.docx
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
#include "..\lib\ultimate_lib.h"

void main(void) 
{
    unsigned char state = 0;
    unsigned char c = 0;
    unsigned char socketnr = 0;
    int datacount = 0;
    char buff[2] = {0,0};
    
    printf("Demo echo server (incoming connections)\n");
    printf("Port 6400\n");
    printf("Waiting...\n");

    uii_tcplistenstart();

    do
    {
        state = uii_tcpgetlistenstate();

    } while(state != 2);

    socketnr =  uii_tcpgetlistensocket();

    if(socketnr != 0)
    {
        printf("Connected!\n\n");
        
        do 
        {
            datacount = uii_tcpsocketread(socketnr, 892);

            if(datacount == 0)
            {
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
                buff[0] = c;
                uii_tcpsocketwrite(socketnr, buff);
            }
        } while(1);      
    }
}