/*
    SMF GUI Player test using the MIDI Sequencer C++ library
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

#include "guiplayer.h"
#include "ui_guiplayer.h"
#include "playerabout.h"
#include "player.h"
#include "song.h"

#include "qsmf.h"
#include "qwrk.h"
#include "qove.h"
#include "alsaevent.h"
#include "alsaclient.h"
#include "alsaqueue.h"
#include "alsaport.h"

#include <QApplication>
#include <QFileDialog>
#include <QInputDialog>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QCloseEvent>
#include <QToolTip>
#include <QMessageBox>
#include <QStatusBar>
#include <QSettings>
#include <QUrl>
#include <QFileInfo>
#include <QTextCodec>
#include <QMimeData>
#include <qmath.h>

GUIPlayer::GUIPlayer(QWidget *parent, Qt::WindowFlags flags)
    : QMainWindow(parent, flags),
    m_portId(-1),
    m_queueId(-1),
    m_initialTempo(0),
    m_tempoFactor(1.0),
    m_tick(0),
    m_state(InvalidState),
    m_smf(0),
    m_wrk(0),
    m_ove(0),
    m_Client(0),
    m_Port(0),
    m_Queue(0),
    m_player(0),
    m_ui(new Ui::GUIPlayerClass),
    m_pd(0),
    m_aboutDlg(0),
    m_song(new Song)
{
	m_ui->setupUi(this);
	setAcceptDrops(true);
    connect(m_ui->actionAbout, SIGNAL(triggered()), SLOT(about()));
    connect(m_ui->actionAboutQt, SIGNAL(triggered()), SLOT(aboutQt()));
    connect(m_ui->actionPlay, SIGNAL(triggered()), SLOT(play()));
    connect(m_ui->actionPause, SIGNAL(triggered()), SLOT(pause()));
    connect(m_ui->actionStop, SIGNAL(triggered()), SLOT(stop()));
    connect(m_ui->actionOpen, SIGNAL(triggered()), SLOT(open()));
    connect(m_ui->actionMIDISetup, SIGNAL(triggered()), SLOT(setup()));
    connect(m_ui->actionQuit, SIGNAL(triggered()), SLOT(quit()));
    connect(m_ui->btnTempo, SIGNAL(clicked()), SLOT(tempoReset()));
    connect(m_ui->btnVolume, SIGNAL(clicked()), SLOT(volumeReset()));
    connect(m_ui->sliderTempo, SIGNAL(valueChanged(int)), SLOT(tempoSlider(int)));
	connect(m_ui->volumeSlider, SIGNAL(valueChanged(int)), SLOT(volumeSlider(int)));
	connect(m_ui->spinPitch, SIGNAL(valueChanged(int)), SLOT(pitchShift(int)));
    connect(m_ui->toolBar->toggleViewAction(), SIGNAL(toggled(bool)),
            m_ui->actionShowToolbar, SLOT(setChecked(bool)));

    m_ui->actionPlay->setShortcut( Qt::Key_MediaPlay );
    m_ui->actionStop->setShortcut( Qt::Key_MediaStop );

    m_Client = new MidiClient(this);
    m_Client->open();
    m_Client->setPoolOutput(50); // small size, for near real-time pitchShift
    m_Client->setClientName("MIDI Player");
    connect( m_Client, SIGNAL(eventReceived(SequencerEvent*)),
             SLOT(sequencerEvent(SequencerEvent*)), Qt::QueuedConnection );

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
                   SLOT(smfHeaderEvent(int,int,int)));
    connect(m_smf, SIGNAL(signalSMFNoteOn(int,int,int)),
                   SLOT(smfNoteOnEvent(int,int,int)));
    connect(m_smf, SIGNAL(signalSMFNoteOff(int,int,int)),
                   SLOT(smfNoteOffEvent(int,int,int)));
    connect(m_smf, SIGNAL(signalSMFKeyPress(int,int,int)),
                   SLOT(smfKeyPressEvent(int,int,int)));
    connect(m_smf, SIGNAL(signalSMFCtlChange(int,int,int)),
                   SLOT(smfCtlChangeEvent(int,int,int)));
    connect(m_smf, SIGNAL(signalSMFPitchBend(int,int)),
                   SLOT(smfPitchBendEvent(int,int)));
    connect(m_smf, SIGNAL(signalSMFProgram(int,int)),
                   SLOT(smfProgramEvent(int,int)));
    connect(m_smf, SIGNAL(signalSMFChanPress(int,int)),
                   SLOT(smfChanPressEvent(int,int)));
    connect(m_smf, SIGNAL(signalSMFSysex(const QByteArray&)),
                   SLOT(smfSysexEvent(const QByteArray&)));
    connect(m_smf, SIGNAL(signalSMFText(int,const QString&)),
                   SLOT(smfUpdateLoadProgress()));
    connect(m_smf, SIGNAL(signalSMFTempo(int)),
                   SLOT(smfTempoEvent(int)));
    connect(m_smf, SIGNAL(signalSMFTrackStart()),
                   SLOT(smfUpdateLoadProgress()));
    connect(m_smf, SIGNAL(signalSMFTrackEnd()),
                   SLOT(smfUpdateLoadProgress()));
    connect(m_smf, SIGNAL(signalSMFendOfTrack()),
                   SLOT(smfUpdateLoadProgress()));
    connect(m_smf, SIGNAL(signalSMFError(const QString&)),
                   SLOT(smfErrorHandler(const QString&)));

    m_wrk = new QWrk(this);
    connect(m_wrk, SIGNAL(signalWRKError(const QString&)),
                   SLOT(wrkErrorHandler(const QString&)));
    connect(m_wrk, SIGNAL(signalWRKUnknownChunk(int,const QByteArray&)),
                   SLOT(wrkUpdateLoadProgress()));
    connect(m_wrk, SIGNAL(signalWRKHeader(int,int)),
                   SLOT(wrkFileHeader(int,int)));
    connect(m_wrk, SIGNAL(signalWRKEnd()),
                   SLOT(wrkEndOfFile()));
    connect(m_wrk, SIGNAL(signalWRKStreamEnd(long)),
                   SLOT(wrkStreamEndEvent(long)));
    connect(m_wrk, SIGNAL(signalWRKGlobalVars()),
                   SLOT(wrkUpdateLoadProgress()));
    connect(m_wrk, SIGNAL(signalWRKTrack(const QString&, const QString&, int,int,int,int,int,bool,bool,bool)),
                   SLOT(wrkTrackHeader(const QString&, const QString&, int,int,int,int,int,bool,bool,bool)));
    connect(m_wrk, SIGNAL(signalWRKTimeBase(int)),
                   SLOT(wrkTimeBase(int)));
    connect(m_wrk, SIGNAL(signalWRKNote(int,long,int,int,int,int)),
                   SLOT(wrkNoteEvent(int,long,int,int,int,int)));
    connect(m_wrk, SIGNAL(signalWRKKeyPress(int,long,int,int,int)),
                   SLOT(wrkKeyPressEvent(int,long,int,int,int)));
    connect(m_wrk, SIGNAL(signalWRKCtlChange(int,long,int,int,int)),
                   SLOT(wrkCtlChangeEvent(int,long,int,int,int)));
    connect(m_wrk, SIGNAL(signalWRKPitchBend(int,long,int,int)),
                   SLOT(wrkPitchBendEvent(int,long,int,int)));
    connect(m_wrk, SIGNAL(signalWRKProgram(int,long,int,int)),
                   SLOT(wrkProgramEvent(int,long,int,int)));
    connect(m_wrk, SIGNAL(signalWRKChanPress(int,long,int,int)),
                   SLOT(wrkChanPressEvent(int,long,int,int)));
    connect(m_wrk, SIGNAL(signalWRKSysexEvent(int,long,int)),
                   SLOT(wrkSysexEvent(int,long,int)));
    connect(m_wrk, SIGNAL(signalWRKSysex(int,const QString&,bool,int,const QByteArray&)),
                   SLOT(wrkSysexEventBank(int,const QString&,bool,int,const QByteArray&)));
    connect(m_wrk, SIGNAL(signalWRKText(int,long,int,const QString&)),
                   SLOT(wrkUpdateLoadProgress()));
    connect(m_wrk, SIGNAL(signalWRKTimeSig(int,int,int)),
                   SLOT(wrkUpdateLoadProgress()));
    connect(m_wrk, SIGNAL(signalWRKKeySig(int,int)),
                   SLOT(wrkUpdateLoadProgress()));
    connect(m_wrk, SIGNAL(signalWRKTempo(long,int)),
                   SLOT(wrkTempoEvent(long,int)));
    connect(m_wrk, SIGNAL(signalWRKTrackPatch(int,int)),
                   SLOT(wrkTrackPatch(int,int)));
    connect(m_wrk, SIGNAL(signalWRKComments(const QString&)),
                   SLOT(wrkUpdateLoadProgress()));
    connect(m_wrk, SIGNAL(signalWRKVariableRecord(const QString&,const QByteArray&)),
                   SLOT(wrkUpdateLoadProgress()));
    connect(m_wrk, SIGNAL(signalWRKNewTrack(const QString&,int,int,int,int,int,bool,bool,bool)),
                   SLOT(wrkNewTrackHeader(const QString&,int,int,int,int,int,bool,bool,bool)));
    connect(m_wrk, SIGNAL(signalWRKTrackName(int,const QString&)),
                   SLOT(wrkUpdateLoadProgress()));
    connect(m_wrk, SIGNAL(signalWRKTrackVol(int,int)),
                   SLOT(wrkTrackVol(int,int)));
    connect(m_wrk, SIGNAL(signalWRKTrackBank(int,int)),
                   SLOT(wrkTrackBank(int,int)));
    connect(m_wrk, SIGNAL(signalWRKSegment(int,long,const QString&)),
                   SLOT(wrkUpdateLoadProgress()));
    connect(m_wrk, SIGNAL(signalWRKChord(int,long,const QString&,const QByteArray&)),
                   SLOT(wrkUpdateLoadProgress()));
    connect(m_wrk, SIGNAL(signalWRKExpression(int,long,int,const QString&)),
                   SLOT(wrkUpdateLoadProgress()));

    m_ove = new QOve(this);
    connect(m_ove, SIGNAL(signalOVEError(const QString&)),
                   SLOT(oveErrorHandler(const QString&)));
    connect(m_ove, SIGNAL(signalOVEHeader(int,int)),
                   SLOT(oveFileHeader(int,int)));
    connect(m_ove, SIGNAL(signalOVEEnd()),
                   SLOT(wrkEndOfFile()));
    connect(m_ove, SIGNAL(signalOVENoteOn(int, long, int, int, int)),
				   SLOT(oveNoteOnEvent(int, long, int, int, int)));
    connect(m_ove, SIGNAL(signalOVENoteOff(int, long, int, int, int)),
                   SLOT(oveNoteOffEvent(int, long, int, int, int)));
    connect(m_ove, SIGNAL(signalOVEKeyPress(int,long,int,int,int)),
                   SLOT(wrkKeyPressEvent(int,long,int,int,int)));
    connect(m_ove, SIGNAL(signalOVECtlChange(int,long,int,int,int)),
                   SLOT(wrkCtlChangeEvent(int,long,int,int,int)));
    connect(m_ove, SIGNAL(signalOVEPitchBend(int,long,int,int)),
                   SLOT(wrkPitchBendEvent(int,long,int,int)));
    connect(m_ove, SIGNAL(signalOVEProgram(int,long,int,int)),
                   SLOT(wrkProgramEvent(int,long,int,int)));
    connect(m_ove, SIGNAL(signalOVEChanPress(int,long,int,int)),
                   SLOT(wrkChanPressEvent(int,long,int,int)));
    connect(m_ove, SIGNAL(signalOVESysexEvent(int,long,int)),
                   SLOT(wrkSysexEvent(int,long,int)));
    connect(m_ove, SIGNAL(signalOVESysex(int,const QString&,bool,int,const QByteArray&)),
                   SLOT(wrkSysexEventBank(int,const QString&,bool,int,const QByteArray&)));
    connect(m_ove, SIGNAL(signalOVETempo(long,int)),
                   SLOT(wrkTempoEvent(long,int)));
    connect(m_ove, SIGNAL(signalOVETrackPatch(int,int,int)),
                   SLOT(oveTrackPatch(int,int,int)));
    connect(m_ove, SIGNAL(signalOVENewTrack(const QString&,int,int,int,int,int,bool,bool,bool)),
                   SLOT(wrkNewTrackHeader(const QString&,int,int,int,int,int,bool,bool,bool)));
    connect(m_ove, SIGNAL(signalOVETrackVol(int,int,int)),
                   SLOT(wrkTrackVol(int,int)));
    connect(m_ove, SIGNAL(signalOVETrackBank(int,int,int)),
                   SLOT(oveTrackBank(int,int,int)));

    m_player = new Player(m_Client, m_portId);
    connect(m_player, SIGNAL(finished()), SLOT(songFinished()));
    connect(m_player, SIGNAL(stopped()), SLOT(playerStopped()));

    m_Client->setRealTimeInput(false);
    m_Client->startSequencerInput();
    tempoReset();
    volumeReset();
    updateState(EmptyState);
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
        m_player->isRunning())
        m_player->stop();
    if (m_initialTempo != 0)
        songFinished();
    else
        updateState(StoppedState);
}

void GUIPlayer::progressDialogInit(const QString& type, int max)
{
    m_pd = new QProgressDialog(0, 0, 0, max, this);
    m_pd->setWindowTitle(QString("Loading %1 file...").arg(type));
    m_pd->setMinimumDuration(1000);
    m_pd->setValue(0);
}

void GUIPlayer::progressDialogUpdate(int pos)
{
    if (m_pd != 0) {
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
            else if (ext == "ove") {
            	m_ove->readFromFile(fileName);
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
          "All files (*.kar *.mid *.midi *.ove *.wrk);;"
          "Karaoke files (*.kar);;"
          "MIDI Files (*.mid *.midi);;"
          "Overture Files (*.ove);;"
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
        int pos = 100 * ev->getTick() / m_tick;
        const snd_seq_real_time_t* rt = m_Queue->getStatus().getRealtime();
        int mins = rt->tv_sec / 60;
        int secs =  rt->tv_sec % 60;
        int cnts = qFloor( rt->tv_nsec / 1.0e7 );
        updateTempoLabel(m_Queue->getTempo().getRealBPM());
        updateTimeLabel(mins, secs, cnts);
        m_ui->progressBar->setValue(pos);
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
    if (event->mimeData()->hasFormat("text/uri-list"))
        event->acceptProposedAction();
}

void GUIPlayer::dropEvent( QDropEvent * event )
{
    QString data = event->mimeData()->text();
    QString fileName = QUrl(data).path().trimmed();
    while (fileName.endsWith(QChar::Null)) fileName.chop(1);
    if ( fileName.endsWith(".ove", Qt::CaseInsensitive) ||
    	 fileName.endsWith(".mid", Qt::CaseInsensitive) ||
         fileName.endsWith(".midi", Qt::CaseInsensitive) ||
         fileName.endsWith(".kar", Qt::CaseInsensitive) ||
         fileName.endsWith(".wrk", Qt::CaseInsensitive) ) {
        stop();
        openFile(fileName);
        event->accept();
    } else {
        QMessageBox::warning(this, QSTR_APPNAME,
            QString("Dropped file %1 is not supported").arg(fileName));
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
    return QMainWindow::event(event);
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
    writeSettings();
    event->accept();
}

void GUIPlayer::about()
{
    if (m_aboutDlg == 0)
        m_aboutDlg = new About(this);
    m_aboutDlg->exec();
}

void GUIPlayer::aboutQt()
{
    qApp->aboutQt();
}

void GUIPlayer::quit()
{
    stop();
    m_player->wait();
    close();
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

/* ********************************* *
 * Cakewalk WRK file format handling
 * ********************************* */

