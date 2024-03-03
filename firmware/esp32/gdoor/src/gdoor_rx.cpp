/* 
 * This file is part of the GDOOR distribution (https://github.com/gdoor-org).
 * Copyright (c) 2024 GDOOR Authors.
 * 
 * This program is free software: you can redistribute it and/or modify  
 * it under the terms of the GNU General Public License as published by  
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "defines.h"
#include "gdoor_rx.h"
#include "gdoor_utils.h"

namespace GDOOR_RX {

    uint16_t counts[MAX_WORDLEN*9]; // Received counter values of bitstream, buffer
    uint16_t isr_cnt = 0; // Interrupt Counter (Counting RX edges)  

    uint8_t words[MAX_WORDLEN]; //Received words buffer

    uint16_t rx_state = 0; // State Machine

    uint8_t bitcounter = 0; //Current bit index, in currently active bitstream

    GDOOR_RX_DATA retval;

    hw_timer_t * timer_bit_received = NULL;
    hw_timer_t * timer_bitstream_received = NULL;

    int pin_rx = 0;
    
    /*
    * We received a 60kHz pulse, so start timeout timer (for bit and whole bitstream) and increment bit pulse count,
    * so that logic knows how much pulses were in this bit pulse-train.
    */
    void ARDUINO_ISR_ATTR isr_extint_rx() {
        rx_state |= (uint16_t)FLAG_RX_ACTIVE
        isr_cnt = isr_cnt + 1;
        timerWrite(timer_bit_received, 0); //reset timer
        timerWrite(timer_bitstream_received, 0); //reset timer
        timerStart(timer_bit_received); //Start timer to detect bit is over
        timerStart(timer_bitstream_received); //Start timer to detect bistream is over
    }

    /*
    * If this timer fires, the rx 60kHz pulse-train stopped,
    * so we should read out how many pulses we got for this bit (to decide 1 or 0)
    */
    void ARDUINO_ISR_ATTR isr_timer_bit_received() {
        if (bitcounter > MAX_WORDLEN*9) {
            bitcounter = 0;
        }
        counts[bitcounter] = isr_cnt;
        
        isr_cnt = 0;
        bitcounter = bitcounter + 1;
        timerStop(timer_bit_received);
    }

    /*
    * If this timer fires, rx bit stream is over
    */
    void ARDUINO_ISR_ATTR isr_timer_bitstream_received() {
        rx_state &= (uint16_t)~FLAG_RX_ACTIVE
        rx_state |= (uint16_t)FLAG_BITSTREAM_RECEIVED;
        timerStop(timer_bitstream_received);
        timerStop(timer_bit_received);
    }

    /*
    * Internal function set reset all internal values.
    */
    void reset() {
        bitcounter = 0;
        isr_cnt = 0;
    }

    /*
    * Function to enable/disable RX, so that during TX we can disable RX to not get our own message
    */
    void enable() {
        reset();
        attachInterrupt(pin_rx, isr_extint_rx, FALLING);
    }

     /*
    * Function to enable/disable RX, so that during TX we can disable RX to not get our own message
    */
    void disable() {
        reset();
        detachInterrupt(pin_rx);
    }
    

    /*
    * Function called by user to setup everything needed for GDOOR.
    * @param int rxpin Pin number where pulses from bus are received
    */
    void setup(int rxpin) {
        reset();
        pin_rx = rxpin;
        pinMode(pin_rx, INPUT);

        retval.len = 0;
        retval.valid = 0;
        retval.data = words;

        // Set bit_received timer frequency to 120kHz
        timer_bit_received = timerBegin(0, 667, true);

        // Attach isr_timer_bit_received function to bit_received timer.
        timerAttachInterrupt(timer_bit_received, &isr_timer_bit_received, true);

        // Set alarm to call isr_timer_bit_received function
        // after 20 120kHz Cycles (=10 60kHz Cycles)
        timerAlarmWrite(timer_bit_received, 20, true);

        // Set bit_received timer frequency to 120kHz
        timer_bitstream_received = timerBegin(1, 667, true);

        // Attach isr_timer_bit_received function to bit_received timer.
        timerAttachInterrupt(timer_bitstream_received, &isr_timer_bitstream_received, true);

        // Set alarm to call isr_timer_bit_received function
        // after 6*STARTBIT_MIN_LEN 120kHz Cycles (= 3 * STARTBIT_MIN_LEN 60kHz Cycles)
        timerAlarmWrite(timer_bitstream_received, 6*STARTBIT_MIN_LEN, true);

        // Enable External RX Interrupt
        enable();

        // Set Timers to default values, just to be sure
        timerWrite(timer_bit_received, 0); //reset timer
        timerWrite(timer_bitstream_received, 0); //reset timer
        timerStop(timer_bitstream_received);
        timerStop(timer_bit_received);

        timerAlarmEnable(timer_bit_received);
        timerAlarmEnable(timer_bitstream_received);
    }

    /*
    * Function called by user, in main loop.
    * Needed for the decoding logic.
    */
    void loop() {
        uint8_t wordcounter = 0; //Current word index
        uint8_t current_pulsetrain_valid = 1; //If parity or crc fails, this is set to 0
        uint16_t bit_one_thres = 0; //Dynamic Bit 1/0 threshold, based on length of startpulse

        if (rx_state & FLAG_BITSTREAM_RECEIVED) {
            uint8_t bitstream_len = bitcounter; // Number of received bits
            uint8_t is_startbit = 1; // Flag to indicate current bit is start bit to determine 1/0 threshold based on its width
            uint8_t bitindex = 0; //Current bit index inside current word, loops from 0 to 8 (9bits per word)

            for (uint8_t i; i<bitstream_len;i++) {
                uint16_t cnt = counts[i];
                uint8_t bit = 0;

                // Filter out smaller pulses, just ignore them
                if (cnt < BIT_MIN_LEN) {
                    continue;
                }

                // Check that first start bit is at least roughly in our expected range
                if(is_startbit && cnt < STARTBIT_MIN_LEN) {
                    continue;
                }

                // First bit is start bit and we use it to determine
                // length of one bit and zero bit
                if (is_startbit) {
                    bit_one_thres = cnt/BIT_ONE_DIV;
                    is_startbit = 0;
                } else { //Normal bit

                    // We start new receive word so preset the word with value 0
                    if (bitindex == 0) {
                        words[wordcounter] = 0;
                    }

                    //Detect zero or one bit value
                    if (cnt < bit_one_thres) {
                        bit = 1;
                    }

                    // Parity Bit
                    if (bitindex == 8) {
                        // Check if parity bit is as expected
                        if (GDOOR_UTILS::parity_odd(words[wordcounter]) != bit) {
                            current_pulsetrain_valid = 0;
                        }
                        bitindex = 0;
                        wordcounter = wordcounter + 1;
                    } else { // Normal Bits from 0 to 7
                        words[wordcounter] |= (uint8_t)(bit << bitindex);
                        bitindex = bitindex + 1;
                    }

                }
            }
            rx_state &= (uint16_t)~FLAG_BITSTREAM_RECEIVED;
            rx_state |= (uint16_t)FLAG_BITSTREAM_CONVERTED;      
        }
        
        if(rx_state & FLAG_BITSTREAM_CONVERTED) {
            if(wordcounter != 0) {

                //Check last word for crc value
                if (GDOOR_UTILS::crc(words, wordcounter-1) != words[wordcounter-1]) {
                    current_pulsetrain_valid = 0;
                }
                retval.len = wordcounter;
                retval.valid = current_pulsetrain_valid;

                //Signal that new data is available
                rx_state |= FLAG_DATA_READY;
            }
            rx_state &= (uint16_t)~FLAG_BITSTREAM_CONVERTED;

            //Prepare for next receive
            reset();
        }
    }

    /**
    * User function, called to see if new data is available.
    * @return Data pointer as GDOOR_RX_DATA class or NULL if no data is available
    */
    GDOOR_RX_DATA* read() {
        if(rx_state & FLAG_DATA_READY) {
            rx_state &= (uint16_t)~FLAG_DATA_READY;
            return &retval;
        }
        return NULL;
    }
}