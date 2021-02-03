/*
    SMF GUI Player test using the MIDI Sequencer C++ library
    Copyright (C) 2006-2021, Pedro Lopez-Cabanillas <plcl@users.sf.net>

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

#include <QApplication>
#include <QCloseEvent>
#include <QDebug>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QInputDialog>
#include <QMessageBox>
#include <QMimeData>
#include <QSettings>
#include <QStatusBar>
#include <QTextCodec>
#include <QToolTip>
#include <QUrl>
#include <qmath.h>

#include "guiplayer.h"
#include "iconutils.h"
#include "player.h"
#include "playerabout.h"
#include "song.h"
#include "ui_guiplayer.h"
#include <drumstick/alsaclient.h>
#include <drumstick/alsaevent.h>
#include <drumstick/alsaport.h>
#include <drumstick/alsaqueue.h>
#include <drumstick/qsmf.h>
#include <drumstick/qwrk.h>
#include <drumstick/sequencererror.h>

using namespace drumstick;
using namespace ALSA;
using namespace File;

const QString GUIPlayer::QSTR_DOMAIN = QStringLiteral("drumstick.sourceforge.net");
const QString GUIPlayer::QSTR_APPNAME = QStringLiteral("GUIPlayer");

GUIPlayer::GUIPlayer(QWidget *parent, Qt::WindowFlags flags)
    : QMainWindow(parent, flags),
    m_portId(-1),
    m_queueId(-1),
    m_initialTempo(0),
    m_currentTrack(0),
    m_tempoFactor(1.0),
    m_tick(0),
    m_state(InvalidState),
    m_smf(nullptr),
    m_wrk(nullptr),
    m_Client(nullptr),
    m_Port(nullptr),
    m_Queue(nullptr),
    m_player(nullptr),
    m_ui(new Ui::GUIPlayerClass),
    m_pd(nullptr),
    m_song(new Song)
{
    m_ui->setupUi(this);
    setAcceptDrops(true);
    connect(m_ui->actionAbout, &QAction::triggered, this, &GUIPlayer::about);
    connect(m_ui->actionAboutQt, &QAction::triggered, qApp, QApplication::aboutQt);
    connect(m_ui->actionPlay, &QAction::triggered, this, &GUIPlayer::play);
    connect(m_ui->actionPause, &QAction::triggered, this, &GUIPlayer::pause);
    connect(m_ui->actionStop, &QAction::triggered, this, &GUIPlayer::stop);
    connect(m_ui->actionOpen, &QAction::triggered, this, &GUIPlayer::open);
    connect(m_ui->actionMIDISetup, &QAction::triggered, this, &GUIPlayer::setup);
    connect(m_ui->actionQuit, &QAction::triggered, this, &GUIPlayer::close);
    connect(m_ui->btnTempo, &QPushButton::clicked, this, &GUIPlayer::tempoReset);
    connect(m_ui->btnVolume, &QPushButton::clicked, this, &GUIPlayer::volumeReset);
    connect(m_ui->sliderTempo, &QSlider::valueChanged, this, &GUIPlayer::tempoSlider);
    connect(m_ui->volumeSlider, &QSlider::valueChanged, this, &GUIPlayer::volumeSlider);
    connect(m_ui->spinPitch, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &GUIPlayer::pitchShift);
    connect(m_ui->toolBar->toggleViewAction(), &QAction::toggled,
            m_ui->actionShowToolbar, &QAction::setChecked);

    m_ui->actionPlay->setIcon(QIcon(IconUtils::GetPixmap(this, ":/resources/play.png")));
    m_ui->actionPlay->setShortcut( Qt::Key_MediaPlay );
    m_ui->actionStop->setIcon(QIcon(IconUtils::GetPixmap(this, ":/resources/stop.png")));
    m_ui->actionStop->setShortcut( Qt::Key_MediaStop );
    m_ui->actionPause->setIcon(QIcon(IconUtils::GetPixmap(this, ":/resources/pause.png")));
    m_ui->actionMIDISetup->setIcon(QIcon(IconUtils::GetPixmap(this, ":/resources/setup.png")));

    m_Client = new MidiClient(this);
    m_Client->open();
    m_Client->setPoolOutput(50); // small size, for near real-time pitchShift
    m_Client->setClientName("MIDI Player");
    connect( m_Client, &MidiClient::eventReceived, this, &GUIPlayer::sequencerEvent, Qt::QueuedConnection );

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
    connect(m_smf, &QSmf::signalSMFHeader, this, &GUIPlayer::smfHeaderEvent);
    connect(m_smf, &QSmf::signalSMFNoteOn, this, &GUIPlayer::smfNoteOnEvent);
    connect(m_smf, &QSmf::signalSMFNoteOff, this, &GUIPlayer::smfNoteOffEvent);
    connect(m_smf, &QSmf::signalSMFKeyPress, this, &GUIPlayer::smfKeyPressEvent);
    connect(m_smf, &QSmf::signalSMFCtlChange, this, &GUIPlayer::smfCtlChangeEvent);
    connect(m_smf, &QSmf::signalSMFPitchBend, this, &GUIPlayer::smfPitchBendEvent);
    connect(m_smf, &QSmf::signalSMFProgram, this, &GUIPlayer::smfProgramEvent);
    connect(m_smf, &QSmf::signalSMFChanPress, this, &GUIPlayer::smfChanPressEvent);
    connect(m_smf, &QSmf::signalSMFSysex, this, &GUIPlayer::smfSysexEvent);
    connect(m_smf, &QSmf::signalSMFText, this, &GUIPlayer::smfUpdateLoadProgress);
    connect(m_smf, &QSmf::signalSMFTempo, this, &GUIPlayer::smfTempoEvent);
    connect(m_smf, &QSmf::signalSMFTrackStart, this, &GUIPlayer::smfUpdateLoadProgress);
    connect(m_smf, &QSmf::signalSMFTrackStart, this, &GUIPlayer::smfTrackStarted);
    connect(m_smf, &QSmf::signalSMFTrackEnd, this, &GUIPlayer::smfTrackEnded);
    connect(m_smf, &QSmf::signalSMFendOfTrack, this, &GUIPlayer::smfUpdateLoadProgress);
    connect(m_smf, &QSmf::signalSMFError, this, &GUIPlayer::smfErrorHandler);

    m_wrk = new QWrk(this);
    connect(m_wrk, &QWrk::signalWRKError, this, &GUIPlayer::wrkErrorHandler);
    connect(m_wrk, &QWrk::signalWRKUnknownChunk, this, &GUIPlayer::wrkUpdateLoadProgress);
    connect(m_wrk, &QWrk::signalWRKHeader, this, &GUIPlayer::wrkFileHeader);
    connect(m_wrk, &QWrk::signalWRKEnd, this, &GUIPlayer::wrkEndOfFile);
    connect(m_wrk, &QWrk::signalWRKStreamEnd, this, &GUIPlayer::wrkStreamEndEvent);
    connect(m_wrk, &QWrk::signalWRKGlobalVars, this, &GUIPlayer::wrkUpdateLoadProgress);
    connect(m_wrk, &QWrk::signalWRKTrack, this, &GUIPlayer::wrkTrackHeader);
    connect(m_wrk, &QWrk::signalWRKTimeBase, this, &GUIPlayer::wrkTimeBase);
    connect(m_wrk, &QWrk::signalWRKNote, this, &GUIPlayer::wrkNoteEvent);
    connect(m_wrk, &QWrk::signalWRKKeyPress, this, &GUIPlayer::wrkKeyPressEvent);
    connect(m_wrk, &QWrk::signalWRKCtlChange, this, &GUIPlayer::wrkCtlChangeEvent);
    connect(m_wrk, &QWrk::signalWRKPitchBend, this, &GUIPlayer::wrkPitchBendEvent);
    connect(m_wrk, &QWrk::signalWRKProgram, this, &GUIPlayer::wrkProgramEvent);
    connect(m_wrk, &QWrk::signalWRKChanPress, this, &GUIPlayer::wrkChanPressEvent);
    connect(m_wrk, &QWrk::signalWRKSysexEvent, this, &GUIPlayer::wrkSysexEvent);
    connect(m_wrk, &QWrk::signalWRKSysex, this, &GUIPlayer::wrkSysexEventBank);
    connect(m_wrk, &QWrk::signalWRKText, this, &GUIPlayer::wrkUpdateLoadProgress);
    connect(m_wrk, &QWrk::signalWRKTimeSig, this, &GUIPlayer::wrkUpdateLoadProgress);
    connect(m_wrk, &QWrk::signalWRKKeySig, this, &GUIPlayer::wrkUpdateLoadProgress);
    connect(m_wrk, &QWrk::signalWRKTempo, this, &GUIPlayer::wrkTempoEvent);
    connect(m_wrk, &QWrk::signalWRKTrackPatch, this, &GUIPlayer::wrkTrackPatch);
    connect(m_wrk, &QWrk::signalWRKComments, this, &GUIPlayer::wrkUpdateLoadProgress);
    connect(m_wrk, &QWrk::signalWRKVariableRecord, this, &GUIPlayer::wrkUpdateLoadProgress);
    connect(m_wrk, &QWrk::signalWRKNewTrack, this, &GUIPlayer::wrkNewTrackHeader);
    connect(m_wrk, &QWrk::signalWRKTrackName, this, &GUIPlayer::wrkUpdateLoadProgress);
    connect(m_wrk, &QWrk::signalWRKTrackVol, this, &GUIPlayer::wrkTrackVol);
    connect(m_wrk, &QWrk::signalWRKTrackBank, this, &GUIPlayer::wrkTrackBank);
    connect(m_wrk, &QWrk::signalWRKSegment, this, &GUIPlayer::wrkUpdateLoadProgress);
    connect(m_wrk, &QWrk::signalWRKChord, this, &GUIPlayer::wrkUpdateLoadProgress);
    connect(m_wrk, &QWrk::signalWRKExpression, this, &GUIPlayer::wrkUpdateLoadProgress);

    m_player = new Player(m_Client, m_portId);
    connect(m_player, &Player::playbackStopped, this, &GUIPlayer::playerStopped, Qt::QueuedConnection);

    m_Client->setRealTimeInput(false);
    m_Client->startSequencerInput();
    tempoReset();
    volumeReset();
    updateState(EmptyState);
    readSettings();
}

GUIPlayer::~GUIPlayer()
{
    m_Client->stopSequencerInput();
    m_Port->detach();
    m_Client->close();
    delete m_player;
    delete m_ui;
    delete m_song;
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
    m_ui->lblTime->setText(stime);
}

void GUIPlayer::updateState(PlayerState newState)
{
    if (m_state == newState)
        return;
    switch (newState) {
    case EmptyState:
        m_ui->actionPlay->setEnabled(false);
        m_ui->actionPause->setEnabled(false);
        m_ui->actionStop->setEnabled(false);
        statusBar()->showMessage("Please, load a song");
        break;
    case PlayingState:
        m_ui->actionPlay->setEnabled(false);
        m_ui->actionPause->setEnabled(true);
        m_ui->actionStop->setEnabled(true);
        statusBar()->showMessage("Playing");
        break;
    case PausedState:
        m_ui->actionPlay->setEnabled(false);
        m_ui->actionStop->setEnabled(true);
        statusBar()->showMessage("Paused");
        break;
    case StoppedState:
        m_ui->actionPause->setChecked(false);
        m_ui->actionPause->setEnabled(false);
        m_ui->actionStop->setEnabled(false);
        m_ui->actionPlay->setEnabled(true);
        statusBar()->showMessage("Stopped");
        break;
    default:
        statusBar()->showMessage("Not initialized");
        break;
    }
    m_state = newState;
}

void GUIPlayer::play()
{
    if (!m_song->isEmpty()) {
        if (m_player->getInitialPosition() == 0) {
            if (m_initialTempo == 0)
                return;
            QueueTempo firstTempo = m_Queue->getTempo();
            firstTempo.setPPQ(m_song->getDivision());
            firstTempo.setTempo(m_initialTempo);
            firstTempo.setTempoFactor(m_tempoFactor);
            m_Queue->setTempo(firstTempo);
            m_Client->drainOutput();
            m_player->sendVolumeEvents();
        }
        m_player->start();
        updateState(PlayingState);
    }
}

void GUIPlayer::pause()
{
    if (m_state == PlayingState || m_player->isRunning()) {
        m_player->stop();
        m_player->setPosition(m_Queue->getStatus().getTickTime());
        updateState(PausedState);
    } else if (!m_song->isEmpty()) {
        m_player->start();
        updateState(PlayingState);
    }
}

void GUIPlayer::stop()
{
    if (m_state == PlayingState || m_state == PausedState ||
        m_player->isRunning()) {
        m_Queue->stop();
        m_Queue->clear();
        m_player->stop();
    }
    if (m_initialTempo != 0)
        songFinished();
    else
        updateState(StoppedState);
}

void GUIPlayer::progressDialogInit(const QString& type, int max)
{
    m_pd = new QProgressDialog("", "", 0, max, this);
    m_pd->setWindowTitle(QString("Loading %1 file...").arg(type));
    m_pd->setMinimumDuration(1000);
    m_pd->setValue(0);
}

void GUIPlayer::progressDialogUpdate(int pos)
{
    if (m_pd != nullptr) {
        m_pd->setValue(pos);
        qApp->processEvents();
    }
}

void GUIPlayer::progressDialogClose()
{
    delete m_pd; // set to 0 by QPointer<>
}

void GUIPlayer::openFile(const QString& fileName)
{
    QFileInfo finfo(fileName);
    if (finfo.exists()) {
        m_song->clear();
        m_loadingMessages.clear();
        m_tick = 0;
        m_initialTempo = 0;
        m_currentTrack = 0;
        try {
            QString ext = finfo.suffix().toLower();
            if (ext == "wrk") {
                progressDialogInit("Cakewalk", finfo.size());
                m_wrk->readFromFile(fileName);
            }
            else if (ext == "mid" || ext == "midi" || ext == "kar") {
                progressDialogInit("MIDI", finfo.size());
                m_smf->readFromFile(fileName);
            }
            progressDialogUpdate(finfo.size());
            if (m_song->isEmpty()) {
                m_ui->lblName->clear();
            } else {
                m_song->sort();
                m_player->setSong(m_song);
                m_ui->lblName->setText(finfo.fileName());
                m_lastDirectory = finfo.absolutePath();
            }
        } catch (...) {
            m_song->clear();
            m_ui->lblName->clear();
        }
        progressDialogClose();
        if (m_initialTempo == 0) {
            m_initialTempo = 500000;
        }
        updateTimeLabel(0,0,0);
        updateTempoLabel(6.0e7f / m_initialTempo);
        m_ui->progressBar->setValue(0);
        if (!m_loadingMessages.isEmpty()) {
            m_loadingMessages.insert(0,
                "Warning, this file may be non-standard or damaged.<br>");
            QMessageBox::warning(this, QSTR_APPNAME, m_loadingMessages);
        }
        if (m_song->isEmpty())
            updateState(EmptyState);
        else
            updateState(StoppedState);
    }
}

void GUIPlayer::open()
{
    QString fileName = QFileDialog::getOpenFileName(this,
          "Open MIDI File", m_lastDirectory,
          "All files (*.kar *.mid *.midi *.wrk);;"
          "Karaoke files (*.kar);;"
          "MIDI Files (*.mid *.midi);;"
          "Cakewalk files (*.wrk)" );
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
    if (ok && !item.isEmpty())
        subscribe(item);
}

void GUIPlayer::songFinished()
{
    m_player->resetPosition();
    updateState(StoppedState);
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
    m_ui->lblOther->setText(stempo);
}

void GUIPlayer::sequencerEvent(SequencerEvent *ev)
{
    if ((ev->getSequencerType() == SND_SEQ_EVENT_ECHO) && (m_tick != 0)){
        auto t = ev->getTick();
        int pos = 100 * t / m_tick;
        const snd_seq_real_time_t* rt = m_Queue->getStatus().getRealtime();
        int mins = rt->tv_sec / 60;
        int secs =  rt->tv_sec % 60;
        int cnts = qFloor( rt->tv_nsec / 1.0e7 );
        updateTempoLabel(m_Queue->getTempo().getRealBPM());
        updateTimeLabel(mins, secs, cnts);
        m_ui->progressBar->setValue(pos);
        if (t >= m_tick) {
            songFinished();
        }
    }
    delete ev;
}

void GUIPlayer::pitchShift(int value)
{
    m_player->setPitchShift(value);
}

void GUIPlayer::tempoReset()
{
    m_ui->sliderTempo->setValue(100);
    tempoSlider(100);
}

void GUIPlayer::volumeReset()
{
    m_ui->volumeSlider->setValue(100);
    volumeSlider(100);
}

void GUIPlayer::tempoSlider(int value)
{
    m_tempoFactor = (value*value + 100.0*value + 20000.0) / 40000.0;
    QueueTempo qtempo = m_Queue->getTempo();
    qtempo.setTempoFactor(m_tempoFactor);
    m_Queue->setTempo(qtempo);
    m_Client->drainOutput();
    if (!m_player->isRunning())
        updateTempoLabel(qtempo.getRealBPM());
    // Slider tooltip
    QString tip = QString("%1\%").arg(m_tempoFactor*100.0, 0, 'f', 0);
    m_ui->sliderTempo->setToolTip(tip);
    QToolTip::showText(QCursor::pos(), tip, this);
}

void GUIPlayer::volumeSlider(int value)
{
    QString tip = QString::number(value)+'%';
    m_ui->lblVolume->setText(tip);
    m_ui->volumeSlider->setToolTip(tip);
    m_player->setVolumeFactor(value);
    QToolTip::showText(QCursor::pos(), tip, this);
}

void GUIPlayer::dragEnterEvent( QDragEnterEvent * event )
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void GUIPlayer::dropEvent( QDropEvent * event )
{
    if ( event->mimeData()->hasUrls() ) {
        QList<QUrl> urls = event->mimeData()->urls();
        if (!urls.empty()) {
            QString fileName = urls.first().toLocalFile();
            if ( fileName.endsWith(".mid", Qt::CaseInsensitive) ||
                 fileName.endsWith(".midi", Qt::CaseInsensitive) ||
                 fileName.endsWith(".kar", Qt::CaseInsensitive) ||
                 fileName.endsWith(".wrk", Qt::CaseInsensitive) ) {
                stop();
                event->accept();
                openFile(fileName);
            } else {
                QMessageBox::warning(this, QSTR_APPNAME,
                    QString("Dropped file %1 is not supported").arg(fileName));
            }
        }
    }
}

void GUIPlayer::readSettings()
{
    QSettings settings;

    settings.beginGroup("Window");
    restoreGeometry(settings.value("Geometry").toByteArray());
    restoreState(settings.value("State").toByteArray());
    settings.endGroup();

    settings.beginGroup("Preferences");
    m_lastDirectory = settings.value("LastDirectory").toString();
    QString midiConn = settings.value("MIDIConnection").toString();
    settings.endGroup();

    if (midiConn.length() > 0)
        subscribe(midiConn);
}

void GUIPlayer::writeSettings()
{
    QSettings settings;

    settings.beginGroup("Window");
    settings.setValue("Geometry", saveGeometry());
    settings.setValue("State", saveState());
    settings.endGroup();

    settings.beginGroup("Preferences");
    settings.setValue("LastDirectory", m_lastDirectory);
    settings.setValue("MIDIConnection", m_subscription);
    settings.endGroup();
}

void GUIPlayer::closeEvent( QCloseEvent *event )
{
    stop();
    m_player->wait();
    writeSettings();
    event->accept();
}

void GUIPlayer::about()
{
    About aboutDlg(this);
    aboutDlg.exec();
}

/* **************************************** *
 * SMF (Standard MIDI file) format handling
 * **************************************** */

