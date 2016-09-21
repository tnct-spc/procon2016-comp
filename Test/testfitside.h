#ifndef TESTFITSIDE_H
#define TESTFITSIDE_H
#include "testerwraper.h"
#include "field.h"

class testfitSide : public TesterWraper
{
public:
    testfitSide();
    procon::Field test(procon::Field field);
};

#endif // TESTFITSIDE_H
