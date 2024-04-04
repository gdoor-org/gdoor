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
#include <Arduino.h>
#include "src/gdoor.h"
#include <Wifi.h>
#include <WiFiManager.h> 
#include <MQTT.h>

#define PIN_RX 12
#define PIN_TX 25
#define PIN_TX_EN 27

#define DEFAULT_WIFI_SSID     "GDOOR"
#define DEFAULT_WIFI_PASSWORD "12345678"
#define DEFAULT_MQTT_SERVER   "0.0.0.0" 
#define DEFAULT_MQTT_PORT     "1883" 

#define MQTT_TOPIC "/gdoor"

WiFiManager wifiManager;
WiFiManagerParameter custom_mqtt_server("mqtt_server", "MQTT Server", DEFAULT_MQTT_SERVER, 40);
WiFiManagerParameter custom_mqtt_port("mqtt_port", "MQTT Port", DEFAULT_MQTT_PORT, 6);

WiFiClient net;
MQTTClient mqttClient;

void messageReceived(String &topic, String &payload)
{
    Serial.println("incoming: " + topic + " - " + payload);    
}

void setup()
{
    Serial.begin(115200);
    Serial.setTimeout(1);

    Serial.println("GDOOR Setup start");

    GDOOR::setup(PIN_TX, PIN_TX_EN, PIN_RX);

    wifiManager.addParameter(&custom_mqtt_server);
    wifiManager.addParameter(&custom_mqtt_port);
    wifiManager.setBreakAfterConfig(true);
    wifiManager.setConfigPortalBlocking(false);
    wifiManager.setTimeout(300);
    wifiManager.autoConnect(DEFAULT_WIFI_SSID, DEFAULT_WIFI_PASSWORD);
    
    mqttClient.begin(custom_mqtt_server.getValue(), atoi(custom_mqtt_port.getValue()), net); /* TODO: check string before conversion */
    mqttClient.onMessage(messageReceived);
    
    if (!mqttClient.connect("GDoor"))
    {
        delay(1000);
        if (!mqttClient.connect("GDoor")){
            Serial.println("Failed to connect to MQTT broker");
        }
    }

    mqttClient.subscribe("/gdoor/send");

    Serial.println("GDOOR Setup done");

    //Temporay
    mqttClient.publish(MQTT_TOPIC, "Initialized");
}

void loop()
{
    wifiManager.process();
    mqttClient.loop();

    GDOOR::loop();
    GDOOR_RX_DATA *rx_data = GDOOR::read();
    if (rx_data != NULL)
    {
        Serial.print("New data:\n");
        Serial.print(*rx_data);

        /* Quick and dirty as function does not support Printable */
        char buffer[50];
        char* buffer_ptr = buffer;
        for (uint8_t i = 0; i< rx_data->len;i++){
            buffer_ptr += sprintf(buffer_ptr, "%.2x", rx_data->data[i]);
        }
        *buffer_ptr = '\0';
        mqttClient.publish(MQTT_TOPIC, buffer);
    }
    else if (!GDOOR::active())
    { // Neither RX nor TX active,
        if (Serial.available() > 0)
        { // let's check the serial port if something is in buffer
            String serialstr = Serial.readString();
            GDOOR::send(serialstr);
            Serial.print("Send data:");
            Serial.println(serialstr);
        }
    }
}
