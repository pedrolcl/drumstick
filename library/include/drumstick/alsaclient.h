/*
    MIDI Sequencer C++ library
    Copyright (C) 2006-2020, Pedro Lopez-Cabanillas <plcl@users.sf.net>

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef DRUMSTICK_ALSACLIENT_H
#define DRUMSTICK_ALSACLIENT_H

#include <QObject>
#include <QPointer>
#include <QThread>
#include <QReadWriteLock>
#include "macros.h"
#include "alsaport.h"

/**
 * @file alsaclient.h
 * Classes managing ALSA Sequencer clients
 */

/**
 * @brief Drumstick common
 */
namespace drumstick {
/**
 * @ingroup ALSAGroup
 * @brief Drumstick ALSA library wrapper
 */
namespace ALSA {

class MidiQueue;
class MidiClient;
class SequencerEvent;
class SequencerInputThread;
class RemoveEvents;

/**
 * @addtogroup ALSAClient ALSA Sequencer Clients
 * @{
 *
 * @class ClientInfo
 * Client information
 *
 * This class is used to retrieve, hold and set some data from
 * sequencer clients, like the name or id.
 */
class DRUMSTICK_EXPORT ClientInfo
{
    friend class MidiClient;

public:
    ClientInfo();
    ClientInfo(const ClientInfo& other);
    explicit ClientInfo(snd_seq_client_info_t* other);
    ClientInfo(MidiClient* seq, int id);
    virtual ~ClientInfo();
    ClientInfo* clone();
    ClientInfo& operator=(const ClientInfo& other);
    int getSizeOfInfo() const;

    int getClientId();
    snd_seq_client_type_t getClientType();
    QString getName();
    bool getBroadcastFilter();
    bool getErrorBounce();
    int getNumPorts();
    int getEventLost();
    void setClient(int client);
    void setName(QString name);
    void setBroadcastFilter(bool val);
    void setErrorBounce(bool val);
    PortInfoList getPorts() const;

#if SND_LIB_VERSION > 0x010010
    void addFilter(int eventType);
    bool isFiltered(int eventType);
    void clearFilter();
    void removeFilter(int eventType);
#endif

protected:
    void readPorts(MidiClient* seq);
    void freePorts();

    const unsigned char* getEventFilter() __attribute__((deprecated));
    void setEventFilter(unsigned char* filter) __attribute__((deprecated));

private:
    snd_seq_client_info_t* m_Info;
    PortInfoList m_Ports;
};

/**
 * List of sequencer client information
 */
typedef QList<ClientInfo> ClientInfoList;

/**
 * System information
 *
 * This class is used to retrieve and hold some data about the
 * whole sequencer subsystem.
 */
class DRUMSTICK_EXPORT SystemInfo
{
    friend class MidiClient;

public:
    SystemInfo();
    SystemInfo(const SystemInfo& other);
    explicit SystemInfo(snd_seq_system_info_t* other);
    explicit SystemInfo(MidiClient* seq);
    virtual ~SystemInfo();
    SystemInfo* clone();
    SystemInfo& operator=(const SystemInfo& other);
    int getSizeOfInfo() const;

    int getMaxClients();
    int getMaxPorts();
    int getMaxQueues();
    int getMaxChannels();
    int getCurrentQueues();
    int getCurrentClients();

private:
    snd_seq_system_info_t* m_Info;
};

/**
 * Sequencer Pool information
 *
 * This class is used to get and set the size of the input and output pool
 * buffers for a sequencer client.
 */
class DRUMSTICK_EXPORT PoolInfo
{
    friend class MidiClient;

public:
    PoolInfo();
    PoolInfo(const PoolInfo& other);
    explicit PoolInfo(snd_seq_client_pool_t* other);
    explicit PoolInfo(MidiClient* seq);
    virtual ~PoolInfo();
    PoolInfo* clone();
    PoolInfo& operator=(const PoolInfo& other);
    int getSizeOfInfo() const;

    int getClientId();
    int getInputFree();
    int getInputPool();
    int getOutputFree();
    int getOutputPool();
    int getOutputRoom();
    void setInputPool(int size);
    void setOutputPool(int size);
    void setOutputRoom(int size);

private:
    snd_seq_client_pool_t* m_Info;
};

/**
 * Sequencer events handler
 *
 * This abstract class is used to define an interface that other class can
 * implement to receive sequencer events. It is one of the three methods of
 * delivering events offered by this library.
 *
 * @see ALSAClient
 */
class DRUMSTICK_EXPORT SequencerEventHandler
{
public:
    /** Destructor */
    virtual ~SequencerEventHandler() {}

    /**
     * Callback function to be implemented by the derived class.
     * It will be invoked by the client to deliver received events to the
     * registered listener.
     *
     * @param ev A pointer to the received SequencerEvent
     * @see MidiClient::setHandler(), MidiClient::startSequencerInput(),
     * MidiClient::stopSequencerInput(), MidiClient::doEvents()
     */
    virtual void handleSequencerEvent(SequencerEvent* ev) = 0;
};

/**
 * Client management.
 *
 * This class represents an ALSA sequencer client
 */
class DRUMSTICK_EXPORT MidiClient : public QObject
{
    Q_OBJECT
public:
    explicit MidiClient( QObject* parent = nullptr );
    virtual ~MidiClient();

