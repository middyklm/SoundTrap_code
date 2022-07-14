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

//	FLSH module
//	file-based FLASH memory interface


// error codes
#define	FILEINUSE			(1)	// file is in use and cannot be closed
#define	NOSEEKONWRITE		(2)	// seek not allowed on a file opened for writing
#define	BADSEEKDIR			(3)	// unknown direction for seeking
#define	BADSEEKSIZE			(4)	// incompatible size for seek
#define	ALLOCFAIL			(5)	// memory allocation fail for file structure
#define	NOTCLEARFORWRITE	(6)	// memory segment is not erased - writing is not possible
#define	WRITESPACETOOSMALL	(7)	// requested file size is too large.
#define	SEEKTOOFAR			(8)	// absolute seek size is too large
#define	BADFILESTATE		(9)	// wrong state at entry to FLSH_open()
#define	FLSHDMAFAILOPEN		(0x101)	// flsh_if
#define	MEMORYFULL			(0x102)	// flsh_if
#define	NTOGONEG			(0x103)	// flsh_if
#define	PAGELOCKERR			(0x104)	// flsh_if
#define	NOBADBLOCKTABLE		(0x201)	// flsh_utils

// types of data in the buffer
#define	WRITE_NONE			(0)		// no data
#define	WRITE_CH			(1)		// chunk header
#define	WRITE_CD			(2)		// chunk data
#define	WRITE_CRC			(3)		// checksum
#define	WRITE_FLUSH			(4)		// no data - flush the buffer (changed from 3)
#define	WRITE_NEWBLK		(5)

// maximum file size in bytes
#define	FLSH_MAXFILESIZE	(1000000000l)

// maximum number of samples in a file (to avoid overflow in wav files).
// This limit is needed despite the limit on the total file size above because
// the data compression may be too effective and generate a FLSH_MAXFILESIZE
// file that cannot be represented when unpacked by a single wav file. This
// number assumes that we are dealing with 16 bit samples.
#define	FLSH_MAXSAMPLES		(1036800000l) //1 hr at fs = 288k

// maximum number of bytes per callback on read
#define FLSH_MAXREAD	(400)

// nice number for callbacks associated with write postings
#define	FLSH_NICE		(MAXNICE-3)

// special events for callbacks
#define	FLSH_END		(1)		// end of memory
#define	FLSH_NEAREND	(2)		// last block of memory

#define	CHUNK_MAGIC		(0xA952)
#define	BLOCK_MAGIC		(0x5ABC)

// initialize FLSH file system, inspect memory and make a directory
extern	int	FLSH_init(void) ;

// open a file for writing at the next available location
extern	int	FLSH_open(void) ;

// re-open a file for writing at the next available location (block).
// This starts a new 'recording' with configuration data etc. without
// interrupting the data flow
extern	int	FLSH_reopen(char *reason) ;

// close a file - if wait=1, the function returns when the
// write queue is empty. If wait=-1, the write queue is
// flushed.
extern  int	FLSH_close(int	wait) ;

extern 	int	FLSH_status(void) ;
extern  void flsh_stop(int signal);

// functions with callbacks
#ifdef _JOB
 // write data to flash with callback
 extern	int	FLSH_write(DATA_Obj *d, JOB_Fxn f, Ptr s) ;

 // write data to flash without callback
 #define FLSH_save(d)		(FLSH_write(d,(JOB_Fxn)NULL,NULL))

 // attach jobs to special events
 extern int		FLSH_attach(int event, JOB_Fxn f, Ptr s, int nice) ;
 extern void	FLSH_remove(int event, JOB_Fxn f) ;
#endif

typedef struct {
				uns	*p ;			// pointer to the information to be written
				int	n ;			// number of words to be written
				int	type ;		// does the buffer point to header or data words?
				} FLSH_Buff ;

typedef  void (*FlashCallbackHandler)(void);
void flsh_regNewFileCallbackHandler( FlashCallbackHandler cb );
void flsh_regFileClosingCallbackHandler( FlashCallbackHandler cb );



