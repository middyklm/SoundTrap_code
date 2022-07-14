// D3-API v1.0
// Copyright (C) 2008-2010, Mark Johnson
//
// This file is part of D3, a real-time patch panel scheduler
// for digital signal processors.
//
// D3 is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// any later version.
//
// D3 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with D3. If not, see <http://www.gnu.org/licenses/>.

/*! \file bwdet.c
    \brief Beaked whale click detector

	Full CFAR beaked whale click detector with the exception of
	adaptive pre-whitening (a fixed filter is used).
*/

#include "d3defs.h"
#include "d3std.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "modnumbers.h"
#include "dmem.h"
#include "job.h"
#include "data.h"
#include "flsh.h"
#include "timr.h"
#include "filt.h"
#include "cfg.h"
#include "info.h"
#include "bpwr.h"
#include "cdet_v2.h"
#include "logg.h"
#include "x3cmpv2.h"
#include "uart.h"
#include <csl_rtc.h>
#include "timr.h"
#include "mspif.h"


#define	DETCODE		"CLICKTRAP"

// #define	DO_CLPAR		// enable this to extract parameters and classify detections

#define SAVERAW


//CSL_RtcTime         GetStartTime, GetDtecTime;

 static DATA    MF[] = {1092, 1092, 1092, 1092, 1092, 1092, 1092, 1092, 1092, 1092, 1092, 1092, 1092, 1092, 1092,
                        1092, 1092, 1092, 1092, 1092, 1092, 1092, 1092, 1092, 1092, 1092, 1092, 1092, 1092} ;


 #define MFCODE		"MF576_POW1_15"

 // gain of 0.7 fixed whitening filter incorporating a differentiator

 // see pp_wf480kHz.m

 static DATA WF[] = {-740,-396,532,1860,3168,3980,3980,3168,1860,532,-396,-740} ;


 #define WFCODE		"PP576_12"

 #define	BW_PWRTC	(6)		// power estimator adaptation time constant


// location to store and share noise level
static long     BWDET_NL[2] = {0l,0l} ;
static int		detlog, wfid, cdetid, powid, mode = 0 ;
static long		ndets = 0, nsaved = 0 ;

#ifdef DO_CLPAR
// header for the detection log
#define DLOG_HDR  "\"rtime\",\"mticks\",\"report\",\"pg\",\"nl\",\"e2n\",\"dur\",\"scnt\""
// Two types of message are saved to the log.
// Detection messages with format:
//	time (unix seconds), microseconds, D, processing gain, instantaneous noise level, energy to noise, duration
// Effort messages with format:
//	time (unix seconds), microseconds, E, state, average noise level, relative detection_threshold, number of detection saved
//
// Noise level, pg and e2n are stored in centiBels (100*log10 of the noise power)
// Duration is stored in samples
 static int		clparid ;
 void     bwreport(Ptr n, DATA_Obj *d) ;

#else
 // header for the detection log
#define DLOG_HDR  "\"rtime\",\"mticks\",\"report\",\"state\",\"nl\",\"thr\",\"scnt\""
// Two types of message are saved to the log.
// Detection messages with format:
//	time (unix seconds), microseconds, D, saved, instantaneous noise level
// Effort messages with format:
//	time (unix seconds), microseconds, E, state, average noise level, relative detection_threshold, number of detection saved
//
// Noise level is stored in centiBels (100*log10 of the noise power)

 void     bwreport(Ptr n, CDET_Data *d) ;
#endif

#define MAXTOSAVE    (200000)		// maximum number of clicks to save per 'dive' (currently 6 hour maximum)

static int	 cmpid, dx3id, snipid ;

void	 BWDET_state(int on) ;
int		 bwcmp(Ptr a, DATA_Obj *d) ;
int		 snipproc(Ptr s, CDET_Data *d) ;
int		 snipattach(int id, int downstr_id, int nice) ;
void     effortrep(int on) ;
int		 bwopenlog(void) ;


int BWDET_open(long fs, CDET_Params * params)
{
 INFO_WavMeta  w = {0l,16,1,"dwv",0,0,0} ;
 int	 err=0 ;
 int     repid ;
 char	 s[20] ;

 detlog = bwopenlog() ;								// log for storing detections
 wfid = FILT_open(WF,sizeof(WF)/sizeof(DATA),1) ;	// pre-whitening filter
 powid = BPWR_open(BW_PWRTC,BWDET_NL,&(BWDET_NL[1])) ;	// power averager for CFAR detector
 cdetid = CDET_open(MF,sizeof(MF)/sizeof(DATA),BWDET_NL, params) ;	// matched filter detector

 // get cfg ids for the reporting job
 repid = CFG_register((JOB_Fxn)bwreport,NULL,NULL) ;


 // setup patches
 err |= CFG_attach(wfid,powid,5) ;
 err |= CFG_attach(wfid,cdetid,4) ;

 // setup the metadata for the compressed snippets
 dx3id = X3_open(1,16,0) ;   // open a compressor for the detections
 w.fs = fs ;
 w.blklen = params->npre + params->npost ;
 cmpid = CFG_register((JOB_Fxn)bwcmp,NULL,NULL) ; // get a cfg id for the output job
 INFO_new(cmpid,FTYPE_WAV,dx3id,dx3id) ;
 INFO_addwavmeta(cmpid,&w) ;
 INFO_end(cmpid) ;			// post the metadata

 // setup the metadata for the snippet extractor
 snipid = CFG_register((JOB_Fxn)snipproc,(CFG_Attach)snipattach,NULL) ;
// snipid = CFG_register((JOB_Fxn)snipproc,NULL,NULL) ;
 INFO_new(snipid,NULL,cdetid,NULLID) ;
 INFO_add(snipid,"PROC",NULL,"snip") ;
 sprintf(s,"%d",w.blklen) ;
 INFO_add(snipid,"LEN",NULL,s) ;
 INFO_end(snipid) ;			// post the metadata

 #ifdef DO_CLPAR
  clparid = CLPAR_open(&bwclsparams) ;					// parameter extractor
  err |= CLPAR_attach_cl1(clparid,snipid,12) ;
  err |= CFG_attach(cdetid,clparid,10) ;
  err |= CFG_attach(clparid,repid,12) ;
  INFO_new(repid,NULL,clparid,NULLID) ;
 #else
  err |= CFG_attach(cdetid,snipid,12) ;
  err |= CFG_attach(cdetid,repid,10) ;
  INFO_new(repid,NULL,cdetid,NULLID) ;
 #endif

 // patch in the click compressor and saver
 err |= CFG_attach(snipid,dx3id,12) ;
 err |= CFG_attach(dx3id,cmpid,12) ;

 // setup the metadata for the detection reports
 INFO_add(repid,"PROC",NULL,DETCODE) ;
 INFO_add(repid,"WF",NULL,WFCODE) ;
 INFO_add(repid,"MF",NULL,MFCODE) ;
 sprintf(s,"ID=\"%d\"",wfid) ;
 INFO_add(repid,"USING",s,NULL) ;
 sprintf(s,"ID=\"%d\"",powid) ;
 INFO_add(repid,"USING",s,NULL) ;
 INFO_end(repid) ;			// post the metadata

 CDET_pause(cdetid,1) ;		// start up with the detector off
 return(err ? 0 : wfid);
}


