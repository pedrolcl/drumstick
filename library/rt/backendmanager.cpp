/*
    Drumstick RT (realtime MIDI In/Out)
    Copyright (C) 2009-2025 Pedro Lopez-Cabanillas <plcl@users.sf.net>

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

#include <QCoreApplication>
#include <QDir>
#include <QLibraryInfo>
#include <QPluginLoader>
#include <QtGlobal>
#include <drumstick/backendmanager.h>

/**
 * @file backendmanager.cpp
 * Implementation of a class managing realtime MIDI input/output backends
 */

namespace drumstick { namespace rt {

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
 * MIDIOutput: for plugins that can consume MIDI events
 *
 * @}
 */

    class BackendManager::BackendManagerPrivate {
    public:
        QList<QPluginLoader *> m_loaders;
        QList<MIDIInput *> m_inputsList;
        QList<MIDIOutput *> m_outputsList;
        static BackendManager *m_instance;

        QString m_inputBackend{QLatin1String("Network")};
#if defined(Q_OS_LINUX)
        QStringList m_outputBackends{QLatin1String("SonivoxEAS"),QLatin1String("FluidSynth"),QLatin1String("ALSA")};
#elif defined(Q_OS_DARWIN)
        QStringList m_outputBackends{QLatin1String("DLS Synth"),QLatin1String("FluidSynth"),QLatin1String("CoreMIDI")};
#elif defined(Q_OS_WINDOWS)
        QStringList m_outputBackends{QLatin1String("Windows MM"),QLatin1String("FluidSynth")};
#elif defined(Q_OS_UNIX)
        QStringList m_outputBackends{QLatin1String("FluidSynth"),QLatin1String("OSS")};
#else
        QStringList m_outputBackends{m_inputBackend};
#endif

        ~BackendManagerPrivate()
        {
            clearLists();
        }

        void clearLists()
        {
            // qDebug() << Q_FUNC_INFO << "loaders:" << m_loaders.count()
            //          << "inputs:" << m_inputsList.count() << "outputs:" << m_outputsList.count();
            while (!m_loaders.empty()) {
                QPluginLoader* pluginLoader = m_loaders.takeFirst();
                //qDebug() << "unloading:" << pluginLoader->fileName();
                pluginLoader->unload();
                delete pluginLoader;
            }
            m_inputsList.clear();
            m_outputsList.clear();
            m_loaders.clear();
        }

        void appendDir(const QString &candidate, QStringList &result)
        {
            QDir checked(candidate.trimmed());
            //qDebug() << Q_FUNC_INFO << candidate << "exists:" << checked.exists();
            if (checked.exists() && !result.contains(checked.absolutePath())) {
                result << checked.absolutePath();
            }
        }

        bool isLoaderNeeded(const QString &fileName)
        {
            auto it = std::find_if(m_loaders.constBegin(),
                                   m_loaders.constEnd(),
                                   [=](QPluginLoader *loader) {
                                       return loader->fileName() == fileName;
                                   });
            return it == m_loaders.constEnd();
        }
    };

    /**
     * @brief Constructor
     */
    BackendManager::BackendManager()
        : d{new BackendManagerPrivate}
    {
        //qDebug() << Q_FUNC_INFO;
        QVariantMap defaultSettings {
            { QSTR_DRUMSTICKRT_PUBLICNAMEIN, QStringLiteral("MIDI In")},
            { QSTR_DRUMSTICKRT_PUBLICNAMEOUT, QStringLiteral("MIDI Out")}
        };
        refresh(defaultSettings);
        BackendManager::BackendManagerPrivate::m_instance = this;
    }

