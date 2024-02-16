/*
 * main_arduino.h
 *
 *  Created on: Nov 19, 2022
 *      Author: xx
 */

#ifndef INC_MAIN_ARDUINO_H_
#define INC_MAIN_ARDUINO_H_

#ifdef __cplusplus
 extern "C" {
#endif

void arduino_setup();
void arduino_loop();
void arduino_gpio_interrupt(uint16_t GPIO_Pin);
#ifdef __cplusplus
}
#endif
#endif /* INC_MAIN_ARDUINO_H_ */
