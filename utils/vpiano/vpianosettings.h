/*
 Virtual Piano test using the MIDI Sequencer C++ library
 Copyright (C) 2006-2022 Pedro Lopez-Cabanillas <plcl@users.sf.net>

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

#ifndef PORTABLESETTINGS_H
#define PORTABLESETTINGS_H

#include <QObject>
#include <QString>
#include <QFont>
#include <QSettings>
#include <drumstick/backendmanager.h>
#include <drumstick/pianokeybd.h>

class VPianoSettings : public QObject
{
    Q_OBJECT

public:
    static VPianoSettings* instance();

    QByteArray geometry() const;
    void setGeometry(const QByteArray &geometry);

    QByteArray state() const;
    void setState(const QByteArray &state);

    QString lastInputBackend() const;
    void setLastInputBackend(const QString &lastInputBackend);

    QString lastOutputBackend() const;
    void setLastOutputBackend(const QString &lastOutputBackend);

    QString lastInputConnection() const;
    void setLastInputConnection(const QString &lastInputConnection);

    QString lastOutputConnection() const;
    void setLastOutputConnection(const QString &lastOutputConnection);

    bool midiThru() const;
    void setMidiThru(bool midiThru);

    bool advanced() const;
    void setAdvanced(bool advanced);

    int inChannel() const;
    void setInChannel(int inChannel);

    int outChannel() const;
    void setOutChannel(int outChannel);

    int velocity() const;
    void setVelocity(int velocity);

    int baseOctave() const;
    void setBaseOctave(int baseOctave);

    int numKeys() const;
    void setNumKeys(int numKeys);

    int startingKey() const;
    void setStartingKey(int startingKey);

    QVariantMap settingsMap() const;

    drumstick::widgets::LabelOrientation namesOrientation() const;
    void setNamesOrientation(const drumstick::widgets::LabelOrientation namesOrientation);

    drumstick::widgets::LabelVisibility namesVisibility() const;
    void setNamesVisibility(const drumstick::widgets::LabelVisibility namesVisibility);

    drumstick::widgets::LabelAlteration alterations() const;
    void setNamesAlterations(const drumstick::widgets::LabelAlteration alterations);

    QFont namesFont() const;
    void setNamesFont(const QFont &namesFont);

    drumstick::widgets::LabelCentralOctave namesOctave() const;
    void setNamesOctave(const drumstick::widgets::LabelCentralOctave namesOctave);

    drumstick::widgets::LabelNaming namingPolicy() const;
    void setNamingPolicy(const drumstick::widgets::LabelNaming namingPolicy);

    QStringList names_sharps() const;
    void setNames_sharps(const QStringList &names_sharps);

    QStringList names_flats() const;
    void setNames_flats(const QStringList &names_flats);

    bool invertedKeys() const;
    void setInvertedKeys(bool newInvertedKeys);

    bool rawKeyboard() const;
    void setRawKeyboard(bool newRawKeyboard);

    bool keyboardInput() const;
    void setKeyboardInput(bool newKeyboardInput);

    bool mouseInput() const;
    void setMouseInput(bool newMouseInput);

    bool touchScreenInput() const;
    void setTouchScreenInput(bool newTouchScreenInput);

signals:
    void ValuesChanged();

public slots:
    void ResetDefaults();
    void ReadSettings();
    void ReadFromFile(const QString &filepath);
    void SaveSettings();
    void SaveToFile(const QString &filepath);

private:
    explicit VPianoSettings(QObject *parent = nullptr);
    void internalRead(QSettings &settings);
    void internalSave(QSettings &settings);

    QByteArray m_geometry;
    QByteArray m_state;
    QString m_lastInputBackend;
    QString m_lastOutputBackend;
    QString m_lastInputConnection;
    QString m_lastOutputConnection;
    bool m_midiThru;
    bool m_advanced;
    int m_inChannel;
    int m_outChannel;
    int m_velocity;
    int m_baseOctave;
    int m_numKeys;
    int m_startingKey;
    QVariantMap m_settingsMap;
    QVariantMap m_defaultsMap;
    drumstick::widgets::LabelVisibility m_namesVisibility;
    drumstick::widgets::LabelAlteration m_namesAlteration;
    drumstick::widgets::LabelCentralOctave m_namesOctave;
    drumstick::widgets::LabelOrientation m_namesOrientation;
    drumstick::widgets::LabelNaming m_namingPolicy;
    QStringList m_names_sharps;
    QStringList m_names_flats;
    QFont m_namesFont;
    bool m_invertedKeys;
    bool m_rawKeyboard;
    bool m_keyboardInput;
    bool m_mouseInput;
    bool m_touchScreenInput;
};

#endif // PORTABLESETTINGS_H
