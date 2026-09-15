#include "eventfilter.h"

#include <QKeyEvent>

EventFilter::EventFilter(QObject *parent)
    : QObject{parent}
{}

bool EventFilter::eventFilter(QObject *watched, QEvent *event)
{
    if (!m_enabled || (m_handler == nullptr)) {
        return QObject::eventFilter(watched, event);
    }

    if (event->type() == QEvent::Type::KeyPress || event->type() == QEvent::Type::KeyRelease) {
        auto keyEvent = static_cast<QKeyEvent *>(event);
        if (!keyEvent->isAutoRepeat()) {
            int keyid =
#if defined(Q_OS_MACOS)
                keyEvent->nativeVirtualKey();
#else
                keyEvent->nativeScanCode();
#endif
            // qDebug() << Q_FUNC_INFO << keyEvent->type() << "key:" << keyid;
            if (keyEvent->type() == QEvent::Type::KeyPress) {
                return m_handler->handleKeyPressed(keyid);
            } else if (keyEvent->type() == QEvent::Type::KeyRelease) {
                return m_handler->handleKeyReleased(keyid);
            }
            return true;
        }
    }
    return QObject::eventFilter(watched, event);
}
