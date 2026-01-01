#ifndef GD_EVDEV_STRING_H
#define GD_EVDEV_STRING_H

#include "gd-evdev.h"
#include "gd-unicode.h"

#include <time.h>
#include <linux/input.h>

struct gd_evdev_key_mod {
    int key;
    unsigned int mod;
};

/**
 * For a target ASCII character, the key and mod key
 * necessary to send it as input.
 */
extern const struct gd_evdev_key_mod gd_evdev_char_key_combo[128];

/**
 * the expected chars from key events
 */
extern const char *key_to_char_map[KEY_MAX + 1];

int gd_evdev_write_char(struct gd_evdev *gdev, char c);

int gd_evdev_write_utf8(struct gd_evdev *gdev, utf8_codepoint utf8);

int gd_evdev_write_string(struct gd_evdev *gdev, size_t n, const char *str);

// TODO: make this take capslock into account
char gd_evdev_expected_char(struct gd_evdev *gdev, struct input_event ev);

#endif // GD_EVDEV_STRING_H
