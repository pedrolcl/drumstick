#ifndef EVENTFILTER_H
#define EVENTFILTER_H

#include <QObject>
#include <drumstick/pianokeybd.h>

using namespace drumstick::widgets;

class EventFilter : public QObject
{
    Q_OBJECT
public:
    explicit EventFilter(QObject *parent = nullptr);

    RawKbdHandler *getRawKbdHandler() { return m_handler; }
    void setRawKbdHandler(RawKbdHandler *h) { m_handler = h; }
    bool isRawKbdEnabled() { return m_enabled; }
    void setRawKbdEnabled(bool b) { m_enabled = b; }

    // QObject interface
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    bool m_enabled{false};
    RawKbdHandler *m_handler{nullptr};
};

#endif // EVENTFILTER_H
