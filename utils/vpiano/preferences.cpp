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

#include "preferences.h"
#include "vpianosettings.h"
#include <QPushButton>
#include <QShowEvent>

Preferences::Preferences(QWidget *parent)
    : QDialog(parent)
{
    ui.setupUi( this );
    connect(ui.buttonBox, &QDialogButtonBox::clicked, this, &Preferences::slotButtonClicked);
}

void Preferences::slotButtonClicked(QAbstractButton *button)
{
    if (dynamic_cast<QPushButton *>(button) == ui.buttonBox->button(QDialogButtonBox::Apply)) {
        apply();
    }
}

void Preferences::showEvent ( QShowEvent *event )
{
    if (event->type() == QEvent::Show) {
        ui.spinInChan->setValue( VPianoSettings::instance()->inChannel() );
        ui.spinOutChan->setValue( VPianoSettings::instance()->outChannel() );
        ui.spinVelocity->setValue( VPianoSettings::instance()->velocity() );
        ui.spinBaseOctave->setValue( VPianoSettings::instance()->baseOctave() );
        ui.spinNumKeys->setValue( VPianoSettings::instance()->numKeys() );
        ui.comboNotes->setCurrentIndex( VPianoSettings::instance()->startingKey() );
    }
}

void Preferences::apply()
{
    VPianoSettings::instance()->setInChannel(ui.spinInChan->value());
    VPianoSettings::instance()->setOutChannel(ui.spinOutChan->value());
    VPianoSettings::instance()->setVelocity(ui.spinVelocity->value());
    VPianoSettings::instance()->setBaseOctave(ui.spinBaseOctave->value());
    VPianoSettings::instance()->setNumKeys(ui.spinNumKeys->value());
    VPianoSettings::instance()->setStartingKey(ui.comboNotes->currentIndex());
    VPianoSettings::instance()->SaveSettings();
}

void Preferences::accept()
{
    apply();
    QDialog::accept();
}
