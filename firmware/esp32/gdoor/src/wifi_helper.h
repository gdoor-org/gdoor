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

#ifndef WIFI_H
#define WIFI_H
#include <Arduino.h>

namespace WIFI_HELPER { //Namespace as we can only use it once
    void loop();
    void setup();
    const char* mqtt_server();
    int mqtt_port();
    const char* mqtt_user();
    const char* mqtt_password();
    const char* mqtt_topic_bus_rx();
    const char* mqtt_topic_bus_tx();
    bool debug();
    uint8_t rx_pin();
    float rx_sensitivity();
};

#endif