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

// simple_recorder - example user application
// Records audio from a single channel, decimates, compresses and
// stores in flash memory

#include <math.h>
#include "d3defs.h"
#include "d3std.h"
#include "protect.h"
#include "board.h"
#include "devdep.h"
#include "misc.h"
#include "modnumbers.h"
#include "error.h"
#include "dmem.h"
#include "job.h"
#include "data.h"
#include "audio.h"
#include "flsh.h"
#include "timr.h"
#include "decm.h"
#include "cfg.h"
#include "logg.h"
#include "i2c.h"
#include "info.h"
#include "x3cmpv2.h"
#include "decfilt.h"
#include "config.h"
#include "mspif.h"
#include "audioOut.h"
#include "sensor.h"
#include "accelerometer.h"
#include "hid.h"
#include "temperatureLogger.h"
#include "uart.h"
#include "filt.h"
#include "cdet_v2.h"
#include "uart.h"
#include "gpsLogger.h"
#include "bwdet.h"
#include "bwdet.h"
#include "adcLogger.h"
#include "mux.h"
#include "serialInit.h"


//#define	PLOG
#ifdef PLOG
static int pLogId1, pLogId2;
#endif

#define	CHAN	AUDIO1		// audio input on channel 1
static int detid, tempLogId, accelLogId; // module handles

int audid;
int saveid ;
int muxid;
int nChan;

static int	ledFlashState = 0;
static CDET_Params bwdetparams;

#define STARTUP_DELAY	10		// time to wait after starting audio before attaching detector in seconds
#define DET_DELAY		10		// time to wait after attaching detector before starting detector in seconds

const Uint32  clkRecApp = 73728000;

int recordjob(Ptr a, DATA_Obj *d)
{
    // this job runs whenever there is a block of compressed audio data
    // to save in the flash memory file system.

    // check for an upstream error
    if (d->size == 0) {
        DATA_free(d);	// no need to signal an error - it has already been done
        return (OK);
    }

    d->id = saveid;
    FLSH_save(d); // save the data
    DATA_free(d);		// free it - it is now FLSH's responsibility

    return (OK);
}

int	start_det(Ptr s, Ptr d)
{
	// Enable the detector. This is done a few milliseconds after the detector is attached
	// to allow the startup transient to pass.
	BWDET_state(1) ;
	return(OK) ;
}

int	attach_det(Ptr s, Ptr d)
{
	// Attach the detector. This is done well after the audio starts to allow
	// the self-calibration to finish otherwise the DSP runs out of time.
	CFG_attach(audid, detid, 4);
	TIMR_doin(DET_DELAY,(JOB_Fxn)start_det,NULL);
	return(OK) ;
}

int	flashnext(Ptr s, Ptr d)
{
	if((--ledFlashState)&1) {
		leds(REDLED|GREENLED);
		TIMR_DO_IN_MS(80, (JOB_Fxn)flashnext, NULL);
	}
	else {
		leds(NOLED);
		if(ledFlashState>0) TIMR_DO_IN_MS(200, (JOB_Fxn)flashnext, NULL);
	}
	return(OK) ;
}


void flash(int n)
{
	ledFlashState = n<<1 ;
	flashnext(NULL,NULL) ;
}

int	wd_off(Ptr s, Ptr d)
{
	mspifwatchdog(0) ;
	return(OK) ;
}

int	watchdog(Ptr s, Ptr d)
{
	flash(1) ;
	mspifwatchdog(1) ;
	TIMR_DO_IN_MS(100,(JOB_Fxn)wd_off,NULL) ;
	return(OK) ;
}

int	startjob(Ptr s, Ptr d)
{
	// start the audio and detector
	AUDIO_Prep(audid);

	AUDIO_Start(); //starts all enabled audio channels

	if(systemConfig.detect1 > 0) {
		TIMR_doin(STARTUP_DELAY,(JOB_Fxn)attach_det,NULL);
	}
	return (OK);
}

int stopjob()
{
	AUDIO_stop(audid);

	if(detid != 0) BWDET_close() ;
    return (OK);
}

