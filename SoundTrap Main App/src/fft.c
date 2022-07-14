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

/*! \file fft.c
    \brief calculate average FFT
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
#include "minmax.h"
#include "flsh.h"
#include "math.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


static int fftOutId;

typedef struct {
	int	nchans ;
	JOB_Obj		*jlist;	// list of jobs attached for receive
	int			id;		// number assigned to this configuration
	int			src;		// source id number
	DATA_Obj	*dout;
	Uint16 cycles;
} Fft_Obj ;

struct cmpx						/* Q15 format */
{
	Int16 re;
	Int16 im;
};

#define	FFT_PTS 1024				/* This is for 128 FFT case */

typedef struct cmpx complex;

#pragma DATA_SECTION(X, ".fftdata");
#pragma DATA_ALIGN(X, 2048);	/* Align for hwafft_br() */
complex X[FFT_PTS];

#pragma DATA_SECTION(fftout, ".fftdata");
Int32 fftout[1024];

#pragma DATA_SECTION(scratch, ".fftscratch");
#pragma DATA_ALIGN(scratch, 2048); /* Align for hwafft_br() */
complex scratch[FFT_PTS];

#pragma DATA_SECTION(hann, ".fftscratch");
int hann[1024];
//int hann[] = {0,0,1,3,5,8,11,15,20,25,31,37,44,52,61,69,79,89,100,111,123,136,149,163,178,193,208,225,242,259,277,296,315,335,356,377,399,421,444,468,492,517,542,568,595,622,650,678,707,736,767,797,829,860,893,926,960,994,1029,1064,1100,1137,1174,1211,1250,1288,1328,1368,1408,1449,1491,1533,1576,1619,1663,1708,1753,1798,1844,1891,1938,1986,2034,2083,2133,2182,2233,2284,2335,2387,2440,2493,2547,2601,2656,2711,2766,2823,2879,2937,2994,3053,3111,3171,3230,3291,3351,3413,3474,3536,3599,3662,3726,3790,3855,3920,3985,4051,4118,4185,4252,4320,4388,4457,4526,4596,4666,4737,4808,4879,4951,5023,5096,5169,5243,5317,5391,5466,5541,5617,5693,5769,5846,5923,6001,6079,6158,6236,6316,6395,6475,6555,6636,6717,6799,6880,6962,7045,7128,7211,7295,7379,7463,7547,7632,7717,7803,7889,7975,8062,8148,8236,8323,8411,8499,8587,8676,8765,8854,8944,9033,9123,9214,9304,9395,9486,9578,9670,9761,9854,9946,10039,10132,10225,10318,10412,10505,10599,10694,10788,10883,10978,11073,11168,11264,11359,11455,11551,11648,11744,11841,11937,12034,12131,12229,12326,12424,12521,12619,12717,12815,12914,13012,13111,13209,13308,13407,13506,13605,13704,13804,13903,14003,14102,14202,14302,14401,14501,14601,14701,14802,14902,15002,15102,15203,15303,15403,15504,15604,15705,15806,15906,16007,16107,16208,16309,16409,16510,16610,16711,16812,16912,17013,17113,17214,17314,17415,17515,17616,17716,17816,17916,18017,18117,18217,18317,18416,18516,18616,18716,18815,18915,19014,19113,19213,19312,19411,19509,19608,19707,19805,19904,20002,20100,20198,20296,20393,20491,20588,20685,20782,20879,20976,21072,21169,21265,21361,21457,21552,21647,21743,21838,21932,22027,22121,22216,22309,22403,22497,22590,22683,22776,22868,22961,23053,23144,23236,23327,23418,23509,23599,23690,23780,23869,23959,24048,24136,24225,24313,24401,24489,24576,24663,24750,24836,24922,25008,25093,25178,25263,25347,25431,25515,25599,25682,25764,25847,25929,26010,26091,26172,26253,26333,26413,26492,26571,26650,26728,26806,26883,26960,27037,27113,27189,27265,27340,27414,27488,27562,27636,27708,27781,27853,27925,27996,28067,28137,28207,28276,28345,28414,28482,28550,28617,28683,28750,28815,28881,28946,29010,29074,29137,29200,29263,29325,29386,29447,29508,29568,29627,29686,29745,29803,29860,29917,29974,30029,30085,30140,30194,30248,30301,30354,30407,30458,30510,30560,30611,30660,30709,30758,30806,30853,30900,30947,30993,31038,31083,31127,31170,31213,31256,31298,31339,31380,31420,31460,31499,31538,31576,31613,31650,31686,31722,31757,31791,31825,31859,31891,31924,31955,31986,32017,32046,32076,32104,32132,32160,32187,32213,32239,32264,32288,32312,32335,32358,32380,32402,32422,32443,32462,32481,32500,32518,32535,32551,32567,32583,32598,32612,32625,32638,32651,32662,32673,32684,32694,32703,32712,32720,32727,32734,32740,32746,32751,32755,32759,32762,32764,32766,32767,32767,32768,32767,32766,32764,32762,32759,32755,32751,32746,32740,32734,32727,32720,32712,32703,32694,32684,32673,32662,32651,32638,32625,32612,32598,32583,32567,32551,32535,32518,32500,32481,32462,32443,32422,32402,32380,32358,32335,32312,32288,32264,32239,32213,32187,32160,32132,32104,32076,32046,32017,31986,31955,31924,31891,31859,31825,31791,31757,31722,31686,31650,31613,31576,31538,31499,31460,31420,31380,31339,31298,31256,31213,31170,31127,31083,31038,30993,30947,30900,30853,30806,30758,30709,30660,30611,30560,30510,30458,30407,30354,30301,30248,30194,30140,30085,30029,29974,29917,29860,29803,29745,29686,29627,29568,29508,29447,29386,29325,29263,29200,29137,29074,29010,28946,28881,28815,28750,28683,28617,28550,28482,28414,28345,28276,28207,28137,28067,27996,27925,27853,27781,27708,27636,27562,27488,27414,27340,27265,27189,27113,27037,26960,26883,26806,26728,26650,26571,26492,26413,26333,26253,26172,26091,26010,25929,25847,25764,25682,25599,25515,25431,25347,25263,25178,25093,25008,24922,24836,24750,24663,24576,24489,24401,24313,24225,24136,24048,23959,23869,23780,23690,23599,23509,23418,23327,23236,23144,23053,22961,22868,22776,22683,22590,22497,22403,22309,22216,22121,22027,21932,21838,21743,21647,21552,21457,21361,21265,21169,21072,20976,20879,20782,20685,20588,20491,20393,20296,20198,20100,20002,19904,19805,19707,19608,19509,19411,19312,19213,19113,19014,18915,18815,18716,18616,18516,18416,18317,18217,18117,18017,17916,17816,17716,17616,17515,17415,17314,17214,17113,17013,16912,16812,16711,16610,16510,16409,16309,16208,16107,16007,15906,15806,15705,15604,15504,15403,15303,15203,15102,15002,14902,14802,14701,14601,14501,14401,14302,14202,14102,14003,13903,13804,13704,13605,13506,13407,13308,13209,13111,13012,12914,12815,12717,12619,12521,12424,12326,12229,12131,12034,11937,11841,11744,11648,11551,11455,11359,11264,11168,11073,10978,10883,10788,10694,10599,10505,10412,10318,10225,10132,10039,9946,9854,9761,9670,9578,9486,9395,9304,9214,9123,9033,8944,8854,8765,8676,8587,8499,8411,8323,8236,8148,8062,7975,7889,7803,7717,7632,7547,7463,7379,7295,7211,7128,7045,6962,6880,6799,6717,6636,6555,6475,6395,6316,6236,6158,6079,6001,5923,5846,5769,5693,5617,5541,5466,5391,5317,5243,5169,5096,5023,4951,4879,4808,4737,4666,4596,4526,4457,4388,4320,4252,4185,4118,4051,3985,3920,3855,3790,3726,3662,3599,3536,3474,3413,3351,3291,3230,3171,3111,3053,2994,2937,2879,2823,2766,2711,2656,2601,2547,2493,2440,2387,2335,2284,2233,2182,2133,2083,2034,1986,1938,1891,1844,1798,1753,1708,1663,1619,1576,1533,1491,1449,1408,1368,1328,1288,1250,1211,1174,1137,1100,1064,1029,994,960,926,893,860,829,797,767,736,707,678,650,622,595,568,542,517,492,468,444,421,399,377,356,335,315,296,277,259,242,225,208,193,178,163,149,136,123,111,100,89,79,69,61,52,44,37,31,25,20,15,11,8,5,3,1,0,0};

