/*
    Standard MIDI File creation from scratch.
    Copyright (C) 2006-2015, Pedro Lopez-Cabanillas <plcl@users.sourceforge.net>
 
    This program is free software; you can redistribute it and/or modify
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

#include "qsmf.h"

using namespace drumstick;

class QSMFBuilder : public QObject
{
    Q_OBJECT

public:
    QSMFBuilder();
    void run(QString fileName);

public slots:
    void errorHandler(const QString& errorStr);
    void trackHandler(int track);

private:
    QSmf *m_engine;
};
