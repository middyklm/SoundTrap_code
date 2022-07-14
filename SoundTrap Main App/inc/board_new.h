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

// BOARD module - Low-level interface and initialization routines
// board.h v1.0

#ifndef _BOARD_H_
#define _BAORD_H_

 #ifndef USBBOOT
  /* Reference start of interrupt vector table   */
  /* This symbol is defined in file, vectors_test.s55 */
  extern void VECSTART(void);
  //extern Uint32 VECSTART ;		// was done like this
 #endif

 // conversion from TI time to UNIX time
 #define UNIXEPOCH			(0x83aa7e80ul)

 // flash memory access macros
 #define FLSH_MAXCHIPS		(8)
 #define CLE				(1)
 #define ALE				(2)
 #define FLSH_READ(a)	(0x0ff & (*(volatile int *)(a)))  
 #define FLSH_WRITE(a,d)	(*(volatile int *)((a)->p) = (d))   
 #define FLSH_ADDR(a,d)	(*(volatile int *)(((a)->p)+ALE) = (d))   
 #define FLSH_CONTR(a,d)	(*(volatile int *)(((a)->p)+CLE) = (d))

 #define FLSH_ISBUSY		((GPIO_RGET(IODATA)&0x20)==0)   
 #define FLSH_INT			(IRQ_EVT_INT1)

 /* timeouts in microseconds for primitive operations */
 #define READ_TIMEOUT		(400)
 #define BUSY_TIMEOUT		(40)
 #define ERASE_TIMEOUT		(30000)
 #define PROG_TIMEOUT		(30000)
 #define RESET_TIMEOUT		(10000)

 // IIC address of the EEPROM on the audio board
 #define AUDIO_EEPROM		(0x52)

 // miscellaneous
 #define EXT_INT			(IRQ_EVT_INT3)
 #define SYNC_INT			(IRQ_EVT_INT0)	// only used on DTAG3

 #define GPIO_READ			GPIO_RGET(IODATA)
 #define GPIO_WRITE(a)		GPIO_RSET(IODATA,(a))
 #define GPIO_IN(b)			GPIO_RSET(IODIR,GPIO_RGET(IODIR)&(~(b)))
 #define GPIO_OUT(b)		GPIO_RSET(IODIR,GPIO_RGET(IODIR)|(b))
 #define GPIO_ISOUT(b)		((GPIO_RGET(IODIR)&(b))!=0)

 #define REDLED				(1)
 #define GREENLED			(2)
 #define NOLED				(0)

 // ext i/o pins and modes
 #define IO_OFF				(0)
 #define IO_ON				(1)
 #define IO_IN				(-1)
 #define IO_UARTA			(0)
 #define IO_UARTB			(1)
 #define IO_EXT1			(2)
 #define IO_EXT2			(3)

 // IIC address of the EEPROM on the main board
 #define MAIN_EEPROM		(0x50)

 // EEPROM parameter store definitions
 #define PARAM_START		(0x6000)
 #define PARAM_MAGIC		(0x5a13)
 #define PARAM_END			(0xff)
 #define PARAM_BADBLOCK		(0xe0)
 #define MAXPARAMSIZE		(250)
 #define EEPROM_MAXADDR		(0x7fff)
 #define EEPROM_PAGE		(64)
 #define CODE_START			(0)
 #define MAXCODESIZE		(PARAM_START)

 // initial boot-up CPU clock frequency in MHz
 #ifdef	 USBBOOT
  #define SYSCLK_INIT		(48)
 #else
  #define SYSCLK_INIT		(24)
 #endif

 typedef 	int	(*CLK_Fxn)(int) ;

 // clock control macros and definitions
 #define EBSR				(0x6c00u)
 #define SYSR				(0x07fdu)
 #define SETCLKOUTDIV(n)	_PREG_SET(SYSR,(n)&7)
 #define CLKOUTON			_PREG_SET(EBSR,0x7fff & _PREG_GET(EBSR))
 #define CLKOUTOFF			_PREG_SET(EBSR,0x8000 | _PREG_GET(EBSR))

 #define BATTERY_FACTOR	(6.152)	// 1000*2*3.15/1024

 // arguments to pass to chargeon()
 #define CHG_LOW		(1)
 #define CHG_HIGH		(2)

 // return bitmap for calls to ispowered()
 #define CHG_USB		(1)
 #define CHG_EXT		(2)

 // Unique id number
 // Number of words in the id
 #define UID_SHORT		(2)
 #define UID_LONG		(5)

 // audio board sync bit maps
 #define SYNC12			(1)
 #define SYNC13			(2)

 //**********************************************
// device-dependent functions in devdep.c
//**********************************************

 extern int		getdevtype(void) ;
 extern int		leds(int which) ;
 extern int		chargeoff(void) ;
 extern int		getbattery(void) ;	// returns battery voltage in mV
 extern int		chargeon(int mode) ;
 extern void	powerdown(void) ;
 extern int		*flashchipaddr(int chip) ;
 extern int		numflashchips(void) ;
 extern int		ispowered(void) ;
 extern int		extint(int on) ;
 extern int		mute(int on) ;
 extern int		setgains(int gains) ;
 extern float	getgain(int setting) ;
 extern int		audiochanpwr(int ch, int on) ;
 extern int		audiopwr(int on) ;
 extern int		audiosync(int ch, int on) ;
 extern int		audionchans(void) ;
 extern int		audiocal(int on) ;
 extern int		isvalidchans(uns chans) ;
 extern int		sensorpwr(int on) ;
 extern int		ecgpwr(int on) ;
 extern int		vbs(int on) ;
 extern void	reboot(int on) ;
 extern ulong	getrtctime(void) ;
 extern void	dtag3_initspi(void) ;
 extern int		dtag3_vhfreq(int on) ;
 
//**********************************************
// board level control functions
//**********************************************

extern void	board_init(void) ;
extern int	setsysclk(int mhz) ;
extern int	getsysclk(void) ;
extern int	onclkchange(CLK_Fxn f) ;
extern int	isusbboot(void) ;
extern int	adcread(uns *d) ;
extern void	hibernate(int restart) ;
extern void	onhibernate(Fxn f) ;

// get board unique id
extern int	uid(unsigned int *d, int n) ;

// functions in misc.c

// set the error task to be the passed function
// The function must have prototype: void (f)(int module,int errno) ;
extern void	onerror(Fxn f) ;
extern int	geterrorcnt(void) ;

extern uchar *bunpack(uchar *b, uns *d, int n) ;
extern uchar *bpack(uns *d, uchar *b, int n) ;
extern long	 byte2long(int *p) ;
extern void	 microdelay(uns n) ;
extern void	 failmess(char *s) ;

#endif
