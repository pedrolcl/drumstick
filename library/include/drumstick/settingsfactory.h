/*
 Drumstick Widgets
 Copyright (C) 2018-2024 Pedro Lopez-Cabanillas <plcl@users.sf.net>

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

#ifndef SETTINGS_H
#define SETTINGS_H

#include <QSettings>
#include <QScopedPointer>
#include "macros.h"

/**
 * @file settingsfactory.h
 * SettingsFactory class declaration
 */

#if defined(DRUMSTICK_STATIC)
#define DRUMSTICK_WIDGETS_EXPORT
#else
#if defined(drumstick_widgets_EXPORTS)
#define DRUMSTICK_WIDGETS_EXPORT Q_DECL_EXPORT
#else
#define DRUMSTICK_WIDGETS_EXPORT Q_DECL_IMPORT
#endif
#endif

/**
 * @class QSettings
 * @brief The QSettings class provides persistent platform-independent application settings.
 * @see https://doc.qt.io/qt-5/qsettings.html
 */

namespace drumstick { namespace widgets {

/**
 * @addtogroup Widgets
 * @{
 *
 * @class SettingsFactory
 * @brief The SettingsFactory class holds a global QSettings object.
 * This class creates and returns a QSettings object globally configured
 * instance using native or file storage.
 */
class DRUMSTICK_WIDGETS_EXPORT SettingsFactory
{
public:
    QSettings *getQSettings();
    QSettings *operator->();
    static void setFileName(const QString name);
    static QString fileName();
    static QSettings::Format format();

private:
    QScopedPointer<QSettings> m_settings{nullptr};
    static QString s_fileName;
};

/** @} */
}} // namespace drumstick::widgets

#endif // SETTINGS_H
