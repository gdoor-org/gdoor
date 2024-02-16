/*
 * gdoor_utils.c
 *
 *  Created on: 21.11.2022
 *      Author: xx
 */
#include "stm32l1xx_hal.h"

uint8_t gdoor_crc(uint8_t *words, uint16_t len) {
	uint8_t crc = 0;
	for(uint16_t i=0; i<len; i++) {//iterate over all words
		crc = crc + words[i];
	}
	return crc;
}

uint8_t gdoor_parity_odd(uint8_t word)
{
	uint8_t ones = 0;

	while(word != 0) {
		ones++;
		word &= (uint8_t)(word-1);
	}

	/* if ones is odd, least significant bit will be 1 */
	return ones &0x01;
}


