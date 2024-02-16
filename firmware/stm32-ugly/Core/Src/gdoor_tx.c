/*
 * gdoor_tx.c
 *
 *  Created on: 21.11.2022
 *      Author: xx
 */

#include "stm32l1xx_hal.h"
#include "main.h"
#include "main_arduino.h"
#include "gdoor_tx.h"
#include "gdoor_timers.h"
#include "gdoor_utils.h"

#define MAX_WORDLEN 25

#define STARTBIT_PULSENUM 66
#define ONE_PULSENUM 16
#define ZERO_PULSENUM 37
#define PAUSE_PULSENUM 30

extern TIM_HandleTypeDef htim2;

uint16_t tx_state = 0;

uint16_t tx_words[MAX_WORDLEN];
uint16_t bits_len = 0;
uint16_t bits_ptr = 0;
uint16_t pulse_cnt = 0;
uint8_t startbit_send = 0;

uint8_t timer_oc_state = 0;

static inline uint16_t bit2pulselen(uint16_t bit) {
	if (bit) {
		return ONE_PULSENUM;
	} else {
		return ZERO_PULSENUM;
	}
}

static inline uint16_t extractBitLen(uint16_t word, uint8_t bitindex) {
	return bit2pulselen((uint16_t) (word & (uint16_t)(0x01<<bitindex))); //LSB first
}

static inline uint16_t byte2word(uint8_t byte) {
	uint16_t value = byte & 0x00FF;
	if(gdoor_parity_odd(byte)) { //If parity, set MSB
		value |= 0x100;
	}
	return value;
}

static inline void start_timer() {
	//timer_set_outputcompare(&htim2, 0);
	tx_state |= STATE_SENDING;
	bits_ptr = 0;
	pulse_cnt = 0;
	timer_oc_state = 0;
	startbit_send = 0;

	//Workaround: Disable comparator to not be disturbed by receive.
	//Better sending scheme is needed
	EXTI->IMR &= ~0x400000; //Disable Comp rising edge int.
	HAL_GPIO_WritePin(PIN_GIRA_TX_EN_GPIO_Port, PIN_GIRA_TX_EN_Pin, GPIO_PIN_SET);
	timer_start(&htim2); //Start timer 2 to send out bitstream
}

static inline void stop_timer() {
	//timer_set_outputcompare(&htim2, 0);
	bits_ptr = 0;
	pulse_cnt = 0;
	startbit_send = 0;
	HAL_GPIO_WritePin(PIN_GIRA_TX_EN_GPIO_Port, PIN_GIRA_TX_EN_Pin, GPIO_PIN_RESET);
	timer_stop(&htim2);
	tx_state &= (uint16_t)~STATE_SENDING;
	//Workaround: Enable comparator after sending
	//Better sending scheme is needed
	EXTI->IMR |= 0x400000; //Enable Comp rising edge int.
	//timer_reset_counters(&htim2);
}


/*
 * This is the sending timer interrupt
 */
void arduino_timer2_interrupt() {
	if(pulse_cnt == 0) { // Update timer, we send out (or waited) enough timer ticks to go to next bit
		if (bits_ptr >= bits_len || bits_ptr >= MAX_WORDLEN*9) {//We send everything
			stop_timer();
			return;
		}

		if(timer_oc_state == 1) {
			// Do not send next 60khz pulses, but send pause (nothing)
			timer_oc_state = 0;
			pulse_cnt = PAUSE_PULSENUM;
			timer_set_outputcompare(&htim2, 0); //disable timer pulse output to send pause
		} else {
			// Load new tick values
			if (!startbit_send) { //First bit, is start bit with fixed value
				pulse_cnt = STARTBIT_PULSENUM;
				startbit_send = 1;
			} else { //Startbit was send, so load bit values now
				uint8_t wordindex = (uint8_t) bits_ptr/9;
				uint8_t bitindex = (uint8_t) bits_ptr%9;
				uint16_t word = tx_words[wordindex];

				pulse_cnt = extractBitLen(word, bitindex);
				bits_ptr = bits_ptr + 1;
			}

			timer_oc_state = 1; //Signal that we are sending, so next time a pause will happen
			timer_set_outputcompare(&htim2, 1); //Enable timer pulse output to send pulses forming the bit
		}
	} else { // Just update timer ticks, we are not finished yet
		pulse_cnt = pulse_cnt - 1;
	}
}

void gdoor_tx_setup() {
	/*Configure GPIO pin : PIN_GIRA_TX_EN_Pin */
	htim2.Instance->CCER |= 0x100;
	htim2.Instance->CCMR2  = 0x70;
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = PIN_GIRA_TX_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.Alternate = 1;
	HAL_GPIO_Init(PIN_GIRA_TX_GPIO_Port, &GPIO_InitStruct);
	stop_timer();
	bits_len = 0;
	tx_state = 0;
}

void gdoor_tx_send_words(uint8_t *data, uint16_t len) {
	if (! (tx_state & STATE_SENDING) && len < MAX_WORDLEN) {
		bits_ptr = 0;
		pulse_cnt = 0;
		bits_len = (uint16_t) (len*9 + 9); // Data bits + CRC (8bit CRC data + parity bit) (Startbit is added by timer int. routine)


		for (uint16_t i=0; i<len; i++) {
			uint8_t byte = data[i];
			tx_words[i] = byte2word(byte);
		}

		uint8_t crc = gdoor_crc(data, len);
		tx_words[len] = byte2word(crc);
		start_timer();
	}
}
