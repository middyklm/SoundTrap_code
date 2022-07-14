-stack    0x2000                /* PRIMARY STACK SIZE    */
-sysstack 0x2000                /* SECONDARY STACK SIZE  */
-heap     0x1000                /* HEAP AREA SIZE        */

MEMORY
{
    MMR     (RW) : origin = 0000000h length = 0000C0h /* MMRs */
    SWVER   (RW) : origin = 00000C0h length = 000040h /* on-chip SARAM  */
    VEC     (RX) : origin = 0000100h length = 000300h /* on-chip ROM vectors */
	DARAM   (RW) : origin = 0000400h length = 005C00h /* on-chip DARAM  */
	DARAM1   (RW) : origin = 0006000h length = 004000h /* on-chip DARAM  */
	DARAM2   (RW) : origin = 000A000h length = 002000h /* on-chip DARAM  */
	DARAM3   (RW) : origin = 000C000h length = 002000h /* on-chip DARAM  */
	DARAM4   (RW) : origin = 000E000h length = 002000h /* on-chip DARAM  */

    SARAM_DMEM (RW) : origin = 0010000h length = 018000h /* on-chip SARAM  */
    SARAM   (RW) : origin = 0028000h length = 020000h /* on-chip SARAM  */
    SARAM_STK (RW) : origin = 0048000h length = 008000h /* on-chip SARAM  */


}

SECTIONS
{
    .swver		: > SWVER  
    vectors     : > VEC    ALIGN = 256
	.pingpong   : > DARAM2	ALIGN = 4
	.audout     : > DARAM	ALIGN = 4
    .sdbuf		: > DARAM1	ALIGN = 4
    .fmem       : > DARAM ALIGN = 4
    .dmem       : > SARAM_DMEM ALIGN = 4
    .text       : > SARAM  ALIGN = 4
    .data       : > SARAM
    .bss        : > SARAM, fill = 0
    .fdata      : > SARAM  
	.cinit 		: > SARAM
	.const 		: > SARAM
	.switch    	: > SARAM
	.sysmem 	: > SARAM
	.cio    	: > SARAM
	.fftdata	: > DARAM3 ALIGN = 4096
	.fftscratch	: > DARAM4 ALIGN = 4096
    .stack      : > SARAM_STK  ALIGN = 4
    .sysstack   : > SARAM_STK  ALIGN = 4
}





