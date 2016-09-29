#ifndef CHOKUDAISEARCH_H
#define CHOKUDAISEARCH_H
#include "Algorithm/beamsearch.h"

struct NeoField {
    procon::Field field;
    Evaluation eva;
};


class ChokudaiSearch : public BeamSearch
{
protected:
    double beam_width = 1;
public:
    ChokudaiSearch();
    void initialization();
    procon::Field run(procon::Field field);
    std::vector<NeoField> evaluateNextMove(NeoField const& field);
    std::vector<NeoField> makeNextField(std::vector<NeoField> const& fields);

};

#endif // CHOKUDAISEARCH_H
