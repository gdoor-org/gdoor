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
        wifiManager.setCustomHeadElement("<style>h1 { color: transparent; background-repeat: no-repeat; background-size: contain; aspect-ratio: 596 / 351; margin: auto; max-width: 260px; background-image: url('data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSI1OTUuOTUiIGhlaWdodD0iMzUwLjk2Ij48ZyB0cmFuc2Zvcm09InRyYW5zbGF0ZSgtMTAzLjMzIC0xNjMuNSkiPjxyZWN0IHdpZHRoPSI1OTUuOTUiIGhlaWdodD0iMzUwLjk2IiB4PSIxMDMuMzMiIHk9IjE2My41MSIgZmlsbD0iI2Y2MCIgcng9IjE2LjcyIiByeT0iMTMuOTgiLz48cmVjdCB3aWR0aD0iMTYwLjIzIiBoZWlnaHQ9IjI3OS4xNSIgeD0iMTUxLjIyIiB5PSIxOTcuNjkiIGZpbGw9Im5vbmUiIHN0cm9rZT0iI2ZmZiIgc3Ryb2tlLWRhc2hvZmZzZXQ9IjMyLjEzIiBzdHJva2UtbGluZWNhcD0icm91bmQiIHN0cm9rZS1saW5lam9pbj0icm91bmQiIHN0cm9rZS1taXRlcmxpbWl0PSIxMS43IiBzdHJva2Utd2lkdGg9IjE4LjkiIHJ4PSIxOC45IiByeT0iMTguOSIvPjxwYXRoIGZpbGw9IiM2NjYiIHN0cm9rZT0iIzY2NiIgc3Ryb2tlLWRhc2hvZmZzZXQ9IjMyLjEzIiBzdHJva2UtbGluZWNhcD0icm91bmQiIHN0cm9rZS1saW5lam9pbj0icm91bmQiIHN0cm9rZS1taXRlcmxpbWl0PSIxMS43IiBzdHJva2Utd2lkdGg9IjIxLjkyIiBkPSJNMjM0Ljk0IDM5MC42di0xNi45NWw1My4yLS4xdjUzLjZjLTguMTcgNy40OS0xNi42IDEzLjEyLTI1LjI3IDE2LjlhNjYuMzUgNjYuMzUgMCAwIDEtMjYuNzMgNS42NmMtMTIuMzMgMC0yMy41NC0zLjA0LTMzLjYyLTkuMTEtMTAuMDgtNi4wOC0xNy42OS0xNC44Ni0yMi44My0yNi4zNi01LjEzLTExLjUtNy43LTI0LjMzLTcuNy0zOC41MiAwLTE0LjA2IDIuNTUtMjcuMTcgNy42Ni0zOS4zNiA1LjExLTEyLjE4IDEyLjQ3LTIxLjIzIDIyLjA2LTI3LjE0IDkuNi01LjkxIDIwLjY0LTguODcgMzMuMTUtOC44NyA5LjA4IDAgMTcuMjggMS43IDI0LjYyIDUuMDhzMTMuMSA4LjEgMTcuMjYgMTQuMTNjNC4xNyA2LjA1IDcuMzQgMTMuOTMgOS41IDIzLjY1bC0xNC45OCA0LjczYy0xLjg4LTcuMzYtNC4yMi0xMy4xNC03LjAyLTE3LjM0LTIuOC00LjItNi44LTcuNTctMTItMTAuMWEzOS4wMSAzOS4wMSAwIDAgMC0xNy4zLTMuOGMtNy42IDAtMTQuMTYgMS4zNC0xOS43IDRhMzcuMSAzNy4xIDAgMCAwLTEzLjQgMTAuNDkgNTEuNTUgNTEuNTUgMCAwIDAtNy45MyAxNC4yOWMtMy4yIDguOTMtNC44IDE4LjYyLTQuOCAyOS4wNiAwIDEyLjg3IDEuOTMgMjMuNjQgNS43OSAzMi4zMSAzLjg1IDguNjcgOS40NiAxNS4xMSAxNi44MyAxOS4zMWE0Ni42MiA0Ni42MiAwIDAgMCAyMy40NyA2LjMxYzcuMiAwIDE0LjIyLTEuNiAyMS4wNy00Ljc4czEyLjA1LTYuNTggMTUuNTktMTAuMnYtMjYuOWgtMzYuOTJaIi8+PHBhdGggZmlsbD0iI2U2ZTZlNiIgc3Ryb2tlPSIjZTZlNmU2IiBzdHJva2UtZGFzaG9mZnNldD0iMzIuMTMiIHN0cm9rZS1saW5lY2FwPSJyb3VuZCIgc3Ryb2tlLWxpbmVqb2luPSJyb3VuZCIgc3Ryb2tlLW1pdGVybGltaXQ9IjExLjciIHN0cm9rZS13aWR0aD0iMTEuNzciIGQ9Ik0yOTQuNDIgNTA2Ljg1di04NC45MWgyOS4yNXE5LjkgMCAxNS4xMiAxLjIxIDcuMyAxLjY4IDEyLjQ1IDYuMDkgNi43MiA1LjY3IDEwLjA2IDE0LjUgMy4zMiA4Ljg0IDMuMzIgMjAuMiAwIDkuNjYtMi4yNSAxNy4xNC0yLjI2IDcuNDctNS44IDEyLjM2LTMuNTMgNC45LTcuNzMgNy43LTQuMiAyLjgyLTEwLjEzIDQuMjYtNS45NCAxLjQ1LTEzLjY1IDEuNDVabTExLjI0LTEwLjAyaDE4LjEzcTguNCAwIDEzLjE4LTEuNTYgNC43OC0xLjU3IDcuNjEtNC40IDQtNCA2LjIzLTEwLjc1IDIuMjMtNi43NSAyLjIzLTE2LjM2IDAtMTMuMzItNC4zNy0yMC40OC00LjM4LTcuMTUtMTAuNjMtOS41OC00LjUyLTEuNzQtMTQuNTQtMS43NGgtMTcuODRabTY5LjIxLTIwLjczcTAtMTcuMSA5LjUtMjUuMzIgNy45NC02LjgzIDE5LjM1LTYuODMgMTIuNjggMCAyMC43MyA4LjMxIDguMDYgOC4zMSA4LjA2IDIyLjk3IDAgMTEuODctMy41NyAxOC42OC0zLjU2IDYuOC0xMC4zNiAxMC41Ny02LjgxIDMuNzYtMTQuODYgMy43Ni0xMi45MiAwLTIwLjg4LTguMjgtNy45Ny04LjI4LTcuOTctMjMuODZabTEwLjcyIDBxMCAxMS44MSA1LjE1IDE3LjY5IDUuMTYgNS44OCAxMi45OCA1Ljg4IDcuNzYgMCAxMi45MS01LjkgNS4xNi01LjkyIDUuMTYtMTguMDIgMC0xMS40MS01LjE4LTE3LjMtNS4xOS01Ljg3LTEyLjktNS44Ny03LjgxIDAtMTIuOTcgNS44NS01LjE1IDUuODUtNS4xNSAxNy42N1ptNDguNiAwcTAtMTcuMSA5LjUtMjUuMzIgNy45My02LjgzIDE5LjM0LTYuODMgMTIuNjkgMCAyMC43NCA4LjMxIDguMDUgOC4zMSA4LjA1IDIyLjk3IDAgMTEuODctMy41NiAxOC42OC0zLjU3IDYuOC0xMC4zNyAxMC41Ny02LjggMy43Ni0xNC44NiAzLjc2LTEyLjkyIDAtMjAuODgtOC4yOC03Ljk2LTguMjgtNy45Ni0yMy44NlptMTAuNzEgMHEwIDExLjgxIDUuMTYgMTcuNjkgNS4xNSA1Ljg4IDEyLjk3IDUuODggNy43NiAwIDEyLjkyLTUuOSA1LjE1LTUuOTIgNS4xNS0xOC4wMiAwLTExLjQxLTUuMTgtMTcuMy01LjE5LTUuODctMTIuODktNS44Ny03LjgyIDAtMTIuOTcgNS44NS01LjE2IDUuODUtNS4xNiAxNy42N1ptNTIuMzYgMzAuNzV2LTYxLjUxaDkuMzl2OS4zMnEzLjU5LTYuNTQgNi42My04LjYzIDMuMDQtMi4wOCA2LjY5LTIuMDggNS4yNyAwIDEwLjcxIDMuMzZsLTMuNTkgOS42N3EtMy44Mi0yLjI2LTcuNjQtMi4yNi0zLjQyIDAtNi4xNCAyLjA2LTIuNzIgMi4wNS0zLjg4IDUuNy0xLjc0IDUuNTYtMS43NCAxMi4xN3YzMi4yWiIgcGFpbnQtb3JkZXI9InN0cm9rZSIgdHJhbnNmb3JtPSJzY2FsZSgxLjEyOTQ2IC44ODUzOCkiLz48L2c+PC9zdmc+'); }</style>");
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