/* adc.h - Declarations for the ADC interface
 * Copyright (c) 2022, Ralf Gerlich
 */
#ifndef ADC_H
#define ADC_H
#include <stdint.h>

enum {
    adc_channel_voltage = 0,
    adc_channel_current,
    adc_channel_temp_1,
    adc_channel_temp_2,

    adc_channel__count
};

extern uint16_t adc_results[adc_channel__count];

void adc_init();

#endif /* ADC_H */
