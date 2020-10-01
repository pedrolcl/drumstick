/*
    MIDI Virtual Piano Keyboard
    Copyright (C) 2008-2020, Pedro Lopez-Cabanillas <plcl@users.sf.net>

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

#ifndef PIANOPALETTE_H
#define PIANOPALETTE_H

#include <QApplication>
#include <QColor>
#include <QList>
#include "macros.h"

/**
 * @file pianopalette.h
 * Piano Palette declarations
 */

namespace drumstick { namespace widgets {

/**
 * @addtogroup Widgets
 * @{
 *
 * @enum PalettePolicy
 * @brief The PalettePolicy enumeration.
 *
 * This enum describes the different kind of color palettes supported, which
 * can be used for highlight, background or foreground colors.
 */
enum PalettePolicy {
    PAL_SINGLE = 0,    ///< Single highlihgting color for all keys
    PAL_DOUBLE = 1,    ///< Two highlihgting colors (naturals/alterations)
    PAL_CHANNELS = 2,  ///< Different highlihgting colors for each channel
    PAL_SCALE = 3,     ///< Background colors for each chromatic scale note
    PAL_KEYS = 4,      ///< Two background colors (naturals/alterations)
    PAL_FONT = 5       ///< Foreground font colors for names
};

/**
 * @brief The PianoPalette class
 */
class DRUMSTICK_EXPORT PianoPalette
{
    Q_GADGET
    Q_DECLARE_TR_FUNCTIONS(PianoPalette)
    Q_ENUM(PalettePolicy)

public:
    explicit PianoPalette(const int id);
    virtual ~PianoPalette() {}

    void resetColors();
    void retranslateStrings();

    int paletteId() const;
    int getNumColors() const;
    bool isHighLight() const;
    bool isBackground() const;
    bool isForeground() const;

    QString paletteName() const;
    void setPaletteName(const QString& name);

    QString paletteText() const;
    void setPaletteText(const QString& text);

    QColor getColor(const int i) const;
    void setColor(const int n, const QString& s, const QColor& c);
    void setColor(const int n, const QColor& c);

    QString getColorName(const int i) const;
    void setColorName(const int n, const QString& s);

    void saveColors() const;
    void loadColors();

    bool operator==(const PianoPalette& other) const;
    bool operator!=(const PianoPalette& other) const;

    static const QString QSTR_PALETTEPREFIX;

protected:
    void initialize();
    void resetPaletteSingle();
    void resetPaletteDouble();
    void resetPaletteChannels();
    void resetPaletteScale();
    void resetPaletteKeys();
    void resetPaletteFont();
    void retranslatePaletteSingle();
    void retranslatePaletteDouble();
    void retranslatePaletteChannels();
    void retranslatePaletteScale();
    void retranslatePaletteKeys();
    void retranslatePaletteFont();

    int m_paletteId;
    QList<QColor> m_colors;
    QList<QString> m_names;
    QString m_paletteName;
    QString m_paletteText;
};

/** @} */

}} // namespace drumstick::widgets

#endif // PIANOPALETTE_H
