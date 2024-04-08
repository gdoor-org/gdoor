
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
#ifndef MQTT_HELPER_H
#define MQTT_HELPER_H
#include <Arduino.h>
#include <MQTT.h>

class MQTT_PRINTER : public Print { // Class/Struct to collect bus related infos
    public:
        MQTTClient *mqttClient;
        char buffer[201];
        uint8_t index = 0;

        MQTT_PRINTER(MQTTClient *mqttClient) {
            this->mqttClient = mqttClient;
        }

        void publish(char *topic) {
            this->mqttClient->publish(topic, this->read());
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
#endif