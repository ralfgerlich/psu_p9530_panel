#include "spi.h"

void spi_init() {
    DDR_SPI |= MASK_MOSI|MASK_SCK;
    PORT_SPI &= ~MASK_SCK;
    PORT_SPI |= MASK_SS;
}