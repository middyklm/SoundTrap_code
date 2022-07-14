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

#include <string.h>
#include "d3defs.h"
#include "d3std.h"
#include "protect.h"
#include "board.h"
#include "devdep.h"
#include "misc.h"
#include "modnumbers.h"
#include "error.h"
#include "dmem.h"
#include "data.h"
#include "job.h"
#include "flsh.h"
#include "cfg.h"
#include "info.h"
#include "logg.h"
#include "timr.h"
#include "sensor.h"

#define	SEN_BUFFLEN		(100)
#define	SEN_ALLOCFAIL 5

typedef	struct 	{
	Uint16 period;
	DATA_Obj	*buff ;
} SEN_Obj ;

int	SEN_flush(int id)
{
	// current buffer is full - save it and assign a new one
	DATA_Obj	*d ;
	SEN_Obj 	*p = (SEN_Obj *)CFG_getstate(id) ;

	if((d=p->buff) != NULLDATA)
		FLSH_save(d) ;	// post the buffer for writing
    	// the FLSH module will free it when it is written

	// allocate a new buffer
	if((d = DATA_alloc(SEN_BUFFLEN))==NULLDATA) {
		//err(LOG_MOD,LOG_ALLOCFAIL) ;
		return(FAIL) ;
	}

	// set some metadata
	TIMR_gettime(&(d->rtime),&(d->mticks)) ;
	d->size = d->nsamps = 0 ;
	d->fs = 0l ;
	d->id = id ;
	p->buff = d ;
	return(OK) ;
}

int	SEN_close(int id)
{
	SEN_Obj *p = (SEN_Obj *)CFG_getstate(id) ;
	// flush the buffer if there is anything in it
	SEN_flush(id) ;
	// free the buffer
	DATA_free(p->buff) ;
	// free the structure
	DMEM_free(p) ;
	return(OK) ;
}

/*
int	SEN_diary(int id, char *mess)
{
	// add an entry with the current time to the log
	ulong	ltime[2] ;

	TIMR_gettime(&(ltime[0]),&(ltime[1])) ;
	return(SEN_add(id,ltime[0],ltime[1],mess)) ;
}
*/

void SenFlush(Uint16 id)
{
	SEN_Obj *so = (SEN_Obj *)CFG_getstate(id);
	if(so == NULL) return; //bad id
	// check if there is enough room for the new entry
	if((so->buff != NULLDATA) && (so->buff->size )) {
		SEN_flush(id);
	}
}


void SenCallBack(Uint16 id, Uint16 *buf, Uint16 length)
{
	Uint16 i;
	Uint16 *p;
	SEN_Obj *so = (SEN_Obj *)CFG_getstate(id);

	if(so == NULL) return; //bad id

	// check if there is enough room for the new entry
	if((so->buff == NULLDATA) || (so->buff->size+length >= so->buff->maxsize )) {
		if( SEN_flush(id) == FAIL) {
			return; //out of buffers - data will be lost
		}
	}

	if(length > so->buff->maxsize) return; //over-size message

	p = (Uint16*)so->buff->p;
	for(i=0; i<length; i++) {
		p[so->buff->size] = buf[i];
		so->buff->size++;
	}
}

int	SEN_open(char *suffix, Uint32 period, JOB_Fxn trigger, initFunction init, char *header)
{
    char s[10];
	int id ;
	SEN_Obj	*p ;

	if((p = (SEN_Obj *)DMEM_alloc(sizeof(SEN_Obj)))==NULL) {
		err(SEN_MOD,SEN_ALLOCFAIL) ;
		return(FAIL) ;
	}

	// initially, no buffer is defined
	p->buff = NULLDATA ;
	p->period = period;

	// request a configuration id number
	id = CFG_register(NULL,NULL,(Ptr)p) ;		// check this is ok

	// notify info module of the new stream
	INFO_new(id,FTYPE_CSV, NULLID, NULLID) ;
	INFO_add(id,"PROC",NULL,"SEN");
	INFO_add(id,"SUFFIX", NULL, suffix) ;
	INFO_add(id,"HEADER", NULL, header) ;

	snprintf(s, 10, "%ld", period);
	INFO_add(id,"PERIOD", "UNIT=\"ms\"", s) ;
	INFO_end(id) ;	// post the metadata

	Ptr state = init(id, SenCallBack);
	if( TIMR_DO_EVERY_MS(period, trigger, state) != OK) return 0;

	return id;
}
