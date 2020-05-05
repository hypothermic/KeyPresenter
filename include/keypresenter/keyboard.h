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
 * @file keyboard.h
 * @brief Definition of abstract high-level keyboard-related methods
 */

#ifndef KEYPRESENTER_KEYBOARD_H
#define KEYPRESENTER_KEYBOARD_H

#include <glib.h>

#include "key.h"
#include "poll.h"

/**
 * Called when the keyboard needs to be initialized.
 *
 * @return (Optional) ptr to an internal data structure used by the keyboard implementation
 */
gpointer
kp_keyboard_init(GtkWindow *window);

/**
 * Retrieve all available keys on the keyboard.
 *
 * @return Ptr to a GArray with elements of KpKey pointer
 */
GArray *
kp_keyboard_get_keys(GtkWindow *window, gpointer internal_keyboard_data);

/**
 * GTask which listens for libXi keyboard presses
 */
void
kp_keyboard_poll_task(GTask *task, gpointer source_obj, gpointer poll_task_result, GCancellable *cancellable);

#endif //KEYPRESENTER_KEYBOARD_H
