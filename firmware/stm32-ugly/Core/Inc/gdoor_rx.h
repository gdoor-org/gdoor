/*
 * gdoor_rx.h
 *
 *  Created on: 21.11.2022
 *      Author: xx
 */

#ifndef INC_GDOOR_RX_H_
#define INC_GDOOR_RX_H_
#include "stm32l1xx_hal.h"

typedef struct {
	uint16_t len;
	uint8_t *data;
	uint8_t valid;
} gdoor_rx;

void arduino_comparator_interrupt();
void arduino_timer3_interrupt();
void arduino_timer4_interrupt();

void gdoor_rx_setup();
void gdoor_rx_loop();
gdoor_rx* gdoor_rx_get_data();




#endif /* INC_GDOOR_RX_H_ */
