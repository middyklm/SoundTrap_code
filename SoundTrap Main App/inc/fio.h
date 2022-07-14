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

// FIO module - file input and output data source/sink for testing
// NOTE: this module assumes that the JTAG is attached and that JTAG
// is defined.
// FIO.h v1.0


#define		FIO_NICE		(MAXNICE-1)

// error messages	
#define		FIO_ALLOCFAIL	(1)
#define		FIO_OPENFAIL	(2)
#define		FIO_WRITEFAIL	(3)
#define		FIO_READFAIL	(4)

// create a file data source/sink with filename fname.
// If ntoread>0, create a data source that will read
// ntoread 16-bit integers from binary file fname, pass
// them to any connected processors and then re-post
// itself. When the end of file is reached, a message
// will be posted to the screen and the module will idle.
// If ntoread==0, a data sink module is created which
// writes any incoming data as 16 bit integers to the
// binary file fname.
extern int		FIO_open(char *fname, int ntoread, int nchs) ;

// close the file
extern int		FIO_close(int id) ;

// attach a job to the output of a data source. Attached jobs 
// will be spawned continuously with data in a DATA_Obj.
extern int		FIO_attach(int id, int downstr_id, int nice) ;

// detach a job from a data source
extern void		FIO_remove(int id, int downstr_id) ;

// prototype for data processor to attach to a data source module
extern int		FIO_proc(Ptr p, DATA_Obj *d) ;

// start a data source file i/o going
extern int		FIO_start(int id) ;

// post metadata for an instance
extern int		FIO_meta(int id) ;
