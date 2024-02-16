/*
 * gdoor_timers.h
 *
 *  Created on: 21.11.2022
 *      Author: xx
 */

#ifndef INC_GDOOR_TIMERS_H_
#define INC_GDOOR_TIMERS_H_
#include "stm32l1xx_hal.h"

#define OC1M_COMPARE1 0x70
#define OC1M_COMPARE1_HIGH 0x40

static inline void timer_start(TIM_HandleTypeDef *htim) {
	htim->Instance->CR1 |= 0x01;
	htim->Instance->DIER |= 0x01;
}

static inline void timer_stop(TIM_HandleTypeDef *htim) {
	htim->Instance->CR1 &= ~0x01;
	htim->Instance->DIER &= ~0x01;
}

static inline void timer_set_outputcompare(TIM_HandleTypeDef *htim, uint8_t mode) {
	  if (mode == 0) {
		  //Disable, force low
		  htim->Instance->CCMR2 = 0x40;
	  } else if(mode == 1){
		  //Enable let it toggle
		  htim->Instance->CCMR2 = 0x70;
	  }

}

static inline void timer_reset_counters(TIM_HandleTypeDef *htim) {
	htim->Instance->EGR = TIM_EGR_UG; //Reset counter values
}


#endif /* INC_GDOOR_TIMERS_H_ */
