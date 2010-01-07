/*
    Virtual Piano test using the MIDI Sequencer C++ library 
    Copyright (C) 2006-2010, Pedro Lopez-Cabanillas <plcl@users.sf.net>

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
    m_baseOctave(4),
    m_numOctaves(5)
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
        ui.spinNumOctaves->setValue( m_numOctaves );
    }
}

void Preferences::apply()
{
    m_inChannel = ui.spinInChan->value();
    m_outChannel = ui.spinOutChan->value();
    m_velocity = ui.spinVelocity->value();
    m_baseOctave = ui.spinBaseOctave->value();
    m_numOctaves = ui.spinNumOctaves->value();
}

void Preferences::accept()
{
    apply();
    QDialog::accept();
}
