#pragma once

#include <chrono>


class Timer
{
public:
    Timer();

    void Tick() noexcept;

    double GetElapsedTimeInSec() const noexcept;
    double GetElapsedTimeInMillisec() const noexcept;
    
    double GetDeltaTimeInSec() const noexcept;
    double GetDeltaTimeInMillisec() const noexcept;

private:
    std::chrono::steady_clock::time_point m_startTime;
    
    std::chrono::steady_clock::time_point m_prevTime;
    std::chrono::steady_clock::time_point m_curTime;
};