void GUIPlayer::smfUpdateLoadProgress()
{
    progressDialogUpdate(m_smf->getFilePos());
}

void GUIPlayer::appendSMFEvent(SequencerEvent* ev)
{
    unsigned long tick = m_smf->getCurrentTime();
    ev->setSource(m_portId);
    if (ev->getSequencerType() != SND_SEQ_EVENT_TEMPO) {
        ev->setSubscribers();
    }
    ev->scheduleTick(m_queueId, tick, false);
    m_song->append(ev);
    if (tick > m_tick)
        m_tick = tick;
    smfUpdateLoadProgress();
}

void GUIPlayer::smfHeaderEvent(int format, int ntrks, int division)
{
    m_song->setHeader(format, ntrks, division);
    smfUpdateLoadProgress();
}

void GUIPlayer::smfNoteOnEvent(int chan, int pitch, int vol)
{
    SequencerEvent* ev = new NoteOnEvent (chan, pitch, vol);
    appendSMFEvent(ev);
}

void GUIPlayer::smfNoteOffEvent(int chan, int pitch, int vol)
{
    SequencerEvent* ev = new NoteOffEvent (chan, pitch, vol);
    appendSMFEvent(ev);
}

void GUIPlayer::smfKeyPressEvent(int chan, int pitch, int press)
{
    SequencerEvent* ev = new KeyPressEvent (chan, pitch, press);
    appendSMFEvent(ev);
}

