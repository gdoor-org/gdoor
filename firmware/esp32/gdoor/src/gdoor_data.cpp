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
#include <map>
#include "defines.h"
#include "gdoor_data.h"
#include "gdoor_utils.h"

// Map the HW Type field between bus value and human readable string
std::map<int, const char*>GDOOR_DATA_HWTYPE = {
    { 0xA1, "INDOOR"},
    { 0xA0, "OUTDOOR"},
    { 0xA3, "CONTROLLER"}
};

// Map the Action field between bus value and human readable string
std::map<int, const char*>GDOOR_DATA_ACTION = {
    { 0x42, "BUTTON"},
    { 0x41, "BUTTON_LIGHT"},
    { 0x31, "DOOR_OPEN"},
    { 0x28, "VIDEO_REQUEST"},
    { 0x21, "AUDIO_REQUEST"},
    { 0x20, "AUDIO_VIDEO_END"},
    { 0x13, "BUTTON_FLOOR"},
    { 0x12, "CALL_INTERNAL"},
    { 0x11, "BUTTON_RING"},
    { 0x0F, "CTRL_DOOROPENER_ACK"},
    { 0x08, "CTRL_RESET"},
    { 0x05, "CTRL_DOORSTATION_ACK"},
    { 0x04, "CTRL_BUTTONS_TRAINING_START"},
    { 0x03, "CTRL_DOOROPENER_TRAINING_START"},
    { 0x02, "CTRL_DOOROPENER_TRAINING_STOP"},
    { 0x01, "CTRL_PROGRAMMING_START"},
    { 0x00, "CTRL_PROGRAMMING_STOP"}
};

/**
 * Parse function, reading in the raw timer count values,
 * populating the GDOOR_DATA class elements.
 * 
 * @param counts Array with pulse counts of bits
 * @param len Number of elements in array
 * @return true if parsing was successful
*/
bool GDOOR_DATA::parse(uint16_t *counts, uint16_t len) {
    uint8_t wordcounter = 0; //Current word index
    uint8_t current_pulsetrain_valid = 1; //If parity or crc fails, this is set to 0
    uint16_t bit_one_thres = 0; //Dynamic Bit 1/0 threshold, based on length of startpulse

    uint8_t is_startbit = 1; // Flag to indicate current bit is start bit to determine 1/0 threshold based on its width
    uint8_t bitindex = 0; //Current bit index inside current word, loops from 0 to 8 (9bits per word)

    bool success=false;

    for (uint8_t i=0; i<len; i++) {
        uint16_t cnt = counts[i];
        uint8_t bit = 0;
        this->raw[i] = cnt;

        // Filter out smaller pulses, just ignore them
        if (cnt < BIT_MIN_LEN) {
            continue;
        }

        // Check that first start bit is at least roughly in our expected range
        if(is_startbit && cnt < STARTBIT_MIN_LEN) {
            continue;
        }

        // First bit is start bit and we use it to determine
        // length of one bit and zero bit
        if (is_startbit) {
            bit_one_thres = cnt/BIT_ONE_DIV;
            is_startbit = 0;
        } else { //Normal bit

            // We start new receive word so preset the word with value 0
            if (bitindex == 0) {
                this->data[wordcounter] = 0;
            }

            //Detect zero or one bit value
            if (cnt < bit_one_thres) {
                bit = 1;
            }

            // Parity Bit
            if (bitindex == 8) {
                // Check if parity bit is as expected
                if (GDOOR_UTILS::parity_odd(this->data[wordcounter]) != bit) {
                    current_pulsetrain_valid = 0;
                }
                bitindex = 0;
                wordcounter = wordcounter + 1;
            } else { // Normal Bits from 0 to 7
                this->data[wordcounter] |= (uint8_t)(bit << bitindex);
                bitindex = bitindex + 1;
            }

        } //End normal bit
    } //End for  
    
    if(wordcounter != 0) {
        //Check last word for crc value
        if (GDOOR_UTILS::crc(this->data, wordcounter-1) != this->data[wordcounter-1]) {
            current_pulsetrain_valid = 0;
        }
        this->len = wordcounter;
        this->valid = current_pulsetrain_valid;
        success = true;
    }
    return success;
}

/*
* Constructor for GDOOR_DATA_PROTOCOL,
* parses the bus data based on GDOOR_DATA,
* and stores a human readable form in its class elements.
*
* @param data GDOOR_DATA element, with parsed bus data.
* @param idle true: generate idle message
*/
GDOOR_DATA_PROTOCOL::GDOOR_DATA_PROTOCOL(GDOOR_DATA* data, bool idle) {
    if(idle) {
        this->type = "TYPE_GDOOR";
        this->action = "BUS_IDLE";
    } else {
        this->type = "TYPE_UNKOWN";
        this->action = "ACTION_UNKOWN";
    }
    this->raw = data;

    this->source[0] = 0x00;
    this->source[1] = 0x00;
    this->source[2] = 0x00;

    this->destination[0] = 0x00;
    this->destination[1] = 0x00;
    this->destination[2] = 0x00;

    this->parameters[0] = 0x00;
    this->parameters[1] = 0x00;

    if(data != NULL && data->valid && data->len >= 9) {
        if(GDOOR_DATA_HWTYPE.find(data->data[8]) != GDOOR_DATA_HWTYPE.end()){
            this->type = GDOOR_DATA_HWTYPE.at(data->data[8]);
        }
        if(GDOOR_DATA_ACTION.find(data->data[2]) != GDOOR_DATA_ACTION.end()){
            this->action = GDOOR_DATA_ACTION.at(data->data[2]);
        }

        this->parameters[0] = data->data[6];
        this->parameters[1] = data->data[7];

        this->source[0] = data->data[3];
        this->source[1] = data->data[4];
        this->source[2] = data->data[5];

        if(data->len >= 12) {
            this->destination[0] = data->data[9];
            this->destination[1] = data->data[10];
            this->destination[2] = data->data[11];
        }
    }
}