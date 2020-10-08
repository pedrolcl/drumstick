/*
    Drumstick MIDI Sequencer C++ library
    Copyright (C) 2006-2020, Pedro Lopez-Cabanillas <plcl@users.sf.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "fluidsettingsdialog.h"
#include "networksettingsdialog.h"
#include <drumstick/configurationdialogs.h>
#if defined(Q_OS_LINUX)
#include "sonivoxsettingsdialog.h"
#endif
#if defined(Q_OS_MACOS)
#include "macsynthsettingsdialog.h"
#endif

/**
 * @file configurationdialogs.cpp
 * Implementation of the configuration dialogs
 */

/**
 * @addtogroup Widgets Drumstick Widgets
 * @{
 */

namespace drumstick { namespace widgets {

/**
 * @brief Input Driver configuration dialog
 * Some RT input drivers can be configured. This function provides a
 * dialog box to show and edit the configurable parameters, and
 * save the settings. Curremtly, only the Network driver is configurable.
 * @param driver name of the driver
 * @param parent optional parent widget
 * @return true if configuration has changed
 */
bool configureInputDriver(const QString driver, QWidget* parent)
{
    if (driver == "Network") {
        NetworkSettingsDialog dlg(parent);
        return (dlg.exec() == QDialog::Accepted);
    }
    return false;
}

/**
 * @brief Output Driver configuration dialog
 * Some RT output drivers can be configured. This function provides a
 * dialog box to show and edit the configurable parameters, and
 * save the settings. Curremtly the Network, FluidSynth, SonivoxEAS,
 * and macOS DLS Synth drivers are configurable.
 * @param driver name of the driver
 * @param parent optional parent widget
 * @return true if configuration has changed
 */
bool configureOutputDriver(const QString driver, QWidget* parent)
{
    if (driver == "Network") {
        NetworkSettingsDialog dlg(parent);
        return (dlg.exec() == QDialog::Accepted);
    } else if (driver == "FluidSynth") {
        FluidSettingsDialog dlg(parent);
        return (dlg.exec() == QDialog::Accepted);
#if defined(Q_OS_LINUX)
    } else if (driver == "SonivoxEAS") {
        SonivoxSettingsDialog dlg(parent);
        return (dlg.exec() == QDialog::Accepted);
#endif
#if defined(Q_OS_MACOS)
    } else if (driver == "DLS Synth") {
        MacSynthSettingsDialog dlg(parent);
        return (dlg.exec() == QDialog::Accepted);
#endif
    }
    return false;
}

/**
 * @brief Changes the sound font configuration
 * Some RT output drivers accept soundfonts. This function allows to
 * change the soundfont file for a driver and store the setting. The
 * FluidSynth and macOS DLS Synth drivers are currently supported.
 * @param driver name of the driver
 * @param fileName name of the soundfont file
 * @param parent optional parent widget
 * @return true if configuration has changed
 */
void changeSoundFont(const QString driver, const QString fileName, QWidget* parent)
{
    if (driver == "FluidSynth") {
        FluidSettingsDialog dlg(parent);
        dlg.changeSoundFont(fileName);
#if defined(Q_OS_MACOS)
    } else if (driver == "DLS Synth") {
        MacSynthSettingsDialog dlg(parent);
        dlg.changeSoundFont(fileName);
#endif
    }
}

/**
 * @brief libraryVersion returns the runtime library version as a QString
 * @return version string
 */
QString libraryVersion()
{
    return QStringLiteral(QT_STRINGIFY(VERSION));
}

} // namespace widgets
} // namespace drumstick

/** @} */
