/*
    Drumstick MIDI Sequencer C++ library
    Copyright (C) 2006-2024, Pedro Lopez-Cabanillas <plcl@users.sf.net>

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

#include <QMetaMethod>
#include <drumstick/backendmanager.h>
#include <drumstick/configurationdialogs.h>

#if defined(ENABLE_FLUIDSYNTH)
#include "fluidsettingsdialog.h"
#endif
#if defined(ENABLE_NETWORK)
#include "networksettingsdialog.h"
#endif
#if defined(ENABLE_SONIVOX)
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
 * @brief inputDriverIsConfigurable
 * @param driver the driver name
 * @return true if the input driver has a configuration dialog
 */
bool inputDriverIsConfigurable(const QString driver)
{
    // internal configuration dialogs:
    if (driver == "Network") {
        return true;
    }
    // external configuration dialogs (residing on plugins):
    drumstick::rt::BackendManager man;
    auto obj = man.inputBackendByName(driver);
    if (obj == nullptr) {
        return false;
    }
    auto metaObj = obj->metaObject();
    if ((metaObj->indexOfProperty("isconfigurable") != -1) &&
        (metaObj->indexOfMethod("configure(QWidget*)") != -1)) {
        auto configurable = obj->property("isconfigurable");
        if (configurable.isValid()) {
            return configurable.toBool();
        }
    }
    return false;
}

/**
 * @brief outputDriverIsConfigurable
 * @param driver the driver name
 * @return  true if the output driver has a configuration dialog
 */
bool outputDriverIsConfigurable(const QString driver)
{
    // internal configuration dialogs
    if ((driver == "Network")
#if defined(ENABLE_SONIVOX)
        || (driver == "SonivoxEAS")
#endif
#if defined(Q_OS_MACOS)
        || (driver == "DLS Synth")
#endif
#if defined(ENABLE_FLUIDSYNTH)
        || (driver == "FluidSynth")
#endif
    ) {
        return true;
    }
    // external configuration dialogs (residing on plugins)
    drumstick::rt::BackendManager man;
    auto obj = man.outputBackendByName(driver);
    if (obj == nullptr) {
        return false;
    }
    auto metaObj = obj->metaObject();
    if ((metaObj->indexOfProperty("isconfigurable") != -1) &&
        (metaObj->indexOfMethod("configure(QWidget*)") != -1)) {
        auto configurable = obj->property("isconfigurable");
        if (configurable.isValid()) {
            return configurable.toBool();
        }
    }
    return false;
}

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
    // internal configuration dialogs
#if defined(ENABLE_NETWORK)
    if (driver == "Network") {
        NetworkSettingsDialog dlg(true, parent);
        return (dlg.exec() == QDialog::Accepted);
    }
#endif
    // external configuration dialogs (residing on plugins):
    drumstick::rt::BackendManager man;
    auto obj = man.inputBackendByName(driver);
    if (obj == nullptr) {
        return false;
    }
    auto metaObj = obj->metaObject();
    if ((metaObj->indexOfProperty("isconfigurable") != -1) &&
        (metaObj->indexOfMethod("configure(QWidget*)") != -1)) {
        auto configurable = obj->property("isconfigurable");
        if (configurable.isValid() && configurable.toBool()) {
            bool ret{false};
            QMetaObject::invokeMethod(obj, "configure", Q_RETURN_ARG(bool, ret), Q_ARG(QWidget*, parent));
            return ret;
        }
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
    // internal configuration dialogs
#if defined(ENABLE_NETWORK)
    if (driver == "Network") {
        NetworkSettingsDialog dlg(false, parent);
        return (dlg.exec() == QDialog::Accepted);
    }
#endif
#if defined(ENABLE_FLUIDSYNTH)
    if (driver == "FluidSynth") {
        FluidSettingsDialog dlg(parent);
        return (dlg.exec() == QDialog::Accepted);
    }
#endif
#if defined(ENABLE_SONIVOX)
    if (driver == "SonivoxEAS") {
        SonivoxSettingsDialog dlg(parent);
        return (dlg.exec() == QDialog::Accepted);
    }
#endif
#if defined(Q_OS_MACOS)
    if (driver == "DLS Synth") {
        MacSynthSettingsDialog dlg(parent);
        return (dlg.exec() == QDialog::Accepted);
    }
#endif
    // external configuration dialogs (residing on plugins):
    drumstick::rt::BackendManager man;
    auto obj = man.outputBackendByName(driver);
    if (obj == nullptr) {
        return false;
    }
    auto metaObj = obj->metaObject();
    if ((metaObj->indexOfProperty("isconfigurable") != -1) &&
        (metaObj->indexOfMethod("configure(QWidget*)") != -1)) {
        auto configurable = obj->property("isconfigurable");
        if (configurable.isValid() && configurable.toBool()) {
            bool ret{true};
            QMetaObject::invokeMethod(obj, "configure", Q_RETURN_ARG(bool, ret), Q_ARG(QWidget*, parent));
            return ret;
        }
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
#if defined(ENABLE_FLUIDSYNTH)
    if (driver == "FluidSynth") {
        FluidSettingsDialog dlg(parent);
        dlg.changeSoundFont(fileName);
    }
#endif
#if defined(ENABLE_SONIVOX)
    if (driver == "SonivoxEAS") {
        SonivoxSettingsDialog dlg(parent);
        dlg.changeSoundFont(fileName);
    }
#endif
#if defined(Q_OS_MACOS)
    if (driver == "DLS Synth") {
        MacSynthSettingsDialog dlg(parent);
        dlg.changeSoundFont(fileName);
    }
#endif
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
