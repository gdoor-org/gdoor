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

#include "mqtt_helper.h"
#include <MQTT.h>

#include <WiFi.h>

class MQTT_PRINTER : public Print { // Class/Struct to collect bus related infos
    public:
        MQTTClient *mqttClient;
        char buffer[201];
        uint8_t index = 0;

        MQTT_PRINTER(MQTTClient *mqttClient) {
            this->mqttClient = mqttClient;
        }

        void publish(char *topic) {
            if (this->mqttClient->connected()) {
                this->mqttClient->publish(topic, this->read());
            }
            Serial.print(this->read());
        }

        size_t write(uint8_t byte) {
            if(index < 200) {
                this->buffer[index] = (char) byte;
                index = index + 1;
                return 1;
            }
            return 0;
        }

        char* read() {
            if(this->index < 200 && this->index > 0) {
                this->buffer[index] = '\0'; //just to be sure
            }
            this->index = 0;
            return this->buffer;
        }
};

namespace MQTT_HELPER { //Namespace as we can only use it once
    MQTTClient mqttClient;

    MQTT_PRINTER printer(&mqttClient);

    bool newly_connected = false;

    void on_message_received(String &topic, String &payload) {
        Serial.println("incoming: " + topic + " - " + payload);  
    }

    void on_wifi_active(WiFiEvent_t event, WiFiEventInfo_t info) {
        newly_connected = true;
    }

    void setup() {
        WiFi.onEvent(on_wifi_active, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
        mqttClient.begin(custom_mqtt_server.getValue(), atoi(custom_mqtt_port.getValue()), net); /* TODO: check string before conversion */
        mqttClient.onMessage(on_message_received);
    }

    void loop() {
        if (newly_connected) {
            if (mqttClient.connect("GDoor")) {
                mqttClient.subscribe("/gdoor/send");
                newly_connected = false;
            }
        }

        mqttClient.loop();
        if (!mqttClient.connected()) {
            mqttClient.connect("GDoor");
            mqttClient.subscribe("/gdoor/send");
        }
    }

};