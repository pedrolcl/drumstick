#include <QCoreApplication>
#include <QFileInfo>
#include <QtPlugin>
#include <QPluginLoader>
#include <QString>
#include <QStringList>
#include <QDebug>

#include "rtmidiinput.h"
#include "rtmidioutput.h"


#if defined(ALSA_BACKEND)
#if QT_VERSION < QT_VERSION_CHECK(5,0,0)
namespace drumstick {
namespace rt {
Q_IMPORT_PLUGIN(drumstick_rt_alsa_in)
Q_IMPORT_PLUGIN(drumstick_rt_alsa_out)
}}
#else
#if defined(DUMMY_BACKEND)
Q_IMPORT_PLUGIN(DummyInput)
Q_IMPORT_PLUGIN(DummyOutput)
#endif
Q_IMPORT_PLUGIN(ALSAMIDIInput)
Q_IMPORT_PLUGIN(ALSAMIDIOutput)
#endif
#endif

using namespace drumstick::rt;


int main(int argc, char **argv)
{
    QCoreApplication::setOrganizationName("drumstick.sourceforge.net");
    QCoreApplication::setOrganizationDomain("drumstick.sourceforge.net");
    QCoreApplication::setApplicationName("drumstick-rt-test");
    QCoreApplication app(argc, argv);

    QString name_in(QLatin1String("TEST MIDI IN"));
    QString name_out(QLatin1String("TEST MIDI OUT"));
    QStringList names;
    names << name_in;
    names << name_out;

    QList<MIDIInput*> inputsList;
    QList<MIDIOutput*> outputsList;

    QFileInfo exeInfo(app.applicationFilePath());
    qDebug() << "program" << exeInfo.fileName();

    QStringList cliFiles;
    foreach(const QString& a, app.arguments()) {
        if (!a.endsWith(exeInfo.fileName()))
            cliFiles << a;
    }
    qDebug() << "arguments" << cliFiles << endl;

    foreach(QObject* obj, QPluginLoader::staticInstances()) {
        if (obj != 0) {
            MIDIInput *input = qobject_cast<MIDIInput*>(obj);
            if (input != 0) {
                input->setPublicName(name_in);
                input->setExcludedConnections(names);
                inputsList << input;
            } else {
                MIDIOutput *output = qobject_cast<MIDIOutput*>(obj);
                if (output != 0) {
                    output->setPublicName(name_out);
                    output->setExcludedConnections(names);
                    outputsList << output;
                }
            }
        }
    }

    foreach(MIDIInput* input, inputsList) {
        qDebug() << "Input Backend" << input->backendName();
        qDebug() << "public name" << input->publicName();
        qDebug() << "ports" << input->connections();
        qDebug() << "advanced ports" << input->connections(true) << endl;
    }

    foreach(MIDIOutput* output, outputsList) {
        qDebug() << "Output Backend" << output->backendName();
        qDebug() << "public name" << output->publicName();
        qDebug() << "ports" << output->connections();
        qDebug() << "advanced ports" << output->connections(true) << endl;
    }

    return 0;
}
