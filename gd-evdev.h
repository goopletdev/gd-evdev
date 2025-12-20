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

struct gd_evdevT {
    struct libevdev *dev;
    int fd;
};

struct gd_evdev_uinputT {
    struct libevdev_uinput *dev;
    int fd;
};

struct gd_evdev {
    struct gd_evdevT dev;
    struct gd_evdev_uinputT ui;
};

int gd_bitmap_popcount(size_t size, const unsigned char *buffer);

struct gd_evdev* gd_evdev_new(void);

int gd_evdev_init_dev(struct gd_evdev *gdev, char *dev_path);

int gd_evdev_init_uinput(struct gd_evdev *gdev);

int gd_evdev_await_all_keys_released(struct gd_evdev *gdev);

int gd_evdev_grab(struct gd_evdev *gdev);
int gd_evdev_ungrab(struct gd_evdev *gdev);

void gd_evdev_log_device(struct gd_evdev *gdev);
void gd_evdev_log_event(struct input_event ev);

int gd_evdev_is_keyboard(struct gd_evdev *gdev);

int gd_evdev_next_event(struct gd_evdev *gdev, struct input_event *ev);

struct input_event gd_evdev_new_event(
        unsigned short type,
        unsigned short code,
        int val);

struct timeval gd_evdev_timeval_diff(struct timeval new, struct timeval old);

int gd_evdev_write_event(struct gd_evdev *gdev, struct input_event ev);

int gd_evdev_cleanup(struct gd_evdev *gdev);

#endif // GD_EVDEV_H
