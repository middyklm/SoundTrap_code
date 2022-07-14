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

//	D3 Module numbers

/*! \file modnumbers.h
    \brief Module numbers used for error reporting.

   To minimise the amount of code space required to store error messages,
   each software module in the D3 API is assigned a unique 16 bit number 
   which is used when that module reports an error. All module numbers 
   are reported in hexadecimal (i.e., h suffix). If you add a module, 
   choose an available number between 10h and 7ffeh. In general module 
   numbers between 0 and 0xFF are reserved for kernel modules. Numbers
   between 0x100 and 0xFFF should be used for data source and data sink 
   modules. Numbers above 0x1000 should be used for data processing modules.
*/

#define	DMEM_MOD	   (1)	      //!<  DMEM: dynamic memory management module
#define	JOB_MOD		(2)	      //!<  JOB: real-time scheduling module
#define	DATA_MOD		(3)	      //!<  DATA: wrapper functions for data capsules
#define	FMEM_MOD	   (4)	      //!<  FMEM: one-shot fast memory management module
#define	MAIN_MOD		(5)	      //!<  main entry point
#define	JOB_RUN_FAIL	(6)	      //!<  used for failed job return
#define BOARD_MOD	(0x0B)	   //!<  BOARD: board-specific functions 	
#define	IIC_MOD		(0x0C)	   //!<  IIC: i2c bus interface functions
#define	CFG_MOD   	(0x019)	   //!<  CFG: patch configuration module
#define	EMEM_MOD	   (0x1F)	   //!<  EMEM: external memory interface
#define	AUDIO_MOD	(0x100)		//!<  AUDIO: sound acquisition module
#define	TIMR_MOD		(0x200)	   //!<  TIMR: time keeping module
#define SCHED_MOD	(0x201)	   //!<  PTRK: pitch tracking algorithm
#define	CAL_MOD		(0x300)	   //!<  CAL: calibration signal generation
#define	EXTINT_MOD	(0x400)	   //!<  EXTINT: external input detection functions
#define	DMSENS_MOD	(0x500)	   //!<  DMSENS: IIC sensor acquisition	
#define	SENS_MOD		(0x510)	   //!<  SENS: DTAG-3 sensor acquisition
#define	GPSCAP_MOD		(0x520)	   //!<  SENS: DTAG-3 sensor acquisition
#define	SNR_MOD		(0x530)	   //!<  SENS: DTAG-3 sensor acquisition
#define	COMM_MOD	   (0x600)	   //!<  COMM: UART communications module	
#define FLSH_MOD		(0x700)	   //!<  FLSH: flash file system	
#define	NMEA_MOD		(0x1001)	   //!<  NMEA: parser for NMEA text messages
#define	DECM_MOD		(0x1010)	   //!<  DECM: multi-channel decimator	
#define	FILT_MOD		(0x1011)	   //!<  FILT: multi-channel filter
#define	CHSEL_MOD	(0x1012)    //!<  CHSEL: channel selector
#define	CDET_MOD		(0x1022)	   //!<  CDET: transient (click) detector
#define	CLPAR_MOD	(0x1023)    //!<  CLPAR: click parameter extractor
#define	BPWR_MOD		(0x1025)    //!<  BPWR: block-wise signal power estimator
#define	PGRAM_MOD	(0x1027)    //!<  PGRAM: fft-based periodogram
#define	KFILT_MOD	(0x1028)    //!<  KFILT: kernel filter for spectrograms
#define	SCOND_MOD	(0x1029)    //!<  SCOND: spectrum conditioner
#define	FIO_MOD		(0x1100)	   //!<  FIO: file input/output for debugging
#define	LOG_MOD		(0x1110)	   //!<  LOG: diary and text logging functions
#define	SEN_MOD		(0x1120)	   //!<  LOG: diary and text logging functions
#define	SSIN_MOD		(0x1200)	   //!<  SSIN: tone generation for calibration
#define	INFO_MOD	   (0x4019)	   //!<  INFO: metadata manager
#define	SAF_MOD		(0x401A)	   //!<  SAF: store-and-forward buffering
#define	CBUFF_MOD	(0x401B)	   //!<  CBUFF: external RAM circular buffering
#define X3_MOD		(0x4067)	   //!<  X3: loss-less audio compressor
#define VIT_MOD		(0x4081)	   //!<  VIT: Viterbi tracking algorithm
#define PTRK_MOD	(0x4082)	   //!<  PTRK: pitch tracking algorithm

// module number for user application code - add more numbers below 0x7fff
// if needed
#define	APP_MOD		(0x7fff)	   //!<  APP: user-defined application
