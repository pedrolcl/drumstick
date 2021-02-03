/*
    Virtual Piano test using the MIDI Sequencer C++ library 
    Copyright (C) 2006-2021, Pedro Lopez-Cabanillas <plcl@users.sf.net>

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.
 
    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
 
    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <QDialog>
#include "ui_preferences.h"

class Preferences : public QDialog
{
    Q_OBJECT

public:
    explicit Preferences(QWidget *parent = nullptr);
    void apply();

public slots:
    void slotButtonClicked(QAbstractButton *button);
    void accept() override;

protected:
    void showEvent ( QShowEvent *event ) override;

private:
    Ui::PreferencesClass ui;
};

#endif // PREFERENCES_H
