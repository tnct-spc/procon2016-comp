#include "solver.h"

#include "singlepolygondisplay.h"
#include "Algorithm/pooralgorithm.h"
#include "Algorithm/simplealgorithm.h"
#include "Algorithm/hillclibming.h"
#include "Algorithm/beamsearch.h"
#include "Algorithm/beamsearchbylength.h"
#include "Algorithm/chokudaisearch.h"

Solver::Solver()
{
}

procon::Field Solver::run(procon::Field field, int algorithm_number)
{
    std::vector<AlgorithmWrapper*> Algorithms;
    Algorithms.push_back(new PoorAlgorithm());
    Algorithms.push_back(new SimpleAlgorithm());
    Algorithms.push_back(new HillClibming());
    Algorithms.push_back(new BeamSearch());
    Algorithms.push_back(new BeamSearchByLength());
    Algorithms.push_back(new ChokudaiSearch());

    //procon::Field result = Algorithms.at(algorithm_number)->run(field);
    procon::Field result = Algorithms.at(5)->run(field);

    for(auto algo : Algorithms) delete(algo);
    Algorithms.clear();

    return result;
}

