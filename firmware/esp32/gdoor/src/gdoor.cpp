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
    /*
    * Setup everything needed for GDOOR.
    * @param int txpin Pin number where PWM is created when sending out data
    * @param int txenpin Pin number where output buffer is turned on/off
    * @param int rxpin Pin number where pulses from bus are received
    */
    void setup(int txpin, int txenpin, int rxpin) {
        GDOOR_RX::setup(rxpin);
        GDOOR_TX::setup(txpin, txenpin);
    }

    /*
    * RX loop, needs to be called in main loop()
    * Needed for the decoding logic.
    */
    void loop() {
        GDOOR_RX::loop();
    }

    /**
    * User function, called to see if new data is available.
    * @return Data pointer as GDOOR_RX_DATA class or NULL if no data is available
    */
    GDOOR_RX_DATA* read() {
        return GDOOR_RX::read();
    }

    /*
    * Send out data.
    * @param data buffer with bus data
    * @param len length of buffer, can be max MAX_WORDLEN
    */
    void send(uint8_t *data, uint16_t len) {
        GDOOR_TX::send(data, len);
    }

    /*
    * Send out data.
    * @param hex string data without 0x prefix
    */
    void send(String str) {
        GDOOR_TX::send(str);
    }

    /*
    * GDOOR activity status
    * @return true: GDOOR RX or TX is active. False: no GDOOR activity.
    */
    inline bool active() {
        return (GDOOR_TX::tx_state != 0 || GDOOR_RX::rx_state != 0);
    }
}