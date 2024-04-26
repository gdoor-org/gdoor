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
#include "printer_helper.h"

/**
 * Overriden WiFiManager class,
 * to enable back button in web gui.
*/
class MyCustomWifiManager: public WiFiManager {
    public:
        void setBackButton(bool enable) {
            _showBack = enable;
        }
};

/**
 * Custom WiFiManager parameter which
 * allows a HTML select menu.
*/
class CheckSelectParameter : public WiFiManagerParameter {
public:
    String myid;
    String label;
    std::vector<String> selectvalues;
    uint32_t selectlength;

    /**
     * Default constructor.
    */
    CheckSelectParameter()
        : WiFiManagerParameter("") {
    }

    /**
     * Main Constructor
     * @param id ID String
     * @param placeholder Label in GUI
     * @param values a String array with selectable values
     * @param len_values Number og values
     * @param maxlen How much memory to reserve, biggest entry of values.
    */
    CheckSelectParameter(const char *id, const char *placeholder, const char ** values, uint32_t len_values, uint32_t maxlen)
        : WiFiManagerParameter("") {
        selectInit(id, placeholder, values, len_values, maxlen);
    }

    /** 
     * Override Setvalue to skip WiFiManager gibberish checks.
    */
    void setValue(const char *defaultValue, int length) {
        if(length <= _length ){
            memcpy(_value, defaultValue, length);  
        }
    }

    /** Mainly the logic of the constructor,
     * separated as it is easier to use for child classes.
     * 
     * @param id ID String
     * @param placeholder Label in GUI
     * @param values a String array with selectable values
     * @param len_values Number og values
     * @param maxlen How much memory to reserve, biggest entry of values.
    */
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

    /**
     * Overriden WiFiManager function to render select.
    */
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

/**
 * Custom WiFiManager parameter which
 * allows a HTML enable/disable parameter.
*/
class EnableDisableParameter : public CheckSelectParameter {
public:
    
    EnableDisableParameter(const char *id, const char *placeholder)
        :  CheckSelectParameter() {
        const char* endis[] = {"disabled", "enabled"};
        selectInit(id, placeholder, endis, 2, 10);
    }
};

/**
 * Custom WiFiManager parameter which
 * allows to return a nullptr, if the value is empty.
*/
class NullableParameter : public WiFiManagerParameter {
public:
    
    NullableParameter(const char *id, const char *label, const char *defaultValue, int length)
        :  WiFiManagerParameter(id, label, defaultValue, length) {
    }

    const char* getNullableValue() {
        const char* value = getValue();
        if (value[0] == '\0') {
            return nullptr;
        } else {
            return value;
        }
    }
};


namespace WIFI_HELPER { //Namespace as we can only use it once
    bool shouldSaveConfig = false;
    

    MyCustomWifiManager wifiManager;
    WiFiManagerParameter custom_mqtt_server("mqtt_server", "MQTT Server", DEFAULT_MQTT_SERVER, 40);
    WiFiManagerParameter custom_mqtt_port("mqtt_port", "MQTT Port", DEFAULT_MQTT_PORT, 6, "type='number' min=0 max=65535");
    NullableParameter custom_mqtt_user("mqtt_user", "MQTT Username", "", 40);
    NullableParameter custom_mqtt_password("mqtt_password", "MQTT Password", "", 40);
    WiFiManagerParameter custom_mqtt_topic_bus_rx("mqtt_topic_bus_rx", "MQTT Topic - from bus", DEFAULT_MQTT_TOPIC_BUS_RX, 40);
    WiFiManagerParameter custom_mqtt_topic_bus_tx("mqtt_topic_bus_tx", "MQTT Topic - to bus", DEFAULT_MQTT_TOPIC_BUS_TX, 40);
    EnableDisableParameter custom_debug("param_6", "Debug Mode"); //param_4 is a very ugly workaround for stupid WifiManager custom fields implementation. Works only with param_<fixedno>

    /**
     * Internal function which creates and writes a file to LittleFS.
     * @param filename Filename
     * @param value Value
    */
    void save_config_file(const char* filename, const char *value) {
        File file = LittleFS.open(filename, FILE_WRITE, true);
        if (file) {
            file.print(value);
            file.close();
        }
    }

    /**
     * Internal function which reads a files content from LittleFS.
     * @param filename Filename
     * @param s Read Value
     * @return true if read was successful
    */
    bool read_config_file(const char* filename, String *s) {
        File file = LittleFS.open(filename, FILE_READ);
        if(file && !file.isDirectory()){
            *s = file.readString();
            return true;
        }
        return false;
    }

    /**
     * WiFiManager Callback, executed when parameters changed.
    */
    void on_save () {
        shouldSaveConfig = true;
    }

    /** Returns MQTT broker host*/
    const char* mqtt_server(){
        return custom_mqtt_server.getValue();
    }

