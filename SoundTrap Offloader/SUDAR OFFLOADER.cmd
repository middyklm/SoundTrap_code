-stack    0x1000                /* PRIMARY STACK SIZE    */
-sysstack 0x1000                /* SECONDARY STACK SIZE  */
-heap     0x0000                /* HEAP AREA SIZE        */

MEMORY
{
    MMR      (RW) : origin = 0000000h length = 0000C0h /* MMRs */
    VEC      (RX) : origin = 00000C0h length = 000300h /* on-chip ROM vectors */
    DARAM    (RW) : origin = 0000400h length = 00FC00h /* on-chip DARAM  */
    SARAM    (RW) : origin = 0010000h length = 030000h /* on-chip SARAM  */
    SARAM_SK (RW) : origin = 0040000h length = 010000h /* on-chip SARAM  */
}

SECTIONS
{
    vectors     : > VEC    ALIGN = 256
    .stack      : > SARAM_SK  ALIGN = 4
    .sysstack   : > SARAM_SK  ALIGN = 4
    .text       : > SARAM  ALIGN = 4
    .data       : > SARAM
    .bss        : > SARAM, fill = 0
	.cinit 		: > SARAM  
	.const 		: > SARAM
	.switch    	: > SARAM
	.sysmem 	: > SARAM
	.cio    	: > SARAM
	.buffer1   	: > SARAM
	.buffer2   	: > SARAM
	.audioBuf   : > DARAM
	.flashBuffer: > DARAM
	.audout		: > DARAM

}


/*
MEMORY
{
  PAGE 0:
    VEC(RWX)	  : origin = 00000C0h length = 000300h
    DATA(RWX)     : origin = 0000400h length = 017C00h 
    PROG(RX)      : origin = 0018000h length = 00A000h      
}

SECTIONS
{
  vectors    : > VEC ALIGN = 256
  .text      : > PROG 
  .data      : > DATA 
  .cinit     : > DATA 
  .switch    : > PROG
  .stack     : > DATA 
  .sysstack  : > DATA 
  .bss       : > DATA , fill =0 
  .sysmem    : > DATA
  .const     : > PROG
  .cio	     : > DATA
}
*/ 
