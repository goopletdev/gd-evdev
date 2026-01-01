#include "gd-evdev-string.h"

char gd_evdev_expected_char(struct gd_evdev *gdev, struct input_event ev) {
    if (ev.type != EV_KEY || ev.value < 1) {
        return '\0';
    }
    const char *str = key_to_char_map[ev.code];
    if (str == NULL) {
        return '\0';
    }
    return gdev->dev.mod & GD_MOD_SHIFT ? str[1] : str[0];
}

// TODO: improve error handling
// TODO: handle presence of already-depressed modifiers, and capslock
int gd_evdev_write_char(struct gd_evdev *gdev, char c) {
    const struct gd_evdev_key_mod keys = gd_evdev_char_key_combo[c];
    if (keys.key == 0) {
        return -1;
    }
    int err;
    if (keys.mod & GD_MOD_SHIFT) {
        err = gd_evdev_send_key_event(gdev, KEY_LEFTSHIFT, 1);
        err = gd_evdev_send_key_event(gdev, keys.key, 1);
        err = gd_evdev_send_key_event(gdev, keys.key, 0);
        err = gd_evdev_send_key_event(gdev, KEY_LEFTSHIFT, 0);
    } else {
        err = gd_evdev_send_key_event(gdev, keys.key, 1);
        err = gd_evdev_send_key_event(gdev, keys.key, 0);
    }
    return err;
}

int gd_evdev_write_utf8(struct gd_evdev *gdev, utf8_codepoint codepoint) {
    char hex_buffer[8];
    snprintf(hex_buffer, 8, "%x", codepoint);
    int err;
    printf("hex: %s\n", hex_buffer);

    // enter unicode input:
    err = gd_evdev_send_key_event(gdev, KEY_LEFTCTRL, 1);
    err = gd_evdev_send_key_event(gdev, KEY_LEFTSHIFT, 1);
    err = gd_evdev_send_key_event(gdev, KEY_U, 1);
    err = gd_evdev_send_key_event(gdev, KEY_LEFTCTRL, 0);
    err = gd_evdev_send_key_event(gdev, KEY_LEFTSHIFT, 0);
    err = gd_evdev_send_key_event(gdev, KEY_U, 0);

    // wait for input screen to load:
    struct timespec sleeptime;
    sleeptime.tv_sec = 0;
    sleeptime.tv_nsec = 50000000L; // 50ms
    nanosleep(&sleeptime, NULL);

    // input unicode hex sequence
    for (size_t i = 0; i < sizeof(hex_buffer) && hex_buffer[i] && err >= 0; i++) {
        err = gd_evdev_write_char(gdev, hex_buffer[i]);
    }

    // submit
    err = gd_evdev_send_key_event(gdev, KEY_ENTER, 1);
    err = gd_evdev_send_key_event(gdev, KEY_ENTER, 0);

    // wait for input screen to close
    nanosleep(&sleeptime, NULL);

    return err;
}

// TODO: error handling; get rid of printf
int gd_evdev_write_string(struct gd_evdev *gdev, size_t n, const char *str) {
    size_t i = 0;
    int err;
    struct timespec sleeptime;
    sleeptime.tv_sec = 0;
    sleeptime.tv_nsec = 0L; // 20000000L; // 20ms
    while (i < n && str[i]) {
        if (0x80 & str[i]) {
            // is unicode
            utf8_codepoint c;
            int increment = utf8_codepoint_from_str(str, n, i, &c);
            if (increment <= 0) {
                printf("error in gd_evdev_write_string");
                return -1;
            }
            i += increment;
            err = gd_evdev_write_utf8(gdev, c);
        } else {
            // is ascii
            err = gd_evdev_write_char(gdev, str[i]);
            i++;
        }
        nanosleep(&sleeptime, NULL);
    }
    return err;
}

/**
 * For a target ASCII character, the key and mod key
 * necessary to send it as input.
 */
