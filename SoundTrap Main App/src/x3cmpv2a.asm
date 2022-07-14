;// SoundTrap Software v1.0
;//
;// Copyright (C) 2011-2014, John Atkins and Mark Johnson
;//
;// This work is a derivative of the D3-API Copyright (C) 2008-2010, Mark Johnson
;//
;// This file is part of the SoundTrap software. SoundTrap is an acoustic
;// recording system intended for underwater acoustic measurements. This
;// component of the SoundTrap project is free software: you can redistribute
;// it and/or modify it under the terms of the GNU General Public License as
;// published by the Free Software Foundation, either version 3 of the License,
;// or any later version.
;//
;// The SoundTrap software is distributed in the hope that it will be useful,
;// but WITHOUT ANY WARRANTY; without even the implied warranty of
;// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
;// General Public License for more details.
;//
;// You should have received a copy of the GNU General Public License
;// along with this code. If not, see <http://www.gnu.org/licenses/>.

; //!< x3cmp.s55 v1.0
; //!< Assembly functions for X3 loss-less audio compressor
; Last modified: Aug 2014 mj fixed output pointer saving for V3.x CPUs
;
	.mmregs

    .global  _firstbigger
	.global  _sdiffmaxs
	.global	 _bpackn
	.global	 _bpack1
	.global	 _bpack16
	.global	 _bpackcode

	; disable remarks - none apply to C5509A
	.noremark 5503		; MMR read of SP/SSP CPU_84
	.noremark 5505		; MMR read of SP/SSP CPU_84
	.noremark 5549		; MAR in compatibility mode CPU_28
	.noremark 5573		; MAR and BK/BSA updates CPU_43
	.noremark 5673		; accumulator shift left overflow CPU_89
	.noremark 5688		; RPTLOCAL in compatibility mode CPU_85

_firstbigger:
; //!< firstbigger
; //!< prototype: 	int	firstbigger(int x, int *v, int n)
; //!< function:		find the first element of v which is > x and return
; //!<		    	   the index. Return n if x is > all elements in v.
; //!< algorithm:	   k= 0
; //!<               do n times,
; //!<					   k += x>(*v++)
; //!<               return k
; //!< cycles:		
; //!< modes:		   works in AR or DSP mode, needs C54CM=0
; //!< notes:		   n 
; passed arguments:
; 				Int x	        T0
;				Int *v 	     AR0
;				Int n 		  T1
;				return Int    T0
	
       SUB       #1,T1            
       MOV       T1,BRC0         
       SUB     *AR0+,T0,AC0

       RPTBLOCAL L10n			; do len times
	    || MOV	  #0,T1

       XCCPART _n1,AC0>#0
       || ADD  #1,T1
_n1:
       SUB     *AR0+,T0,AC0
L10n:

	   NOP
       RET
       || MOV	  T1,T0


_sdiffmaxs:
; prototype: 	int	sdiffmaxs(int *op, int *ip, int *state, int skip, int n)
; function:		compute the single-difference of a buffer of n samples
;		    	and the maximum absolute value of the result.
; algorithm:	do n times,
;					*op = *(ip+skip) - *ip
;					res = max(res,abs(*op))
;					ip+=skip, op++
;               returns res/2 = max(abs(*op))/2
; cycles:		3n+9 (verified)
; modes:		works in AR or DSP mode, needs C54CM=0
; notes:		n is the number of differences computed. The input
;			    buffer must be n*skip long. n must be at least 2.
;				input and output buffers can be anywhere in memory
; passed arguments:
;				Int *op 	pointer to data			AR0
;				Int *ip 	pointer to data			AR1
;				Int *ip 	pointer to state		AR2
;				Int	skip	16 bit data				T0
;				Int n 		16 bit data				T1
;				return Int							T0
	
        SUB       #2,T1   		; do the loop n-1 times         
        MOV       T1,BRC0         
        || MOV    #0,AC1		; clear A (the max value accum.)

        RPTBLOCAL L10s			; do len times
	    || MOV	  *AR2,AC0		; recover the state

	    SUB	      AC0,*AR1,AC0	; reverse subtract value at AR1
	    MOV       AC0,*AR0+		; store the difference
	    || ABS	  AC0			; take the absolute value