void GUIPlayer::smfCtlChangeEvent(int chan, int ctl, int value)
{
    SequencerEvent* ev = new ControllerEvent (chan, ctl, value);
    appendSMFEvent(ev);
}

void GUIPlayer::smfPitchBendEvent(int chan, int value)
{
    SequencerEvent* ev = new PitchBendEvent (chan, value);
    appendSMFEvent(ev);
}

void GUIPlayer::smfProgramEvent(int chan, int patch)
{
    SequencerEvent* ev = new ProgramChangeEvent (chan, patch);
    appendSMFEvent(ev);
}

void GUIPlayer::smfChanPressEvent(int chan, int press)
{
    SequencerEvent* ev = new ChanPressEvent (chan, press);
    appendSMFEvent(ev);
}

void GUIPlayer::smfSysexEvent(const QByteArray& data)
{
    SequencerEvent* ev = new SysExEvent (data);
    appendSMFEvent(ev);
}

void GUIPlayer::smfTempoEvent(int tempo)
{
    if ( m_initialTempo == 0 ) {
        m_initialTempo = tempo;
    }
    SequencerEvent* ev = new TempoEvent (m_queueId, tempo);
    appendSMFEvent(ev);
}

void GUIPlayer::smfErrorHandler(const QString& errorStr)
{
    if (m_loadingMessages.length() < 1024)
        m_loadingMessages.append(QString("%1 at file offset %2<br>")
                                 .arg(errorStr).arg(m_smf->getFilePos()));
}

