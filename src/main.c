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
 * @brief Application entry-point
 */

#include <cairo.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <stdio.h>

#include <keypresenter/keypresenter.h>
#include "appstate.h"
#include "macro.h"
#include "polltaskresult.h"

#ifdef KEYPRESENTER_BUILD_USE_X11
#include "x11.h"
#endif

#define WINDOW_LEAVE_EVENT_BOUNDS_MARGIN 5

static void on_screen_changed(GtkWidget *window, GdkScreen *old_screen, gpointer app_state);
static gboolean on_draw(GtkWidget *window, GdkEventExpose *event, gpointer app_state);
static gboolean on_enter(GtkWidget *window, GdkEventCrossing *event, gpointer app_state_p);
static gboolean on_leave(GtkWidget *window, GdkEventCrossing *event, gpointer app_state_p);

static const gchar *NOTICE = "\nKeypresenter  Copyright (C) 2020  https://www.hypothermic.nl\n"
                             "This program comes with ABSOLUTELY NO WARRANTY.\n"
                             "This is free software, and you are welcome to redistribute it under\n"
                             "certain conditions which are described in the GNU GPL v3 license.\n\n";

gint
main(gint argc, gchar **argv) {
    GtkWidget *window, *grid;
    gpointer keyboard_data;
    GArray *keyboard_keys;
    GHashTable *key_button_table;
    AppState app_state = {FALSE, TRUE};

    fprintf(stdout, "%s\n", NOTICE);
    gtk_init(&argc, &argv);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), -1, -1);
    gtk_window_set_title(GTK_WINDOW(window), KEYPRESENTER_APP_NAME);
    gtk_window_set_keep_above(GTK_WINDOW(window), TRUE);
    g_signal_connect(G_OBJECT(window), "delete-event", gtk_main_quit, NULL);

    gtk_widget_set_app_paintable(window, TRUE);

    g_signal_connect(G_OBJECT(window), "draw", G_CALLBACK(on_draw), &app_state);
    g_signal_connect(G_OBJECT(window), "screen-changed", G_CALLBACK(on_screen_changed), NULL);

    gtk_window_set_decorated(GTK_WINDOW(window), FALSE);
    gtk_widget_add_events(window, GDK_BUTTON_PRESS_MASK);

    g_signal_connect(G_OBJECT(window), "enter-notify-event", G_CALLBACK(on_enter), &app_state);
    g_signal_connect(G_OBJECT(window), "leave-notify-event", G_CALLBACK(on_leave), &app_state);

    grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(window), grid);

    gtk_grid_set_row_spacing(GTK_GRID(grid), 3);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 3);
    gtk_widget_set_margin_top(grid, 8);
    gtk_widget_set_margin_start(grid, 8);
    gtk_widget_set_margin_bottom(grid, 8);
    gtk_widget_set_margin_end(grid, 8);

    keyboard_data = kp_keyboard_init(GTK_WINDOW(window));
    keyboard_keys = kp_keyboard_get_keys(GTK_WINDOW(window), keyboard_data);

    key_button_table = g_hash_table_new(g_int64_hash, g_int64_equal);

#ifdef AUTO_LOOKUP_AVAILABLE_KEYS
    uint row_width = keyboard_keys->len / 15;
