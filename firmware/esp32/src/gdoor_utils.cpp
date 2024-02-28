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
#include "gdoor_utils.h"

namespace GDOOR_UTILS {
    uint8_t crc(uint8_t *words, uint16_t len) {
        uint8_t crc = 0;
        for(uint16_t i=0; i<len; i++) {//iterate over all words
            crc = crc + words[i];
        }
        return crc;
    }

    uint8_t parity_odd(uint8_t word)
    {
        uint8_t ones = 0;

        while(word != 0) {
            ones++;
            word &= (uint8_t)(word-1);
        }

        /* if ones is odd, least significant bit will be 1 */
        return ones &0x01;
    }
}