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

#define	_PSTRING

// amount to augment the buffer in pstr_add if it is too small
#define	PSTR_LENINC		(50)

typedef	struct {
				uns	*b ;			// pointer to the packed string data
				int	nby ;			// number of bytes in the packed string
				int	ntogo ;		// number of words left in the buffer
				} PSTRING ;

#define	pstr_size(p)			(((p)->nby+1)>>1)

extern void	pstr_init(PSTRING *p, uns *buff, int len) ;
extern int	pstr_cat(PSTRING *p, char *s) ;
extern int	pstr_cpy(PSTRING *dest, PSTRING *src) ;
extern int	pstr_pncat(PSTRING *p, uns *s, int n) ;

// cat with buffer re-allocation if necessary
extern int	pstr_add(PSTRING *p, char *s) ;

extern void	pstr_print(PSTRING *p) ;
