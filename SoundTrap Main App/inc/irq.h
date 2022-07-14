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


#define RESET_EVENT  0
/** Non Maskable Interrupt    */
#define NMI_EVENT    1
/** External User Interrupt 0 */
#define INT0_EVENT   2
/** External User Interrupt 1 */
#define INT1_EVENT   3
/** TIMER Interrupt           */
#define TINT_EVENT   4
/** Programmable transmit Interrupt 0 (I2S0 Tx or MMC/SD0 Interrupt)     */
#define PROG0_EVENT  5
/** UART Interrupt            */
#define UART_EVENT   6
/** Programmable Receive Interrupt 0 (I2S0 Rx or MMC/SD0 SDIO Interrupt) */
#define PROG1_EVENT  7
/** DMA Interrupt */
#define DMA_EVENT    8
/** Programmable transmit Interrupt 1 (I2S1 Tx or MMC/SD1 Interrupt)     */
#define PROG2_EVENT  9
/** CoProcessor Interrupt */
#define CoProc_EVENT 10
/** Programmable Receive Interrupt 1 (I2S1 Rx or MMC/SD1 SDIO Interrupt) */
#define PROG3_EVENT  11
/** LCD Interrupt */
#define LCD_EVENT    12
/** SAR Interrupt */
#define SAR_EVENT    13
/** I2S2 Transmit Interrupt */
#define XMT2_EVENT   14
/** I2S2 Receive Interrupt */
#define RCV2_EVENT   15
/** I2S3 Transmit Interrupt */
#define XMT3_EVENT   16
/** I2S3 Receive Interrupt */
#define RCV3_EVENT   17
/** Wakeup or RTC Interrupt */
#define RTC_EVENT    18
/** SPI Interrupt */
#define SPI_EVENT    19
/** USB Interrupt */
#define USB_EVENT    20
/** GPIO Interrupt */
#define GPIO_EVENT   21
/** EMIF Interrupt */
#define EMIF_EVENT   22
/** I2C Interrupt */
#define I2C_EVENT    23
/** Bus Error Interrupt */
#define BERR_EVENT   24
/** Emulation Interrupt DLOG */
#define DLOG_EVENT   25
/** Emulation Interrupt RTOS */
#define RTOS_EVENT   26
/** These event bits (27-31) are reserved in IFR and IER Register */
/** Emulation Interrupt RTDX Receive */
#define RTDXRCV_EVENT  27
/** Emulation Interrupt RTDX Transmit */
#define RTDXXMT_EVENT  28
/** Emulation monitor mode            */
#define EMUINT_EVENT  29
/** Software Interrupt 30             */
#define SINT30_EVENT  30
/** Software Interrupt 31             */
#define SINT31_EVENT  31



typedef  void (*irqIsr)(void);


Uint16 irqGlobalDisable();
void  irqGlobalEnable();
void irqEnable( Uint16 eventId  );
void irqDisable( Uint16 eventId );
void irqPlug(Uint16 eventId, irqIsr isr);
void irqClear( Uint16 eventId );
