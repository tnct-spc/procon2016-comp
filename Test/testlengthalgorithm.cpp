#include "testlengthalgorithm.h"
#include "Algorithm/lengthalgorithm.h"

testLengthAlgorithm::testLengthAlgorithm()
{

}

bool testLengthAlgorithm::run(){
    lengthalgorithm solver;
    solver.test();
    debugprint("OK");
    return true;
}
