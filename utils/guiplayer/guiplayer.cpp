/*
    SMF GUI Player test using the MIDI Sequencer C++ library
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

#include "guiplayer.h"
#include "player.h"

#include "qsmf.h"
#include "qwrk.h"
#include "alsaevent.h"
#include "alsaclient.h"
#include "alsaqueue.h"
#include "alsaport.h"

#include <QtGui/QApplication>
#include <QtGui/QFileDialog>
#include <QtGui/QInputDialog>
#include <QtGui/QDragEnterEvent>
#include <QtGui/QDropEvent>
#include <QtGui/QCloseEvent>
#include <QtGui/QToolTip>
#include <QtGui/QMessageBox>
#include <QtCore/QSettings>
#include <QtCore/QUrl>
#include <QtCore/QFileInfo>
#include <QtCore/QTextCodec>
#include <cmath>

GUIPlayer::GUIPlayer(QWidget *parent)
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
    m_Client->open();
    m_Client->setPoolOutput(100);
    m_Client->setClientName("MIDI Player");
    connect(m_Client, SIGNAL(eventReceived(SequencerEvent*)),
                      SLOT(sequencerEvent(SequencerEvent*)));

    m_Port = new MidiPort(this);
    m_Port->attach( m_Client );
    m_Port->setPortName("MIDI Player Output Port");
    m_Port->setCapability( SND_SEQ_PORT_CAP_READ |
                           SND_SEQ_PORT_CAP_SUBS_READ |
                           SND_SEQ_PORT_CAP_WRITE );
    m_Port->setPortType( SND_SEQ_PORT_TYPE_APPLICATION |
                         SND_SEQ_PORT_TYPE_MIDI_GENERIC );

    m_Queue = m_Client->createQueue(QSTR_APPNAME);
    m_queueId = m_Queue->getId();
    m_portId = m_Port->getPortId();

    m_smf = new QSmf(this);
    connect(m_smf, SIGNAL(signalSMFHeader(int,int,int)),
                   SLOT(headerEvent(int,int,int)));
    connect(m_smf, SIGNAL(signalSMFNoteOn(int,int,int)),
                   SLOT(noteOnEvent(int,int,int)));
    connect(m_smf, SIGNAL(signalSMFNoteOff(int,int,int)),
                   SLOT(noteOffEvent(int,int,int)));
    connect(m_smf, SIGNAL(signalSMFKeyPress(int,int,int)),
                   SLOT(keyPressEvent(int,int,int)));
    connect(m_smf, SIGNAL(signalSMFCtlChange(int,int,int)),
                   SLOT(ctlChangeEvent(int,int,int)));
    connect(m_smf, SIGNAL(signalSMFPitchBend(int,int)),
                   SLOT(pitchBendEvent(int,int)));
    connect(m_smf, SIGNAL(signalSMFProgram(int,int)),
                   SLOT(programEvent(int,int)));
    connect(m_smf, SIGNAL(signalSMFChanPress(int,int)),
                   SLOT(chanPressEvent(int,int)));
    connect(m_smf, SIGNAL(signalSMFSysex(const QByteArray&)),
                   SLOT(sysexEvent(const QByteArray&)));
    connect(m_smf, SIGNAL(signalSMFText(int,const QString&)),
                   SLOT(textEvent(int,const QString&)));
    connect(m_smf, SIGNAL(signalSMFTempo(int)),
                   SLOT(tempoEvent(int)));
    connect(m_smf, SIGNAL(signalSMFTrackStart()),
                   SLOT(updateSMFLoadProgress()));
    connect(m_smf, SIGNAL(signalSMFTrackEnd()),
                   SLOT(updateSMFLoadProgress()));
    connect(m_smf, SIGNAL(signalSMFendOfTrack()),
                   SLOT(updateSMFLoadProgress()));
    connect(m_smf, SIGNAL(signalSMFError(const QString&)),
                   SLOT(errorHandler(const QString&)));

    m_wrk = new QWrk(this);
    connect(m_wrk, SIGNAL(signalWRKError(const QString&)),
                   SLOT(errorHandlerWRK(const QString&)));
    connect(m_wrk, SIGNAL(signalWRKUnknownChunk(int,const QByteArray&)),
                   SLOT(unknownChunk(int,const QByteArray&)));
    connect(m_wrk, SIGNAL(signalWRKHeader(int,int)),
                   SLOT(fileHeader(int,int)));
    connect(m_wrk, SIGNAL(signalWRKEnd()),
                   SLOT(endOfWrk()));
    connect(m_wrk, SIGNAL(signalWRKStreamEnd(long)),
                   SLOT(streamEndEvent(long)));
    connect(m_wrk, SIGNAL(signalWRKGlobalVars()),
                   SLOT(globalVars()));
    connect(m_wrk, SIGNAL(signalWRKTrack(const QString&, const QString&, int,int,int,int,int,bool,bool,bool)),
                   SLOT(trackHeader(const QString&, const QString&, int,int,int,int,int,bool,bool,bool)));
    connect(m_wrk, SIGNAL(signalWRKTimeBase(int)),
                   SLOT(timeBase(int)));
    connect(m_wrk, SIGNAL(signalWRKNote(int,long,int,int,int,int)),
                   SLOT(noteEvent(int,long,int,int,int,int)));
    connect(m_wrk, SIGNAL(signalWRKKeyPress(int,long,int,int,int)),
                   SLOT(keyPressEvent(int,long,int,int,int)));
    connect(m_wrk, SIGNAL(signalWRKCtlChange(int,long,int,int,int)),
                   SLOT(ctlChangeEvent(int,long,int,int,int)));
    connect(m_wrk, SIGNAL(signalWRKPitchBend(int,long,int,int)),
                   SLOT(pitchBendEvent(int,long,int,int)));
    connect(m_wrk, SIGNAL(signalWRKProgram(int,long,int,int)),
                   SLOT(programEvent(int,long,int,int)));
    connect(m_wrk, SIGNAL(signalWRKChanPress(int,long,int,int)),
                   SLOT(chanPressEvent(int,long,int,int)));
    connect(m_wrk, SIGNAL(signalWRKSysexEvent(int,long,int)),
                   SLOT(sysexEvent(int,long,int)));
    connect(m_wrk, SIGNAL(signalWRKSysex(int,const QString&,bool,int,const QByteArray&)),
                   SLOT(sysexEventBank(int,const QString&,bool,int,const QByteArray&)));
    connect(m_wrk, SIGNAL(signalWRKText(int,long,int,const QString&)),
                   SLOT(textEvent(int,long,int,const QString&)));
    connect(m_wrk, SIGNAL(signalWRKTimeSig(int,int,int)),
                   SLOT(timeSigEvent(int,int,int)));
    connect(m_wrk, SIGNAL(signalWRKKeySig(int,int)),
                   SLOT(keySigEventWRK(int,int)));
    connect(m_wrk, SIGNAL(signalWRKTempo(long,int)),
                   SLOT(tempoEvent(long,int)));
    connect(m_wrk, SIGNAL(signalWRKTrackPatch(int,int)),
                   SLOT(trackPatch(int,int)));
    connect(m_wrk, SIGNAL(signalWRKComments(const QString&)),
                   SLOT(comments(const QString&)));
    connect(m_wrk, SIGNAL(signalWRKVariableRecord(const QString&,const QByteArray&)),
                   SLOT(variableRecord(const QString&,const QByteArray&)));
    connect(m_wrk, SIGNAL(signalWRKNewTrack(const QString&,int,int,int,int,int,bool,bool,bool)),
                   SLOT(newTrackHeader(const QString&,int,int,int,int,int,bool,bool,bool)));
    connect(m_wrk, SIGNAL(signalWRKTrackName(int,const QString&)),
                   SLOT(trackName(int,const QString&)));
    connect(m_wrk, SIGNAL(signalWRKTrackVol(int,int)),
                   SLOT(trackVol(int,int)));
    connect(m_wrk, SIGNAL(signalWRKTrackBank(int,int)),
                   SLOT(trackBank(int,int)));
    connect(m_wrk, SIGNAL(signalWRKSegment(int,long,const QString&)),
                   SLOT(segment(int,long,const QString&)));
    connect(m_wrk, SIGNAL(signalWRKChord(int,long,const QString&,const QByteArray&)),
                   SLOT(chord(int,long,const QString&,const QByteArray&)));
    connect(m_wrk, SIGNAL(signalWRKExpression(int,long,int,const QString&)),
                   SLOT(expression(int,long,int,const QString&)));

    m_player = new Player(m_Client, m_portId);
    connect(m_player, SIGNAL(finished()), SLOT(songFinished()));
    connect(m_player, SIGNAL(stopped()), SLOT(playerStopped()));

    m_Client->startSequencerInput();
}

GUIPlayer::~GUIPlayer()
{
    m_Client->stopSequencerInput();
    m_Port->detach();
    m_Client->close();
    delete m_player;
}

void GUIPlayer::subscribe(const QString& portName)
{
    try {
        if (!m_subscription.isEmpty()) {
            m_Port->unsubscribeTo(m_subscription);
        }
        m_subscription = portName;
        m_Port->subscribeTo(m_subscription);
    } catch (const SequencerError& err) {
        qWarning() << "SequencerError exception. Error code: " << err.code()
                   << " (" << err.qstrError() << ")";
        qWarning() << "Location: " << err.location();
    }
}

void GUIPlayer::updateTimeLabel(int mins, int secs, int cnts)
{
    static QChar fill('0');
    QString stime = QString("%1:%2.%3").arg(mins,2,10,fill)
                                       .arg(secs,2,10,fill)
                                       .arg(cnts,2,10,fill);
    ui.lblTime->setText(stime);
}

void GUIPlayer::play()
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

void GUIPlayer::pause()
{
    if (m_player->isRunning()) {
        m_player->stop();
        m_player->setPosition(m_Queue->getStatus().getTickTime());
    }
}

void GUIPlayer::stop()
{
    if (m_player->isRunning() && (m_initialTempo != 0)) {
        m_player->stop();
        songFinished();
    }
}

void GUIPlayer::openFile(const QString& fileName)
{
    QFileInfo finfo(fileName);
    if (finfo.exists()) {
        m_song.clear();
        m_loadingMessages.clear();
        m_tick = 0;
        m_initialTempo = 0;
        try {
            m_pd = new QProgressDialog(0, 0, 0, finfo.size(), this);
            m_pd->setWindowTitle("Loading MIDI file...");
            m_pd->setMinimumDuration(1000);
            m_pd->setValue(0);
            QString ext = finfo.suffix().toLower();
            if (ext == "wrk")
                m_wrk->readFromFile(fileName);
            else if (ext == "mid" || ext == "kar")
                m_smf->readFromFile(fileName);
            m_pd->setValue(finfo.size());
            if (m_song.isEmpty()) {
                ui.lblName->clear();
                ui.lblCopyright->clear();
            } else {
                m_song.sort();
                m_player->setSong(&m_song);
                ui.lblName->setText(finfo.fileName());
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
        if (!m_loadingMessages.isEmpty()) {
            m_loadingMessages.insert(0, "Warning, this file may be non-standard or damaged<br>");
            QMessageBox::warning(this, QSTR_APPNAME, m_loadingMessages);
        }
    }
}

void GUIPlayer::open()
{
    QString fileName = QFileDialog::getOpenFileName(this,
          "Open MIDI File", m_lastDirectory,
          "MIDI Files (*.mid *.midi);;"
          "Karaoke files (*.kar);;"
          "Cakewalk files (*.wrk);;"
          "All files (*.*)");
    if (! fileName.isEmpty() ) {
        stop();
        openFile(fileName);
    }
}

void GUIPlayer::setup()
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

void GUIPlayer::songFinished()
{
    m_player->resetPosition();
    ui.btnStop->setChecked(true);
}

void GUIPlayer::playerStopped()
{
    int portId = m_Port->getPortId();
    for (int channel = 0; channel < 16; ++channel) {
        ControllerEvent ev1(channel, MIDI_CTL_ALL_NOTES_OFF, 0);
        ev1.setSource(portId);
        ev1.setSubscribers();
        ev1.setDirect();
        m_Client->outputDirect(&ev1);
        ControllerEvent ev2(channel, MIDI_CTL_ALL_SOUNDS_OFF, 0);
        ev2.setSource(portId);
        ev2.setSubscribers();
        ev2.setDirect();
        m_Client->outputDirect(&ev2);
    }
    m_Client->drainOutput();
}

void GUIPlayer::updateTempoLabel(float ftempo)
{
    QString stempo = QString("%1 bpm").arg(ftempo, 0, 'f', 2);
    ui.lblOther->setText(stempo);
}

void GUIPlayer::sequencerEvent(SequencerEvent *ev)
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

void GUIPlayer::headerEvent(int format, int ntrks, int division)
{
    m_song.setHeader(format, ntrks, division);
    updateSMFLoadProgress();
}

void GUIPlayer::noteOnEvent(int chan, int pitch, int vol)
{
    SequencerEvent* ev = new NoteOnEvent (chan, pitch, vol);
    appendSMFEvent(ev);
}

void GUIPlayer::noteOffEvent(int chan, int pitch, int vol)
{
    SequencerEvent* ev = new NoteOffEvent (chan, pitch, vol);
    appendSMFEvent(ev);
}

void GUIPlayer::keyPressEvent(int chan, int pitch, int press)
{
    SequencerEvent* ev = new KeyPressEvent (chan, pitch, press);
    appendSMFEvent(ev);
}

void GUIPlayer::ctlChangeEvent(int chan, int ctl, int value)
{
    SequencerEvent* ev = new ControllerEvent (chan, ctl, value);
    appendSMFEvent(ev);
}

void GUIPlayer::pitchBendEvent(int chan, int value)
{
    SequencerEvent* ev = new PitchBendEvent (chan, value);
    appendSMFEvent(ev);
}

void GUIPlayer::programEvent(int chan, int patch)
{
    SequencerEvent* ev = new ProgramChangeEvent (chan, patch);
    appendSMFEvent(ev);
}

void GUIPlayer::chanPressEvent(int chan, int press)
{
    SequencerEvent* ev = new ChanPressEvent (chan, press);
    appendSMFEvent(ev);
}

void GUIPlayer::sysexEvent(const QByteArray& data)
{
    SequencerEvent* ev = new SysExEvent (data);
    appendSMFEvent(ev);
}

void GUIPlayer::textEvent(int type, const QString& data)
{
    m_song.addText(type, data);
    updateSMFLoadProgress();
}

void GUIPlayer::tempoEvent(int tempo)
{
    if ( m_initialTempo == 0 )
    {
        m_initialTempo = tempo;
    }
    SequencerEvent* ev = new TempoEvent (m_queueId, tempo);
    appendSMFEvent(ev);
}

void GUIPlayer::errorHandler(const QString& errorStr)
{
    if (m_loadingMessages.length() < 1024)
        m_loadingMessages.append(QString("%1 at file offset %2<br>")
            .arg(errorStr).arg(m_smf->getFilePos()));
}

void GUIPlayer::tempoReset()
{
    ui.sliderTempo->setValue(100);
    ui.sliderTempo->setToolTip("100%");
    m_tempoFactor = 1.0;
}

void GUIPlayer::tempoSlider(int value)
{
    m_tempoFactor = (value*value + 100.0*value + 20000.0) / 40000.0;
    QueueTempo qtempo = m_Queue->getTempo();
    qtempo.setTempoFactor(m_tempoFactor);
    m_Queue->setTempo(qtempo);
    m_Client->drainOutput();
    if (!m_player->isRunning()) {
        updateTempoLabel(qtempo.getRealBPM());
    }
    // Slider tooltip
    QString tip = QString("%1\%").arg(m_tempoFactor*100.0, 0, 'f', 0);
    ui.sliderTempo->setToolTip(tip);
    QToolTip::showText(QCursor::pos(), tip, this);
}

void GUIPlayer::quit()
{
    stop();
    m_player->wait();
    close();
}

void GUIPlayer::dragEnterEvent( QDragEnterEvent * event )
{
    if (event->mimeData()->hasFormat("text/uri-list")) {
        event->acceptProposedAction();
    }
}

void GUIPlayer::dropEvent( QDropEvent * event )
{
    QString data = event->mimeData()->text();
    QString fileName = QUrl(data).path().trimmed();
    while (fileName.endsWith(QChar::Null)) fileName.chop(1);
    if ( fileName.endsWith(".mid", Qt::CaseInsensitive) ||
         fileName.endsWith(".midi", Qt::CaseInsensitive) ||
         fileName.endsWith(".kar", Qt::CaseInsensitive) ) {
        stop();
        openFile(fileName);
        event->accept();
    } else {
        qDebug() << "Dropped object is not supported:" << fileName;
    }
}

bool GUIPlayer::event( QEvent * event )
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

void GUIPlayer::readSettings()
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

void GUIPlayer::writeSettings()
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

void GUIPlayer::closeEvent( QCloseEvent *event )
{
    writeSettings();
    event->accept();
}

void GUIPlayer::updateSMFLoadProgress()
{
    if (m_pd != NULL) {
        m_pd->setValue(m_smf->getFilePos());
    }
}

void GUIPlayer::appendSMFEvent(SequencerEvent* ev)
{
    unsigned long tick = m_smf->getCurrentTime();
    ev->setSource(m_portId);
    if (ev->getSequencerType() != SND_SEQ_EVENT_TEMPO) {
        ev->setSubscribers();
    }
    ev->scheduleTick(m_queueId, tick, false);
    m_song.append(ev);
    if (tick > m_tick)
        m_tick = tick;
    updateSMFLoadProgress();
}

/* ********************************* *
 * Cakewalk WRK file format handling
 * ********************************* */

