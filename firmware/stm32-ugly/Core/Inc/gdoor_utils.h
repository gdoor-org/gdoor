/*
 * gdoor_utils.h
 *
 *  Created on: 21.11.2022
 *      Author: xx
 */

#ifndef INC_GDOOR_UTILS_H_
#define INC_GDOOR_UTILS_H_
#include "stm32l1xx_hal.h"

uint8_t gdoor_crc(uint8_t *words, uint16_t len);
uint8_t gdoor_parity_odd(uint8_t word);


#endif /* INC_GDOOR_UTILS_H_ */
