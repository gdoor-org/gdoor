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

class MyCustomWifiManager: public WiFiManager {
    public:
        void setBackButton(bool enable) {
            _showBack = enable;
        }
};

class CheckSelectParameter : public WiFiManagerParameter {
public:
    String myid;
    String label;
    std::vector<String> selectvalues;
    uint32_t selectlength;

    CheckSelectParameter()
        : WiFiManagerParameter("") {
    }

    CheckSelectParameter(const char *id, const char *placeholder, const char ** values, uint32_t len_values, uint32_t maxlen)
        : WiFiManagerParameter("") {
        selectInit(id, placeholder, values, len_values, maxlen);
    }

    void setValue(const char *defaultValue, int length) {
        if(length <= _length ){
            memcpy(_value, defaultValue, length);  
        }
    }

    void selectInit(const char *id, const char *placeholder, const char ** values, uint32_t len_values, uint32_t maxlen) {
        myid = id;
        label = placeholder;
        
        selectlength = len_values;

        for(int i=0; i<len_values; i++) {
            selectvalues.push_back((values[i]));
        }
        _value  = new char[maxlen + 1];
        _length = maxlen;
    }

    virtual const char *getCustomHTML() const {
        static String pre_select = "";
        pre_select = "";
        pre_select += "<br/><label for='" + myid + "'>" + label + "</label>";
        pre_select += "<select name='" + myid + "' id='" + myid + "'>";

        for(String value : selectvalues) {
            String selected = "";
            if(value == _value) {
                selected = " selected";
            }
            pre_select += "<option value='" + value + "'" + selected +">" + value + "</option>";
        }
        pre_select += "</select>";

        return pre_select.c_str();
    }

};

class EnableDisableParameter : public CheckSelectParameter {
public:
    
    EnableDisableParameter(const char *id, const char *placeholder)
        :  CheckSelectParameter() {
        const char* endis[] = {"disabled", "enabled"};
        selectInit(id, placeholder, endis, 2, 10);
    }
};


namespace WIFI_HELPER { //Namespace as we can only use it once
    bool shouldSaveConfig = false;
    

    MyCustomWifiManager wifiManager;
    WiFiManagerParameter custom_mqtt_server("mqtt_server", "MQTT Server", DEFAULT_MQTT_SERVER, 40);
    WiFiManagerParameter custom_mqtt_port("mqtt_port", "MQTT Port", DEFAULT_MQTT_PORT, 6, "type='number' min=0 max=65535");
    WiFiManagerParameter custom_mqtt_topic("mqtt_topic", "MQTT TOPIC", DEFAULT_MQTT_TOPIC, 20);
    EnableDisableParameter custom_debug("param_3", "Debug Mode"); //param_3 ugly workaround for stupid WifiManager Bug custom fields only with param_<fixedno>

    void save_config_file(const char* filename, const char *value) {
        File file = LittleFS.open(filename, FILE_WRITE, true);
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

        bool filesystem_mounted = LittleFS.begin(true);
        if (filesystem_mounted) {
            read_config_file("/custom_mqtt_server", &filevalue);
            if (filevalue.length() > 0) {
                custom_mqtt_server.setValue(filevalue.c_str(), 40);
            }

            read_config_file("/custom_mqtt_port", &filevalue);
            if (filevalue.length() > 0) {
                custom_mqtt_port.setValue(filevalue.c_str(), 6);
            }

            read_config_file("/custom_mqtt_topic", &filevalue);
            if (filevalue.length() > 0) {
                custom_mqtt_topic.setValue(filevalue.c_str(), 20);
            }

            read_config_file("/custom_debug", &filevalue);
            if (filevalue.length() > 0) {
                custom_debug.setValue(filevalue.c_str(), 10);
            }

            LittleFS.end();
        } else {
            Serial.println("Could not mount filesystem on load");
        }

        wifiManager.setTitle(GDOOR_LOGO);
        wifiManager.setBackButton(true);
        
        wifiManager.addParameter(&custom_mqtt_server);
        wifiManager.addParameter(&custom_mqtt_port);
        wifiManager.addParameter(&custom_mqtt_topic);

        wifiManager.addParameter(&custom_debug);

        wifiManager.setSaveConfigCallback(on_save);
        wifiManager.setSaveParamsCallback(on_save);
        
        wifiManager.setBreakAfterConfig(true);
        wifiManager.setConfigPortalBlocking(false);
        wifiManager.setTimeout(300);
        std::vector<const char *> menu = {"wifi","param","sep","update"};
        wifiManager.setMenu(menu);
        //wifiManager.setDebugOutput(false);
        wifiManager.autoConnect(DEFAULT_WIFI_SSID, DEFAULT_WIFI_PASSWORD);
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
        return strcmp(custom_debug.getValue(), "enabled") == 0;
    }

    void loop() {
        wifiManager.process();

        /*if (shouldSaveConfig) {
            shouldSaveConfig = false;
            bool filesystem_mounted = LittleFS.begin(true);
            if (filesystem_mounted) {
                save_config_file("/custom_mqtt_server", custom_mqtt_server.getValue());
                save_config_file("/custom_mqtt_port", custom_mqtt_port.getValue());
                save_config_file("/custom_mqtt_topic", custom_mqtt_topic.getValue());
                save_config_file("/custom_debug", custom_debug.getValue());
                LittleFS.end();
                //ESP.restart();
            } else {
                Serial.println("Could not mount filesystem");
            }
            
        }*/
    }
};