/*****************************************************************
Ultimate II+ DOS Command Library
Scott Hutter

Based on ultimate_dos-1.1.docx and command interface.docx
https://github.com/markusC64/1541ultimate2/tree/master/doc

Disclaimer:  Because of the nature of DOS commands, use this code
soley at your own risk.

Patches and pull requests are welcome

Demo program does not alter any data
******************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <c64.h>
#include "../lib/ultimate_lib.h"

void main(void) 
{
	// -----------------------------------------------------------
	// Control target
	// -----------------------------------------------------------
	// no need to reset the target - the uii_freeze does it automatically. Just be sure to set it back
	// for more dos commands to TARGET_DOS1 or TARGET_DOS2
	uii_freeze();
	printf("\n\nGoing into Ultimate menu: %s", uii_status);
}