void GUIPlayer::updateWRKLoadProgress()
{
    if (m_pd != NULL) {
        m_pd->setValue(m_wrk->getFilePos());
    }
}

void
GUIPlayer::appendWRKEvent(unsigned long ticks, int /*trck*/, SequencerEvent* ev)
{
    ev->setSource(m_portId);
    if (ev->getSequencerType() != SND_SEQ_EVENT_TEMPO) {
        ev->setSubscribers();
    }
    ev->scheduleTick(m_queueId, ticks, false);
    m_song.append(ev);
    if (ticks > m_tick)
        m_tick = ticks;
    updateWRKLoadProgress();
    qApp->processEvents();
}

void GUIPlayer::errorHandlerWRK(const QString& errorStr)
{
    qWarning() << "*** Warning! " << errorStr
               << " at file offset " << m_wrk->getFilePos();
}

void GUIPlayer::fileHeader(int /*verh*/, int /*verl*/)
{
    //fileFormat = QString("WRK file version %1.%2").arg(verh).arg(verl);
    m_song.setHeader(1, 0, 120);
}

void GUIPlayer::timeBase(int timebase)
{
    m_song.setDivision(timebase);
}

void GUIPlayer::globalVars()
{
    keySigEventWRK(0, m_wrk->getKeySig());
    updateWRKLoadProgress();
}

