#include "testlengthalgorithm.h"
#include "Algorithm/lengthalgorithm.h"

testLengthAlgorithm::testLengthAlgorithm()
{

}

bool testLengthAlgorithm::run(){
    lengthalgorithm solver;
    procon::Field fake_field;
    solver.test(fake_field);
    debugprint("OK");
    return true;
}
