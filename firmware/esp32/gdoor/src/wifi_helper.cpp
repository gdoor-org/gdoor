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
#include "wifi.h"
#include <WiFiManager.h>
#include <LittleFS.h>


namespace WIFI_HELPER { //Namespace as we can only use it once
    WiFiManager wifiManager;
    WiFiManagerParameter custom_mqtt_server("mqtt_server", "MQTT Server", DEFAULT_MQTT_SERVER, 40);
    WiFiManagerParameter custom_mqtt_port("mqtt_port", "MQTT Port", DEFAULT_MQTT_PORT, 6);
    WiFiManagerParameter custom_debug("debug", "Debug Mode", "false", 10,"placeholder=\"Custom Field Placeholder\" type=\"checkbox\""); 

    WiFiClient net;

    void setup() {
        wifiManager.addParameter(&custom_mqtt_server);
        wifiManager.addParameter(&custom_mqtt_port);
        wifiManager.addParameter(&custom_debug);
        wifiManager.setBreakAfterConfig(true);
        wifiManager.setConfigPortalBlocking(false);
        wifiManager.setTimeout(300);
        wifiManager.autoConnect(DEFAULT_WIFI_SSID, DEFAULT_WIFI_PASSWORD);
    }

    void loop() {
        wifiManager.process();
    }
};