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

/*! \file info.c
    \brief Metadata manager.
*/
#include "string.h"

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
#include "pstr.h"
#include "timr.h"
#include "cfg.h"
#include "info.h"
#include "swVer.h"

// metadata field for configuration metadata
#define	CFG_FIELD1		"<CFG ID=\"%d\">\n"
#define	CFG_FIELD2		"<CFG ID=\"%d\" FTYPE=\"%s\">\n"
#define	CFG_FIELD3		"<CFG ID=\"%d\" FTYPE=\"%s\" CODEC=\"%d\">\n"
#define	CFG_TERM		"</CFG>\n"
#define	CFG_SRCFIELD    "<SRC ID=\"%d\" />\n"

// maximum length of the <CFG...> string
#define	INFO_IDLEN		(50)

// initial length of the packed string buffer
#define	INFO_LEN		(100)

// file type attibutes arranged so that the FTYPE_ definitions are indices
//char	*FTYPES[] = {"\0","xml","txt","csv","wav"} ;
//#define	NFTYPES			(sizeof(FTYPES)/sizeof(char *))

// INFO structure
typedef struct	{
				PSTRING		p ;
				int			done ;
				} INFO_Entry ;

// the memory block for storing file descriptor data is in
// a special section (.fdata)
#define	FDATA_LENGTH	(256)
#pragma	DATA_SECTION(FDATA_TAB,".fdata")
int		FDATA_TAB[FDATA_LENGTH] ;

// local functions
void		info_startfield(PSTRING *p, char *field, char *attr, int vals) ;
int			info_pstralloc(PSTRING *p) ;
DATA_Obj	*info_pstr2data(PSTRING *p) ;
DATA_Obj	*info_getfdata(void) ;


int	INFO_new(int id, char *ftype, int src, int codecid)
{
 // initialize a new metadata descriptor with ID and FTYPE attributes,
 // and with a <SRC> element.
 // ftype must be one of the file types defined in info.h

 INFO_Entry	*m ;
 char	    s[INFO_IDLEN] ;
 PSTRING 	*p ;
 
 // check if there is any existing metadata and free it
 if((m=(INFO_Entry *)CFG_getmeta(id))!=NULL)
	DMEM_free(m) ;

 // check the id and clear the metadata just in case we fail in the memory allocations
 if(CFG_setmeta(id,NULL)==FAIL)
	return(FAIL) ;

 // allocate a new metadata structure and buffer space for the pstring
 if((m=(INFO_Entry *)DMEM_alloc(sizeof(INFO_Entry)))==NULL)
	return(FAIL) ;

 // initialize a packed string for the metadata
 p = &(m->p) ;
 if(info_pstralloc(p)!=OK) {
	DMEM_free(m) ;
	return(FAIL) ;
	}

 if(ftype==NULL)
    snprintf(s, INFO_IDLEN, CFG_FIELD1,id) ;

 else {
 	if(codecid==NULLID)
	   snprintf(s, INFO_IDLEN, CFG_FIELD2,id,ftype) ;
	else
	   snprintf(s, INFO_IDLEN, CFG_FIELD3,id,ftype,codecid) ;
	}

 pstr_add(p,s) ;

 if(src>0) {
    snprintf(s, INFO_IDLEN, CFG_SRCFIELD,src) ;
    pstr_add(p,s) ;
	}

 // post the metadata
 m->done = 0 ;						// metadata is not yet ready for release
 return(CFG_setmeta(id,(Ptr)m)) ;
}


int		INFO_add(int id, char *field, char *attr, char *value)
{
 // Add an information field to the metadata for configuration id.
 // The format of the field will be:
 // <field attr> value <\field>
 // or, if value is NULL
 // <field attr \> 
 // attr can be NULL

 PSTRING 	*p ;

 if(value==NULL) {
    INFO_addopen(id,field,attr,0) ;
    return(OK) ;
	}

 p = INFO_addopen(id,field,attr,1) ;
 INFO_addvalue(p,value) ;
 INFO_endfield(p,field) ;
 return(OK) ;
}


