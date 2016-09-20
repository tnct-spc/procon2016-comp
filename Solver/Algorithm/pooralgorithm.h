#ifndef POORALGORITHM_H
#define POORALGORITHM_H

#include "algorithmwrapper.h"

class PoorAlgorithm : public AlgorithmWrapper
{
public:
    PoorAlgorithm();
    procon::Field run(procon::Field field);
};

#endif // POORALGORITHM_H
