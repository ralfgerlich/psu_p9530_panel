/* kbd.h - Declarations for the keyboard interface
 * Copyright (c) 2022, Ralf Gerlich
 */
#ifndef KBD_H
#define KBD_H


/** Key codes for individual keys
 * These are both the key codes and the indices
 * of the respective bit associated with the key line
 */
enum KeyCode {
    kbd_p,
    kbd_lock,
    kbd_unused_0,
    kbd_i,
    kbd_memory,
    kbd_unused_1,
    kbd_u,
    kbd_remote,
    kbd_left,
    kbd_standby,
    kbd_enter,
    kbd_right,
    kbd_ce,
    kbd_0,
    kbd_dot,
    kbd_9,
    kbd_8,
    kbd_7,
    kbd_6,
    kbd_5,
    kbd_4,
    kbd_3,
    kbd_2,
    kbd_1,
    
    kbd__count_physical, /* Number of physical key codes */

    /* These are special key codes for representing rotations on the encoder */
    kbd_enc_cw,
    kbd_enc_ccw,

    /* Special code for empty buffer */
    kbd_none
};

/* Length of the keyboard buffer */
#define KBD_BUFFER_LEN 16

/** Initialize the keyboard interface */
void kbd_init();

/** Scan for keypresses and update the buffer */
void kbd_update();

/** Remove an entry from the keyboard buffer */
KeyCode kbd_remove();


#endif /* KBD_H */