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

/*! \file x3cmpv2.c
    \brief Multi-channel loss-less audio compressor.

   The X3 compression algorithm is documented in:
     Johnson, Hurst and Partan, JASA 133(3):1387-1398, 2013.
*/


/* Enable the following for 1's complement coding. In this, the leading bits
   of the Rice codes (i.e., up to the suffix) are stored in the 1's complement
   of those in the above paper. This is an attempt to reduce bit wear in the FLASH.
*/
//#define 	COMPL_CODING


#include <stdlib.h>

#include 	"d3std.h"
#include 	"error.h"

#include	"d3defs.h"
#include	"dmem.h"
#include	"data.h"
#include	"job.h"
#include	"cfg.h"
#include	"info.h"
#include 	"x3cmpv2.h"
#include 	"modnumbers.h"
#include 	"modnumbers.h"

typedef struct {
	uns	   *op ;
	int	   ntogo ;
	uns	   pword ;
    uns		*start ;
	uns		*top ;
	} Pack ;


//****************** Code structures and definitions ******************* 
typedef struct {
		uns	nbits ;
		uns	code ;
		} Code ;


// RICE-0 Code for offset binary
#ifdef COMPL_CODING
 static Code	rcode0[] = {{12,0xffe},{10,0x3fe},{8,0xfe},{6,0x3e},{4,0xe},{2,2},
         {1,0},{3,6},{5,0x1e},{7,0x7e},{9,0x1fe},{11,0x7fe},{13,0xffe}} ;
#else
 static Code	rcode0[] = {{12,1},{10,1},{8,1},{6,1},{4,1},{2,1},
         {1,1},{3,1},{5,1},{7,1},{9,1},{11,1},{13,1}} ;
#endif

#define	rcode0_offs	(6)
#define	rcode0_name	"RICE0"

// RICE-1 Code for offset binary - currently not used in X3
#ifdef COMPL_CODING
 static Code	rcode1[] = {{12,0xffd},{11,0x7fd},{10,0x3fd},{9,0x1fd},{8,0xfd},{7,0x7d},
         {6,0x3d},{5,0x1d},{4,0xd},{3,5},{2,1},{2,0},{3,4},{4,0xc},{5,0x1c},
         {6,0x3c},{7,0x7c},{8,0xfc},{9,0x1fc},{10,0x3fc},{11,0x7fc},{12,0xffc},{13,0x1ffc}} ;
#else
 static Code	rcode1[] = {{12,3},{11,3},{10,3},{9,3},{8,3},{7,3},
         {6,3},{5,3},{4,3},{3,3},{2,3},{2,2},{3,2},{4,2},{5,2},
         {6,2},{7,2},{8,2},{9,2},{10,2},{11,2},{12,2},{13,2}} ;
#endif

#define	rcode1_offs	(11)
#define	rcode1_name	"RICE1"
/*
// RICE-2 Code for offset binary
#ifdef COMPL_CODING
 static Code rcode2[] = {{11,2041},{10,1019},{10,1017},{9,507},{9,505},{8,251},
	     {8,249},{7,123},{7,121},{6,59},{6,57},{5,27},{5,25},{4,11},{4,9},{3,3},{3,1},
         {3,0},{3,2},{4,8},{4,10},{5,24},{5,26},{6,56},{6,58},{7,120},{7,122},
	     {8,248},{8,250},{9,504},{9,506},{10,1016},{10,1018},{11,2040},{11,2042}} ;
#else
 static Code rcode2[] = {{11,5},{10,7},{10,5},{9,7},{9,5},{8,7},
	     {8,5},{7,7},{7,5},{6,7},{6,5},{5,7},{5,5},{4,7},{4,5},{3,7},{3,5},
         {3,4},{3,6},{4,4},{4,6},{5,4},{5,6},{6,4},{6,6},{7,4},{7,6},
	     {8,4},{8,6},{9,4},{9,6},{10,4},{10,6},{11,4},{11,6}} ;
#endif
*/
#define	rcode2_offs	(17)
#define	rcode2_name	"RICE2"

