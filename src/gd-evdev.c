#include "gd-evdev.h"

enum gd_evdev_mod_keys gd_evdev_mod_map[KEY_MAX + 1] = {
    [0 ... KEY_MAX] = GD_MOD_NONE,

    [KEY_LEFTSHIFT] = GD_MOD_L_SHIFT,
    [KEY_RIGHTSHIFT] = GD_MOD_R_SHIFT,
    [KEY_LEFTALT] = GD_MOD_L_ALT,
    [KEY_RIGHTALT] = GD_MOD_R_ALT,
    [KEY_LEFTCTRL] = GD_MOD_L_CTRL,
    [KEY_RIGHTCTRL] = GD_MOD_R_CTRL,
    [KEY_LEFTMETA] = GD_MOD_L_META,
    [KEY_RIGHTMETA] = GD_MOD_R_META,
};

int gd_bitmap_popcount(size_t size, const unsigned char *buffer) {
    int popcount = 0;
    for (size_t i = 0; i < size; i++) {
        popcount += __builtin_popcount(buffer[i]);
    }
    return popcount;
}

struct gd_evdev* gd_evdev_new(void) {
    struct gd_evdev *g = (struct gd_evdev*)malloc(sizeof(struct gd_evdev));
    g->dev.dev = NULL;
    g->dev.fd = -1;
    g->dev.mod = GD_MOD_NONE;
    g->ui.dev = NULL;
    g->ui.fd = -1;
    g->ui.mod = GD_MOD_NONE;
    return g;
}

int gd_evdev_init_dev(struct gd_evdev *gdev, char *dev_path) {
    struct gd_evdevT d;
    d.fd = open(dev_path, O_RDONLY);
    d.dev = libevdev_new();
    d.mod = GD_MOD_NONE;
    if (!d.dev) {
        return ENOMEM;
    }
    int err = libevdev_set_fd(d.dev, d.fd);
    if (err < 0) {
        printf("Failed (errno %d): %s\n", -err, strerror(-err));
        return err;
    }

    gdev->dev = d;
    return 0;
}

int gd_evdev_init_uinput(struct gd_evdev *gdev) {
    struct gd_evdev_uinputT ui;
    ui.fd = open(UINPUT_PATH, O_RDWR);
    if (ui.fd < 0) {
        return -errno;
    }

    int err = libevdev_uinput_create_from_device(gdev->dev.dev, ui.fd, &ui.dev);
    ui.mod = GD_MOD_NONE;
    gdev->ui = ui;
    return err;
}

int gd_evdev_init_gdev_from_dev_path(struct gd_evdev *gdev, char *dev_path) {
    int err = gd_evdev_init_dev(gdev, dev_path);
    if (err != 0) return err;
    err = gd_evdev_init_uinput(gdev);
    return err;
}

int gd_evdev_await_all_keys_released(struct gd_evdev *gdev) {
    char key_buffer[GD_KEY_BITMAP_SIZE] = { '\0' };
    int popcount = 0;
    int err = 0;
    struct input_event ev;

    do {
        ioctl(gdev->dev.fd, EVIOCGKEY(GD_KEY_BITMAP_SIZE), key_buffer);
        popcount = gd_bitmap_popcount(GD_KEY_BITMAP_SIZE, key_buffer);
        if (popcount > 0) {
            err = libevdev_next_event(gdev->dev.dev, LIBEVDEV_READ_FLAG_NORMAL, &ev);
            if (err < 0) {
                return err;
            }
        }
    } while (popcount > 0);

    return 0;
}

int gd_evdev_grab(struct gd_evdev *gdev) {
    return libevdev_grab(gdev->dev.dev, LIBEVDEV_GRAB);
}

int gd_evdev_ungrab(struct gd_evdev *gdev) {
    return libevdev_grab(gdev->dev.dev, LIBEVDEV_UNGRAB);
}

void gd_evdev_log_device(struct gd_evdev *gdev) {
    struct libevdev *dev = gdev->dev.dev;
    printf("Input device name: \"%s\"\n", libevdev_get_name(dev));
    printf("Input device ID: bus %#x vendor %#x product %#x\n",
            libevdev_get_id_bustype(dev),
            libevdev_get_id_vendor(dev),
            libevdev_get_id_product(dev));
}

void gd_evdev_log_event(struct input_event ev) {
    printf("Event: %s %s %d\n",
            libevdev_event_type_get_name(ev.type),
            libevdev_event_code_get_name(ev.type, ev.code),
            ev.value);
}

int gd_evdev_is_keyboard(struct gd_evdev *gdev) {
    return libevdev_has_event_code(gdev->dev.dev, EV_KEY, KEY_SPACE);
}

int gd_evdev_next_event(struct gd_evdev *gdev, struct input_event *ev) {
    int err = libevdev_next_event(gdev->dev.dev, LIBEVDEV_READ_FLAG_NORMAL, ev);
    if (ev->type == EV_KEY) {
        if (ev->value > 0) {
            gdev->dev.mod |= gd_evdev_mod_map[ev->code];
        } else {
            gdev->dev.mod &= ~(gd_evdev_mod_map[ev->code]);
        }
    }
    return err;
}

struct input_event gd_evdev_new_event(unsigned short type, unsigned short code, int val) {
    struct input_event ev;
    gettimeofday(&(ev.time), NULL);
    ev.type = type;
    ev.code = code;
    ev.value = val;
    return ev;
}

struct timeval gd_evdev_timeval_diff(struct timeval new, struct timeval old) {
    unsigned long long microseconds = (new.tv_sec * 1000000) + new.tv_usec - ((old.tv_sec * 1000000) + old.tv_usec);
    struct timeval diff;
    diff.tv_usec = microseconds % 1000000;
    diff.tv_sec = microseconds / 1000000;
    return diff;
}

int gd_evdev_write_event(struct gd_evdev *gdev, struct input_event ev) {
    int err = libevdev_uinput_write_event(gdev->ui.dev, ev.type, ev.code, ev.value);
    if (ev.type == EV_KEY) {
        if (ev.value > 0) {
            gdev->ui.mod |= gd_evdev_mod_map[ev.code];
        } else {
            gdev->ui.mod &= ~(gd_evdev_mod_map[ev.code]);
        }
    }
    return err;
}

int gd_evdev_send_key_event(struct gd_evdev *gdev, unsigned short key, int val) {
    struct input_event ev = gd_evdev_new_event(EV_KEY, key, val);
    int err = gd_evdev_write_event(gdev, ev);
    ev = gd_evdev_new_event(EV_SYN, SYN_REPORT, 0);
    err = gd_evdev_write_event(gdev, ev);
    return err;
}

int gd_evdev_cleanup(struct gd_evdev *gdev) {
    int err = gd_evdev_ungrab(gdev);
    if (err != 0) return err;
    libevdev_uinput_destroy(gdev->ui.dev);
    libevdev_free(gdev->dev.dev);
    err = close(gdev->ui.fd);
    if (err < 0) return err;
    err = close(gdev->dev.fd);
    if (err < 0) return err;
    free(gdev);
    return 0; 
}