    /**
     * @brief Destructor
     */
    BackendManager::~BackendManager()
    {
        //qDebug() << Q_FUNC_INFO;
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
        d->appendDir( appPath + "../lib/" + QSTR_DRUMSTICK, result );
    #else
    #if defined(Q_OS_MAC)
        d->appendDir( appPath + QStringLiteral("../PlugIns/") + QSTR_DRUMSTICK, result );
    #endif // Linux, Unix...
        QStringList libs;
        libs << "../lib/";
    #if defined(LIBSUFFIX)
        QString libextra(QT_STRINGIFY(LIBSUFFIX));
        if (QDir::isAbsolutePath(libextra)) {
            d->appendDir( libextra + QDir::separator() + QSTR_DRUMSTICK, result );
        } else {
            libs << QString("../%1/").arg(libextra);
        }
    #endif
        foreach(const QString& lib, libs) {
            d->appendDir( appPath + lib + QSTR_DRUMSTICK, result );
        }
    #endif
        d->appendDir( appPath + ".." + QDir::separator() + QSTR_DRUMSTICK, result );
        QByteArray envdir = qgetenv(QSTR_DRUMSTICKRT.toLatin1());
        //qDebug() << Q_FUNC_INFO << "envdir:" << envdir;
        if(!envdir.isEmpty()) {
            d->appendDir(QString(envdir), result );
        }
        d->appendDir( QDir::homePath() + QDir::separator() + QSTR_DRUMSTICK, result );
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
        d->appendDir( QLibraryInfo::location(QLibraryInfo::PluginsPath) + QDir::separator() + QSTR_DRUMSTICK, result );
#else
        d->appendDir( QLibraryInfo::path(QLibraryInfo::PluginsPath) + QDir::separator() + QSTR_DRUMSTICK, result );
#endif
        foreach(const QString& path, QCoreApplication::libraryPaths()) {
            d->appendDir( path + QDir::separator() + QSTR_DRUMSTICK, result );
        }
        return result;
    }

    /**
     * @brief BackendManager::refresh finds the installed backends applying the provided settings.
     * @param settings Program settings
     */
    void BackendManager::refresh(QSettings *settings)
    {
        //qDebug() << Q_FUNC_INFO;
        QVariantMap tmpMap;
        settings->beginGroup(QSTR_DRUMSTICKRT_GROUP);
        const QStringList allKeys = settings->allKeys();
        //qDebug() << Q_FUNC_INFO << allKeys;
        for(const auto &k : allKeys) {
            tmpMap.insert(k, settings->value(k));
        }
        settings->endGroup();
        refresh(tmpMap);
    }

