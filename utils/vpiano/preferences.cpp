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

#include "preferences.h"
#include <QPushButton>
#include <QShowEvent>

Preferences::Preferences(QWidget *parent)
    : QDialog(parent),
    m_inChannel(0),
    m_outChannel(0),
    m_velocity(100),
    m_baseOctave(1),
    m_numKeys(88),
    m_startingKey(9)
{
    ui.setupUi( this );
    connect(ui.buttonBox, SIGNAL(clicked(QAbstractButton*)), SLOT(slotButtonClicked(QAbstractButton*)));
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
        ui.spinInChan->setValue( m_inChannel );
        ui.spinOutChan->setValue( m_outChannel );
        ui.spinVelocity->setValue( m_velocity );
        ui.spinBaseOctave->setValue( m_baseOctave );
        ui.spinNumKeys->setValue( m_numKeys );
        ui.comboNotes->setCurrentIndex( m_startingKey );
    }
}

void Preferences::apply()
{
    m_inChannel = ui.spinInChan->value();
    m_outChannel = ui.spinOutChan->value();
    m_velocity = ui.spinVelocity->value();
    m_baseOctave = ui.spinBaseOctave->value();
    m_numKeys = ui.spinNumKeys->value();
    m_startingKey = ui.comboNotes->currentIndex();
}

void Preferences::accept()
{
    apply();
    QDialog::accept();
}

void Preferences::setInChannel(const int chan)
{
    m_inChannel = chan;
    ui.spinInChan->setValue(m_inChannel);
}

void Preferences::setOutChannel(const int chan)
{
    m_outChannel = chan;
    ui.spinOutChan->setValue(m_outChannel);
}

void Preferences::setVelocity(const int vel)
{
    m_velocity = vel;
    ui.spinVelocity->setValue(m_velocity);
}

void Preferences::setBaseOctave(const int octave)
{
    m_baseOctave = octave;
    ui.spinBaseOctave->setValue(m_baseOctave);
}

void Preferences::setNumKeys(const int keys)
{
    m_numKeys = keys;
    ui.spinNumKeys->setValue(m_numKeys);
}

void Preferences::setStartingKey(const int key)
{
    m_startingKey = key;
    ui.comboNotes->setCurrentIndex(m_startingKey);
}