void GUIPlayer::streamEndEvent(long time)
{
    unsigned long ticks = time;
    if (ticks > m_tick)
        m_tick = ticks;
}

void GUIPlayer::trackHeader( const QString& name1, const QString& name2,
                           int trackno, int channel,
                           int pitch, int velocity, int /*port*/,
                           bool /*selected*/, bool /*muted*/, bool /*loop*/ )
{
    TrackMapRec rec;
    rec.channel = channel;
    rec.pitch = pitch;
    rec.velocity = velocity;
    m_trackMap[trackno] = rec;
    QString trkName = name1 + ' ' + name2;
    trkName = trkName.trimmed();
    if (!trkName.isEmpty()) {
        SequencerEvent* ev = new TextEvent(trkName, Song::TrackName);
        appendWRKEvent(0, trackno, ev);
    }
    updateWRKLoadProgress();
}

void GUIPlayer::noteEvent(int track, long time, int chan, int pitch, int vol, int dur)
{
    int channel = chan;
    TrackMapRec rec = m_trackMap[track];
    int key = pitch + rec.pitch;
    int velocity = vol + rec.velocity;
    if (rec.channel > -1)
        channel = rec.channel;
    SequencerEvent* ev = new NoteEvent(channel, key, velocity, dur);
    appendWRKEvent(time, track, ev);
}

