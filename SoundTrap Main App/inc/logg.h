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

// LOG module - buffer text messages and store in FLASH memory
// logg.h v1.0
// Note: name of the files is logg with double g to differentiate them
// from a log module in the TI CSL or RTS libraries.


// log formatting modes
#define		NOQUOTE		(0)
#define		USEQUOTE	(1)

// error messages	
#define		LOG_ALLOCFAIL	(1)
#define		LOG_BADMODE		(2)

// open a log file. It will generate a file on the host PC with
// the given suffix. The underlying file type is text.
// mode can be NOQUOTE or USEQUOTE to force the string to appear
// without or with surrounding quotes in the text file.
extern int	LOG_open(char *suffix, int mode) ;

// close the log file
extern int	LOG_close(int id) ;

// add an entry to the log file specifying the time of the entry
extern int	LOG_add(int id, ulong rtime, ulong mticks, char *mess) ;

// add an entry to the log file using the current time
extern int	LOG_diary(int id, char *mess) ;

// flush the log buffer to FLASH memory
extern int	LOG_flush(int id) ;
