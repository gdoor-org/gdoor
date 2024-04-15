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
#include "src/mqtt_helper.h"
#include "src/wifi_helper.h"

#define PIN_RX 12
#define PIN_TX 25
#define PIN_TX_EN 27

boolean debug = false; // Global variable to indicate if we are in debug mode (true)
const char* mqtt_topic_bus_rx = NULL;

/**
 * Function which parses user provided serial input
 * for commands and executes logic based on these commands.
 * @param input String with input from user.
 * @return boolean, true if a command was found, false if no valid command was found.
*/
boolean parse(String  &input) {
    if(input == "*debug") {
        debug = true;
        return true;
    } else if(input == "*normal") {
        debug = false;
        return true;
    }
    return false;
}

/**
 * Function which outputs bus data via the serial port and MQTT.
 * Depending in debug mode, it may output more data.
 * 
 * In normal mode, it also checks the valid flag and only
 * outputs valid bus messages.
 * @param busmessage The bus message to be send out to the user.
*/
void output(GDOOR_DATA_PROTOCOL &busmessage, const char* topic) {
    if(!debug) {
        if(busmessage.raw->valid) {
            MQTT_HELPER::printer.print("{");
            MQTT_HELPER::printer.print(busmessage);
            MQTT_HELPER::printer.println("}");
            MQTT_HELPER::printer.publish((const char*)topic);
        }
    } else {
        MQTT_HELPER::printer.print("{");
        MQTT_HELPER::printer.print(busmessage);
        MQTT_HELPER::printer.println("}");
        MQTT_HELPER::printer.print("{");
        MQTT_HELPER::printer.print(*(busmessage.raw));
        MQTT_HELPER::printer.println("}");
        MQTT_HELPER::printer.publish((const char*)topic);
    }
}

void setup() {
    Serial.begin(115200);
    Serial.setTimeout(1);
    Serial.println("GDOOR Setup start");
    GDOOR::setup(PIN_TX, PIN_TX_EN, PIN_RX);
    WIFI_HELPER::setup();
    MQTT_HELPER::setup(WIFI_HELPER::mqtt_server(), WIFI_HELPER::mqtt_port(), WIFI_HELPER::mqtt_topic_bus_tx());

    mqtt_topic_bus_rx = WIFI_HELPER::mqtt_topic_bus_rx();
    debug = WIFI_HELPER::debug();

    Serial.println("GDOOR Setup done");
}

void loop() {
    WIFI_HELPER::loop();
    MQTT_HELPER::loop();
    GDOOR::loop();
    GDOOR_DATA* rx_data = GDOOR::read();
    if(rx_data != NULL) {
        GDOOR_DATA_PROTOCOL busmessage = GDOOR_DATA_PROTOCOL(rx_data);
        output(busmessage, mqtt_topic_bus_rx);        
    } else if (!GDOOR::active()) { // Neither RX nor TX active,
        String str_received("");
        if (Serial.available() > 0) { // let's check the serial port if something is in buffer
            str_received = Serial.readString();
            str_received.trim();    
        } else {
            str_received = MQTT_HELPER::receive();
        }

        if(str_received.length() > 0) {
            if(!parse(str_received)) { //Check if received string is a command
                GDOOR::send(str_received); // Send to bus if it is not a command
                Serial.print("Send: ");
                Serial.println(str_received);
            }
        }
        
    }
}
