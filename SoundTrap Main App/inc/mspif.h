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

// MSPIF module - communication with a MSP430F2112 wakeup controller
// on DTAG-3 main boards
// mspif.h v1.0

#ifndef MSPIF_H_
#define MSPIF_H_

// most functions return these indicators on success or failure
#define	MSP_OK			(0)
#define	MSP_FAIL		(-1)


#define	MSP_SLAVE_ADDR	(0x60)

// register file size and definitions
// File size is 256 bytes of which bytes 0-127 are RAM and 128-255 are ROM.
// The ROM values encompass the INFOD and INFOC sections of the FLASH.
#define ROMMASK		(0x80)		// if addr&ROMMASK, the INFO sections are being addressed

/* RAM register file on the MSP has the following structure:
typedef struct {				// byte offset
		uchar	command;		// 0
		uchar	mode;			// 1
		uns		eventFlags;		// 2 non persistant flags
		uns 	unused;			// 4 was GAP in V0
		uns		sws ;			// 6
		uns		tempr;			// 8
		uns		vbint ;			// 10
		uns		vbext ;			// 12
		ulong	tick ;			// 14
		ulong	scratch1 ;		// 18 not currently used
		uns		scratch0 ;		// 22 not currently used
		uns 	swver;			// 24
		ulong	rtime ;			// 26
		uns		rcnt ;			// 30
		uns		sws_thrsh ;		// 32
		ulong 	nextStartTime;	// 34
		uns		irCommand ;			// 38
		uns		eCode[2] ;			// 40
		uns		scratch[44] ;	// 44
	} RegFile ;

*/

// definitions of byte addresses

#define	COMMAND_ADDR	(0)
#define	MODE_ADDR		(1)
#define	FLAGS_ADDR		(2)
#define	IRCOMMAND_ADDR	(38)
#define	SWS_ADDR		(6)
#define	TEMPR_ADDR		(8)
#define	VBS_ADDR		(10)
#define	H_ADDR			(12)
#define	RTIME_ADDR		(14)
#define	SW_VER_ADDR		(24)
#define	E_CODE_ADDR		(40)

//#define	RELEASEIN_ADDR	(22)		// no longer used
//#define	RELEASEAT_ADDR	(26)
//#define	RELEASECNT_ADDR	(30)
#define	SWSTHRSH_ADDR	(32)
#define	STARTTIME_ADDR	(34)
#define	APP_PARAMS		(38)
#define	N_APP_PARAMS	(40)	// number of 16-bit words

// COMMAND register bits
// bit  7		SENSOR_REQ
//		6		IRON_REQ
//		5		DSP_ON
//		4		DSP_OFF
//		3		IROFF_REQ
//		2		CHG_OFF
//		1		HIPWR_CHG
//		0		LOWPWR_CHG

#define LOWPWR_REQ		(0x1)	// request low power charging
#define HIPWR_REQ		(0x2)	// request high power charging
#define NOCHG_REQ		(0x4)	// request no charging
#define PWROFF_REQ		(0x10)	// power down DSP now
#define PWRON_REQ		(0x20)	// power up the DSP now
#define REBOOT_REQ		(0x30)	// power-cycle the DSP
#define PWR_MASK		(0x30)	// mask the power mode
#define IRON_REQ		(0x40)	// turn on the infrared remote
#define IROFF_REQ		(0x8)	// turn off the infrared remote
#define SENSOR_REQ		(0x80)	// read the sensor channels

// Flag register bits

#define SHUTDOWN_FLAG		(0x0001)	// Prepare for shut down
#define IR_COMMAND_FLAG		(0x0002)	// DSP IR command received
#define USB_PWR_FLAG		(0x0008)	// Turn-on due to external power attached
#define START_IR_FLAG		(0x0010)	// Turn-on due to IR remote
#define START_TIM_FLAG		(0x0020)	// Turn-on due to time
#define START_USB_CON_FLAG	(0x0040)	// Turn-on due to USB connect
#define START_USB_DIS_FLAG	(0x0080)	// Turn-on due to USB disconnect
#define START_SWS_FLAG		(0x0100)	// Turn-on due to salt-water switch
#define START_MSP_WD_FLAG	(0x0200)	// Turn-on due to msp watchdog reset
#define START_DSP_WD_FLAG	(0x0400)	// Turn-on due to dsp watchdog reset

#define MSP_REBOOTED_FLAG	(0x1000)	// MSP had rebooted
#define START_EXT_BAT_FLAG	(0x2000)	// Restart due to connection of external battery
#define START_INT_BAT_FLAG	(0x4000)	// Restart due to connection of external battery


//Mode register bits
#define ARM_DISCON_BIT	(0x1)	// if 1, start immediately on USB disconnect
#define ARM_SWS_BIT		(0x2)	// if 1, check SWS for start condition when DSP is off
#define ARM_TIMER_BIT   (0x4)   // if 1, check time for start condition when DSP is off

// arming modes
#define	ARMDISABLE		(0)				// disarm everything
#define	ARMDISCON	(ARM_DISCON_BIT)	
#define	ARMSWS		(ARM_SWS_BIT)
#define	ARMTIMER	(ARM_TIMER_BIT)

// timeout for sync in 10s of micro-seconds
#define	SYNCTIMEOUT	(500000)

typedef struct {
			int		vb ;
			Int16	tempr ;
			int		sws ;
			int		extvb ;
			} MSP_Sensors ;

typedef struct {
			ulong	by ;
			ulong	in ;
			uns		cnt ;
			} MSP_Release ;

typedef struct {
			ulong	rtime ;
			ulong	mticks ;
			int		h ;
			} MSP_Time ;

typedef	int (*MSP_TimeFxn)(ulong *,ulong *) ;

extern int mspSwVersion;

extern int	msp_hibernate(int reboot) ;
extern int	msp_getsensors(MSP_Sensors *s) ;
extern int	msp_getflags(void) ;
extern int	msp_getIrCommand(void);
extern int	msp_arm(int mode) ;
extern int	msp_get(Uint16 addr, int *r, int n);
extern int	msp_SendRequest(int request) ;
extern int	msp_setsws(uns thresh) ;
extern int	msp_settime(MSP_Time *t, MSP_TimeFxn f) ;
extern int	msp_getparams(uns *r, int n) ;
extern int	msp_gettime(MSP_Time *t, MSP_Time *local, MSP_TimeFxn f) ;
extern int	msp_releasenow(void) ;
extern ulong	msp_getticks(void) ;
extern int msp_setStartTime(Uint32 startTime);
extern int msp_getStartTime(Uint32 *startTime);
extern int mspPrepRestart();
extern void mspifInit();
void mspifRegisterInterruptHandler(Fxn isr);
int	msp_requestSensorScan();
int	msp_getSensorData(MSP_Sensors *s);
int *msp_brpack(uns *d, int *b, int n);
void mspifwatchdog(Bool on) ;
int	msp_enableBattCharge(uns enable);
int	msp_getFirmwareVersion();
Int16 msp_countToDegC(Uint16 count);
int	msp_IrPower(int on);
int	msp_getEcode( Uint16 *mod, Uint16 *errno);
int	msp_setEcode( Uint16 mod, Uint16 errno);


#endif