// RICE-3 Code for offset binary
#ifdef COMPL_CODING
 static Code rcode3[] = {{10,0x3f5},{10,0x3f3},{10,0x3f1},{9,0x1f7},{9,0x1f5},
         {9,0x1f3},{9,0x1f1},{8,0xf7},{8,0xf5},{8,0xf3},{8,0xf1},{7,0x77},{7,0x75},
         {7,0x73},{7,0x71},{6,0x37},{6,0x35},{6,0x33},{6,0x31},{5,0x17},{5,0x15},
         {5,0x13},{5,0x11},{4,7},{4,5},{4,3},{4,1},{4,0},{4,2},{4,4},{4,6},{5,0x10},{5,0x12},
         {5,0x14},{5,0x16},{6,0x30},{6,0x32},{6,0x34},{6,0x36},{7,0x70},{7,0x72},
         {7,0x74},{7,0x76},{8,0xf0},{8,0xf2},{8,0xf4},{8,0xf6},{9,0x1f0},{9,0x1f2},
         {9,0x1f4},{9,0x1f6},{10,0x3f0},{10,0x3f2},{10,0x3f4},{10,0x3f6}} ;
#else
 static Code rcode3[] = {{10,13},{10,11},{10,9},{9,15},{9,13},
         {9,11},{9,9},{8,15},{8,13},{8,11},{8,9},{7,15},{7,13},
         {7,11},{7,9},{6,15},{6,13},{6,11},{6,9},{5,15},{5,13},
         {5,11},{5,9},{4,15},{4,13},{4,11},{4,9},{4,8},{4,10},{4,12},{4,14},{5,8},{5,10},
         {5,12},{5,14},{6,8},{6,10},{6,12},{6,14},{7,8},{7,10},
         {7,12},{7,14},{8,8},{8,10},{8,12},{8,14},{9,8},{9,10},
         {9,12},{9,14},{10,8},{10,10},{10,12},{10,14}} ;
#endif

#define	rcode3_offs	(27)
#define	rcode3_name	"RICE3"

#define	bfp_name	"BFP"

// maximum absolute level that can be represented by the code
//#define	MAXCODETHR(len,offs)		((((len)-(offs))<=(offs))? ((len)-(offs)-1) : (offs))

//static X3_Code		CODES[] = {{rcode1,rcode1_name,rcode1_len,rcode1_offs,MAXCODETHR(rcode1_len,rcode1_offs)},
//							   {rcode2,rcode2_name,rcode2_len,rcode2_offs,MAXCODETHR(rcode2_len,rcode2_offs)}} ;

#define	NCODES		(3)
static char *CODES[NCODES] = {rcode0_name,rcode1_name,rcode3_name} ;
static int  CTHR[NCODES] = {CODE_THR1,CODE_THR2,CODE_THR3} ;
static Code *CCenters[NCODES] = {rcode0+rcode0_offs, rcode1+rcode1_offs, rcode3+rcode3_offs} ;

//********************************************************************

#define X3_RICEHDR		(1)
#define X3_RICEHDRLEN	(2)
#define X3_BFPHDR		(0)
#define X3_BFPHDRLEN	(6)
#define X3_DIFFHDRLEN	(12)

// BIG is the abs-magnitude above which coding is suppressed
#define BIG			(16384)

typedef struct {
    uns		blklen ;			// block length to use
	int		consec ;			// flag to pack output buffer across input buffers
	Pack    p ;					// bit packer
	DATA_Timekey tkey ;			// structure to handle time of output buffers
	DATA_Obj	*dout ;			// output data object
	int		nchs ;				// number of channels in the data to be compressed
	int		id ;				// number assigned to this configuration
	int		src ;				// source id number
	JOB_Obj	*jlist ;			// attached modules
	int		state[X3_MAXCHS] ;	// place to store filter states between calls
	} X3_Obj ;


// local temporary buffer for compressing filtered data 
int    x3tempbuff[X3_MAXBLKLEN] ;

