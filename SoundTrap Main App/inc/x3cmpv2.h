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

/*! \file x3cmp.c
    \brief Multi-channel loss-less audio compressor.

   The X3 compressor is an implementation of a subset of the loss-less 
   compression algorithm developed by Robinson and implemented in GPL
   compression software Shorten and FLAC. See: Robinson T (1994) SHORTEN:
   Simple lossless and near-lossless waveform compression. Technical report 
   CUED/F-INFENG/TR.156, Cambridge University Engineering Department, 
   Cambridge, UK.

	Limitations: currently does not handle:
	1. channel muting
	2. double diff filter
	3. >16 bit data
*/

// errors
#define X3_INVALIDCODE	     (1)
#define X3_OBJALLOCFAIL      (2)
#define X3_DATAALLOCFAIL     (3)

#define X3_MAXBLKLEN    (64)
#define X3_MINBLKLEN    (4)
#define	X3_MAXCHS		(8)

// output buffer length
#define X3_OUTLEN		   (BIGBUFFSIZE)

#define	CODE_THR1		(3)
#define	CODE_THR2		(8)
#define	CODE_THR3		(20)

// create an instance of the compressor
// blklen must be >=4 and even
extern int		X3_open(int nchs, uns blklen, int consec);

// disable and free a compressor
extern int		X3_close(int id) ;

// attach a job to the output of the compressor - everytime X3_OUTLEN
// output samples are generated, attached jobs will be spawned with the 
// data in a DATA_Obj.
extern int		X3_attach(int id, int downstr_id, int nice) ;

// detach a job from the decimator
extern void		X3_remove(int id, int downstr_id) ;

// prototype for data processor to attach to a data source module
extern int		X3_proc(Ptr p, DATA_Obj *d) ;

// post metadata for an instance
extern int		X3_meta(int id) ;

// force a chunk flush. This is used when there is a discontinuity in the data stream
extern int		X3_flush(int id) ;
