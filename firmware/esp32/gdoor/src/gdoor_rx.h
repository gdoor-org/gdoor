/* 
 * This file is part of the GDoor distribution (https://github.com/gdoor-org).
 * Copyright (c) 2024 GDoor authors.
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

#ifndef GDOOR_RX_H

#define GDOOR_RX_H
#include <Arduino.h>
#include "gdoor_data.h"

namespace GDOOR_RX { //Namespace as we can only use it once
    extern uint16_t rx_state;
    void setup(uint8_t rxpin);
    void loop();
    void enable();
    void disable();
    GDOOR_DATA* read();
};

#endif