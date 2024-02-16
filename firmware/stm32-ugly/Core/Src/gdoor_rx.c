/*
 * gdoor_rx.c
 *
 *  Created on: 21.11.2022
 *      Author: xx
 */
#include "stm32l1xx_hal.h"
#include "main.h"
#include "main_arduino.h"
#include "gdoor_rx.h"
#include "gdoor_timers.h"
#include "gdoor_utils.h"

#define FLAG_BIT_RECEIVED 0x01
#define FLAG_BITSTREAM_RECEIVED 0x02
#define FLAG_DATA_READY 0x04

#define MAX_WORDLEN 25
#define BIT_ONE_DIV 2.5
#define BIT_MIN_LEN 5
#define STARTBIT_MIN_LEN 45

#define COMPARATOR_THRESHOLD 0xBF0

extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;
extern DAC_HandleTypeDef hdac;

uint16_t debug_bf[MAX_WORDLEN*9];
uint16_t rx_state = 0;
uint16_t _cnt = 0;
uint16_t cnt = 0;
uint8_t words[MAX_WORDLEN];
uint16_t bit_one_thres = 0;
uint8_t bitcounter = 0;
uint8_t is_startbit = 1;
uint16_t wordcounter = 0;
uint8_t current_pulsetrain_valid = 1;

gdoor_rx retval;

/*
 * We received a 60kHz pulse, so start timeout timer (for bit and whole bitstream) and increment bit pulse count,
 * so that logic knows how much pulses were in this bit pulse-train.
 */
void arduino_comparator_interrupt() {
	_cnt = _cnt + 1;
	timer_reset_counters(&htim3);
	timer_reset_counters(&htim4);
	timer_start(&htim3); //Start timer 3 to detect bit is over
	timer_start(&htim4); //Start timer 4 to detect bistream is over
}

/*
 * If this timer fires, the rx 60kHz pulse-train stopped,
 * so we should read out how many pulses we got for this bit (to decide 1 or 0)
 */
void arduino_timer3_interrupt() {
	cnt = _cnt;
	_cnt = 0;
	rx_state |= FLAG_BIT_RECEIVED;
	timer_stop(&htim3);
}

/*
 * If this timer fires, rx bit stream is over
 */
void arduino_timer4_interrupt() {
	rx_state |= FLAG_BITSTREAM_RECEIVED;
	timer_stop(&htim4);
	timer_stop(&htim3);
}

void gdoor_rx_reset() {
	bit_one_thres = 0;
	bitcounter = 0;
	wordcounter = 0;
	_cnt = 0;
	cnt = 0;
	is_startbit = 1;

	current_pulsetrain_valid = 1;
}

void gdoor_rx_setup() {
	gdoor_rx_reset();

	htim3.Instance->CR1 |= TIM_CR1_URS;
	htim4.Instance->CR1 |= TIM_CR1_URS;

	COMP->CSR |= 0x180000; //Set DAC1 as - input
	EXTI->RTSR |= 0x400000; //Enable Comp rising edge int.
	EXTI->IMR |= 0x400000; //Enable Comp rising edge int.

	// Setup DAC which is comparator - input
	HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, COMPARATOR_THRESHOLD);
	HAL_DAC_Start(&hdac, DAC_CHANNEL_1);

	rx_state = 0;

	retval.len = 0;
	retval.valid = 0;
	retval.data = words;

	timer_stop(&htim3);
	timer_stop(&htim4);

	timer_reset_counters(&htim3);
	timer_reset_counters(&htim4);
}

void gdoor_rx_loop() {
	uint8_t bit = 0;
	if(rx_state & FLAG_BITSTREAM_RECEIVED) {
			rx_state &= (uint16_t)~FLAG_BITSTREAM_RECEIVED;

			if(wordcounter != 0) {

				//Check last word for crc value
				if (gdoor_crc(words, wordcounter-1) != words[wordcounter-1]) {
					current_pulsetrain_valid = 0;
				}
				retval.len = wordcounter;
				retval.valid = current_pulsetrain_valid;

				//Signal that new data is available
				rx_state |= FLAG_DATA_READY;
			}

			//Prepare for next receive
			gdoor_rx_reset();

		}
	if (rx_state & FLAG_BIT_RECEIVED) {
		rx_state &= (uint16_t)~FLAG_BIT_RECEIVED;

		debug_bf[wordcounter*9+bitcounter] = cnt;

		// Avoid overflow, even though it will destroy this rx bitstream,
		// graceful error :)
		if (wordcounter > MAX_WORDLEN-1) {
			wordcounter = 0;
		}

		// Filter out smaller pulses, just ignore them
		if (cnt < BIT_MIN_LEN) {
			return;
		}

		// Check that first start bit is at least roughly in our expected range
		if(is_startbit && cnt < STARTBIT_MIN_LEN) {
			return;
		}

		// First bit is start bit and we use it to determine
		// length of one bit and zero bit
		if (is_startbit) {
			bit_one_thres = cnt/BIT_ONE_DIV;
			is_startbit = 0;
		} else { //Normal bit

			// We start new receive word so preset the word with value 0
			if (bitcounter == 0) {
				words[wordcounter] = 0;
			}

			//Detect zero or one bit value
			if (cnt < bit_one_thres) {
				bit = 1;
			}

			// Parity Bit
			if (bitcounter == 8) {
				// Check if parity bit is as expected
				if (gdoor_parity_odd(words[wordcounter]) != bit) {
					current_pulsetrain_valid = 0;
				}
				bitcounter = 0;
				wordcounter = wordcounter + 1;
			} else { // Normal Bits from 0 to 7
				words[wordcounter] |= (uint8_t)(bit << bitcounter);
				bitcounter = bitcounter + 1;
			}

		}
	}

}

gdoor_rx* gdoor_rx_get_data() {
	if(rx_state & FLAG_DATA_READY) {
		rx_state &= (uint16_t)~FLAG_DATA_READY;
		return &retval;
	}
	return NULL;
}


