#include "pch.h"
#include "engine/timer/timer.h"


namespace chr = std::chrono;


template<typename MeasureUnits>
static float GetDuration(const chr::steady_clock::time_point& A, const chr::steady_clock::time_point& B) noexcept
{
    return chr::duration_cast<MeasureUnits>(B - A).count();
}


Timer::Timer()
    : m_startTime(chr::steady_clock::now()), 
    m_prevTime(chr::steady_clock::now()), 
    m_curTime(chr::steady_clock::now())
{
}


void Timer::Reset() noexcept
{
    m_startTime = chr::steady_clock::now();
}


float Timer::GetElapsedTimeInSec() const noexcept
{
    return GetDuration<chr::seconds>(m_startTime, m_curTime);
}


float Timer::GetElapsedTimeInMillisec() const noexcept
{
    return GetDuration<chr::milliseconds>(m_startTime, m_curTime);
}


void Timer::Tick() noexcept
{
    m_prevTime = m_curTime;
    m_curTime = chr::steady_clock::now();
}


float Timer::GetDeltaTimeInSec() const noexcept
{
    return GetDuration<chr::seconds>(m_prevTime, m_curTime);
}


float Timer::GetDeltaTimeInMillisec() const noexcept
{
    return GetDuration<chr::milliseconds>(m_prevTime, m_curTime);
}
