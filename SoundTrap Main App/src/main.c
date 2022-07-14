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

// The SoundTrap software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this code. If not, see <http://www.gnu.org/licenses/>.


//-----Middy's added header------
#include <tistdtypes.h>
#include "uart.h"
#include <csl_rtc.h>
#include <stdlib.h>
#include "d3std.h"
//-------End added header--------
#include <cstdlib>
#include "csl_intc.h"
#include "d3defs.h"
#include "d3std.h"
#include "protect.h"
#include "board.h"
#include "devdep.h"
#include "misc.h"
#include "modnumbers.h"
#include "error.h"
#include "dmem.h"
#include "fmem.h"
#include "job.h"
#include "data.h"
#include "timr.h"
#include "flsh.h"
#include "logg.h"
#include "devdep.h"
#include "misc.h"
#include "config.h"
#include "info.h"
#include "schedule.h"
#include "mspif.h"
#include "mspInterruptHandler.h"
#include "sar.h"
#include "hid.h"

//------------Middy's added code--------------
Uint32 sysClock;        // return system clock frequency
Uint16 myTime1, myTime2;
Uint32 time1, time2, myUnixTime;
const Uint32  clkMain = 73728000;
//--------------End added code----------------

// user-defined application programs
extern int	APP_init(void) ;      // launches user tasks
extern int	APP_close(Ptr s, Ptr d) ;     // shutdown user tasks

// idle job runs whenever there is no other job to run.
// it is defined in idle.c and can be overlaid with a user function
extern void	IDLE_init(void) ;

// log to write error messages
static int	err_log = 0 ;
Uint32 systemStartTime;

int   report_errors(int mod,int errno) ;

int   report_errors(int mod,int errno)
{
 if(err_log>0) {
	char s[40] ;
  	snprintf(s, 40, "ERROR %04xh from module %04xh", errno, mod) ;
	LOG_diary(err_log,s) ;
 }
 return(OK) ;
}


int logStartCondition()
{
	int startFlags;
	if( (startFlags = msp_getflags()) != MSP_FAIL)
	{
		if(startFlags & START_IR_FLAG)	INFO_event("START",NULL,"IR_REMOTE");
		if(startFlags & START_TIM_FLAG)	INFO_event("START",NULL,"TIM");
		if(startFlags & START_USB_CON_FLAG)	INFO_event("START",NULL,"USB_CON");
		if(startFlags & START_USB_DIS_FLAG)	INFO_event("START",NULL,"USB_DIS");
		if(startFlags & START_SWS_FLAG)	INFO_event("START",NULL,"SWS");
		if(startFlags & START_MSP_WD_FLAG)	INFO_event("START",NULL,"MSPWD");
		if(startFlags & START_DSP_WD_FLAG)	INFO_event("START",NULL,"DSPWD");
		if(startFlags & START_EXT_BAT_FLAG)	INFO_event("START",NULL,"EX_BAT_APPLIED");
		if(startFlags & MSP_REBOOTED_FLAG)	INFO_event("START",NULL,"MSP_REBOOT");
		if(startFlags & START_INT_BAT_FLAG)	INFO_event("START",NULL,"REVERT_INTERN_BAT");
	}
	else {
		startFlags = 0;
		INFO_event("START",NULL,"UNKNOWN");
	}
	return startFlags;
}

void logBatteryCondition()
{
    char s[30];
    MSP_Sensors sens;
	if( msp_getSensorData(&sens) == MSP_OK) {
		sprintf(s, "%d", mspSwVersion);
		INFO_event("MSPVER", NULL, s);
		sprintf(s, "%d", sens.vb);
		INFO_event("INT_BATT", "UNITS=\"mV\"", s);
		sprintf(s, "%d", sens.extvb);
		INFO_event("EX_BATT", "UNITS=\"mV\"", s);
		sprintf(s, "%d", sens.tempr);
		INFO_event("TEMPERATURE", "UNITS=\"DegCx100\"", s);
	}
}

void logLastEcode()
{
	Uint16  mod, errno;
    char s[10];
	if(msp_getEcode(&mod, &errno) == MSP_OK) {
		if(mod != 0) {
			snprintf(s, 10, "0x%x", mod);
			INFO_event("LAST_ECODE_MOD", NULL, s);
			snprintf(s, 10, "0x%x", errno);
			INFO_event("LAST_ECODE_CODE", NULL, s);
			msp_setEcode(0,0);
		}
	}
	else INFO_event("ECODE", NULL, "UNKONWN");
}



//called when a new flash file is opened
void newFileEventHandler(void)
{
	INFO_event("HARDWARE_ID", NULL, hidGetString()) ;
	logBatteryCondition();
}


