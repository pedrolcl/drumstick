/*
    Copyright (C) 2008-2010, Pedro Lopez-Cabanillas <plcl@users.sf.net>

    This file is part of the Drumstick project, see https://sf.net/p/drumstick

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; If not, see <http://www.gnu.org/licenses/>.
*/

#include <QString>
#include <QtTest>
#include <drumstick/alsatimer.h>

using namespace drumstick::ALSA;

class AlsaTest2 : public QObject, public TimerEventHandler
{
    Q_OBJECT

public:
    AlsaTest2();
    // TimerEventHandler implementation
    void handleTimerEvent(int ticks, int msecs) override;

private Q_SLOTS:
    void testTimer();
    void initTestCase();
    void cleanupTestCase();

private:
    QPointer<Timer> m_test_timer;
    int             m_count;
};

AlsaTest2::AlsaTest2():
    m_test_timer(nullptr),
    m_count(0)
{ }

void AlsaTest2::handleTimerEvent(int , int )
{
    m_count++;
}

void AlsaTest2::initTestCase()
{
    try {
        QFileInfo check_devsnd("/dev/snd/");
        QVERIFY(check_devsnd.exists() && check_devsnd.isDir());
        QFileInfo check_devsndseq("/dev/snd/seq");
        QVERIFY(check_devsndseq.exists() && !check_devsndseq.isFile() && !check_devsndseq.isDir());
        QFileInfo check_devsndtimer("/dev/snd/timer");
        QVERIFY(check_devsndtimer.exists() && !check_devsndseq.isFile() && !check_devsndseq.isDir());
        m_test_timer = Timer::bestGlobalTimer( SND_TIMER_OPEN_NONBLOCK |
                                               SND_TIMER_OPEN_TREAD );
    } catch (...) {
        QWARN("Timer test initialization failed");
    }
}

void AlsaTest2::cleanupTestCase()
{
    delete m_test_timer;
}

void AlsaTest2::testTimer()
{
    if (m_test_timer != nullptr) {
        m_count = 0;
        try {
            TimerParams tparams;
            TimerInfo tinfo = m_test_timer->getTimerInfo();
            tparams.setAutoStart(true);
            if (!tinfo.isSlave()) {
                /* 50 Hz */
                tparams.setTicks( 1000000000L / tinfo.getResolution() / 50);
                if (tparams.getTicks() < 1) {
                    tparams.setTicks(1);
                }
            } else {
                tparams.setTicks(1);
            }
            tparams.setFilter(1 << SND_TIMER_EVENT_TICK);
            m_test_timer->setTimerParams(tparams);
            m_test_timer->setHandler(this);
            // Testing timer callback method
            m_test_timer->start();
            m_test_timer->startEvents();
            QTest::qWait(1000);
            m_test_timer->stopEvents();
            m_test_timer->stop();
            QVERIFY2(qAbs(50 - m_count) <= 1, "Timer results are wrong");
            TimerStatus tstatus = m_test_timer->getTimerStatus();
            QCOMPARE(tstatus.getLost(), 0L);
            QCOMPARE(tstatus.getOverrun(), 0L);
        } catch (...) {
            QFAIL("Timer test failed");
        }
    }
}

QTEST_GUILESS_MAIN(AlsaTest2)

#include "alsatest2.moc"
