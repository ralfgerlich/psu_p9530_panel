/* kbd.cpp - Functions for the keyboard interface
 * Copyright (c) 2022, Ralf Gerlich
 */
#include <Arduino.h>
#include "kbd.h"
#include "hal_spi.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>

/* Port for column lines */
#define PORT_COL PORTD
#define PIN_COL PIND
#define DDR_COL DDRD

/* Pin numbers for column lines */
#define PIN_COL0 PIN4

/* Port for chip select line */
#define PORT_CS PORTB
#define DDR_CS DDRB

/* Pin number for chip select line */
#define PIN_CS PIN0

/* Bit mask for chip select line */
#define MASK_CS _BV(PIN_CS)

/* Port for encoder lines */
#define PORT_ENC PORTD
#define PIN_ENC PIND
#define DDR_ENC DDRD

/* Pin numbers for encoder lines */
#define PIN_ENC_A PIN2
#define PIN_ENC_B PIN3

/* Bit masks for encoder lines */
#define MASK_ENC_A _BV(PIN_ENC_A)
#define MASK_ENC_B _BV(PIN_ENC_B)
#define MASK_ENC (MASK_ENC_A|MASK_ENC_B)

/* Keyboard buffer */
static KeyCode kbd_buffer[KBD_BUFFER_LEN];

/* Read pointer */
static uint8_t kbd_buffer_read;

/* Fill state */
static uint8_t kbd_buffer_count;

/* Current keyboard state */
static uint32_t kbd_state;

// encoder state
static int8_t encoderState = 0;

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
        kbd_remove_unsafe();
    }
    const uint8_t write_ptr = (kbd_buffer_read+kbd_buffer_count) % KBD_BUFFER_LEN;
    kbd_buffer[write_ptr] = code;
    kbd_buffer_count++;
}

void kbd_emplace(KeyCode code) {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        kbd_emplace_unsafe(code);
    }
}

void kbd_init() {
    /* Set up pins for keyboard as input with pull-up */
    DDR_COL &= ~(7<<PIN_COL0);
    PORT_COL |= (7<<PIN_COL0);
    DDR_CS |= MASK_CS;
    PORT_CS |= MASK_CS;

    /* Set up encoder pins as input with pull-up */
    DDR_ENC &= ~MASK_ENC;
    PORT_ENC |= MASK_ENC;
    /* Configure pin interrupts for encoder.
     * The interrupts will be generated on a rising edge of
     * the respective pin. */
    EICRA = (1<<ISC10)|(1<<ISC00);
    EIMSK = _BV(INT1)|_BV(INT0);
    EIFR = _BV(INTF1)|_BV(INTF0);

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
    for (uint8_t i = 0; i < 16; i++)
    {
        /* Shift by one */
        kbd_toggle_clk();
    }
    /* Next is a single low level */
    PORT_SPI &= ~MASK_MOSI;
    for (uint8_t i = 0; i < 8; i++)
    {
        /* Shift by one */
        kbd_toggle_clk();

        /* Read the COL pins */
        new_state <<= 3UL;
        new_state |= (PIN_COL>>PIN_COL0)&7;

        /* Send high from here on */
        PORT_SPI |= MASK_MOSI;
    }

    kbd_release();

    return new_state;
}

void kbd_update() {
    /* Disable the SPI as we will be using the pins for bit-banging. */
    spi_disable();
    uint32_t new_state = kbd_scan_matrix();

    /* De-bounce */
    if (new_state != kbd_scan_matrix()) {
        return;
    }

    /* Process everything else */
    uint32_t mask = 1;
    for (KeyCode code = (KeyCode)0; code < kbd__count_physical; code = (KeyCode)(code+1), mask<<=1) {
        if (!(kbd_state & mask) && (new_state & mask)) {
            /* The key has been released */
            kbd_emplace_unsafe(code);
        }
    }

    /* Update the keyboard state */
    kbd_state = new_state;
}

// aka ENC_A
ISR(INT0_vect) {
    // we see wrong flanks for some reason
    // sometimes 00 is missing
    // sometimes you get fake 01 for the oposite direction (in phy world)
    uint8_t currentEncoderState = PIN_ENC & MASK_ENC;
    if (currentEncoderState & MASK_ENC_A) {
        // A goes high
        if (currentEncoderState & MASK_ENC_B) {
            // B is high
            encoderState--;
            if (encoderState <= -2) {
                // Pin A has gone high last => CCW
                // two out of four state counts for this direction are enough
                kbd_emplace_unsafe(kbd_enc_ccw);
            }
            encoderState = 0;
        } else {
            // B is low
            encoderState++;
        }
    } else {
        // A goes low
        if (currentEncoderState & MASK_ENC_B) {
            // B is high
            encoderState++;
        } else if (encoderState < 0) {
            // B is low
            // only counts for predominent rotation direction
            encoderState--;
        }
    }
}

// aka ENC_B
ISR(INT1_vect) {
    // we see wrong flanks for some reason
    // sometimes 00 is missing
    // sometimes you get fake 01 for the oposite direction (in phy world)
    uint8_t currentEncoderState = PIN_ENC & MASK_ENC;
    if (currentEncoderState & MASK_ENC_B) {
        // B goes high
        if (currentEncoderState & MASK_ENC_A) {
            // A is high
            encoderState++;
            if (encoderState >= 2) {
                // Pin B has gone high last => CW
                // two out of four state counts for this direction are enough
                kbd_emplace_unsafe(kbd_enc_cw);
            }
            encoderState = 0;
        } else {
            // A is low
            encoderState--;
        }
    } else {
        // B goes low
        if (currentEncoderState & MASK_ENC_A) {
            // A is high
            encoderState--;
        } else if (encoderState > 0) {
            // A is low
            // only counts for predominent rotation direction
            encoderState++;
        }
    }
}