    /**
     * @brief BackendManager::refresh finds the installed backends searching the list of paths
     * provided by the function defaultPaths() applying the provided settings map as well.
     * @param map Program settings relevant section
     */
    void BackendManager::refresh(const QVariantMap &map)
    {
        QString name_in;
        QString name_out;
        QStringList names;
        QStringList paths;

        d->appendDir(map.value(QSTR_DRUMSTICKRT_PATH).toString(), paths);
        name_in = map.value(QSTR_DRUMSTICKRT_PUBLICNAMEIN).toString();
        name_out = map.value(QSTR_DRUMSTICKRT_PUBLICNAMEOUT).toString();
        names << map.value(QSTR_DRUMSTICKRT_EXCLUDED).toStringList();
        names << (name_in.isEmpty() ? QStringLiteral("MIDI In") : name_in);
        names << (name_out.isEmpty() ? QStringLiteral("MIDI Out") : name_out);
        paths << defaultPaths();

        //qDebug() << Q_FUNC_INFO << "names:" << names << "paths:" << paths;

        // Dynamic backends
        foreach(const QString& dir, paths) {
            QDir pluginsDir(dir);
            foreach (QString fileName, pluginsDir.entryList(QDir::Files)) {
                auto absolutePath = pluginsDir.absoluteFilePath(fileName);
                if (QLibrary::isLibrary(absolutePath) && d->isLoaderNeeded(absolutePath)) {
                    QPluginLoader *loader = new QPluginLoader(absolutePath);
                    //qDebug() << "plugin loader created:" << loader->fileName();
                    d->m_loaders << loader;
                    QObject *obj = loader->instance();
                    if (obj != nullptr) {
                        MIDIInput *input = qobject_cast<MIDIInput *>(obj);
                        if (input != nullptr && !d->m_inputsList.contains(input)) {
                            //qDebug() << "input plugin instantiated:" << name_in;
                            if (!name_in.isEmpty()) {
                                input->setPublicName(name_in);
                            }
                            input->setExcludedConnections(names);
                            d->m_inputsList << input;
                        } else {
                            MIDIOutput *output = qobject_cast<MIDIOutput *>(obj);
                            if (output != nullptr && !d->m_outputsList.contains(output)) {
                                //qDebug() << "output plugin instantiated:" << name_out;
                                if (!name_out.isEmpty()) {
                                    output->setPublicName(name_out);
                                }
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
            if (obj != nullptr) {
                MIDIInput *input = qobject_cast<MIDIInput*>(obj);
                if (input != nullptr && !d->m_inputsList.contains(input)) {
                    if (!name_in.isEmpty()) {
                        input->setPublicName(name_in);
                    }
                    input->setExcludedConnections(names);
                    d->m_inputsList << input;
                } else {
                    MIDIOutput *output = qobject_cast<MIDIOutput*>(obj);
                    if (output != nullptr && !d->m_outputsList.contains(output)) {
                        if (!name_out.isEmpty()) {
                            output->setPublicName(name_out);
                        }
                        output->setExcludedConnections(names);
                        d->m_outputsList << output;
                    }
                }
            }
        }

        foreach (MIDIInput *in, d->m_inputsList) {
            if (!name_in.isEmpty()) {
                in->setPublicName(name_in);
            }
            in->setExcludedConnections(names);
        }

        foreach (MIDIOutput *out, d->m_outputsList) {
            if (!name_out.isEmpty()) {
                out->setPublicName(name_out);
            }
            out->setExcludedConnections(names);
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

    MIDIInput* BackendManager::inputBackendByName(const QString name)
    {
        foreach (MIDIInput* i, d->m_inputsList) {
            if (i->backendName() == name) {
                return i;
            }
        }
        return nullptr;
    }

    MIDIOutput* BackendManager::outputBackendByName(const QString name)
    {
        foreach (MIDIOutput* i, d->m_outputsList) {
            if (i->backendName() == name) {
                return i;
            }
        }
        return nullptr;
    }

    MIDIInput* BackendManager::findInput(QString name)
    {
        QStringList names{name};
        names << d->m_inputBackend;
        names.removeDuplicates();
        if (!names.isEmpty()) {
            foreach(const QString& n, names) {
                foreach(MIDIInput* input, d->m_inputsList) {
                    if (input->backendName() == n)  {
                        return input;
                    }
                }
            }
        }
        return nullptr;
    }

    MIDIOutput* BackendManager::findOutput(QString name)
    {
        QStringList names{name};
        names << d->m_outputBackends;
        names.removeDuplicates();
        if (!names.isEmpty()) {
            foreach(const QString& n, names) {
                foreach(MIDIOutput* output, d->m_outputsList) {
                    if (output->backendName() == n)  {
                        return output;
                    }
                }
            }
        }
        return nullptr;
    }

    const QString BackendManager::QSTR_DRUMSTICK = QStringLiteral("drumstick2");
    const QString BackendManager::QSTR_DRUMSTICK_VERSION = QStringLiteral(QT_STRINGIFY(VERSION));
    const QString BackendManager::QSTR_DRUMSTICKRT = QStringLiteral("DRUMSTICKRT");
    const QString BackendManager::QSTR_DRUMSTICKRT_GROUP = QStringLiteral("DrumstickRT");
    const QString BackendManager::QSTR_DRUMSTICKRT_PUBLICNAMEIN = QStringLiteral("PublicNameIN");
    const QString BackendManager::QSTR_DRUMSTICKRT_PUBLICNAMEOUT = QStringLiteral("PublicNameOUT");
    const QString BackendManager::QSTR_DRUMSTICKRT_EXCLUDED = QStringLiteral("ExcludedNames");
    const QString BackendManager::QSTR_DRUMSTICKRT_PATH = QStringLiteral("BackendsPath");

    BackendManager *BackendManager::BackendManagerPrivate::m_instance = nullptr;

    /**
     * @brief drumstickLibraryVersion provides the Drumstick version as an edited QString
     * @return Drumstick library version
     */
    QString drumstickLibraryVersion()
    {
        return BackendManager::QSTR_DRUMSTICK_VERSION;
    }

    /**
     * @brief lastBackendManagerInstance provides the latest BackendManager instance,
     * or a new instance if needed.
     * @return a BackendManager instance pointer
     * @since 2.11
     */
    BackendManager *lastBackendManagerInstance()
    {
        if (BackendManager::BackendManagerPrivate::m_instance == nullptr) {
            return new BackendManager;
        }
        return BackendManager::BackendManagerPrivate::m_instance;
    }

} // namespace rt
} // namespace drumstick