L10s:    
	    MAX	      AC0,AC1		; keep track of the max(abs())
	    || MOV	  *(AR1+T0),AC0	; load value at AR1 and postincrement AR1

		; last iteration
	    SUB	      AC0,*AR1,AC0	; reverse subtract value at AR1
	    MOV       AC0,*AR0		; store the last difference
	    || ABS	  AC0   
	    MAX	      AC0,AC1
		|| MOV	  *AR1,AC0		; get the last input value
		MOV		  AC0,*AR2		; and store it to the state

		SAT		  AC1			; saturate on overflow
        RET						; return the max(abs())
        || MOV	  AC1,T0


_bpackn:
; prototype: 	void bpackn(Pack *p, Int *ip, Int nbits, Int len)
; function: 	Fixed length bit packer for block floating point
; algorithm:	Initialize accumulator to the pword element of p.
;				Pack the BFP header (encodes 'nbits-1' in 6 bits)
;				For len: mask input wordlet to nbits
;					pack nbit-length words and store completed
;					16-bit words at op.
;					repeat for next input wordlet
;				Return remaining accumulator in p.
; cycles: 	    3n+OH or 5n+OH if a word is saved (verified)
; modes:		works in AR mode, needs C54CM=0
; notes:		input data can be signed or unsigned. 
;				nbits can be 1 to 15 
;				input and output buffers can be anywhere in memory.
;				n must be >= 2 but can be even or odd.
; passed arguments:
;				Pack *p 	pointer to structure	AR0
;				Uns *ip 	pointer to data			AR1
;				Int nbits 	16 bit data				T0
;				Int	len		16 bit data				T1

		MOV	  	  #1,AC0		; prepare the mask
		BTST	  @#0,T1,TC1	; check if an odd n is requested
		MOV		  T1,AC1		; copy the block count
		SFTL	  T1,#-1		; divide block count by two
        SUB       #1,T1     	; block repeat count is len-1 
		|| SFTL	  AC0,T0		; shift 1 nbits to the left
        MOV       T1,BRC0       ; load block repeat counter 
		SUB		  #1,AC0,AR3	; subtract 1 to make an nbit AND mask
		;MOV	  *AR0(short(#1)),XAR2		; recover the output pointer
		MOV		  dbl(*AR0),XAR2		; recover the output pointer
        MOV       *AR0(short(#2)),T1      ; recover ntogo 
		SUB		  #6,T1			; 6 bits of BFP header
		MOV	  	  #16,AR4		; prepare #16 to add to ntogo
        MOV       *AR0(short(#3))<<#6,AC0  ; recover pword and shift 6 bits for the BFP header
		|| SUB	  #1,T0			; subtract 1 from nbits for the header
		OR		  T0,AC0		; add the BFP header to pword
		; in case there is now a full pword...
		|| XCC	  T1<=#0		; if ntogo<=0
		MOV	  	  AC0<<T1,*AR2+	; store the packed word with a shift
		|| ADD	  AR4,T1		; ntogo += 16 to make up for the word taken

		ADD		  #1,T0			; restore the bit count in T0
		BCC		  L21,!TC1		; skip the next instructions if n was even

		AND	  	  *AR1+,AR3,AC1	; mask the next input wordlet
		|| ASUB   T0,T1			; ntogo -= n
		ADD		  AC0<<T0,AC1	; shift pword and append the new wordlet
		|| XCC	  T1<=#0		; if ntogo<=0
		; conditional
		MOV	  	  AC1<<T1,*AR2+	; store the packed word with a shift
		|| ADD	  AR4,T1		; ntogo += 16 to make up for the word taken
		; end conditional
		MOV		  AC1,AC0		; return pword to ac0

L21:
        RPTBLOCAL L22        ; start block

								; pword is in ac0
		AND	  	  *AR1+,AR3,AC1	; mask the next input wordlet
		|| ASUB   T0,T1			; ntogo -= n
		ADD		  AC0<<T0,AC1	; shift pword and append the new wordlet
		|| XCC	  T1<=#0		; if ntogo<=0
		; conditional
		MOV	  	  AC1<<T1,*AR2+	; store the packed word with a shift
		|| ADD	  AR4,T1		; ntogo += 16 to make up for the word taken
		; end conditional

		AND	  	  *AR1+,AR3,AC0	; mask the next input wordlet
		|| ASUB   T0,T1			; ntogo -= n
		ADD		  AC1<<T0,AC0	; shift pword and append the new wordlet
		|| XCC	  T1<=#0		; if ntogo<=0
L22: 	; conditional
		MOV	  	  AC0<<T1,*AR2+	; store the packed word with a shift
		|| ADD	  AR4,T1		; ntogo += 16 to make up for the word taken
		; end conditional

		NOP
		MOV		  AC0,*AR0(short(#3))		; store the pword
		MOV	  	  T1,*AR0(short(#2))		; store ntogo
        RET
		|| MOV	  XAR2,dbl(*AR0)		; store the output pointer


_bpack1:
; prototype: 	void bpack1(Pack *p, uns x, int nbits)
; function: 	variable length single-word bit packer 
; algorithm:	pack bits in x and store filled words at op
; cycles: 		13 or 14 if a word is saved
; modes:		works in AR mode, needs C54CM=0
; notes:		only for unsigned input arguments - this routine
;				does not mask the input wordlet.
; passed arguments:
;				Pack *p 	pointer to structure	AR0
;				Uns x 		data					T0
;				Int nbits 	16 bit data				T1

		BCLR	  SXMD
		MOV		  #16,AR4		; prepare #16 to add to ntogo
		MOV		  dbl(*AR0),XAR2		; recover the output pointer
        ;MOV       *AR0(short(#1)),AR2     ; recover output pointer
		|| MOV	  T0,AC0		; temporary store wordlet to be packed
        MOV       *AR0(short(#2)),T0      ; recover ntogo 
		SUB		  T1,T0			; ntogo -= n
        || MOV    *AR0(short(#3))<<T1,AC1  ; recover pword with left shift to make room
		OR		  AC0,AC1		; append the new wordlet
		|| XCC	  T0<=#0		; if ntogo<=0
		MOV	  	  AC1<<T0,*AR2+	; store the packed word with a shift
		|| ADD	  AR4,T0		; ntogo += 16 to make up for the word taken

		MOV		  AC1,*AR0(short(#3))		; store the pword
		MOV	  	  T0,*AR0(short(#2))		; store ntogo
		|| BSET	  SXMD

        RET
		|| MOV	  XAR2,dbl(*AR0)		; store the output pointer


_bpack16:
; prototype: 	void bpack16(Pack *p, uns *ip, int skip, int n)
; function: 	bit packer for 16-bit integers
; algorithm:	for len: pack bits in ip and store filled words at op
; cycles: 		3n+11 (verified)
; modes:		works in AR mode, needs C54CM=0
; notes:		n is the number of input data words. The input
;			    buffer must be n*skip long. n must be at least 1.
;				The output buffer must be at least n words long.
;				input and output buffers can be anywhere in memory
; passed arguments:
;				Pack *p 	pointer to structure	AR0
;				uns *ip 	pointer to data			AR1
;				int skip 	16 bit data				T0
;				int len 	16 bit data				T1

        SUB       #1,T1     	; block repeat count is len-1 
        MOV       T1,BRC0       ; load block repeat counter 
		MOV		  dbl(*AR0),XAR2		; recover the output pointer
		;MOV		  *AR0(short(#1)),AR2		; recover the output pointer
		MOV		  *AR0(short(#2)),T1		; recover ntogo

        RPTBLOCAL L30           ; start block 
        || MOV    *AR0(short(#3))<<#16,AC1     ; recover pword 

		OR		  *(AR1+T0),AC1		; append the new wordlet
		MOV	  	  HI(AC1<<T1),*AR2+	; store the packed word with a shift
L30: 
		SFTL	  AC1,#16		; shift the packed word to make room

		MOV		  HI(AC1),*AR0(short(#3))		; store the pword
        RET						; no need to store ntogo
		|| MOV	  XAR2,dbl(*AR0)		; store the output pointer


_bpackcode:
; prototype: 	void bpackcode(Pack *p, Code *c, Int *ip, Int csel, Int len)
; function: 	variable length code encoder and bit packer 
; algorithm:	Initialize accumulator to p->pword
;				add 2-bit Rice block header (a 1)
;				For len/2: look up *ip in the code table to get code and
;					nbits
;					pack the nbit code into pword and store complete
;					16-bit words at op.
;					repeat for the next input point
;				Return remaining accumulator in p.
; cycles: 	    5n+OH or 6n+OH if a word is saved (verified)
; modes:		works in AR mode, needs C54CM=0
; notes:		input data can be signed or unsigned but cannot have more
;				significant bits than len. len must be at least 2.
;				The code table is arranged in {nbits,code} pairs. The code
;				table must have at least 2*n entries where n is the maximum
;				magnitude of the input words. 
;				input and output buffers can be anywhere is memory.
; passed arguments:
;				Pack *p 	pointer to structure	AR0
;				Code *c 	pointer to code			AR1
;				Int  *ip 	pointer to input data	AR2
;               Int  csel   16 bit data             T0
;				Int	 len	16 bit data				T1

		PSH	  	  T2
        || ADD     #1,T0                    ; increment code index to make Rice header
		PSHBOTH	  XAR5
		MOV		  dbl(*AR0),XAR3		; recover the output pointer

		BTST	  @#0,T1,TC1	; check if an odd n is requested
		SFTL	  T1,#-1
        SUB       #1,T1     	; block repeat count is len-2
        MOV       T1,BRC0       ; load block repeat counter 
        || MOV    *AR0(short(#2)),T1      ; recover ntogo 

		SUB		  #2,T1			; the 2-bit Rice blk header will be added
        || MOV    *AR0(short(#3))<<#2,AC0  ; recover pword, shifted left for blk header
		MOV		  #16,T2		; prepare #16 to add to ntogo

		MOV		  XAR1,XAR4		; copy the table base address to two temporary register
		MOV		  XAR1,XAR5
		OR	  	  T0,AC0		; add Rice blk header

		BCC		  L40,!TC1		; go to loop if n was even
		MOV		  *AR2+<<#1,AC2	; first offset is *ip x 2
		ADD		  AC2,AR4		; make the first table lookup address
		MOV	  	  *AR4,T0		; get next nbits
		MOV		  *AR4(short(#1)),AC1		; get the new wordlet
		ADD	  	  AC0<<T0,AC1		; add the wordlet to the shifted pword
		ASUB	  T0,T1				; ntogo -= nbits
		|| XCC	  T1<=#0			; if ntogo<=0
		; conditional
		MOV	  	  AC1<<T1,*AR3+	; store the packed word with a shift
		|| ADD	  T2,T1		; ntogo += 16 to make up for the word taken
		; end conditional
		MOV		  AC1,AC0		; restore pword in ac0
		MOV		  AR1,AR4

L40:	; prepare the first two table lookups and prefetch the third input point
		MOV		  *AR2+<<#1,AC2	; first offset is *ip x 2
		ADD		  AC2,AR4		; make the first table lookup address

		MOV		  *AR2+<<#1,AC2	; 2nd offset is *ip x 2
		ADD		  AC2,AR5		; make the 2nd table lookup address
		MOV	  	  *AR2+<<#1,AC2		; 3rd offset is *ip x 2

        RPTBLOCAL L41           ; start block 
		|| MOV	  *AR4,T0		; get next nbits

								; ac0 is pword
		MOV		  *AR4(short(#1)),AC1		; get the next wordlet
	    || ADD	  AR1,AC2			; get the lookup table base address
		ADD	  	  AC0<<T0,AC1		; add the wordlet to the shifted pword
		|| MOV	  AC2,AR4			; complete the next lookup address
		ASUB	  T0,T1				; ntogo -= nbits
		|| MOV	  *AR5,T0			; get next nbits
		MOV	  	  low_byte(*AR2+)<<#1,AC2		; next offset is *ip x 2
		|| XCC	  T1<=#0			; if ntogo<=0
		; conditional
		MOV	  	  AC1<<T1,*AR3+	; store the packed word with a shift
		|| ADD	  T2,T1		; ntogo += 16 to make up for the word taken
		; end conditional

									; ac1 is pword
		MOV		  *AR5(short(#1)),AC0		; get the new wordlet
		|| ADD	  AR1,AC2			; get the lookup table base address
		ADD	  	  AC1<<T0,AC0		; add the wordlet to the shifted pword
		|| MOV	  AC2,AR5			; complete the lookup address
		ASUB	  T0,T1				; ntogo -= nbits
		|| MOV	  *AR4,T0			; get next nbits
		MOV	      low_byte(*AR2+)<<#1,AC2		; next offset is *ip x 2
		|| XCC	  T1<=#0			; if ntogo<=0
L41:	; conditional
		MOV	  	  AC0<<T1,*AR3+	; store the packed word with a shift
		|| ADD	  T2,T1		; ntogo += 16 to make up for the word taken
		; end conditional

		NOP							; just in case
		POPBOTH	  XAR5
		MOV		  AC0,*AR0(short(#3))		; store the pword
		|| POP	  T2
		MOV	  	  T1,*AR0(short(#2))		; store ntogo
        RET
		|| MOV	  XAR3,dbl(*AR0)		; store the output pointer


	; re-enable remarks
	.remark 5503
	.remark 5505
	.remark 5549
	.remark 5573
	.remark 5673
	.remark 5688
