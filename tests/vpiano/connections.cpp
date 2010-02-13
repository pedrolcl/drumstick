/*
    Virtual Piano test using the MIDI Sequencer C++ library 
    Copyright (C) 2006-2010, Pedro Lopez-Cabanillas <plcl@users.sf.net>

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

#include "connections.h"
#include "connectionitem.h"

Connections::Connections(QWidget *parent)
    : QDialog(parent)
{
    ui.setupUi(this);
}

ConnectionItem* Connections::createConnectionItem(PortInfo& pi, PortInfoList& subs)
{
    ConnectionItem *itm = new ConnectionItem(QString("%1:%2").
                                             arg(pi.getClientName()).
                                             arg(pi.getPort()));
    itm->setCheckState(Qt::Unchecked);
    itm->setPortInfo(pi);
    PortInfoList::ConstIterator it;
    for( it = subs.constBegin(); it != subs.constEnd(); ++it ) {
        PortInfo s = *it;
        if ((s.getClient() == pi.getClient()) &&
            (s.getPort() == pi.getPort())) {
            itm->setCheckState(Qt::Checked);
            break;
        }
    }
    return itm;
}

void Connections::setInputs(PortInfoList inputs, PortInfoList subs)
{
    ui.m_listInputs->clear();
    PortInfoList::Iterator it;
    for( it = inputs.begin(); it != inputs.end(); ++it) {
        ui.m_listInputs->addItem(createConnectionItem(*it, subs));
    }
}

void Connections::setOutputs(PortInfoList outputs, PortInfoList subs)
{
    ui.m_listOutputs->clear();
    PortInfoList::Iterator it;
    for( it = outputs.begin(); it != outputs.end(); ++it) {
        ui.m_listOutputs->addItem(createConnectionItem(*it, subs));
    }
}

PortInfoList Connections::getSelectedInputPorts() const
{
    PortInfoList lst;
    int row;
    for ( row = 0; row < ui.m_listInputs->count(); ++row ) {
        ConnectionItem *itm = dynamic_cast<ConnectionItem*>(ui.m_listInputs->item(row));
        if ((itm != NULL) && (itm->checkState() == Qt::Checked)) {
            lst << itm->getPortInfo();
        }
    }
    return lst;
}

PortInfoList Connections::getSelectedOutputPorts() const
{
    PortInfoList lst;
    int row;
    for ( row = 0; row < ui.m_listOutputs->count(); ++row ) {
        ConnectionItem *itm = dynamic_cast<ConnectionItem*>(ui.m_listOutputs->item(row));
        if ((itm != NULL) && (itm->checkState() == Qt::Checked)) {
            lst << itm->getPortInfo();
        }
    }
    return lst;
}
