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

/*! \file audio.c
 \brief AUDIO module - Low-level audio ADC interface driver.
 */


#include "math.h"
#include "d3defs.h"
#include "d3std.h"
#include "protect.h"
#include "board.h"
#include "devdep.h"
#include "misc.h"
#include "modnumbers.h"
#include "dmem.h"
#include "job.h"
#include "data.h"
#include "cfg.h"
#include "info.h"
#include "audio.h"
#include "audio_if.h"
#include "timr.h"
#include "error.h"
#include "audioOut.h"
#include "config.h"
#include "gpio.h"


// states that each audio channel can be in
#define		AUDIO_CLOSED		(0)
#define		AUDIO_OPEN			(1)
#define		AUDIO_RUNNING		(2)

// structure of information about each channel
typedef struct
{
    DATA_Obj *next;		// pointer to next data_obj to be loaded
    JOB_List jlist;		// job list attached to the channel
    DATA_Meta meta;		// metadata to fill in each DATA_Obj
    uns state;		// current state of the interface
    ulong buffcnt;	// number of full buffers received
    ulong errcnt;	// number of error instances
    uns band;		// band associated with these channels
    int nch;		// number of channels
    int channels;	// channel bitmap
    int bufflen;	// length of buffer to allocate - must be an
                    // integer multiple of nch
} AUDIO_Obj;

// band names - order must match definitions in audio.h
static char *ABANDS[] = { "HF1", "HF2", "MF1", "MF2", "LF1", "LF2" };


// bitmap lock for audio channels
static int chanlock = 0;		// all channels unlocked at startup

// local functions
void audio_logevent(AUDIO_Obj *ah, int state);


int audioChanOn(int channels)
{
	int chan;
	for(chan=0; chan < MAX_CHANNELS; chan++)
	{
		if(channels & (0x0001 << chan)) {
			audiopwr(chan, 1);
		}
	}
	return OK;
}

int audio_chansoff(int channels)
{
	int chan;
	for(chan=0; chan < MAX_CHANNELS; chan++)
	{
		if(channels & (0x0001 << chan)) {
			audiopwr(chan, 0);
		}
	}
	return OK;
}

void audio_reportgains(int channels, int opt)
{
    char s[20];

    if(opt)
    	snprintf(s, 20, "Gain=\"High\"");
    else
        snprintf(s, 20, "Gain=\"Low\"");

    INFO_event("AUDIO", s , NULL);
}

void audio_reportHPass(int channels, int opt)
{
    char s[30];
    if(opt)
    	snprintf(s, 30, "HighPass=\"ON\"");
    else
       snprintf(s, 30, "HighPass=\"OFF\"");

    INFO_event("AUDIO", s , NULL);
}

int AUDIO_mute(int on)
{
    char s[20];
    // report the event
    snprintf(s, 20, "MUTE=\"%d\"", on);
    INFO_event("AUDIO", s, NULL);
    return (mute(on));
}

/* open an audio channel */

int AUDIO_open(int channels, int band)
{
    AUDIO_Obj *ah;
    int nsamps;
    int c;
    int nChan = 0;

    for(c=0; c<MAX_CHANNELS; c++)
    	if(channels & (0x0001 << c)) nChan++;

    // catch the first time through to do initialization
    if (chanlock == 0)
        audio_init();

    // allocate a structure for the audio object
    if ((ah = (AUDIO_Obj *) DMEM_alloc(sizeof(AUDIO_Obj))) == NULL) {
        err(AUDIO_MOD, OPENALLOCFAIL);
        return (NULLID);
    }

    // initialize the AUDIO_Obj structure
    ah->state = AUDIO_OPEN;
    ah->buffcnt = ah->errcnt = 0;
    ah->jlist = NULLJOB;
    ah->band = band;
    ah->channels = channels;
    ah->nch = nChan;
    // the buffers should be equal to the largest multiple of n less than AUDIO_BUFFLEN
    ah->bufflen = AUDIO_BUFFLEN;
    nsamps = AUDIO_BUFFLEN/ah->nch;
    nsamps &=  ~0x0001; //must be even

    // initialize the underlying audio channels
    if (audio_openchans(channels, nChan, band, ah) == FAIL) {
        DMEM_free(ah);
        return (NULLID);
    }

    // initialize metadata
    ah->meta.fs = AUDIO_getfs(band);
    ah->meta.size = ah->bufflen;
    ah->meta.nsamps = nsamps;
    ah->meta.nbits = 16;
    ah->meta.nch = ah->nch;
    ah->meta.id = CFG_register(NULL, AUDIO_attach, (Ptr) ah);

    chanlock = 1;
    return (ah->meta.id);
}

