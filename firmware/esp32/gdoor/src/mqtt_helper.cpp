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
#include "printer_helper.h"
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
    JSONDEBUG("MQTT_PRINTER publish()");
    if (this->mqttClient->connected()) {
        this->mqttClient->publish(topic, this->read());
    }
    PRINT(this->read());
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
    } else {
        JSONDEBUG("!!WARNING MQTT_PRINTER OUTPUT OVERFLOW, LOOSING DATA!!");
    }
    return 0;
}

/** Function which returns collected data.
 * @return string of collected data
*/
char* MQTT_PRINTER::read() {
    JSONDEBUG("MQTT_PRINTER read()");
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
    const char* tx_topic_name; // For HA discovery message
    const char* user; // Username
    const char* password; // Password

    MQTT_PRINTER printer(&mqttClient); // Printer, so that code can use the Arduino print functions

    String received_mqtt_payload; // Global variable which stores received MQTT payload
    String empty(""); //Empty string, useful as global and fixed allocated value.
    String availability_topic; //Topic where availabiliy is shown

    bool new_string_available = false; // Global variable to indicate that a new MQTT String was received

    bool newly_connected = true; // Global variable to indicate a newly established WIFI connection

    /**
     * Function which sends home assistant discovery message,
     * e.g.
     * {
     *  "name": "Bus Data",
     *  "force_update": True,
     *  "icon": "mdi:door",
     *  "state_topic": "gdoor/rx",
     *  "command_topic": "gdoor/tx",
     *  "json_attributes_topic": "gdoor/rx",
     *  "value_template": "{{ value_json.action }}",
     *  "uniq_id": "gdoor_data_mac",
     *  "device": {
     *      "sw_version": "3.0",
     *      "name": "GDOOR Adapter",
     *      "model": "ESP32 (de:ad:be:ef)",
     *      "manufacturer": "GDOOR Project",
     *      "configuration_url": "http://127.0.0.1",
     *      "ids": "gdoor_mac",
     *      "availability_topic": "homeassistant/sensor/gdoor/data/config/mac"
     *  }
     * }
    */
    void send_ha_discovery(){
        String mac = WiFi.macAddress();
        String mac_clean = mac;
        mac_clean.replace(":", "_");

        availability_topic = "homeassistant/sensor/gdoor/data/config/"+mac_clean;

        String ip;
        ip = WiFi.localIP().toString();

        String message = 
R"""(
{
"name": "Bus Data",
"force_update": True,
"icon": "mdi:door",
"value_template": "{{ value_json.action }}",
"device": {
"sw_version": "3.0",
"name": "GDOOR Adapter",
"manufacturer": "GDOOR Project",
)"""
"";

        message += "\"model\": \"ESP32 (" + mac + ")\",";
        message += "\"configuration_url\": \"" + ip + "\",";
        message += "\"ids\": \"gdoor_" + mac_clean + "\",";
        message += "\"availability_topic\": \"" + availability_topic + "\"";
        message += "},";
        message += "\"uniq_id\": \"gdoor_data_" + mac_clean + "\",";
        message += "\"state_topic\": \"" + String(rx_topic_name) + "\",";
        message += "\"json_attributes_topic\": \"" + String(tx_topic_name) + "\",";
        message += "\"command_topic\": \"" + String(tx_topic_name) + "\"";
        message += "}";
        mqttClient.publish("homeassistant/sensor/gdoor/data/config", message, true, 1);

        
        mqttClient.setWill(availability_topic.c_str(), "offline");
    }
   

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
     * @param tx_topic Topic from which is announced in HA discovery as bus outgoing topic
    */
    void setup(const char* server, int port, const char* username, const char* pw, const char* rx_topic, const char* tx_topic) {
        WiFi.onEvent(on_wifi_active, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
        
        mqttClient.begin(server, port, net);
        mqttClient.onMessage(on_message_received);
        rx_topic_name = rx_topic;
        tx_topic_name = tx_topic;
        user = username;
        password = pw;
    }

    /**
     * MQTT loop function,
     * needs to be executed in main loop().
    */
    void loop() {
        static uint16_t counter = 0;
        if(WiFi.getMode() == WIFI_MODE_STA && WiFi.status() == WL_CONNECTED) {
            if (newly_connected) {
                JSONDEBUG("Newly connected WIFI detected in MQTT loop");
                if (mqttClient.connect("GDoor", user, password)) {
                    JSONDEBUG("Successfully connected MQTT");
                    mqttClient.subscribe(rx_topic_name);

                    send_ha_discovery();

                    newly_connected = false;
                    counter = 0;
                }
            }

            // Send every 2^16 a availability message
            if (! counter && !newly_connected ) {
                mqttClient.publish(availability_topic, "online");
            }

            mqttClient.loop();
            if (!mqttClient.connected()) {
                JSONDEBUG("MQTT lost connection");
                newly_connected = true;
            }
            counter = counter + 1;
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