void GUIPlayer::smfTrackStarted()
{
    m_currentTrack++;
}

void GUIPlayer::smfTrackEnded()
{
    if (m_currentTrack == m_smf->getTracks()) {
        SequencerEvent* ev = new SystemEvent(SND_SEQ_EVENT_ECHO);
        appendSMFEvent(ev);
    }
}

/* ********************************* *
 * Cakewalk WRK file format handling
 * ********************************* */

void GUIPlayer::wrkUpdateLoadProgress()
{
    if (m_pd != nullptr)
        progressDialogUpdate(m_wrk->getFilePos());
}

void
GUIPlayer::appendWRKEvent(unsigned long ticks, SequencerEvent* ev)
{
    ev->setSource(m_portId);
    if (ev->getSequencerType() != SND_SEQ_EVENT_TEMPO) {
        ev->setSubscribers();
    }
    ev->scheduleTick(m_queueId, ticks, false);
    m_song->append(ev);
    if (ticks > m_tick)
        m_tick = ticks;
    wrkUpdateLoadProgress();
}

void GUIPlayer::wrkErrorHandler(const QString& errorStr)
{
    if (m_loadingMessages.length() < 1024)
        m_loadingMessages.append(QString("%1 at file offset %2<br>")
            .arg(errorStr).arg(m_wrk->getFilePos()));
}

