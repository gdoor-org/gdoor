/*
 * gdoor_io.h
 *
 *  Created on: 23.11.2022
 *      Author: xx
 */

#ifndef INC_GDOOR_IO_H_
#define INC_GDOOR_IO_H_

void arduino_timer11_interrupt();
void gdoor_io_setup_ch(uint8_t ch, GPIO_TypeDef* gpioport, uint16_t gpiopin);
void gdoor_io_pulse(uint8_t ch);

#endif /* INC_GDOOR_IO_H_ */
