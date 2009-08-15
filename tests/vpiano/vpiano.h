/*
    Virtual Piano test using the MIDI Sequencer C++ library
    Copyright (C) 2006-2009, Pedro Lopez-Cabanillas <plcl@users.sf.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef VPIANO_H
#define VPIANO_H

#include <QMainWindow>
#include "ui_vpiano.h"
#include "about.h"
#include "connections.h"
#include "preferences.h"

#include "alsaevent.h"
#include "alsaclient.h"
#include "alsaport.h"
#include "alsaqueue.h"

using namespace aseqmm;

/* MidiClient can deliver SequencerEvents with only
 * signals or posting QEvents to the QApplication loop */
#undef USE_QEVENTS
//#define USE_QEVENTS

class VPiano : public QMainWindow
{
    Q_OBJECT

public:
    VPiano( QWidget * parent = 0, Qt::WindowFlags flags = 0 );
    virtual ~VPiano();

public slots:
    void slotAbout();
    void slotAboutQt();
    void slotConnections();
    void slotPreferences();
    void slotNoteOn(const int midiNote);
    void slotNoteOff(const int midiNote);
    void slotSubscription(MidiPort* port, Subscription* subs);

#ifdef USE_QEVENTS
protected:
    virtual void customEvent( QEvent *ev );
#else
    void sequencerEvent( SequencerEvent* ev );
#endif

private:
    void displayEvent( SequencerEvent* ev );

    int m_portId;
    MidiClient* m_Client;
    MidiPort* m_Port;
    Ui::VPiano ui;
    About dlgAbout;
    Connections dlgConnections;
    Preferences dlgPreferences;
};

#endif // VPIANO_H
