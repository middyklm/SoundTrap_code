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

// SSIN module - swept-sinewave generator
// ssin.h v1.0


// Table-based stepped sinewave generator.
// Attach data processing or data sink modules to the SSIN module.
// Data will be generated in packets and passed to attached modules
// via DATA_Objs. After each data packet is produced, SSIN re-posts
// itself with a higher nice number than any of the attached modules.
// This means that it will not run again to generate more data until
// all of the attached modules have processed the current data.

// error numbers
#define	SSIN_ALLOCFAIL		(1)	// no memory available for structures in SSIN_open
#define	SSIN_NOSTEPS		(2)	// no steps specified
#define	SSIN_DATAFAIL		(3)	// data allocate fail in SSIN_proc

#define	SSIN_SYNC		(1)
#define	SSIN_ASYNC		(0)

// don't change the order in the following structure - the assembly language
// module scaledsine.s55 expects fr, level and offset at fixed positions in the
// structure.
typedef struct {
	DATA		fr ;			// sampling rate relative to sampling rate
	DATA		level ;		// level relative to full-scale
	DATA		offset ;		// dc offset
	long		nsamps ;		// number of samples to produce (0=no limit)
	long		pause ;		// number of samples to pause at end of tone
	} SSIN_Step ;

// data packet size in words that will be produced
#define	SSIN_BUFFLEN	(BIGBUFFSIZE)

// create an instance of the SSIN module 
extern int	SSIN_open(SSIN_Step *steps, int nsteps, int sync) ;

// close an instance of the SSIN module 
extern int	SSIN_close(int id) ;

// request the status of the SSIN module 
extern int	SSIN_status(int id) ;

// attach a job to the output of the SSIN generator. Attached jobs 
// will be spawned continuously with segments of signal as the data 
// payload in a DATA_Obj. 
extern int	SSIN_attach(int id, int downstr_id, int nice) ;

// detach a job from the SSIN generator
extern void	SSIN_remove(int id, int downstr_id) ;

// attach a callback job to the module. The attached job 
// will be spawned when the SSIN sequence is completed.
extern int	SSIN_whendone(int id, JOB_Fxn f, Ptr state, int nice) ;

// start the SSIN generator going. This will only work if there
// is at least one module attached. Attach all downstream modules
// and then call SSIN_start to initiate the flow of data.
extern int	SSIN_start(int id) ;

// post metadata for an instance
extern int	SSIN_meta(int id) ;
