
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

#define BUFFER_SIZE 2048

class MQTT_PRINTER : public Print { // Class/Struct to collect bus related infos
    public:
        MQTTClient *mqttClient;
        char buffer[BUFFER_SIZE + 1];
        uint16_t index = 0;

        MQTT_PRINTER(MQTTClient *mqttClient);

        void publish(const char *topic);
        size_t write(uint8_t byte);
        char* read();
};

namespace MQTT_HELPER { //Namespace as we can only use it once
    extern MQTT_PRINTER printer;

    void setup(const char* server, int port, const char* username, const char* pw, const char* rx_topic);
    String& receive();
    void loop();

};

#endif