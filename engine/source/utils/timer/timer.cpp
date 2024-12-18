#include "pch.h"
#include "engine/timer/timer.h"


namespace chr = std::chrono;


template<typename MeasureUnits>
static double GetDuration(const chr::steady_clock::time_point& A, const chr::steady_clock::time_point& B) noexcept
{
    return chr::duration_cast<MeasureUnits>(B - A).count();
}


Timer::Timer()
    : m_startTime(chr::steady_clock::now()), 
    m_prevTime(chr::steady_clock::now()), 
    m_curTime(chr::steady_clock::now())
{
}


double Timer::GetElapsedTimeInSec() const noexcept
{
    return GetElapsedTimeInMillisec() / 1000.0;
}


double Timer::GetElapsedTimeInMillisec() const noexcept
{
    return GetDuration<chr::milliseconds>(m_startTime, chr::steady_clock::now());
}


void Timer::Tick() noexcept
{
    m_prevTime = m_curTime;
    m_curTime = chr::steady_clock::now();
}


double Timer::GetDeltaTimeInSec() const noexcept
{
    return GetDeltaTimeInMillisec() / 1000.0;
}


double Timer::GetDeltaTimeInMillisec() const noexcept
{
    return GetDuration<chr::milliseconds>(m_prevTime, m_curTime);
}
