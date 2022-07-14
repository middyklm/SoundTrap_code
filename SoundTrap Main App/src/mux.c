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

/*! \file mux.c
    \brief multplex data from multiple channels into single stream.
*/

#include <math.h>
#include <string.h>
#include "d3defs.h"
#include "d3std.h"
#include "protect.h"
#include "board.h"
#include "devdep.h"
#include "misc.h"
#include "modnumbers.h"
#include "error.h"
#include "job.h"
#include "dmem.h"
#include "fmem.h"
#include "data.h"
#include "cfg.h"
#include "info.h"
#include "mux.h"
#include "minmax.h"


typedef struct {
	int	nchans ;
	JOB_Obj		*jlist;	// list of jobs attached for receive
	int			id;		// number assigned to this configuration
	int			src;		// source id number
} Mux_Obj ;



void muxReperr(int id)
{
	Mux_Obj *p = (Mux_Obj *)CFG_getstate(id) ;
    char s[40];
    snprintf(s, 40, "WARNING=\"UNKNOWN\"");
    INFO_event("MUX", s, NULL);
}

int	muxProc(Ptr pt, DATA_Obj *d)
{
	// called when there is data to work with
	int	nch, nin, i, c;
	int *pout, *pin;
	DATA_Obj	*dout;
	Mux_Obj	*p = (Mux_Obj *)pt ;

	if(p->jlist == NULLJOB) {		// no downstream jobs attached - nothing to do
		DATA_free(d) ;
		return(OK) ;
	}

	nin = d->nsamps;
	if( nin==0 ) {
		return(OK) ;
	}


    if(d->nch == 1) //nothing to do for single channel
    {
    	d->id = p->id;
    	DATA_POST(&(p->jlist),d) ;  // pass through
    	return(OK) ;
    }

	if((dout = (DATA_Obj *)DATA_alloc(BIGBUFFSIZE)) == NULLDATA) {
		muxReperr(p->id);
		DATA_free(d);
		return(OK);
	}

	// set the metadata
	dout->id = p->id;
	dout->fs = d->fs;
	dout->nch = d->nch ;
	dout->nbits = d->nbits;
	dout->rtime = d->rtime;
	dout->mticks = d->mticks;
    dout->nsamps = d->nsamps ;
	dout->size = d->size;

	/*----------------------------------------
	Do the mux work
	Source data is in d->nch contiguous blocks
	of length d->nsamps/d->nch
	Our task is to re-arrange data so channel
	data is interleaved
	-----------------------------------------*/

	nch = d->nch;
	pout = dout->p;
	pin = d->p;

	for(c=0; c<nch; c++)
	{
		int j = c;
		pin = d->p;
		pin += c*nin;

		switch(nch) //optimise speed
		{
			case (2):
				for(i=0; i<nin; i++)
				{
					pout[j] = *pin++;
					j+=2;
				}
				break;
			case (3):
				for(i=0; i<nin; i++)
				{
					pout[j] = *pin++;
					j+=3;
				}
				break;
			case (4):
				for(i=0; i<nin; i++)
				{
					pout[j] = *pin++;
					j+=4;
				}
		}
	}

	DATA_free(d) ;
    DATA_POST(&(p->jlist),dout) ;  // post the jobs attached
	return(OK) ;
}

int	muxMeta(int id)
{
 Mux_Obj 	*p = (Mux_Obj *)CFG_getstate(id) ;
  // setup the metadata
 INFO_new(id,NULL,p->src,NULLID) ;
 INFO_add(id,"PROC",NULL,"MUX") ;
 return(INFO_end(id)) ;			// post the metadata
}

int	muxAttach(int id, int downstr_id, int nice)
{
	Mux_Obj *p = (Mux_Obj *)CFG_getstate(id) ;

	if(nice==SRCNICE) {		// this is a SRC id notification call
		p->src = downstr_id ;
		return(muxMeta(id)) ;		// post the metadata for this instance
	}

	return(JOB_add(&(p->jlist),CFG_getprocfxn(downstr_id),CFG_getstate(downstr_id),nice)) ;
}

void	muxRemove(int id, int downstr_id)
{
	Mux_Obj	*p = (Mux_Obj *)CFG_getstate(id) ;
	JOB_remove(&(p->jlist),CFG_getprocfxn(downstr_id)) ;
}

int	muxClose(int id)
{
	Mux_Obj *p = (Mux_Obj *)CFG_getstate(id) ;
	// free the control structure
	DMEM_free(p) ;
	return(OK) ;
}

int	muxOpen()
{
	Mux_Obj	*p ;
	int		id ;

	if((p=(Mux_Obj *)DMEM_alloc(sizeof(Mux_Obj)))==(Mux_Obj *)NULL)
	 return(NULLID) ;		// allocation error
	p->jlist = NULLJOB ;
	id = CFG_register((JOB_Fxn)muxProc, muxAttach, p);
	p->id = id ;
	p->src = NULLID ;
	return(id) ;
}


