/*
 * gdoor_io.c
 *
 *  Created on: 23.11.2022
 *      Author: xx
 */
#include "stm32l1xx_hal.h"
#include "main.h"
#include "gdoor_timers.h"

#define ON 1
#define OFF 0
#define NUM_CHANNELS 3

extern TIM_HandleTypeDef htim11;

uint8_t channels_trigger[NUM_CHANNELS] = {
		OFF, OFF, OFF
};

uint8_t channels_running[NUM_CHANNELS] = {
		OFF, OFF, OFF
};

GPIO_TypeDef* channels_gpioport[NUM_CHANNELS] = {
		NULL, NULL, NULL
};

uint16_t channels_pinno[NUM_CHANNELS] = {
		0, 0, 0
};

void arduino_timer11_interrupt() {
	uint8_t all_off = 1;
	for(uint8_t i=0; i<NUM_CHANNELS; i++) { //For all channels
		if(channels_gpioport[i] != NULL) { //If channel has assigned gpio
			if(channels_running[i] == ON) { // Check if channel is running and we should turn it off
				channels_running[i] = OFF;
				HAL_GPIO_WritePin(channels_gpioport[i], channels_pinno[i], GPIO_PIN_RESET);
			}
			if(channels_trigger[i] == ON) { //Channel is triggered so turn it on
				all_off = 0; //As we turned something on, we can not disable the timer
				channels_trigger[i] = OFF;
				channels_running[i] = ON;
				HAL_GPIO_WritePin(channels_gpioport[i], channels_pinno[i], GPIO_PIN_SET);
			}
		}
	}
	if(all_off) { //Only disable timer, if no channel is running anymore
		timer_stop(&htim11);
	}
}

void gdoor_io_pulse(uint8_t ch) {
	if(ch < NUM_CHANNELS && channels_gpioport[ch] != NULL) {
		channels_trigger[ch] = ON;
		timer_start(&htim11);
	}

}

void gdoor_io_setup_ch(uint8_t ch, GPIO_TypeDef* gpioport, uint16_t gpiopin) {
	if(ch < NUM_CHANNELS) {
		channels_gpioport[ch] = gpioport;
		channels_pinno[ch] = gpiopin;
	}
}
