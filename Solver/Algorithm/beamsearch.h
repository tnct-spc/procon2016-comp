#ifndef BEAMSEARCH_H
#define BEAMSEARCH_H

#include "field.h"
#include "Utils/evaluation.h"
#include "Utils/polygonconnector.h"
#include "algorithmwrapper.h"
#include <queue>
#include <algorithm>

class BeamSearch : public AlgorithmWrapper
{
private:
protected:
    //ビーム幅
    const double beam_width = 100;

    virtual void evaluateNextMove
    (std::vector<Evaluation> & evaluations,std::vector<procon::Field> const& field_vec);
    virtual std::vector<procon::Field> makeNextField
    (std::vector<Evaluation> const& evaluations,std::vector<procon::Field> const& field_vec);
public:
    BeamSearch();
    procon::Field run(procon::Field field);
};


#endif // BEAMSEARCH_H
