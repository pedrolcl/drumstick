/*
    Drumstick RT (realtime MIDI In/Out)
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

#include <QDir>
#include <QPluginLoader>
#include <QCoreApplication>
#include "backendmanager.h"

#if defined(ALSA_BACKEND)
Q_IMPORT_PLUGIN(ALSAMIDIInput)
Q_IMPORT_PLUGIN(ALSAMIDIOutput)
#endif

#if defined(MAC_BACKEND)
Q_IMPORT_PLUGIN(MacMIDIInput)
Q_IMPORT_PLUGIN(MacMIDIOutput)
#endif

#if defined(WIN_BACKEND)
Q_IMPORT_PLUGIN(WinMIDIInput)
Q_IMPORT_PLUGIN(WinMIDIOutput)
#endif

#if defined(NET_BACKEND)
Q_IMPORT_PLUGIN(NetMIDIInput)
Q_IMPORT_PLUGIN(NetMIDIOutput)
#endif

#if defined(DUMMY_BACKEND)
Q_IMPORT_PLUGIN(DummyInput)
Q_IMPORT_PLUGIN(DummyOutput)
#endif

#if defined(SYNTH_BACKEND)
Q_IMPORT_PLUGIN(SynthOutput)
#endif

#if defined(OSS_BACKEND)
Q_IMPORT_PLUGIN(OSSInput)
Q_IMPORT_PLUGIN(OSSOutput)
#endif

namespace drumstick {
namespace rt {

    class BackendManager::BackendManagerPrivate {
    public:
        QList<MIDIInput*> m_inputsList;
        QList<MIDIOutput*> m_outputsList;
    };

    BackendManager::BackendManager():
        d(new BackendManagerPrivate)
    {
        refresh();
    }

    QStringList BackendManager::defaultPaths()
    {
        QStringList result;

        const QStringList path_list = QCoreApplication::libraryPaths();

        const QString dirName = QStringLiteral("drumstick");
        foreach (const QString &path, path_list) {
            QString libPath = path;
            libPath += QDir::separator();
            libPath += dirName;
            result.append(libPath);
        }

        /*QString homeLibPath = QDir::homePath();
        homeLibPath += QDir::separator();
        homeLibPath += QStringLiteral(".drumstick");
        homeLibPath += QDir::separator();
        homeLibPath += QStringLiteral("backends");
        result.append(homeLibPath);*/

        return result;
    }

    void BackendManager::refresh(QSettings *settings)
    {
        QString dir;
        QString name_in;
        QString name_out;
        QStringList names;

        if (settings != 0) {
            settings->beginGroup(QSTR_DRUMSTICKRT_GROUP);
            dir = settings->value(QSTR_DRUMSTICKRT_PATH).toString();
            name_in = settings->value(QSTR_DRUMSTICKRT_PUBLICNAMEIN).toString();
            name_out = settings->value(QSTR_DRUMSTICKRT_PUBLICNAMEOUT).toString();
            names << settings->value(QSTR_DRUMSTICKRT_EXCLUDED).toStringList();
            names << name_in;
            names << name_out;
            settings->endGroup();
        }

        // Dynamic backends
        QDir pluginsDir(dir);
        foreach (QString fileName, pluginsDir.entryList(QDir::Files)) {
            QPluginLoader loader(pluginsDir.absoluteFilePath(fileName));
            QObject *obj = loader.instance();
            if (obj != 0) {
                MIDIInput *input = qobject_cast<MIDIInput*>(obj);
                if (input != 0) {
                    input->setPublicName(name_in);
                    input->setExcludedConnections(names);
                    d->m_inputsList << input;
                } else {
                    MIDIOutput *output = qobject_cast<MIDIOutput*>(obj);
                    if (output != 0) {
                        output->setPublicName(name_out);
                        output->setExcludedConnections(names);
                        d->m_outputsList << output;
                    }
                }
            }
        }

        // Static backends
        foreach(QObject* obj, QPluginLoader::staticInstances()) {
            if (obj != 0) {
                MIDIInput *input = qobject_cast<MIDIInput*>(obj);
                if (input != 0) {
                    input->setPublicName(name_in);
                    input->setExcludedConnections(names);
                    d->m_inputsList << input;
                } else {
                    MIDIOutput *output = qobject_cast<MIDIOutput*>(obj);
                    if (output != 0) {
                        output->setPublicName(name_out);
                        output->setExcludedConnections(names);
                        d->m_outputsList << output;
                    }
                }
            }
        }
    }

    QList<MIDIInput*> BackendManager::inputsAvailable()
    {
        return d->m_inputsList;
    }

    QList<MIDIOutput*> BackendManager::outputsAvailable()
    {
        return d->m_outputsList;
    }

}}
