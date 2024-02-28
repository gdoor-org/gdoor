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
#include "gdoor.h"

namespace GDOOR {
    void setup(int txpin, int txenpin, int rxpin) {
        GDOOR_RX::setup(rxpin);
        GDOOR_TX::setup(txpin, txenpin);
    }

    void loop() {
        GDOOR_RX::loop();
    }

    GDOOR_RX_DATA* read() {
        return GDOOR_RX::read();
    }

    void send(uint8_t *data, uint16_t len) {
        GDOOR_TX::send(data, len);
    }

    void send(String str) {
        GDOOR_TX::send(str);
    }
}