int init_chans(int channels, int band)
{
    int id;
    int err = 0;
    AUDIO_Stat a;
    INFO_WavMeta wm = {0l,16,1,"wav",0,1,0};

    audid = AUDIO_open(channels, band);
    id = audid;
    AUDIO_gain(channels, systemConfig.gain);
    if(hid == ST300) AUDIO_hpass(channels, systemConfig.highPass);

    AUDIO_status(audid, &a);		// find out the audio sampling rate

    audioOutInit(band);

    wm.nch = a.nch;
    wm.fs = a.fs;

    if(nChan > 1) {
		muxid = muxOpen();
		err = CFG_attach(id, muxid, 3);
		id = muxid;
    }

	if (systemConfig.decimator > 0) {	// if audio turned on
		if (systemConfig.decimator > 1) {	// if a decimation factor is requested
			Uint16 decimator = systemConfig.decimator;

			if(decimator> getMaxDecFilt())
			{
				//requested decimation is too large to do in one hit
				//use 2 decimators, starting with a divide by 2
				DecFilt *dflt = getdecfilt(2);	// get a decimation filter
				if (dflt == NULL)
					return (-1);
				// open a decimator
				int decid = DECM_open(dflt->filt, dflt->len, 2, a.nch);
				// patch the audio chain through the decimator
				err = CFG_attach(id, decid, 3);
				id = decid;
				decimator /= 2;
			}

			DecFilt *dflt = getdecfilt(decimator);	// get a decimation filter
			if (dflt == NULL)
				return (-1);

			// open a decimator
			int decid = DECM_open(dflt->filt, dflt->len, decimator, a.nch);
			// patch the audio chain through the decimator
			err = CFG_attach(id, decid, 3);
			id = decid;

			wm.fs = a.fs / systemConfig.decimator;
		}

		if(systemConfig.compressionMode  > 0) {	// if a compression is requested
			// open a loss-less compressor
			int cmpid = X3_open(a.nch,16,1) ;
			err |= CFG_attach(id, cmpid, 6);
			id = cmpid;
		}


		// get a cfg id for the output job
		saveid = CFG_register((JOB_Fxn) recordjob, NULL, NULL);
		err |= CFG_attach( id  , saveid, 2);

		// setup the metadata
		INFO_new(saveid, FTYPE_WAV, id, id);
		INFO_addwavmeta(saveid, &wm);
		INFO_end(saveid);	// post the metadata

	} //if dec > 0

    // initialize the click detector
    if(systemConfig.detect1 > 0) {
    	bwdetparams.nblank = (int)(a.fs* systemConfig.detect4 / 1000000 );        // 5ms blanking time
    	bwdetparams.rthresh =  pow( 10.0, (float)systemConfig.detect2 / 10.0); //(25); // 14dB detection threshold (10^(14/10)), was 40 (10^(16/10))
    	bwdetparams.npre  = (int)(a.fs*systemConfig.detect5 / 1000000);      // 0.75ms pre-trigger
    	bwdetparams.npost = (int)(a.fs*systemConfig.detect6 / 1000000);      // 0.75ms post-trigger
    	bwdetparams.npwr = (int)(a.fs*systemConfig.detect3 / 1000000);		// 200 us power summing
    	bwdetparams.storeout = 0;						// don't store filter output
    	bwdetparams.minnp = (int)(2);		 		// Minimum noise power

    	detid = BWDET_open(a.fs, &bwdetparams);
    }
    return (err);
}


int APP_flush()
{
	if(tempLogId != 0) SenFlush(tempLogId);
	if(accelLogId != 0) SenFlush(accelLogId);
	if(detid != 0)	BWDET_flush();
#ifdef PLOG
	if(pLogId1 != 0) SenFlush(pLogId1);
	if(pLogId2 != 0) SenFlush(pLogId2);
#endif
	gpsLoggerFlush();
    return (OK);
}

//called when a flash file is about to be re-opened
void fileClosingEventHandler(void)
{
	APP_flush();
}

int APP_close(Ptr s, Ptr d)
{
	stopjob();
    return (OK);
}

int	JOB_MemCardFull(Ptr s, Ptr d)
{
	restartOnExit = (hidMemCardCount > 1);
 	return(FAIL) ;
}

int	JOB_MemCardNearFull(Ptr s, Ptr d)
{
	restartOnExit = (hidMemCardCount > 1);
 	return(FAIL) ;
}

int APP_init(void)
{
    int err, i;
    Uint16 audioHardwareId;

    nChan = 0;
    for(i=0; i<MAX_CHANNELS; i++)
    	if(systemConfig.channelEnable & (0x0001 << i)) nChan++;

    if( nChan > 1) systemConfig.detect1 = 0; //disable detector for multi channel

    mspifwatchdog(1) ;			// keep the watchdog happy while we are initializing

	err = getAudioHardwareId(&audioHardwareId);

	if(err == OK) {
		if(hid == ST4300)
			setsysclk(73728000);
		else{
			switch(audioHardwareId){
				case 1:
					setsysclk(36864000);  	//ST-HF
					break;
				default:
					setsysclk(18432000);	//ST-STD
			}
		}
    }


#ifndef PLOG
	if(hid == ST4300)
		err |= init_chans(systemConfig.channelEnable, audioHardwareId ? AUD_DIV_128 : AUD_DIV_256);
	else {
		err |= init_chans(0x0001, AUD_DIV_64); //single channel
	}
#endif


    if(systemConfig.auxSensorEnable & ACCEL_SENSOR) {
    	accelWakeup(); //TODO - put it to sleep between readings? Currently adds 100 uA
    	accelLogId = SEN_open("accel", systemConfig.auxSensorInterval*1000, accelTrigger, accelInit, "unix time, X, Y, Z");
    }


    if(systemConfig.auxSensorEnable & TEMP_SENSOR) {
    	tempLogId = SEN_open("temp", systemConfig.auxSensorInterval*1000, tempLogTrigger, tempLogInit, "unix time, degrees C");
	}

    if(systemConfig.serialLogMode) {
    	gpsLoggerInit(audid, systemConfig.decimator );
    }


#ifdef PLOG
    pLogId1 = SEN_open("pressure1", 500 , adcLoggerTrigger, adcLoggerInit, "unix time, CH1 (cm H20), CH2 (cm H20), CH3 (cm H20), CH4 (cm H20)");
    pLogId2 = SEN_open("pressure2", 500 , adcLoggerTrigger, adcLoggerInit, "unix time, CH5 (cm H20), CH6 (cm H20), CH7 (cm H20), CH8 (cm H20)");
    adcLoggerSetScale(0.0264);
#endif
	// start timed jobs
    err |= TIMR_doevery(3,(JOB_Fxn)watchdog,NULL);	// exercise the watchdog so that the MSP doesn't shut us down
#ifndef PLOG
    err |= TIMR_doin(1,(JOB_Fxn)startjob,NULL);
#endif

	// when flash is full, shutdown immediately
	FLSH_attach(FLSH_END, JOB_MemCardNearFull, NULL,MAXPRI) ;
	// when flash is nearly full, shutdown gracefully
	FLSH_attach(FLSH_NEAREND,JOB_MemCardFull, NULL,MAXPRI) ;
   	mspifwatchdog(0) ;

   	flsh_regFileClosingCallbackHandler(fileClosingEventHandler);

    uartInitial();

   	return (err);
}



