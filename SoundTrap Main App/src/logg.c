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

/*! \file logg.c
    \brief Buffer text messages and store in FLASH memory.

	Each instance of the log initialized with a LOG_open call will
	generate a separate file on the host when recordings are unpacked.
*/

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

//#define	LOG_BUFFLEN		(BIGBUFFSIZE)
#define	LOG_BUFFLEN		(200)		// TODO!!!

char	*LOG_FORMATS[] = {"none","quotes"} ;

typedef	struct {
				DATA_Obj	*buff ;
				} LOG_Obj ;


int	LOG_open(char *suffix, int mode)
{
 int			id ;
 LOG_Obj		*p ;

 if((p = (LOG_Obj *)DMEM_alloc(sizeof(LOG_Obj)))==NULL) {
	err(LOG_MOD,LOG_ALLOCFAIL) ;
	return(FAIL) ;
	}

 if(mode<0 || mode>=sizeof(LOG_FORMATS)/sizeof(char *)) {
	err(LOG_MOD,LOG_BADMODE) ;
	return(FAIL) ;
	}

 // initially, no buffer is defined
 p->buff = NULLDATA ;

 // request a configuration id number
 id = CFG_register(NULL,NULL,(Ptr)p) ;		// check this is ok

 // notify info module of the new stream
 INFO_new(id,FTYPE_TXT,NULLID,NULLID) ;
 INFO_add(id,"PROC",NULL,"LOG") ;
 INFO_add(id,"SUFFIX",NULL,suffix) ;
 INFO_add(id,"FORMAT",NULL,LOG_FORMATS[mode]) ;
 INFO_end(id) ;			// post the metadata
 return(id) ;
}


int	LOG_close(int id)
{
 LOG_Obj 	*p = (LOG_Obj *)CFG_getstate(id) ;

 // flush the buffer if there is anything in it
 LOG_flush(id) ;

 // free the buffer
 DATA_free(p->buff) ;

 // free the structure
 DMEM_free(p) ;
 return(OK) ;
}


int	LOG_diary(int id, char *mess)
{
 // add an entry with the current time to the log
 ulong	ltime[2] ;

 TIMR_gettime(&(ltime[0]),&(ltime[1])) ;
 return(LOG_add(id,ltime[0],ltime[1],mess)) ;
}



int	LOG_add(int id, ulong rtime, ulong mticks, char *mess)
{
 // add an entry to the log with a specific time
 int			k, m, n, *b ;
 DATA_Obj	*d ;
 LOG_Obj 	*p = (LOG_Obj *)CFG_getstate(id) ;

 m = strlen(mess) ;
 n = 5 + ((m+1)>>1) ;
 d = p->buff ;

 // check if there is enough room for the new entry
 if((d == NULLDATA) || (d->size+n>=d->maxsize)) {
	 if(LOG_flush(id)!=OK)
		 return(FAIL) ;
	 d = p->buff ;
	 }

 // pack the times and message length
 b = (int *)d->p+d->size ;
 *b++ = (int)(rtime>>16) ;
 *b++ = (int)(rtime&0x0ffff) ;
 *b++ = (int)(mticks>>16) ;
 *b++ = (int)(mticks&0x0ffff) ;
 *b++ = m ;

 // pack the string
 for(k=0;k<(n-5);k++) {
	 int	r = (*mess++)<<8 ;
	 *b++ = r | *mess++ ; 
	 }

 ++(d->nsamps) ;
 d->size += n ;
 return(OK) ;
}


int	LOG_flush(int id)
{
 // current buffer is full - save it and assign a new one
 DATA_Obj	*d ;
 LOG_Obj 	*p = (LOG_Obj *)CFG_getstate(id) ;

 if((d=p->buff) != NULLDATA)
    FLSH_save(d) ;	// post the buffer for writing
    // the FLSH module will free it when it is written

 // allocate a new buffer
 if((d = DATA_alloc(LOG_BUFFLEN))==NULLDATA) {
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


