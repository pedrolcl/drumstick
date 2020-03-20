/*
 Drumstick Widgets
 Copyright (C) 2018-2020 Pedro Lopez-Cabanillas <plcl@users.sf.net>

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <QFileInfo>
#include <drumstick/settingsfactory.h>

namespace drumstick { namespace widgets {

QString SettingsFactory::s_fileName;

void SettingsFactory::setFileName(const QString name)
{
    SettingsFactory::s_fileName = name;
    QSettings::setDefaultFormat(QSettings::IniFormat);
}

QSettings* SettingsFactory::getQSettings()
{
    if (m_settings.isNull()) {
        if (s_fileName.isEmpty() || QSettings::defaultFormat() == QSettings::NativeFormat) {
            m_settings.reset(new QSettings());
        } else {
            m_settings.reset(new QSettings(s_fileName, QSettings::IniFormat));
        }
    }
    return m_settings.get();
}

QSettings &SettingsFactory::operator*()
{
    return *getQSettings();
}

QSettings *SettingsFactory::operator->()
{
    return getQSettings();
}

}} // namespace drumstick::widgets
