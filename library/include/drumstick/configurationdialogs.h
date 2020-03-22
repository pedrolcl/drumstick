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
 * MIDI Widgets
 * @defgroup Widgets MIDI Widgets
 * @{
 */

namespace drumstick {

//! Drumstick Widgets library
namespace widgets {

/**
 * @brief Input Driver configuration dialog
 * @param driver name of the driver
 * @param parent optional parent widget
 * @return true if configuration has changed
 */
bool DRUMSTICK_EXPORT configureInputDriver(const QString driver, QWidget* parent = nullptr);

/**
 * @brief Output Driver configuration dialog
 * @param driver name of the driver
 * @param parent optional parent widget
 * @return true if configuration has changed
 */
bool DRUMSTICK_EXPORT configureOutputDriver(const QString driver, QWidget* parent = nullptr);

/**
 * @brief Changes the sound font configuration
 * @param driver name of the driver
 * @param fileName name of the soundfont file
 * @param parent optional parent widget
 * @return true if configuration has changed
 */
void DRUMSTICK_EXPORT changeSoundFont(const QString driver, const QString fileName, QWidget* parent = nullptr);

}}

/** @} */

#endif // CONFIGURATIONDIALOGS_H
