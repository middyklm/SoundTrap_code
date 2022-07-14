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

// DEVDEP module - Low-level board-dependent interface and initialization routines
// devdep.h v1.0

 // number of different device types supported
 #define	NDEVS			(5)

 // Device type is determined by the value on ADC channel 0
 // device number is used as an index into configuration arrays.
 #define DWM_DEV		(0)	// DWM if ADC[0] is 0-128
 #define CTAG_DEV		(1)	// if ADC[0] is 128-384
 #define NA1_DEV		(2)	// NA1 (not allocated) if ADC[0] is 384-640
 #define NA2_DEV		(3)	// NA2 (not allocated) if ADC[0] is 640-896
 #define DTAG3_DEV		(4)	// DTAG3 if ADC[0] is 896-1024
 
 // conversion routine for ADC channel 0 to device type
 #define DEVICETYPE(x)	((x+128)>>8)

 // flash memory setup initializers have elements:
 // long	base address of first chip
 // long	address increment to subsequent chips
 // int		number of chips (i.e., chip select lines) in the array
 // int		high byte mask - if the chip number & high byte mask
 //			is true, the chip is accessed by the high byte of the
 //			data bus.

 #define DWM_FLASHSETUP		{0x40000l,0x200000l,4,0}
 #define CTAG_FLASHSETUP	{0x40000l,0x200000l,4,0}
 #define DTAG3_FLASHSETUP	{0x400000l,8l,8,0}

 // miscellaneous
 #define DWM_SWS_DIS		asm("       BCLR XF")
 #define DWM_SWS_EN			asm("       BSET XF")
 #define DTAG3_THERM_DIS	asm("       BCLR XF")
 #define DTAG3_THERM_EN		asm("       BSET XF")

 #define DWM_HIBERNATE			(GPIO_WRITE(0))
 // following definition is no longer needed mj 24/2/12
 //#define DTAG3_HIBERNATE		(TIMR_tout(1))

 // control register definitions
 // DWM initial GPIO: CLEAN & /STATUS & /HOST
 #define DWM_PREG_INITVAL		(0x84)
 // only FLASHSTATUS (GPIO.5) is an input on DWM
 // GPIO.1 (/HOST) varies between input and output as required to control
 // the host led and read the /USBBOOT line
 #define DWM_PREG_INITDIR		(0xdf)
 #define DWM_PREG_INIT			{GPIO_WRITE(DWM_PREG_INITVAL);GPIO_RSET(IODIR,DWM_PREG_INITDIR);}

 // DTAG3 initial GPIO out: CLEAN & HOST & WIO2
 #define DTAG3_PREG_INITVAL		(0x85)		// was 5

 // WIO1, FLASHSTATUS and /USBBOOT (GPIO.6,5,1) are initially
 // inputs on DTAG3.
 #define DTAG3_PREG_INITDIR		(0x9d)		// was 0x1d
 #define DTAG3_PREG_INIT		{GPIO_WRITE(DTAG3_PREG_INITVAL);GPIO_RSET(IODIR,DTAG3_PREG_INITDIR);}

 #define DWM_LEDMASK		(0x12)
 #define DWM_REDLED			(0x10)
 #define DWM_GREENLED		(2)
 #define DTAG3_LEDMASK		(0x11)
 #define DTAG3_REDLED		(0x10)
 #define DTAG3_GREENLED		(1)

 #define DTAG3_VBSMASK		(8)

  // recharge power modes
 #define DWM_PWRMASK			(8)
 #define DWM_USBHIPWR			(0)
 #define DWM_USBPWROFF			(8)
 #define DWM_ALLPWROFF			(8)
 #define DWM_ALLPWRON			(0)

 // there is no direct way to sense USBPWR - use the bootmode instead
 #define DWM_USBPWRGD			(isusbboot())
 #define DWM_EXTPWRGD			(0)

 #define DWM_REBOOTMASK	 	(1)

 // control WIOx lines on the DTAG3
 #define DTAG3_wio1_input	(GPIO_RSET(IODIR,GPIO_RGET(IODIR)&0xbf))
 #define DTAG3_wio1_output	(GPIO_RSET(IODIR,GPIO_RGET(IODIR)|0x40))
 #define DTAG3_wio2_input	(GPIO_RSET(IODIR,GPIO_RGET(IODIR)&0x7f))
 #define DTAG3_wio2_output	(GPIO_RSET(IODIR,GPIO_RGET(IODIR)|0x80))
 #define DTAG3_wio1_high	(GPIO_RSET(IODATA,GPIO_RGET(IODATA)|0x40))
 #define DTAG3_wio1_low		(GPIO_RSET(IODATA,GPIO_RGET(IODATA)&0xbf))
 #define DTAG3_wio1_get		((GPIO_RGET(IODATA)&0x40)!=0)
 #define DTAG3_wio2_high	(GPIO_RSET(IODATA,GPIO_RGET(IODATA)|0x80))
 #define DTAG3_wio2_low		(GPIO_RSET(IODATA,GPIO_RGET(IODATA)&0x7f))
 #define DTAG3_wio2_get		((GPIO_RGET(IODATA)&0x80)!=0)
 #define DTAG3_WIO2_MASK	(0x80)

 // AUDIO BOARDS
 // Audio board I2C peripherals slave addresses - up to two 8-bit registers are
 // supported. In DWM and DTAG3 there is only one register.
 #define AUDIOCREGS			{0x0058,0x0058,0,0,0x0058}

 // audio control register masks for audio board functions
 // initializers comprise the mask for the bit for each device.
 // there must be 5 entries in order: DWM,CTAG,NA1,NA2,DTAG3

 // mute function (no mute on DWM or CTAG)
 #define MUTEMASK			{0,0,0,0,0x80}
 // channel gain - this is the shift to reach the gain channels
 #define GAINSHIFT			{5,5,0,0,5}
 // sync function - this is the shift to reach the sync control bit(s)
 // no sync function on CTAG
 #define SYNCSHIFT			{4,8,0,0,4}
 // test function (this is done with BDX1 on DTAG3, no test function on CTAG)
 #define TESTMASK			{0x80,0,0,0,0}
 // channel power (note: on DWM, CTAG, control sense is inverted)
 #define CHANPWRSHIFT		{2,2,0,0,2}
 // audio power (not needed on DTAG3)
 #define AUDIOPWRMASK		{2,4,0,0,0}
 // number of audio channels supported
 #define AUDIONCHANS		{2,3,0,0,2}
 // sensor power (same on all devices)
 #define SENSORPWRMASK		(1)
 // channel combinations - which values of chans are acceptable
 // The number is a bitmap with a 1 at a bit position (0-15) indicating
 // that a chans value equal to that bit position is acceptable, e.g.,
 // if the only valid chans values are 001b and 011b for a device, the
 // channel combination word is 01010b (chans==1 or 3).
 #define VALIDCHANS			{0x0e,0x0aa,0,0,0x0a}
   
 // ECG preamplifier power mask on DTAG3
 #define DTAG3_ECGPWRMASK	(2)

 // note: DTAG3 values are not firm yet
 // these values ought to be the total gain from preamp input
 // to ADC value (32768==1.0)
 // Values are correct for DTAG-3 HF model. All other values
 // need to be checked.
