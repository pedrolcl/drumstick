/*
    Drumgrid test using the MIDI Sequencer C++ library
    Copyright (C) 2006-2022, Pedro Lopez-Cabanillas <plcl@users.sf.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "drumgridabout.h"

About::About(QWidget *parent)
    : QDialog(parent)
{
    ui.setupUi(this);
    QString aboutText = ui.AboutTextView->toHtml();
    QString strver(QT_STRINGIFY(VERSION));
#ifdef REVISION
    strver.append("<br/>");
    strver.append(tr("Revision"));
    strver.append("&nbsp;");
    strver.append(QT_STRINGIFY(REVISION));
#endif
    aboutText.replace("%VERSION%", strver);
    aboutText.replace("%QT_VERSION%", qVersion());
    ui.AboutTextView->setHtml(aboutText);
}

