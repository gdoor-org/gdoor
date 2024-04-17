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
#include "defines.h"
#include "mqtt_helper.h"
#include <MQTT.h>

#include <WiFi.h>
/** 
 * Constructor for MQTT_RINTER.
 * @param mqttClient the parent mqttClient, used to publish data
*/
MQTT_PRINTER::MQTT_PRINTER(MQTTClient *mqttClient) {
    this->mqttClient = mqttClient;
}

/**
 * Sends out the collected data via Serial and MQTT (if available).
 * @param topic The MQTT topic to which the collected data is send.
*/
void MQTT_PRINTER::publish(const char *topic) {
    if (this->mqttClient->connected()) {
        this->mqttClient->publish(topic, this->read());
    }
    Serial.print(this->read());
}

/**
 * Arduino IDE specific write function ti implement prints().
 * @param byte: Byte to write
 * @return Number of bytes written
*/
size_t MQTT_PRINTER::write(uint8_t byte) {
    if(index < BUFFER_SIZE) {
        this->buffer[index] = (char) byte;
        index = index + 1;
        return 1;
    }
    return 0;
}

/** Function which returns collected data.
 * @return string of collected data
*/
char* MQTT_PRINTER::read() {
    if(this->index < BUFFER_SIZE && this->index > 0) {
        this->buffer[index] = '\0'; //just to be sure
    }
    this->index = 0;
    return this->buffer;
}

namespace MQTT_HELPER { //Namespace as we can only use it once
    WiFiClient net; // Arduinio helper object, needed by MQTTClient
    MQTTClient mqttClient; // MQTT Library object

    const char* rx_topic_name; // Which callback topic
    const char* user; // Username
    const char* password; // Password

    MQTT_PRINTER printer(&mqttClient); // Printer, so that code can use the Arduino print functions

    String received_mqtt_payload; // Global variable which stores received MQTT payload
    String empty(""); //Empty string, useful as global and fixed allocated value.

    bool new_string_available = false; // Global variable to indicate that a new MQTT String was received

    bool newly_connected = true; // Global variable to indicate a newly established WIFI connection

    /**
     * MQTT Callback function,
     * executes when a new String is received via the subscribed topic
    */
    void on_message_received(String &topic, String &payload) {
        new_string_available = true;
        received_mqtt_payload = payload;
        received_mqtt_payload.trim();
    }

    /**
     * Internal WiFi Event callback,
     * used to detect if a new Wifi connection was established
    */
    void on_wifi_active(WiFiEvent_t event, WiFiEventInfo_t info) {
        newly_connected = true;
    }

    /**
     * Setup MQTT.
     * @param server MQTT Broker ip/hostname
     * @param port MQTT Broker TCP port
     * @param username MQTT Broker username
     * @param pw MQTT Broker password
     * @param rx_topic Topic from which data is received
    */
    void setup(const char* server, int port, const char* username, const char* pw, const char* rx_topic) {
        WiFi.onEvent(on_wifi_active, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
        mqttClient.begin(server, port, net);
        mqttClient.onMessage(on_message_received);
        rx_topic_name = rx_topic;
        user = username;
        password = pw;
    }

    /**
     * MQTT loop function,
     * needs to be executed in main loop().
    */
    void loop() {
        if(WiFi.getMode() == WIFI_MODE_STA && WiFi.status() == WL_CONNECTED) {
            if (newly_connected) {
                if (mqttClient.connect("GDoor", user, password)) {
                    mqttClient.subscribe(rx_topic_name);
                    newly_connected = false;
                }
            }

            mqttClient.loop();
            if (!mqttClient.connected()) {
                mqttClient.connect("GDoor", user, password);
                mqttClient.subscribe(rx_topic_name);
            }
        }
    }

    /**
     * If a new MQTT message is received, it can be retrievd via this function.
     * Trailing whitespaces of received data is removed.
     * @return String, if nothiung was received, receives an empty String.
    */
    String& receive() {
        if(new_string_available) {
            new_string_available = false;
            return received_mqtt_payload;
        }
        return empty;
    }

};