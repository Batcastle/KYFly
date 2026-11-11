// SPDX-License-Identifier: GPL-3.0-or-later
#include "screenkeeper.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>

namespace {
const QString kService = QStringLiteral("com.canonical.Unity.Screen");
const QString kPath = QStringLiteral("/com/canonical/Unity/Screen");
const QString kInterface = QStringLiteral("com.canonical.Unity.Screen");
} // namespace

ScreenKeeper::ScreenKeeper(QObject *parent)
    : QObject(parent)
{
}

ScreenKeeper::~ScreenKeeper()
{
    release();
}

void ScreenKeeper::setKeepOn(bool on)
{
    if (on == m_keepOn)
        return;
    m_keepOn = on;
    emit keepOnChanged();
    if (on)
        request();
    else
        release();
}

void ScreenKeeper::request()
{
    if (m_cookie >= 0 || m_pending)
        return;
    QDBusConnection bus = QDBusConnection::systemBus();
    if (!bus.isConnected()) {
        setStatus(QStringLiteral("no system bus"));
        return;
    }
    // Plain method call (no QDBusInterface) to avoid an introspection round trip
    // that confinement might not allow.
    const QDBusMessage msg = QDBusMessage::createMethodCall(kService, kPath, kInterface,
                                                            QStringLiteral("keepDisplayOn"));
    m_pending = true;
    auto *watcher = new QDBusPendingCallWatcher(bus.asyncCall(msg, 3000), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher *w) {
        QDBusPendingReply<int> reply = *w;
        w->deleteLater();
        m_pending = false;
        if (reply.isError()) {
            setStatus(QStringLiteral("failed: ") + reply.error().name());
            return;
        }
        m_cookie = reply.value();
        if (!m_keepOn) { // switched off while the call was in flight
            release();
            return;
        }
        setStatus(QStringLiteral("on"));
    });
}

void ScreenKeeper::release()
{
    if (m_cookie < 0)
        return;
    QDBusMessage msg = QDBusMessage::createMethodCall(kService, kPath, kInterface,
                                                      QStringLiteral("removeDisplayOnRequest"));
    msg << m_cookie;
    QDBusConnection::systemBus().asyncCall(msg);
    m_cookie = -1;
    setStatus(QStringLiteral("off"));
}

void ScreenKeeper::setStatus(const QString &s)
{
    if (s == m_status)
        return;
    m_status = s;
    emit statusChanged();
}
