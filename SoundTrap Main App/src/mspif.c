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

/*! \file mspif.c
    \brief Communication with a MSP430F2112 wakeup controller
*/

#include "math.h"
#include "d3defs.h"
#include "d3std.h"
#include "protect.h"
#include "board.h"
#include "devdep.h"
#include "job.h"
#include "misc.h"
#include "modnumbers.h"
#include "error.h"
#include "devdep.h"
#include "i2c.h"
#include "mspif.h"
#include "gpio.h"
#include "config.h"
#include "timr.h"

#define	MSPNICE			(5)

//#define	msp_setaddr(b)	(i2cWrite(MSP_SLAVE_ADDR, (Uint16 *)(b),1, MSP_I2C_TIMEOUT))
#define	msp_rxdata(b,n)	(i2cRead(MSP_SLAVE_ADDR, (Uint16 *)(b), n))
#define	msp_txdata(b,n)	(i2cWrite(MSP_SLAVE_ADDR, (Uint16 *)(b), n))

#define	BYTE2UNS(r)		((*((uns *)(r)+1)<<8)|(*((uns *)(r))))
#define	UNS2BYTE(b,r)	{*(b)=(r)&0x0ff; *(b+1)=(r)>>8;}

// approx. conversions from ADC units to mV
#define	VBS2MV(v)		((v)*5 - ((v)>>3))
#define	VBS2MV_V212(v)		((v)*5.57)

#define	SWS2MV(v)		((v)*5 + ((v)>>3))
#define	TEMPR2MV(v)		((v)*2 + ((v)>>1))

#define SWS_THRES 600

#define txlow			(DTAG3_wio2_low)
#define	txhigh			(DTAG3_wio2_high)
#define	rxget			(DTAG3_wio1_get)
#define	txoutput		(DTAG3_wio2_output)
#define	txinput			(DTAG3_wio2_input)
#define	rxinput			(DTAG3_wio1_input)

static Fxn mspRegisteredInterruptHandler;


int mspSwVersion;

ulong	byte2ulong(int *r) ;
void	ulong2byte(int *b, ulong w) ;

int		msp_hibernate(int reboot)
{
	Uint16	b[2]={COMMAND_ADDR, PWROFF_REQ};

 	if(reboot)
		b[1] |= REBOOT_REQ ;

 	if(i2cWrite(MSP_SLAVE_ADDR, b, 2) != OK)
		return(MSP_FAIL) ;

 	return(MSP_OK) ;
}

int	msp_requestSensorScan()
{
 	int b[2]={COMMAND_ADDR, SENSOR_REQ} ;

 	if(msp_txdata(b,2))		// send the sensor read command
		return(MSP_FAIL) ;

 	return(MSP_OK) ;
}

int	msp_IrPower(int on)
{
 	int b[2]={COMMAND_ADDR, 0} ;
 	b[1] = on ? IRON_REQ : IROFF_REQ;

 	if(msp_txdata(b,2))		// send the sensor read command
		return(MSP_FAIL) ;

 	return(MSP_OK) ;
}

//Constants for Panasonic thermistor ERTJ0EP473F (1%)

#define R1 47000.0 //series resistor
#define R0 47000.0 //resistance at T0
#define B 4050.0	//thermistor B value
#define T0 298.15	//25 deg C in kelvin
#define REF_V 2.5	//ADC reference
#define EXCIT_V 2.85 //excitation voltage
#define K0 273.15 //0 deg C in kelvin

Int16 msp_countToDegC(Uint16 count)
{
	double R, t;
	R = R1 / ((1023.0 / count) - 1);
	R = R * (EXCIT_V/REF_V);
	t = (1.0/T0) + ((1.0/B) * log(R/R0));
	t = 1.0/t;
	return (t - K0) * 100; //returns deg C * 100
}

int	msp_getSensorData(MSP_Sensors *s)
{

	int err, r[8];;
	Uint16 b[2]={6, 0} ;//SWS_ADDR

	err = i2cWriteRead(MSP_SLAVE_ADDR, b, 1, (Uint16 *)r, 8);

	if(err)
		return(MSP_FAIL) ;

	if(mspSwVersion > 211) {
		s->vb = VBS2MV_V212(BYTE2UNS(r+4)) ;
		s->extvb = VBS2MV_V212(BYTE2UNS(r+6)) ;
	}
	else {
		s->vb = VBS2MV(BYTE2UNS(r+4)) ;
		s->extvb = VBS2MV(BYTE2UNS(r+6)) ;
	}
	s->sws = BYTE2UNS(r);
	s->tempr = msp_countToDegC(BYTE2UNS(r+2)); //deg C * 100

	return(MSP_OK) ;
}

