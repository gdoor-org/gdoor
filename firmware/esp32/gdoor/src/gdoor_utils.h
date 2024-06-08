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
#ifndef GDOOR_UTILS_H
#define GDOOR_UTILS_H
#include <Arduino.h>

namespace GDOOR_UTILS {
    uint8_t crc(uint8_t *words, uint16_t len);
    uint8_t parity_odd(uint8_t word);

    /*
    * Template Function (needs to live in header file),
    * used to print out json hex array.
    * 
    * "keyname": {"0xdata[0]", ..., "0xdata[len-1]"}
    */
    template<typename T> size_t print_json_hexarray(Print& p, const char *keyname, const T* data, const uint16_t len) {
        size_t r = 0;
        r+= p.print("\"");
        r+= p.print(keyname);
        r+= p.print("\": [");
        for(uint16_t i=0; i<len; i++) {
            r+= p.print("\"0x");
            r+= p.print(data[i], HEX);
            if (i==len-1) {
                r+= p.print("\"");
            } else {
                r+= p.print("\", ");
            }
        }
        r+= p.print("]");
        return r;
    }

    template<typename T> size_t print_json_value(Print& p, const char *keyname, const T &value) {
        size_t r = 0;
        r+= p.print("\"");
        r+= p.print(keyname);
        r+= p.print("\": \"");
        r+= p.print(value);
        r+= p.print("\"");
        return r;
    }

    template<typename T> size_t print_json_hexstring(Print& p, const char *keyname, const T* data, const uint16_t len) {
        size_t r = 0;
        r+= p.print("\"");
        r+= p.print(keyname);
        r+= p.print("\": \"");
        for(uint16_t i=0; i<len; i++) {
            if(data[i] < 16) {
                r+= p.print("0");
            }
            r+= p.print(data[i], HEX);
        }
        r+= p.print("\"");
        return r;
    }

    template<typename T> size_t print_json_bool(Print& p, const char *keyname, const T value) {
        size_t r = 0;
        r+= p.print("\"");
        r+= p.print(keyname);
        r+= p.print("\": ");
        if(value) {
            r+= p.print("true");
        } else {
            r+= p.print("false");
        }
        r+= p.print("");
        return r;
    }

    size_t print_json_string(Print& p, const char *keyname, const char *value);
}

#endif