/*
    Virtual Piano test using the MIDI Sequencer C++ library 
    Copyright (C) 2006-2015, Pedro Lopez-Cabanillas <plcl@users.sf.net>

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
 
    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
 
    You should have received a copy of the GNU General Public License along 
    with this program; if not, write to the Free Software Foundation, Inc., 
    51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.    
*/

#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <QDialog>
#include "ui_preferences.h"

class Preferences : public QDialog
{
    Q_OBJECT

public:
    Preferences(QWidget *parent = 0);

    void setInChannel(const int chan);
    void setOutChannel(const int chan);
    void setVelocity(const int vel);
    void setBaseOctave(const int octave);
    void setNumKeys(const int keys);
    void setStartingKey(const int key);

    int getInChannel() const { return m_inChannel; }
    int getOutChannel() const { return m_outChannel; }
    int getVelocity() const { return m_velocity; }
    int getBaseOctave() const { return m_baseOctave; }
    int getNumKeys() const { return m_numKeys; }
    int getStartingKey() const { return m_startingKey; }

    void apply();

public slots:
    void slotButtonClicked(QAbstractButton *button);
    void accept();

protected:
    void showEvent ( QShowEvent *event ); 

private:
    Ui::PreferencesClass ui;
    int m_inChannel;
    int m_outChannel;
    int m_velocity;
    int m_baseOctave;
    int m_numKeys;
    int m_startingKey;
};

#endif // PREFERENCES_H
