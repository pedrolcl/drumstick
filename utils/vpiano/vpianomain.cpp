/*
    Virtual Piano test using the MIDI Sequencer C++ library 
    Copyright (C) 2006-2011, Pedro Lopez-Cabanillas <plcl@users.sf.net>

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

#include "vpiano.h"
#include "rawkeybdapp.h"
#include "cmdlineargs.h"

#include "drumstickcommon.h"
#include <QtGui/QApplication>
#include <QtGui/QMessageBox>

const QString errorstr = "Fatal error from the ALSA sequencer. "
    "This usually happens when the kernel doesn't have ALSA support, "
    "or the device node (/dev/snd/seq) doesn't exists, "
    "or the kernel module (snd_seq) is not loaded. "
    "Please check your ALSA/MIDI configuration.";

int main(int argc, char *argv[])
{
    RawKeybdApp a(argc, argv);
    CmdLineArgs args;
    args.setStdQtArgs(true);
    args.setUsage("[options]");
    args.parse(a.argc(), a.argv());
    try {
        VPiano w;
        w.show();
        return a.exec();
    } catch (const SequencerError& ex) {
        QMessageBox::critical(0, "Error",
            errorstr + " Returned error was: " + ex.qstrError() );
    } catch (...) {
        qWarning() << errorstr;
    }
    return 0;
}
