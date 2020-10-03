/*
    Drumstick MIDI Sequencer C++ library
    Copyright (C) 2006-2020, Pedro Lopez-Cabanillas <plcl@users.sf.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef CONFIGURATIONDIALOGS_H
#define CONFIGURATIONDIALOGS_H

#include <QString>
#include <QWidget>
#include "macros.h"

/**
 * @file configurationdialogs.h
 * Functions providing configuration dialogs
 */

namespace drumstick {

/**
 * @ingroup Widgets
 * @brief Drumstick Widgets library
 * MIDI related widgets and functions.
 */
namespace widgets {

bool DRUMSTICK_EXPORT configureInputDriver(const QString driver, QWidget* parent = nullptr);
bool DRUMSTICK_EXPORT configureOutputDriver(const QString driver, QWidget* parent = nullptr);
void DRUMSTICK_EXPORT changeSoundFont(const QString driver, const QString fileName, QWidget* parent = nullptr);
QString DRUMSTICK_EXPORT libraryVersion();

}} // namespace drumstick::widgets

#endif // CONFIGURATIONDIALOGS_H
