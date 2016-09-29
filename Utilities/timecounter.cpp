#include "timecounter.h"

procon::TimeCounter::TimeCounter()
{

}

void procon::TimeCounter::startTimer()
{
    start_time = std::chrono::system_clock::now();
}

double procon::TimeCounter::getElapsedTime()
{
    auto end_time = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
}
