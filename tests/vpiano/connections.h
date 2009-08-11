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

#ifndef CONNECTIONS_H
#define CONNECTIONS_H

#include <QDialog>
#include "alsaport.h"
#include "ui_connections.h"
#include "connectionitem.h"

USE_ALSASEQ_NAMESPACE

class Connections : public QDialog
{
    Q_OBJECT

public:
    Connections(QWidget *parent = 0);
    void setInputs(PortInfoList inputs, PortInfoList subs);
    void setOutputs(PortInfoList outputs, PortInfoList subs);
    PortInfoList getSelectedInputPorts() const;
    PortInfoList getSelectedOutputPorts() const;

private:
    ConnectionItem* createConnectionItem(PortInfo& pi, PortInfoList& subs);
    Ui::ConnectionsClass ui;
};

#endif // CONNECTIONS_H
