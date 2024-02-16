/*
 * gdoor_tx.h
 *
 *  Created on: 21.11.2022
 *      Author: xx
 */

#ifndef INC_GDOOR_TX_H_
#define INC_GDOOR_TX_H_
#define STATE_SENDING 0x01
void arduino_timer2_interrupt();
void gdoor_tx_send_words(uint8_t *words, uint16_t len);
void gdoor_tx_setup();
#endif /* INC_GDOOR_TX_H_ */
