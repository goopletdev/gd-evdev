#ifndef GD_EVDEV_H
#define GD_EVDEV_H

#include <errno.h> // errno
#include <fcntl.h> // open
#include <libevdev/libevdev-uinput.h>
#include <limits.h> // charbit
#include <stdio.h> // printf
#include <stdlib.h> // free, NULL
#include <string.h> // strerror
#include <unistd.h> // close

#define UINPUT_PATH "/dev/uinput"
#define GD_KEY_BITMAP_SIZE ((KEY_MAX / CHAR_BIT) + 1)

/*
 * struct input_event {
 *   struct timeval time;
 *   unsigned short type;
 *   unsigned short code;
 *   int value;
 * };
 */

enum gd_evdev_mod_keys {
    GD_MOD_NONE = 0,
    GD_MOD_L_SHIFT = 1U << 0,
    GD_MOD_R_SHIFT = 1U << 1,
    GD_MOD_L_ALT = 1U << 2,
    GD_MOD_R_ALT = 1U << 3,
    GD_MOD_L_CTRL = 1U << 4,
    GD_MOD_R_CTRL = 1U << 5,
    GD_MOD_L_META = 1U << 6,
    GD_MOD_R_META = 1U << 7,

    GD_MOD_SHIFT = GD_MOD_L_SHIFT | GD_MOD_R_SHIFT,
    GD_MOD_ALT = GD_MOD_L_ALT | GD_MOD_R_ALT,
    GD_MOD_CTRL = GD_MOD_L_CTRL | GD_MOD_R_CTRL,
    GD_MOD_META = GD_MOD_L_META | GD_MOD_R_META,
};

extern enum gd_evdev_mod_keys gd_evdev_mod_map[KEY_MAX + 1];

struct gd_evdevT {
    struct libevdev *dev;
    int fd;
    enum gd_evdev_mod_keys mod;
};

struct gd_evdev_uinputT {
    struct libevdev_uinput *dev;
    int fd;
    enum gd_evdev_mod_keys mod;
};

struct gd_evdev {
    struct gd_evdevT dev;
    struct gd_evdev_uinputT ui;
};

int gd_bitmap_popcount(size_t size, const unsigned char *buffer);

struct gd_evdev* gd_evdev_new(void);

int gd_evdev_init_dev(struct gd_evdev *gdev, char *dev_path);
int gd_evdev_init_uinput(struct gd_evdev *gdev);

/**
 * shorthand for:
 *
 * err = gd_evdev_init_dev(gdev, dev_path);
 * if (err != 0) return err;
 * err = gd_evdev_init_uinput(gdev);
 * return err;
 */
int gd_evdev_init_gdev_from_dev_path(struct gd_evdev *gdev, char *dev_path);

int gd_evdev_await_all_keys_released(struct gd_evdev *gdev);

int gd_evdev_grab(struct gd_evdev *gdev);
int gd_evdev_ungrab(struct gd_evdev *gdev);

void gd_evdev_log_device(struct gd_evdev *gdev);
void gd_evdev_log_event(struct input_event ev);

int gd_evdev_is_keyboard(struct gd_evdev *gdev);

/**
 * gets next libevdev event, sets mod for physical device
 */
int gd_evdev_next_event(struct gd_evdev *gdev, struct input_event *ev);

struct input_event gd_evdev_new_event(
        unsigned short type,
        unsigned short code,
        int val);

struct timeval gd_evdev_timeval_diff(struct timeval new, struct timeval old);

int gd_evdev_write_event(struct gd_evdev *gdev, struct input_event ev);

int gd_evdev_cleanup(struct gd_evdev *gdev);

#endif // GD_EVDEV_H
