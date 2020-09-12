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

#include <QGuiApplication>
#include <QPalette>

#include <drumstick/settingsfactory.h>
#include <drumstick/pianopalette.h>

namespace drumstick {
namespace widgets {

const QString QSTR_PALETTEPREFIX("Palette_");

PianoPalette::PianoPalette(int id) :
    m_paletteId(id)
{
    initialize();
    resetColors();
    retranslateStrings();
}

void
PianoPalette::initialize()
{
    int maxcolors = 0;
    switch(m_paletteId) {
    case PAL_SINGLE:
        maxcolors = 1;
        m_isHighLight = true;
        break;
    case PAL_DOUBLE:
        maxcolors = 2;
        m_isHighLight = true;
        break;
    case PAL_CHANNELS:
        maxcolors = 16;
        m_isHighLight = true;
        break;
    case PAL_SCALE:
        maxcolors = 12;
        m_isHighLight = false;
        break;
    case PAL_KEYS:
        maxcolors = 2;
        m_isHighLight = false;
        break;
    case PAL_FONT:
        maxcolors = 4;
        m_isHighLight = false;
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
    default:
        return;
    }
}

void
PianoPalette::resetPaletteSingle()
{
    setColor(0, QString(), qApp->palette().highlight().color());
}

void
PianoPalette::resetPaletteDouble()
{
    setColor(0, tr("N"), qApp->palette().highlight().color());
    setColor(1, tr("#"), QColor("lawngreen"));
}

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

void
PianoPalette::resetPaletteScale()
{
    /*
                    R       G       B
            C       -       -       100%    0
            C#      50%     -       100%    1
            D       100%    -       100%    2
            D#      100%    -       50%     3
            E       100%    -       -       4
            F       100%    50%     -       5
            F#      100%    100%    -       6
            G       50%     100%    -       7
            G#      -       100%    -       8
            A       -       100%    50%     9
            A#      -       100%    100%    10
            B       -       50%     100%    11
    */
    setColor(0, tr("C"), QColor::fromRgb(0,0,255));
    setColor(1, tr("C#"), QColor::fromRgb(127,0,255));
    setColor(2, tr("D"), QColor::fromRgb(255,0,255));
    setColor(3, tr("D#"), QColor::fromRgb(255,0,127));
    setColor(4, tr("E"), QColor::fromRgb(255,0,0));
    setColor(5, tr("F"), QColor::fromRgb(255,127,0));
    setColor(6, tr("F#"), QColor::fromRgb(255,255,0));
    setColor(7, tr("G"), QColor::fromRgb(127,255,0));
    setColor(8, tr("G#"), QColor::fromRgb(0,255,0));
    setColor(9, tr("A"), QColor::fromRgb(0,255,127));
    setColor(10, tr("A#"), QColor::fromRgb(0,255,255));
    setColor(11, tr("B"), QColor::fromRgb(0,127,255));
}

void PianoPalette::resetPaletteKeys()
{
    setColor(0, tr("N"), QColor("white"));
    setColor(1, tr("#"), QColor("black"));
}

void PianoPalette::resetPaletteFont()
{
    setColor(0, tr("N"), QColor("black"));
    setColor(1, tr("#"), QColor("white"));
    setColor(2, tr("N*"), QColor("white"));
    setColor(3, tr("#*"), QColor("white"));
}

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
    default:
        return;
    }
}

void
PianoPalette::retranslatePaletteSingle()
{
    setColorName(0, QString());
}

void
PianoPalette::retranslatePaletteDouble()
{
    setColorName(0, tr("N"));
    setColorName(1, tr("#"));
}

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

void PianoPalette::retranslatePaletteKeys()
{
    setColorName(0, tr("N"));
    setColorName(1, tr("#"));
}

void PianoPalette::retranslatePaletteFont()
{
    setColorName(0, tr("N"));
    setColorName(1, tr("#"));
    setColorName(2, tr("N*"));
    setColorName(3, tr("#*"));
}

bool PianoPalette::getIsHighLight() const
{
    return m_isHighLight;
}

void PianoPalette::setIsHighLight(bool isHighLight)
{
    m_isHighLight = isHighLight;
}

int
PianoPalette::paletteId() const
{
    return m_paletteId;
}

void
PianoPalette::setColor(const int n, const QString& s, const QColor& c)
{
    if (n < m_colors.size()) {
        m_colors[n] = c;
        m_names[n] = s;
    }
}

void
PianoPalette::setColor(const int n, const QColor& c)
{
    if (n < m_colors.size())
        m_colors[n] = c;
}

void
PianoPalette::setColorName(const int n, const QString& s)
{
    if (n < m_names.size())
        m_names[n] = s;
}

QColor
PianoPalette::getColor(const int i) const
{
    if (i < m_colors.size())
        return m_colors[i];
    return QColor();
}

QString
PianoPalette::getColorName(const int i) const
{
    if (i < m_names.size())
        return m_names[i];
    return QString();
}

int
PianoPalette::getNumColors() const
{
    return m_colors.size();
}

QString
PianoPalette::paletteName() const
{
    return m_paletteName;
}

void
PianoPalette::setPaletteName(const QString& name)
{
    if (m_paletteName != name) {
        m_paletteName = name;
    }
}

QString
PianoPalette::paletteText() const
{
    return m_paletteText;
}

void
PianoPalette::setPaletteText(const QString& help)
{
    m_paletteText = help;
}

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

bool
PianoPalette::operator==(const PianoPalette& other) const
{
    return (m_paletteId == other.m_paletteId) &&
           (m_colors == other.m_colors);
}

bool
PianoPalette::operator!=(const PianoPalette &other) const
{
    return !(*this == other);
}

}} // namespace drumstick::widgets
