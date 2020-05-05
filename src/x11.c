/*
 * KeyPresenter - A graphical visualisation tool for computer input
 * Copyright (C) 2020  hypothermic <admin@hypothermic.nl>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
/**
 * @file main.c
 * @brief Xinput (libXi) implementation for keyboard-related methods
 */

#include <stdlib.h>
#include <gtk/gtk.h>
#include <X11/XKBlib.h>
#include <X11/extensions/XInput2.h>

#include "keypresenter/key.h"
#include "polltaskresult.h"
#include "x11.h"

gpointer
kp_keyboard_init(GtkWindow *window) {
    KpX11KeyboardData *data = g_new0(KpX11KeyboardData, 1);
    data->displays = g_array_new(TRUE, TRUE, sizeof(Display*));

    DIR* d = opendir("/tmp/.X11-unix");

    if (d != NULL) {
        struct dirent *dr;
        while ((dr = readdir(d)) != NULL) {
            if (dr->d_name[0] != 'X')
                continue;

            char display_name[64] = ":";
            strcat(display_name, dr->d_name + 1);

            Display *display = XOpenDisplay(display_name);

            if (display == NULL) {
                fprintf(stderr, "Display %s not available.\n", display_name);
                continue;
            }

            int xiOpcode, queryEvent, queryError;
            if (!XQueryExtension(display, "XInputExtension", &xiOpcode, &queryEvent, &queryError)) {
                fprintf(stderr, "X Input extension not available for display %s.\n", display_name);
                continue;
            }
            data->xi_extension_opcode = xiOpcode;

            fprintf(stdout, "Found valid display %s", display_name);

            Window root = DefaultRootWindow(display);
            XIEventMask m;
            m.deviceid = XIAllMasterDevices;
            m.mask_len = XIMaskLen(XI_LASTEVENT);
            m.mask = calloc(m.mask_len, sizeof(char));
            XISetMask(m.mask, XI_RawKeyPress);
            XISetMask(m.mask, XI_RawKeyRelease);
            XISelectEvents(display, root, &m, 1);
            XSync(display, FALSE);
            free(m.mask);

            fprintf(stdout, "Using configured display %s", display_name);

            g_array_append_val(data->displays, display);
        }

        closedir(d);
    }

    return data;
}

GArray *
kp_keyboard_get_keys(GtkWindow *window, gpointer internal_keyboard_data) {
    GArray *result = g_array_new(TRUE, TRUE, sizeof(KpKey*));

    KpKey *key_a = g_new0(KpKey, 1);
    key_a->label = 'a';
    key_a->code = 1;

    g_array_append_val(result, key_a);

    return result;
}

static gboolean
on_poll_task_result(gpointer poll_task_result) {
    KpPollTaskResult *result = poll_task_result;

    fprintf(stderr, "Pressed key %c (%d) with state %s. TODO feed this into the GUI\n", result->poll.key.label, result->poll.key.code, result->poll.pressed ? "PRESSED" : "RELEASED");

    return G_SOURCE_REMOVE;
}

void
kp_keyboard_poll_task(GTask *task, gpointer source_obj, gpointer poll_task_result, GCancellable *cancellable) {
    KpPollTaskResult *result = poll_task_result;
    KpX11KeyboardData *keyboard_data = X11_KEYBOARD_DATA(result->keyboard_data);

    while ("forever") {
        if (g_cancellable_is_cancelled (cancellable)) {
            g_task_return_new_error(task,
                                    G_IO_ERROR, G_IO_ERROR_CANCELLED,
                                    "Task cancelled");
            return;
        }

        for (int i = 0; i < keyboard_data->displays->len; ++i) {
            Display *display = g_array_index(keyboard_data->displays, Display*, i);

            XEvent event;
            XGenericEventCookie *cookie = (XGenericEventCookie *) &event.xcookie;
            XNextEvent(display, &event);

            if (XGetEventData(display, cookie) &&
                cookie->type == GenericEvent &&
                cookie->extension == keyboard_data->xi_extension_opcode) {
                switch (cookie->evtype) {
                    case XI_RawKeyRelease:
                    case XI_RawKeyPress: {
                        XIRawEvent *ev = cookie->data;

                        // Ask X what it calls that key
                        KeySym s = XkbKeycodeToKeysym(display, ev->detail, 0, 0);
                        if (NoSymbol == s) continue;
                        char *str = XKeysymToString(s);
                        if (NULL == str) continue;

                        result->poll.key.code = s;
                        result->poll.key.label = *str;
                        result->poll.pressed = cookie->evtype == XI_RawKeyPress ? TRUE : FALSE;

                        g_main_context_invoke(NULL, on_poll_task_result, result);
                    }
                }
            }
        }
    }
}