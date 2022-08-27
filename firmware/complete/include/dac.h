/* dac.h - Declarations for the DAC interface
 * Copyright (c) 2022, Ralf Gerlich
 */
#ifndef DAC_H
#define DAC_H

#include <stdint.h>

/** Initialize the DAC */
void dac_init();

/** Set the DAC output
 * NOTE: This function does not ensure atomicity!
 */
void dac_set_unsafe(uint16_t value);

/** Set the DAC output */
void dac_set(uint16_t value);

#endif /* DAC_H */
