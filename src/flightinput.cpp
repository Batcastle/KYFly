// SPDX-License-Identifier: GPL-3.0-or-later
#include "flightinput.h"

#include <QtMath>

FlightInput::FlightInput(QObject *parent)
    : QObject(parent)
{
}

void FlightInput::setAxes(int source, qreal yaw, qreal throttle, qreal roll, qreal pitch)
{
    yaw = clamp1(yaw);
    throttle = clamp1(throttle);
    roll = clamp1(roll);
    pitch = clamp1(pitch);

    if (source != m_active) {
        const bool deliberate = qAbs(yaw) > kTakeoverThreshold || qAbs(roll) > kTakeoverThreshold ||
                                qAbs(pitch) > kTakeoverThreshold ||
                                qAbs(throttle - m_axes.throttle) > kTakeoverThreshold;
        if (m_active != NoSource && !deliberate)
            return; // let the current source keep control
        m_active = source;
        emit activeSourceChanged();
    }

    m_lastInput.restart();
    if (yaw == m_axes.yaw && throttle == m_axes.throttle && roll == m_axes.roll && pitch == m_axes.pitch)
        return;
    m_axes.yaw = yaw;
    m_axes.throttle = throttle;
    m_axes.roll = roll;
    m_axes.pitch = pitch;
    emit axesChanged();
}

void FlightInput::releaseSource(int source)
{
    if (source != m_active)
        return;
    m_active = NoSource;
    m_axes.yaw = m_axes.roll = m_axes.pitch = 0;
    if (m_throttleSprings)
        m_axes.throttle = 0;
    emit activeSourceChanged();
    emit axesChanged();
    emit sourceLost(source);
}

void FlightInput::trigger(int action)
{
    if (action >= 0 && action < ActionCount)
        emit actionTriggered(action);
}

void FlightInput::resetThrottle()
{
    m_axes.throttle = m_throttleSprings ? 0 : -1;
    emit axesChanged();
}

void FlightInput::setThrottleSprings(bool on)
{
    if (on == m_throttleSprings)
        return;
    m_throttleSprings = on;
    emit throttleSpringsChanged();
}
