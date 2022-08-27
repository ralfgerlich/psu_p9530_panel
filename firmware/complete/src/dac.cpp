#include "dac.h"
#include "spi.h"
#include <avr/io.h>
#include <util/atomic.h>

#define PORT_DAC PORTC
#define DDR_DAC DDRC

#define PIN_DAC_CS 5
#define MASK_DAC_CS _BV(PIN_DAC_CS)

void dac_init() {
    DDR_DAC |= MASK_DAC_CS;
    PORT_DAC |= MASK_DAC_CS;
}

static inline void dac_acquire() {
    PORT_DAC &= ~MASK_DAC_CS;
}

static inline void dac_release() {
    PORT_DAC |= MASK_DAC_CS;
}

void dac_set_unsafe(uint16_t value) {
    /* Left-align the value */
    value <<= 2;

    spi_enable();

    dac_acquire();
    /* Send the high byte */
    spi_send(value>>8);
    /* Send the low byte */
    spi_send(value);

    dac_release();

    /* Disable the SPI controller again */
    spi_disable();
}

void dac_set(uint16_t value) {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        dac_set_unsafe(value);
    }
}