#endif
    uint current_x = 0, current_y = 0, n = 0;

    for (int i = 0; i < keyboard_keys->len; ++i) {
        KpKey* key = g_array_index(keyboard_keys, KpKey*, i);

#ifdef NDEBUG
        fprintf(stderr, "Found key %s with code %d\n", key->label, key->code);
#endif
        // TODO add these to a GHashTable to find them later.
        GtkWidget *button = gtk_toggle_button_new_with_label(key->label);
        gtk_button_set_relief(GTK_BUTTON(button), GTK_RELIEF_HALF);
        gtk_widget_set_size_request(button, 85, 50);
        if (g_strcmp0(key->label, "space") == 0) {
            if (n++ > 0) {
                gtk_grid_attach(GTK_GRID(grid), button, 0, current_y, 10, 1);
            }
        } else {
            gtk_grid_attach(GTK_GRID(grid), button, current_x, current_y, 1, 1);
        }

        g_hash_table_insert(key_button_table, &key->code, button);

#ifdef AUTO_LOOKUP_AVAILABLE_KEYS
        if (++current_x > row_width) {
#else
        current_x++;
        if (g_strcmp0(key->label, "0") == 0
            || g_strcmp0(key->label, "p") == 0
            || g_strcmp0(key->label, "l") == 0
            || g_strcmp0(key->label, "m") == 0) {
#endif
            current_y++;
            current_x = 0;
        }
    }

    KpPollTaskResult *task_result = g_new0(KpPollTaskResult, 1);
    task_result->keyboard_data = keyboard_data;
    task_result->key_button_table = key_button_table;

    GCancellable *cancellable = g_cancellable_new();
    GTask *task = g_task_new(window, cancellable, NULL, task_result);
    g_task_set_task_data(task, task_result, g_free);
    g_task_run_in_thread(task, kp_keyboard_poll_task);
    g_object_unref(task);

    // Trigger initial screen change
    on_screen_changed(window, NULL, &app_state);

    gtk_widget_show_all(window);
    gtk_main();

    return EXIT_SUCCESS;
}

static void
on_screen_changed(GtkWidget *window, GdkScreen *old_screen, gpointer app_state) {
    GdkScreen *screen = gtk_widget_get_screen(window);
    GdkVisual *visual = gdk_screen_get_rgba_visual(screen);

    if (!visual) {
        visual = gdk_screen_get_system_visual(screen);
        APP_STATE(app_state)->screen_supports_alpha_channel = FALSE;
    } else {
        APP_STATE(app_state)->screen_supports_alpha_channel = TRUE;
    }

    gtk_widget_set_visual(window, visual);
}

static gboolean
on_draw(GtkWidget *window, GdkEventExpose *event, gpointer app_state_p) {
    cairo_t *cr = gdk_cairo_create(gtk_widget_get_window(window));
    AppState *app_state = APP_STATE(app_state_p);

    if (app_state->screen_supports_alpha_channel) {
        cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, app_state->is_transparent ? 0.0 : 0.4);
    } else {
        cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    }

    cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
    cairo_paint(cr);

    cairo_destroy(cr);

    return FALSE;
}

static gboolean
on_enter(GtkWidget *window, GdkEventCrossing *event, gpointer app_state_p) {
    gtk_window_set_decorated(GTK_WINDOW(window), TRUE);
    APP_STATE(app_state_p)->is_transparent = FALSE;

    return FALSE;
}

static gboolean
on_leave(GtkWidget *window, GdkEventCrossing *event, gpointer app_state_p) {
    // If mouse pointer is moving to the top side of the window (titlebar),
    // no action is taken and the event is propagated further.
    if (event->y <= 1) {
        return TRUE;
    }

    // If mouse pointer has left to another widget within this window, it will sometimes trigger this leave event.
    // To counter these incorrect leave events, check if the mouse pointer is still in the window's bounds.
    gint window_width, window_height;
    gtk_window_get_size(GTK_WINDOW(window), &window_width, &window_height);

    if (event->x > WINDOW_LEAVE_EVENT_BOUNDS_MARGIN
        && event->x < window_width - WINDOW_LEAVE_EVENT_BOUNDS_MARGIN
        && event->y < window_height - WINDOW_LEAVE_EVENT_BOUNDS_MARGIN) {

        return TRUE;
    }

#ifdef NDEBUG
    fprintf(stderr, "Leaving window at %0.0f, %0.0f - window size is %d, %d\n", event->x, event->y, window_width, window_height);
#endif

    gtk_window_set_decorated(GTK_WINDOW(window), FALSE);
    APP_STATE(app_state_p)->is_transparent = TRUE;
    return FALSE;
}

#undef WINDOW_LEAVE_EVENT_BOUNDS_MARGIN