void GUIPlayer::wrkFileHeader(int /*verh*/, int /*verl*/)
{
    m_song->setHeader(1, 0, 120);
    wrkUpdateLoadProgress();
}

void GUIPlayer::wrkTimeBase(int timebase)
{
    m_song->setDivision(timebase);
    wrkUpdateLoadProgress();
}

void GUIPlayer::wrkStreamEndEvent(long time)
{
    unsigned long ticks = time;
    if (ticks > m_tick)
        m_tick = ticks;
    wrkUpdateLoadProgress();
}

void GUIPlayer::wrkTrackHeader( const QString& /*name1*/,
                           const QString& /*name2*/,
                           int trackno, int channel,
                           int pitch, int velocity, int /*port*/,
                           bool /*selected*/, bool /*muted*/, bool /*loop*/ )
{
    TrackMapRec rec;
    rec.channel = channel;
    rec.pitch = pitch;
    rec.velocity = velocity;
    m_trackMap[trackno] = rec;
    wrkUpdateLoadProgress();
}

void GUIPlayer::wrkNoteEvent(int track, long time, int chan, int pitch, int vol, int dur)
{
    int channel = chan;
    TrackMapRec rec = m_trackMap[track];
    int key = pitch + rec.pitch;
    int velocity = vol + rec.velocity;
    if (rec.channel > -1)
        channel = rec.channel;
    SequencerEvent* ev = new NoteEvent(channel, key, velocity, dur);
    appendWRKEvent(time, ev);
}