    void open( const QString deviceName = "default",
               const int openMode = SND_SEQ_OPEN_DUPLEX,
               const bool blockMode = false );
    void open( snd_config_t* conf,
               const QString deviceName = "default",
               const int openMode = SND_SEQ_OPEN_DUPLEX,
               const bool blockMode = false );
    void close();
    void startSequencerInput();
    void stopSequencerInput();
    MidiPort* createPort();
    MidiQueue* createQueue();
    MidiQueue* createQueue(QString const& name);
    MidiQueue* getQueue();
    MidiQueue* useQueue(int queue_id);
    MidiQueue* useQueue(const QString& name);
    MidiQueue* useQueue(MidiQueue* queue);
    void portAttach(MidiPort* port);
    void portDetach(MidiPort* port);
    void detachAllPorts();
    void addEventFilter(int evtype);
    void output(SequencerEvent* ev, bool async = false, int timeout = -1);
    void outputDirect(SequencerEvent* ev, bool async = false, int timeout = -1);
    void outputBuffer(SequencerEvent* ev);
    void drainOutput(bool async = false, int timeout = -1);
    void synchronizeOutput();

    int getClientId();
    snd_seq_type_t getSequencerType();
    snd_seq_t* getHandle();
    bool isOpened();

    size_t getOutputBufferSize();
    void setOutputBufferSize(size_t newSize);
    size_t getInputBufferSize();
    void setInputBufferSize(size_t newSize);
    QString getDeviceName();
    int getOpenMode();
    bool getBlockMode();
    void setBlockMode(bool newValue);
    QString getClientName();
    QString getClientName(const int clientId);
    void setClientName(QString const& newName);
    bool getBroadcastFilter();
    void setBroadcastFilter(bool newValue);
    bool getErrorBounce();
    void setErrorBounce(bool newValue);

    ClientInfo& getThisClientInfo();
    void setThisClientInfo(const ClientInfo& val);
    MidiPortList getMidiPorts() const;
    ClientInfoList getAvailableClients();
    PortInfoList getAvailableInputs();
    PortInfoList getAvailableOutputs();
    SystemInfo& getSystemInfo();
    QList<int> getAvailableQueues();

    PoolInfo& getPoolInfo();
    void setPoolInfo(const PoolInfo& info);
    void setPoolInput(int size);
    void setPoolOutput(int size);
    void setPoolOutputRoom(int size);
    void resetPoolInput();
    void resetPoolOutput();
    void dropInput();
    void dropInputBuffer();
    void dropOutput();
    void dropOutputBuffer();
    void removeEvents(const RemoveEvents* spec);
    SequencerEvent* extractOutput();
    int outputPending();
    int inputPending(bool fetch);
    int getQueueId(const QString& name);

    void addListener(QObject* listener);
    void removeListener(QObject* listener);
    void setEventsEnabled(const bool bEnabled);
    bool getEventsEnabled() const;
    void setHandler(SequencerEventHandler* handler);
    bool parseAddress( const QString& straddr, snd_seq_addr& result );
    void setRealTimeInput(bool enabled);
    bool realTimeInputEnabled();

signals:
    /** Signal emitted when an event is received
     * @param ev pointer to the received event
     */
    void eventReceived(SequencerEvent* ev);

protected:
    void doEvents();
    void applyClientInfo();
    void readClients();
    void freeClients();
    void updateAvailablePorts();
    PortInfoList filterPorts(unsigned int filter);

    /* low level public functions */
    const char * _getDeviceName();
    int getPollDescriptorsCount(short events);
    int pollDescriptors(struct pollfd *pfds, unsigned int space, short events);
    unsigned short pollDescriptorsRevents(struct pollfd *pfds, unsigned int nfds);

    /* mid level functions */
    void _setClientName( const char *name );
    int createSimplePort( const char *name,
                          unsigned int caps,
                          unsigned int type );
    void deleteSimplePort( int port );
    void connectFrom(int myport, int client, int port);
    void connectTo(int myport, int client, int port);
    void disconnectFrom(int myport, int client, int port);
    void disconnectTo(int myport, int client, int port);

private:
    class SequencerInputThread;
    class MidiClientPrivate;
    MidiClientPrivate *d;
};

#if SND_LIB_VERSION > 0x010004
DRUMSTICK_EXPORT QString getRuntimeALSALibraryVersion();
DRUMSTICK_EXPORT int getRuntimeALSALibraryNumber();
#endif
DRUMSTICK_EXPORT QString getRuntimeALSADriverVersion();
DRUMSTICK_EXPORT int getRuntimeALSADriverNumber();

/**
 * ALSA library version as a constant string.
 *
 * This string corresponds to the compilation library, which may be
 * different to the runtime library.
 * @see getRuntimeALSALibraryVersion
 */
const QString LIBRARY_VERSION = QLatin1String(SND_LIB_VERSION_STR);

/** @} */

}} /* namespace drumstick::ALSA */

#endif // DRUMSTICK_ALSACLIENT_H
