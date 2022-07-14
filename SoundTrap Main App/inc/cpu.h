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

typedef struct  {
    volatile Uint16 IER0;
    volatile Uint16 IFR0;
    volatile Uint16 ST0_55;
    volatile Uint16 ST1_55;
    volatile Uint16 ST3_55;
    volatile Uint16 RSVD0[64];
    volatile Uint16 IER1;
    volatile Uint16 IFR1;
    volatile Uint16 RSVD1[2];
    volatile Uint16 IVPD;
    volatile Uint16 IVPH;
    volatile Uint16 ST2_55;
} cpuRegs;

typedef volatile cpuRegs * cpuRegsOvly;
#define CPU_REGS ((cpuRegsOvly)  0x0000)

#define CPU_ST1_55_INTM 0x0001
