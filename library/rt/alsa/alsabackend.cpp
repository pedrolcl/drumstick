/*
    Drumstick Backend using the ALSA Sequencer
    Copyright (C) 2009-2014 Pedro Lopez-Cabanillas <plcl@users.sf.net>

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

#include <QtPlugin>
#include <QWidget>
#include <QDesktopServices>
#include <QMessageBox>
#include <QDebug>
#include <QSettings>
#include <QFileDialog>
#include <sched.h>

#include "alsabackend.h"
#include "alsamidiobject.h"
#include "alsamidioutput.h"
#include "externalsoftsynth.h"
#include "ui_prefs_progs.h"

using namespace drumstick;

namespace DrumstickKaraoke {

    class ALSABackend::BackendPrivate {
    public:
        BackendPrivate():
            m_initialized(false),
            m_backendString("ALSA Sequencer"),
            m_object(0),
            m_output(0),
            m_fluidsynth(new FluidSoftSynth),
            m_timidity(new TimiditySoftSynth),
            m_widget(0)
        { }

        ~BackendPrivate()
        {
            delete m_fluidsynth;
            delete m_timidity;
        }

        void applyConfiguration()
        {
            //qDebug() << Q_FUNC_INFO;
            if (m_widget != 0) {
                m_fluidsynth->setExecutionFlag(ui_prefs_progs.exec_fluid->isChecked());
                m_fluidsynth->setCommand(ui_prefs_progs.cmd_fluid->text());
                m_fluidsynth->setAudio(ui_prefs_progs.audio_fluid->currentText());
                m_fluidsynth->setAudioDev(ui_prefs_progs.audiodev_fluid->text());
                m_fluidsynth->setRate(ui_prefs_progs.rate_fluid->text());
                m_fluidsynth->setUserArgs(ui_prefs_progs.args_fluid->text());
                m_fluidsynth->setSoundFont(ui_prefs_progs.sf2_fluid->text());

                m_timidity->setExecutionFlag(ui_prefs_progs.exec_timidity->isChecked());
                m_timidity->setCommand(ui_prefs_progs.cmd_timidity->text());
                m_timidity->setAudio(ui_prefs_progs.audio_timidity->currentText());
                m_timidity->setAudioDev(ui_prefs_progs.audiodev_timidity->text());
                m_timidity->setRate(ui_prefs_progs.rate_timidity->text());
                m_timidity->setUserArgs(ui_prefs_progs.args_timidity->text());
            }
        }

        bool m_initialized;
        QString m_backendString;
        ALSAMIDIObject *m_object;
        ALSAMIDIOutput *m_output;
        FluidSoftSynth* m_fluidsynth;
        TimiditySoftSynth* m_timidity;
        QWidget *m_widget;
        Ui::prefs_progs ui_prefs_progs;
    };

    ALSABackend::ALSABackend(QObject* parent, const QVariantList& args)
        : Backend(parent, args), d(new BackendPrivate)
    {
        try {
            d->m_object = new ALSAMIDIObject(this);
            d->m_output = new ALSAMIDIOutput(this);
            d->m_object->initialize(d->m_output);
            d->m_initialized = true;
        } catch (const SequencerError& ex) {
            QString errorstr = tr("Fatal error from the ALSA sequencer backend. "
                "This usually happens when the kernel does not have ALSA support, "
                "the device node (/dev/snd/seq) does not exist, "
                "or the kernel module (snd_seq) is not loaded. "
                "Please check your ALSA/MIDI configuration. "
                "Returned error was: %1").arg(ex.qstrError());
            QMessageBox::critical(0, tr("ALSA Sequencer Backend Error"), errorstr);
        } catch (...) {
            qDebug() << "Fatal error from the ALSA sequencer backend. "
            "This usually happens when the kernel does not have ALSA support, "
            "the device node (/dev/snd/seq) does not exist, "
            "or the kernel module (snd_seq) is not loaded. "
            "Please check your ALSA/MIDI configuration. ";
        }
    }

    ALSABackend::~ALSABackend()
    {
        delete d;
    }

    bool ALSABackend::initialized()
    {
        return d->m_initialized;
    }

    QString ALSABackend::backendName()
    {
        return d->m_backendString;
    }

    MIDIObject* ALSABackend::midiObject()
    {
        return d->m_object;
    }

    MIDIOutput* ALSABackend::midiOutput()
    {
        return d->m_output;
    }

    bool ALSABackend::hasSoftSynths()
    {
        return true;
    }

    bool ALSABackend::usingSoftSynths()
    {
        return ( d->m_fluidsynth->needsExecution() ||
                 d->m_timidity->needsExecution() );
    }

    void ALSABackend::setupConfigurationWidget(QWidget* widget)
    {
        //qDebug() << Q_FUNC_INFO;
        if (widget != NULL) {
            d->ui_prefs_progs.setupUi(widget);
            d->m_widget = widget;
            QIcon icon(QIcon::fromTheme("document-open"));
            d->ui_prefs_progs.btn_fluidsynth_cmd->setIcon(icon);
            d->ui_prefs_progs.btn_fluidsynth_sf2->setIcon(icon);
            d->ui_prefs_progs.btn_timidity_cmd->setIcon(icon);
            connect(d->ui_prefs_progs.btn_fluidsynth_cmd, SIGNAL(clicked()), SLOT(openFluidCommand()));
            connect(d->ui_prefs_progs.btn_fluidsynth_sf2, SIGNAL(clicked()), SLOT(openFluidSoundFont()));
            connect(d->ui_prefs_progs.btn_timidity_cmd, SIGNAL(clicked()), SLOT(openTimidityCommand()));


            connect(d->ui_prefs_progs.exec_fluid, SIGNAL(toggled(bool)), SIGNAL(softSynthSettingChanged()));
            connect(d->ui_prefs_progs.cmd_fluid, SIGNAL(textChanged(QString)), SIGNAL(softSynthSettingChanged()));
            connect(d->ui_prefs_progs.audio_fluid, SIGNAL(currentIndexChanged(int)), SIGNAL(softSynthSettingChanged()));
            connect(d->ui_prefs_progs.audiodev_fluid, SIGNAL(textChanged(QString)), SIGNAL(softSynthSettingChanged()));
            connect(d->ui_prefs_progs.rate_fluid, SIGNAL(textChanged(QString)), SIGNAL(softSynthSettingChanged()));
            connect(d->ui_prefs_progs.args_fluid, SIGNAL(textChanged(QString)), SIGNAL(softSynthSettingChanged()));
            connect(d->ui_prefs_progs.sf2_fluid, SIGNAL(textChanged(QString)), SIGNAL(softSynthSettingChanged()));

            connect(d->ui_prefs_progs.exec_timidity, SIGNAL(toggled(bool)), SIGNAL(softSynthSettingChanged()));
            connect(d->ui_prefs_progs.cmd_timidity, SIGNAL(textChanged(QString)), SIGNAL(softSynthSettingChanged()));
            connect(d->ui_prefs_progs.audio_timidity, SIGNAL(currentIndexChanged(int)), SIGNAL(softSynthSettingChanged()));
            connect(d->ui_prefs_progs.audiodev_timidity, SIGNAL(textChanged(QString)), SIGNAL(softSynthSettingChanged()));
            connect(d->ui_prefs_progs.rate_timidity, SIGNAL(textChanged(QString)), SIGNAL(softSynthSettingChanged()));
            connect(d->ui_prefs_progs.args_timidity, SIGNAL(textChanged(QString)), SIGNAL(softSynthSettingChanged()));
        }
    }

    void ALSABackend::initializeSoftSynths()
    {
        //qDebug() << Q_FUNC_INFO;
        //d->m_fluidsynth = new FluidSoftSynth();
        //d->m_fluidsynth->readSettings();
        connect( d->m_fluidsynth,
                 SIGNAL(synthErrors(const QString&, const QStringList&)),
                 SIGNAL(softSynthErrors(const QString&, const QStringList&)) );
        connect( d->m_fluidsynth,
                 SIGNAL(synthReady(const QString&, const QStringList&)),
                 SIGNAL(softSynthStarted(const QString&, const QStringList&)) );
        //d->m_fluidsynth->check();
        d->m_fluidsynth->setMidiOutput(d->m_output);
        if (d->m_fluidsynth->needsExecution())
            d->m_fluidsynth->start();
        //d->m_timidity = new TimiditySoftSynth();
        //d->m_timidity->readSettings();
        connect( d->m_timidity,
                 SIGNAL(synthErrors(const QString&, const QStringList&)),
                 SIGNAL(softSynthErrors(const QString&, const QStringList&)) );
        connect(d->m_timidity,
                SIGNAL(synthReady(const QString&, const QStringList&)),
                SIGNAL(softSynthStarted(const QString&, const QStringList&)) );
        //d->m_timidity->check();
        d->m_timidity->setMidiOutput(d->m_output);
        if (d->m_timidity->needsExecution())
            d->m_timidity->start();
    }

    void ALSABackend::terminateSoftSynths()
    {
        //qDebug() << Q_FUNC_INFO;
        d->m_timidity->terminate();
        d->m_fluidsynth->terminate();
    }

    bool ALSABackend::applySoftSynthSettings()
    {
        qDebug() << Q_FUNC_INFO;
        bool changedFluid(false);
        bool changedTimidity(false);
        d->applyConfiguration();
        changedFluid = d->m_fluidsynth->settingsChanged();
        if (changedFluid) {
            d->m_fluidsynth->terminate();
            d->m_fluidsynth->check();
            if (d->m_fluidsynth->needsExecution())
                d->m_fluidsynth->start(true);
            d->m_fluidsynth->writeSettings();
            d->m_fluidsynth->clearSettingsChanged();
        }
        changedTimidity = d->m_timidity->settingsChanged();
        if (changedTimidity) {
            d->m_timidity->terminate();
            d->m_timidity->check();
            if (d->m_timidity->needsExecution())
                d->m_timidity->start(true);
            d->m_timidity->writeSettings();
            d->m_timidity->clearSettingsChanged();
        }
        return changedTimidity | changedFluid;
    }

    void ALSABackend::updateConfigWidget()
    {
        //qDebug() << Q_FUNC_INFO;
        QString version;
        QString available;
        bool Ok = d->m_fluidsynth->isProgramOK();
        version = d->m_fluidsynth->programVersion();
        d->ui_prefs_progs.label_fluid_icon->setPixmap(
            Ok ? QIcon::fromTheme("flag-green").pixmap(24,24) :
                 QIcon::fromTheme("flag-red").pixmap(24,24) );
        available = version.isEmpty() ? tr("Not valid") :
            tr("Found version: %1").arg(version);
        if (!Ok) {
            available += "<br>";
            available += d->m_fluidsynth->getSettingsErrorMessage();
        }
        d->ui_prefs_progs.label_fluid_available->setText(available);
        d->ui_prefs_progs.exec_fluid->setEnabled(Ok);
        d->ui_prefs_progs.exec_fluid->setChecked(d->m_fluidsynth->needsExecution());
        d->ui_prefs_progs.cmd_fluid->setText(d->m_fluidsynth->command());
        d->ui_prefs_progs.audio_fluid->setEnabled(Ok);
        d->ui_prefs_progs.audio_fluid->setCurrentIndex(d->m_fluidsynth->audioIndex());
        d->ui_prefs_progs.args_fluid->setEnabled(Ok);
        d->ui_prefs_progs.args_fluid->setText(d->m_fluidsynth->userArgs());
        d->ui_prefs_progs.audiodev_fluid->setEnabled(Ok);
        d->ui_prefs_progs.audiodev_fluid->setText(d->m_fluidsynth->audioDev());
        d->ui_prefs_progs.rate_fluid->setEnabled(Ok);
        d->ui_prefs_progs.rate_fluid->setText(d->m_fluidsynth->rate());
        d->ui_prefs_progs.sf2_fluid->setText(d->m_fluidsynth->soundFont());

        Ok = d->m_timidity->isProgramOK();
        version = d->m_timidity->programVersion();
        d->ui_prefs_progs.label_timidity_icon->setPixmap(
            Ok ? QIcon::fromTheme("flag-green").pixmap(24,24) :
                 QIcon::fromTheme("flag-red").pixmap(24,24) );
        available = version.isEmpty() ? tr("Not valid") :
            tr("Found version: %1").arg(version);
        if (!Ok) {
            available += "<br>";
            available += d->m_timidity->getSettingsErrorMessage();
        }
        d->ui_prefs_progs.label_timidity_available->setText(available);
        d->ui_prefs_progs.exec_timidity->setEnabled(Ok);
        d->ui_prefs_progs.exec_timidity->setChecked(d->m_timidity->needsExecution());
        d->ui_prefs_progs.cmd_timidity->setText(d->m_timidity->command());
        d->ui_prefs_progs.audio_timidity->setEnabled(Ok);
        d->ui_prefs_progs.audio_timidity->setCurrentIndex(d->m_timidity->audioIndex());
        d->ui_prefs_progs.args_timidity->setEnabled(Ok);
        d->ui_prefs_progs.args_timidity->setText(d->m_timidity->userArgs());
        d->ui_prefs_progs.audiodev_timidity->setEnabled(Ok);
        d->ui_prefs_progs.audiodev_timidity->setText(d->m_timidity->audioDev());
        d->ui_prefs_progs.rate_timidity->setEnabled(Ok);
        d->ui_prefs_progs.rate_timidity->setText(d->m_timidity->rate());
    }

    void ALSABackend::saveSettings()
    {
        //qDebug() << Q_FUNC_INFO;
        d->applyConfiguration();
        d->m_fluidsynth->writeSettings();
        d->m_timidity->writeSettings();
    }

    void ALSABackend::openFluidCommand()
    {
        QString fileName = QFileDialog::getOpenFileName(d->m_widget,
            tr("Open Command"), "/usr/bin", QString());
        if (!fileName.isEmpty()) {
            d->ui_prefs_progs.cmd_fluid->setText(fileName);
        }
    }

    void ALSABackend::openFluidSoundFont()
    {
        const QString ubuntuDir("/usr/share/sounds/sf2");
        const QString fedoraDir("/usr/share/soundfonts");
        QString initialdir;
        QDir dirInfo(ubuntuDir);
        if (dirInfo.exists())
            initialdir = ubuntuDir;
        else
            initialdir = fedoraDir;
        QString fileName = QFileDialog::getOpenFileName(d->m_widget,
            tr("Open SoundFont"), initialdir, tr("SoundFonts (*.sf2)"));
        if (!fileName.isEmpty()) {
            d->ui_prefs_progs.sf2_fluid->setText(fileName);
        }
    }

    void ALSABackend::openTimidityCommand()
    {
        QString fileName = QFileDialog::getOpenFileName(d->m_widget,
            tr("Open Command"), "/usr/bin", QString());
        if (!fileName.isEmpty()) {
            d->ui_prefs_progs.cmd_timidity->setText(fileName);
        }
    }

    Q_EXPORT_PLUGIN2(backend_alsa, ALSABackend)
}
