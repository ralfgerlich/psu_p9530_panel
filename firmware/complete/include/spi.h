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

static inline uint8_t spi_send(uint8_t value) {
    SPDR = value;
    while ((SPSR & (1<<SPIF))==0);
    return SPDR;
}


#endif /* SPI_H */
