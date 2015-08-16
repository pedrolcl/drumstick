/*
    Virtual Piano test using the MIDI Sequencer C++ library 
    Copyright (C) 2006-2015, Pedro Lopez-Cabanillas <plcl@users.sf.net>

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

#include <QApplication>
#include <QFileInfo>
#include <QSettings>
#include <QTextStream>
#include "cmdlineargs.h"
#include "vpiano.h"
#include "backendmanager.h"

int main(int argc, char *argv[])
{
    QTextStream cout(stdout, QIODevice::WriteOnly);
    QTextStream cerr(stderr, QIODevice::WriteOnly);
    QCoreApplication::setOrganizationName("drumstick.sourceforge.net");
    QCoreApplication::setOrganizationDomain("drumstick.sourceforge.net");
    QCoreApplication::setApplicationName("VPiano");
    QApplication app(argc, argv);

    CmdLineArgs args;
    args.setStdQtArgs(true);
    args.parse(argc, argv);

    QFileInfo exeInfo(app.applicationFilePath());
    cout << "program=" << exeInfo.fileName() << endl;

    QSettings settings;
    settings.beginGroup(QSTR_DRUMSTICKRT_GROUP);
    settings.setValue(QSTR_DRUMSTICKRT_PUBLICNAMEIN, QLatin1String("Virtual Piano IN"));
    settings.setValue(QSTR_DRUMSTICKRT_PUBLICNAMEOUT, QLatin1String("Virtual Piano OUT"));
    //settings.setValue(QSTR_DRUMSTICKRT_PATH, backendir);
    settings.endGroup();
    settings.sync();

    try {
        VPiano w;
        w.show();
        return app.exec();
    } catch (...) {
        cerr << "Fatal error from a MIDI backend." << endl;
    }
    return 0;
}