PSTRING		*INFO_addopen(int id, char *field, char *attr, int vals)
{
 // Add an information field to the metadata for configuration id.
 // The format of the field will be:
 // <field attr>
 // subsequent calls to INFO_addvalue and INFO_addclose are needed
 // to make a complete entry.

 INFO_Entry	*m ;
 PSTRING 	*p ;

 // check that the metadata is started
 if((m=(INFO_Entry *)CFG_getmeta(id))==NULL)
    return((PSTRING *)NULL) ;

 if(field==NULL) {		// there must be something in the field string
	err(INFO_MOD,BADFIELDSIZE) ;
	return((PSTRING *)NULL) ;
	}

 p = &(m->p) ;						// make a pointer to the metadata packed string

 // check if the metadata is already ready for release
 if(m->done == 1) {
	int	n ;
	// if so, we need to remove the CFG_TERM at the end of the packed string
	n = strlen(CFG_TERM) ;
	p->nby -= n ;
	p->ntogo += n ;
	m->done = 0 ;
	}

 info_startfield(p, field, attr, vals) ;
 return(p) ;
}


int	INFO_end(int id)
{
 INFO_Entry	*m ;
 
 // check that there is existing metadata
 if((m=(INFO_Entry *)CFG_getmeta(id))==NULL)
    return(FAIL) ;

 // check if the metadata is already ready for release
 if(m->done == 1)
	return(OK) ;

 // add the end terminator
 pstr_add(&(m->p),CFG_TERM) ;
 m->done = 1 ;						// label the metadata as ready for release
 return(FLSH_save(INFO_request(id))) ;		// post the metadata to the flash memory
}


DATA_Obj *INFO_request(int id)
{
 // request metadata associated with an id
 INFO_Entry	*m ;
 PSTRING	*p ;

 // check that there is metadata
 if((m=(INFO_Entry *)CFG_getmeta(id))==NULL)
    return(NULLDATA) ;

 // check that the metadata is ready for release
 if(m->done != 1)
	return(NULLDATA) ;

 p = &(m->p) ;				// make a pointer to the metadata packed string
 return(info_pstr2data(p)) ;
}


int		INFO_requestall(void)
{
 // request metadata for all active configurations ids
 int	k, err = OK ;
 DATA_Obj	*d ;

 err = FLSH_save(info_getfdata()) ;		// first send the filename data

 // go through all the active configurations and check that the 
 // metadata for each one is ready for release
 for(k=1;k<=CFG_getidcnt() && err==OK;k++) {
	d = INFO_request(k) ;
	if(d!=NULLDATA)
	   err = FLSH_save(d) ;
	}

 return(err) ;
}


int		INFO_addwavmeta(int id, INFO_WavMeta *w)
{
 // Add metadata for a wav file
 char		s[16] ;

 snprintf(s, 16,"%ld",w->fs) ;
 INFO_add(id,"FS",NULL,s) ;
 snprintf(s, 16,"%d",w->nbits) ;
 INFO_add(id,"NBITS",NULL,s) ;
 snprintf(s, 16,"%d",w->nch) ;
 INFO_add(id,"NCHS",NULL,s) ;
 if(w->suffix!=(char *)NULL)
    INFO_add(id,"SUFFIX",NULL,w->suffix) ;

 // optional metadata
 if(w->exp!=0) {                          // default EXP=0
    snprintf(s, 16,"%d",w->exp) ;
    INFO_add(id,"EXP",NULL,s) ;
    }

 if(w->timechk!=1) {                   // default TIMECHK=1
    snprintf(s, 16,"%d",w->timechk) ;
    INFO_add(id,"TIMECHK",NULL,s) ;
    }

 if(w->blklen!=0) {                   // default BLKLEN=0
    snprintf(s, 16,"%d",w->blklen) ;
    INFO_add(id,"BLKLEN",NULL,s) ;
    }

 return(OK) ;
}


int		INFO_event(char *field, char *attr, char *value)
{
 // Post an information field to the metadata
 // The format of the field will be:
 // <EVENT TIME="...">
 // <field attr> value </field>
 // or, if value is NULL
 // <field attr /> 
 // </EVENT>
 // attr can be NULL

 PSTRING 	p ;
 DATA_Obj	*d ;

 if(field==NULL) {		// there must be something in the field string
	err(INFO_MOD,BADFIELDSIZE) ;
	return(FAIL) ;
	}

 if(info_pstralloc(&p)!=OK)
 	return(FAIL) ;

 pstr_add(&p,"<EVENT>\n") ;
 info_startfield(&p,field,attr,value!=NULL) ;
 if(value!=NULL) {
    INFO_addvalue(&p,value) ;
    INFO_endfield(&p,field) ;
	}
 pstr_add(&p,"</EVENT>\n") ;

 d = info_pstr2data(&p) ;
 DMEM_free(p.b) ;

 return(FLSH_save(d)) ;	// post the metadata to the flash memory
}


