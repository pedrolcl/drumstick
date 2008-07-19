/*
    Virtual Piano test using the MIDI Sequencer C++ library 
    Copyright (C) 2006-2008, Pedro Lopez-Cabanillas <plcl@users.sf.net>

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

Connections::Connections(QWidget *parent)
    : QDialog(parent)
{
    ui.setupUi(this);
}

void Connections::setInputs(PortInfoList inputs, SubscribersList subs)
{
    ui.m_listInputs->clear();
    //qDebug() << "setInputs()";
    PortInfoList::ConstIterator it;
    for( it = inputs.begin(); it != inputs.end(); ++it) {
        PortInfo pi = (*it);
        //qDebug() << "Port=" << pi.getClient() << ":" << pi.getPort();
        QListWidgetItem *itm = new QListWidgetItem(QString("%1:%2").
                                                   arg(pi.getClientName()).
                                                   arg(pi.getPort()));
        itm->setCheckState(Qt::Unchecked);
        SubscribersList::ConstIterator it;
        for( it = subs.begin(); it != subs.end(); ++it ) {
            Subscriber s = *it;
            //qDebug() << "Subscriber=" << s.getAddr()->client << ":" << s.getAddr()->port;
            if ((s.getAddr()->client == pi.getClient()) &&
                (s.getAddr()->port == pi.getPort())) {
                itm->setCheckState(Qt::Checked);
                break;
            }
        }
        ui.m_listInputs->addItem(itm);
    }
}

void Connections::setOutputs(PortInfoList outputs, SubscribersList subs)
{
    ui.m_listOutputs->clear();
    //qDebug() << "setOutputs()";
    PortInfoList::ConstIterator it;
    for( it = outputs.begin(); it != outputs.end(); ++it) {
        PortInfo pi = (*it);
        //qDebug() << "Port=" << pi.getClient() << ":" << pi.getPort();
        QListWidgetItem *itm = new QListWidgetItem(QString("%1:%2").
                                                   arg(pi.getClientName()).
                                                   arg(pi.getPort()));
        itm->setCheckState(Qt::Unchecked);
        SubscribersList::ConstIterator it;
        for( it = subs.begin(); it != subs.end(); ++it ) {
            Subscriber s = *it;
            //qDebug() << "Subscriber=" << s.getAddr()->client << ":" << s.getAddr()->port;
            if ((s.getAddr()->client == pi.getClient()) &&
                (s.getAddr()->port == pi.getPort())) {
                itm->setCheckState(Qt::Checked);
                break;
            }
        }
        ui.m_listOutputs->addItem(itm);
    }
}

QStringList Connections::getSelectedInputs() const
{
    QStringList lst;
    int row;
    for ( row = 0; row < ui.m_listInputs->count(); ++row ) {
        QListWidgetItem *itm = ui.m_listInputs->item(row);
        if (itm->checkState() == Qt::Checked) {
            lst << itm->text();
        }
    }
    return lst;
}

QStringList Connections::getSelectedOutputs() const
{
    QStringList lst;
    int row;
    for ( row = 0; row < ui.m_listOutputs->count(); ++row ) {
        QListWidgetItem *itm = ui.m_listOutputs->item(row);
        if (itm->checkState() == Qt::Checked) {
            lst << itm->text();
        }
    }
    return lst;
}