// #define LOWGAINS			{8.9,0,0,1.4,20.7}
// #define HIGHGAINS			{28.9,0,0,13.3,32.7}

 // These values are now correct for the standard DTAG-3
 #define LOWGAINS			{8.9,0,0,1.4,21.3}
 #define HIGHGAINS			{28.9,0,0,13.3,33.4}

 // MSP interface instructions for the DTAG-3
 #define MSP_LED_ON			(0x30)
 #define MSP_LED_OFF		(0x31)
 #define MSP_HIPWR_ON		(0x32)
 #define MSP_HIPWR_OFF		(0x33)
 #define MSP_HIBERNATE		(0x34)
 #define MSP_REBOOT_ON		(0x36)
 #define MSP_REBOOT_OFF		(0x37)

 #define MSP_IICADDR		(0x60)

 #define DMSENS_ADC_ADDR	(0x99)
 
 
 int devdep_init(void);
 int leds(int which);
 void reboot(int on);
 void powerdown(void);
 int isvalidchans(uns chans);
 int mute(int on);
 int enableCalDAC(int on);
 int setgain(int channel, int gainOn);
int audiopwr(int channel, int on);
 int audiosync(int mode, int on);
 int audiochanpwr(int ch, int on);
 float getgain(int level);
 Uint32 getrtctime(void);
 int getAudioHardwareId(Uint16 *id);
 int setHPass(int channel, int hpass);
 int audiocreginit(int channel);

 
 
 
 
 
 
