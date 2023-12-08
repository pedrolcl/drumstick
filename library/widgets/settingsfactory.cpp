/*
 Drumstick Widgets
 Copyright (C) 2018-2023 Pedro Lopez-Cabanillas <plcl@users.sf.net>

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

/**
 * @file settingsfactory.cpp
 * Implementation of the Settings Factory class
 */

namespace drumstick { namespace widgets {

/**
 * @brief SettingsFactory::s_fileName is a global string providing the file name
 * of the persistent settings using the INI file format
 */
QString SettingsFactory::s_fileName;

/**
 * @brief SettingsFactory::setFileName sets the global file name for the
 * persistent settings and sets the INI format as well
 * @param name the new file name
 */
void SettingsFactory::setFileName(const QString name)
{
    SettingsFactory::s_fileName = name;
    if (name.isEmpty()) {
        QSettings::setDefaultFormat(QSettings::NativeFormat);
    } else {
        QSettings::setDefaultFormat(QSettings::IniFormat);
    }
}

/**
 * @brief SettingsFactory::fileName returns the file name of the persistent settings
 * @return QString the global file name of the persistent settings
 */
QString SettingsFactory::fileName()
{
    return s_fileName;
}

/**
 * @brief SettingsFactory::format returns the storage format of the persistent settings
 * @return QSettings::Format the storage format of the persistent settings
 */
QSettings::Format SettingsFactory::format()
{
    return QSettings::defaultFormat();
}

/**
 * @brief SettingsFactory::getQSettings creates and/or returns a QSettings object pointer
 * @return the internal QSettings object pointer
 */
QSettings*
SettingsFactory::getQSettings()
{
    if (m_settings.isNull()) {
        if (s_fileName.isEmpty() || QSettings::defaultFormat() == QSettings::NativeFormat) {
            m_settings.reset(new QSettings());
        } else {
            m_settings.reset(new QSettings(s_fileName, QSettings::IniFormat));
        }
        m_settings->setAtomicSyncRequired(true);
    }
    return m_settings.data();
}

/**
 * @brief SettingsFactory::operator -> is equivalent to calling getQSettings()
 * @return  the internal QSettings object pointer
 */
QSettings*
SettingsFactory::operator->()
{
    return getQSettings();
}

} // namespace widgets
} // namespace drumstick
