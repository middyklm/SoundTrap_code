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

typedef struct{
	Uint16 autoLoad;    //Auto reload
	Uint16 preScaleDiv; //Prescale division
	Uint32 prd;         //period low
} Timer_Config;


typedef  void (*Timer_Isr)(void);

void timerInit();
void timerConfig(Uint16 timer, Timer_Config config);
void timerStop(Uint16 timer);
void timerStart(Uint16 timer);
void timerInterruptHandlerDeRegister(Uint16 timer);
void timerInterruptHandlerRegister(Uint16 timer, Timer_Isr isr);





