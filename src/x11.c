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
#include "macro.h"
#include "polltaskresult.h"
#include "x11.h"

#ifndef DEFAULT_KEY_ANIMATION_TIMEOUT
#define DEFAULT_KEY_ANIMATION_TIMEOUT 400
#endif

static const gchar DEFAULT_LATIN_ALLOWED_CHARACTERS[] = {'1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
                                                         'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',
                                                         'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't',
                                                         'u', 'v', 'w', 'x', 'y', 'z'};

gpointer
kp_keyboard_init(GtkWindow *UNUSED(window)) {
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
kp_keyboard_get_keys(GtkWindow *UNUSED(window), gpointer internal_keyboard_data) {
    GArray *result = g_array_new(TRUE, TRUE, sizeof(KpKey*));
    GHashTable *hash_table = g_hash_table_new(g_int64_hash, g_int64_equal);
    KpX11KeyboardData *data = X11_KEYBOARD_DATA(internal_keyboard_data);

    for (int i = 0; i < data->displays->len; ++i) {
        Display *display = g_array_index(data->displays, Display*, i);

        int min_keycodes_return, max_keycodes_return;
        XDisplayKeycodes(display, &min_keycodes_return, &max_keycodes_return);

        int keycode_count = max_keycodes_return - min_keycodes_return - 1,
            keysyms_per_keycode_return;
        KeySym *keysyms = XGetKeyboardMapping(display, min_keycodes_return, keycode_count, &keysyms_per_keycode_return);

        for (int j = 0; j < (keycode_count * keysyms_per_keycode_return); ++j) {
            KeySym keysym = keysyms[j];
            KeyCode keycode = XKeysymToKeycode(display, keysym);
            gchar *key_str = XKeysymToString(keysym);

            if (NULL == key_str) continue;
            if (!XkbIsLegalKeycode(keycode)) continue;

            if (g_hash_table_contains(hash_table, &keycode)) continue;
            g_hash_table_add(hash_table, &keycode);

            for (int k = 0; k < strlen(DEFAULT_LATIN_ALLOWED_CHARACTERS); k++) {
                gchar key_label = DEFAULT_LATIN_ALLOWED_CHARACTERS[k];

                if (key_label == key_str[0] && key_str[1] == '\0') {
                    KpKey *key = g_new0(KpKey, 1);
                    key->label = key_str;
                    key->code = keycode;

                    g_array_append_val(result, key);
                    break;
                }
            }
        }
    }

    g_hash_table_destroy(hash_table);

    return result;
}

static gboolean
on_toggle_button_idle_reset(gpointer button) {
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), FALSE);

    return G_SOURCE_REMOVE;
}

static gboolean
on_poll_task_result(gpointer poll_task_result) {
    KpPollTaskResult *result = poll_task_result;

    fprintf(stderr, "%s key %s (%d).\n", result->poll.pressed ? "Pressed" : "Released", result->poll.key.label, result->poll.key.code);

    if (result->poll.pressed) {
        GtkWidget *button = g_hash_table_lookup(result->key_button_table, &result->poll.key.code);

        if (GTK_IS_TOGGLE_BUTTON(button)) {
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), TRUE);
            g_timeout_add(DEFAULT_KEY_ANIMATION_TIMEOUT, on_toggle_button_idle_reset, button);
        }
    }

    return G_SOURCE_REMOVE;
}

void
kp_keyboard_poll_task(GTask *task, gpointer UNUSED(source_obj), gpointer poll_task_result, GCancellable *cancellable) {
    KpPollTaskResult *result = poll_task_result;
    KpX11KeyboardData *keyboard_data = X11_KEYBOARD_DATA(result->keyboard_data);

    while ("forever") {
        if (g_cancellable_is_cancelled (cancellable)) {
            g_task_return_new_error(task, G_IO_ERROR, G_IO_ERROR_CANCELLED, "Task cancelled");
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
                        KeySym keysym = XkbKeycodeToKeysym(display, ev->detail, 0, 0);
                        if (NoSymbol == keysym) continue;
                        gchar *key_str = XKeysymToString(keysym);
                        if (NULL == key_str) continue;

                        result->poll.key.code = ev->detail;
                        result->poll.key.label = key_str;
                        result->poll.pressed = cookie->evtype == XI_RawKeyPress ? TRUE : FALSE;

                        g_main_context_invoke(NULL, on_poll_task_result, result);
                    }
                }
            }
        }
    }
}