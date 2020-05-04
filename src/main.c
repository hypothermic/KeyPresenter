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

#define WINDOW_LEAVE_EVENT_BOUNDS_MARGIN 5

static void on_screen_changed(GtkWidget *window, GdkScreen *old_screen, gpointer app_state);
static gboolean on_draw(GtkWidget *window, GdkEventExpose *event, gpointer app_state);
static void on_clicked(GtkButton *button, gpointer window);
static gboolean on_enter(GtkWidget *window, GdkEventCrossing *event);
static gboolean on_leave(GtkWidget *window, GdkEventCrossing *event);

static gchar *NOTICE = "\nKeypresenter  Copyright (C) 2020  https://www.hypothermic.nl\n"
                       "This program comes with ABSOLUTELY NO WARRANTY.\n"
                       "This is free software, and you are welcome to redistribute it under\n"
                       "certain conditions which are described in the GNU GPL v3 license.\n\n";

gint
main(gint argc, gchar **argv) {
    AppState app_state;

    printf("%s\n", NOTICE);
    gtk_init(&argc, &argv);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 400);
    gtk_window_set_title(GTK_WINDOW(window), KEYPRESENTER_APP_NAME);
    g_signal_connect(G_OBJECT(window), "delete-event", gtk_main_quit, NULL);

    gtk_widget_set_app_paintable(window, TRUE);

    g_signal_connect(G_OBJECT(window), "draw", G_CALLBACK(on_draw), &app_state);
    g_signal_connect(G_OBJECT(window), "screen-changed", G_CALLBACK(on_screen_changed), NULL);

    gtk_window_set_decorated(GTK_WINDOW(window), FALSE);
    gtk_widget_add_events(window, GDK_BUTTON_PRESS_MASK);

    g_signal_connect(G_OBJECT(window), "enter-notify-event", G_CALLBACK(on_enter), NULL);
    g_signal_connect(G_OBJECT(window), "leave-notify-event", G_CALLBACK(on_leave), NULL);

    GtkWidget *fixed_container = gtk_fixed_new();
    gtk_container_add(GTK_CONTAINER(window), fixed_container);
    GtkWidget *button = gtk_button_new_with_label("button1");
    gtk_widget_set_size_request(button, 100, 100);
    gtk_container_add(GTK_CONTAINER(fixed_container), button);

    g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(on_clicked), window);

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
on_draw(GtkWidget *window, GdkEventExpose *event, gpointer app_state) {
    cairo_t *cr = gdk_cairo_create(gtk_widget_get_window(window));

    if (APP_STATE(app_state)->screen_supports_alpha_channel) {
        cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.0);
    } else {
        cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    }

    cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
    cairo_paint(cr);

    cairo_destroy(cr);

    return FALSE;
}

static void
on_clicked(GtkButton *button, gpointer window) {
    GtkWindow *_window = GTK_WINDOW(window);
}

static gboolean
on_enter(GtkWidget *window, GdkEventCrossing *event) {
    gtk_window_set_decorated(GTK_WINDOW(window), TRUE);
    return FALSE;
}

static gboolean
on_leave(GtkWidget *window, GdkEventCrossing *event) {
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

    fprintf(stderr, "Leaving window at %0.0f, %0.0f - window size is %d, %d\n", event->x, event->y, window_width, window_height);

    gtk_window_set_decorated(GTK_WINDOW(window), FALSE);
    return FALSE;
}

#undef WINDOW_LEAVE_EVENT_BOUNDS_MARGIN