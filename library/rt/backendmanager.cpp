/*
    Drumstick RT (realtime MIDI In/Out)
    Copyright (C) 2009-2015 Pedro Lopez-Cabanillas <plcl@users.sf.net>

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

#include <QtGlobal>
#include <QDir>
#include <QPluginLoader>
#include <QCoreApplication>
#include <QLibraryInfo>
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

/**
 * @file backendmanager.cpp
 * Implementation of a class managing realtime MIDI input/output backends
 */

namespace drumstick {
namespace rt {


/**
 * @addtogroup RT
 * @{
 *
 * BackendManager provides a mechanism to find and enumerate backends (plugins)
 * to manage realtime MIDI input/output
 *
 * This class and plugins are multiplatform. The backends implement one of these
 * interfaces:
 *
 * MIDIInput: for plugins that can receive MIDI events
 *
 * MIDIOutput: for plugings that can produce MIDI events
 *
 * @}
 */

    class BackendManager::BackendManagerPrivate {
    public:
        QList<MIDIInput*> m_inputsList;
        QList<MIDIOutput*> m_outputsList;
        ~BackendManagerPrivate()
        {
            clearLists();
        }
        void clearLists()
        {
            m_inputsList.clear();
            m_outputsList.clear();
        }
        void appendDir(const QString& candidate, QStringList& result)
        {
            //qDebug() << "testing " << candidate;
            QDir checked(candidate);
            if (checked.exists() && !result.contains(checked.absolutePath())) {
                result << checked.absolutePath();
            }
        }
    };

    /**
     * @brief Constructor
     */
    BackendManager::BackendManager(): d(new BackendManagerPrivate)
    {
        refresh();
    }

    /**
     * @brief Destructor
     */
    BackendManager::~BackendManager()
    {
        delete d;
    }

    /**
     * @brief returns the paths where backends are searched
     * @return list of paths
     */
    QStringList BackendManager::defaultPaths()
    {
        QStringList result;
        QString appPath = QCoreApplication::applicationDirPath() + QDir::separator();
    #if defined(Q_OS_WIN)
        d->appendDir( appPath + QSTR_DRUMSTICK, result );
    #elif defined(Q_OS_MAC)
        d->appendDir( appPath + QStringLiteral("../PlugIns/") + QSTR_DRUMSTICK, result );
    #else // Linux, Unix...
        QStringList libs;
        libs << "../lib/" << "../lib32/" << "../lib64/";
        foreach(const QString& lib, libs) {
            d->appendDir( appPath + lib + QSTR_DRUMSTICK, result );
        }
    #endif
        d->appendDir( appPath + ".." + QDir::separator() + QSTR_DRUMSTICK, result );
        QByteArray envdir = qgetenv(QSTR_DRUMSTICKRT.toLatin1());
        if(!envdir.isEmpty()) {
            d->appendDir(QString(envdir), result );
        }
        d->appendDir( QDir::homePath() + QDir::separator() + QSTR_DRUMSTICK, result );
        d->appendDir( QLibraryInfo::location(QLibraryInfo::PluginsPath) + QDir::separator() + QSTR_DRUMSTICK, result );
        foreach(const QString& path, QCoreApplication::libraryPaths()) {
            d->appendDir( path + QDir::separator() + QSTR_DRUMSTICK, result );
        }
        return result;
    }

    /**
     * @brief BackendManager::refresh finds the installed backends searching the list of paths
     * provided by the function defaultPaths() applying the optional settings as well.
     * @param settings (optional)
     */
    void BackendManager::refresh(QSettings *settings)
    {
        QString name_in;
        QString name_out;
        QStringList names;
        QStringList paths;

        if (settings != 0) {
            settings->beginGroup(QSTR_DRUMSTICKRT_GROUP);
            d->appendDir(settings->value(QSTR_DRUMSTICKRT_PATH).toString(), paths);
            name_in = settings->value(QSTR_DRUMSTICKRT_PUBLICNAMEIN).toString();
            name_out = settings->value(QSTR_DRUMSTICKRT_PUBLICNAMEOUT).toString();
            names << settings->value(QSTR_DRUMSTICKRT_EXCLUDED).toStringList();
            names << name_in;
            names << name_out;
            settings->endGroup();
        }
        paths << defaultPaths();
        d->clearLists();

        // Dynamic backends
        foreach(const QString& dir, paths) {
            QDir pluginsDir(dir);
            foreach (QString fileName, pluginsDir.entryList(QDir::Files)) {
                if (QLibrary::isLibrary(fileName)) {
                    QPluginLoader loader(pluginsDir.absoluteFilePath(fileName));
                    QObject *obj = loader.instance();
                    if (obj != 0) {
                        MIDIInput *input = qobject_cast<MIDIInput*>(obj);
                        if (input != 0 && !d->m_inputsList.contains(input)) {
                            input->setPublicName(name_in);
                            input->setExcludedConnections(names);
                            d->m_inputsList << input;
                        } else {
                            MIDIOutput *output = qobject_cast<MIDIOutput*>(obj);
                            if (output != 0 && !d->m_outputsList.contains(output)) {
                                output->setPublicName(name_out);
                                output->setExcludedConnections(names);
                                d->m_outputsList << output;
                            }
                        }
                    }
                }
            }
        }

        // Static backends
        foreach(QObject* obj, QPluginLoader::staticInstances()) {
            if (obj != 0) {
                MIDIInput *input = qobject_cast<MIDIInput*>(obj);
                if (input != 0 && !d->m_inputsList.contains(input)) {
                    input->setPublicName(name_in);
                    input->setExcludedConnections(names);
                    d->m_inputsList << input;
                } else {
                    MIDIOutput *output = qobject_cast<MIDIOutput*>(obj);
                    if (output != 0 && !d->m_outputsList.contains(output)) {
                        output->setPublicName(name_out);
                        output->setExcludedConnections(names);
                        d->m_outputsList << output;
                    }
                }
            }
        }
    }

    QList<MIDIInput*> BackendManager::availableInputs()
    {
        return d->m_inputsList;
    }

    QList<MIDIOutput*> BackendManager::availableOutputs()
    {
        return d->m_outputsList;
    }

}}