int AUDIO_close(int id)
{
    AUDIO_Obj *ah = (AUDIO_Obj *) CFG_getstate(id);

    // if the channel is still running, stop it first
    if (ah->state == AUDIO_RUNNING)
        AUDIO_stop(id);

    audio_close();
    JOB_freelist(ah->jlist);

    // turn off power to the channels on the audio board
    // now no need
    //audio_chansoff(chans) ;

    //chanlock &= ~chans;
    DMEM_free(ah);
    return (OK);
}

int AUDIO_attach(int id, int downstr, int nice)
{
    AUDIO_Obj *ah = (AUDIO_Obj *) CFG_getstate(id);

    AUDIO_meta(id);
    return (JOB_add(&(ah->jlist), CFG_getprocfxn(downstr),
            CFG_getstate(downstr), nice));
}

int AUDIO_remove(int id, int downstr)
{
    AUDIO_Obj *ah = (AUDIO_Obj *) CFG_getstate(id);

    // remove a job from an audio stream
    JOB_remove(&(ah->jlist), CFG_getprocfxn(downstr));
    return (OK);
}

int AUDIO_startDaq()
{
	TcalParams p;
	p.startFrequency = 1000;
	p.tonePeriod = 300;
	p.repeatForBothGainSettings = 0;
	p.noiseFloorPeriod = 250;
	p.toneCount = 7;
    if(  systemConfig.flags && CONFIG_FLAGS_DISABLE_CAL_ROUTINE  ) {
    	//calibration disabled, just start recording
    	audioOutEnableCal(0);
   		AUDIO_mute(0);
   		audio_start();
    }
    else {
   		AUDIO_mute(0);
    	audio_start();
       	AUDIO_start_cal(p);
    }
    return OK;
}



void AUDIO_Start()
{
	//audio_start();
	AUDIO_startDaq();
}

int AUDIO_Prep(int id)
{
    AUDIO_Obj *ah = (AUDIO_Obj *) CFG_getstate(id);

    if (ah->state == AUDIO_RUNNING)
        return (OK);

    audioChanOn(ah->channels);			// turn on the audio hardware

    ah->state = AUDIO_RUNNING;
    ah->buffcnt = ah->errcnt = 0;

    ah->next = NULLDATA;

    audio_logevent(ah, 1);                          // record the event

    return (OK);
}

int AUDIO_stop(int id)
{
    AUDIO_Obj *ah = (AUDIO_Obj *) CFG_getstate(id);

    if (ah->state != AUDIO_RUNNING)			// channel is already stopped
        return (OK);

    audio_stop();		    // stop audio acquisition
    audio_chansoff(ah->channels);			// turn off the audio hardware

    if (ah->next != NULLDATA )
        DATA_free(ah->next);

    ah->next = NULLDATA;
    ah->state = AUDIO_OPEN;		// change state
    audio_logevent(ah, 0);			// record the event
    return (OK);
}

int AUDIO_status(int id, AUDIO_Stat *s)
{
    AUDIO_Obj *ah = (AUDIO_Obj *) CFG_getstate(id);

    s->state = ah->state;
    s->fs = ah->meta.fs;
    s->buffcnt = ah->buffcnt;
    s->errcnt = ah->errcnt;
    s->chans = ah->channels;
    s->nch = ah->nch;
    return (OK);
}

int AUDIO_meta(int id)
{
    char s[20];
    AUDIO_Obj *ah = (AUDIO_Obj *) CFG_getstate(id);

    // setup the metadata
    INFO_new(id, NULL, NULLID, NULLID);
    INFO_add(id, "PROC", NULL, "AUDIO");
    snprintf(s, 20, "%d", ah->nch);		// report channel bit map
    INFO_add(id, "NCHS", NULL, s);
    INFO_add(id, "BAND", NULL, ABANDS[ah->band]);	// report the band
    snprintf(s, 20, "%ld", ah->meta.fs);		// report the sampling rate in Hz
    INFO_add(id, "FS", "UNIT=\"Hz\"", s);
    snprintf(s, 20, "%d", ah->meta.nbits);	// report number of bits
    INFO_add(id, "NBITS", NULL, s);
    INFO_end(id);						// post the metadata
    //audio_reportgains(ah->channel);		// report the gain of each channel
    return (OK);
}





void onCalDone()
{
}

TcalParams calParams;

enum eCalState {CAL_STATE_IDLE, CAL_STATE_PLAY_TONES, CAL_STATE_MUTE, CAL_STATE_ONDONE};
enum eCalState calState;

