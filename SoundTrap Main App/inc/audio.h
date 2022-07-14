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

/*! \file audio.h
    \brief High-level audio ADC interface driver.
    
    Use this driver to access data streams from the audio analog-to-digital
    convertors.
*/

#ifndef AUDIO_H_
#define AUDIO_H_

#define MAX_CHANNELS 4

// Error codes in audio.c
#define	BADCHANNEL			(1)	//!<  invalid audio channel specified
#define	CHANNELINUSE		(2)	//!<  audio channel is already in use
#define	AUDIO1MUSTBEMASTER 	(3)	//!<  audio channel 1 must be a master
#define	CHANMUSTBERUNNING 	(4)	//!<  channel must be running for this command
#define	DATAALLOCFAIL 		(5)	//!<  unable to allocate buffer during interrupt
#define	OPENALLOCFAIL 		(6)	//!<  unable to allocate buffer during initialization

// Errors in audio_if.c
#define ADCPORTOPENFAIL     (0x105) //!< cannot open MCBSP port in adc_init
#define BANDINVALID         (0x106) //!< requested band is undefined
#define CPUCLKTOOSLOW       (0x107) //!< cpu clock frequency is too slow for the requested band
#define NBITSTOOBIG         (0x108) // 
#define BSPFAILOPEN         (0x109) // 
#define DMAFAILOPEN         (0x110) // 

// real-time errors reported via audio_reperr
#define	ENDPROG_ERR		(1)
#define	DROP_ERR		(2)
#define	OVERRUN_ERR		(3)
#define	DROP2_ERR		(4)
#define	BAD_BUF_SIZE_ERR (5)
/*! size of audio buffer on each channel in words.
    For optimum performance, match this to a memory block size defined in dmem.c */
#define		AUDIO_BUFFLEN	(BIGBUFFSIZE)

/*! maximum number of audio channels supported. Change this if your hardware
    supports more and add additional channel bit map definitions */
#define	    AUDIO_NCHANS	(3)

#define		AUDIO1		(1)		//!< bitmap for audio channel 1
#define		AUDIO2		(2)		//!< bitmap for audio channel 2
#define		AUDIO3		(4)		//!< bitmap for audio channel 3

/*! pre-defined frequency bands to use with AUDIO_open() */
#define		AUD_DIV_64			(0)			//!< high frequency acquisition (1024 kHz)
#define		AUD_DIV_128			(1)			//!< high frequency acquisition (512 kHz)
#define		AUD_DIV_256			(2)			//!< medium frequency acquisition (256 kHz)
#define		AUD_DIV_512			(3)			//!< medium frequency acquisition (128 kHz)
#define		AUD_DIV_1024		(4)			//!< low frequency acquisition (64 kHz)
#define		AUD_DIV_2048		(5)			//!< low frequency acquisition (32 kHz)

/*! gain setting options to use with AUDIO_gain() */
// These definitions match the sense of the GAIN pin on the audio
// board for the given gain setting
#define		LOWGAIN		(0)
#define		HIGHGAIN	(1)

#define		CHAN_CNT(c)	(_count(c,0xf))

/*! Structure to pass to AUDIO_stat to obtain the current status of an
    instance of the audio driver. Note that members buffcnt and errcnt
    are reset to zero after AUDIO_stat is called */
typedef struct {
		ulong	fs ;         //!< Sampling rate in kHz
		uns		state ;      //!< Current state e.g., open, running
		ulong	buffcnt ;    //!< Number of buffers handled by the driver (wraps at 65536)
		uns		errcnt ;     //!< Number of errors detected by the driver (wraps at 65536)
		uns		chans ;      //!< Bitmap of the channels accessed by the instance
		uns		nch ;      //!< number of channels
		} AUDIO_Stat ;


typedef struct  {
	Uint32 startFrequency;
	int toneCount;
	int tonePeriod;
	int repeatForBothGainSettings;
	int noiseFloorPeriod;
} TcalParams;


/*! \brief Opens one or more audio channels.

    \param chans Bitmap of the channels to be opened. 
    Use the channel values AUDIO1, AUDIO2, etc defined in audio.h
    Where the hardware permits, combinations of channel can be opened 
    e.g. AUDIO1 | AUDIO3. Data from these channels will be treated as
    multi-channel data. Only certain combinations may be available on
    any given hardware. 
    \param band The frequency band of the channel. This controls the
    sampling rate used and must be chosen from the values defined in
    audio.h e.g., MF, LF1. Make sure that the audio hardware you are
    using supports the band you want to open.
    \return The configuration id or handle for the channels. This
    will be used to attach data processing modules to the audio data
    source.
*/

extern	int AUDIO_open(int channel, int band);

