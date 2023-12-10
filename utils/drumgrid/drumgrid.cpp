/*
    MIDI Sequencer C++ library
    Copyright (C) 2006-2023, Pedro Lopez-Cabanillas <plcl@users.sf.net>

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <QDebug>
#include <QInputDialog>
#include <QSettings>
#include <QShortcut>
#include <QToolTip>
#include <QtMath>
#include <drumstick/alsaclient.h>
#include <drumstick/alsaevent.h>
#include <drumstick/alsaport.h>
#include <drumstick/alsaqueue.h>
#include <drumstick/sequencererror.h>
#include "drumgrid.h"
#include "drumgridabout.h"
#include "drumgridmodel.h"
#include "ui_drumgrid.h"

using namespace drumstick::ALSA;

const QString DrumGrid::QSTR_WINDOW = QStringLiteral("Window");
const QString DrumGrid::QSTR_GEOMETRY = QStringLiteral("Geometry");
const QString DrumGrid::QSTR_STATE = QStringLiteral("State");
const QString DrumGrid::QSTR_MIDI = QStringLiteral("MIDI");
const QString DrumGrid::QSTR_CONNECTION = QStringLiteral("Connection");
const QString DrumGrid::QSTR_TEMPO = QStringLiteral("Tempo");
const QString DrumGrid::QSTR_PATTERN = QStringLiteral("Pattern");

const int DrumGrid::TEMPO_MIN = 25;
const int DrumGrid::TEMPO_MAX = 250;
const int DrumGrid::TEMPO_DEFAULT = 120;
const int DrumGrid::NOTE_DURATION = 10;
const int DrumGrid::METRONOME_CHANNEL = 9;
const int DrumGrid::METRONOME_VELOCITY = 100;
const int DrumGrid::METRONOME_PROGRAM = 0;
const int DrumGrid::METRONOME_RESOLUTION = 120;
const int DrumGrid::METRONOME_VOLUME = 100;
const int DrumGrid::METRONOME_PAN = 64;
const int DrumGrid::VOLUME_CC = 7;
const int DrumGrid::PAN_CC = 10;

DrumGrid::DrumGrid(QWidget *parent)
    : QMainWindow(parent),
    m_ui(new Ui::DrumGrid),
    m_clientId(-1),
    m_portId(-1),
    m_queueId(-1),
    m_tick(0),
    m_weak_velocity(METRONOME_VELOCITY / 2),
    m_strong_velocity(METRONOME_VELOCITY),
    m_program(METRONOME_PROGRAM),
    m_channel(METRONOME_CHANNEL),
    m_volume(METRONOME_VOLUME),
    m_pan(METRONOME_PAN),
    m_resolution(METRONOME_RESOLUTION),
    m_bpm(TEMPO_DEFAULT),
    m_noteDuration(NOTE_DURATION),
    m_autoconnect(false),
    m_playing(false),
    m_useNoteOff(true)
{
    m_ui->setupUi(this);
    m_ui->startButton->setIcon(style()->standardIcon(QStyle::StandardPixmap(QStyle::SP_MediaPlay)));
    m_ui->startButton->setShortcut(Qt::Key_MediaPlay);
    m_ui->stopButton->setIcon(style()->standardIcon(QStyle::StandardPixmap(QStyle::SP_MediaStop)));
    m_ui->stopButton->setShortcut(Qt::Key_MediaStop);
    m_ui->tempoSlider->setMaximum(TEMPO_MAX);
    m_ui->tempoSlider->setMinimum(TEMPO_MIN);
    m_ui->tempoSlider->setValue(m_bpm);

    connect( m_ui->actionAbout, &QAction::triggered, this, &DrumGrid::slotAbout);
    connect( m_ui->actionAbout_Qt, &QAction::triggered, qApp, &QApplication::aboutQt);
    connect( m_ui->actionQuit, &QAction::triggered, this, &DrumGrid::close);
    connect( m_ui->actionConnect, &QAction::triggered, this, &DrumGrid::connectMidi);
    connect( m_ui->startButton, &QPushButton::clicked, this, &DrumGrid::play);
    connect( m_ui->stopButton, &QPushButton::clicked, this, &DrumGrid::stop);
    connect( m_ui->tempoSlider, &QSlider::valueChanged, this, &DrumGrid::tempoChange);
    connect( m_ui->gridColumns, QOverload<int>::of(&QSpinBox::valueChanged), this, &DrumGrid::gridColumns);

    m_model = new DrumGridModel(this);
    m_model->fillSampleData();
    m_ui->tableView->setModel(m_model);
    connect ( this, &DrumGrid::signalUpdate, this, &DrumGrid::updateDisplay );

    addShortcut(QKeySequence(Qt::Key_F), "f");
    addShortcut(QKeySequence(Qt::Key_P), "p");
    addShortcut(QKeySequence(Qt::Key_1), "1");
    addShortcut(QKeySequence(Qt::Key_2), "2");
    addShortcut(QKeySequence(Qt::Key_3), "3");
    addShortcut(QKeySequence(Qt::Key_4), "4");
    addShortcut(QKeySequence(Qt::Key_5), "5");
    addShortcut(QKeySequence(Qt::Key_6), "6");
    addShortcut(QKeySequence(Qt::Key_7), "7");
    addShortcut(QKeySequence(Qt::Key_8), "8");
    addShortcut(QKeySequence(Qt::Key_9), "9");
    addShortcut(QKeySequence(Qt::Key_0), QString());
    addShortcut(QKeySequence::Delete, QString());
    connect ( m_ui->tableView, &QTableView::doubleClicked, m_model,
              QOverload<const QModelIndex &>::of(&DrumGridModel::changeCell) );

    m_Client = new MidiClient(this);
    m_Client->open();
    m_Client->setClientName("DrumGrid");
    connect( m_Client, &MidiClient::eventReceived, this, &DrumGrid::sequencerEvent, Qt::QueuedConnection );
    m_Port = new MidiPort(this);
    m_Port->attach( m_Client );
    m_Port->setPortName("DrumGrid Output Port");
    m_Port->setCapability( SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_WRITE |
                           SND_SEQ_PORT_CAP_SUBS_READ );
    m_Port->setPortType( SND_SEQ_PORT_TYPE_APPLICATION |
                         SND_SEQ_PORT_TYPE_MIDI_GENERIC );
    m_Queue = m_Client->createQueue("DrumGrid");
    m_queueId = m_Queue->getId();
    m_portId = m_Port->getPortId();
    m_clientId = m_Client->getClientId();
    m_Client->setRealTimeInput(false);
    m_Client->startSequencerInput();
    readSettings();
    updateView();
}

DrumGrid::~DrumGrid()
{
    foreach(QShortcut* s, m_shortcuts)
        delete s;
    m_Port->detach();
    m_Client->close();
    delete m_ui;
}

void DrumGrid::updateView()
{
    m_ui->tableView->resizeColumnsToContents();
    m_ui->tableView->resizeRowsToContents();
}

void DrumGrid::subscribe(const QString& portName)
{
    try {
        if (!m_subscription.isEmpty()) {
            m_Port->unsubscribeTo(m_subscription);
            m_subscription.clear();
        }
        m_Port->subscribeTo(portName);
        m_subscription = portName;
    } catch (const SequencerError& err) {
        qWarning() << "SequencerError exception. Error code: " << err.code()
                   << " (" << err.qstrError() << ")";
        qWarning() << "Location: " << err.location();
    }
}

void DrumGrid::connectMidi()
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
    QString item = QInputDialog::getItem(this, "MIDI port subscription",
                                         "Output port:", items,
                                         current, false, &ok);
    if (ok && !item.isEmpty()) {
        subscribe(item);
    }
}

void DrumGrid::sequencerEvent(SequencerEvent *ev)
{
    switch (ev->getSequencerType()) {
    case SND_SEQ_EVENT_USR0:
        metronome_pattern(ev->getTick());
        m_bar++;
        m_beat = 0;
        break;
    case SND_SEQ_EVENT_USR1:
        m_beat++;
        Q_EMIT signalUpdate(m_bar, m_beat-1);
        break;
    }
    delete ev;
}

void DrumGrid::play()
{
    metronome_set_tempo();
    metronome_start();
}

void DrumGrid::stop()
{
    metronome_stop();
}

void DrumGrid::tempoChange(int newTempo)
{
    QString tip = QString::number(newTempo);
    m_bpm = newTempo;
    metronome_set_tempo();
    m_ui->tempoSlider->setToolTip(tip);
    QToolTip::showText(QCursor::pos(), tip, this);
}

void DrumGrid::gridColumns(int columns)
{
    m_model->updatePatternColumns(columns);
    updateView();
}

void DrumGrid::shortcutPressed(const QString& value)
{
    QModelIndex index = m_ui->tableView->currentIndex();
    m_model->changeCell(index, value);
}

void DrumGrid::addShortcut(const QKeySequence& key, const QString& value)
{
    QShortcut* shortcut = new QShortcut(key, m_ui->tableView);
    connect(shortcut, &QShortcut::activated, this, [=]{ shortcutPressed(value); });
    m_shortcuts.append(shortcut);
}

void DrumGrid::readSettings()
{
    QSettings settings;
    settings.beginGroup(QSTR_WINDOW);
    restoreGeometry(settings.value(QSTR_GEOMETRY).toByteArray());
    restoreState(settings.value(QSTR_STATE).toByteArray());
    settings.endGroup();

    settings.beginGroup(QSTR_MIDI);
    QString midiConn = settings.value(QSTR_CONNECTION).toString();
    m_bpm = settings.value(QSTR_TEMPO, TEMPO_DEFAULT).toInt();
    settings.endGroup();
    if (midiConn.length() > 0) {
        subscribe(midiConn);
    }

    settings.beginGroup(QSTR_PATTERN);
    QStringList keys = settings.allKeys();
    if (!keys.empty()) {
        keys.sort();
        m_model->clearPattern();
        foreach(const QString& key, keys) {
            QStringList row = settings.value(key).toStringList();
            m_model->addPatternData(key.toInt(), row);
        }
        m_model->endOfPattern();
    }
    settings.endGroup();
}

void DrumGrid::writeSettings()
{
    QSettings settings;
    settings.clear();

    settings.beginGroup(QSTR_WINDOW);
    settings.setValue(QSTR_GEOMETRY, saveGeometry());
    settings.setValue(QSTR_STATE, saveState());
    settings.endGroup();

    settings.beginGroup(QSTR_MIDI);
    settings.setValue(QSTR_CONNECTION, m_subscription);
    settings.setValue(QSTR_TEMPO, m_bpm);
    settings.endGroup();

    settings.beginGroup(QSTR_PATTERN);
    for(int r = 0; r < m_model->rowCount(); ++r) {
        settings.setValue( m_model->patternKey(r),
                           m_model->patternData(r) );
    }
    settings.endGroup();

    settings.sync();
}

void DrumGrid::closeEvent( QCloseEvent *event )
{
    writeSettings();
    event->accept();
}

void DrumGrid::metronome_event_output(SequencerEvent* ev)
{
    ev->setSource(m_portId);
    ev->setSubscribers();
    ev->setDirect();
    m_Client->outputDirect(ev);
}

void DrumGrid::sendControlChange(int cc, int value)
{
    ControllerEvent ev(m_channel, cc, value);
    metronome_event_output(&ev);
}

void DrumGrid::sendInitialControls()
{
    metronome_set_program();
    metronome_set_controls();
    metronome_set_tempo();
}

void DrumGrid::metronome_set_program()
{
    ProgramChangeEvent ev(m_channel, m_program);
    metronome_event_output(&ev);
}

void DrumGrid::metronome_schedule_event(SequencerEvent* ev, int tick)
{
    ev->setSource(m_portId);
    if (ev->getSequencerType() >= SND_SEQ_EVENT_USR0)
        ev->setDestination(m_clientId, m_portId);
    else
        ev->setSubscribers();
    ev->scheduleTick(m_queueId, tick, false);
    m_Client->outputDirect(ev);
}

void DrumGrid::metronome_note(int note, int vel, int tick)
{
    if (m_useNoteOff) {
        NoteEvent ev(m_channel, note, vel, m_noteDuration);
        metronome_schedule_event(&ev, tick);
    } else {
        NoteOnEvent ev(m_channel, note, vel);
        metronome_schedule_event(&ev, tick);
    }
}

void DrumGrid::metronome_echo(int tick, int ev_type)
{
    SystemEvent ev(ev_type);
    metronome_schedule_event(&ev, tick);
}

int DrumGrid::decodeVelocity(const QString drumVel)
{
    const qreal f = 127.0 / 9.0;
    int num = 0;
    bool isNum = false;
    if (drumVel.isEmpty())
        return 0;
    if (drumVel == "f")
        return m_strong_velocity;
    else if (drumVel == "p")
        return m_weak_velocity;
    num = drumVel.toInt(&isNum);
    if (isNum)
        return qRound(f * num);
    return 0;
}

void DrumGrid::metronome_pattern(int tick)
{
    int i, j, t, duration, key, vel;
    t = tick;
    duration = m_resolution / 4;
    for(i=0; i<m_model->columnCount(); ++i) {
        for(j=0; j<m_model->rowCount(); ++j) {
            QString n = m_model->patternHit(j, i);
            if (!n.isEmpty()) {
                key = m_model->patternKey(j).toInt();
                vel = decodeVelocity(n);
                metronome_note(key, vel, t);
            }
        }
        metronome_echo(t, SND_SEQ_EVENT_USR1);
        t += duration;
    }
    metronome_echo(t, SND_SEQ_EVENT_USR0);
}

void DrumGrid::metronome_set_tempo()
{
    QueueTempo t = m_Queue->getTempo();
    t.setPPQ(m_resolution);
    t.setNominalBPM(m_bpm);
    m_Queue->setTempo(t);
    m_Client->drainOutput();
}

void DrumGrid::metronome_set_controls()
{
    sendControlChange(VOLUME_CC, m_volume);
    sendControlChange(PAN_CC, m_pan);
}

void DrumGrid::metronome_start()
{
    m_Queue->start();
    m_patternDuration = m_resolution * m_model->columnCount() / 4;
    metronome_pattern(0);
    m_bar = 1;
    m_beat = 0;
    m_playing = true;
}

void DrumGrid::metronome_stop()
{
    m_Queue->stop();
    m_playing = false;
}

void DrumGrid::updateDisplay(int /*bar*/, int beat)
{
    m_ui->tableView->selectColumn(beat);
}

void DrumGrid::slotAbout()
{
    About dlgAbout(this);
    dlgAbout.exec();
}