int		msp_getsensors(MSP_Sensors *s)
{
	int	err, r[8], b[2]={COMMAND_ADDR, SENSOR_REQ} ;

	if(msp_txdata(b,2))		// send the sensor read command
		return(MSP_FAIL) ;

	do {						// wait for the sensors to be read
		err = i2cWriteRead(MSP_SLAVE_ADDR, (Uint16 *)b, 1, (Uint16 *)b+1, 1);
	} while(err==0 && b[1]&SENSOR_REQ) ;

	if(err)
		return(MSP_FAIL) ;

	b[0] = SWS_ADDR ;			// read all of the sensor values
	err = i2cWriteRead(MSP_SLAVE_ADDR, (Uint16 *)b, 1, (Uint16 *)r, 8);

	if(err)
		return(MSP_FAIL) ;

	// convert to mV and store in the structure
	if(mspSwVersion > 211)
		s->vb = VBS2MV_V212(BYTE2UNS(r+4)) ;
	else
		s->vb = VBS2MV(BYTE2UNS(r+4)) ;

	s->sws = SWS2MV(BYTE2UNS(r)) ;
	s->tempr = TEMPR2MV(BYTE2UNS(r+2)) ;
	return(MSP_OK) ;
}


int	msp_arm(int mode)
{
	int b[2]={MODE_ADDR, 0} ;
	b[1] = mode;
	if(msp_txdata(b,2))
		return(MSP_FAIL) ;
	return(MSP_OK) ;
}

int	msp_get(Uint16 addr, int *r, int n)
{
	int	err ;
	err = i2cWriteRead(MSP_SLAVE_ADDR, &addr, 1, (Uint16 *)r, n);
	if(err)
		return(MSP_FAIL) ;

	return(MSP_OK) ;
}

int	msp_getflags(void)
{
 int	r[2] ;
 if(msp_get(FLAGS_ADDR,r,2)==MSP_FAIL)
	return(MSP_FAIL) ;

 return(byte2ulong(r));
}

int	msp_setEcode( Uint16 mod, Uint16 errno)
{
	int	r[5];
	r[0] = E_CODE_ADDR;
	UNS2BYTE(r+1, mod);
	UNS2BYTE(r+3, errno);
	if(msp_txdata(r,4)==MSP_FAIL)
		return(MSP_FAIL) ;
	return MSP_OK;
}

int	msp_getEcode( Uint16 *mod, Uint16 *errno)
{
	int	r[4] ;
	if(msp_get(E_CODE_ADDR,r,4)==MSP_FAIL)
		return(MSP_FAIL) ;
	*mod = BYTE2UNS(r);
	*errno = BYTE2UNS(r+2);
	return MSP_OK;
}



int	msp_getFirmwareVersion()
{
	int	r[2] ;
	if(msp_get(SW_VER_ADDR,r,2)==MSP_FAIL)
		return(MSP_FAIL) ;
	mspSwVersion = BYTE2UNS(r);
	return MSP_OK;
}

int	msp_getIrCommand(void)
{
 int	r[1] ;
 if(msp_get(IRCOMMAND_ADDR,r,1)==MSP_FAIL)
	return(MSP_FAIL) ;

 return(r[0]) ;
}
ulong	msp_getticks(void)
{
 int	rr[8] ;
 uns	rw[4] ;
 ulong	t1, t2 ;

 if(msp_get(RTIME_ADDR,rr,4)==MSP_FAIL)
	return(0) ;

 if(msp_get(RTIME_ADDR,rr+4,4)==MSP_FAIL)
	return(0) ;

 // convert to ulong and store in the structure
 msp_brpack(rw,rr,4) ;
 t1 = (ulong)rw[0] | ((ulong)rw[1]<<16) ;
 t2 = (ulong)rw[2] | ((ulong)rw[3]<<16) ;
 return((t1<t2) ? t1:t2) ;
}


int msp_setStartTime(Uint32 startTime)
{
 int b[5];
 b[0] = STARTTIME_ADDR;
 ulong2byte(&b[1], startTime);
 if(msp_txdata(b,5))
	return(MSP_FAIL) ;
 return(MSP_OK) ;
}

int msp_getStartTime(Uint32 *startTime)
{
 	int	err, r[4], b[2]={STARTTIME_ADDR} ;
 	err = i2cWriteRead(MSP_SLAVE_ADDR, (Uint16 *)b, 1, (Uint16 *)r, 4);

 	if(err)
		return(MSP_FAIL) ;
 	*startTime = byte2ulong(r);
 	return(MSP_OK) ;
}



int	msp_setsws(uns thresh)
{
 uns	b[3]={SWSTHRSH_ADDR} ;
 UNS2BYTE(b+1,thresh) ;
 if(msp_txdata(b,3))
	return(MSP_FAIL) ;

 return(MSP_OK) ;
}

