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

MQTT_PRINTER::MQTT_PRINTER(MQTTClient *mqttClient) {
    this->mqttClient = mqttClient;
}

void MQTT_PRINTER::publish(const char *topic) {
    if (this->mqttClient->connected()) {
        this->mqttClient->publish(topic, this->read());
    }
    Serial.print(this->read());
}

size_t MQTT_PRINTER::write(uint8_t byte) {
    if(index < 200) {
        this->buffer[index] = (char) byte;
        index = index + 1;
        return 1;
    }
    return 0;
}

char* MQTT_PRINTER::read() {
    if(this->index < 200 && this->index > 0) {
        this->buffer[index] = '\0'; //just to be sure
    }
    this->index = 0;
    return this->buffer;
}

namespace MQTT_HELPER { //Namespace as we can only use it once
    WiFiClient net;
    MQTTClient mqttClient;

    MQTT_PRINTER printer(&mqttClient);

    String received_mqtt_payload;
    String empty("");

    bool new_string_available = false;

    bool newly_connected = true;

    void on_message_received(String &topic, String &payload) {
        new_string_available = true;
        received_mqtt_payload = payload;
        received_mqtt_payload.trim();
    }

    void on_wifi_active(WiFiEvent_t event, WiFiEventInfo_t info) {
        newly_connected = true;
    }

    void setup(String &server, int port) {
        WiFi.onEvent(on_wifi_active, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
        mqttClient.begin(server.c_str(), port, net);
        mqttClient.onMessage(on_message_received);
    }

    void loop() {
        if(WiFi.getMode() == WIFI_MODE_STA && WiFi.status() == WL_CONNECTED) {
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
    }

    String& receive() {
        if(new_string_available) {
            new_string_available = false;
            return received_mqtt_payload;
        }
        return empty;
    }

};