void AUDIO_calRunStateMachine()
{
    static int toneNo;
    Uint16 delayMs = 0;
    switch(calState) {
        case(CAL_STATE_IDLE):
            AUDIO_mute(1);
            toneNo = 0;
            delayMs = 100;
            calState++;
            break;
        case(CAL_STATE_PLAY_TONES):
            if( toneNo <  calParams.toneCount ) {
                audioOutEnableCal(0);
                audioOutsetCalTone( calParams.startFrequency * pow(2, toneNo));
                audioOutEnableCal(1);
                delayMs = calParams.tonePeriod;
                toneNo++;
            }
            else {
            	audioOutEnableCal(0);
            	delayMs = calParams.noiseFloorPeriod;
                calState++;
            }
            break;

        case(CAL_STATE_MUTE):
			delayMs = calParams.noiseFloorPeriod;
            calState++;
            break;

        case(CAL_STATE_ONDONE):
       		AUDIO_mute(0);
			onCalDone();
			calState = CAL_STATE_IDLE;
            break;

    }
    if(delayMs) TIMR_DO_IN_MS(delayMs,(JOB_Fxn)AUDIO_calRunStateMachine,NULL);
}


int AUDIO_start_cal( TcalParams params )
{
    char s[20];
    if( calState == CAL_STATE_IDLE ) {
		snprintf(s, 20, "CAL=\"1\"");
		INFO_event("AUDIO", s, NULL);
		calState = CAL_STATE_IDLE;
		calParams = params;
		AUDIO_calRunStateMachine();
    }
    return OK;
}


TcalParams p;
int AUDIO_start_full_cal()
{
	p.startFrequency = 1000;
	p.toneCount = 8;
	p.tonePeriod = 100;
	p.repeatForBothGainSettings = 1;
	p.noiseFloorPeriod = 1000;
	AUDIO_start_cal(p);
	return OK;
}

int AUDIO_gain(int channels, int opt)
{
    audio_reportgains(channels, opt);
    return (setgain(channels, opt));
}

int AUDIO_hpass(int channels, int opt)
{
    audio_reportHPass(channels, opt);
    return (setHPass(channels, opt));
}

void audio_reperr(int chan, int count, int etype, Ptr ah)
{
    AUDIO_Obj *ap = (AUDIO_Obj *) ah;
    char s[50];
    snprintf(s, 50, "WARNING=\"%d\" CHAN=\"%d\" CNT=\"%d\" BUF=\"%lu\"", etype, chan, count, ap->buffcnt);
#ifdef JTAG
    printf(s);
    exit(1);
#else
    INFO_event("AUDIO", s, NULL);
#endif
}


ulong GetSampleCount(Ptr ah)
{
	ulong secs, us;
	ulong et;

	if (ah == NULL) return 0;

	AUDIO_Obj *s = (AUDIO_Obj *) ah;

	if(s->buffcnt == 0) return 0;

	TIMR_gettime(&secs, &us);
	et = us - s->next->mticks;
	et += (secs - s->next->rtime) * 1000000;
	return  ((ulong)s->buffcnt * s->bufflen) + ((et *  s->meta.fs) / 1000000);
}


// private functions
void audio_run(Ptr ah)
{
	static int drop = 0;
	AUDIO_Obj *s = (AUDIO_Obj *) ah;
    DATA_Obj *prev, *d;
    int br;

    prev = s->next;

    //check how many buffers remaining, and throttle data to avoid downstream logjam
    br = DMEM_blocks_remaining();
    if(drop) {
    	if(br > 30) drop = 0;
    }
    else {
    	if(br < 10) drop = 1;
    }

    if (!drop && (d = DATA_alloc(s->bufflen)) != NULLDATA) {// get a new empty buffer
		++(s->buffcnt);	// increment buffer count
        s->next = d;
        TIMR_gettime(&(s->next->rtime), &(s->next->mticks)); //TODO - this is time at end of frame - fix
        audio_nextbuff(s->next->p, s->meta.nsamps); // notify the driver of the new data buffer
    }
    else
    {
    	// there will be a gap in the data flow
    	s->errcnt++;
        s->next = NULLDATA;
   	}

    // handle the old data buffer (outgoing data)
    if (prev != NULLDATA ) {			// if there is some data to post...
        // fill out the metadata in the DATA Obj
        memcpy(&(prev->fs), &(s->meta), sizeof(DATA_Meta));
        prev->sema = 0;	// note: the semaphore gets overwritten in the memcpy
        // post the jobs attached to this data source and set the
        // semaphore for the number of data sinks attached to this source
        if (s->jlist != NULLJOB )
            DATA_SETSEMA(prev,JOB_post(&(s->jlist),prev));
        else
            DATA_free(prev);
    }
}

// private utilities

void audio_logevent(AUDIO_Obj *ah, int state)
{
    // report a change in state of an audio channel
    char s[30];

    snprintf(s, 30, "RUN=\"%d\"", state);
    INFO_event("AUDIO", s, NULL);
}