int		BWDET_close(void)
{
 BWDET_state(0) ;
 LOG_close(detlog) ;
 FILT_close(wfid) ;
 BPWR_close(powid) ;
 CDET_close(cdetid) ;
 X3_close(dx3id) ;   // close the compressor
 #ifdef DO_CLPAR
  CLPAR_close(clparid) ;
 #endif
 return(OK) ;
}


void	BWDET_state(int on)
{
 if(mode == on)
	return ;

 mode = on ;
 CDET_pause(cdetid,on==0) ;
 effortrep(on) ;
}


void	BWDET_flush(void)
{
 //called on new file
 LOG_flush(detlog) ;
 LOG_add(detlog,0,0,DLOG_HDR) ;
 effortrep(mode);
 nsaved = 0;
}


int		bwopenlog(void)
{
 int logid = LOG_open("bcl",NOQUOTE) ;
 LOG_add(logid,0,0,DLOG_HDR) ;
 return(logid) ;
}

#ifdef DO_CLPAR
void     bwreport(Ptr n, DATA_Obj *d)
{
 // report a detection to the log
 // report format is: pg, nl, e2no, rdur
  char    s[50] ;
  CLPAR_Data *p = (CLPAR_Data *)d->p ;

  if(p->cls==1 && nsaved<MAXTOSAVE)
     sprintf(s,"D,%d,%d,%d,%d,%d",p->fpk-p->rpk,p->nl,p->re-p->nl,p->rdur,nsaved) ;
  else
     sprintf(s,"D,%d,%d,%d,%d,0",p->fpk-p->rpk,p->nl,p->re-p->nl,p->rdur) ;

 LOG_add(detlog,d->rtime,d->mticks,s) ;
 DATA_free(d) ;
}

#else

void     bwreport(Ptr n, CDET_Data *d)
{
 // report a detection to the log
 // report format is: pg, nl, e2no, rdur

 char    s[25] ;
 int nl = (int)(100*log10((double)d->nl+0.5))-30*BPWR_PSHFT ;      // noise level in centiBels
 sprintf(s,"D,%d,%d,,",nsaved<MAXTOSAVE,nl) ;
 //sprintf(s,"D,%d,%d,%ld,%ld",nsaved<MAXTOSAVE,nl,d->nl,BWDET_NL[0]) ;
 #ifdef JTAG
  printf(s);printf("\n");
 #endif
 LOG_add(detlog,d->din->rtime,d->din->mticks,s) ;
 CDET_free(d) ;
 ++ndets ;

 uartTx_Str((Uint16 *)"Explosion detected*10*10\r\n");
}
#endif


void     effortrep(int on)
{
 // called on file flush
 // report statistics
 // report format is:E,state,nl,ndetected,nsaved
 // state 1 means that the detector is turning on - no other values are given as they are not valid
 //
 char    s[40] = "E,1,,," ;

 if(on==0) {
    int nl = (int)(100*log10((double)BWDET_NL[1]+0.5))-30*BPWR_PSHFT ;      // noise level in centiBels
    sprintf(s,"E,0,%d,%ld,%ld",nl,ndets,nsaved) ;
    ndets = 0 ;
    nsaved = 0; //reset save count
 }

 #ifdef JTAG
  printf(s); printf("\n") ;
 #endif
 LOG_diary(detlog,s) ;
}


int		snipproc(Ptr s, CDET_Data *d)
{
 // when this is called there is a detection to be saved to memory
 if(nsaved >= MAXTOSAVE) {
	CDET_free(d) ;
    return(OK) ;
    }

 ++nsaved ;
 DATA_INCSEMA(d->din) ;
 CFG_pass(dx3id,d->din) ;
 CDET_free(d) ;
 return(OK) ;
}


int		snipattach(int id, int downstr_id, int nice)
{
// if(nice==SRCNICE)          // this is a SRC id notification call
 //  return(X3_meta(id)) ;    // post the metadata for this instance

 return(OK) ;
}


int		bwcmp(Ptr s, DATA_Obj *d)
{
 d->id = cmpid ;
 FLSH_save(d) ;
 DATA_free(d) ;
 return(OK) ;
}
