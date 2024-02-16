/*
 * gdoor_terminal.c
 *
 *  Created on: 11.12.2022
 *      Author: xx
 */
#include "gdoor_terminal.h"
#include <stdio.h>

#define TERMINAL_RINGBUFFER_SIZE 100

uint8_t terminal_commandbuffer[TERMINAL_RINGBUFFER_SIZE+1];
uint8_t terminal_ringbuffer[TERMINAL_RINGBUFFER_SIZE];
uint8_t *terminal_ringbuffer_head = (uint8_t *)&terminal_ringbuffer;
uint8_t *terminal_ringbuffer_tail = (uint8_t *)&terminal_ringbuffer;
uint8_t terminal_size = 0;

extern uint32_t usb_ptr;
extern uint8_t *usb_buffer;

#define terminal_ringbuffer_end (terminal_ringbuffer+TERMINAL_RINGBUFFER_SIZE-1)

void ringerbuffer_add(uint8_t charval) {
	//We do not check for overflow head/tail
	if(terminal_ringbuffer_tail+1 <= terminal_ringbuffer_end) {
		terminal_ringbuffer_tail = terminal_ringbuffer_tail + 1;
		*terminal_ringbuffer_tail = charval;

	} else {
		terminal_ringbuffer_tail = terminal_ringbuffer;
	}
}

uint8_t ringbuffer_readline() {
	uint8_t *ptr = terminal_ringbuffer_head;
	uint8_t i = 0;

	while(ptr != terminal_ringbuffer_tail) {
		if(*ptr == '\n') {
			terminal_ringbuffer_head = ptr;
			break;
		} else {
			terminal_commandbuffer[i] = *ptr;
			i = i+1;
		}

		//Next char
		if(ptr+1 <= terminal_ringbuffer_end) {
			ptr = terminal_ringbuffer;
		} else {
			ptr = ptr + 1;
		}
	}
	terminal_commandbuffer[i] = '\0'; // End String
	return i;
}

uint8_t* findstring(uint8_t* string, uint8_t *search) {
	uint8_t *string_it = string;
	uint8_t *search_it = search;
	while(*string_it != '\0' && *string_it!= ' ' && *search_it != '\0') {
		if(*string_it != *search_it) {
			break;
		}
		string_it = string_it + 1;
		search_it = search_it + 1;

	}

	if( (*string_it == '\0' || *string_it == ' ') &&
		(*search_it == '\0')) {
		if(*string_it == ' ') {
			return string_it+1;
		}
		return string_it;
	}
	return NULL;
}

void printHelp() {
	printf("> HELP\n");
	printf("> IMPORTANT:\n");
	printf(" - All command arguments need to be upper char!\n");
	printf(" - Hex value need to be always full bytes without 0x. E.g. DE or 0E\n\n");
	printf("> Possible commands:\n");

	printf("set_relais N ACTION SRC PARAMETER1 PARAMETER2\n");
	printf("- N: Number of relais, 1 to 3\n");
	printf("- ACTION: Bus Action in hex, without 0x! Set to FF to ignore action field\n");
	printf("- SRC: Bus source sender, 3 Bytes in hex without 0x. E.g. 04AD05\n");
	printf("- PARAMETER1: Bus parameter 1 field in hex, without 0x! Set to FF to ignore action field\n");
	printf("- PARAMETER2: Bus parameter 2 field in hex, without 0x! Set to FF to ignore action field\n\n");
}

uint8_t parseRelais(gdoor_terminal_businfo *answer) {
	uint8_t *argument = findstring(terminal_commandbuffer, (uint8_t *)"set_relais");

	if(argument == NULL || *argument == '\0') {
		return 0;
	}

	if(argument[1] != ' ' || argument[4] != ' ' || argument[11] != ' ' || argument[14] != ' ') {
		return 0;
	}

	answer->relais_num = argument[0]-0x30;
	answer->action[0] = argument[2];
	answer->action[1] = argument[3];

	//TODO Parse hex string as bytes
	answer->source[0] = argument[5];
	answer->source[1] = argument[6];
	answer->source[2] = argument[7];

	//TODO Parse hex string as bytes
	answer->parameter1[0] = argument[12];
	answer->parameter1[1] = argument[13];

	//TODO Parse hex string as bytes
	answer->parameter2[0] = argument[15];
	answer->parameter2[1] = argument[16];
	return 1;
}

void gdoor_terminal_loop() {
	gdoor_terminal_businfo info;
	while(usb_ptr > 0) {
		ringerbuffer_add(usb_buffer[usb_ptr]);
		usb_ptr = usb_ptr - 1;
	}
	if(terminal_ringbuffer_tail != terminal_ringbuffer_head) { //Data available
		uint8_t commandlen = ringbuffer_readline();
		if(commandlen) {
			if(parseRelais(&info)) {
				printf("OK\n");
			}
			printHelp();
		}
	}
}


