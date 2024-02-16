/*
 * main_arduino.c
 *
 *  Created on: Nov 19, 2022
 *      Author: xx
 */
#include "stm32l1xx_hal.h"
#include "main.h"
#include "main_arduino.h"
#include "gdoor_timers.h"
#include "gdoor_rx.h"
#include "gdoor_tx.h"
#include "gdoor_io.h"
#include "gdoor_protocol.h"
#include "gdoor_terminal.h"

#include <stdio.h>

#define DEBOUNCE_TIME 5

extern uint16_t tx_state;

//--------------------------------------------------------------------------

// Action = Door opener
uint8_t gdoor_open_door[] = {
  0x02, 0x00, 0x31, 0x78, 0x56, 0x4E, 0x00, 0x00, 0xA1, 0xf0, 0xD2, 0x95
};

uint8_t gdoor_call_from_door[] = {
  0x01, 0x10, 0x11, 0xa6, 0x78, 0x98, 0x01, 0xA0, 0xA0
};

void arduino_gpio_interrupt(uint16_t GPIO_Pin) {

}

static inline void in4_function() {
	static uint8_t debounce = 0;
	if(HAL_GPIO_ReadPin(PIN_IN4_GPIO_Port, PIN_IN4_Pin)) {
		HAL_GPIO_WritePin(PIN_LED2_GPIO_Port, PIN_LED2_Pin, GPIO_PIN_SET);
		if (debounce <= DEBOUNCE_TIME) {
			debounce++;
		}
	} else {
		HAL_GPIO_WritePin(PIN_LED2_GPIO_Port, PIN_LED2_Pin, GPIO_PIN_RESET);
		if(debounce >= DEBOUNCE_TIME) {
			gdoor_tx_send_words(gdoor_open_door, 12);
		}
		debounce = 0;
	}
}

static inline void decode_rx_data(gdoor_rx *rx_data) {
	gdoor_protocol message;
	gdoor_protocol_decode(rx_data, &message);

	if(message.action == BUTTON_LIGHT) {
		if(HAL_GPIO_ReadPin(PIN_IN4_GPIO_Port, PIN_IN4_Pin)) {
			HAL_GPIO_TogglePin(PIN_LED1_GPIO_Port, PIN_LED1_Pin);
		}

		if(message.source[0] == 0x56 && message.source[1] ==  0x20 && message.source[2] == 0x99) { //OG1
			gdoor_call_from_door[6] = 0x01;
			gdoor_tx_send_words(gdoor_call_from_door, 9);
		}
	} else if(message.action == BUTTON_RING) {
		if(message.parameters[0] == 0x01) {
			gdoor_io_pulse(0);
		} else if (message.parameters[0] == 0x02) {
			gdoor_io_pulse(1);
		} else if (message.parameters[0] == 0x03) {
			gdoor_io_pulse(2);
		}
	}
}

void arduino_setup() {
	HAL_PWREx_EnableLowPowerRunMode();
	HAL_GPIO_WritePin(PIN_LED1_GPIO_Port, PIN_LED1_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(PIN_LED2_GPIO_Port, PIN_LED2_Pin, GPIO_PIN_SET);

	gdoor_rx_setup();
	gdoor_tx_setup();

	gdoor_io_setup_ch(0, PIN_OUT1_GPIO_Port, PIN_OUT1_Pin);
	gdoor_io_setup_ch(1, PIN_OUT2_GPIO_Port, PIN_OUT2_Pin);
	gdoor_io_setup_ch(2, PIN_OUT3_GPIO_Port, PIN_OUT3_Pin);

	printf("Setup GDOOR\n");
}

void arduino_loop() {
	static gdoor_rx *rx_data = NULL;

	// Run RX decoding stuff
	gdoor_rx_loop();

	//Check if new data was received
	rx_data = gdoor_rx_get_data();
	if(rx_data != NULL) {
		printf("New data:\n");
		for(uint16_t i=0; i<rx_data->len; i++) {
			printf("0x%x ", rx_data->data[i]);
		}
		printf("\nValid: %u\n", rx_data->valid);
		decode_rx_data(rx_data);
	}

	in4_function();

	// USB Serial terminal
	//gdoor_terminal_loop();
	//Sleep here
	//if(!tx_state&STATE_SENDING) {
	//	HAL_PWR_EnterSLEEPMode(PWR_LOWPOWERREGULATOR_ON, PWR_SLEEPENTRY_WFI);
	//}
}