void GUIPlayer::wrkKeyPressEvent(int track, long time, int chan, int pitch, int press)
{
    int channel = chan;
    TrackMapRec rec = m_trackMap[track];
    int key = pitch + rec.pitch;
    if (rec.channel > -1)
        channel = rec.channel;
    SequencerEvent* ev = new KeyPressEvent(channel, key, press);
    appendWRKEvent(time, ev);
}

void GUIPlayer::wrkCtlChangeEvent(int track, long time, int chan, int ctl, int value)
{
    int channel = chan;
    TrackMapRec rec = m_trackMap[track];
    if (rec.channel > -1)
        channel = rec.channel;
    SequencerEvent* ev = new ControllerEvent(channel, ctl, value);
    appendWRKEvent(time, ev);
}

void GUIPlayer::wrkPitchBendEvent(int track, long time, int chan, int value)
{
    int channel = chan;
    TrackMapRec rec = m_trackMap[track];
    if (rec.channel > -1)
        channel = rec.channel;
    SequencerEvent* ev = new PitchBendEvent(channel, value);
    appendWRKEvent(time, ev);
}

void GUIPlayer::wrkProgramEvent(int track, long time, int chan, int patch)
{
    int channel = chan;
    TrackMapRec rec = m_trackMap[track];
    if (rec.channel > -1)
        channel = rec.channel;
    SequencerEvent* ev = new ProgramChangeEvent(channel, patch);
    appendWRKEvent(time, ev);
}