extern   Uint16 hwafft_br(Int32 *, Int32 *, Int16);
extern   Uint16 hwafft_1024pts(Int32 *, Int32 *, Int16, Int16);
//extern   Uint16 hwafft_1024pts(Int32 *data,Int32 *scratch,Int32 *duplicate_scratch,Int32 * duplicate_data,Uint16 fft_flag,Uint16 scale_flag);





void fftReperr(int id)
{
	Fft_Obj *p = (Fft_Obj *)CFG_getstate(id) ;
    char s[40];
    snprintf(s, 40, "WARNING=\"UNKNOWN\"");
    INFO_event("FFT", s, NULL);
}


extern Uint16 sine (int *x, int *r, Uint16 nx);


void fft(Int16  *inDataPtr, int reset)
{
	Int32 m;
	int *p1;

	int i;
	int res;
	for (i=0; i<FFT_PTS; i++)
	{
		//X[i].re = inDataPtr[i];
		m = inDataPtr[i];
		m*= hann[i];
		m/= 16384;//65536;//32768;
		X[i].re = m;
		//X[i].re = ((((Int32)*inDataPtr) * hann[i]) / 32768);
		X[i].im = 0;
		//inDataPtr++;
	}

	hwafft_br((Int32 *)X, (Int32 *)scratch, FFT_PTS); /* Arrange X[] in bit reversal order and store in temp */
	res = hwafft_1024pts((Int32 *)scratch, (Int32 *)X, 0, 0);
	//res = hwafft_1024pts(pscratch, pdata, pdata, pscratch, 0, 0);

	p1 = res ? (int *)X : (int *)scratch;

	if(reset) {
		for (i=0; i<1024; i++)
		{
			fftout[i] = p1[i];
			//fftout[i] = p1[i*2];
		}
	}
	else {
		for (i=0; i<1024; i++)
		{
			fftout[i] += p1[i];
			//fftout[i] += p1[i*2];
		}
	}
}

