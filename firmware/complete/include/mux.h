/* mux.h - Declarations for the Analog MUX interface
 * Copyright (c) 2022, Ralf Gerlich
 */
#ifndef MUX_H
#define MUX_H

enum mux_channel_t {
    mux_channel_voltage = 0,
    mux_channel_current = 1
};

void mux_init();

void mux_select_channel(mux_channel_t channel);

#endif /* MUX_H */
