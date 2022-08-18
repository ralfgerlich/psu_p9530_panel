/* kbd.cpp - Functions for the keyboard interface
 * Copyright (c) 2022, Ralf Gerlich
 */
#include <Arduino.h>
#include "kbd.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

/* Port for column lines */
#define PORT_COL PORTD
#define PIN_COL PIND
#define DDR_COL DDRD

/* Pin numbers for column lines */
#define PIN_COL0 PIN2
#define PIN_COL1 (PIN_COL0+1)
#define PIN_COL2 (PIN_COL0+2)

/* Bit mask for the column pins */
#define MASK_COL (_BV(PIN_COL0)|_BV(PIN_COL1)|_BV(PIN_COL2))

/* Port for chip select line */
#define PORT_CS PORTD
#define DDR_CS DDRD

/* Pin number for chip select line */
#define PIN_CS PIN6

/* Bit mask for chip select line */
#define MASK_CS _BV(PIN_CS)

/* Port for SPI */
#define PORT_SPI PORTB
#define DDR_SPI DDRB

/* Pins for SPI */
#define PIN_MOSI PIN3
#define PIN_MISO PIN4
#define PIN_SCK PIN5

/* Masks for SPI */
#define MASK_MOSI _BV(PIN_MOSI)
#define MASK_MISO _BV(PIN_MISO)
#define MASK_SCK _BV(PIN_SCK)

#define MASK_ENC_A _BV(kbd_enc_a)
#define MASK_ENC_B _BV(kbd_enc_b)
#define MASK_ENC (MASK_ENC_A|MASK_ENC_B)

/* Keyboard buffer */
static kbd_code_t kbd_buffer[KBD_BUFFER_LEN];

/* Read pointer */
static uint8_t kbd_buffer_read;

/* Fill state */
static uint8_t kbd_buffer_count;

/* Current keyboard state */
static uint32_t kbd_state;

static inline void kbd_toggle_clk() {
    PORT_SPI |= MASK_SCK;
    PORT_SPI &= ~MASK_SCK;
}

static inline void kbd_acquire() {
    PORT_CS |= MASK_CS;
}

static inline void kbd_release() {
    PORT_CS &= ~MASK_CS;
}

static inline void kbd_reset() {
    kbd_acquire();
    kbd_release();
}

kbd_code_t kbd_remove() {
    if (!kbd_buffer_count) {
        /* Buffer is empty */
        return kbd_none;
    } else {
        /* Remove one item from the buffer */
        const kbd_code_t code = kbd_buffer[kbd_buffer_read];
        kbd_buffer_count--;
        kbd_buffer_read++;
        kbd_buffer_read %= KBD_BUFFER_LEN;
        return code;
    }
}

static inline void kbd_emplace(kbd_code_t code) {
    if (kbd_buffer_count==KBD_BUFFER_LEN) {
        /* Buffer is full => drop one key */
        kbd_remove();
    }
    const uint8_t write_ptr = (kbd_buffer_read+KBD_BUFFER_LEN-kbd_buffer_count) % KBD_BUFFER_LEN;
    kbd_buffer[write_ptr] = code;
    kbd_buffer_count++;
}

void kbd_init() {
    DDR_COL &= ~MASK_COL;
    PORT_COL |= MASK_COL;
    DDR_CS |= MASK_CS;
    DDR_SPI |= MASK_MOSI|MASK_SCK;
    PORT_CS |= MASK_CS;
    PORT_SPI &= ~MASK_SCK;

    kbd_state = ~0;
    kbd_buffer_count = 0;
    kbd_buffer_read = 0;

    kbd_reset();
}

static uint32_t kbd_scan_matrix() {
    uint32_t new_state = 0;

    kbd_acquire();

    /* Send 8 high levels */
    PORT_SPI |= MASK_MOSI;
    for (int i = 0; i < 8; i++)
    {
        /* Shift by one */
        kbd_toggle_clk();
    }
    /* Next is a single low level */
    PORT_SPI &= ~MASK_MOSI;
    for (int i = 0; i < 8; i++)
    {
        /* Shift by one */
        kbd_toggle_clk();

        /* Read the COL pins */
        new_state <<= 3;
        new_state |= (PIN_COL&MASK_COL)>>PIN_COL0;

        /* Send high from here on */
        PORT_SPI |= MASK_MOSI;
    }

    kbd_release();

    return new_state;
}

void kbd_update() {
    uint32_t new_state = kbd_scan_matrix();

    /* De-bounce */
    if (new_state != kbd_scan_matrix()) {
        return;
    }

    /* Check the encoder */
    if ((new_state & MASK_ENC) == MASK_ENC) {
        /* We returned to the idle state of the encoder; check were we came from */
        if ((kbd_state & MASK_ENC) == 0) {
            /* Clockwise */
            kbd_emplace(kbd_enc_cw);
        } else if ((kbd_state & MASK_ENC) == MASK_ENC_B) {
            /* Counter-clockwise */
            kbd_emplace(kbd_enc_ccw);
        }
    }

    /* Process everything else */
    uint32_t clean_state = new_state & ~MASK_ENC;
    uint32_t mask = 1;
    for (kbd_code_t code = (kbd_code_t)0; code < kbd__count_physical; code = (kbd_code_t)(code+1), mask<<=1) {
        if (!(kbd_state & mask) && (clean_state & mask)) {
            kbd_emplace(code);
        }
    }

    /* Update the keyboard state */
    kbd_state = new_state;
}