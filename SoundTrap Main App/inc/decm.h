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

// DECM module - decimation (down-sampling) data processor
// decm.h v1.0


// output buffer length
#define		DECM_OUTLEN		(BIGBUFFSIZE)

// error codes
#define		DECM_PROCALLOC	(1)

// create an instance of the decimator with decimation factor df.
// filt is a pointer to a set of integer filter coefficients.
extern int		DECM_open(int *filt, int nfilt, uns df, int nchs) ;

// disable and free a decimator
extern int		DECM_close(int id) ;

// attach a job to the output of the decimator - everytime DECM_OUTLEN
// output samples are generated, attached jobs will be spawned with the 
// data in a DATA_Obj.
extern int		DECM_attach(int id, int downstr_id, int nice) ;

// detach a job from the decimator
extern void		DECM_remove(int id, int downstr_id) ;

// request and reset the overload state of a decimator
extern int		DECM_status(int id) ;

// prototype for data processor to attach to a data source module
extern int		DECM_proc(Ptr p, DATA_Obj *d) ;

// post metadata for an instance
extern int		DECM_meta(int id) ;
