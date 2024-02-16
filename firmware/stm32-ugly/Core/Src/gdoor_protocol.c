/*
 * gdoor_protocol.c
 *
 *  Created on: 22.11.2022
 *      Author: xx
 */
#include "gdoor_protocol.h"
void gdoor_protocol_decode(gdoor_rx *data, gdoor_protocol *protocol) {
	protocol->type = TYPE_UNKOWN;
	protocol->action = ACTION_UNKOWN;

	protocol->raw = data;

	protocol->source[0] = 0x00;
	protocol->source[1] = 0x00;
	protocol->source[2] = 0x00;

	protocol->destination[0] = 0x00;
	protocol->destination[1] = 0x00;
	protocol->destination[2] = 0x00;

	protocol->parameters[0] = 0x00;
	protocol->parameters[1] = 0x00;

	if(data->valid && data->len >= 9) {
		protocol->type = gdoor_protocol_hex2type(data->data[8]);
		protocol->action = gdoor_protocol_hex2action(data->data[2]);

		protocol->parameters[0] = data->data[6];
		protocol->parameters[1] = data->data[7];

		protocol->source[0] = data->data[3];
		protocol->source[1] = data->data[4];
		protocol->source[2] = data->data[5];

		if(data->len >= 12) {
			protocol->destination[0] = data->data[9];
			protocol->destination[1] = data->data[10];
			protocol->destination[2] = data->data[11];
		}
	}
}

uint8_t gdoor_protocol_type2hex(gdoor_protocol_hwtype type) {
	switch (type) {
		case INDOOR_AUDIO:
			return 0xA1;
		case OUTDOOR_AUDIO:
			return 0xA0;
		case CONTROLLER_AUDIO:
			return 0xA3;
		case TYPE_UNKOWN:
			return 0xFF;
		default:
			return 0xFF;
	}
	return 0xFF;
}

gdoor_protocol_hwtype gdoor_protocol_hex2type(uint8_t value) {
	switch (value) {
		case 0xA1:
			return INDOOR_AUDIO;
		case 0xA0:
			return OUTDOOR_AUDIO;
		case 0xA3:
			return CONTROLLER_AUDIO;
		default:
			return TYPE_UNKOWN;
	}
	return TYPE_UNKOWN;
}

uint8_t gdoor_protocol_action2hex(gdoor_protocol_action action) {
	switch (action) {
		case BUTTON_RING_UNKOWN:
			return 0x42;
		case BUTTON_LIGHT:
			return 0x41;
		case OPEN_DOOR:
			return 0x31;
		case CALL_ACCEPT:
			return 0x21;
		case CALL_CLOSE:
			return 0x20;
		case BUTTON_FLOOR:
			return 0x13;
		case BUTTON_RING:
			return 0x11;
		case CTRL_DOOROPENER_ACK:
			return 0x0F;
		case CTRL_RESET:
			return 0x08;
		case CTRL_DOORSTATION_ACK:
			return 0x05;
		case CTRL_BUTTONS_TRAINING_START:
			return 0x04;
		case CTRL_DOOROPENER_TRAINING_START:
			return 0x03;
		case CTRL_DOOROPENER_TRAINING_STOP:
			return 0x02;
		case CTRL_PROGRAMMING_START:
			return 0x01;
		case CTRL_PROGRAMMING_STOP:
			return 0x00;
		case ACTION_UNKOWN:
			return 0xFF;
		default:
			return 0xFF;
	}
	return 0xFF;
}

gdoor_protocol_action gdoor_protocol_hex2action(uint8_t value) {
	switch (value) {
		case 0x42:
			return BUTTON_RING_UNKOWN;
		case 0x41:
			return BUTTON_LIGHT;
		case 0x31:
			return OPEN_DOOR;
		case 0x21:
			return CALL_ACCEPT;
		case 0x20:
			return CALL_CLOSE;
		case 0x13:
			return BUTTON_FLOOR;
		case 0x11:
			return BUTTON_RING;
		case 0x0F:
			return CTRL_DOOROPENER_ACK;
		case 0x08:
			return CTRL_RESET;
		case 0x05:
			return CTRL_DOORSTATION_ACK;
		case 0x04:
			return CTRL_BUTTONS_TRAINING_START;
		case 0x03:
			return CTRL_DOOROPENER_TRAINING_START;
		case 0x02:
			return CTRL_DOOROPENER_TRAINING_STOP;
		case 0x01:
			return CTRL_PROGRAMMING_START;
		case 0x00:
			return CTRL_PROGRAMMING_STOP;
		default:
			return ACTION_UNKOWN;
	}
	return ACTION_UNKOWN;
}
