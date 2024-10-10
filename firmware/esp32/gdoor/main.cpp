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
#include <Arduino.h>
#include "src/gdoor.h"
#include "src/mqtt_helper.h"
#include "src/wifi_helper.h"
#include "src/printer_helper.h"
#include "src/udp_transmitter.h"

GDOOR_DATA_PROTOCOL gdoor_data_idle(NULL, true);

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
void output(GDOOR_DATA_PROTOCOL &busmessage, const char* topic, bool force=false) {
    if(force || debug || (busmessage.raw != NULL && busmessage.raw->valid)) {
        MQTT_HELPER::printer.print("{");
        MQTT_HELPER::printer.print(busmessage);
        MQTT_HELPER::printer.println("}");
        MQTT_HELPER::printer.publish((const char*)topic);
    }
}

void setup() {
    Serial.begin(115200);
    Serial.setTimeout(1);
    JSONDEBUG("GDoor Setup start");
    
    WIFI_HELPER::setup();
    MQTT_HELPER::setup(WIFI_HELPER::mqtt_server(),
                       WIFI_HELPER::mqtt_port(),
                       WIFI_HELPER::mqtt_user(),
                       WIFI_HELPER::mqtt_password(),
                       WIFI_HELPER::mqtt_topic_bus_tx(),
                       WIFI_HELPER::mqtt_topic_bus_rx());

    GDOOR::setRxThreshold(PIN_RX_THRESH, WIFI_HELPER::rx_sensitivity());
    GDOOR::setup(PIN_TX, PIN_TX_EN, WIFI_HELPER::rx_pin());

    mqtt_topic_bus_rx = WIFI_HELPER::mqtt_topic_bus_rx();
    debug = WIFI_HELPER::debug();

    UDP_TRANSMITTER::setup();

    JSONDEBUG("GDoor Setup done");
    JSONDEBUG("RX Pin: ");
    JSONDEBUG(WIFI_HELPER::rx_pin());
    JSONDEBUG("RX Sensitivity: ");
    JSONDEBUG(WIFI_HELPER::rx_sensitivity());
}

void loop() {
    WIFI_HELPER::loop();
    MQTT_HELPER::loop();
    GDOOR::loop();
    GDOOR_DATA* rx_data = GDOOR::read();

    // Output bus idle message on new MQTT connections to set a defined state
    if(MQTT_HELPER::isNewConnection()) {
        output(gdoor_data_idle, mqtt_topic_bus_rx, true);
    }
    if(rx_data != NULL) {
        JSONDEBUG("Received data from bus");
        GDOOR_DATA_PROTOCOL busmessage = GDOOR_DATA_PROTOCOL(rx_data);
        output(busmessage, mqtt_topic_bus_rx);
        JSONDEBUG("Output bus data via Serial and MQTT, done");
        // Output idle message after bus message, to reset values so that
        //home automation can trigger again
        output(gdoor_data_idle, mqtt_topic_bus_rx, true);

    } else if (!GDOOR::active()) { // Neither RX nor TX active,
        String str_received("");
        if (Serial.available() > 0) { // let's check the serial port if something is in buffer
            str_received = Serial.readString();
        } else {
            str_received = MQTT_HELPER::receive();
        }
        str_received.trim();    

        if(str_received.length() > 0) {
            if(!parse(str_received)) { //Check if received string is a command
                GDOOR::send(str_received); // Send to bus if it is not a command
                JSONDEBUG("Send: ");
                JSONDEBUG(str_received);
            }
        }
        
    }
}
