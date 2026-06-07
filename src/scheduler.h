#pragma once

#include <functional>
#include <vector>
#include <mutex>
#include <queue>

#define TIMER_FLAG_REPEAT       (1 << 0)
#define TIMER_FLAG_NO_MAPCHANGE (1 << 1)

struct Timer
{
    float Interval;
    double ExecTime;
    std::function<void()> Callback;
    int Flags;
    bool KillMe = false;
    bool InExec = false;

    Timer(float interval, double execTime, std::function<void()> cb, int flags)
        : Interval(interval), ExecTime(execTime), Callback(std::move(cb)), Flags(flags)
    {
    }
};

extern double g_dUniversalTime;

namespace scheduler
{
    void Init();
    void Shutdown();
    void Tick(bool simulating = true);
    void RemoveMapChangeTimers();

    Timer* AddTimer(float interval, std::function<void()> callback, int flags = 0);
    void KillTimer(Timer* timer);
    void NextFrame(std::function<void()> task);
}
