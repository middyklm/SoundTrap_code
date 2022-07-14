// D3-API v1.0
// Copyright (C) 2008-2010, Mark Johnson / WHOI
//
// This file is part of D3, a real-time patch panel scheduler
// for digital signal processors.
//
// D3 is free software: you can redistribute it 
// and/or modify it under the terms of the GNU General Public License 
// as published by the Free Software Foundation, either version 3 of 
// the License, or any later version.
//
// D3 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with D3. If not, see <http://www.gnu.org/licenses/>.

/*! \file filt.h
    \brief Multi-channel FIR filter.
    
    Filter processor for single or multi-channel data. For multi-channel
    data, the same filter is applied to all channels. The filter
    coefficients can be changed on the fly provided that the length
    of the filter remains the same. The actual filter implementation is
    in an assembly language function in file filtmc_asm.s55.
*/

typedef int DATA;

// error messages	
#define		FILT_ALLOCFAIL	(1)
#define		FILT_NOFILT		(2)
 
// create an instance of the filter for nch channels.
// filt is a pointer to a set of integer filter coefficients.
extern int		FILT_open(DATA *filt, int nfilt, int nch) ;

// disable and free a filter
extern int		FILT_close(int id) ;

// attach a job to the output of the filter - everytime an
// output buffer is generated, attached jobs will be spawned with the 
// data in a DATA_Obj.
extern int		FILT_attach(int id, int downstr_id, int nice) ;

// detach a job from the filter
extern void		FILT_remove(int id, int downstr_id) ;

// prototype for the filter data processor. This should not be
// called directly. It will be called when an instance of the
// filter is attached to a data source.
extern int		FILT_proc(Ptr p, DATA_Obj *d) ;

// prototype for the filter data processor when called from another
// function instead of from the JOB queue. An output data object
// will be created and returned in dout but with no metadata. No 
// downstream jobs will be run. 
extern DATA_Obj *FILT_embedproc(Ptr pt, DATA_Obj *din) ;

// post metadata for an instance of the filter
extern int		FILT_meta(int id) ;

// get overload status of an instance of the filter
extern int		FILT_status(int id) ;

// error handler - usually only for internal use
extern void	   FILT_error(int id) ;
