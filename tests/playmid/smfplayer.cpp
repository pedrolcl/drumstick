/*
    SMF GUI Player test using the MIDI Sequencer C++ library 
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

#include <QApplication>
#include <QFileDialog>
#include <QFileInfo>
#include <QUrl>
#include <QInputDialog>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QCloseEvent>
#include <QToolTip>
#include <QSettings>
#include <QMessageBox>
#include <cmath>

#include "smfplayer.h"

SMFPlayer::SMFPlayer(QWidget *parent)
    : QWidget(parent),
    m_portId(-1),
    m_queueId(-1),
    m_initialTempo(0),
    m_tempoFactor(1.0),
    m_tick(0)
{
	ui.setupUi(this);
    tempoReset();
    connect(ui.btnPlay, SIGNAL(clicked()), SLOT(play()));
    connect(ui.btnPause, SIGNAL(clicked()), SLOT(pause()));
    connect(ui.btnStop, SIGNAL(clicked()), SLOT(stop()));
    connect(ui.btnOpen, SIGNAL(clicked()), SLOT(open()));
    connect(ui.btnSetup, SIGNAL(clicked()), SLOT(setup()));
    connect(ui.btnReset, SIGNAL(clicked()), SLOT(tempoReset()));
    connect(ui.sliderTempo, SIGNAL(valueChanged(int)), SLOT(tempoSlider(int)));
	connect(ui.btnExit, SIGNAL(clicked()), SLOT(quit()));

    m_Client = new MidiClient(this);
    m_Port = new MidiPort(this);

    m_Client->setOpenMode(SND_SEQ_OPEN_DUPLEX);
    m_Client->setBlockMode(false);
    m_Client->open();
    m_Client->setPoolOutput(100);
    m_Client->setClientName("MIDI Player");
    connect(m_Client, SIGNAL(eventReceived(SequencerEvent*)), 
                      SLOT(sequencerEvent(SequencerEvent*)));

    m_Port->setMidiClient(m_Client);
    m_Port->setPortName("MIDI Player Output Port");
    m_Port->setCapability( SND_SEQ_PORT_CAP_READ  | 
                           SND_SEQ_PORT_CAP_SUBS_READ | 
                           SND_SEQ_PORT_CAP_WRITE );
    m_Port->setPortType( SND_SEQ_PORT_TYPE_APPLICATION |
                         SND_SEQ_PORT_TYPE_MIDI_GENERIC );
    m_Port->attach();

    m_Queue = m_Client->createQueue("SMFPlayer");
    m_queueId = m_Queue->getId();
    m_portId = m_Port->getPortId();

    m_engine = new QSmf(this);
    connect(m_engine, SIGNAL(signalSMFHeader(int,int,int)), SLOT(headerEvent(int,int,int)));
    connect(m_engine, SIGNAL(signalSMFNoteOn(int,int,int)), SLOT(noteOnEvent(int,int,int)));
    connect(m_engine, SIGNAL(signalSMFNoteOff(int,int,int)), SLOT(noteOffEvent(int,int,int)));
    connect(m_engine, SIGNAL(signalSMFKeyPress(int,int,int)), SLOT(keyPressEvent(int,int,int)));
    connect(m_engine, SIGNAL(signalSMFCtlChange(int,int,int)), SLOT(ctlChangeEvent(int,int,int)));
    connect(m_engine, SIGNAL(signalSMFPitchBend(int,int)), SLOT(pitchBendEvent(int,int)));
    connect(m_engine, SIGNAL(signalSMFProgram(int,int)), SLOT(programEvent(int,int)));
    connect(m_engine, SIGNAL(signalSMFChanPress(int,int)), SLOT(chanPressEvent(int,int)));
    connect(m_engine, SIGNAL(signalSMFSysex(const QByteArray&)), SLOT(sysexEvent(const QByteArray&)));
    connect(m_engine, SIGNAL(signalSMFText(int,const QString&)), SLOT(textEvent(int,const QString&)));
    connect(m_engine, SIGNAL(signalSMFTempo(int)), SLOT(tempoEvent(int)));
    connect(m_engine, SIGNAL(signalSMFTrackStart()), SLOT(updateLoadProgress()));
    connect(m_engine, SIGNAL(signalSMFTrackEnd()), SLOT(updateLoadProgress()));
    connect(m_engine, SIGNAL(signalSMFendOfTrack()), SLOT(updateLoadProgress()));
    connect(m_engine, SIGNAL(signalSMFError(const QString&)), SLOT(errorHandler(const QString&)));

    m_player = new Player(m_Client, m_portId);
    connect(m_player, SIGNAL(finished()), SLOT(songFinished()));

    m_Client->startSequencerInput();
}

SMFPlayer::~SMFPlayer()
{
    m_Client->stopSequencerInput();
    m_Port->detach();
    m_Client->close();
    delete m_player;
}

void SMFPlayer::subscribe(const QString& portName)
{
    try {
        if (!m_subscription.isEmpty()) {
            m_Port->unsubscribeTo(m_subscription);
        }
        m_subscription = portName;
        m_Port->subscribeTo(m_subscription);
    } catch (SequencerError& err) {
        qDebug() << "SequencerError exception. Error code: " << err.code() 
                 << " (" << err.qstrError() << ")";
        qDebug() << "Location: " << err.location();
    }
}

void SMFPlayer::updateTimeLabel(int mins, int secs, int cnts)
{
    static QChar fill('0');
    QString stime = QString("%1:%2.%3").arg(mins,2,10,fill)
                                       .arg(secs,2,10,fill)
                                       .arg(cnts,2,10,fill);
    ui.lblTime->setText(stime);
}

void SMFPlayer::play()
{
    if (!m_song.isEmpty()) {
        if (m_player->getInitialPosition() == 0) {
            if (m_initialTempo == 0) { 
                return;
            }
            QueueTempo firstTempo = m_Queue->getTempo();
            firstTempo.setPPQ(m_song.getDivision());
            firstTempo.setTempo(m_initialTempo);
            firstTempo.setTempoFactor(m_tempoFactor);
            m_Queue->setTempo(firstTempo);
            m_Client->drainOutput();
        }
        m_player->start();
    }
}

void SMFPlayer::pause()
{
    if (m_player->isRunning()) {
        m_player->stop();
        m_player->setPosition(m_Queue->getStatus().getTickTime());
    }
}

void SMFPlayer::stop()
{
    if (m_player->isRunning() && (m_initialTempo != 0)) {
        m_player->stop();
        songFinished();
    }
}

void SMFPlayer::openFile(const QString& fileName)
{
    QFileInfo finfo(fileName);
    if (finfo.exists()) {
        m_song.clear();
        m_tick = 0;
        m_initialTempo = 0;
        try {
            m_pd = new QProgressDialog(0, 0, 0, finfo.size(), this);
            m_pd->setWindowTitle("Loading MIDI file...");
            m_pd->setMinimumDuration(1000);
            m_pd->setValue(0);
            m_engine->readFromFile(fileName);
            m_pd->setValue(finfo.size());
            if (m_song.isEmpty()) {
                ui.lblName->clear();
                ui.lblCopyright->clear();
            } else {
                m_song.sort();
                m_player->setSong(&m_song);
                ui.lblName->setText(fileName);
                ui.lblCopyright->setText(m_song.getCopyright());
                m_lastDirectory = finfo.absolutePath();
            }
        } catch (...) {
            m_song.clear();
            ui.lblName->clear();
            ui.lblCopyright->clear();
        }
        delete m_pd;
        if (m_initialTempo == 0) {
            m_initialTempo = 500000;
        }
        updateTimeLabel(0,0,0);
        updateTempoLabel(6.0e7f / m_initialTempo);
        ui.progressBar->setValue(0);
    }
}

void SMFPlayer::open()
{
    QFileDialog dlg(this, "Open MIDI File", m_lastDirectory, 
        "MIDI Files (*.mid *.midi);;Karaoke files (*.kar);;All files (*.*)");
    if (dlg.exec())
    {
        stop();
        QStringList fileNames = dlg.selectedFiles();
        openFile(fileNames.takeFirst());
    }
}

void SMFPlayer::setup()
{
    bool ok;
    int current;
    QStringList items;
    QListIterator<PortInfo> it(m_Client->getAvailableOutputs());
    while(it.hasNext()) {
        PortInfo p = it.next();
        items << QString("%1:%2").arg(p.getClientName()).arg(p.getPort());
    }
    current = items.indexOf(m_subscription);
    QString item = QInputDialog::getItem(this, "Player subscription", 
                                         "Output port:", items, 
                                         current, false, &ok);
    if (ok && !item.isEmpty()) {
        subscribe(item);
    }
}

void SMFPlayer::songFinished()
{
    m_player->resetPosition();
    ui.btnStop->setChecked(true);
}

void SMFPlayer::updateTempoLabel(float ftempo)
{
    QString stempo = QString("%1 bpm").arg(ftempo, 0, 'f', 2);
    ui.lblOther->setText(stempo);
}

void SMFPlayer::sequencerEvent(SequencerEvent *ev)
{
    if ((ev->getSequencerType() == SND_SEQ_EVENT_ECHO) && (m_tick != 0)){
        int pos = 100 * ev->getTick() / m_tick;
        const snd_seq_real_time_t* rt = m_Queue->getStatus().getRealtime();
        int mins = rt->tv_sec / 60; 
        int secs =  rt->tv_sec % 60;
        int cnts = floor( rt->tv_nsec / 1.0e7 );
        updateTempoLabel(m_Queue->getTempo().getRealBPM());
        updateTimeLabel(mins, secs, cnts);
        ui.progressBar->setValue(pos);
    }
    delete ev;
}

void SMFPlayer::appendEvent(SequencerEvent* ev)
{
    unsigned long tick = m_engine->getCurrentTime();
    ev->setSource(m_portId);
    if (ev->getSequencerType() != SND_SEQ_EVENT_TEMPO) {
        ev->setSubscribers();
    }
    ev->scheduleTick(m_queueId, tick, false);
    m_song.append(ev);
    if (tick > m_tick) m_tick = tick;
    updateLoadProgress();
}

void SMFPlayer::headerEvent(int format, int ntrks, int division)
{
    m_song.setHeader(format, ntrks, division);
    updateLoadProgress();
}

void SMFPlayer::noteOnEvent(int chan, int pitch, int vol)
{
    SequencerEvent* ev = new NoteOnEvent (chan, pitch, vol);
    appendEvent(ev);
}

void SMFPlayer::noteOffEvent(int chan, int pitch, int vol)
{
    SequencerEvent* ev = new NoteOffEvent (chan, pitch, vol);
    appendEvent(ev);
}

void SMFPlayer::keyPressEvent(int chan, int pitch, int press)
{
    SequencerEvent* ev = new KeyPressEvent (chan, pitch, press);
    appendEvent(ev);
}

void SMFPlayer::ctlChangeEvent(int chan, int ctl, int value)
{
    SequencerEvent* ev = new ControllerEvent (chan, ctl, value);
    appendEvent(ev);
}

void SMFPlayer::pitchBendEvent(int chan, int value)
{
    SequencerEvent* ev = new PitchBendEvent (chan, value);
    appendEvent(ev);
}

void SMFPlayer::programEvent(int chan, int patch)
{
    SequencerEvent* ev = new ProgramChangeEvent (chan, patch);
    appendEvent(ev);
}

void SMFPlayer::chanPressEvent(int chan, int press)
{
    SequencerEvent* ev = new ChanPressEvent (chan, press);
    appendEvent(ev);
}

void SMFPlayer::sysexEvent(const QByteArray& data)
{
    SequencerEvent* ev = new SysExEvent (data);
    appendEvent(ev);
}

void SMFPlayer::textEvent(int type, const QString& data)
{
    m_song.addText(type, data);
    updateLoadProgress();
}

void SMFPlayer::tempoEvent(int tempo)
{
    if ( m_initialTempo == 0 )
    {
        m_initialTempo = tempo;
    }
    SequencerEvent* ev = new TempoEvent (m_queueId, tempo);
    appendEvent(ev);
}

void SMFPlayer::errorHandler(const QString& errorStr)
{
    QMessageBox::warning(this, QSTR_APPNAME, errorStr);
    m_song.clear();
}

void SMFPlayer::tempoReset()
{
    ui.sliderTempo->setValue(100);
    ui.sliderTempo->setToolTip("100%");
    m_tempoFactor = 1.0;
}

void SMFPlayer::tempoSlider(int value)
{
    m_tempoFactor = value / 100.0;
    QueueTempo qtempo = m_Queue->getTempo();
    qtempo.setTempoFactor(m_tempoFactor);
    m_Queue->setTempo(qtempo);
    m_Client->drainOutput();
    if (!m_player->isRunning()) {
        updateTempoLabel(qtempo.getRealBPM());
    }
    // Slider tooltip
    QString tip = QString("%1\%").arg(value);
    ui.sliderTempo->setToolTip(tip);
    QToolTip::showText(QCursor::pos(), tip, this);    
}

void SMFPlayer::quit()
{
    stop();
    m_player->wait();
    close();
}

void SMFPlayer::dragEnterEvent( QDragEnterEvent * event )
{
    if (event->mimeData()->hasFormat("text/uri-list")) {
        event->acceptProposedAction();
    }
}

void SMFPlayer::dropEvent( QDropEvent * event )
{
    QString data = event->mimeData()->text();
    QString fileName = QUrl(data).path().trimmed();
    QString lname = fileName.toLower(); 
    if ( lname.endsWith(".mid") || lname.endsWith(".midi") || 
         lname.endsWith(".kar") ) {
        stop();
        openFile(fileName);
        event->accept();
    } else {
        qDebug() << "Dropped object is not supported: " << data << endl;
    }
}

bool SMFPlayer::event( QEvent * event )
{
    if(event->type() == QEvent::Polish) {
        readSettings();
        /* Process the command line arguments.
           The first argument should be a MIDI file name */  
        QStringList args = QCoreApplication::arguments();
        if (args.size() > 1) {
            QString first = args.at(1);
            openFile(first);
        }
        event->accept();
    }
    return QWidget::event(event);
}

void SMFPlayer::readSettings()
{
    QSettings settings;
    
    settings.beginGroup("Window");
    restoreGeometry(settings.value("Geometry").toByteArray());
    settings.endGroup();
    
    settings.beginGroup("Preferences");
    m_lastDirectory = settings.value("LastDirectory").toString();
    QString midiConn = settings.value("MIDIConnection").toString();
    settings.endGroup();
    
    if (midiConn.length() > 0) {
        subscribe(midiConn);
    }
}

void SMFPlayer::writeSettings()
{
    QSettings settings;

    settings.beginGroup("Window");
    settings.setValue("Geometry", saveGeometry());
    settings.endGroup();

    settings.beginGroup("Preferences");
    settings.setValue("LastDirectory", m_lastDirectory);
    settings.setValue("MIDIConnection", m_subscription);
    settings.endGroup();
}

void SMFPlayer::closeEvent( QCloseEvent *event )
{
    writeSettings();
    event->accept();
}

void SMFPlayer::updateLoadProgress()
{
    if (m_pd != NULL) {
        m_pd->setValue(m_engine->getFilePos());
    }
}
