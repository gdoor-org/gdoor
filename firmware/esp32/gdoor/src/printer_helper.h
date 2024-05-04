
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
#ifndef PRINTER_HELPER_H
#define PRINTER_HELPER_H
#include <Arduino.h>

extern boolean debug;

#define JSONDEBUG(...) { \
            if(debug) { \
                Serial.print("{\"debug\": \""); \
                Serial.print(__VA_ARGS__); \
                Serial.println("\"}"); \
            }\
        }

#define JSONPRINT(...) { \
            Serial.print("{\"message\": \""); \
            Serial.print(__VA_ARGS__); \
            Serial.println("\"}"); \
        }

#define PRINT(...) { \
            Serial.print(__VA_ARGS__); \
        }

#define PRINTLN(...) { \
            Serial.println(__VA_ARGS__); \
        }

#define DEBUG(...) { \
            if(debug) { \
                Serial.print(__VA_ARGS__); \
            }\
        }

#define DEBUGLN(...) { \
            if(debug) { \
                Serial.println(__VA_ARGS__); \
            }\
        }

#endif