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
#ifndef DEFINES_H
#define DEFINES_H

// GDoor
#define GDOOR_VERSION "dev"
#define MAX_WORDLEN 25

// RX Statemachine
#define FLAG_RX_ACTIVE 0x01
#define FLAG_BITSTREAM_RECEIVED 0x02
#define FLAG_BITSTREAM_CONVERTED 0x04
#define FLAG_DATA_READY 0x08

// RX
#define BIT_ONE_DIV 2.5
#define BIT_MIN_LEN 5
#define STARTBIT_MIN_LEN 45

// TX
#define STARTBIT_PULSENUM 66
#define ONE_PULSENUM 16
#define ZERO_PULSENUM 37
#define PAUSE_PULSENUM 30

#define STATE_SENDING 0x01

// WIFI
#define DEFAULT_WIFI_SSID     "GDoor"
#define DEFAULT_WIFI_PASSWORD "12345678"

// MQTT
#define DEFAULT_MQTT_SERVER   "0.0.0.0" 
#define DEFAULT_MQTT_PORT     "1883" 
#define DEFAULT_MQTT_TOPIC_BUS_RX "gdoor/bus_rx"
#define DEFAULT_MQTT_TOPIC_BUS_TX "gdoor/bus_tx"

// Settings

#define PIN_TX 25
#define PIN_TX_EN 27
#define PIN_RX_THRESH 26

// RX Pin Settings
#define RX_PIN_22_NAME "v3.1 adjustable (IO22)"
#define RX_PIN_22_NUM 22

#define RX_PIN_21_NAME "v3.1 (IO21)"
#define RX_PIN_21_NUM 21

#define RX_PIN_12_NAME "v3.0 bugfix (IO12)"
#define RX_PIN_12_NUM 12

#define RX_PIN_32_NAME "v3.0 (IO32)"
#define RX_PIN_32_NUM 32

#define RX_PIN_CHOICES {RX_PIN_22_NAME, RX_PIN_21_NAME, RX_PIN_12_NAME, RX_PIN_32_NAME}
#define RX_PIN_CHOICES_LEN 4

// RX Pin Sensitivity (only working for RX PIN22)
#define RX_SENS_LOW_NAME "Low (1.3V)"
#define RX_SENS_LOW_NUM 1.3
#define RX_SENS_MED_NAME "Med (1.45V)"
#define RX_SENS_MED_NUM 1.45
#define RX_SENS_HIGH_NAME "High (1.65V)"
#define RX_SENS_HIGH_NUM 1.65

#define RX_SENS_CHOICES {RX_SENS_HIGH_NAME, RX_SENS_MED_NAME, RX_SENS_LOW_NAME}
#define RX_SENS_CHOICES_LEN 3

#endif