void GUIPlayer::keyPressEvent(int track, long time, int chan, int pitch, int press)
{
    int channel = chan;
    TrackMapRec rec = m_trackMap[track];
    int key = pitch + rec.pitch;
    if (rec.channel > -1)
        channel = rec.channel;
    SequencerEvent* ev = new KeyPressEvent(channel, key, press);
    appendWRKEvent(time, track, ev);
}

void GUIPlayer::ctlChangeEvent(int track, long time, int chan, int ctl, int value)
{
    int channel = chan;
    TrackMapRec rec = m_trackMap[track];
    if (rec.channel > -1)
        channel = rec.channel;
    SequencerEvent* ev = new ControllerEvent(channel, ctl, value);
    appendWRKEvent(time, track, ev);
}

void GUIPlayer::pitchBendEvent(int track, long time, int chan, int value)
{
    int channel = chan;
    TrackMapRec rec = m_trackMap[track];
    if (rec.channel > -1)
        channel = rec.channel;
    SequencerEvent* ev = new PitchBendEvent(channel, value);
    appendWRKEvent(time, track, ev);
}

void GUIPlayer::programEvent(int track, long time, int chan, int patch)
{
    int channel = chan;
    TrackMapRec rec = m_trackMap[track];
    if (rec.channel > -1)
        channel = rec.channel;
    SequencerEvent* ev = new ProgramChangeEvent(channel, patch);
    appendWRKEvent(time, track, ev);
}

