#ifndef TIMECOUNTER_H
#define TIMECOUNTER_H

#include <chrono>

namespace procon {
class TimeCounter
{
protected:
    std::chrono::system_clock::time_point start_time;
public:
    TimeCounter();
    void startTimer();
    double getElapsedTime();
};
}

#endif // TIMECOUNTER_H
