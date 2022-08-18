/* dac.h - Declarations for the DAC interface
 * Copyright (c) 2022, Ralf Gerlich
 */
#ifndef DAC_H
#define DAC_H

#include <stdint.h>

/** Initialize the DAC */
void dac_init();

/** Set the DAC output */
void dac_set(uint16_t value);

#endif /* DAC_H */