void GUIPlayer::chanPressEvent(int track, long time, int chan, int press)
{
    int channel = chan;
    TrackMapRec rec = m_trackMap[track];
    if (rec.channel > -1)
        channel = rec.channel;
    SequencerEvent* ev = new ChanPressEvent(channel, press);
    appendWRKEvent(time, track, ev);
}

void GUIPlayer::sysexEvent(int track, long time, int bank)
{
    SysexEventRec rec;
    rec.track = track;
    rec.time = time;
    rec.bank = bank;
    m_savedSysexEvents.append(rec);
}

void GUIPlayer::sysexEventBank(int bank, const QString& /*name*/, bool autosend, int /*port*/, const QByteArray& data)
{
    SysExEvent* ev = new SysExEvent(data);

    if (autosend)
        appendWRKEvent(0, 0, ev->clone());

    foreach(const SysexEventRec& rec, m_savedSysexEvents) {
        if (rec.bank == bank) {
            appendWRKEvent(rec.time, rec.track, ev->clone());
        }
    }

    delete ev;
}

void GUIPlayer::textEvent(int track, long time, int type, const QString& data)
{
    SequencerEvent* ev = new TextEvent(data, type);
    appendWRKEvent(time, track, ev);
}

void GUIPlayer::timeSigEvent(int bar, int num, int den)
{
    SequencerEvent* ev = new SequencerEvent();
    ev->setSequencerType(SND_SEQ_EVENT_TIMESIGN);
    int div, d = den;
    for ( div = 0; d > 1; d /= 2 )
        ++div;
    ev->setRaw8(0, num);
    ev->setRaw8(1, div);
    ev->setRaw8(2, 24 * 4 / den);
    ev->setRaw8(3, 8);

    TimeSigRec newts;
    newts.bar = bar;
    newts.num = num;
    newts.den = den;
    newts.time = 0;
    if (m_bars.isEmpty()) {
        m_bars.append(newts);
    } else {
        bool found = false;
        foreach(const TimeSigRec& ts, m_bars) {
            if (ts.bar == bar) {
                newts.time = ts.time;
                found = true;
                break;
            }
        }
        if (!found) {
            TimeSigRec& lasts = m_bars.last();
            newts.time = lasts.time + ( lasts.num * 4 / lasts.den
                * m_song.getDivision() * (bar - lasts.bar) );
            m_bars.append(newts);
        }
    }
    appendWRKEvent(newts.time, 0, ev);
}

