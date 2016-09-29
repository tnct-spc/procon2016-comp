#include "testutilities.h"
#include "timecounter.h"

TestUtilities::TestUtilities()
{

}

bool TestUtilities::run()
{
    procon::TimeCounter t;
    t.startTimer();
    volatile long int a;
    while(t.getElapsedTime() < 3000) {
        std::cout << t.getElapsedTime() << std::endl;
        a++;
    }
    print(a);
}
