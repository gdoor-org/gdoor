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
#include "wifi_helper.h"
#include <WiFiManager.h>
#include <LittleFS.h>


namespace WIFI_HELPER { //Namespace as we can only use it once
    bool shouldSaveConfig = false;

    WiFiManager wifiManager;
    WiFiManagerParameter custom_mqtt_server("mqtt_server", "MQTT Server", DEFAULT_MQTT_SERVER, 40);
    WiFiManagerParameter custom_mqtt_port("mqtt_port", "MQTT Port", DEFAULT_MQTT_PORT, 6);
    WiFiManagerParameter custom_mqtt_topic("mqtt_topic", "MQTT TOPIC", DEFAULT_MQTT_TOPIC, 20);
    WiFiManagerParameter custom_debug("debug", "Debug Mode", "false", 6, "placeholder=\"Custom Field Placeholder\" type=\"checkbox\"");

    void save_config_file(const char* filename, const char *value) {
        File file = LittleFS.open(filename, FILE_WRITE);
        if (file) {
            file.print(value);
            file.close();
        }
    }

    void read_config_file(const char* filename, String *s) {
        File file = LittleFS.open(filename, FILE_READ);
        if(file && !file.isDirectory()){
            *s = file.readString();
        }
    }

    void on_save () {
        shouldSaveConfig = true;
    }

    void setup() {
        String filevalue;

        /*bool filesystem_mounted = LittleFS.begin(true);
        if (filesystem_mounted) {
            read_config_file("custom_mqtt_server", &filevalue);
            if (filevalue.length() > 0) {
                custom_mqtt_server.setValue(filevalue.c_str(), 40);
            }

            read_config_file("custom_mqtt_port", &filevalue);
            if (filevalue.length() > 0) {
                custom_mqtt_port.setValue(filevalue.c_str(), 6);
            }

            read_config_file("custom_mqtt_topic", &filevalue);
            if (filevalue.length() > 0) {
                custom_mqtt_topic.setValue(filevalue.c_str(), 20);
            }

            read_config_file("custom_debug", &filevalue);
            if (filevalue.length() > 0) {
                custom_debug.setValue(filevalue.c_str(), 6);
            }

            LittleFS.end();
        }
        
        wifiManager.addParameter(&custom_mqtt_server);
        wifiManager.addParameter(&custom_mqtt_port);
        wifiManager.addParameter(&custom_mqtt_topic);
        wifiManager.addParameter(&custom_debug);
        wifiManager.setSaveConfigCallback(on_save);*/
        wifiManager.setBreakAfterConfig(true);
        wifiManager.setConfigPortalBlocking(false);
        wifiManager.setTimeout(300);
        //wifiManager.autoConnect(DEFAULT_WIFI_SSID, DEFAULT_WIFI_PASSWORD);

        
    }

    const char* mqtt_server(){
        return custom_mqtt_server.getValue();
    }

    int mqtt_port() {
        const char* strvalue = custom_mqtt_port.getValue();
        return atoi(strvalue);
    }

    const char* mqtt_topic(){
        return custom_mqtt_topic.getValue();
    }

    bool debug(){
        return custom_debug.getValue();
    }

    void loop() {
        wifiManager.process();

        if (shouldSaveConfig) {
            shouldSaveConfig = false;
            bool filesystem_mounted = LittleFS.begin(true);
            if (filesystem_mounted) {
                save_config_file("/custom_mqtt_server", custom_mqtt_server.getValue());
                save_config_file("/custom_mqtt_port", custom_mqtt_port.getValue());
                save_config_file("/custom_mqtt_topic", custom_mqtt_topic.getValue());
                save_config_file("/custom_debug", custom_debug.getValue());
                LittleFS.end();
                //ESP.restart();
            }
            
        }
    }
};