void GUIPlayer::wrkChanPressEvent(int track, long time, int chan, int press)
{
    int channel = chan;
    TrackMapRec rec = m_trackMap[track];
    if (rec.channel > -1)
        channel = rec.channel;
    SequencerEvent* ev = new ChanPressEvent(channel, press);
    appendWRKEvent(time, ev);
}

void GUIPlayer::wrkSysexEvent(int track, long time, int bank)
{
    SysexEventRec rec;
    rec.track = track;
    rec.time = time;
    rec.bank = bank;
    m_savedSysexEvents.append(rec);
    wrkUpdateLoadProgress();
}

void GUIPlayer::wrkSysexEventBank(int bank, const QString& /*name*/,
        bool autosend, int /*port*/, const QByteArray& data)
{
    SysExEvent* ev = new SysExEvent(data);
    if (autosend)
        appendWRKEvent(0, ev->clone());
    foreach(const SysexEventRec& rec, m_savedSysexEvents) {
        if (rec.bank == bank) {
            appendWRKEvent(rec.time, ev->clone());
        }
    }
    delete ev;
    wrkUpdateLoadProgress();
}

void GUIPlayer::wrkTempoEvent(long time, int tempo)
{
    double bpm = tempo / 100.0;
    if ( m_initialTempo < 0 )
        m_initialTempo = qRound( bpm );
    SequencerEvent* ev = new TempoEvent(m_queueId, qRound ( 6e7 / bpm ) );
    appendWRKEvent(time, ev);
}