void main()
{
	int startFlags;
	char s[32] ;

	//asm("	BCLR CLKOFF,ST3_55"); //enable clk out

	board_init() ;      // initialize hardware

	leds(REDLED) ;      // show a red led to make user feel good

	FMEM_init() ;		// initialize the fast memory manager
 	END_PROTECT ;       // enable global interrupts
	END_ATOMIC ;
	IRQ_globalEnable();

	if(DMEM_init() == FAIL)   // initialize the memory manager
		fatal(MAIN_MOD, 0);// fail, log error

	if(TIMR_open() == FAIL)   // initialize the clock and timer
		fatal(MAIN_MOD, 1);// fail, log error

	systemStartTime = TIMR_getUnixTime();


	if( sarInit() == FAIL)
		fatal(MAIN_MOD, 4);// fail, log error

	if( hidInit() == FAIL)
		fatal(MAIN_MOD, 5);// fail, log error

	if(devdep_init() == FAIL)
		fatal(MAIN_MOD, 6);// fail, log error

	if(FLSH_init() == FAIL)   // initialize the flash memory file system
		fatal(MAIN_MOD, 2);// fail, log error

	if(FLSH_open() == FAIL)   // start the flash memory file system
		fatal(MAIN_MOD, 3);// fail, log error

	if( !configInit()) {
		snprintf(s, 32, "WARNING=\"USING DEFAULTS\"");
		INFO_event("CONFIG",s,NULL) ;
	}

	mspifInit();

	msp_getFirmwareVersion();

	// initialize the job scheduler (real-time executive)
	if(JOB_init() == FAIL)
		fatal(MAIN_MOD, 7);// fail, log error

	// from here-on, jobs can be posted to the ready queue
	IDLE_init() ;       // post the idle job so there is always something to do

	//TODO: Fix this. Dmem errors currently cause endless malloc loop & stack overflow. JMJA
	//err_log = LOG_open("err",NOQUOTE) ;   // open a log for errors
	//onerror((Fxn)report_errors) ;


	startFlags = logStartCondition();
	logLastEcode();

	if(scheduleInit(startFlags) == FAIL) //record schedule init
		fatal(MAIN_MOD, 8);// fail, log error

	INFO_event("HARDWARE_ID", NULL, hidGetString()) ;
	logBatteryCondition();

	flsh_regNewFileCallbackHandler(newFileEventHandler);

	// initialize the user module
	if(APP_init() == FAIL)
		fatal(MAIN_MOD, 9);// fail, log error

	mspInterryptHandlerInit();

	//------------Middy's code--------------

//	uartTx_Str((Uint16 *)"Ocean Instruments\r\n");
//	uartTx_Str((Uint16 *)"Reprogrammed: Middy Khong, 2021\r\n");
//	uartTx_Str((Uint16 *)"Blast detector\r\n");
//	uartTx_Str((Uint16 *)"Start recording\r\n");    // uart_init() in APP_init(), all uart strings MUST below APP_init()
/*
    sysClock = getsysclk();
	CSL_RtcTime         GetRtcTime1, GetRtcTime2;
	MSP_Time m;

	    // SET TIME
//      CSL_RtcConfig       myRTC_config;
//	    myRTC_config.rtcyear  = 20;
//	    myRTC_config.rtcmonth = 3;
//	    myRTC_config.rtcday   = 6;
//	    myRTC_config.rtchour  = 12;
//	    myRTC_config.rtcmin   = 0;
//	    myRTC_config.rtcsec   = 0;
//	    myRTC_config.rtcmSec  = 0;

	RTC_start();

    RTC_getTime(&GetRtcTime1);
    time1 = TIMR_getUnixTime();

    msp_gettime(&m,NULL,NULL);
    myTime1 = m.rtime;

    delay_ms(1000);         // Do NOT make delay > 1000. *Check speed to adjust delay, it's not 1 millisecond resolution

    RTC_getTime(&GetRtcTime2);
    time2 = TIMR_getUnixTime();

    //myTime1 = GetRtcTime1.secs;
    //myTime2 = GetRtcTime2.secs;
    RTC_stop();

    msp_gettime(&m,NULL,NULL);
    myTime2 = m.rtime;
    myUnixTime = TIMR_getUnixTime();

    char myTime1_str[20], myTime2_str[20], sysClock_str[20], systemStartTime_str[20], myUnixTime_str[20];
    ltoa(sysClock, sysClock_str);
    ltoa(myTime1, myTime1_str);
    ltoa(myTime2, myTime2_str);
    ltoa(systemStartTime, systemStartTime_str);
    ltoa(myUnixTime, myUnixTime_str);

    uartTx_Str((Uint16 *)"system clock = "); uartTx_Str((Uint16 *) sysClock_str); uartTx_Str((Uint16 *)" Hz\r\n");
    uartTx_Str((Uint16 *)"system start time = "); uartTx_Str((Uint16 *) systemStartTime_str); uartTx_Str((Uint16 *)"\r\n");
    uartTx_Str((Uint16 *)"myUnixTime = "); uartTx_Str((Uint16 *) myUnixTime_str); uartTx_Str((Uint16 *)"\r\n");
    uartTx_Str((Uint16 *)"myTime1 = "); uartTx_Str((Uint16 *) myTime1_str); uartTx_Str((Uint16 *)"\r\n");
    uartTx_Str((Uint16 *)"myTime2 = "); uartTx_Str((Uint16 *) myTime2_str); uartTx_Str((Uint16 *)"\r\n");


    if(myTime2 > myTime1){
        uartTx_Str((Uint16 *)"Got new time\r\n");
    }else{
        uartTx_Str((Uint16 *)"Same time\r\n");
    }
*/

    //----------End Middy's code------------

	leds(GREENLED) ;    // indicate that initialization is over
	JOB_scheduler() ;   // start job scheduling
	// only reach here when a terminator job is run or when there are no more jobs

	// shutdown
	APP_close(NULL, NULL) ;       // close user application
	//LOG_close(err_log) ;      // close error log
	err_log = 0 ;
	leds(GREENLED) ;    // indicate that initialization is over

	// shutdown services
	FLSH_close(1);
	leds(GREENLED) ;// indicate waiting for MSP to cut the power
	TIMR_close() ;
	leds(REDLED|GREENLED) ;    // indicate that initialization is over
	hibernate(restartOnExit);
	exit(1) ;
}