int	msp_enableBattCharge(uns enable)
{
	Uint16	b[2]={COMMAND_ADDR, PWROFF_REQ};
	b[1] = enable ? 0x02 : 0x04 ;

 	if(i2cWrite(MSP_SLAVE_ADDR, b, 2) != OK)
		return(MSP_FAIL) ;

 	return(MSP_OK) ;
}

int	msp_SendRequest(int request)
{
 int	b[2]={COMMAND_ADDR,0} ;

 b[1] = request;
 if(msp_txdata(b,2))
	return(MSP_FAIL) ;
 return(MSP_OK) ;
}

int		msp_settime(MSP_Time *t, MSP_TimeFxn f)
{
 int		b[11]={H_ADDR,0,0} ;

 // if a time function is provided, call this to update the time
 if(f!=(MSP_TimeFxn)NULL)
 	if((f)(&(t->rtime),&(t->mticks))==FAIL)		// get the time
		return(MSP_FAIL);

 // message format is:
 // H_ADDR,00,rtime,mticks
 ulong2byte(b+3,t->rtime) ;		// configure a time-set message
 ulong2byte(b+7,t->mticks) ;
 if(msp_txdata(b,11))
	return(MSP_FAIL) ;

 return(MSP_OK) ;
}


int		msp_gettime(MSP_Time *t, MSP_Time *local, MSP_TimeFxn f)
{
 	int	err, r[10], b[2]={H_ADDR} ;

 // if a time function is provided, call this to update the local time
	 if(f!=(MSP_TimeFxn)NULL)
 		if((f)(&(local->rtime),&(local->mticks))==FAIL)		// get the time
			return(MSP_FAIL);

   //July 2014 - changed to using i2c restart to fix occasional error here
   err = i2cWriteRead(MSP_SLAVE_ADDR, (Uint16 *)b, 1, (Uint16 *)r, 10);

 	if(err)
		return(MSP_FAIL) ;

 	t->h = BYTE2UNS(r) ;			//TODO this looks to be wrong
 	t->rtime = byte2ulong(r+2) ;
 	t->mticks = byte2ulong(r+6) ; 	//TODO as does this
 	return(MSP_OK) ;
}


ulong	byte2ulong(int *r)
{
 	ulong	w = (ulong)BYTE2UNS(r+2) ;
 	return((w<<16) | (ulong)BYTE2UNS(r)) ;
}


void	ulong2byte(int *b, ulong w)
{
 	int	k ;
 	for(k=0; k<4; k++, w>>=8)
 		*b++ = (int)(w & 0x0ff) ;
}


int	 *msp_brpack(uns *d, int *b, int n)
{
 // byte pack in little endian order
 	int	k ;

 	for(k=0;k<n;k++,b+=2)
		*d++ = *b | (*(b+1)<<8) ;

 	return(b) ;
}


int mspPrepRestart()
{
	int result = TRUE;
	switch(systemConfig.startTriggerMode) {

		case startTriggerModeManual:
				result &= ( msp_arm(ARMDISABLE) == MSP_OK);
		break;

		case startTriggerModeOnDisconnect:
				result &= ( msp_arm(ARMDISCON) == MSP_OK);
		break;
		case startTriggerModeTime:
			if(systemConfig.startTime > TIMR_getUnixTime())
			{
				result = ( msp_setStartTime( systemConfig.startTime) == MSP_OK);
				result &= ( msp_arm(ARMTIMER) == MSP_OK);
			}
			else {
				result = 0;
				//result &= ( msp_arm(ARMDISCON) == MSP_OK);
			}
		break;
		case startTriggerModeSWS:
				result &= msp_setsws(SWS_THRES);
				result &= msp_arm(ARMSWS)== MSP_OK;
		break;
	}
	return result;
}


// The actual interrupt service routine.
void mspifSynchInterruptHandler()
{
  if(mspRegisteredInterruptHandler != NULL)
	  mspRegisteredInterruptHandler();
}


// register a job to post when the msp interrupt fires
void mspifRegisterInterruptHandler(Fxn isr)
{
	mspRegisteredInterruptHandler = isr;
}


void mspifInit()
{
	mspRegisteredInterruptHandler = NULL;
	gpioSetDir( GPIO_BIT_WIF1, GPIO_DIR_IN);
	gpioSetDir( GPIO_BIT_WIF2, GPIO_DIR_OUT);
	gpioEnableInterrupt( GPIO_BIT_WIF1, TRUE, FALSE);
	gpioInterruptHandlerRegister(GPIO_BIT_WIF1, mspifSynchInterruptHandler);
}


void mspifwatchdog(Bool on)
{
 gpioSetVal( GPIO_BIT_WIF2, on);
}
