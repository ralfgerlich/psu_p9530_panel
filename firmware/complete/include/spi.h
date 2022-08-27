/* spi.h - Declarations for the SPI interface
 * Copyright (c) 2022, Ralf Gerlich
 */
#ifndef SPI_H
#define SPI_H

#include <avr/io.h>

/* Port for SPI */
#define PORT_SPI PORTB
#define DDR_SPI DDRB

/* Pins for SPI */
#define PIN_SS PIN2
#define PIN_MOSI PIN3
#define PIN_MISO PIN4
#define PIN_SCK PIN5

/* Masks for SPI */
#define MASK_SS _BV(PIN_SS)
#define MASK_MOSI _BV(PIN_MOSI)
#define MASK_MISO _BV(PIN_MISO)
#define MASK_SCK _BV(PIN_SCK)

/** Initialize SPI port */
static inline void spi_init() {
    DDR_SPI |= MASK_MOSI|MASK_SCK;
    PORT_SPI &= ~MASK_SCK;
    PORT_SPI |= MASK_SS;
}

static inline void spi_enable() {
    /* Enable SPI in Mode 3 at fCPU/4, MSB first */
    SPCR = (1<<SPE)|(0<<DORD)|(1<<MSTR)|(1<<CPOL)|(1<<CPHA)|(0<<SPR1)|(0<<SPR0);
    SPSR = (0<<SPI2X);
}

static inline void spi_disable() {
    SPCR = 0;
}

static inline void spi_wait_until_tx_complete() {
    loop_until_bit_is_set(SPSR, SPIF);
}

static inline uint8_t spi_send(uint8_t value) {
    SPDR = value;
    spi_wait_until_tx_complete();
    return SPDR;
}

/* Save the SPI state and restore it at the end of the block.
 * Example:
 * ISOLATE_SPI { // Stores the current SPI state
 *     // Do something with the SPI
 * } // Here the SPI state is restored
 */
#define ISOLATE_SPI for ( uint16_t __state __attribute__((__cleanup__(__spi_restore_state))) = __spi_save_state(), __Todo = 1; __Todo ; __Todo = 0)

/* Helper function for saving the SPI state */
static inline uint16_t __spi_save_state() {
    return SPCR | (SPSR<<8);
}

/* Helper function for saving the SPI state */
static inline void __spi_restore_state(uint16_t* state) {
    SPCR = *state;
    SPSR = *state>>8;
}


#endif /* SPI_H */