int		info_pstralloc(PSTRING *p)
{
 uns	*b ;

 if((b=(uns *)DMEM_alloc(INFO_LEN))==NULL)
	 return(FAIL) ;

 pstr_init(p,b,INFO_LEN) ;
 return(OK) ;
}


void	info_startfield(PSTRING *p, char *field, char *attr, int vals)
{
 pstr_add(p,"<") ;
 pstr_add(p,field) ;

 if(attr!=NULL) {
	pstr_add(p," ") ;	// add the attributes
	pstr_add(p,attr) ;
	}

 if(vals==0) {
	// close the element field with a />
	pstr_add(p," />\n") ;
	return ;
	}

 pstr_add(p,"> ") ;
}


void	INFO_addvalue(PSTRING *p, char *value)
{
 pstr_add(p,value) ;
}


void	INFO_endfield(PSTRING *p, char *field)
{
 pstr_add(p," </") ;
 pstr_add(p,field) ;
 pstr_add(p,">\n") ;
}


DATA_Obj	*info_pstr2data(PSTRING *p)
{
 int		nw ;
 DATA_Obj	*d ;

 nw = pstr_size(p) ;		// get the string size in words

 // allocate a DATA_Obj big enough for the metadata
 if((d=(DATA_Obj *)DATA_alloc(nw))==NULLDATA) {
	err(INFO_MOD,INFO_ALLOCFAIL) ;
    return(d) ;
	}

 // set the DATA_Obj metadata
 d->size = nw ;
 d->id = ROOTID ;
 d->nsamps = 1 ;								// only one CFG entity
 TIMR_gettime(&(d->rtime),&(d->mticks)) ;

 // copy metadata to the data object
 memcpy((uns *)(d->p),p->b,nw) ;
 return(d) ;
}


DATA_Obj	*info_getfdata(void)
// make a metadata (packed string) data object with the name and date of
// the executable software currently running on the target
{
 char	    s[INFO_IDLEN] ;
 PSTRING 	p ;
 DATA_Obj	*d ;
 //long		*q = (long *)FDATA_TAB ;
 int		e = 1 ;
 //uns		*qq ;

 // allocate a DATA_Obj big enough for the fdata
 if((d=(DATA_Obj *)DATA_alloc(FDATA_LENGTH))==NULLDATA) {
	err(INFO_MOD,INFO_ALLOCFAIL) ;
    return(d) ;
	}

 // initialize a packed string for the metadata
 pstr_init(&p,(uns *)(d->p),d->maxsize) ;
 snprintf(s, INFO_IDLEN, CFG_FIELD2,0,FTYPE_XML) ;
 pstr_cat(&p,s) ;

 snprintf(s, INFO_IDLEN, "<BINFILE TIME=\"%s\"", swTime) ;
 e = pstr_cat(&p,s) ;
 snprintf(s, INFO_IDLEN, " DATE=\"%s\" ", swDate) ;
 e |= pstr_cat(&p,s) ;
 snprintf(s, INFO_IDLEN, " VER=\"%s\"> ", swVer) ;
 e |= pstr_cat(&p,s) ;
 
 e |= pstr_cat(&p," </BINFILE>\n") ;

/*
 if(*q++ == FDATA_KEY) {		// good fdata
    // copy metadata to the data object
    snprintf(s, INFO_IDLEN, "<BINFILE MODTIME=\"%lx\"",*q++) ;
    e = pstr_cat(&p,s) ;
    snprintf(s, INFO_IDLEN, " LOADTIME=\"%lx\"> ",*q++) ;
    e |= pstr_cat(&p,s) ;
	qq = (uns *)q ;
	e |= pstr_pncat(&p,qq+1,*qq) ;
	e |= pstr_cat(&p," </BINFILE>\n") ;
	}
*/
 e |= pstr_cat(&p,CFG_TERM) ;

 if(e) {
    pstr_init(&p,(uns *)(d->p),d->maxsize) ;
    snprintf(s, INFO_IDLEN, CFG_FIELD2,0,FTYPE_XML) ;
    pstr_cat(&p,s) ;
    pstr_cat(&p,"<BINFILE />\n") ;
	pstr_cat(&p,CFG_TERM) ;
	}

 // set the DATA_Obj metadata
 d->size = pstr_size(&p) ;
 d->id = ROOTID ;
 d->nsamps = 1 ;								// only one CFG entity
 TIMR_gettime(&(d->rtime),&(d->mticks)) ;
 return(d) ;
}
