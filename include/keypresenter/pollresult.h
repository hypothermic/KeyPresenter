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
 * @brief Definition of the high-level abstract KpKeyboardPollResult enum
 */

#ifndef KEYPRESENTER_POLLRESULT_H
#define KEYPRESENTER_POLLRESULT_H

typedef enum _KeyboardPollResult KpKeyboardPollResult;

enum _KeyboardPollResult {
    POLL_EMPTY = 0,
    POLL_OK    = 1,
    POLL_ERROR = 2,
};

#endif //KEYPRESENTER_POLLRESULT_H
