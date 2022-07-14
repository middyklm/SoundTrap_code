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

/*! \file cdet.h
    \brief Matched-filter based click/transient sound detector.
 
    This is a simple CFAR matched filter click detector optimized for short 
    transient sounds. It produces an output vector whenever the instantaneous
    power in the matched filter output exceeds a measure of the prevailing
    noise power times a relative threshold. The output vector contains a
    number of samples before and after the detection defined by npre and
    npost. The total number of samples cannot exceed BIGBUFFSIZE (defined
    in dmem.h. After each detection, there is a blanking interval during
    which no detection can be made. This interval is determined by nblank
    in samples and must be at least equal to BIGBUFFSIZE.
    Note: also currently restricted to single channel data.
*/

// error messages	
#define	 CDET_ALLOCFAIL	   (1)

// maximum number of detects waiting to finish at any time
#define  CDET_MAXDETS      (3)

typedef struct {
         int         nblank ;    // blanking count in samples
		 int         rthresh ;   // relative power threshold
         int         npre ;      // number of pre-detection samples
         int         npost ;     // number of post-detection samples
         int		 npwr ;		 // number of samples to sum power over in blkpwr detector
         int		 storeout ;	 // 1 if the filter output is required by downstream modules
         int		 minnp ;	 // minimum noise power (scaled by BPWR_PSHFT)
         } CDET_Params ;

/*! Data structure passed to attached processors when a detection is made
*/

typedef	struct {
         DATA_Obj *din ;         // raw data and timing information
	     DATA_Obj *dout ;		 // filter output data, if requested (i.e., storeout = 1 in CDET_Params)
         long     nl ;           // noise level at time of detection
         int      offset ;       // number of samples to detection sample
         int      q ;            // quality measure
         int	  sema ;		 // semaphore for deleting the detection structure
		 } CDET_Data ;

// Open an instance of the click detector
extern int		CDET_open(DATA *filt, int nfilt, long *nl, CDET_Params *p) ;

extern int		CDET_close(int id) ;

// prevent or enable detection
extern void    CDET_pause(int id, int on) ;

extern void     CDET_free(CDET_Data *d) ;

// attach a job to the output of the click detector - everytime a detection
// is made, attached jobs will be spawned with the data in a CDET_Data object.
extern int		CDET_attach(int id, int downstr_id, int nice) ;

// detach a job from the detector
extern void		CDET_remove(int id, int downstr_id) ;

