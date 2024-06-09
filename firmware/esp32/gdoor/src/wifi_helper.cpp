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
    const char* rx_pin_select_values[] = RX_PIN_CHOICES;    
    const char* rx_sensitivity_select_values[] = RX_SENS_CHOICES;    

    MyCustomWifiManager wifiManager;
    WiFiManagerParameter custom_mqtt_server("mqtt_server", "MQTT Server", DEFAULT_MQTT_SERVER, 40);
    WiFiManagerParameter custom_mqtt_port("mqtt_port", "MQTT Port", DEFAULT_MQTT_PORT, 6, "type='number' min=0 max=65535");
    NullableParameter custom_mqtt_user("mqtt_user", "MQTT Username (optional)", "", 40);
    NullableParameter custom_mqtt_password("mqtt_password", "MQTT Password (optional)", "", 40);
    WiFiManagerParameter custom_mqtt_topic_bus_rx("mqtt_topic_bus_rx", "MQTT Topic - from bus", DEFAULT_MQTT_TOPIC_BUS_RX, 40);
    WiFiManagerParameter custom_mqtt_topic_bus_tx("mqtt_topic_bus_tx", "MQTT Topic - to bus", DEFAULT_MQTT_TOPIC_BUS_TX, 40);
    EnableDisableParameter custom_debug("param_6", "Debug Mode"); //param_4 is a very ugly workaround for stupid WifiManager custom fields implementation. Works only with param_<fixedno>
    CheckSelectParameter custom_rx_pin("param_7", "RX Input", rx_pin_select_values, RX_PIN_CHOICES_LEN, 40); 
    CheckSelectParameter custom_rx_sens("param_8", "IO22 Sensitivity", rx_sensitivity_select_values, RX_SENS_CHOICES_LEN, 40); 

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

    /** Returns io number of select RX method*/
    uint8_t rx_pin() {
        const char* value = custom_rx_pin.getValue();

        if(!strcmp(value, RX_PIN_22_NAME)) {
            return RX_PIN_22_NUM;
        } else if(!strcmp(value, RX_PIN_21_NAME)) {
            return RX_PIN_21_NUM;
        } else if(!strcmp(value, RX_PIN_12_NAME)) {
            return RX_PIN_12_NUM;
        } else if(!strcmp(value, RX_PIN_32_NAME)) {
            return RX_PIN_32_NUM;
        }
        return RX_PIN_22_NUM;
    }

    /** Returns DAC value for RX comparator*/
    float rx_sensitivity() {
        const char* value = custom_rx_sens.getValue();
        if(!strcmp(value, RX_SENS_LOW_NAME)) {
            return RX_SENS_LOW_NUM;
        } else if(!strcmp(value, RX_SENS_MED_NAME)) {
            return RX_SENS_MED_NUM;
        } else if(!strcmp(value, RX_SENS_HIGH_NAME)) {
            return RX_SENS_HIGH_NUM;
        }
        return RX_SENS_MED_NUM;
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

            if (read_config_file("/custom_rx_pin", &filevalue) && filevalue.length() > 0 ) {
                custom_rx_pin.setValue(filevalue.c_str(), 40);
            }

            if (read_config_file("/custom_rx_sens", &filevalue) && filevalue.length() > 0 ) {
                custom_rx_sens.setValue(filevalue.c_str(), 40);
            }

            LittleFS.end();
        } else {
            JSONPRINT("Could not mount filesystem on load");
        }

        wifiManager.setTitle("GDoor");
        
        wifiManager.setCustomHeadElement("<style>h1 { color: transparent; background-repeat: no-repeat; background-size: contain; aspect-ratio: 596 / 351; margin: auto; max-width: 260px; background-image: url(\"data:image/svg+xml,%0A%3Csvg xmlns='http://www.w3.org/2000/svg' width='595.95' height='350.96'%3E%3Cg transform='translate(-103.32 -163.52)'%3E%3Crect width='595.95' height='350.96' x='103.32' y='163.53' fill='%23f60' rx='16.72' ry='13.98'/%3E%3Crect width='160.23' height='279.15' x='151.22' y='197.69' fill='none' stroke='%23fff' stroke-dashoffset='32.13' stroke-linecap='round' stroke-linejoin='round' stroke-miterlimit='11.7' stroke-width='18.9' rx='18.9' ry='18.9'/%3E%3Cpath stroke='%23666' stroke-dashoffset='32.13' stroke-linecap='round' stroke-linejoin='round' stroke-miterlimit='11.7' stroke-width='21.92' d='M176.05 811.8h49.4v70q-11.6 3.8-23.4 5.6-11.8 1.8-26.8 1.8-22.2 0-37.4-8.8-15.2-9-23.2-25.4-7.8-16.6-7.8-39.2 0-22.4 8.8-38.8t25.2-25.4q16.6-9.2 40-9.2 12 0 22.6 2.2 10.8 2.2 20 6.2l-6.8 15.6q-7.6-3.4-17.2-5.8-9.4-2.4-19.6-2.4-25.6 0-40 15.4-14.2 15.4-14.2 42.2 0 17 5.4 30.2 5.6 13 17.4 20.4 11.8 7.2 31 7.2 9.4 0 16-1t12-2.4v-42.4h-31.4z' aria-label='G' style='shape-padding:.90503' transform='matrix(.8771 0 0 1.00886 86.59 -447.81)'/%3E%3Cpath stroke='%23e6e6e6' stroke-dashoffset='32.13' stroke-linecap='round' stroke-linejoin='round' stroke-miterlimit='11.7' stroke-width='11.77' d='M364.63 463.67q0 21.47-11.74 32.39-11.63 10.8-32.5 10.8h-23.61v-84.7h26.1q12.8 0 22.18 4.74 9.37 4.74 14.47 14 5.1 9.13 5.1 22.77zm-11.27.36q0-16.97-8.42-24.8-8.3-7.94-23.6-7.94h-13.89v66.43h11.51q34.4 0 34.4-33.7zm83.87 10.91q0 15.78-8.07 24.44-7.94 8.66-21.59 8.66-8.42 0-15.06-3.8-6.53-3.91-10.32-11.27-3.8-7.47-3.8-18.03 0-15.78 7.95-24.32 7.95-8.54 21.59-8.54 8.66 0 15.18 3.92 6.65 3.8 10.33 11.15 3.8 7.23 3.8 17.8zm-48.04 0q0 11.27 4.39 17.91 4.5 6.53 14.23 6.53 9.61 0 14.12-6.53 4.5-6.64 4.5-17.9 0-11.28-4.5-17.68-4.51-6.41-14.24-6.41-9.72 0-14.11 6.4-4.4 6.41-4.4 17.68zm119.81 0q0 15.78-8.07 24.44-7.94 8.66-21.59 8.66-8.42 0-15.06-3.8-6.53-3.91-10.32-11.27-3.8-7.47-3.8-18.03 0-15.78 7.95-24.32 7.95-8.54 21.59-8.54 8.66 0 15.18 3.92 6.65 3.8 10.32 11.15 3.8 7.23 3.8 17.8zm-48.04 0q0 11.27 4.39 17.91 4.5 6.53 14.23 6.53 9.6 0 14.12-6.53 4.5-6.64 4.5-17.9 0-11.28-4.5-17.68-4.51-6.41-14.24-6.41t-14.11 6.4q-4.4 6.41-4.4 17.68zm94.18-32.86c1.2 0 2.46.08 3.8.24 1.42.08 2.65.24 3.68.47l-1.3 9.61a34.33 34.33 0 0 0-6.89-.83c-3.24 0-6.28.91-9.13 2.73a19.1 19.1 0 0 0-6.88 7.47c-1.66 3.17-2.5 6.88-2.5 11.16v33.92H525.5v-63.58h8.54l1.19 11.62-1.86 10.93c2.05-3.48 7.04-17.41 10.28-19.94a18.15 18.15 0 0 1 11.5-3.8h0Z' aria-label='Door' transform='scale(1.12946 .88538)'/%3E%3C/g%3E%3C/svg%3E\"); }</style>");
        wifiManager.setBackButton(true);
        
        wifiManager.addParameter(&custom_mqtt_server);
        wifiManager.addParameter(&custom_mqtt_port);
        wifiManager.addParameter(&custom_mqtt_user);
        wifiManager.addParameter(&custom_mqtt_password);
        wifiManager.addParameter(&custom_mqtt_topic_bus_rx);
        wifiManager.addParameter(&custom_mqtt_topic_bus_tx);
        
        wifiManager.addParameter(&custom_debug);

        wifiManager.addParameter(&custom_rx_pin);
        wifiManager.addParameter(&custom_rx_sens);

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
                save_config_file("/custom_rx_pin", custom_rx_pin.getValue());
                save_config_file("/custom_rx_sens", custom_rx_sens.getValue());
                LittleFS.end();
                ESP.restart();
            } else {
                JSONPRINT("Could not mount filesystem");
            }
            
        }
    }
};