#include "testfitside.h"

#include "polygonviewer.h"
#include "Utils/polygonconnector.h"

testfitSide::testfitSide()
{

}

procon::Field testfitSide::test(procon::Field field)
{
    AlgorithmWrapper::AlgorithmWrapper(field);
    return field;
}
