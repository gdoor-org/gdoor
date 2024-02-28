/* 
 * This file is part of the GDOOR distribution (https://github.com/gdoor-org).
 * Copyright (c) 2024 GDOOR Authors.
 * 
 * This program is free software: you can redistribute it and/or modify  
 * it under the terms of the GNU General Public License as published by  
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef DEFINES_H
#define DEFINES_H

// GDOOR
#define MAX_WORDLEN 25

// RX Statemachine
#define FLAG_BIT_RECEIVED 0x01
#define FLAG_BITSTREAM_RECEIVED 0x02
#define FLAG_DATA_READY 0x04

// RX
#define BIT_ONE_DIV 2.5
#define BIT_MIN_LEN 5
#define STARTBIT_MIN_LEN 45

// TX
#define STARTBIT_PULSENUM 66
#define ONE_PULSENUM 16
#define ZERO_PULSENUM 37
#define PAUSE_PULSENUM 30

#endif