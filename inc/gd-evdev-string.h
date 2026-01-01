#ifndef GD_EVDEV_STRING_H
#define GD_EVDEV_STRING_H

#include <linux/input.h>

const unsigned int MOD_LEFT_SHIFT = 1U << 0;
const unsigned int MOD_RIGHT_SHIFT = 1U << 1;
const unsigned int MOD_LEFT_ALT = 1U << 2;
const unsigned int MOD_RIGHT_ALT = 1U << 3;
const unsigned int MOD_LEFT_CTRL = 1U << 4;
const unsigned int MOD_RIGHT_CTRL = 1U << 5;
const unsigned int MOD_LEFT_META = 1U << 6;
const unsigned int MOD_RIGHT_META = 1U << 7;

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

int gd_evdev_write_utf8(struct gd_evdev *gdev, const char *utf8);

int gd_evdev_write_string(struct gd_evdev *gdev, const char *str);

char gd_evdev_expected_char(struct gd_evdev *gdev, int key);

#endif // GD_EVDEV_STRING_H