    /** Returns MQTT broker port*/
    int mqtt_port() {
        const char* strvalue = custom_mqtt_port.getValue();
        return atoi(strvalue);
    }

    /** Returns MQTT Username*/
    const char* mqtt_user(){
        return custom_mqtt_user.getNullableValue();
    }

    /** Returns MQTT Password*/
    const char* mqtt_password(){
        return custom_mqtt_password.getNullableValue();
    }

    /** Returns MQTT topic where bus data is send to (bus rx, MQTT tx)*/
    const char* mqtt_topic_bus_rx(){
        return custom_mqtt_topic_bus_rx.getValue();
    }

    /** Returns MQTT topic where MQTT data is send to the bus (MQTT rx, bus tx)*/
    const char* mqtt_topic_bus_tx(){
        return custom_mqtt_topic_bus_tx.getValue();
    }

    /** Returns true if debug mode is enabled */
    bool debug(){
        return strcmp(custom_debug.getValue(), "enabled") == 0;
    }

    void setup() {
        String filevalue;

        bool filesystem_mounted = LittleFS.begin(true);
        if (filesystem_mounted) {
            if (read_config_file("/custom_mqtt_server", &filevalue) && filevalue.length() > 0) {
                custom_mqtt_server.setValue(filevalue.c_str(), 40);
            }

            if (read_config_file("/custom_mqtt_port", &filevalue) && filevalue.length() > 0) {
                custom_mqtt_port.setValue(filevalue.c_str(), 6);
            }

            if (read_config_file("/custom_mqtt_user", &filevalue) && filevalue.length() > 0) {
                custom_mqtt_user.setValue(filevalue.c_str(), 40);
            }

            if (read_config_file("/custom_mqtt_password", &filevalue) && filevalue.length() > 0) {
                custom_mqtt_password.setValue(filevalue.c_str(), 40);
            }

            if (read_config_file("/custom_mqtt_topic_bus_rx", &filevalue) && filevalue.length() > 0) {
                custom_mqtt_topic_bus_rx.setValue(filevalue.c_str(), 20);
            }

            if (read_config_file("/custom_mqtt_topic_bus_tx", &filevalue) && filevalue.length() > 0) {
                custom_mqtt_topic_bus_tx.setValue(filevalue.c_str(), 20);
            }

            if (read_config_file("/custom_debug", &filevalue) && filevalue.length() > 0 ) {
                custom_debug.setValue(filevalue.c_str(), 10);
            }

            LittleFS.end();
        } else {
            PRINTLN("Could not mount filesystem on load");
        }

        wifiManager.setTitle(GDOOR_LOGO);
        wifiManager.setBackButton(true);
        
        wifiManager.addParameter(&custom_mqtt_server);
        wifiManager.addParameter(&custom_mqtt_port);
        wifiManager.addParameter(&custom_mqtt_user);
        wifiManager.addParameter(&custom_mqtt_password);
        wifiManager.addParameter(&custom_mqtt_topic_bus_rx);
        wifiManager.addParameter(&custom_mqtt_topic_bus_tx);

        wifiManager.addParameter(&custom_debug);

        wifiManager.setSaveConfigCallback(on_save);
        wifiManager.setSaveParamsCallback(on_save);

        wifiManager.setHostname("GDOOR");
        wifiManager.setShowPassword(true);
        
        wifiManager.setBreakAfterConfig(true);
        wifiManager.setConfigPortalBlocking(false);
        wifiManager.setTimeout(300);
        std::vector<const char *> menu = {"wifi","param","sep","update"};
        wifiManager.setMenu(menu);
        wifiManager.setDebugOutput(debug());
        wifiManager.autoConnect(DEFAULT_WIFI_SSID, DEFAULT_WIFI_PASSWORD);
        wifiManager.startWebPortal();
    }

    void loop() {
        wifiManager.process();

        if (shouldSaveConfig) {
            shouldSaveConfig = false;
            bool filesystem_mounted = LittleFS.begin(true);
            if (filesystem_mounted) {
                save_config_file("/custom_mqtt_server", custom_mqtt_server.getValue());
                save_config_file("/custom_mqtt_port", custom_mqtt_port.getValue());
                save_config_file("/custom_mqtt_user", custom_mqtt_user.getValue());
                save_config_file("/custom_mqtt_password", custom_mqtt_password.getValue());
                save_config_file("/custom_mqtt_topic_bus_rx", custom_mqtt_topic_bus_rx.getValue());
                save_config_file("/custom_mqtt_topic_bus_tx", custom_mqtt_topic_bus_tx.getValue());
                save_config_file("/custom_debug", custom_debug.getValue());
                LittleFS.end();
                ESP.restart();
            } else {
                PRINTLN("Could not mount filesystem");
            }
            
        }
    }
};