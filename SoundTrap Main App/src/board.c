// SoundTrap Software v1.0
//
// Copyright (C) 2011-2014, John Atkins and Mark Johnson
//
// This work is a derivative of the D3-API Copyright (C) 2008-2010, Mark Johnson
//
// This file is part of the SoundTrap software. SoundTrap is an acoustic
// recording system intended for underwater acoustic measurements. This
// component of the SoundTrap project is free software: you can redistribute
// it and/or modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation, either version 3 of the License,
// or any later version.
//
// The SoundTrap software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this code. If not, see <http://www.gnu.org/licenses/>.

/*! \file board.c
    \brief Low-level board-dependent interface and initialization routines.
*/

#include <tistdtypes.h>

#include "csl_general.h"
#include "csl_intc.h"
#include "csl_pll.h"
#include "d3defs.h"
#include "d3std.h"
#include "protect.h"
#include "board.h"
#include "devdep.h"
#include "misc.h"
#include "modnumbers.h"
#include "error.h"
#include "mspif.h"
#include "i2c.h"
#include "devdep.h"
#include "misc.h"
#include "sysControl.h"
#include "gpio.h"
#include "dma.h"

#define	MAXCLKCHANGEFXNS (8)

static int nclkchangefxns = 0 ;
static CLK_Fxn clkchangefxns[MAXCLKCHANGEFXNS] ;
static Fxn	hibernate_fxn = NULL ;

Uint32 sysclk = SYSCLK_INIT ;
PLL_Obj pllObj;

void	board_init(void)
{
	// initialize the board - careful: order matters here

	int	*ier0 = (int *)0 ;
	int	*ier1 = (int *)0x45 ; //JMJA Checked OK

	// Disable all interrupts and clear any pending flags.
	// IFR0 and IFR1 are at the next address up from IER0 and IER1.
	*ier0 = 0x0000 ;
	*(ier0+1) = 0x0000 ;
	*ier1 = 0x0000 ;
	*(ier1+1) = 0x0000 ;

	IRQ_globalDisable() ;		// disable all interrupts

	sysControlInit();
	sysControlResetPeripheral(0xFFFF);

	PLL_init(&pllObj,0);

	IRQ_setVecs((Uint32)(&VECSTART)) ;

	setsysclk(SYSCLK_INIT) ;

	gpioInit();
 	dmaInterruptHandlerInit();

 	i2cInit(sysclk);				// start the I2C bus

	gpioSetDir(GPIO_BIT_DSPIO, GPIO_DIR_OUT); //Used for debug

}


void	hibernate(int restart)
{
	leds(REDLED);
	if(hibernate_fxn!=NULL)
		(hibernate_fxn)(restart);
	microdelay(50000) ;	// wait for the reboot circuit to settle
	while (msp_hibernate(restart) != MSP_OK) {
		leds(0);
		microdelay(50000) ;	// wait for the reboot circuit to settle
		leds(2);
	}
	GO_ATOMIC;			// disable interrupts forever
	while(1) {
		leds(0);
		microdelay(50000) ;	// wait for the reboot circuit to settle
		leds(2);
		// should never get to here
	}
}


void	onhibernate(Fxn f)
{
	hibernate_fxn = f ;
}

Uint32 setsysclk(Uint32 clk)
{
    PLL_Config configInfo;
	Uint16	m, d, k;

	switch(clk) {
		case 4096000:
			m = 2000;
			d = 16;
			sysclk = 4096000;
			break;
		case 8192000:
			m = 2000;
			d = 8;
			sysclk = 8192000;
			break;
		case 16384000:
			m = 2000;
			d = 4;
			sysclk = 16384000;
			break;

		case 18432000:
			m = 2249;
			d = 4;
			sysclk = 18432000;
			break;

		case 32768000:
			m = 2000;
			d = 2;
			sysclk = 32768000;
			break;

		case 36864000:
			//m = 2250;
			m = 2249; //this gives multiple of 2500 - appears to be a hardware bug.
			d = 2;
			sysclk = 36864000;
			break;

		case 49152000:
			m = 3000;
			d = 2;
			sysclk = 49152000;
			break;

		case 65536000:
			m = 2000;
			d = 1;
			sysclk = 65536000;
			break;

		case 73728000:
			m = 2250;
			d = 1;
			sysclk = 73728000;
			break;

		case 96000000:
			m = 2930;
			d = 1;
			sysclk = 96000000;
			break;

		case 98304000:
			m = 3000;
			d = 1;
			sysclk = 98304000;
			break;

		case 100000:
			m = 3000;
			d = 1;
			sysclk = 98304000;
			break;

		default:
			m = 2930;
			d = 1;
			sysclk = 96000000;
			break;
	}

 	// call any clkchange functions that have been contributed
 	for(k=0;k<nclkchangefxns;k++) (clkchangefxns[k])(sysclk) ;

 	GO_ATOMIC ;
    configInfo.PLLCNTL1 = (m-4) & 0x0FFF; //CGCR1
    configInfo.PLLINCNTL = 0x8000;	//CGCR2
    configInfo.PLLOUTCNTL = d > 1 ? 0x0200 | (d-1) : 0x0000; //CRCR4
    configInfo.PLLCNTL2 = 0x0806; //CCR2
    PLL_bypass(&pllObj);
    PLL_reset(&pllObj);
    PLL_config(&pllObj, &configInfo);
 	PLL_enable(&pllObj);
 	END_ATOMIC ;
 	return( sysclk ) ;
}

Uint32	getsysclk(void)
{
 	return(sysclk) ;
}

int	onclkchange(CLK_Fxn f)
{
 	if(nclkchangefxns>=MAXCLKCHANGEFXNS)
		return(FAIL) ;

	clkchangefxns[nclkchangefxns] = f ;
	++nclkchangefxns ;
	return(OK) ;
}

int	uid(unsigned int *d, int n)
{
 	// return the unique identification number of the DSP chip
	ioport volatile CSL_SysRegs  *sysRegs;
	sysRegs = (CSL_SysRegs *)CSL_SYSCTRL_REGS;
 	if(n == UID_LONG) {
    	*d++ = sysRegs->DIEIDR3;
    	*d++ = sysRegs->DIEIDR2;
	}
	*d++ = sysRegs->DIEIDR1;
 	*d =   sysRegs->DIEIDR0;
 	return(n);
}

int	adcread(Uint16 *d)
{
 	return(0); //TODO
}

int	isusbboot(void)
{
 	return(0); //TODO
}
