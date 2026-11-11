// SPDX-License-Identifier: GPL-3.0-or-later
// Keeps the display on while the app is in the foreground.
//
// Calls com.canonical.Unity.Screen.keepDisplayOn on the system bus (served by
// repowerd on Ubuntu Touch). Confined apps may only do this with the
// "keep-display-on" AppArmor policy group. The request is tied to our bus
// connection, so it is dropped automatically if the app exits or crashes.
#pragma once

#include <QObject>

class ScreenKeeper : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool keepOn READ keepOn WRITE setKeepOn NOTIFY keepOnChanged)
    Q_PROPERTY(bool active READ active NOTIFY statusChanged)
    Q_PROPERTY(QString status READ status NOTIFY statusChanged)

public:
    explicit ScreenKeeper(QObject *parent = nullptr);
    ~ScreenKeeper() override;

    bool keepOn() const { return m_keepOn; }
    void setKeepOn(bool on);
    bool active() const { return m_cookie >= 0; }
    QString status() const { return m_status; }

signals:
    void keepOnChanged();
    void statusChanged();

private:
    void request();
    void release();
    void setStatus(const QString &s);

    bool m_keepOn = false;
    bool m_pending = false;
    int m_cookie = -1;
    QString m_status = QStringLiteral("off");
};