void GUIPlayer::wrkTrackPatch(int track, int patch)
{
    int channel = 0;
    TrackMapRec rec = m_trackMap[track];
    if (rec.channel > -1)
        channel = rec.channel;
    wrkProgramEvent(track, 0, channel, patch);
}

void GUIPlayer::wrkNewTrackHeader( const QString& /*name*/,
                              int trackno, int channel,
                              int pitch, int velocity, int /*port*/,
                              bool /*selected*/, bool /*muted*/, bool /*loop*/ )
{
    TrackMapRec rec;
    rec.channel = channel;
    rec.pitch = pitch;
    rec.velocity = velocity;
    m_trackMap[trackno] = rec;
    wrkUpdateLoadProgress();
}

void GUIPlayer::wrkTrackVol(int track, int vol)
{
    int channel = 0;
    int lsb, msb;
    TrackMapRec rec = m_trackMap[track];
    if (rec.channel > -1)
        channel = rec.channel;
    if (vol < 128)
        wrkCtlChangeEvent(track, 0, channel, MIDI_CTL_MSB_MAIN_VOLUME, vol);
    else {
        lsb = vol % 0x80;
        msb = vol / 0x80;
        wrkCtlChangeEvent(track, 0, channel, MIDI_CTL_LSB_MAIN_VOLUME, lsb);
        wrkCtlChangeEvent(track, 0, channel, MIDI_CTL_MSB_MAIN_VOLUME, msb);
    }
}

void GUIPlayer::wrkTrackBank(int track, int bank)
{
    // assume GM/GS bank method
    int channel = 0;
    int lsb, msb;
    TrackMapRec rec = m_trackMap[track];
    if (rec.channel > -1)
        channel = rec.channel;
    lsb = bank % 0x80;
    msb = bank / 0x80;
    wrkCtlChangeEvent(track, 0, channel, MIDI_CTL_MSB_BANK, msb);
    wrkCtlChangeEvent(track, 0, channel, MIDI_CTL_LSB_BANK, lsb);
}

void GUIPlayer::wrkEndOfFile()
{
    if (m_initialTempo < 0)
        m_initialTempo = 120;
    SequencerEvent* ev = new SystemEvent(SND_SEQ_EVENT_ECHO);
    appendWRKEvent(m_tick, ev);
}