void GUIPlayer::wrkUpdateLoadProgress()
{
    if (m_pd != 0)
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

/* ********************************* *
 * Overture OVE file format handling
 * ********************************* */

void
GUIPlayer::appendOVEEvent(unsigned long ticks, SequencerEvent* ev)
{
    ev->setSource(m_portId);
    if (ev->getSequencerType() != SND_SEQ_EVENT_TEMPO)
        ev->setSubscribers();
    ev->scheduleTick(m_queueId, ticks, false);
    m_song->append(ev);
    if (ticks > m_tick)
        m_tick = ticks;
}

void GUIPlayer::oveErrorHandler(const QString& errorStr)
{
    if (m_loadingMessages.length() < 1024)
        m_loadingMessages.append(errorStr);
}

void GUIPlayer::oveFileHeader(int quarter, int trackCount)
{
	m_song->setHeader(1, trackCount, quarter);
}

void GUIPlayer::oveNoteOnEvent(int /*track*/, long tick, int channel, int pitch, int vol)
{
    SequencerEvent* ev = new NoteOnEvent(channel, pitch, vol);
    appendOVEEvent(tick, ev);
}

void GUIPlayer::oveNoteOffEvent(int /*track*/, long tick, int channel, int pitch, int vol)
{
    SequencerEvent* ev = new NoteOffEvent(channel, pitch, vol);
    appendOVEEvent(tick, ev);
}

void GUIPlayer::oveTrackPatch(int track, int channel, int patch)
{
    int ch = channel;
    TrackMapRec rec = m_trackMap[track];
    if (rec.channel > -1)
        ch = rec.channel;
    wrkProgramEvent(track, 0, ch, patch);
}

void GUIPlayer::oveTrackVol(int track, int channel, int vol)
{
    int ch = channel;
    int lsb, msb;
    TrackMapRec rec = m_trackMap[track];
    if (rec.channel > -1)
        ch = rec.channel;
    if (vol < 128)
        wrkCtlChangeEvent(track, 0, ch, MIDI_CTL_MSB_MAIN_VOLUME, vol);
    else {
        lsb = vol % 0x80;
        msb = vol / 0x80;
        wrkCtlChangeEvent(track, 0, ch, MIDI_CTL_LSB_MAIN_VOLUME, lsb);
        wrkCtlChangeEvent(track, 0, ch, MIDI_CTL_MSB_MAIN_VOLUME, msb);
    }
}

void GUIPlayer::oveTrackBank(int track, int channel, int bank)
{
    // assume GM/GS bank method
    int ch = channel;
    int lsb, msb;
    TrackMapRec rec = m_trackMap[track];
    if (rec.channel > -1)
        ch = rec.channel;
    lsb = bank % 0x80;
    msb = bank / 0x80;
    wrkCtlChangeEvent(track, 0, ch, MIDI_CTL_MSB_BANK, msb);
    wrkCtlChangeEvent(track, 0, ch, MIDI_CTL_LSB_BANK, lsb);
}
