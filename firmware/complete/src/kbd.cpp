/* kbd.cpp - Functions for the keyboard interface
 * Copyright (c) 2022, Ralf Gerlich
 */
#include <Arduino.h>
#include "kbd.h"
#include "spi.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>

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

#define MASK_ENC_A _BV(kbd_enc_a)
#define MASK_ENC_B _BV(kbd_enc_b)
#define MASK_ENC (MASK_ENC_A|MASK_ENC_B)

/* Keyboard buffer */
static KeyCode kbd_buffer[KBD_BUFFER_LEN];

/* Read pointer */
static uint8_t kbd_buffer_read;

/* Fill state */
static uint8_t kbd_buffer_count;

/* Current keyboard state */
static uint32_t kbd_state;

static inline void kbd_toggle_clk() {
    PORT_SPI &= ~MASK_SCK;
    PORT_SPI |= MASK_SCK;
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

/** Remove a keycode from the keyboard buffer.
 * NOTE: This function does not ensure atomicity, e.g.
 *       in case of interrupts. Use kbd_remove() for this.
 */
static KeyCode kbd_remove_unsafe() {
    if (!kbd_buffer_count) {
        /* Buffer is empty */
        return kbd_none;
    } else {
        /* Remove one item from the buffer */
        const KeyCode code = kbd_buffer[kbd_buffer_read];
        kbd_buffer_count--;
        kbd_buffer_read++;
        kbd_buffer_read %= KBD_BUFFER_LEN;
        return code;
    }
}

/** Remove a key code from the keyboard buffer. */
KeyCode kbd_remove() {
    KeyCode code;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        code = kbd_remove_unsafe();
    }
    return code;
}

static inline void kbd_emplace_unsafe(KeyCode code) {
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
    PORT_CS |= MASK_CS;

    kbd_state = ~0UL;
    kbd_buffer_count = 0;
    kbd_buffer_read = 0;

    kbd_reset();
}

static uint32_t kbd_scan_matrix() {
    uint32_t new_state = 0;

    kbd_acquire();

    /* Send 8 high levels */
    PORT_SPI |= MASK_MOSI;
    for (int i = 0; i < 16; i++)
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
        new_state <<= 3UL;
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
        if ((kbd_state & MASK_ENC) == MASK_ENC_A) {
            /* Clockwise */
            kbd_emplace_unsafe(kbd_enc_cw);
        } else if ((kbd_state & MASK_ENC) == MASK_ENC_B) {
            /* Counter-clockwise */
            kbd_emplace_unsafe(kbd_enc_ccw);
        }
    }

    /* Process everything else */
    uint32_t clean_state = new_state & ~MASK_ENC;
    uint32_t mask = 1;
    for (KeyCode code = (KeyCode)0; code < kbd__count_physical; code = (KeyCode)(code+1), mask<<=1) {
        if (!(kbd_state & mask) && (clean_state & mask)) {
            kbd_emplace_unsafe(code);
        }
    }

    /* Update the keyboard state */
    kbd_state = new_state;
}