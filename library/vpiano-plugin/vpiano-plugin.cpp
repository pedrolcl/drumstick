/*
    Virtual Piano Widget for Qt
    Copyright (C) 2008-2024, Pedro Lopez-Cabanillas <plcl@users.sf.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along 
    with this program; If not, see <http://www.gnu.org/licenses/>.
*/

#include "vpiano-plugin.h"
#include <QtPlugin>
#include <drumstick/pianokeybd.h>

using namespace drumstick::widgets;

/**
 * @file vpiano-plugin.cpp
 * Implementation of the PianoKeybdPlugin class (Qt Designer plugin)
 */

PianoKeybdPlugin::PianoKeybdPlugin(QObject *parent)
    : QObject(parent)
{ }

void PianoKeybdPlugin::initialize(QDesignerFormEditorInterface * /* core */)
{
    if (initialized)
        return;

    initialized = true;
}

bool PianoKeybdPlugin::isInitialized() const
{
    return initialized;
}

QWidget *PianoKeybdPlugin::createWidget(QWidget *parent)
{
    return new PianoKeybd(parent);
}

QString PianoKeybdPlugin::name() const
{
    return QStringLiteral("drumstick::widgets::PianoKeybd");
}

QString PianoKeybdPlugin::group() const
{
    return QStringLiteral("Drumstick");
}

QIcon PianoKeybdPlugin::icon() const
{
    return QIcon(":/vpiano-plugin.png");
}

QString PianoKeybdPlugin::toolTip() const
{
    return QStringLiteral("Virtual Piano Keyboard");
}

QString PianoKeybdPlugin::whatsThis() const
{
    return QStringLiteral("The Virtual Piano Keyboard is a MIDI controller emulator");
}

bool PianoKeybdPlugin::isContainer() const
{
    return false;
}

QString PianoKeybdPlugin::includeFile() const
{
    return  QStringLiteral("<drumstick/pianokeybd.h>");
}

QString PianoKeybdPlugin::domXml() const
{
    return "<ui language=\"c++\">\n"
           " <widget class=\"drumstick::widgets::PianoKeybd\" name=\"pianoKeybd\">\n"
           "  <property name=\"geometry\">\n"
           "   <rect>\n"
           "    <x>0</x>\n"
           "    <y>0</y>\n"
           "    <width>640</width>\n"
           "    <height>80</height>\n"
           "   </rect>\n"
           "  </property>\n"
           " </widget>\n"
           "</ui>";
}
