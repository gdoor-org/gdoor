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
#include <Arduino.h>
#include "src/gdoor.h"

#define PIN_RX 32
#define PIN_TX 25
#define PIN_TX_EN 27

void setup() {
    Serial.begin(115200);
    Serial.setTimeout(1);
    GDOOR::setup(PIN_TX, PIN_TX_EN, PIN_RX);
}

void loop() {
    while(1) {
        GDOOR::loop();
        GDOOR_RX_DATA* rx_data = GDOOR::read();
        if(rx_data != NULL) {
            Serial.print("New data:\n");
            for(uint16_t i=0; i<rx_data->len; i++) {
                Serial.print("0x");
                Serial.print(rx_data->data[i], HEX);
                Serial.print(" ");
            }
            Serial.print("\nValid: ");
            Serial.print(rx_data->valid);
            Serial.print("\n");
        } else if (!GDOOR::active()) { //Neither RX nor TX active,
            if (Serial.available() > 0) { // let's check the serial port if something is in buffer
                String serialstr = Serial.readString();
                GDOOR::send(serialstr);
            }
        }
    }
}
