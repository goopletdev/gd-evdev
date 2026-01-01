#include "gd-evdev.h"
#include "gd-evdev-string.h"

#include <errno.h>
#include <stdio.h>

#define TEST_DEV_PATH "/dev/input/event12"

int main (int argc, char **argv) {
    struct gd_evdev *gdev = NULL;
    struct input_event ev;
    int err;
    gdev = gd_evdev_new();

    /////////////////
    /// init gdev ///
    /////////////////
    err = gd_evdev_init_gdev_from_dev_path(gdev, TEST_DEV_PATH);
    if (err != 0) {
        printf("ERROR (%i): Failed to initialize dev or create ui dev\n", err);
        return err;
    }
    gd_evdev_log_device(gdev);

    /////////////////////////
    /// grab physical dev ///
    /////////////////////////
    gd_evdev_await_all_keys_released(gdev);
    err = gd_evdev_grab(gdev);
    if (err != 0) {
        printf("ERROR (%i): Failed to grab device\n", err);
        return err;
    }

    /////////////////
    /// main loop ///
    /////////////////
    do {
        err = gd_evdev_next_event(gdev, &ev);
        if (err == 0) {
            gd_evdev_log_event(ev);
            // passthrough grabbed device event
            err = gd_evdev_write_event(gdev, ev);
        } 
        if (ev.type == EV_SYN) {
            printf("Mod keys: %i %i\n", __builtin_popcount(gdev->ui.mod), __builtin_popcount(gdev->dev.mod));
        }
        char expected = gd_evdev_expected_char(gdev, ev);
        if (expected) {
            printf("Expected output: %c\n", expected);
        }
    } while (err == 1 || err == 0 || err == -EAGAIN);

    
    ///////////////
    /// cleanup ///
    ///////////////
    err = gd_evdev_cleanup(gdev);
    return err;
}