const struct gd_evdev_key_mod gd_evdev_char_key_combo[128] = {
    [0 ... 127] = { '\0', 0 },

    #define KEYCOMBO(c,k,m) [c] = { k, m }

    KEYCOMBO('\b', KEY_BACKSPACE, 0),
    KEYCOMBO('\t', KEY_TAB, 0),
    KEYCOMBO('\n', KEY_ENTER, 0),
    KEYCOMBO(' ', KEY_SPACE, 0),
    KEYCOMBO('!', KEY_1, GD_MOD_L_SHIFT),
    KEYCOMBO('"', KEY_APOSTROPHE, GD_MOD_L_SHIFT),
    KEYCOMBO('#', KEY_3, GD_MOD_L_SHIFT),
    KEYCOMBO('$', KEY_4, GD_MOD_L_SHIFT),
    KEYCOMBO('%', KEY_5, GD_MOD_L_SHIFT),
    KEYCOMBO('&', KEY_7, GD_MOD_L_SHIFT),
    KEYCOMBO('\'', KEY_APOSTROPHE, 0),
    KEYCOMBO('(', KEY_9, GD_MOD_L_SHIFT),
    KEYCOMBO(')', KEY_0, GD_MOD_L_SHIFT),
    KEYCOMBO('*', KEY_8, GD_MOD_L_SHIFT),
    KEYCOMBO('+', KEY_EQUAL, GD_MOD_L_SHIFT),
    KEYCOMBO(',', KEY_COMMA, 0),
    KEYCOMBO('-', KEY_MINUS, 0),
    KEYCOMBO('.', KEY_DOT, 0),
    KEYCOMBO('/', KEY_SLASH, 0),
    KEYCOMBO('0', KEY_0, 0),
    KEYCOMBO('1', KEY_1, 0),
    KEYCOMBO('2', KEY_2, 0),
    KEYCOMBO('3', KEY_3, 0),
    KEYCOMBO('4', KEY_4, 0),
    KEYCOMBO('5', KEY_5, 0),
    KEYCOMBO('6', KEY_6, 0),
    KEYCOMBO('7', KEY_7, 0),
    KEYCOMBO('8', KEY_8, 0),
    KEYCOMBO('9', KEY_9, 0),
    KEYCOMBO(':', KEY_SEMICOLON, GD_MOD_L_SHIFT),
    KEYCOMBO(';', KEY_SEMICOLON, 0),
    KEYCOMBO('<', KEY_COMMA, GD_MOD_L_SHIFT),
    KEYCOMBO('=', KEY_EQUAL, 0),
    KEYCOMBO('>', KEY_DOT, GD_MOD_L_SHIFT),
    KEYCOMBO('?', KEY_SLASH, GD_MOD_L_SHIFT),
    KEYCOMBO('@', KEY_2, GD_MOD_L_SHIFT),
    KEYCOMBO('A', KEY_A, GD_MOD_L_SHIFT),
    KEYCOMBO('B', KEY_B, GD_MOD_L_SHIFT),
    KEYCOMBO('C', KEY_C, GD_MOD_L_SHIFT),
    KEYCOMBO('D', KEY_D, GD_MOD_L_SHIFT),
    KEYCOMBO('E', KEY_E, GD_MOD_L_SHIFT),
    KEYCOMBO('F', KEY_F, GD_MOD_L_SHIFT),
    KEYCOMBO('G', KEY_G, GD_MOD_L_SHIFT),
    KEYCOMBO('H', KEY_H, GD_MOD_L_SHIFT),
    KEYCOMBO('I', KEY_I, GD_MOD_L_SHIFT),
    KEYCOMBO('J', KEY_J, GD_MOD_L_SHIFT),
    KEYCOMBO('K', KEY_K, GD_MOD_L_SHIFT),
    KEYCOMBO('L', KEY_L, GD_MOD_L_SHIFT),
    KEYCOMBO('M', KEY_M, GD_MOD_L_SHIFT),
    KEYCOMBO('N', KEY_N, GD_MOD_L_SHIFT),
    KEYCOMBO('O', KEY_O, GD_MOD_L_SHIFT),
    KEYCOMBO('P', KEY_P, GD_MOD_L_SHIFT),
    KEYCOMBO('Q', KEY_Q, GD_MOD_L_SHIFT),
    KEYCOMBO('R', KEY_R, GD_MOD_L_SHIFT),
    KEYCOMBO('S', KEY_S, GD_MOD_L_SHIFT),
    KEYCOMBO('T', KEY_T, GD_MOD_L_SHIFT),
    KEYCOMBO('U', KEY_U, GD_MOD_L_SHIFT),
    KEYCOMBO('V', KEY_V, GD_MOD_L_SHIFT),
    KEYCOMBO('W', KEY_W, GD_MOD_L_SHIFT),
    KEYCOMBO('X', KEY_X, GD_MOD_L_SHIFT),
    KEYCOMBO('Y', KEY_Y, GD_MOD_L_SHIFT),
    KEYCOMBO('Z', KEY_Z, GD_MOD_L_SHIFT),
    KEYCOMBO('[', KEY_LEFTBRACE, 0),
    KEYCOMBO('\\', KEY_BACKSLASH, 0),
    KEYCOMBO(']', KEY_RIGHTBRACE, 0),
    KEYCOMBO('^', KEY_6, GD_MOD_L_SHIFT),
    KEYCOMBO('_', KEY_MINUS, GD_MOD_L_SHIFT),
    KEYCOMBO('`', KEY_GRAVE, 0),
    KEYCOMBO('a', KEY_A, 0),
    KEYCOMBO('b', KEY_B, 0),
    KEYCOMBO('c', KEY_C, 0),
    KEYCOMBO('d', KEY_D, 0),
    KEYCOMBO('e', KEY_E, 0),
    KEYCOMBO('f', KEY_F, 0),
    KEYCOMBO('g', KEY_G, 0),
    KEYCOMBO('h', KEY_H, 0),
    KEYCOMBO('i', KEY_I, 0),
    KEYCOMBO('j', KEY_J, 0),
    KEYCOMBO('k', KEY_K, 0),
    KEYCOMBO('l', KEY_L, 0),
    KEYCOMBO('m', KEY_M, 0),
    KEYCOMBO('n', KEY_N, 0),
    KEYCOMBO('o', KEY_O, 0),
    KEYCOMBO('p', KEY_P, 0),
    KEYCOMBO('q', KEY_Q, 0),
    KEYCOMBO('r', KEY_R, 0),
    KEYCOMBO('s', KEY_S, 0),
    KEYCOMBO('t', KEY_T, 0),
    KEYCOMBO('u', KEY_U, 0),
    KEYCOMBO('v', KEY_V, 0),
    KEYCOMBO('w', KEY_W, 0),
    KEYCOMBO('x', KEY_X, 0),
    KEYCOMBO('y', KEY_Y, 0),
    KEYCOMBO('z', KEY_Z, 0),
    KEYCOMBO('{', KEY_LEFTBRACE, GD_MOD_L_SHIFT),
    KEYCOMBO('|', KEY_BACKSLASH, GD_MOD_L_SHIFT),
    KEYCOMBO('}', KEY_RIGHTBRACE, GD_MOD_L_SHIFT),
    KEYCOMBO('~', KEY_GRAVE, GD_MOD_L_SHIFT),
};

