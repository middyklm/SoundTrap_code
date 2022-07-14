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


/*! \file cfg.c
    \brief Manages patches between data sources, processors and sinks.
*/

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
#include "data.h"
#include "cfg.h"

#define	NOMOREIDS		(1)

// CFG structure - there is one entry for each participating data source
// Contains pointers to an attach and proc function, an optional state
// and an optional metadata

typedef struct	{
				JOB_Fxn		proc ;
				CFG_Attach	attach ;
				Ptr			state ;
				Ptr			meta ;
				} CFG_Entry ;

// There is one CFG list with cfg_idcnt entries
static int				cfg_idcnt = 0 ;
static CFG_Entry		cfg_list[MAXID] ;


int	CFG_register(JOB_Fxn proc, CFG_Attach attach, Ptr state)
{
 // request a new configuration id and register attach and processing functions,
 // and an optional state

 CFG_Entry	*en ;
 int		id = NULLID ;

 if(cfg_idcnt>=MAXID) {
	 err(CFG_MOD,NOMOREIDS) ;
    return(id) ;
	}

 en = &(cfg_list[cfg_idcnt]) ;
 en->proc = proc ;
 en->attach = attach ;
 en->state = state ;
 en->meta = NULL ;

 // allocate an id number
 id = (++cfg_idcnt) ;
 return(id) ;
}


int	CFG_attach(int upstr_id, int downstr_id, int nice)
{
 // attach the downstream processing function to the upstream
 // module with a given nice number.
 CFG_Entry	*up, *down ;
 int		e ;

 if(upstr_id>cfg_idcnt || downstr_id>cfg_idcnt) {
	err(CFG_MOD,CFG_BADID) ;
	return(FAIL) ;
	}

 up = &(cfg_list[upstr_id-1]) ;
 down = &(cfg_list[downstr_id-1]) ;
 if(up->attach == NULL || down->proc == NULL) {
	err(CFG_MOD,CFG_BADATTACH) ;
	return(FAIL) ;
	}

 e = (up->attach)(upstr_id,downstr_id,nice) ;
 if(down->attach != NULL)
    e |= (down->attach)(downstr_id,upstr_id,SRCNICE) ;

 return(e) ;
}


Ptr	CFG_getstate(int id)
{
 if(id<=0 || id>cfg_idcnt)
	return(NULL) ;

 return(cfg_list[id-1].state) ;
}


JOB_Fxn	CFG_getprocfxn(int id)
{
 if(id<=0 || id>cfg_idcnt)
	return(NULL) ;

 return(cfg_list[id-1].proc) ;
}


Ptr	CFG_getmeta(int id)
{
 if(id<=0 || id>cfg_idcnt)
	return(NULL) ;

 return(cfg_list[id-1].meta) ;
}


int	CFG_setmeta(int id, Ptr meta)
{
 if(id<=0 || id>cfg_idcnt)
	return(FAIL) ;

 cfg_list[id-1].meta = meta ;
 return(OK) ;
}


int	CFG_pass(int id, Ptr data)
{
 CFG_Entry	*en ;

 if(id<=0 || id>cfg_idcnt)
	return(FAIL) ;

 en = &(cfg_list[id-1]) ;
 if(en->proc == NULL)
	return(FAIL) ;		// attempting to pass data to invalid processor

 return((en->proc)(en->state,data)) ;
}


int	CFG_getidcnt(void)
{
 return(cfg_idcnt) ;
}
