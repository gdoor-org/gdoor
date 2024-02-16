/*
 * gdoor_protocol.h
 *
 *  Created on: 22.11.2022
 *      Author: xx
 */

#ifndef INC_GDOOR_PROTOCOL_H_
#define INC_GDOOR_PROTOCOL_H_
#include "gdoor_rx.h"
#include "stm32l1xx_hal.h"

typedef enum {INDOOR_AUDIO, OUTDOOR_AUDIO, CONTROLLER_AUDIO, TYPE_UNKOWN} gdoor_protocol_hwtype;
typedef enum {
	BUTTON_RING_UNKOWN, BUTTON_LIGHT, OPEN_DOOR,
	CALL_ACCEPT, CALL_CLOSE, BUTTON_FLOOR, BUTTON_RING,
	CTRL_DOOROPENER_ACK, CTRL_RESET, CTRL_DOORSTATION_ACK,
	CTRL_BUTTONS_TRAINING_START, CTRL_DOOROPENER_TRAINING_START,
	CTRL_DOOROPENER_TRAINING_STOP, CTRL_PROGRAMMING_START, CTRL_PROGRAMMING_STOP,
	ACTION_UNKOWN
} gdoor_protocol_action;

typedef struct {
	gdoor_rx *raw;
	gdoor_protocol_hwtype type;
	gdoor_protocol_action action;
	uint8_t parameters[2];
	uint8_t source[3];
	uint8_t destination[3];
} gdoor_protocol;

uint8_t gdoor_protocol_action2hex(gdoor_protocol_action action);
gdoor_protocol_action gdoor_protocol_hex2action(uint8_t value);

uint8_t gdoor_protocol_type2hex(gdoor_protocol_hwtype type);
gdoor_protocol_hwtype gdoor_protocol_hex2type(uint8_t value);

void gdoor_protocol_decode(gdoor_rx *data, gdoor_protocol *protocol);
#endif /* INC_GDOOR_PROTOCOL_H_ */