/**
 * the expected chars from key events
 */
const char *key_to_char_map[KEY_MAX + 1] = {
    [0 ... KEY_MAX] = NULL,

    #define KEY_CHARMAP(k, s) [k] = s

    KEY_CHARMAP(KEY_BACKSPACE, "\b\b"),
    KEY_CHARMAP(KEY_TAB, "\t\0"),
    KEY_CHARMAP(KEY_ENTER, "\n\n"),
    KEY_CHARMAP(KEY_SPACE, "  "),
    KEY_CHARMAP(KEY_1, "1!"),
    KEY_CHARMAP(KEY_2, "2@"),
    KEY_CHARMAP(KEY_3, "3#"),
    KEY_CHARMAP(KEY_4, "4$"),
    KEY_CHARMAP(KEY_5, "5%"),
    KEY_CHARMAP(KEY_7, "7&"),
    KEY_CHARMAP(KEY_8, "8*"),
    KEY_CHARMAP(KEY_9, "9("),
    KEY_CHARMAP(KEY_0, "0)"),
    KEY_CHARMAP(KEY_APOSTROPHE, "'\""),
    KEY_CHARMAP(KEY_EQUAL, "=+"),
    KEY_CHARMAP(KEY_MINUS, "-_"),
    KEY_CHARMAP(KEY_DOT, ".>"),
    KEY_CHARMAP(KEY_SLASH, "/?"),
    KEY_CHARMAP(KEY_SEMICOLON, ";:"),
    KEY_CHARMAP(KEY_COMMA, ",<"),
    KEY_CHARMAP(KEY_A, "aA"),
    KEY_CHARMAP(KEY_B, "bB"),
    KEY_CHARMAP(KEY_C, "cC"),
    KEY_CHARMAP(KEY_D, "dD"),
    KEY_CHARMAP(KEY_E, "eE"),
    KEY_CHARMAP(KEY_F, "fF"),
    KEY_CHARMAP(KEY_G, "gG"),
    KEY_CHARMAP(KEY_H, "hH"),
    KEY_CHARMAP(KEY_I, "iI"),
    KEY_CHARMAP(KEY_J, "jJ"),
    KEY_CHARMAP(KEY_K, "kK"),
    KEY_CHARMAP(KEY_L, "lL"),
    KEY_CHARMAP(KEY_M, "mM"),
    KEY_CHARMAP(KEY_N, "nN"),
    KEY_CHARMAP(KEY_O, "oO"),
    KEY_CHARMAP(KEY_P, "pP"),
    KEY_CHARMAP(KEY_Q, "qQ"),
    KEY_CHARMAP(KEY_R, "rR"),
    KEY_CHARMAP(KEY_S, "sS"),
    KEY_CHARMAP(KEY_T, "tT"),
    KEY_CHARMAP(KEY_U, "uU"),
    KEY_CHARMAP(KEY_V, "vV"),
    KEY_CHARMAP(KEY_W, "wW"),
    KEY_CHARMAP(KEY_X, "xX"),
    KEY_CHARMAP(KEY_Y, "yY"),
    KEY_CHARMAP(KEY_Z, "zZ"),
    KEY_CHARMAP(KEY_LEFTBRACE, "[{"),
    KEY_CHARMAP(KEY_BACKSLASH, "\\|"),
    KEY_CHARMAP(KEY_RIGHTBRACE, "]}"),
    KEY_CHARMAP(KEY_6, "6^"),
    KEY_CHARMAP(KEY_GRAVE, "`~"),
};