// local functions
static inline int	newbuffer(X3_Obj *c, DATA_Buff *b) ;
static inline int	x3pack(X3_Obj *c, DATA_Buff *b) ;
static inline void	encodeframe(X3_Obj *c, Pack *p, int *x, int *state, int n, int nchs) ;
static inline void	postbuffer(X3_Obj *c, int *ip) ;
void	x3_error(int id) ;

// assembly language functions in x3cmpv2a.asm
extern int	firstbigger(int x, int *v, int n) ;
extern uns	sdiffmaxs(int *op, int *ip, int *state, int skip, int n) ;
extern void bpackcode(Pack *p, Code *c, int *ip, int csel, int len) ;
extern void bpack16(Pack *p, uns *ip, int skip, int len) ;
extern void bpack1(Pack *p, uns x, int nbits) ;
extern void bpackn(Pack *p, uns *ip, int nbits, int len) ;


int	X3_open(int nchs, uns blklen, int consec)
{
 X3_Obj		*c ;
 int		id ;

 // initialize a compressor structure and get a configuration id
 // first check the requested parameters
 blklen = MIN(blklen,X3_MAXBLKLEN) ;
 blklen = MAX(blklen,X3_MINBLKLEN) ;

 if((c = (X3_Obj *)DMEM_alloc(sizeof(X3_Obj))) == (X3_Obj *)NULL) {
	 err(X3_MOD,X3_OBJALLOCFAIL) ;
	 return(NULLID) ;
	 }

 // load the code parameters
 c->nchs = nchs ;
 c->blklen = blklen ;
 c->consec = consec ;
 DATA_INITTKEY(&(c->tkey)) ;
 c->dout = NULLDATA ;
 c->jlist = NULLJOB ;

 // request a configuration id number
 id = CFG_register((JOB_Fxn)X3_proc,X3_attach,(Ptr)c) ;
 c->id = id ;
 c->src = NULLID ;
 return(id) ;
}


int		X3_close(int id)
{
// disable and free a compressor
 X3_Obj 	*p = (X3_Obj *)CFG_getstate(id) ;

 // free the control structure
 DMEM_free(p) ;
 return(OK) ;
}


// attach a job to the output of the compressor - everytime data is
// received by X3_proc and compressed, the attached jobs will be 
// spawned with the compressed data in a DATA_Obj.
int		X3_attach(int id, int downstr_id, int nice)
{
 X3_Obj 	*p = (X3_Obj *)CFG_getstate(id) ;

 if(nice==SRCNICE) {          // this is a SRC id notification call
   p->src = downstr_id ;
   return(X3_meta(id)) ;    // post the metadata for this instance
   }

 return(JOB_add(&(p->jlist),CFG_getprocfxn(downstr_id),CFG_getstate(downstr_id),nice)) ;
}


// detach a job from the compressor
void		X3_remove(int id, int downstr_id)
{
 X3_Obj 	*p = (X3_Obj *)CFG_getstate(id) ;
 JOB_remove(&(p->jlist),CFG_getprocfxn(downstr_id)) ;
}


// post metadata for an instance
int		X3_meta(int id)
{
 char		s[20] ;
 X3_Obj 	*p = (X3_Obj *)CFG_getstate(id) ;
 int		k ;

  // setup the metadata
 INFO_new(id,"X3V2",p->src,NULLID) ;
 snprintf(s, 20, "%d",p->blklen) ;
 INFO_add(id,"BLKLEN",NULL,s) ;
 snprintf(s, 20, "%d",p->nchs) ;
 INFO_add(id,"NCHS",NULL,s) ;
 INFO_add(id,"FILTER",NULL,"diff") ;
 INFO_add(id,"NBITS",NULL,"16") ;

 for(k=0;k<NCODES;k++) {
	snprintf(s, 20, "THRESH=\"%d\"",CTHR[k]-1) ;
    INFO_add(id,"CODE",s,CODES[k]) ;
    }

 INFO_add(id,"CODE",NULL,"BFP") ;
 return(INFO_end(id)) ;			// post the metadata
}