void GUIPlayer::keySigEventWRK(int bar, int alt)
{
    SequencerEvent* ev = new SequencerEvent();
    ev->setSequencerType(SND_SEQ_EVENT_KEYSIGN);
    ev->setRaw8(0, alt);
    long time = 0;
    foreach(const TimeSigRec& ts, m_bars) {
        if (ts.bar == bar) {
            time = ts.time;
            break;
        }
    }
    appendWRKEvent(time, 0, ev);
}

void GUIPlayer::tempoEvent(long time, int tempo)
{
    double bpm = tempo / 100.0;
    if ( m_initialTempo < 0 )
    {
        m_initialTempo = round( bpm );
    }
    SequencerEvent* ev = new TempoEvent(m_queueId, round ( 6e7 / bpm ) );
    appendWRKEvent(time, 0, ev);
}

void GUIPlayer::trackPatch(int track, int patch)
{
    int channel = 0;
    TrackMapRec rec = m_trackMap[track];
    if (rec.channel > -1)
        channel = rec.channel;
    programEvent(track, 0, channel, patch);
}

void GUIPlayer::comments(const QString& cmt)
{
    SequencerEvent* ev = new TextEvent("Comment: " + cmt, Song::Text);
    appendWRKEvent(0, 0, ev);
}

void GUIPlayer::variableRecord(const QString& name, const QByteArray& data)
{
    SequencerEvent* ev = NULL;
    QString s;
    bool isReadable = ( name == "Title" || name == "Author" ||
                       name == "Copyright" || name == "Subtitle" ||
                       name == "Instructions" || name == "Keywords" );
    if (isReadable) {
        QByteArray b2 = data.left(qstrlen(data));
        if (m_wrk->getTextCodec() == 0)
            s = QString(b2);
        else
            s = m_wrk->getTextCodec()->toUnicode(b2);
        if ( name == "Title" )
            ev = new TextEvent(s, Song::TrackName);
        else if ( name == "Copyright" )
            ev = new TextEvent(s, Song::Copyright);
        else
            ev = new TextEvent(name + ": " + s, Song::Text);
        appendWRKEvent(0, 0, ev);
    }
}

