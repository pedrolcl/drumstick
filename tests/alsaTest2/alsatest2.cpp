#include <QString>
#include <QtTest>
#include "alsatimer.h"

using namespace drumstick;

class AlsaTest2 : public QObject, public TimerEventHandler
{
    Q_OBJECT

public:
    AlsaTest2();
    // TimerEventHandler implementation
    void handleTimerEvent(int ticks, int msecs);

private Q_SLOTS:
    void testTimer();

private:
    TimerInfo   m_info;
    TimerStatus m_status;
    TimerParams m_params;
    int         m_count;
};

AlsaTest2::AlsaTest2():
    m_count(0)
{
}

void AlsaTest2::handleTimerEvent(int , int )
{
    m_count++;
}

void AlsaTest2::testTimer()
{
    QPointer<Timer> test_timer;
    test_timer = Timer::bestGlobalTimer( SND_TIMER_OPEN_NONBLOCK |
                                         SND_TIMER_OPEN_TREAD );
    if (test_timer != 0) {
        m_count = 0;
        m_info = test_timer->getTimerInfo();
        try {
            m_params.setAutoStart(true);
            if (!m_info.isSlave()) {
                /* 50 Hz */
                m_params.setTicks( 1000000000L / m_info.getResolution() / 50);
                if (m_params.getTicks() < 1) {
                    m_params.setTicks(1);
                }
            } else {
                m_params.setTicks(1);
            }
            m_params.setFilter(1 << SND_TIMER_EVENT_TICK);
            test_timer->setTimerParams(m_params);
            test_timer->setHandler(this);
            // Testing timer callback method
            test_timer->start();
            test_timer->startEvents();
            QTest::qWait(1000);
            test_timer->stopEvents();
            test_timer->stop();
            QCOMPARE(m_count, 51);
            m_status = test_timer->getTimerStatus();
            QCOMPARE(m_status.getLost(), 0L);
            QCOMPARE(m_status.getOverrun(), 0L);
        } catch (...) {
            QFAIL("Timer test failed");
        }
        delete test_timer;
    }
}

QTEST_MAIN(AlsaTest2)

#include "alsatest2.moc"
