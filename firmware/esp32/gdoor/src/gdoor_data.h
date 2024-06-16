
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
#ifndef GDOOR_DATA_H

#define GDOOR_DATA_H
#include <map>
#include <Arduino.h>
#include "defines.h"
#include "gdoor_utils.h"

extern boolean debug;

class GDOOR_DATA : public Printable { // Class/Struct to collect bus related infos
    public:
        uint16_t len;
        uint8_t data[MAX_WORDLEN];
        uint16_t raw[MAX_WORDLEN*9];
        uint8_t valid;

        boolean parse(uint16_t *counts, uint16_t len);

        virtual size_t printTo(Print& p) const {
            size_t r = 0;

            // Json compatible output
            r+= GDOOR_UTILS::print_json_hexstring<uint8_t>(p, "busdata", data, len);
            r+= p.print(", ");

            r+= GDOOR_UTILS::print_json_hexarray<uint16_t>(p, "raw", raw, len*9);
            r+= p.print(", ");

            r+= GDOOR_UTILS::print_json_bool<uint8_t>(p, "valid", valid);

            return r;
       }
};

class GDOOR_DATA_PROTOCOL : public Printable { // Class/Struct to collect bus high level protocol data
    public:
        GDOOR_DATA *raw;
        const char *type;
        const char *action;
        uint8_t parameters[2];
        uint8_t source[3];
        uint8_t destination[3];

        GDOOR_DATA_PROTOCOL(GDOOR_DATA* data, bool idle = false);

        virtual size_t printTo(Print& p) const {
            size_t r = 0;
            static uint32_t cnt = 0;

            // Json compatible output
            r+= GDOOR_UTILS::print_json_string(p, "action", action);
            r+= p.print(", ");

            r+= GDOOR_UTILS::print_json_hexstring<uint8_t>(p, "parameters", parameters, 2);
            r+= p.print(", ");

            r+= GDOOR_UTILS::print_json_hexstring<uint8_t>(p, "source", source, 3);
            r+= p.print(", ");

            r+= GDOOR_UTILS::print_json_hexstring<uint8_t>(p, "destination", destination, 3);
            r+= p.print(", ");

            r+= GDOOR_UTILS::print_json_string(p, "type", type);
            r+= p.print(", ");

            if (this->raw != NULL) {
                r+= GDOOR_UTILS::print_json_hexstring<uint8_t>(p, "busdata", this->raw->data, this->raw->len);
                r+= p.print(", ");

                if(debug) {
                    r+= GDOOR_UTILS::print_json_hexarray<uint16_t>(p, "raw", this->raw->raw, this->raw->len*9);
                    r+= p.print(", ");
                }
            }

            r+= GDOOR_UTILS::print_json_value<uint32_t>(p, "event_id", cnt++);

            return r;
        }
};

#endif