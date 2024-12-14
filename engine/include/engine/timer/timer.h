#pragma once

#include <chrono>


class Timer
{
public:
    Timer();

    void Reset() noexcept;

    float GetElapsedTimeInSec() const noexcept;
    float GetElapsedTimeInMillisec() const noexcept;

    void Tick() noexcept;
    
    float GetDeltaTimeInSec() const noexcept;
    float GetDeltaTimeInMillisec() const noexcept;

private:
    std::chrono::steady_clock::time_point m_startTime;
    
    std::chrono::steady_clock::time_point m_prevTime;
    std::chrono::steady_clock::time_point m_curTime;
};