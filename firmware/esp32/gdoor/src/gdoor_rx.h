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

#ifndef GDOOR_RX_H

#define GDOOR_RX_H
#include <Arduino.h>

class GDOOR_RX_DATA : public Printable { // Class/Struct to collect bus related infos
    public:
        uint16_t len;
        uint8_t *data;
        uint8_t valid;

        size_t printTo(Print& p) const {
            size_t r = 0;

            for(uint16_t i=0; i<rx_data->len; i++) {
                r+= p.print("0x");
                r+= p.print(data[i], HEX);
                r+= p.print(" ");
            }
            r+= p.print("\nValid: ");
            r+= p.print(valid);
            r+= p.print("\n");
            return r;
       }
};

namespace GDOOR_RX { //Namespace as we can only use it once
    extern uint16_t rx_state;
    void setup( int rxpin);
    void loop();
    void enable();
    void disable();
    GDOOR_RX_DATA* read();
};

#endif