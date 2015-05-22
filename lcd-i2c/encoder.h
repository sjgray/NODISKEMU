/* NODISKEMU - SD/MMC to IEEE-488 interface/controller
   Copyright (C) 2007-2012  Ingo Korb <ingo@akana.de>

   NODISKEMU is a fork of sd2iec by Ingo Korb (et al.), http://sd2iec.de

   Inspired by MMC2IEC by Lars Pontoppidan et al.

   FAT filesystem access based on code from ChaN and Jim Brain, see ff.c|h.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License only.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA


   encoder.h: Encoder and button polling

*/

#ifndef ENCODER_H
#define ENCODER_H

#include "timer.h"

void encoder_init(void);
void encoder_buttonisr(void);

extern volatile int8_t  encoder_position;
extern volatile uint8_t button_state;
extern volatile tick_t  last_button_change;

#endif
