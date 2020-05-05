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
 * @file x11.h
 * @brief Internal data structures for XI2 support
 */

#ifndef KEYPRESENTER_X11_H
#define KEYPRESENTER_X11_H

#define X11_KEYBOARD_DATA(keyboard_data) (((KpX11KeyboardData*) keyboard_data))

typedef struct _X11KeyboardData KpX11KeyboardData;

struct _X11KeyboardData {
    /**
     * LibXi extension opcode
     */
    int xi_extension_opcode;

    /**
     * An array with Display element type
     */
    GArray *displays;
};

#endif //KEYPRESENTER_X11_H
