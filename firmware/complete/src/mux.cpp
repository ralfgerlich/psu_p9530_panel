/* mux.cpp - Functions for the analog MUX
 * Copyright (c) 2022, Ralf Gerlich
 */
#include "mux.h"
#include <avr/io.h>

#define PORT_MUX PORTC
#define DDR_MUX DDRC
#define PIN_MUX PIN4
#define MASK_MUX _BV(PIN_MUX)

void mux_init() {
    DDR_MUX |= MASK_MUX;
    mux_select_channel(mux_channel_voltage);
}

void mux_select_channel(mux_channel_t channel) {
    switch (channel) {
    case mux_channel_voltage:
        PORT_MUX &= ~MASK_MUX;
        break;
    case mux_channel_current:
        PORT_MUX |= MASK_MUX;
        break;
    default:
        /* Do nothing */
        break;
    }
}