/*! \brief Close audio channels.

    \param id The configuration id for the channels to close. This
    must be the id issued by a previous call to AUDIO_open.
    \return OK if the channels were closed without problem, FAIL otherwise.
*/

extern	int	AUDIO_close(int id) ;

/*! \brief Prepare a channel for converting.

    \param id The configuration id for the channels to start. This
    must be the id issued by a previous call to AUDIO_open.
    \return OK if the channel was preped without problem, FAIL otherwise.
*/

int AUDIO_Prep(int id);



void AUDIO_Start();


/*! \brief Stop a channel or channel group converting.

    Conversion will be paused and can be restarted again using AUDIO_start.
    \param id The configuration id for the channels to stop. This
    must be the id issued by a previous call to AUDIO_open.
    \return OK if the channels were stopped without problem, FAIL otherwise.
*/

extern	int	AUDIO_stop(int id) ;

/*! \brief Report the status of a channel or channel group converting.

    \param id The configuration id for the channels to query. This
    must be the id issued by a previous call to AUDIO_open.
    \param s The status information will be written to the AUDIO_Stat
    structure pointed to by s. The structure should be allocated by the
    calling function.
    \return OK.
*/

extern	int	AUDIO_status(int id, AUDIO_Stat *s) ;

/*! \brief Post metadata for a channel or channel group.

    The metadata will be written to the current FLASH file (see flsh.h).
    This function is called automatically when a channel is opened and
    whenever a new FLASH file is opened. It should not need to be called
    by the user.
    \param id The configuration id for the channels. This
    must be the id issued by a previous call to AUDIO_open.
    \return OK if the metadata was posted without problem, FAIL otherwise.
*/

extern  int	AUDIO_meta(int id) ;

#ifdef _JOB
/*! \brief Attach a job to an audio stream.

    This function should not be called by the user. Use CFG_attach
    to patch processors.
    \param id The configuration id for the audio channels that will be
    the data source. This must be the id issued by a previous call to 
    AUDIO_open.
    \param downstr The configuration id for the downstream data processing
    function. This must be the id issued by a previous call to the open
    function of the data processor. This processor will be scheduled by the
    AUDIO module whenever there is data enough to pass.
    \param nice The nice number to be used when posting the downstream job.
    \return OK if the processes were patched without problem, FAIL otherwise.
*/

 extern	int   AUDIO_attach(int id, int downstr, int nice) ;

/*! \brief Remove a job from an audio stream.

    This function should not be called by the user. Use CFG_remove
    to un-patch processors.
    \param id The configuration id for the audio channels that will be
    the data source. This must be the id issued by a previous call to 
    AUDIO_open.
    \return OK if the processes were unpatched without problem, FAIL otherwise.
    \todo The process of removing patches is not yet complete. It ought to
    work through CFG_remove but there are some problems still.
*/

 extern int   AUDIO_remove(int id, int downstr) ;
#endif

/*! \brief Turn mute mode on or off.

    The preamplifier input is disconnected from the hydophone/microphone in
    mute mode and tied to ground. This is useful for measuring the self noise
    of the system. All audio channels, open or not, are muted by this function.
    Muting is not supported by all hardware and hydrophones/microphones.
    \param on Mute the audio channels if 1. Remove the mute if 0.
    \return OK if the operation was performed without problem, FAIL otherwise.
*/
 extern	 int AUDIO_mute(int on);

/*! \brief Change the gain setting of audio channels.

    Changes the gain of any combination of currently open channels. There
    are usually only two gain choices: HIGHGAIN gives the widest dynamic
    range and lowest noise floor. LOWGAIN gives the highest clipping level.
    \param chans Bitmap of audio channels for gain setting. 
    Use the channel values AUDIO1, AUDIO2, etc defined in audio.h
    The gain of any combination of channels can be changed in a single
    call e.g. AUDIO1 | AUDIO3. The channels do not have to match sets of 
    channels opened together by a call to AUDIO_open.
    \param opt Gain setting. Use the defined values in audio.h e.g., LOWGAIN.
    \return OK if the operation was performed without problem, FAIL otherwise.
*/

extern	int	AUDIO_gain(int chans, int opt) ;


/*! \brief Get the sampling rate associated with an audio band.

    This function reports the sampling rate that is used in a given standard
    audio band. An audio channel does not have to be open before using this 
    function. The definition of this function is in audio_if.c.
    \param band One of the defined band values in audio.h e.g., MF.
    \return the sampling rate in Hz or -1 if the band is invalid.
*/
extern long	AUDIO_getfs(uns band) ;

int AUDIO_start_cal(TcalParams params);
int AUDIO_start_full_cal();
int AUDIO_hpass(int chans, int opt);
ulong GetSampleCount(Ptr ah);


#endif
