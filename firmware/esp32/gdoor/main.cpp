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

#define DEFAULT_WIFI_SSID       "GDoor"
#define DEFAULT_WIFI_PASSWORD   "12345678"

#define MQTT_TOPIC      "/gdoor"


WiFiManager wifiManager;
WiFiClient net;
MQTTClient mqttClient;

unsigned long lastMillis = 0;

void messageReceived(String &topic, String &payload) {
  Serial.println("incoming: " + topic + " - " + payload);

}

void setup() {
    Serial.begin(115200);
    Serial.setTimeout(1);
    
    Serial.println("GDOOR Setup start");

    wifiManager.autoConnect(DEFAULT_WIFI_SSID, DEFAULT_WIFI_PASSWORD);

    mqttClient.begin("192.168.10.249", 1884, net);
    mqttClient.onMessage(messageReceived);

    while (!mqttClient.connect("arduino")) {
        Serial.print(".");
        delay(1000);
    } 

    mqttClient.subscribe(MQTT_TOPIC);

    GDOOR::setup(PIN_TX, PIN_TX_EN, PIN_RX);
    Serial.println("GDOOR Setup done");
}

void loop() {
    GDOOR::loop();
    GDOOR_RX_DATA* rx_data = GDOOR::read();
    if(rx_data != NULL) {
        Serial.print("New data:\n");
        Serial.print(*rx_data);
        mqttClient.publish(MQTT_TOPIC, "1");
    } else if (!GDOOR::active()) { //Neither RX nor TX active,
        if (Serial.available() > 0) { // let's check the serial port if something is in buffer
            String serialstr = Serial.readString();
            GDOOR::send(serialstr);
            Serial.print("Send data:");
            Serial.println(serialstr);
        }
    }

}