const int runsPerOutput = 2;
int runs;

int	fftProc(Ptr pt, DATA_Obj *d)
{
	int i;
	// called when there is data to work with
	Fft_Obj	*p = (Fft_Obj *)pt ;

	if(p->jlist == NULLJOB) {		// no downstream jobs attached - nothing to do
		DATA_free(d) ;
		return(OK) ;
	}

	if( d->nsamps==0 ) {
		return(OK) ;
	}

	fft(d->p, runs == 0); //do the fft

	if( runs >= runsPerOutput) runs = 0; else runs++;

	if( runs == runsPerOutput) {

		DATA_Obj	*dout = p->dout ;	  // recover the output data object
		if((p->dout=dout=(DATA_Obj *)DATA_alloc(BIGBUFFSIZE))==NULLDATA) {
			fftReperr(p->id);
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

		for(i=0; i<1024; i++) {
			((int *)dout->p)[i] = (Int16)(fftout[i]/runs);
		}
	    DATA_POST(&(p->jlist),p->dout) ;  // post the jobs attached
	    runs = 0;
	}

	DATA_free(d) ;
	return(OK);
}

int	fftMeta(int id)
{
 Fft_Obj 	*p = (Fft_Obj *)CFG_getstate(id) ;
  // setup the metadata
 INFO_new(id,NULL,p->src,NULLID) ;
 INFO_add(id,"PROC",NULL,"FFT") ;
 return(INFO_end(id)) ;			// post the metadata
}

int	fftAttach(int id, int downstr_id, int nice)
{
	Fft_Obj *p = (Fft_Obj *)CFG_getstate(id) ;

	if(nice==SRCNICE) {		// this is a SRC id notification call
		p->src = downstr_id ;
		return(fftMeta(id)) ;		// post the metadata for this instance
	}

	return(JOB_add(&(p->jlist),CFG_getprocfxn(downstr_id),CFG_getstate(downstr_id),nice)) ;
}

void	fftRemove(int id, int downstr_id)
{
	Fft_Obj	*p = (Fft_Obj *)CFG_getstate(id) ;
	JOB_remove(&(p->jlist),CFG_getprocfxn(downstr_id)) ;
}

int	fftClose(int id)
{
	Fft_Obj *p = (Fft_Obj *)CFG_getstate(id) ;
	// free the control structure
	DMEM_free(p) ;
	return(OK) ;
}

int	fftSave(Ptr s, DATA_Obj *d)
{
 d->id = fftOutId ;
 FLSH_save(d) ;
 DATA_free(d) ;
 return(OK) ;
}
int	fftOpen()
{
	INFO_WavMeta  w = {9600l,16,1,"fft",0,0,0} ;
	Fft_Obj	*p ;
	int		id;
	int i;

	if((p=(Fft_Obj *)DMEM_alloc(sizeof(Fft_Obj)))==(Fft_Obj *)NULL)
	 return(NULLID) ;		// allocation error
	p->jlist = NULLJOB ;
	id = CFG_register((JOB_Fxn)fftProc, fftAttach, p);
	p->id = id ;
	p->src = NULLID ;

	fftOutId = CFG_register((JOB_Fxn)fftSave,NULL,NULL) ; // get a cfg id for the output job
	INFO_new(fftOutId,"wav",id,id) ;
	INFO_addwavmeta(fftOutId,&w) ;
	INFO_end(fftOutId) ;			// post the metadata
	CFG_attach(id,fftOutId,12) ;

	for (i=0; i<1024; i++)
	{
		hann[i] = 16383 * (1.0 - cos(2*M_PI*i/1023));
	}

	return(id) ;
}

