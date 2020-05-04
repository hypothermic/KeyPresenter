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
 * @file poll.h
 * @brief Definition of the high-level abstract KpKeyboardPoll structure
 */

#ifndef KEYPRESENTER_POLL_H
#define KEYPRESENTER_POLL_H

#include <glib.h>

#include "key.h"
#include "pollresult.h"

typedef struct _KeyboardPoll KpKeyboardPoll;

struct _KeyboardPoll {
    KpKeyboardPollResult result;
    guint16 keycode;
};

#endif //KEYPRESENTER_POLL_H