// flush any data left in the compressor
int		X3_flush(int id)
{
 X3_Obj 	*c = (X3_Obj *)CFG_getstate(id) ;

 if(c->dout!=NULLDATA && c->dout->nsamps>0)
    postbuffer(c,NULL) ;

 return(OK) ;
}


void	x3_error(int id)
{
// Send a zero-length buffer downstream
 DATA_Obj	*d ;
 X3_Obj 	*p = (X3_Obj *)CFG_getstate(id) ;

 // create and post a zero-length buffer
 d = DATA_alloc(0) ;
 d->nsamps = 0 ;
 DATA_POST(&(p->jlist),d) ;		// post the jobs attached

 // free the current buffer, if allocated
 if((d=p->dout) != NULLDATA) {
	 DATA_free(d) ;
	 p->dout = NULLDATA ;
	 }
}


int		X3_proc(Ptr p, DATA_Obj *d)
{
 // X3 loss-less audio compressor
 // x3proc allocates a DATA_Obj and packs 16-bit words in d into a 16-bit 
 // packed buffer using settings in the compressor structure at p.

 DATA_Buff	b ;	
 X3_Obj	*c = (X3_Obj *)p ;

 if(c->jlist == NULLJOB) {		// no downstream jobs attached - nothing to do
	DATA_free(d) ;
	return(OK) ;
	}

 if(d->nsamps==0) {	 // zero length input data means a timing error in the source module
	x3_error(c->id) ;
	DATA_free(d) ;
	return(OK) ;
	}

 BUFF_INIT(&b,d,d->nsamps) ;	// initialize a buffer to work through the input data
 while(!BUFF_ISDONE(&b)) {				// while there is still input data to compress
    // if there is no current output buffer...
    if(c->dout == NULLDATA) {		// allocate an output buffer, set the metadata,
    	if( newbuffer(c,&b) != 0) {
    		//allocation failed
    		//x3_error(c->id); //TODO - causing firmware to hang
    		DATA_free(d);
    		return(OK) ;
    	}
    }

	// if there is no more room in the output buffer or if input data is not consecutive
	// terminate the output buffer and distribute it
	if(x3pack(c,&b))            // compress the input data
		postbuffer(c,b.p) ;
 }
 DATA_free(d) ;			  // free the input data object
 return(OK) ;
}


static inline int	x3pack(X3_Obj *c, DATA_Buff *db)
{
 // compress the input buffer. The bit packer and first samples are already 
 // initialized/done.

 int		nchs ;
 DATA_Obj *d ;
 Pack		*p = &(c->p) ;

 d = c->dout ;
 nchs = d->nch ;

 while(!BUFF_ISDONE(db)) {					// proceed blk by blk encoding 
	 int	n, k, *ip ;

	 n = MIN(BUFF_NTODO(db),c->blklen) ;	// next block size in samples/channel
	 if((int)(p->top-p->op) <= n*nchs)	  // will the next block fit?
	    return(1) ;                       // if not, flush output buffer

	 // if block size if not equal to the default, make a different-block-length header
	 if(n!=c->blklen && c->consec)
	    bpack1(p,n-1,X3_DIFFHDRLEN) ;

	 if(n<2) {						// if the block has only 1 sample/ch, it must be pass-through coded
	    bpack1(p,15,X3_BFPHDRLEN) ;		// raw block header
	    bpack16(p,(uns *)db->p,1,nchs) ;		// pack 16 bit ints from source
	    memcpy(c->state,db->p,nchs) ;	// copy the input data to the state
	    }

	 else
   	    for(k=0,ip=db->p;k<nchs;k++,ip++)		// compress the frame of n samples for each channel
		   encodeframe(c,p,ip,c->state+k,n,nchs) ;

 	 BUFF_UPDATE(db,n) ;						// move input data pointer and sample count
	 d->nsamps += n ;			// keep track of the number of samples compressed
     }

 return(c->consec==0) ;            // flush output buffer
}