void GUIPlayer::newTrackHeader( const QString& name,
                              int trackno, int channel,
                              int pitch, int velocity, int /*port*/,
                              bool /*selected*/, bool /*muted*/, bool /*loop*/ )
{
    TrackMapRec rec;
    rec.channel = channel;
    rec.pitch = pitch;
    rec.velocity = velocity;
    m_trackMap[trackno] = rec;
    if (!name.isEmpty())
        textEvent(trackno, 0, Song::TrackName, name);
    updateWRKLoadProgress();
}

void GUIPlayer::trackName(int trackno, const QString& name)
{
    SequencerEvent* ev = new TextEvent(name, Song::TrackName);
    appendWRKEvent(0, trackno, ev);
}

void GUIPlayer::trackVol(int track, int vol)
{
    int channel = 0;
    int lsb, msb;
    TrackMapRec rec = m_trackMap[track];
    if (rec.channel > -1)
        channel = rec.channel;
    if (vol < 128)
        ctlChangeEvent(track, 0, channel, MIDI_CTL_MSB_MAIN_VOLUME, vol);
    else {
        lsb = vol % 0x80;
        msb = vol / 0x80;
        ctlChangeEvent(track, 0, channel, MIDI_CTL_LSB_MAIN_VOLUME, lsb);
        ctlChangeEvent(track, 0, channel, MIDI_CTL_MSB_MAIN_VOLUME, msb);
    }
}

void GUIPlayer::trackBank(int track, int bank)
{
    // assume GM/GS bank method
    int channel = 0;
    int lsb, msb;
    TrackMapRec rec = m_trackMap[track];
    if (rec.channel > -1)
        channel = rec.channel;
    lsb = bank % 0x80;
    msb = bank / 0x80;
    ctlChangeEvent(track, 0, channel, MIDI_CTL_MSB_BANK, msb);
    ctlChangeEvent(track, 0, channel, MIDI_CTL_LSB_BANK, lsb);
}

void GUIPlayer::segment(int track, long time, const QString& name)
{
    if (!name.isEmpty()) {
        SequencerEvent *ev = new TextEvent("Segment: " + name, Song::Marker);
        appendWRKEvent(time, track, ev);
    }
}

void GUIPlayer::chord(int track, long time, const QString& name, const QByteArray& /*data*/ )
{
    if (!name.isEmpty()) {
        SequencerEvent *ev = new TextEvent("Chord: " + name, Song::Text);
        appendWRKEvent(time, track, ev);
    }
}

void GUIPlayer::expression(int track, long time, int /*code*/, const QString& text)
{
    if (!text.isEmpty()) {
        SequencerEvent *ev = new TextEvent(text, Song::Text);
        appendWRKEvent(time, track, ev);
    }
}

void GUIPlayer::endOfWrk()
{
    if (m_initialTempo < 0)
        m_initialTempo = 120;
}

void GUIPlayer::unknownChunk(int /*type*/, const QByteArray& /*data*/)
{
    /*qDebug() << "dec:" << type
             << "hex:" << hex << type << dec
             << "size:" << data.length();*/
}
