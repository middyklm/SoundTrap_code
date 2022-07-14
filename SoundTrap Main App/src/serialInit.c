/*
 * serialInit.c
 *
 *  Created on: 2 Mar. 2022
 *      Author: Middy Khong
 */

#include "d3defs.h"
#include "d3std.h"
#include "board.h"
#include "uart.h"

void uartInitial(void)
{
    Uint32 sysMainAppClock = getsysclk();
    uartInit(sysMainAppClock, 9600);
    uartEnableTx(TRUE);
    uartSelectTransceiver(FALSE);

    uartTx_Str((Uint16 *)"Ocean Instruments\r\n");
    uartTx_Str((Uint16 *)"Reprogrammed: Middy Khong, 2021\r\n");
    uartTx_Str((Uint16 *)"Blast detector\r\n");
    uartTx_Str((Uint16 *)"Start recording\r\n");
}