static inline void		encodeframe(X3_Obj *c, Pack *p, int *x, int *state, int n, int nchs)
{
 // filter the n incoming samples to a temporary buffer and find the maximum 
 // magnitude. Depending on the magnitude, code the frame with a variable-
 // length code, a block floating point code, or pack it as an uncoded key 
 // frame, i.e., ma = max(abs(diff(x)))
 //   if ma<THR0
 //			rice0 encode filtered stream
 //   elseif ma<THR1
 //			rice2 encode filtered stream
 //   elseif ma<THR2
 //			rice3 encode filtered stream
 //   elseif ma < BIG
 //			block floating point encode filtered stream
 //   else
 //			raw encode unfiltered stream
      
 int    csel, nb ;
 uns	ma ;

 ma = sdiffmaxs(x3tempbuff,x,state,nchs,n) ;	 // apply filter and get max magnitude
 if(ma>=BIG) {                 					 // raw encoding - use unfiltered samples
	bpack1(p,15,X3_BFPHDRLEN) ;			// raw block header
	bpack16(p,(uns *)x,nchs,n) ;		// pack 16 bit ints from source
	return ;
    }

 if((csel=firstbigger(ma,CTHR,NCODES))<NCODES) {			 // encode frame with variable-length
    bpackcode(p,CCenters[csel],x3tempbuff,csel,n) ;            // Rice encoder requires n>=2
	return ;
	}

 nb = 16-_norm(ma) ;				// number of bits per sample
 bpackn(p,(uns *)x3tempbuff,nb,n) ;	// mask and pack from temp_buff
 return ;
}


static inline int	newbuffer(X3_Obj *c, DATA_Buff *b)
{
 Pack		*p ;
 DATA_Obj	*out, *in = b->d ;

 // a compressed DATA_Obj has:
 // nsamps = number of samples per channel represented by the object
 // nch = number of channels
 // size = the total size in 16-bit words of the object
 // NOTE: nsamps*nch will not necessarily equal size

 // allocate an output buffer
 if((out=c->dout=DATA_alloc(X3_OUTLEN))==NULLDATA) {
    //err(X3_MOD,X3_DATAALLOCFAIL) ;
    return(1) ;
    }

 // fill in some of the metadata
 out->fs = in->fs ;
 out->nch = in->nch ;
 out->nbits = in->nbits ;
 out->id = c->id ;

 // set the time in the output object
 DATA_settime(&(c->tkey),out,in,in->nsamps-b->n) ;

 // initialize the bit packer and store the initial header
 p = &(c->p) ;
 p->start = (uns *)(out->p) ;	 	// op points to the buffer for packed output data
 p->pword = 0 ;		          		// initialize the bit accumulator - it is right justified
 p->ntogo = 16 ;				    // there are 16 bits of room in pword
 p->top = p->start+out->maxsize-1 ;		// pointer to the last word that can be used

 // make first frame = first sample for each channel (nb bits each)
 //   Encoding is raw, packed. 
 memcpy(p->start,b->p,in->nch) ;
 memcpy(c->state,b->p,in->nch) ;	// copy the input data to the state
 p->op = p->start+in->nch ;
 out->nsamps = 1 ;
 BUFF_UPDATE(b,1) ;
 return(0) ;
}


static inline void	postbuffer(X3_Obj *c, int *ip)
{
 Pack		*p ;
 DATA_Obj	*d = c->dout ;

 // If any bits are left in the bit packer accumulator pword, save these to the output
 // buffer zero-filling to the right. Set the output data object size to the number of 
 // words packed.
 p = &(c->p) ;
 if(p->ntogo<16 && p->ntogo>=0)				// if there are any bits remaining
    *(p->op)++ = (p->pword)<<p->ntogo ;	// save them to the output, left justified

 d->size = (int)(p->op-p->start) ;			// size is the number of packed words
 DATA_POST(&(c->jlist),d) ;			// post the jobs attached
 c->dout = NULLDATA ;
}
