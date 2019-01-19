/*
    MIDI Sequencer C++ library
    Copyright (C) 2006-2019, Pedro Lopez-Cabanillas <plcl@users.sf.net>

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <QApplication>
#include <QMessageBox>
#include <QCommandLineParser>

#include "cmdversion.h"
#include "drumstickcommon.h"
#include "drumgrid.h"

const QString QSTR_APPNAME("DrumGrid");
const QString QSTR_DOMAIN("drumstick.sourceforge.net");
const QString PGM_DESCRIPTION("Drumstick Drum Grid");
const QString errorstr = "Fatal error from the ALSA sequencer. "
    "This usually happens when the kernel doesn't have ALSA support, "
    "or the device node (/dev/snd/seq) doesn't exists, "
    "or the kernel module (snd_seq) is not loaded. "
    "Please check your ALSA/MIDI configuration.";

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName(QSTR_DOMAIN);
    QCoreApplication::setOrganizationDomain(QSTR_DOMAIN);
    QCoreApplication::setApplicationName(QSTR_APPNAME);
    QCoreApplication::setApplicationVersion(PGM_VERSION);
    QApplication app(argc, argv);

    QCommandLineParser parser;
    parser.setApplicationDescription(PGM_DESCRIPTION);
    auto helpOption = parser.addHelpOption();
    auto versionOption = parser.addVersionOption();
    QCommandLineOption portOption({"p", "port"}, "MIDI Out Port.", "client:port");
    parser.addOption(portOption);
    parser.process(app);

    if (parser.isSet(versionOption) || parser.isSet(helpOption)) {
        return 0;
    }

    try {
        DrumGrid w;
        if (parser.isSet(portOption)) {
            QString port = parser.value(portOption);
            w.subscribe(port);
        }
        w.show();
        return app.exec();
    } catch (const SequencerError& ex) {
        QMessageBox::critical(nullptr, "Error",
            errorstr + " Returned error was: " + ex.qstrError() );
    } catch (...) {
        qWarning() << errorstr;
    }
    return 0;
}
