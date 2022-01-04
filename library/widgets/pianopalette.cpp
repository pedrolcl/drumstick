/*
    MIDI Virtual Piano Keyboard
    Copyright (C) 2008-2022, Pedro Lopez-Cabanillas <plcl@users.sf.net>

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

#include <QGuiApplication>
#include <QPalette>

#include <drumstick/pianopalette.h>
#include <drumstick/settingsfactory.h>

/**
 * @file pianopalette.cpp
 * Implementation of the Piano Palette
 */

namespace drumstick {
namespace widgets {

/**
 * @brief PianoPalette::QSTR_PALETTEPREFIX is the string prefix for all
 * the settings stored as persisting settings
 */
const QString PianoPalette::QSTR_PALETTEPREFIX = QStringLiteral("Palette_");

/**
 * @brief PianoPalette::PianoPalette Constructor
 * @param id The Palette Policy identifier
 */
PianoPalette::PianoPalette(int id) :
    m_paletteId(id)
{
    initialize();
    resetColors();
    retranslateStrings();
}

/**
 * @brief PianoPalette::initialize reserves and initializes space to store colors
 * according to the palette policy identifier
 */
void
PianoPalette::initialize()
{
    int maxcolors = 0;
    switch(m_paletteId) {
    case PAL_SINGLE:
        maxcolors = 1;
        break;
    case PAL_DOUBLE:
        maxcolors = 2;
        break;
    case PAL_CHANNELS:
        maxcolors = 16;
        break;
    case PAL_SCALE:
        maxcolors = 12;
        break;
    case PAL_KEYS:
        maxcolors = 2;
        break;
    case PAL_FONT:
        maxcolors = 4;
        break;
    case PAL_HISCALE:
        maxcolors = 12;
        break;
    default:
        return;
    }
    m_colors.reserve(maxcolors);
    m_names.reserve(maxcolors);
    for(int i=0; i<maxcolors; ++i) {
        m_colors.insert(i, QColor());
        m_names.insert(i, QString());
    }
}

/**
 * @brief PianoPalette::resetColors resets the colors to the standard values
 * according to the palette policy identifier
 */
void
PianoPalette::resetColors()
{
    switch(m_paletteId) {
    case PAL_SINGLE:
        resetPaletteSingle();
        break;
    case PAL_DOUBLE:
        resetPaletteDouble();
        break;
    case PAL_CHANNELS:
        resetPaletteChannels();
        break;
    case PAL_SCALE:
        resetPaletteScale();
        break;
    case PAL_KEYS:
        resetPaletteKeys();
        break;
    case PAL_FONT:
        resetPaletteFont();
        break;
    case PAL_HISCALE:
        resetPaletteScale();
        break;
    default:
        return;
    }
}

/**
 * @brief PianoPalette::resetPaletteSingle resets the colors to the standard values
 * for the PAL_SINGLE palette policy
 */
void
PianoPalette::resetPaletteSingle()
{
    setColor(0, QString(), qApp->palette().highlight().color());
}

/**
 * @brief PianoPalette::resetPaletteDouble  resets the colors to the standard values
 * for the PAL_DOUBLE palette policy
 */
void
PianoPalette::resetPaletteDouble()
{
    setColor(0, tr("N"), qApp->palette().highlight().color());
    setColor(1, tr("#"), QColor("lawngreen"));
}

/**
 * @brief PianoPalette::resetPaletteChannels resets the colors to the standard values
 * for the PAL_CHANNELS palette policy
 */
void
PianoPalette::resetPaletteChannels()
{
    setColor(0, tr("1"), QColor("red"));
    setColor(1, tr("2"), QColor("lime"));
    setColor(2, tr("3"), QColor("blue"));
    setColor(3, tr("4"), QColor("gold"));
    setColor(4, tr("5"), QColor("maroon"));
    setColor(5, tr("6"), QColor("green"));
    setColor(6, tr("7"), QColor("navy"));
    setColor(7, tr("8"), QColor("darkorange"));
    setColor(8, tr("9"), QColor("purple"));
    setColor(9, tr("10"), qApp->palette().highlight().color());
    setColor(10, tr("11"), QColor("teal"));
    setColor(11, tr("12"), QColor("chocolate"));
    setColor(12, tr("13"), QColor("fuchsia"));
    setColor(13, tr("14"), QColor("olivedrab"));
    setColor(14, tr("15"), QColor("aqua"));
    setColor(15, tr("16"), QColor("greenyellow"));
}

/**
 * @brief PianoPalette::resetPaletteScale resets the colors to the standard values
 * for the PAL_SCALE palette policy
 */
void
PianoPalette::resetPaletteScale()
{
    /*
                    R       G       B
            C       100%    -       -       0
            C#      100%    50%     -       1
            D       100%    100%    -       2
            D#      50%     100%    -       3
            E       -       100%    -       4
            F       -       100%    50%     5
            F#      -       100%    100%    6
            G       -       50%     100%    7
            G#      -       -       100%    8
            A       50%     -       100%    9
            A#      100%    -       100%    10
            B       100%    -       50%     11
    */
    setColor(0,  tr("C"),  QColor::fromRgb(255,0,0));
    setColor(1,  tr("C#"), QColor::fromRgb(255,127,0));
    setColor(2,  tr("D"),  QColor::fromRgb(255,255,0));
    setColor(3,  tr("D#"), QColor::fromRgb(127,255,0));
    setColor(4,  tr("E"),  QColor::fromRgb(0,255,0));
    setColor(5,  tr("F"),  QColor::fromRgb(0,255,127));
    setColor(6,  tr("F#"), QColor::fromRgb(0,255,255));
    setColor(7,  tr("G"),  QColor::fromRgb(0,127,255));
    setColor(8,  tr("G#"), QColor::fromRgb(0,0,255));
    setColor(9,  tr("A"),  QColor::fromRgb(127,0,255));
    setColor(10, tr("A#"), QColor::fromRgb(255,0,255));
    setColor(11, tr("B"),  QColor::fromRgb(255,0,127));
}

/**
 * @brief PianoPalette::resetPaletteKeys resets the colors to the standard values
 * for the PAL_KEYS palette policy
 */
void
PianoPalette::resetPaletteKeys()
{
    setColor(0, tr("N"), QColor("white"));
    setColor(1, tr("#"), QColor("black"));
}

/**
 * @brief PianoPalette::resetPaletteFont resets the colors to the standard values
 * for the PAL_FONT palette policy
 */
void
PianoPalette::resetPaletteFont()
{
    setColor(0, tr("N"), QColor("black"));
    setColor(1, tr("#"), QColor("white"));
    setColor(2, tr("N*"), QColor("white"));
    setColor(3, tr("#*"), QColor("white"));
}

/**
 * @brief PianoPalette::retranslateStrings retranslates the names and description
 * texts according to the palette policy
 */
void
PianoPalette::retranslateStrings()
{
    switch(m_paletteId) {
    case PAL_SINGLE:
        setPaletteName(tr("Single color highlight"));
        setPaletteText(tr("A single color to highlight all note events"));
        retranslatePaletteSingle();
        break;
    case PAL_DOUBLE:
        setPaletteName(tr("Two colors highlight"));
        setPaletteText(tr("One color to highlight natural notes and a different one for accidentals"));
        retranslatePaletteDouble();
        break;
    case PAL_CHANNELS:
        setPaletteName(tr("MIDI Channels highlight"));
        setPaletteText(tr("A different color to highlight each MIDI channel. Enable Omni mode in the MIDI IN connection"));
        retranslatePaletteChannels();
        break;
    case PAL_SCALE:
        setPaletteName(tr("Chromatic scale background"));
        setPaletteText(tr("One color for each note in the chromatic scale"));
        retranslatePaletteScale();
        break;
    case PAL_KEYS:
        setPaletteName(tr("Keys background"));
        setPaletteText(tr("One color for natural notes and another for accidentals"));
        retranslatePaletteKeys();
        break;
    case PAL_FONT:
        setPaletteName(tr("Font foreground"));
        setPaletteText(tr("Colors for note names"));
        retranslatePaletteFont();
        break;
    case PAL_HISCALE:
        setPaletteName(tr("Chromatic scale highlight"));
        setPaletteText(tr("One color for each note in the chromatic scale"));
        retranslatePaletteScale();
        break;
    default:
        return;
    }
}

/**
 * @brief PianoPalette::retranslatePaletteSingle retranslates the color names
 * for the PAL_SINGLE palette policy
 */
void
PianoPalette::retranslatePaletteSingle()
{
    setColorName(0, QString());
}

/**
 * @brief PianoPalette::retranslatePaletteDouble retranslates the color names
 * for the PAL_DOUBLE palette policy
 */
void
PianoPalette::retranslatePaletteDouble()
{
    setColorName(0, tr("N"));
    setColorName(1, tr("#"));
}

/**
 * @brief PianoPalette::retranslatePaletteChannels retranslates the color names
 * for the PAL_CHANNELS palette policy
 */
void
PianoPalette::retranslatePaletteChannels()
{
    setColorName(0, tr("1"));
    setColorName(1, tr("2"));
    setColorName(2, tr("3"));
    setColorName(3, tr("4"));
    setColorName(4, tr("5"));
    setColorName(5, tr("6"));
    setColorName(6, tr("7"));
    setColorName(7, tr("8"));
    setColorName(8, tr("9"));
    setColorName(9, tr("10"));
    setColorName(10, tr("11"));
    setColorName(11, tr("12"));
    setColorName(12, tr("13"));
    setColorName(13, tr("14"));
    setColorName(14, tr("15"));
    setColorName(15, tr("16"));
}

/**
 * @brief PianoPalette::retranslatePaletteScale retranslates the color names
 * for the PAL_SCALE palette policy
 */
void
PianoPalette::retranslatePaletteScale()
{
    setColorName(0, tr("C"));
    setColorName(1, tr("C#"));
    setColorName(2, tr("D"));
    setColorName(3, tr("D#"));
    setColorName(4, tr("E"));
    setColorName(5, tr("F"));
    setColorName(6, tr("F#"));
    setColorName(7, tr("G"));
    setColorName(8, tr("G#"));
    setColorName(9, tr("A"));
    setColorName(10, tr("A#"));
    setColorName(11, tr("B"));
}

/**
 * @brief PianoPalette::retranslatePaletteKeys retranslates the color names
 * for the PAL_KEYS palette policy
 */
void
PianoPalette::retranslatePaletteKeys()
{
    setColorName(0, tr("N"));
    setColorName(1, tr("#"));
}

/**
 * @brief PianoPalette::retranslatePaletteFont retranslates the color names
 * for the PAL_FONT palette policy
 */
void
PianoPalette::retranslatePaletteFont()
{
    setColorName(0, tr("N"));
    setColorName(1, tr("#"));
    setColorName(2, tr("N*"));
    setColorName(3, tr("#*"));
}

/**
 * @brief PianoPalette::isHighLight palette function
 * @return true if the palette is used for keys highlighting
 */
bool
PianoPalette::isHighLight() const
{
    return (m_paletteId == PAL_SINGLE) ||
           (m_paletteId == PAL_DOUBLE) ||
           (m_paletteId == PAL_CHANNELS) ||
           (m_paletteId == PAL_HISCALE);
}

/**
 * @brief PianoPalette::isBackground palette function
 * @return true if the palette is used for painting the keys background
 */
bool
PianoPalette::isBackground() const
{
    return (m_paletteId == PAL_SCALE) ||
           (m_paletteId == PAL_KEYS);
}

/**
 * @brief PianoPalette::isForeground palette function
 * @return true if the palette is used to paint text over the keys
 */
bool
PianoPalette::isForeground() const
{
    return (m_paletteId == PAL_FONT);
}

/**
 * @brief PianoPalette::paletteId palette policy
 * @return the palette policy identifier
 */
int
PianoPalette::paletteId() const
{
    return m_paletteId;
}

/**
 * @brief PianoPalette::setColor changes a palette color
 * @param n the color number
 * @param s the color name
 * @param c the color value
 */
void
PianoPalette::setColor(const int n, const QString& s, const QColor& c)
{
    if (n < m_colors.size()) {
        m_colors[n] = c;
        m_names[n] = s;
    }
}

/**
 * @brief PianoPalette::setColor changes a palette color
 * @param n the color number
 * @param c the color value
 */
void
PianoPalette::setColor(const int n, const QColor& c)
{
    if (n < m_colors.size())
        m_colors[n] = c;
}

/**
 * @brief PianoPalette::setColorName changes a palette color name
 * @param n the color number
 * @param s the color name
 */
void
PianoPalette::setColorName(const int n, const QString& s)
{
    if (n < m_names.size())
        m_names[n] = s;
}

/**
 * @brief PianoPalette::getColor gets a palette color
 * @param i the color number
 * @return  the color value
 */
QColor
PianoPalette::getColor(const int i) const
{
    if (i < m_colors.size())
        return m_colors[i];
    return {};
}

/**
 * @brief PianoPalette::getColorName gets a palette color name
 * @param i the color number
 * @return  the color name
 */
QString
PianoPalette::getColorName(const int i) const
{
    if (i < m_names.size())
        return m_names[i];
    return QString();
}

/**
 * @brief PianoPalette::getNumColors palette policy colors size
 * @return the number of colors represented by the palette
 */
int
PianoPalette::getNumColors() const
{
    return m_colors.size();
}

/**
 * @brief PianoPalette::paletteName palette policy name
 * @return the name of the palette
 */
QString
PianoPalette::paletteName() const
{
    return m_paletteName;
}

/**
 * @brief PianoPalette::setPaletteName changes the palette name
 * @param name new name of the palette
 */
void
PianoPalette::setPaletteName(const QString& name)
{
    if (m_paletteName != name) {
        m_paletteName = name;
    }
}

/**
 * @brief PianoPalette::paletteText gets the palette description
 * @return new description of the palette
 */
QString
PianoPalette::paletteText() const
{
    return m_paletteText;
}

/**
 * @brief PianoPalette::setPaletteText changes the palette description
 * @param help new palette description string
 */
void
PianoPalette::setPaletteText(const QString& help)
{
    m_paletteText = help;
}

/**
 * @brief PianoPalette::saveColors stores the set of colors as persistent settings
 */
void
PianoPalette::saveColors() const
{
    SettingsFactory settings;
    settings->beginWriteArray(QSTR_PALETTEPREFIX + QString::number(m_paletteId));
    for(int i=0; i<m_colors.size(); ++i) {
        settings->setArrayIndex(i);
        settings->setValue("color", m_colors[i]);
    }
    settings->endArray();
    settings->sync();
}

/**
 * @brief PianoPalette::loadColors loads the set of colors from persistent settings
 */
void
PianoPalette::loadColors()
{
    SettingsFactory settings;
    int size = settings->beginReadArray(QSTR_PALETTEPREFIX + QString::number(m_paletteId));
    if (size > m_colors.size())
        size = m_colors.size();
    for(int i=0; i<size; ++i) {
        settings->setArrayIndex(i);
        QColor c = settings->value("color", QColor()).value<QColor>();
        setColor(i, c);
    }
    settings->endArray();
}

/**
 * @brief PianoPalette::operator == compares two palettes
 * @param other another palette object
 * @return true if both palettes are equal
 */
bool
PianoPalette::operator==(const PianoPalette& other) const
{
    return (m_paletteId == other.m_paletteId) &&
           (m_colors == other.m_colors);
}

/**
 * @brief PianoPalette::operator != compares two palettes
 * @param other another palette object
 * @return true if both palettes are different
 */
bool
PianoPalette::operator!=(const PianoPalette &other) const
{
    return !(*this == other);
}

/**
 * @brief Serialize a PianoPalette instance into a QDataStream
 * @param stream a QDataStream
 * @param palette instance
 * @return the QDataStream
 */
QDataStream &operator<<(QDataStream &stream, const PianoPalette &palette)
{
    stream << palette.m_paletteId;
    stream << palette.m_colors;
    stream << palette.m_names;
    stream << palette.m_paletteName;
    stream << palette.m_paletteText;
    return stream;
}

/**
 * @brief Deserialize a PianoPalette instance from a QDataStream
 * @param stream a QDataStream
 * @param palette instance
 * @return the QDataStream
 */
QDataStream &operator>>(QDataStream &stream, PianoPalette &palette)
{
    stream >> palette.m_paletteId;
    stream >> palette.m_colors;
    stream >> palette.m_names;
    stream >> palette.m_paletteName;
    stream >> palette.m_paletteText;
    return stream;
}

} // namespace widgets
} // namespace drumstick
