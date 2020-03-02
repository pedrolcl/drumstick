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

#include <drumstick/configurationdialogs.h>
#include "networksettingsdialog.h"
#include "fluidsettingsdialog.h"
#include "sonivoxsettingsdialog.h"
#include "macsynthsettingsdialog.h"

namespace drumstick {
namespace widgets {

bool configureInputDriver(const QString driver, QWidget* parent)
{
    if (driver == "Network") {
        NetworkSettingsDialog dlg(parent);
        return (dlg.exec() == QDialog::Accepted);
    }
    return false;
}

bool configureOutputDriver(const QString driver, QWidget* parent)
{
    if (driver == "Network") {
        NetworkSettingsDialog dlg(parent);
        return (dlg.exec() == QDialog::Accepted);
    } else if (driver == "FluidSynth") {
        FluidSettingsDialog dlg(parent);
        return (dlg.exec() == QDialog::Accepted);
    } else if (driver == "SonivoxEAS") {
        SonivoxSettingsDialog dlg(parent);
        return (dlg.exec() == QDialog::Accepted);
    } else if (driver == "DLS Synth") {
        MacSynthSettingsDialog dlg(parent);
        return (dlg.exec() == QDialog::Accepted);
    }
    return false;
}

}}

